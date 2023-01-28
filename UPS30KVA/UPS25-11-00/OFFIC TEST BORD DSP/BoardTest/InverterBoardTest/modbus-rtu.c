
#include "DSP28x_Project.h"
#include "DSP2833x_CpuTimers.h"
#include "modbus-rtu.h"

extern float ReadOnlyData[16];
extern float ReadWriteData[16];

extern Uint16 CheckSettings(Uint16 add, Uint16 val);

#define OUTPUT_REG_QTY			40
//extern Uint16 OutputRegister[OUTPUT_REG_QTY];

/* SciA used variable */
Uint16 sdataA[sdataALentgh];
Uint16 rdataA[rdataALentgh];
Uint16 rdata_pointA;
Uint16 sdata_pointA;
Uint16 SdataEndIndex;

Uint16 time_out_reached = 0;
static Uint16 EntryHoldingRegAdd;
static Uint16 EntryHoldingRegQty;
static Uint16 EntryCoilAdd;
static Uint16 EntryCoilQty;

Uint16 *HoldingRegPtr;
static Uint16 HoldingRegStartAdd;
static Uint16 HoldingRegEndAddress;
static Uint16 HoldingRegQty;
Uint16 *InputRegPtr;
static Uint16 InputRegStartAdd;
static Uint16 InputRegEndAddress;
static Uint16 InputRegQty;
Uint16 *CoilPtr;
static Uint16 CoilStartAdd;
static Uint16 CoilEndAddress;
static Uint16 CoilQty;

void modbus_rtu_init(Uint16* holdingRegPtr, Uint16 holdingRegQty, Uint16 holdingRegStartAdd,
		Uint16* inputRegPtr, Uint16 inputRegQty, Uint16 inputRegStartAdd,
		Uint16* coilPtr, Uint16 coilQty, Uint16 coilStartAdd)
{
	HoldingRegPtr = holdingRegPtr;
	HoldingRegQty = holdingRegQty;
	HoldingRegStartAdd = holdingRegStartAdd;
	HoldingRegEndAddress = (holdingRegStartAdd + holdingRegQty); // - 1

	InputRegPtr = inputRegPtr;
	InputRegQty = inputRegQty;
	InputRegStartAdd = inputRegStartAdd;
	InputRegEndAddress = (inputRegStartAdd + inputRegQty); // - 1

	CoilPtr = coilPtr;
	CoilQty = coilQty;
	CoilStartAdd = coilStartAdd;
	CoilEndAddress = (coilStartAdd + coilQty); // - 1

	GPIO_Setup();

	EALLOW;
	PieVectTable.SCIRXINTB = &sciaRxFifoIsr;
	PieVectTable.SCITXINTB = &sciaTxFifoIsr;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;

	rdata_pointA=0;
	sdata_pointA=0;
	SdataEndIndex=0;

	scia_fifo_init();

	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, 150, 2300);

	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx4 = 1;

	//macro_dre_rx();
}

static void GPIO_Setup(void)
{
	EALLOW;
//--------------------------------------------------------------------------------------
//  GPIO-24 - PIN FUNCTION = --Spare--
	//GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;	// 0=GPIO,  1=ECAP1,  2=EQEPA-2,  3=MDX-B
	//GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;		// 1=OUTput,  0=INput
//	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;	// uncomment if --> Set Low initially
	//GpioDataRegs.GPASET.bit.GPIO24 = 1;		// uncomment if --> Set High initially
//--------------------------------------------------------------------------------------
//  GPIO-28 - PIN FUNCTION = SCI-RX
	GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 2;	// 0=GPIO,  1=SCIRX-A,  2=Resv,  3=Resv
	GpioCtrlRegs.GPADIR.bit.GPIO19 = 0;		// 1=OUTput,  0=INput
//	GpioDataRegs.GPACLEAR.bit.GPIO28 = 1;	// uncomment if --> Set Low initially
//	GpioDataRegs.GPASET.bit.GPIO28 = 1;		// uncomment if --> Set High initially
//--------------------------------------------------------------------------------------
//  GPIO-29 - PIN FUNCTION = SCI-TX
	GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;   // Enable pullup on GPIO29
	GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 2;	// 0=GPIO,  1=SCITXD-A,  2=XA19,  3=Resv
	GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;		// 1=OUTput,  0=INput
//	GpioDataRegs.GPACLEAR.bit.GPIO29 = 1;	// uncomment if --> Set Low initially
//	GpioDataRegs.GPASET.bit.GPIO29 = 1;		// uncomment if --> Set High initially
//--------------------------------------------------------------------------------------
	EDIS;	// Disable register access
}

#pragma CODE_SECTION(cpu_timer0_isr, "ramfuncs2");
interrupt void cpu_timer0_isr(void)
{
	/* stop timer */
	CpuTimer0Regs.TCR.bit.TSS = 1;
    time_out_reached = 1;
    PieCtrlRegs.PIEACK.bit.ACK1 = 1;
}

#pragma CODE_SECTION(ReadHoldingRegister, "ramfuncs2");
static void ReadHoldingRegister(void)
{
	Uint16 StartingAdd;
	Uint16 QuantityOfReg;
	Uint16 i;
	Uint16 Crc;
	Uint16 add;

	StartingAdd = (rdataA[2] << 8) | (rdataA[3]);
	QuantityOfReg = (rdataA[4] << 8) | (rdataA[5]);

	/* check address */
	if ((StartingAdd < HoldingRegStartAdd) || (StartingAdd > HoldingRegEndAddress))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0083;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	add = StartingAdd - HoldingRegStartAdd + 1;

	/* check quantity */
	if ((QuantityOfReg < 1) || (QuantityOfReg > HoldingRegQty))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0083;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	if ((add + QuantityOfReg - 1) > HoldingRegQty)
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0083;
		sdataA[2] = 2;
		Crc=CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc&0x00ff);
		sdataA[4] = (Crc&0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	/* response */
	sdataA[0] = MODBUS_SLAVE_ID;
	sdataA[1] = 0x0003;
	sdataA[2] = 2 * rdataA[5];

	for (i = 0; i < QuantityOfReg; i++)
	{
		sdataA[3 + (2 * i)] = (HoldingRegPtr[i + add] & 0xff00) >> 8;
		sdataA[4 + (2 * i)] = HoldingRegPtr[i + add] & 0x00ff;
	}

	Crc = CrcCalculate(sdataA, 0, 2 + (2 * QuantityOfReg));
	sdataA[3 + (2 * QuantityOfReg)] = (Crc & 0x00ff);
	sdataA[4 + (2 * QuantityOfReg)] = (Crc & 0xff00) >> 8;
	SdataEndIndex = 4 + (2 * QuantityOfReg);
	scia_Tx_fifo_init();
	return;
}

#pragma CODE_SECTION(ReadInputRegister, "ramfuncs2");
static void ReadInputRegister(void)
{
	Uint16 StartingAdd;
	Uint16 QuantityOfReg;
	Uint16 i;
	Uint16 Crc;
	Uint16 add; /* holds the SRAM array index */

	StartingAdd = (rdataA[2] << 8) | (rdataA[3]);
	QuantityOfReg = (rdataA[4] << 8) | (rdataA[5]);

	/* check address */
	if ((StartingAdd < InputRegStartAdd) || (StartingAdd > InputRegEndAddress))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	add = StartingAdd - InputRegStartAdd + 1;

	/* check quantity */
	if ((QuantityOfReg < 1) || (QuantityOfReg > InputRegQty))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	if ((add + QuantityOfReg - 1) > (InputRegQty))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 2;
		Crc=CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc&0x00ff);
		sdataA[4] = (Crc&0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return;
	}

	/* response */
	sdataA[0] = MODBUS_SLAVE_ID;
	sdataA[1] = 0x0004;
	sdataA[2] = 2 * rdataA[5];

	for (i = 0; i < QuantityOfReg; i++)
	{
		sdataA[3 + (2 * i)] = (InputRegPtr[i + add] & 0xff00) >> 8;
		sdataA[4 + (2 * i)] = InputRegPtr[i + add] & 0x00ff;
	}

	Crc = CrcCalculate(sdataA, 0, 2 + (2 * QuantityOfReg));
	sdataA[3 + (2 * QuantityOfReg)] = (Crc & 0x00ff);
	sdataA[4 + (2 * QuantityOfReg)] = (Crc & 0xff00) >> 8;
	SdataEndIndex = 4 + (2 * QuantityOfReg);
	scia_Tx_fifo_init();
	return;
}

/* return value is number of holding register which recieved */
#pragma CODE_SECTION(WriteSingleCoil, "ramfuncs2");
static Uint16 WriteSingleCoil(void)
{
	Uint16 RegAdd;
	Uint16 i;
	Uint16 Crc;
	Uint16 add; /* holds the SRAM array index */
	Uint16 val;

	//EntryHoldingRegQty = 0;
	RegAdd = (rdataA[2] << 8) | (rdataA[3]);

	/* check address */
	if ((RegAdd < CoilStartAdd) || (RegAdd > CoilEndAddress))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0085;
		sdataA[2] = 2;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}

	/* get value */
	add = RegAdd - CoilStartAdd + 1;

	/* check value */
	val = (rdataA[4] << 8) | rdataA[5];
	if (val != 0x0000 && val != 0xFF00)
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0085;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}
	
	CoilPtr[add] = val;

	/* response */
	for (i = 0; i <= 7; i++) sdataA[i] = rdataA[i];

	SdataEndIndex = 7;
	scia_Tx_fifo_init();
	EntryCoilAdd = add;
	return 1;
}

/* return value is number of holding register which recieved */
#pragma CODE_SECTION(WriteSingleRegister, "ramfuncs2");
static Uint16 WriteSingleRegister(void)
{
	Uint16 RegAdd;
	Uint16 i;
	Uint16 Crc;
	Uint16 add; /* holds the SRAM array index */
	Uint16 val;

	//EntryHoldingRegQty = 0;
	RegAdd = (rdataA[2] << 8) | (rdataA[3]);

	/* check address */
	if ((RegAdd < HoldingRegStartAdd) || (RegAdd > HoldingRegEndAddress))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0086;
		sdataA[2] = 2;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}

//	OutputRegister[rdataA[3]]=(rdataA[4]<<8)|rdataA[5];
	
	add = RegAdd - HoldingRegStartAdd + 1;

	/* check value */
	val = (rdataA[4] << 8) | rdataA[5];
	if (!CheckSettings(add, val))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0086;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}
	/* getting value */
	HoldingRegPtr[add] = val;

	/* response */
	for (i = 0; i <= 7; i++) sdataA[i] = rdataA[i];

	SdataEndIndex = 7;
	scia_Tx_fifo_init();
	EntryHoldingRegAdd = add;
	return 1;
}

/* return value is number of holding register which recieved */
#pragma CODE_SECTION(WriteMultipleRegister, "ramfuncs2");
static Uint16 WriteMultipleRegister(void)
{
	Uint16 StartingAdd;
	Uint16 QuantityOfReg;
	Uint16 i, j;
	Uint16 Crc;
	Uint16 add;
	unsigned char byteCnt;

	StartingAdd = (rdataA[2] << 8) | (rdataA[3]);
	QuantityOfReg = (rdataA[4] << 8) | (rdataA[5]);
	byteCnt = rdataA[6];

	/* check address */
	if ((StartingAdd < HoldingRegStartAdd) || (StartingAdd > HoldingRegEndAddress))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0086;
		sdataA[2] = 2;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}

	add = StartingAdd - HoldingRegStartAdd;

	/* check quantity */
	if ((QuantityOfReg < 1) || (QuantityOfReg > InputRegQty))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 3;
		Crc = CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc & 0x00ff);
		sdataA[4] = (Crc & 0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}

	if ((add + QuantityOfReg) > InputRegQty)//ModbusInputLentgh
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 2;
		Crc=CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc&0x00ff);
		sdataA[4] = (Crc&0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}

	/* check byte cnt */
	if (byteCnt != (QuantityOfReg * 2))
	{
		sdataA[0] = MODBUS_SLAVE_ID;
		sdataA[1] = 0x0084;
		sdataA[2] = 2;
		Crc=CrcCalculate(sdataA, 0, 2);
		sdataA[3] = (Crc&0x00ff);
		sdataA[4] = (Crc&0xff00) >> 8;
		SdataEndIndex = 4;
		scia_Tx_fifo_init();
		return 0;
	}
	/* check all the value */

	/* update map with the value */
	j = 7;
	for (i = 0; i < QuantityOfReg; i++)
	{
		HoldingRegPtr[add + i] = (rdataA[j] << 8) | rdataA[j + 1];
		j += 2;
	}

	/* response */
	sdataA[0] = MODBUS_SLAVE_ID;
	sdataA[1] = 0x0010;
	sdataA[2] = rdataA[2];
	sdataA[3] = rdataA[3];
	sdataA[4] = rdataA[4];
	sdataA[5] = rdataA[5];

	Crc = CrcCalculate(sdataA, 0, 5);
	sdataA[6] = (Crc & 0x00ff);
	sdataA[7] = (Crc & 0xff00) >> 8;
	SdataEndIndex = 7;
	scia_Tx_fifo_init();
	EntryHoldingRegAdd = add;
	return QuantityOfReg;
}

#pragma CODE_SECTION(FunError, "ramfuncs2");
static void FunError(void)
{
	Uint16 Crc;

	sdataA[0] = MODBUS_SLAVE_ID;
	sdataA[1] = sdataA[1] | 0x0080;
	sdataA[2] = 1;
	Crc = CrcCalculate(sdataA, 0, 2);
	sdataA[3] = (Crc & 0x00ff);
	sdataA[4] = (Crc & 0xff00) >> 8;
	SdataEndIndex = 4;
	scia_Tx_fifo_init();
	return;
}

#pragma CODE_SECTION(CrcCalculate, "ramfuncs2");
static Uint16 CrcCalculate(Uint16 array[],Uint16 StIndex,Uint16 EndIndex)
{
	Uint16 Pnt = StIndex;
	Uint16 Crc = 0xffff;
	Uint16 BitCounter;

	while (Pnt <= EndIndex)
	{
		Crc = Crc ^ array[Pnt];
		for	(BitCounter = 0 ; BitCounter < 8 ; BitCounter++)
		{
			if ((Crc & 1) == 1)
			{
				Crc = Crc >> 1;
				Crc = Crc ^ 0xa001;
			}
			else
			Crc = Crc >> 1;
		}
		Pnt++;
	}
	return Crc;
}

#pragma CODE_SECTION(scia_Tx_fifo_init, "ramfuncs2");
void scia_Tx_fifo_init(void)
{
	/* DRE */
//	macro_dre_tx();
	ScibRegs.SCICTL1.bit.RXENA=0;
	sdata_pointA = 0;
	ScibRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
}

#pragma CODE_SECTION(sciaTxFifoIsr, "ramfuncs2");
interrupt void sciaTxFifoIsr(void)
{
	if (sdata_pointA > SdataEndIndex)
	{
//		macro_dre_rx();
		ScibRegs.SCICTL1.bit.RXENA=1;
		ScibRegs.SCIFFTX.bit.TXFIFOXRESET=0;
	}
	
	while ((ScibRegs.SCIFFTX.bit.TXFFST<16)&&(sdata_pointA<=SdataEndIndex))
	{
		ScibRegs.SCITXBUF=sdataA[sdata_pointA];
		sdata_pointA++;
	}

	ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;
	PieCtrlRegs.PIEACK.all|=0x100;
}

#pragma CODE_SECTION(sciaRxFifoIsr, "ramfuncs2");
interrupt void sciaRxFifoIsr(void)
{
	//GpioDataRegs.GPASET.bit.GPIO24 = 1;
	if (ScibRegs.SCIRXST.bit.RXERROR==1)
	{
		ScibRegs.SCICTL1.bit.SWRESET=0;
		ScibRegs.SCICTL1.bit.SWRESET=1;
		ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;
		PieCtrlRegs.PIEACK.bit.ACK9 = 1;	
		return;
	}

	rdataA[rdata_pointA]=(ScibRegs.SCIRXBUF.all)&0x00ff;
    
    if ((ScibRegs.SCICTL1.bit.SLEEP == 1) && (ScibRegs.SCIRXEMU == MODBUS_SLAVE_ID))
    {
    	ScibRegs.SCICTL1.bit.SLEEP = 0;
    	CpuTimer0Regs.TCR.bit.TRB = 1;
    	CpuTimer0Regs.TCR.bit.TSS = 0;
    }
    	
	if (ScibRegs.SCICTL1.bit.SLEEP == 0)
	{
		if (rdata_pointA < rdataALentgh) rdata_pointA++;
		CpuTimer0Regs.TCR.bit.TRB = 1;
	}
	
	ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;
	//GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;
}

void scia_fifo_init(void)
{
	ScibRegs.SCICCR.all =0x0067;  	 	// 1 stop bit,  No loopback
	                                  	// Even parity,8 char bits,
	                                  	// async mode, idle-line protocol
	ScibRegs.SCICTL1.all =0x0047;  		// enable TX, RX, internal SCICLK, SLEEP,RX ERR
	                     		    	// Disable TXWAKE  
	ScibRegs.SCIHBAUD = 0x0000;			//Baud rate=38400
	ScibRegs.SCILBAUD = 0x0079;
	
	ScibRegs.SCIFFTX.all=0xC020;
	ScibRegs.SCIFFRX.all=0x0021;
	ScibRegs.SCIFFCT.all=0x00;

	ScibRegs.SCICTL1.all =0x0067;
	ScibRegs.SCIFFRX.bit.RXFIFORESET=1;
}

/* return value is recieved function code */
Uint16 ModbusHandler(void)
{
	Uint16 Crc;
	Uint16 rtn = 0;

	if(time_out_reached == 1)
	{
		rdata_pointA--;
		Crc = CrcCalculate(rdataA,0,rdata_pointA);
		if (Crc == 0)
		{
			switch (rdataA[1])
			{
				case 3:
					ReadHoldingRegister();
					rtn = 3;
					break;
				case 4:
					ReadInputRegister();
					rtn = 4;
					break;
				case 5:
					WriteSingleCoil();
					rtn = 5;
					break;
				case 6:
					WriteSingleRegister();
					rtn = 6;
					break;
				case 16:
					WriteMultipleRegister();
					rtn = 16;
					break;
				default:
					FunError();
					break;
			}
		}

	    rdata_pointA=0;
		time_out_reached=0;
		ScibRegs.SCICTL1.bit.SLEEP=1;
	}

	return rtn;
}

Uint16 ModbusEntryHoldingRegistrAddress(void)
{
	return EntryHoldingRegAdd;
}

Uint16 ModbusEntryHoldingRegistrQty(void)
{
	return EntryHoldingRegQty;
}

Uint16 ModbusEntryCoilAddress(void)
{
	return EntryCoilAdd;
}

Uint16 ModbusEntryCoilQty(void)
{
	return EntryCoilQty;
}

