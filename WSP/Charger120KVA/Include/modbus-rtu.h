
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#define CPU_FREQ			150E6
#define LSPCLK_FREQ			CPU_FREQ/4
#define SCI_FREQ			100E3
#define SCI_PRD				(LSPCLK_FREQ/(SCI_FREQ*8))-1
#define	MODBUS_SLAVE_ID		1
#define	ModbusInputLentgh	40
#define	ModbusOutputLentgh	40
#define	sdataALentgh		256
#define	rdataALentgh		256
#define	True				1
#define	False				0

#define MODBUS_RTU_COIL_ON	0xFF00
#define MODBUS_RTU_COIL_OFF 0x0000

#define	MODBUS_CPU_INTERRUPTS 0x101

//#define macro_dre_tx() GpioDataRegs.GPASET.bit.GPIO24 = 1
//#define macro_dre_rx() GpioDataRegs.GPACLEAR.bit.GPIO24 = 1

void sciaTxFifoIsr(void);
void sciaRxFifoIsr(void);
void cpu_timer0_isr(void);
void scia_Tx_fifo_init(void);
void scia_fifo_init(void);
void modbus_rtu_init(
		Uint16* holdingRegPtr, Uint16 holdingRegQty, Uint16 holdingRegStartAdd,
		Uint16* inputRegPtr, Uint16 inputRegQty, Uint16 inputRegStartAdd,
		Uint16* coilPtr, Uint16 coilQty, Uint16 coilStartAdd);
static void GPIO_Setup(void);
static void ReadHoldingRegister(void);
static void ReadInputRegister(void);
static Uint16 WriteSingleCoil(void);
static Uint16 WriteSingleRegister(void);
static Uint16 WriteMultipleRegister(void);
static void FunError(void);
static Uint16 CrcCalculate(Uint16[], Uint16, Uint16);
Uint16 ModbusHandler(void);
Uint16 ModbusEntryHoldingRegistrAddress(void);
Uint16 ModbusEntryHoldingRegistrQty(void);
Uint16 ModbusEntryCoilAddress(void);
Uint16 ModbusEntryCoilQty(void);

#endif /* MODBUS_RTU_H */
