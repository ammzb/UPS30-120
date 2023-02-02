

#include "SolarInverter-Includes.h"
//#include "SPLL_3PH_SRF_F.h"
//#include "ABC_DQ0_POS_F.h"
#include "C28x_FPU_FastRTS.h"
#include "SPLL_1ph_SOGI_F.h"
//#include "sgen.h"
#include "CNTL_PI_F.h"
//#include "iPARK_F.h"
//#include "iCLARKE_F.h"
#include <math.h> 
//#include "svgen_F.h"
#include "DLOG_4CH_F.h"
#include "UPS_Config.h"
#include "CNTL_2P2Z_F.h"
/* MODBUS include header */
#include "modbus-rtu.h"
/* MODBUS include header END */

//lines 100
#define	APP_PWR_R_H 0X0004
#define	APP_PWR_R_L 0X0005
#define	APP_PWR_S_H 0X0006
#define	APP_PWR_S_L 0X0007
#define	APP_PWR_T_H 0X0008
#define	APP_PWR_T_L 0X0009
#define	INVERTER_R_CURRENT 0X000A
#define	INVERTER_S_CURRENT 0X000B
#define	INVERTER_T_CURRENT 0X000C
#define	POWER_FACTOR_R 0X000D
#define	POWER_FACTOR_ST 0X000E
#define	SHDN_REM_TIME 0X000F
#define	PARALLEL_CODE_12 0X0010
#define	PARALLEL_CODE_34 0X0011
#define	LOAD_T_VOLTAGE 0X0012
#define	LOAD_S_VOLTAGE 0X0013
#define	LOAD_R_VOLTAGE 0X0014
#define	OUT_PWR_R_H 0X0015
#define	OUT_PWR_R_L 0X0016
#define	OUT_PWR_S_H 0X0017
#define	OUT_PWR_S_L 0X0018
#define	OUT_PWR_T_H 0X0019
#define	OUT_PWR_T_L 0X001A
#define	FAULT_CODE 0X001B
#define	FIRMWARE_VERSION 0X001C
#define	BYPASS_R_VOLTAGE 0X001D
#define	INVERTER_T_VOLTAGE 0X001E
#define	LOAD_T_CURRRENT 0X001F
#define	INVERTER_S_VOLTAGE 0X0020
#define	LOAD_S_CURRENT 0X0021
#define	INVERTER_R_VOLTAGE 0X0022
#define	LOAD_R_CURRENT 0X0023
#define	STATUS_1 0X0024
#define	STATUS_2 0X0025
#define	STATUS_3 0X0026
#define	STATUS_4 0X0027


struct UPS_STATUS_1
{
	Uint16 MANUALLY_BYPASS:1;
	Uint16 INVERTER_ON_LOAD:1;
	Uint16 BYPASS_ON_LOAD:1;
	Uint16 SYNC:1;
	Uint16 OVER_LOAD_EXIST:1;
	Uint16 rsvd:11;
};



struct UPS_STATUS_2
{
	Uint16 IDF1:1;
	Uint16 IDF2:1; 
	Uint16 IDF3:1;
	Uint16 BYPASS_FREQUENCY_FAIL:1;
	Uint16 INVERTER_OVER_LOAD:1;
	Uint16 INVERTER_SHORT_CIRCUIT:1;
	Uint16 LOAD_BREAKER:1;
	Uint16 INVERTER_FAIL:1;
	Uint16 INV_STS_FAULT:1;
	Uint16 VOLTAGE_OUT_OF_RANGE:1;
	Uint16 BYPASS_STS_FAULT:1;
	Uint16 OVER_TEMP:1;
	Uint16 rsvd:4;

};

struct UPS_STATUS_3
{
	Uint16 rsvd:16;
};

struct UPS_STATUS_4
{
	Uint16 rsvd:16;
};

union 
{
	struct UPS_STATUS_1 Bit;
	Uint16 Reg;
} UpsStatus1;

union 
{
	struct UPS_STATUS_2 Bit;
	Uint16 Reg;
} UpsStatus2;

union 
{
	struct UPS_STATUS_3 Bit;
	Uint16 Reg;
} UpsStatus3;

union 
{
	struct UPS_STATUS_4 Bit;
	Uint16 Reg;
} UpsStatus4;

#define FLASH

#ifdef FLASH
#pragma CODE_SECTION(MainISR,"ramfuncs");
#endif

// Prototype statements for functions found within this file.
interrupt void MainISR(void);
void InitSysCtrl();
void MemCopy();
void InitFlash();
void ADC_Setup(void);
void PWM_INIT(PWMGEN*);
void PWM_Drive(PWMGEN*,float32,float32,float32,float32);
void ServiceDog(void);
void Turn_on_system(void);
void Turn_on_STS(void);
void Shut_down_system(void);
void Turn_on_bypass(void);
void Turn_on_power(void);
void OverLoadCounterRef(void);
void Reset_System(void);
extern void IO_Setup(void);
float32 maximum(float32,float32,float32);

Uint16 CheckSettings(Uint16 add, Uint16 val);

/* MODBUS local var */
//Uint16 entryHoldingRegQty;
//Uint16 entryHoldingRegAdd;
Uint16 modbusFn;
Uint16 modbusAdd;
//Uint16 modbusQty;
/* MODBUS local var END */

// State Machine function prototypes
//------------------------------------
// Alpha states
//void A0(void);	//state A0
void B0(void);	//state B0
void C0(void);	//state C0

// A branch states
//void A1(void);	//state A1
//void A2(void);	//state A2
//void A3(void);	//state A3

// B branch states
void B1(void);	//state B1
void B2(void);	//state B2
void B3(void);	//state B3

// C branch states
void C1(void);	//state C1
void C2(void);	//state C2
void C3(void);	//state C3

// Variable declarations
void (*Alpha_State_Ptr)(void);	// Base States pointer
//void (*A_Task_Ptr)(void);		// State pointer A branch
void (*B_Task_Ptr)(void);		// State pointer B branch
void (*C_Task_Ptr)(void);		// State pointer C branch

// Used for running BackGround in flash, and ISR in RAM
extern Uint16 *RamfuncsLoadStart, *RamfuncsLoadEnd, *RamfuncsRunStart;
extern Uint16 *Ramfuncs2LoadStart, *Ramfuncs2LoadEnd, *Ramfuncs2RunStart;
int16	VTimer0[4];			// Virtual Timers slaved off CPU Timer 0 (A events)
int16	VTimer1[4]; 		// Virtual Timers slaved off CPU Timer 1 (B events)
int16	VTimer2[4]; 		// Virtual Timers slaved off CPU Timer 2 (C events)



//---------------PWM Handle Declaration--------------------------------------------
PWMGEN PWM = PWMGEN_DEFAULTS;
PWMGEN *pwm1=&PWM;


// ------------- Solar Sine Analyzer Block to measure RMS, frequency and ZCD ---------
SINEANALYZER_DIFF_wPWR_F sine_mainsV_R;
SINEANALYZER_DIFF_wPWR_F sine_mainsV_S;
SINEANALYZER_DIFF_wPWR_F sine_mainsV_T;


SINEANALYZER_DIFF_wPWR_F sine_mainsV_Rout;
SINEANALYZER_DIFF_wPWR_F sine_mainsV_Sout;
SINEANALYZER_DIFF_wPWR_F sine_mainsV_Tout;




//-------------------------------PLL Handle Declaration--------------------------------
SPLL_1ph_SOGI_F spll1; 
SPLL_1ph_SOGI_F spll2; 

//--------------------------------DQ Handle Declaration----------------------------------
//ABC_DQ0_POS_F abc_dq0_pos1;
//ABC_DQ0_POS_F abc_dq0_pos2;


//--------------------------------PI Handle Declaration----------------------------------
CNTL_PI_F cntl_pi1;     //I_R Outer Controleer
CNTL_PI_F cntl_pi2; 	//I_S Outer Controleerr
CNTL_PI_F cntl_pi3; 	//I_T Outer Controleer
CNTL_PI_F cntl_pi4; 	//I_R inner controller
CNTL_PI_F cntl_pi5; 	//I_S inner controller
CNTL_PI_F cntl_pi6; 	//I_T inner controller

//--------------------------------PR Handle Declaration----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h1;
CNTL_2P2Z_F_VARS  Vars1_h1;

CNTL_2P2Z_F_COEFFS Coeff2_h1;
CNTL_2P2Z_F_VARS  Vars2_h1;

CNTL_2P2Z_F_COEFFS Coeff3_h1;
CNTL_2P2Z_F_VARS  Vars3_h1;

//--------------------------------PR Handle Declaration h3----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h3;
CNTL_2P2Z_F_VARS  Vars1_h3;

CNTL_2P2Z_F_COEFFS Coeff2_h3;
CNTL_2P2Z_F_VARS  Vars2_h3;

CNTL_2P2Z_F_COEFFS Coeff3_h3;
CNTL_2P2Z_F_VARS  Vars3_h3;

//--------------------------------PR Handle Declaration h5----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h5;
CNTL_2P2Z_F_VARS  Vars1_h5;

CNTL_2P2Z_F_COEFFS Coeff2_h5;
CNTL_2P2Z_F_VARS  Vars2_h5;

CNTL_2P2Z_F_COEFFS Coeff3_h5;
CNTL_2P2Z_F_VARS  Vars3_h5;

//--------------------------------PR Handle Declaration h5----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h7;
CNTL_2P2Z_F_VARS  Vars1_h7;

CNTL_2P2Z_F_COEFFS Coeff2_h7;
CNTL_2P2Z_F_VARS  Vars2_h7;

CNTL_2P2Z_F_COEFFS Coeff3_h7;
CNTL_2P2Z_F_VARS  Vars3_h7;

//--------------------------------PR Handle Declaration h5----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h9;
CNTL_2P2Z_F_VARS  Vars1_h9;

CNTL_2P2Z_F_COEFFS Coeff2_h9;
CNTL_2P2Z_F_VARS  Vars2_h9;

CNTL_2P2Z_F_COEFFS Coeff3_h9;
CNTL_2P2Z_F_VARS  Vars3_h9;

//--------------------------------PR Handle Declaration h5----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h11;
CNTL_2P2Z_F_VARS  Vars1_h11;

CNTL_2P2Z_F_COEFFS Coeff2_h11;
CNTL_2P2Z_F_VARS  Vars2_h11;

CNTL_2P2Z_F_COEFFS Coeff3_h11;
CNTL_2P2Z_F_VARS  Vars3_h11;

//---------------------- instance of Signal generator module------------------------------    
//SGENT_1 sgen = SGENT_1_DEFAULTS;


//---------------------- instance of Inverse Park module------------------------------    
//iPARK_F ipark1;

//---------------------- instance of Inverse Clarke module------------------------------ 
//iCLARKE_F iclarke1;


//------------------------------------------Space Vector Modul Initialization---------------------------------
//SVGEN_F S={0,0,0,0,0};		


//----------------------------4 Channel Data Logger Declaration----------------------------
//DLOG_4CH_F dlog_4ch1; 
//float32 DBUFF_4CH1[240]; 
//float32 DBUFF_4CH2[240]; 
//float32 DBUFF_4CH3[240]; 
//float32 DBUFF_4CH4[240]; 
//float32 dval1; 
//float32 dval2; 
//float32 dval3; 
//float32 dval4; 


//-------------------------------Global Parameters Declaration----------------------------

extern struct UPS_V UPS;

float32 DutyR=0;
float32 DutyS=0;
float32 DutyT=0;
float32 Duty_STS=-0.8;


Uint16 Flag_Start=0,Counter_Start=0,Flag_Sync=2,Flag_ShutDown=0,Counter_Emergency_Shutdown=0,nsamples=0,Counter_SyncSlowly=0,Flag_PowerOk=0, Counter_Sync_OK=0;
Uint16 ChargerReady_Prev=0,ChargerReady=0,Shutdown_Counter=0,Emergency_Shutdown_Prev=0,Counter_Shutdown=0,SystemIsHot=0, Emergency_Flag = 0;
Uint16 PowerGood=0, PowerBad=0, NormalPower_Counter=0, TurnOnBypass=0,Flag_ZCD=0,Counter_Sequence_OK=0, GPIO58_prev=0,Flag_Manual_STS = 0, Flag_Manual_Bypass = 0, Flag_Auto_STS = 0;
Uint16 Counter_IDF=0, Counter_ChargerFault=0, Counter_OverVoltage=0, Counter_ShortCircuitR=0, Counter_ShortCircuitS=0, Counter_ShortCircuitT=0, Counter_BypassSTSFault=0, Counter_InvSTSFault=0, Counter_BuckFault=0;
Uint16 Flag_ShortCircuitR=0, Flag_ShortCircuitS = 0 , Flag_ShortCircuitT = 0, Counter_ShortCircuitExistR = 0, Counter_ShortCircuitExistS = 0, Counter_ShortCircuitExistT = 0;
Uint16 Counter_ShortCircuit_SoftStartR=0, Counter_ShortCircuit_SoftStartS=0, Counter_ShortCircuit_SoftStartT=0,shortcircuit_limit_nsamplesR=2,shortcircuit_limit_nsamplesS=2,shortcircuit_limit_nsamplesT=2;
int32 OverLoadIndex=0, OverLoadIndex1=0, OverLoadIndex2=0; //use for signal generation module

float32 V_R, V_S, V_T, I_R, I_S, I_T,V_Rout,V_Sout,V_Tout,I_Rout,I_Sout,I_Tout,V_RCha;
float32 Vrms_R, Vrms_S, Vrms_T,Irms_R=0,Irms_S=0,Irms_T=0,Vrms_Rout,Vrms_Sout,Vrms_Tout,Irms_Rout,Irms_Sout,Irms_Tout;
float32 V_R_Start=0,V_S_Start=0,V_T_Start=0,mf_Start=0.0, mf_StartR=0.0, mf_StartS=0.0, mf_StartT=0.0;
float32 ipowerR=0.0, ipowerS=0.0, ipowerT=0.0;
float32 OLC[100],OverCurrentAccumulator=0.0,ChargerFrequency=0.0;
//Uint16  in[40] = {0}, FlagTest=0;
Uint16  inverter_voltage1 = 0, Counter_AfterReset = 0, Flag_Inverter_ClearTrip = 0, Emergency_Flag1 = 0, Counter_Detect_OnCommand = 0, inverter_frequency1 = 0;
float32 inverter_voltage =0.0, Vref_Sin = 0.0,Max_Voltage =0.0,Min_Voltage = 0.0, V_DC_BAT = 0.0, I_DC_BAT = 0.0;

Uint16 offset_avg_counter = 0,Flag_StratUp = 0,Counter_State1_AutoReset=0, State_AutoReset=0;
int16 offset= 0,offset_sum = 0,offset_avg=0;
//Uint16 in[25];
//float32 s[25];
//--------------------------------------------------------------------------------------------
/* MODBUS def */
void ModbusInputRegUpdate(void);
void ModbusHoldingRegUpdate(void);
void ModbusRwDataUpdate(void);
#define COIL_QTY			8
#define COIL_START_ADD	0x0001
Uint16 Coil[COIL_QTY + 1];

#define HOLDING_REG_QTY			39
#define HOLDING_REG_START_ADD	0x0801
Uint16 HoldingRegister[HOLDING_REG_QTY + 1];

#define INPUT_REG_QTY			39
#define INPUT_REG_START_ADD	0x0001
Uint16 InputRegister[INPUT_REG_QTY + 1];

#define OUTPUT_REG_QTY			0x0001
Uint16 OutputRegister[OUTPUT_REG_QTY + 1];
/* MODBUS def END */

void main(void)
{


	InitSysCtrl(); 
// Device Life support & GPIO	
// Only used if running from FLASH
// Note that the variable FLASH is defined by the compiler



#ifdef FLASH 
// Copy time critical code and Flash setup code to RAM
// The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
// symbols are created by the linker. Refer to the linker files. 
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	MemCopy(&Ramfuncs2LoadStart, &Ramfuncs2LoadEnd, &Ramfuncs2RunStart);
// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
	InitFlash();	// Call the flash wrapper init function
#endif //(FLASH)

// Timing sync for background loops 
// Timer period definitions found in device specific PeripheralHeaderIncludes.h
	//CpuTimer0Regs.PRD.all =  mSec1;		// A tasks
	CpuTimer1Regs.PRD.all =  mSec5;		// B tasks
	CpuTimer2Regs.PRD.all =  mSec50;	// C tasks

// Tasks State-machine init
	Alpha_State_Ptr = &B0;
//	A_Task_Ptr = &A1;
	B_Task_Ptr = &B1;
	C_Task_Ptr = &C1;
	

//----------------------Define PWM Frequency--------------------------------------------------------
	pwm1->PeriodMax = PWM_MAX ;
	PWM_INIT(pwm1);



	Reset_System();
	GpioDataRegs.GPASET.bit.GPIO25 = 1;		// uncomment if --> Set High initially
	UPS.Inverter.Status.BypassOnLoad=0;

	Flag_StratUp = 1;

	/* MODBUS initial */
	modbus_rtu_init(
			HoldingRegister, HOLDING_REG_QTY, HOLDING_REG_START_ADD,
			InputRegister, INPUT_REG_QTY, INPUT_REG_START_ADD,
			Coil, COIL_QTY, COIL_START_ADD);
	/* MODBUS initial END */

	UPS.Inverter.Status.ShutDown = 1;
   
//   GpioDataRegs.GPASET.bit.GPIO16 = 1;
//   asm("   RPT #200 || NOP"); 
//   GpioDataRegs.GPACLEAR.bit.GPIO16 = 1; 
   
	/* MODBUS interrupt enable */
	/* User specific code, enable interrupts: */
	/* Enable CPU int9 which is connected to SciA */
	/* Enable CPU int1 which is connected to CPU-Timer 0 */
	IER |= MODBUS_CPU_INTERRUPTS;	// Enable CPU INT
	/* MODBUS interrupt enable END */
	
	
// Reassign ISRs. 
	EALLOW;	// This is needed to write to EALLOW protected registers
	PieVectTable.EPWM1_INT = &MainISR;
	EDIS;
	
	

	
	ADC_Setup();

// Enable PIE group 3 interrupt 1 for EPWM1_INT
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;

// Enable CNT_zero interrupt using EPWM1 Time-base
    EPwm1Regs.ETSEL.bit.INTEN = 1;   // Enable EPWM1INT generation 
    EPwm1Regs.ETSEL.bit.INTSEL = 2;  // Enable interrupt CNT_PRD event
    EPwm1Regs.ETPS.bit.INTPRD = 1;   // Generate interrupt on the 1st event
	EPwm1Regs.ETCLR.bit.INT = 1;     // Enable more interrupts

// Enable CPU INT3 for EPWM1_INT:
	IER |= M_INT3;
	asm(" NOP");
// Enable global Interrupts and higher priority real-time debug events:
	EINT;   // Enable Global interrupt INTM
	ERTM;	// Enable Global realtime interrupt DBGM




      
// Enable the watchdog
//   EALLOW;
//   SysCtrlRegs.WDCR = 0x0028;   
 //  EDIS;

// Reset the watchdog counter
//   ServiceDog();
   
   	
	// IDLE loop. Just sit and loop forever:	
	for(;;)  //infinite loop
	{
		/* MODBUS process (in program main loop) */
		//ModbusInputRegUpdate();
		
		
		
		// State machine entry & exit point
		//===========================================================
		(*Alpha_State_Ptr)();	// jump to an Alpha state (A0,B0,...)
		//===========================================================
		
	}
	
	
}



void B0(void)
{
	// loop rate synchronizer for B-tasks
	if(CpuTimer1Regs.TCR.bit.TIF == 1)
	{
		CpuTimer1Regs.TCR.bit.TIF = 1;				// clear flag

		//-----------------------------------------------------------
		(*B_Task_Ptr)();		// jump to a B Task (B1,B2,B3,...)
		//-----------------------------------------------------------
		VTimer1[0]++;			// virtual timer 1, instance 0 (spare)
		
	}
	/* MODBUS process (in program main loop) */
	modbusFn = ModbusHandler();
 		if (modbusFn)
 		{
 			switch (modbusFn)
 			{
 				case 0x06:
					ModbusHoldingRegUpdate();
 					break;
 				case 0x05:
 					modbusAdd = ModbusEntryCoilAddress();
 					switch (modbusAdd)
 					{
 						case 0x0001: /* ups on load */
 							if (Coil[modbusAdd] == 0xFF00) {
 								//FlagTest=1;	
 								Flag_Manual_STS = 1;
 								//Turn_on_STS();
 							}
 							break;
 						case 0x0002: /* byp on load */
 							if (Coil[modbusAdd] == 0xFF00){ 
 							//FlagTest=2;
 								Flag_Manual_Bypass = 1;
 							//Turn_on_bypass();
 							}
 							break;	
 					}
 					break;
 				case 0x10:
 					break;
 				default:
 					break;	
 			}
 		}
 		/* MODBUS process END */

	Alpha_State_Ptr = &C0;		// Allow C state tasks
}

void C0(void)
{
	// loop rate synchronizer for C-tasks
	if(CpuTimer2Regs.TCR.bit.TIF == 1)
	{
		CpuTimer2Regs.TCR.bit.TIF = 1;				// clear flag

		//-----------------------------------------------------------
		(*C_Task_Ptr)();		// jump to a C Task (C1,C2,C3,...)
		//-----------------------------------------------------------
		VTimer2[0]++;			//virtual timer 2, instance 0 (spare)
		
	}

	Alpha_State_Ptr = &B0;	// Back to State B0
}





//=================================================================================
//	B - TASKS (executed in every 5 msec)
//=================================================================================

//----------------------------------- USER ----------------------------------------

//----------------------------------------
void B1(void) // Toggle GPIO-00
//----------------------------------------
{

	//-----------------
	//the next time CpuTimer1 'counter' reaches Period value go to B2
	B_Task_Ptr = &B2;	
	//-----------------
}

//----------------------------------------
void B2(void) //  SPARE
//----------------------------------------
{

	//-----------------
	//the next time CpuTimer1 'counter' reaches Period value go to B3
	B_Task_Ptr = &B3;
	//-----------------
}

//----------------------------------------
void B3(void) //  SPARE
//----------------------------------------
{

	//-----------------
	//the next time CpuTimer1 'counter' reaches Period value go to B1
	B_Task_Ptr = &B1;	
	//-----------------
}


//=================================================================================
//	C - TASKS (executed in every 50 msec)
//=================================================================================

//--------------------------------- USER ------------------------------------------

//----------------------------------------
void C1(void) 	// Toggle GPIO-34 
//----------------------------------------
{

		//if(UPS.Inverter.Status.OffCommand == 1){
			//GpioDataRegs.GPBTOGGLE.bit.GPIO58 = 1;
			//Shutdown_Counter++;
		//}

			
			
	
	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C2
	C_Task_Ptr = &C2;	
	//-----------------

}

//----------------------------------------
void C2(void) //  SPARE
//----------------------------------------
{
	//---------------------------------------------Auto Reset---------------------------------

	if(State_AutoReset == 1 && UPS.Inverter.Status.OverLoadExist == 0 && Counter_State1_AutoReset < 400)
		Counter_State1_AutoReset++;
	else
		Counter_State1_AutoReset = 0;


	 if(Counter_State1_AutoReset == 400){
		 GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1;
	 }

	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C3
	C_Task_Ptr = &C3;	
	//-----------------
}


//-----------------------------------------
void C3(void) //  SPARE
//-----------------------------------------
{



	if(UPS.Inverter.Status.InverterOnLoad == 1 || UPS.Inverter.Status.BypassOnLoad == 1){
		if(NormalPower_Counter > 2)
		{		
			UPS.Inverter.Status.OverLoadExist=0;
			OverCurrentAccumulator=0;
			NormalPower_Counter=0;
		}
					
		else 
		{
			UPS.Inverter.Status.OverLoadExist=1;
			OverLoadIndex1=(int)(100 *( maximum(sine_mainsV_Rout.Pavg ,sine_mainsV_Sout.Pavg ,sine_mainsV_Tout.Pavg )/Nominal_Active_Power))-100;
			OverLoadIndex2=(int)(100 *( maximum(sine_mainsV_Rout.Papp ,sine_mainsV_Sout.Papp ,sine_mainsV_Tout.Papp )/Nominal_Apparent_Power))-100;
			if(OverLoadIndex1 > OverLoadIndex2)
				OverLoadIndex = OverLoadIndex1;
			else
				OverLoadIndex = OverLoadIndex2;
		}
		
		if(OverLoadIndex > 99)
			OverLoadIndex=99;

		if(OverLoadIndex < 0)
			OverLoadIndex=0;
			
		if(UPS.Inverter.Status.OverLoadExist == 1 && UPS.Inverter.Status.InverterOnLoad == 1){
			OverCurrentAccumulator=OverCurrentAccumulator+OLC[OverLoadIndex];
		}
		
		if(OverCurrentAccumulator > 1000.0 && UPS.Inverter.Status.InverterOnLoad == 1){
			Flag_ShutDown=1;
			UPS.Inverter.Status.OverLoad = 1;
			UPS.Inverter.Status.OverLoadExist =0;
			OverCurrentAccumulator = 0;
		}
	}	
	
	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C1
	C_Task_Ptr = &C1;	
	//-----------------
}
//Inverter    charger
//GPIO57 ---> GPIO57 Command From Charger To Turn on Inverter
//GPIO56 ---> GPIO56 Command From Charger to Turn off Inverter
//GPIO54 ---> GPIO54 Say to Charger Battery Contactor is Connected
//GPIO55 ---> GPIO55 Command From Charger To Emergency Shutsown Inverter
//GPIO22 ---> GPIO58 Command From Inverter to Turn off Charger
//GPIO23 ---> GPIO59 Charger Says to Inverter that Bypass is in Range
//GPIO58 ---> GPIO23 Charger Says to Inverter that Battery is Reverse
//GPIO59 ---> GPIO24 Charger Says to Inverter that open the battery Contactor


//Inverter    charger
//GPIO57 ---> GPIO57 Charger Says to Inverter that Start Turn on Inverter*
//GPIO56 ---> GPIO56 Charger Says to Inverter that Turn off Inverter*
//GPIO54 ---> GPIO54 Inverter Say to Charger Turn off charger*
//GPIO55 ---> GPIO55 Inverter says to charger that Inverter reset is complete*
//GPIO22 ---> GPIO58 Inverter says to charger that Turn off Charger
//GPIO23 ---> GPIO59 Charger Says to Inverter that Bypass is in Range*
//GPIO58 ---> GPIO23 Charger Says to Inverter that Turn on Inverter*
//GPIO59 ---> GPIO24 Charger Says to Inverter that open the battery Contactor

interrupt void MainISR(void)
{ 		
	
	if(Flag_Inverter_ClearTrip == 1){
		Turn_on_system();
		Flag_Inverter_ClearTrip = 0;
	}
	
	if(SystemIsHot == 0 && GpioDataRegs.GPBDAT.bit.GPIO58 == 0 && GPIO58_prev == 1 && Counter_AfterReset == 0 && Counter_Detect_OnCommand < 3 && UPS.Inverter.Status.ShutDown == 1){
				
		Counter_Detect_OnCommand++;     
	}
	GPIO58_prev = GpioDataRegs.GPBDAT.bit.GPIO58;
	
//	if(Counter_Detect_OnCommand == 3){
//		Counter_Detect_OnCommand++;
//		Counter_AfterReset++; 
//	}

	if(Counter_Detect_OnCommand == 3){
		Reset_System();
		Counter_Detect_OnCommand = 4;
		Counter_AfterReset = 1; 
	}
			
	
	if(SystemIsHot == 0 && Counter_AfterReset == 1 && GpioDataRegs.GPBDAT.bit.GPIO57 == 0){
		GpioDataRegs.GPBCLEAR.bit.GPIO55 = 1;
		//clear WD flag
// 	    EALLOW;
//        	SysCtrlRegs.WDCR = 0x00E8;   
//        EDIS;	
        
        SystemIsHot = 1;
        
        Counter_AfterReset++; 
	}	
	
	
	if(GpioDataRegs.GPBDAT.bit.GPIO57 == 1 && TurnOnBypass < 10 && Counter_AfterReset == 2)
		TurnOnBypass++;	
	else
		TurnOnBypass=0;
		
	if(TurnOnBypass == 10 && UPS.Inverter.Status.BypassOnStart == 0 && Counter_AfterReset == 2 ){
		Turn_on_bypass();
		UPS.Inverter.Status.BypassOnStart=1;
		GpioDataRegs.GPBSET.bit.GPIO55 = 1;
		TurnOnBypass++;
		Counter_AfterReset++;	
	}		
	
	
	if(GpioDataRegs.GPBDAT.bit.GPIO57 == 0 && ChargerReady < 10 && Counter_AfterReset == 3 ){
		ChargerReady++;	
	} 
	
	if(ChargerReady == 10 && UPS.Inverter.Status.Charger_Ready == 0 && Counter_AfterReset == 3){
		UPS.Inverter.Status.Charger_Ready=1;
		GpioDataRegs.GPBCLEAR.bit.GPIO55 = 1;
		ChargerReady++;
		Counter_AfterReset++;
	}
			
	if(UPS.Inverter.Status.ComFault == 0 && UPS.Inverter.Status.Charger_Ready == 1  && Counter_AfterReset == 4 && GpioDataRegs.GPBDAT.bit.GPIO57 == 1 ){    
		GpioDataRegs.GPBSET.bit.GPIO55 = 1;
		Flag_Inverter_ClearTrip = 1;
		Counter_AfterReset++;
		Flag_Start=1;
	}	

	if(UPS.Inverter.Status.PG == 1 && UPS.Inverter.Status.InverterOnLoad == 0 && UPS.Inverter.Status.ON == 1 && UPS.Inverter.Status.Sync == 1 && UPS.Inverter.Status.SequenceOK == 1 && Flag_ShutDown == 0 && Flag_Auto_STS == 0 && Counter_AfterReset == 5  && Counter_SyncSlowly == 60000){
    	Turn_on_STS(); 
    	Flag_Auto_STS = 1;	
    	Counter_AfterReset++;
    } 



	if(Flag_Manual_Bypass == 1){
		if(UPS.Inverter.Status.BypassFail == 0 && UPS.Inverter.Status.BypassOnLoad == 0 && UPS.Inverter.Status.InverterOnLoad == 1 && UPS.Inverter.Status.ON == 1 && UPS.Inverter.Status.Sync == 1  && Flag_ShutDown == 0){
			Turn_on_bypass();
			Flag_Manual_Bypass = 0;
		}
		else
			Flag_Manual_Bypass = 0;
	}


    if(Flag_Manual_STS == 1){
		if(UPS.Inverter.Status.PG == 1 && UPS.Inverter.Status.InverterOnLoad == 0 && UPS.Inverter.Status.BypassOnLoad == 1 && UPS.Inverter.Status.ON == 1 && UPS.Inverter.Status.Sync == 1 && UPS.Inverter.Status.SequenceOK == 1 && Flag_ShutDown == 0){
			Turn_on_STS();
			Flag_Manual_STS = 0;
		}
		else
			Flag_Manual_STS = 0;
    }
    
//---------------------------Emergency ShutDown-------------------------	
	if(GpioDataRegs.GPCDAT.bit.GPIO72 == 0)
		UPS.Inverter.Status.EmergencyShutDown = 1;
	else
		UPS.Inverter.Status.EmergencyShutDown = 0;	
		
	if(Emergency_Flag < 10){
		if(UPS.Inverter.Status.EmergencyShutDown)
			Emergency_Flag++;
		else
			Emergency_Flag=0;	
	}  
	if(Emergency_Flag == 10){
		Flag_ShutDown=1;
		Emergency_Flag++;
	}  		
			
		
		
	if(SystemIsHot == 1){
			
		//IDFR                                   IDFS						             IDFT
		//if (GpioDataRegs.GPADAT.bit.GPIO12 == 0 || GpioDataRegs.GPADAT.bit.GPIO13 == 0 || GpioDataRegs.GPADAT.bit.GPIO14 == 0)
		if(EPwm1Regs.TZFLG.bit.OST == 1 && UPS.Inverter.Status.ON == 1 && Counter_ShortCircuitR > 0 )
			UPS.Inverter.Status.IDF = 1;
		else
			UPS.Inverter.Status.IDF = 0;
			
				
//---------------------- Charger fault detection ------------------	
		if(GpioDataRegs.GPBDAT.bit.GPIO56 == 0 || GpioDataRegs.GPADAT.bit.GPIO22 == 0) // Gpio56 indicates fault in charger
			UPS.Inverter.Status.Charger_Fault = 1;
		else 
			UPS.Inverter.Status.Charger_Fault = 0;
							
//=====================================================================	
		if(GpioDataRegs.GPADAT.bit.GPIO23 == 1)	
			UPS.Inverter.Status.BypassFail = 1;
		else
			UPS.Inverter.Status.BypassFail = 0;
							
		if((V_R > RST_Inverter_OverVoltage) || (V_S > RST_Inverter_OverVoltage) || (V_T > RST_Inverter_OverVoltage) || (V_R < -RST_Inverter_OverVoltage) || (V_S < -RST_Inverter_OverVoltage) || (V_T < -RST_Inverter_OverVoltage))
			UPS.Inverter.Status.AC_OverVoltage=1;
		else 
			UPS.Inverter.Status.AC_OverVoltage=0;	
			
		//--------------------------------------------STS Inverter Fault Detection-------------------------------------------    
    if(UPS.Inverter.Status.InverterOnLoad == 1){
 		if(fabs(V_R - V_Rout) > BYPASS_FAULT_THRESHOLD || fabs(V_S - V_Sout) > BYPASS_FAULT_THRESHOLD || fabs(V_T - V_Tout) > BYPASS_FAULT_THRESHOLD)
 			UPS.Inverter.Status.InvSTSFault = 1;
 		else
 			UPS.Inverter.Status.InvSTSFault = 0;
 	
 		if(UPS.Inverter.Status.InvSTSFault == 1)
 			Counter_InvSTSFault++;
 		else
 			Counter_InvSTSFault=0;			
    }
    
//----------------------------------------STS Bypass Fault Detection---------------------------------------------------	
	    if(sine_mainsV_R.nsamples == 0 && UPS.Inverter.Status.BypassOnLoad == 1){
	    	if(Vrms_Rout < Min_Voltage || Vrms_Sout < Min_Voltage || Vrms_Tout < Min_Voltage || Vrms_Rout > Max_Voltage || Vrms_Sout > Max_Voltage || Vrms_Tout > Max_Voltage)
	    		UPS.Inverter.Status.BypassSTSFault = 1;
	    	else
	    		UPS.Inverter.Status.BypassSTSFault = 0;
	    		
	    	if(UPS.Inverter.Status.BypassSTSFault == 1)
	 			Counter_BypassSTSFault++;
	 		else
	 			Counter_BypassSTSFault=0;		  				
	    }    
    				
		if(Counter_OverVoltage < 3){	
			if(UPS.Inverter.Status.AC_OverVoltage) 
				Counter_OverVoltage++;			 
			else
				Counter_OverVoltage=0;
		}	
			
		if(Counter_ChargerFault < 3){		
			if(UPS.Inverter.Status.Charger_Fault) 
				Counter_ChargerFault++;			 
			else
				Counter_ChargerFault=0;
		}	
		
		if(Counter_IDF < 1){		
			if(UPS.Inverter.Status.IDF) 
				Counter_IDF++;			 
			else
				Counter_IDF=0;
		}			    								
//-------------------------------------------Protection Time Determination----------------------------------------	
	
		if(Counter_ChargerFault == 3){
			Flag_ShutDown=1;
			Counter_ChargerFault++;
		}
		if(Counter_OverVoltage == 3){
			Flag_ShutDown=1;
			Counter_OverVoltage++;
		}
		
		if(Counter_IDF == 1){
			Flag_ShutDown=1;
			Counter_IDF++;
		}
		
	    if(UPS.Inverter.Status.PG == 0 && UPS.Inverter.Status.InverterOnLoad == 1)  	
	    	Flag_ShutDown=1;
	    	
	    if(Counter_InvSTSFault == 100 && UPS.Inverter.Status.InverterOnLoad == 1){  	
	    	Flag_ShutDown=1;
	    	Counter_InvSTSFault++; 
	 	}	
	    
	 	if(Counter_BypassSTSFault == 100 && UPS.Inverter.Status.BypassOnLoad == 1){
	    	Flag_ShutDown=1;
	    	Counter_BypassSTSFault++; 
	 	}
	}
	
 //---------------------------------------------Update Modbus------------------------------    
 	ModbusInputRegUpdate();
//-----------------------Reading ADC Chanels----------------------------------
 	offset=AdcMirror.ADCRESULT13-2048;
	V_R=(AdcMirror.ADCRESULT0*0.00024414-0.5)*2*GAIN_CAL_VAC;
	V_S=(AdcMirror.ADCRESULT1*0.00024414-0.5)*2*GAIN_CAL_VAC;
	V_T=(AdcMirror.ADCRESULT2*0.00024414-0.5)*2*GAIN_CAL_VAC;

	I_R=(AdcMirror.ADCRESULT3*0.00024414-0.5)*2*GAIN_CAL_IAC;
	I_S=(AdcMirror.ADCRESULT4*0.00024414-0.5)*2*GAIN_CAL_IAC;
	I_T=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_IAC;

	V_Rout=(AdcMirror.ADCRESULT6*0.00024414-0.5)*2*GAIN_CAL_VAC;
	V_Sout=(AdcMirror.ADCRESULT7*0.00024414-0.5)*2*GAIN_CAL_VAC;
	V_Tout=(AdcMirror.ADCRESULT8*0.00024414-0.5)*2*GAIN_CAL_VAC;

	I_Rout=(AdcMirror.ADCRESULT9*0.00024414-0.5)*2*GAIN_CAL_IAC;
	I_Sout=(AdcMirror.ADCRESULT10*0.00024414-0.5)*2*GAIN_CAL_IAC;
	I_Tout=(AdcMirror.ADCRESULT11*0.00024414-0.5)*2*GAIN_CAL_IAC;

	V_RCha=(AdcMirror.ADCRESULT12*0.00024414-0.5)*2*GAIN_CAL_VAC;

	V_DC_BAT=(AdcMirror.ADCRESULT14*0.00024414-0.5)*2*GAIN_CAL_VDC*1.468;
	I_DC_BAT=(AdcMirror.ADCRESULT10*0.00024414-0.5)*2*GAIN_CAL_IAC;


    if(offset_avg_counter <= 100 && Flag_StratUp == 1){
		if(offset < 200 && offset > -200 ){
			offset_avg_counter++;
			if(offset_avg_counter > 100){
			   offset_avg=(offset_sum/100);
			   AdcRegs.ADCOFFTRIM.bit.OFFSET_TRIM -= offset_avg;
			}
			else{
				offset_sum = offset_sum + offset;
			}
		}
    }

    	
//---------------------------------Short Circuit Protection------------------------    
    if(UPS.Inverter.Status.Charger_Ready && Flag_ShutDown == 0){
    	if(I_R > RST_Inverter_ShortCircuit || I_R < -RST_Inverter_ShortCircuit)
				UPS.Inverter.Status.ShortCircuitR=1;
		else
				UPS.Inverter.Status.ShortCircuitR=0;
				
		if(I_S > RST_Inverter_ShortCircuit || I_S < -RST_Inverter_ShortCircuit)
				UPS.Inverter.Status.ShortCircuitS=1;
		else
				UPS.Inverter.Status.ShortCircuitS=0;	
		
		if(I_T > RST_Inverter_ShortCircuit || I_T < -RST_Inverter_ShortCircuit)
				UPS.Inverter.Status.ShortCircuitT=1;
		else
				UPS.Inverter.Status.ShortCircuitT=0;
				
		if(Counter_ShortCircuitR < shortcircuit_limit_nsamplesR){
			if(UPS.Inverter.Status.ShortCircuitR) 
				Counter_ShortCircuitR++;			 
			else
				Counter_ShortCircuitR=0;	
		}
		
		if(Counter_ShortCircuitS < shortcircuit_limit_nsamplesS){
			if(UPS.Inverter.Status.ShortCircuitS) 
				Counter_ShortCircuitS++;			 
			else
				Counter_ShortCircuitS=0;	
		}
		
		if(Counter_ShortCircuitT < shortcircuit_limit_nsamplesT){
			if(UPS.Inverter.Status.ShortCircuitT) 
				Counter_ShortCircuitT++;			 
			else
				Counter_ShortCircuitT=0;	
		}		
					
    }
    
    if(Counter_ShortCircuitR == shortcircuit_limit_nsamplesR){
		Flag_ShutDown=1;
		Counter_ShortCircuitR++;
	}
	
	if(Counter_ShortCircuitS == shortcircuit_limit_nsamplesS){
		Flag_ShutDown=1;
		Counter_ShortCircuitS++;
	}
	
	if(Counter_ShortCircuitT == shortcircuit_limit_nsamplesT){
		Flag_ShutDown=1;
		Counter_ShortCircuitT++;
	}
    
    if(Counter_ShortCircuitR == 1 && Flag_ShortCircuitR == 0){
		Flag_ShortCircuitR = 1;
		EALLOW;
	    EPwm1Regs.TZFRC.bit.OST=1;
	    EDIS;
		mf_StartR=0.0;
	}
	
	if(Counter_ShortCircuitS == 1 && Flag_ShortCircuitS == 0){
		Flag_ShortCircuitS = 1;
		EALLOW;
	    EPwm2Regs.TZFRC.bit.OST=1;
	    EDIS;
		mf_StartS=0.0;
	}
		
	if(Counter_ShortCircuitT == 1 && Flag_ShortCircuitT == 0){
		Flag_ShortCircuitT = 1;
		EALLOW;
	    EPwm3Regs.TZFRC.bit.OST=1;
	    EDIS;
		mf_StartT=0.0;
	}
	
	
//------------------------------------sync Determination of inverter output with charger input----------------    
	if(sine_mainsV_R.nsamples == 0){
		if(UPS.Inverter.Status.BypassFail == 0 && UPS.Inverter.Status.PG == 1 && Flag_ShutDown == 0){// from GPIO59 says input voltage is in range 
			if((fabs(spll1.theta[0] - spll2.theta[0]) < MAX_PHASEDIFF) || (fabs(spll1.theta[0] - spll2.theta[0]) > (2.0*3.1415926 - MAX_PHASEDIFF))) 
			    Counter_Sync_OK++;
			
			else{
		         UPS.Inverter.Status.Sync = 0;
		         Counter_Sync_OK = 0;
			}
		}
	
	   	else{
	         UPS.Inverter.Status.Sync = 0;
	         Counter_Sync_OK = 0;
	   	}
	} 
//-------------------------------------------------sequence Fail Detection------------------------------------ 	
 	if(sine_mainsV_R.ZCD == 1 && Flag_ZCD == 0){
 		Flag_ZCD = 1;		
 	}
 	
 	if(sine_mainsV_S.ZCD == 1 && Flag_ZCD == 1){
 		Flag_ZCD = 0;
 		Counter_Sequence_OK++;
 				
 	}
 	
 	if(sine_mainsV_T.ZCD == 1 && Flag_ZCD == 1){
 		Flag_ZCD = 0;
 		Counter_Sequence_OK = 0;
 		UPS.Inverter.Status.SequenceOK = 0;		
 	}
 		
 	if(Counter_Sequence_OK > 10){
 		Counter_Sequence_OK = 10;
 		UPS.Inverter.Status.SequenceOK = 1;
 	} 	 

    if(Counter_Sync_OK > 10){
        Counter_Sync_OK = 10;
        UPS.Inverter.Status.Sync = 1;
    }

//--------------------RMS Module Implementation--------------------------------------				
    sine_mainsV_R.Vin=V_R*BASE_VOLTAGE;
    sine_mainsV_S.Vin=V_S*BASE_VOLTAGE;
    sine_mainsV_T.Vin=V_T*BASE_VOLTAGE;
    
//    sine_mainsV_R.Vref=spll1.sin;
//    sine_mainsV_S.Vref=spll1.sin;
//    sine_mainsV_T.Vref=spll1.sin;
    
    sine_mainsV_R.Iin=I_R*BASE_CURRENT;
    sine_mainsV_S.Iin=I_S*BASE_CURRENT;
    sine_mainsV_T.Iin=I_T*BASE_CURRENT;
    	
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_R);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_S);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_T);
    
	
    Vrms_R=sine_mainsV_R.Vrms;
    Vrms_S=sine_mainsV_S.Vrms;
    Vrms_T=sine_mainsV_T.Vrms;
    
    Irms_R=sine_mainsV_R.Irms;
    Irms_S=sine_mainsV_S.Irms;
    Irms_T=sine_mainsV_T.Irms;
//---------------------------------------------------------    
  
    sine_mainsV_Rout.Vin=V_Rout*BASE_VOLTAGE;
    sine_mainsV_Sout.Vin=V_Sout*BASE_VOLTAGE;
    sine_mainsV_Tout.Vin=V_Tout*BASE_VOLTAGE;
    
//    sine_mainsV_Rout.Vref=spll1.sin;
//    sine_mainsV_Sout.Vref=spll1.sin;
//    sine_mainsV_Tout.Vref=spll1.sin;
    
    sine_mainsV_Rout.Iin=I_Rout*BASE_CURRENT;
    sine_mainsV_Sout.Iin=I_Sout*BASE_CURRENT;
    sine_mainsV_Tout.Iin=I_Tout*BASE_CURRENT;
    	
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_Rout);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_Sout);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_Tout);
    
	
    Vrms_Rout=sine_mainsV_Rout.Vrms;
    Vrms_Sout=sine_mainsV_Sout.Vrms;
    Vrms_Tout=sine_mainsV_Tout.Vrms;  

	
    Irms_Rout=sine_mainsV_Rout.Irms;
    Irms_Sout=sine_mainsV_Sout.Irms;
    Irms_Tout=sine_mainsV_Tout.Irms;   
    
 
 
//------------------------- Output Voltage out of range Detection----------------------------    	
    
    if(sine_mainsV_R.nsamples == 0 && Flag_Start == 2){
    	Flag_PowerOk = 0;
    	if( Vrms_R > Min_Voltage && Vrms_S > Min_Voltage && Vrms_T > Min_Voltage && Vrms_R < Max_Voltage && Vrms_S < Max_Voltage && Vrms_T < Max_Voltage){
    		if(sine_mainsV_R.SigFreq > Min_Freq && sine_mainsV_S.SigFreq > Min_Freq && sine_mainsV_T.SigFreq > Min_Freq){  
    			if(sine_mainsV_R.SigFreq < Max_Freq && sine_mainsV_S.SigFreq < Max_Freq && sine_mainsV_T.SigFreq < Max_Freq){
    					PowerGood++;
    					PowerBad=0;
    					Flag_PowerOk=1;
    			}
    		}
    	}
    	
    	if(Flag_PowerOk == 0)
    		PowerBad++;

    	if(PowerBad==PB_DETECTION_CYCLES){
    		PowerGood=0; 
    		UPS.Inverter.Status.PG=0;
    		PowerBad=0; 	
    	}	
    }

 
    if(PowerGood > PG_DETECTION_CYCLES){        
    	PowerGood=PG_DETECTION_CYCLES;
    	UPS.Inverter.Status.PG=1;
    }
//================================================================================================================    
    
inverter_voltage = (float32)(inverter_voltage1);
Vref_Sin = 0.004545*inverter_voltage;
Max_Voltage = 1.15*inverter_voltage;
Min_Voltage = 0.85*inverter_voltage;


//=====================================================================================================================

 	
 	if(sine_mainsV_R.nsamples == 0 && (UPS.Inverter.Status.InverterOnLoad == 1 || UPS.Inverter.Status.BypassOnLoad == 1)){
		if((sine_mainsV_Rout.Pavg < Nominal_Active_Power) && (sine_mainsV_Sout.Pavg  < Nominal_Active_Power) && (sine_mainsV_Tout.Pavg  < Nominal_Active_Power)
				&& (sine_mainsV_Rout.Papp < Nominal_Apparent_Power) && (sine_mainsV_Sout.Papp  < Nominal_Apparent_Power) && (sine_mainsV_Tout.Papp  < Nominal_Apparent_Power))
		{		
			NormalPower_Counter++;
		}
	}	
 	 
//=====================================================================================  
//-----------------------Signal Generator Module Implementation----------------------    	

 //   sgen.calc(&sgen); 
  	 
//==========================================================================   	 		
   	 		
//----------------------PLL Module Implementation--------------------------------------			
	if(UPS.Inverter.Status.BypassFail == 1){//from GPIO59 says input voltage is not in range
		V_RCha=0.0;
		spll1.Umax = 0.0;
		spll1.Umin = 0.0;
		Counter_SyncSlowly=0;
	}
	else if(UPS.Inverter.Status.BypassFail == 0 && Counter_SyncSlowly < 60000){
		spll1.Umax = 0.5;
		spll1.Umin = -0.5;
		Counter_SyncSlowly++;
	}
	
	else{
		spll1.Umax = (Max_Freq - GRID_FREQ);
		spll1.Umin = -(GRID_FREQ - Min_Freq);
	}
	
	
	spll1.u[0]=(float32)(V_RCha*Gain_pu); 
	SPLL_1ph_SOGI_F_FUNC(&spll1);
	
		
	spll2.u[0]=(float32)(V_R*Gain_pu); 
	SPLL_1ph_SOGI_F_FUNC(&spll2);
//===========================================================================

//-------------------------------------ShutDown Routine---------------------
	if(Flag_ShutDown == 1){	
		UPS.Inverter.Status.ComFault=1;
		
		Shut_down_system();

		if(UPS.Inverter.Status.BypassFail == 0 && UPS.Inverter.Status.Sync == 1 
		&& UPS.Inverter.Status.EmergencyShutDown == 0 && GpioDataRegs.GPADAT.bit.GPIO22 == 1 // Manual ShutDown Of UPS
		&& UPS.Inverter.Status.ShortCircuitR == 0 && UPS.Inverter.Status.ShortCircuitS == 0 && UPS.Inverter.Status.ShortCircuitT == 0){
			 Turn_on_bypass();
		}
		
		Flag_ShutDown++;		
	}   
//--------------------------4 Channel Data Logger Implementation----------------------
//    dval1= (V_R); 
//	dval2= (V_S); 
//	dval3= (V_T); 
//	dval4= (V_T_Start); 
//	DLOG_4CH_F_FUNC(&dlog_4ch1); 
	
//=====================================================================================	
//    if(Flag_Start == 0){
//    	PWM_Drive(pwm1,0,0,0,Duty_STS);
//    }
    
    
	if(Flag_Start != 0){
		if(Flag_Start == 1){
    		mf_Start+=Step_SoftStart;
		} 
    	
    	if(mf_Start > Vref_Sin && Flag_Start == 1){
    		mf_Start=Vref_Sin;
    		Flag_Start=2;
    	}
    	
    	
	if(Flag_Start == 2){
		if((Vref_Sin - mf_Start) >  2*0.0000416667)
			mf_Start+=0.0000416667;
			
		else if((Vref_Sin - mf_Start)  < -2*0.0000416667)
			mf_Start-=0.0000416667;
		
		else
			mf_Start = Vref_Sin;	
		}    	
    	   	
    		
   		V_R_Start=mf_Start*(spll1.sin);      
   	 	V_S_Start=mf_Start*(spll1.sin240);      
    	V_T_Start=mf_Start*(spll1.sin120);      
    		        	                                  
	    

//----------------------------------------------------ShortCircuit Detection-----------------------------------------------------	    	   	
	    if(Flag_ShortCircuitR == 1 && Counter_ShortCircuitExistR == 0){
	    	if(I_R > 0.5*RST_Inverter_ShortCircuit || I_R < -0.5*RST_Inverter_ShortCircuit){
	    		V_R_Start = 0.0;
	    	}
	    	else{
	    		V_R_Start = mf_StartR*spll1.sin;
	    	}	
	    	Counter_ShortCircuit_SoftStartR++;	
	    	
	    	if(Counter_ShortCircuit_SoftStartR < 10){
	    		EALLOW;
	    		EPwm1Regs.TZFRC.bit.OST=1;
	    		EDIS;
	    		Counter_ShortCircuitR=0;
	    	}	
	    	else if(Counter_ShortCircuit_SoftStartR >= 10 && Counter_ShortCircuit_SoftStartR < 15 ){
	    		EALLOW;
	    		EPwm1Regs.TZCLR.bit.OST = 1;
	    		EDIS; 
	    		Counter_ShortCircuitR=0;
	    	}
	    	else{
	    		mf_StartR+=0.001;
	    	}
	    							    		
	    }


	    if(mf_StartR < 0.25)
	    	shortcircuit_limit_nsamplesR = 2;

	    else if(mf_StartR >= 0.25 && mf_StartR < 0.5)
	    	shortcircuit_limit_nsamplesR = 3;

	    else if(mf_StartR >= 0.5 && mf_StartR < 0.75)
	    	shortcircuit_limit_nsamplesR = 4;

	    else
	    	shortcircuit_limit_nsamplesR = 5;


	    	
	 
		if(mf_StartR > Vref_Sin && Flag_ShortCircuitR == 1 && Counter_ShortCircuitExistR < 500){ 
	   		Counter_ShortCircuitExistR++;
	   }
	   
	   	if(Counter_ShortCircuitExistR == 500){
	   		Flag_ShortCircuitR = 0;
	   		Counter_ShortCircuitExistR = 0;
	   		Counter_ShortCircuit_SoftStartR=0;
	   		shortcircuit_limit_nsamplesR = 2;
	   }
//---------------------------------------------------------------	   	
	    if(Flag_ShortCircuitS == 1 && Counter_ShortCircuitExistS == 0){
	    	if(I_S > 0.5*RST_Inverter_ShortCircuit || I_S < -0.5*RST_Inverter_ShortCircuit){
	    		V_S_Start = 0.0;
	    	}
	    	else{
	    		V_S_Start = mf_StartS*spll1.sin240;
	    	}	
	    	Counter_ShortCircuit_SoftStartS++;	
	    	
	    	if(Counter_ShortCircuit_SoftStartS < 10){
	    		EALLOW;
	    		EPwm2Regs.TZFRC.bit.OST=1;
	    		EDIS;
	    		Counter_ShortCircuitS=0;
	    	}	
	    	else if(Counter_ShortCircuit_SoftStartS >=10 && Counter_ShortCircuit_SoftStartS < 15){
	    		EALLOW;
	    		EPwm2Regs.TZCLR.bit.OST = 1;
	    		EDIS; 
	    		Counter_ShortCircuitS=0;
	    	}
	    	else{
	    		mf_StartS+=0.001;
	    		}						    		
	    }


	    if(mf_StartS < 0.25)
	    	shortcircuit_limit_nsamplesS = 2;

	    else if(mf_StartS >= 0.25 && mf_StartS < 0.5)
	    	shortcircuit_limit_nsamplesS = 3;

	    else if(mf_StartS >= 0.5 && mf_StartS < 0.75)
	    	shortcircuit_limit_nsamplesS = 4;

	    else
	    	shortcircuit_limit_nsamplesS = 5;





	    	
	 
		if(mf_StartS > Vref_Sin && Flag_ShortCircuitS == 1 && Counter_ShortCircuitExistS < 500){ 
	   		Counter_ShortCircuitExistS++;
	   }
	   
	   	if(Counter_ShortCircuitExistS == 500){
	   		Flag_ShortCircuitS = 0;
	   		Counter_ShortCircuitExistS = 0;
	   		Counter_ShortCircuit_SoftStartS = 0;
	   		shortcircuit_limit_nsamplesS = 2;
	   }
	   	
//---------------------------------------------------------------	   	
	    if(Flag_ShortCircuitT == 1 && Counter_ShortCircuitExistT == 0){
	    	if(I_T > 0.5*RST_Inverter_ShortCircuit || I_T < -0.5*RST_Inverter_ShortCircuit){
	    		V_T_Start = 0.0;
	    	}
	    	else{
	    		V_T_Start = mf_StartT*spll1.sin120;
	    	}	
	    	Counter_ShortCircuit_SoftStartT++;
	    		    	
	    	if(Counter_ShortCircuit_SoftStartT < 10){
	    		EALLOW;
	    		EPwm3Regs.TZFRC.bit.OST = 1;
	    		EDIS;
	    		Counter_ShortCircuitT = 0;
	    	}
	    	
	    	else if(Counter_ShortCircuit_SoftStartT >= 10 && Counter_ShortCircuit_SoftStartT < 15){
	      		EALLOW;
	    		EPwm3Regs.TZCLR.bit.OST = 1;
	    		EDIS;
	    		Counter_ShortCircuitT=0;
	    	}	
	    	else{
	    		mf_StartT+=0.001; 
	    	}
	    							    		
	    }



	    if(mf_StartT < 0.25)
	    	shortcircuit_limit_nsamplesT = 2;

	    else if(mf_StartT >= 0.25 && mf_StartT < 0.5)
	    	shortcircuit_limit_nsamplesT = 3;

	    else if(mf_StartT >= 0.5 && mf_StartT < 0.75)
	    	shortcircuit_limit_nsamplesT = 4;

	    else
	    	shortcircuit_limit_nsamplesT = 5;
	    	
	 
		if(mf_StartT > Vref_Sin && Flag_ShortCircuitT == 1 && Counter_ShortCircuitExistT < 500){ 
	   		Counter_ShortCircuitExistT++;
	   }
	   
	   	if(Counter_ShortCircuitExistT == 500){
	   		Flag_ShortCircuitT = 0;
	   		Counter_ShortCircuitExistT = 0;
	   		Counter_ShortCircuit_SoftStartT = 0;
	   		shortcircuit_limit_nsamplesT = 2;
	   }
//---------------------PI Regulator for Phase a Voltage--------------------------------------    	
		cntl_pi1.Ref =V_R_Start; 
	   	cntl_pi1.Fbk =V_R; 
	    CNTL_PI_F_FUNC(&cntl_pi1);		

//---------------------PI Regulator for Phase b Voltage--------------------------------------  
		cntl_pi2.Ref =V_S_Start; 
	   	cntl_pi2.Fbk = V_S; 
	    CNTL_PI_F_FUNC(&cntl_pi2);		
    
    
//---------------------PI Regulator for Phase c Voltage--------------------------------------  
		cntl_pi3.Ref =V_T_Start;  
	   	cntl_pi3.Fbk = V_T; 
	    CNTL_PI_F_FUNC(&cntl_pi3);  
    
 //---------------------PR Regulator------------------------------------------  

		Vars1_h1.Ref = V_R_Start;
		Vars1_h1.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h1,&Vars1_h1);
	
		Vars2_h1.Ref = V_S_Start;
		Vars2_h1.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h1,&Vars2_h1);
	
		Vars3_h1.Ref = V_T_Start;
		Vars3_h1.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h1,&Vars3_h1);
//----------------------------------------------------------------------		
		Vars1_h3.Ref = V_R_Start;
		Vars1_h3.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h3,&Vars1_h3);
		
		Vars2_h3.Ref = V_S_Start;
		Vars2_h3.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h3,&Vars2_h3);
		
		Vars3_h3.Ref = V_T_Start;
		Vars3_h3.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h3,&Vars3_h3); 
//-------------------------------------------------------------------------------------		
		Vars1_h5.Ref = V_R_Start;
		Vars1_h5.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h5,&Vars1_h5);
		
		Vars2_h5.Ref = V_S_Start;
		Vars2_h5.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h5,&Vars2_h5);
		
		Vars3_h5.Ref = V_T_Start;
		Vars3_h5.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h5,&Vars3_h5);
//-------------------------------------------------------------------------------------		
		Vars1_h7.Ref = V_R_Start;
		Vars1_h7.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h7,&Vars1_h7);
		
		Vars2_h7.Ref = V_S_Start;
		Vars2_h7.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h7,&Vars2_h7);
		
		Vars3_h7.Ref = V_T_Start;
		Vars3_h7.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h7,&Vars3_h7);
//-------------------------------------------------------------------------------------		
		Vars1_h9.Ref = V_R_Start;
		Vars1_h9.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h9,&Vars1_h9);
		
		Vars2_h9.Ref = V_S_Start;
		Vars2_h9.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h9,&Vars2_h9);
		
		Vars3_h9.Ref = V_T_Start;
		Vars3_h9.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h9,&Vars3_h9);		
//-------------------------------------------------------------------------------------		
		Vars1_h11.Ref = V_R_Start;
		Vars1_h11.Fdbk = V_R;
		CNTL_2P2Z_F_FUNC( &Coeff1_h11,&Vars1_h11);
		
		Vars2_h11.Ref = V_S_Start;
		Vars2_h11.Fdbk = V_S;
		CNTL_2P2Z_F_FUNC( &Coeff2_h11,&Vars2_h11);
		
		Vars3_h11.Ref = V_T_Start;
		Vars3_h11.Fdbk = V_T;
		CNTL_2P2Z_F_FUNC( &Coeff3_h11,&Vars3_h11);			
	
//---------------------PI Regulator for Phase a Current--------------------------------------- 
		cntl_pi4.Ref =cntl_pi1.Out + 1000.0*(Vars1_h1.Out + K*0.5*Vars1_h3.Out + K*0.5*Vars1_h5.Out + K*0.2*Vars1_h7.Out + K*0.2*Vars1_h9.Out + K*0.2*Vars1_h11.Out); 
		//cntl_pi4.Ref =cntl_pi1.Out + 3800.0*Vars1_h1.Out;
	   	cntl_pi4.Fbk = I_R; 
	    CNTL_PI_F_FUNC(&cntl_pi4);
    
//---------------------PI Regulator for Phase b Current-------------------------------------- 
		cntl_pi5.Ref =cntl_pi2.Out + 1000.0*(Vars2_h1.Out + K*0.5*Vars2_h3.Out + K*0.5*Vars2_h5.Out + K*0.2*Vars2_h7.Out + K*0.2*Vars2_h9.Out + K*0.2*Vars2_h11.Out);  
		//cntl_pi5.Ref =cntl_pi2.Out + 3800.0*Vars2_h1.Out;
	   	cntl_pi5.Fbk = I_S; 
	    CNTL_PI_F_FUNC(&cntl_pi5);   
    
//---------------------PI Regulator for Phase c Current--------------------------------------
    	cntl_pi6.Ref =cntl_pi3.Out + 1000.0*(Vars3_h1.Out + K*0.5*Vars3_h3.Out + K*0.5*Vars3_h5.Out + K*0.2*Vars3_h7.Out + K*0.2*Vars3_h9.Out + K*0.2*Vars3_h11.Out);;
	    //cntl_pi6.Ref =cntl_pi3.Out + 3800.0*Vars3_h1.Out;
	   	cntl_pi6.Fbk = I_T; 
	    CNTL_PI_F_FUNC(&cntl_pi6); 
         

    
    
	
		
		DutyR=cntl_pi4.Out;
		DutyS=cntl_pi5.Out;
		DutyT=cntl_pi6.Out;
    
//---------------------Saturation Limits--------------------------------------  
	
		if(DutyR > 0.95)
			DutyR=0.95;
		if(DutyS > 0.95)
			DutyS=0.95;
		if(DutyT > 0.95)
			DutyT=0.95;	
		if(DutyR < -0.95)
			DutyR=-0.95;
		if(DutyS < -0.95)
			DutyS=-0.95;
		if(DutyT <- 0.95)
			DutyT=-0.95;	
		Duty_STS=-0.8;	//for duty cycle of 10%
		
//-----------------------PWM Module Implementation-----------------------------------------	         
		PWM_Drive(pwm1,DutyR,DutyS,DutyT,Duty_STS);	// Calculate the new PWM compare values	
		
//-----------------------------------------------------------------------------------------	
    	
	}
	

	
//	GpioDataRegs.GPACLEAR.bit.GPIO10 = 1; // Enable RST PWM	
//
//	EALLOW;
//	EPwm1Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM1
//	EPwm2Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM2
//	EPwm3Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM3
//	EDIS;
	


//Turn_on_bypass();
//	PWM_Drive(pwm1,0.0,0.0,0.5,Duty_STS);	


	EPwm1Regs.ETCLR.bit.INT = 1;	// Clear INT flag for this timer

		
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;	// Acknowledge this interrupt to receive more interrupts from group 3
 
}




float maximum(float a, float b, float c)
{
	if ((a>b)&&(a>c)) return a;
	else if ((b>a)&&(b>c)) return b;
	else return c;
}

void OverLoadCounterRef(void)
{
	int i;
	for (i=100;i<200;i++)
		{
			OLC[i-100]=150.0/(14183*exp(-0.0000016194*i*i*i));
		}
}

void Shut_down_system(void){   //shut down system because of Fault in inverter or charger
	//UPS.Inverter.Status.OffCommand = 1;
	GpioDataRegs.GPASET.bit.GPIO10 = 1; // Disable RST PWM
	GpioDataRegs.GPBSET.bit.GPIO49 = 1;   //DISABLE STS for inverter
 	GpioDataRegs.GPASET.bit.GPIO25 = 1; //Disable STS for Bypass
 	//GpioDataRegs.GPCCLEAR.bit.GPIO77 = 1;   //Turn off Battery Contactor K1
	EALLOW;
	EPwm1Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM1
	EPwm2Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM2
	EPwm3Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM3
	EPwm6Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM6
	EDIS;
	

	UPS.Inverter.Status.Charger_Ready=0;
	UPS.Inverter.Status.InverterOnLoad = 0;
	UPS.Inverter.Status.BypassOnLoad = 0;
	
	GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1; // Fault Command to Charger
	Counter_AfterReset = 0;
	Counter_Detect_OnCommand = 0;
	UPS.Inverter.Status.ShutDown = 1;

	
	SystemIsHot = 0;
	
	if(UPS.Inverter.Status.OverLoad == 1 && UPS.Inverter.Status.ON == 1){
		State_AutoReset = 1;
	}

	UPS.Inverter.Status.ON = 0;

}



 void Turn_on_system(void){
 	GpioDataRegs.GPACLEAR.bit.GPIO10 = 1; // Enable RST PWM	

	EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM1
	EPwm2Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM2
	EPwm3Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM3
	EDIS;
	UPS.Inverter.Status.ON = 1;
 }
 
 void Turn_on_bypass(void){
	GpioDataRegs.GPBSET.bit.GPIO49 = 1; //Disable STS for inverter
	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1; //Enable STS for Bypass
	
	EALLOW;
 	EPwm6Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM6
 	EDIS;	
 	
	UPS.Inverter.Status.BypassOnLoad = 1;
	UPS.Inverter.Status.InverterOnLoad = 0;
}
 
 
 void Turn_on_STS(void){
 	GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1;   //Enable STS for inverter
 	GpioDataRegs.GPASET.bit.GPIO25 = 1; //Disable STS for Bypass
 	EALLOW;
 	EPwm6Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM6
 	EDIS;
 	UPS.Inverter.Status.InverterOnLoad = 1;
 	UPS.Inverter.Status.BypassOnLoad = 0; 
 } 
 
void ModbusInputRegUpdate(void)
{


//	InputRegister[POWER_FACTOR_R] = ((Uint16)(100.0*s[0] + 0.5) & 0x00FF);
//	InputRegister[POWER_FACTOR_ST] = (((Uint16)(100.0*s[1] + 0.5) << 8) & 0xFF00) | ((Uint16)(100.0*s[1] + 0.5) & 0x00FF);
//	InputRegister[SHDN_REM_TIME] = s[2];//
//	InputRegister[PARALLEL_CODE_12] = s[3];//
//	InputRegister[PARALLEL_CODE_34] = s[4];//
//	InputRegister[LOAD_T_VOLTAGE] = (Uint16)(s[5] + 0.5);
//	InputRegister[LOAD_S_VOLTAGE] = (Uint16)(s[6] + 0.5);
//	InputRegister[LOAD_R_VOLTAGE] = (Uint16)(s[7] + 0.5);
//	InputRegister[OUT_PWR_R_H] = ((Uint32)s[8] >> 16);//
//	InputRegister[OUT_PWR_R_L] = ((Uint32)s[8]);//
//	InputRegister[OUT_PWR_S_H] = ((Uint32)s[9] >> 16);//
//	InputRegister[OUT_PWR_S_L] = ((Uint32)s[9]);//
//	InputRegister[OUT_PWR_T_H] = ((Uint32)s[10] >> 16);//
//	InputRegister[OUT_PWR_T_L] = ((Uint32)s[10]);//
//	InputRegister[FAULT_CODE] = 0; //PROCESSOR STATE	:	000
//	InputRegister[FIRMWARE_VERSION]  =  1; /* FIRMWARE VERSION */ //INVERTER	FW. VER.
//	InputRegister[BYPASS_R_VOLTAGE] = (Uint16)(s[14] + 0.5);
//	InputRegister[INVERTER_T_VOLTAGE] = (Uint16)(s[15] + 0.5);
//	InputRegister[LOAD_T_CURRRENT] = (Uint16)(s[16] + 0.5);
//	InputRegister[INVERTER_S_VOLTAGE] = (Uint16)(s[17]  + 0.5);
//	InputRegister[LOAD_S_CURRENT] = (Uint16)(s[18] + 0.5);
//	InputRegister[INVERTER_R_VOLTAGE] = (Uint16)(s[19] + 0.5);
//	InputRegister[LOAD_R_CURRENT] = (Uint16)(s[20] + 0.5);
//
//	UpsStatus1.Bit.MANUALLY_BYPASS =in[0];// 0;//; //MANUALLY BYPASS********
//										        //OVER LOAD
//	UpsStatus1.Bit.INVERTER_ON_LOAD = in[1];//UPS.Inverter.Status.InverterOnLoad; //UPS ON LOAD
//	UpsStatus1.Bit.BYPASS_ON_LOAD = in[2];//UPS.Inverter.Status.BypassOnLoad; // BYPASS ON LOAD
//	UpsStatus1.Bit.SYNC = in[3];//UPS.Inverter.Status.Sync;
//	UpsStatus1.Bit.OVER_LOAD_EXIST = in[4];//0;//; //******************
//	UpsStatus1.Bit.rsvd = in[5];//0;
//
//	UpsStatus2.Bit.IDF1 = in[6];//0;//UPS.Inverter.Status.IDF1;       //IGBT DRIVER FAULT 1
//	UpsStatus2.Bit.IDF2 = in[7];//0;//UPS.Inverter.Status.IDF2;       //IGBT DRIVER FAULT 2
//	UpsStatus2.Bit.IDF3 = in[8];//0;//UPS.Inverter.Status.IDF3;       //IGBT DRIVER FAULT 3
//	UpsStatus2.Bit.BYPASS_FREQUENCY_FAIL = in[9];//UPS.Inverter.Status.BypassFail;	// from GPIO59 says input voltage is in range ;
//												  //P2->BYPASS FREQUENCY FAIL
//												  //P2->BYPASS FAULT
//	UpsStatus2.Bit.INVERTER_OVER_LOAD = in[10];//UPS.Inverter.Status.OverCurrent;//P2->OVER LOAD
//	UpsStatus2.Bit.INVERTER_SHORT_CIRCUIT = in[11];//UPS.Inverter.Status.ShortCircuitR || UPS.Inverter.Status.ShortCircuitS || UPS.Inverter.Status.ShortCircuitT;//P2-> SHORT CIRCUIT
//	UpsStatus2.Bit.LOAD_BREAKER = in[12];//0;//;//P2-> LOAD DISCONNECT
//	UpsStatus2.Bit.INVERTER_FAIL = in[13];//UPS.Inverter.Status.ComFault;//0-> P1-> INVERTER OK
//																		 //1-> P1-> INVERTER FAILED P2-> INVERTER FAIL
//	UpsStatus2.Bit.INV_STS_FAULT = in[14];//UPS.Inverter.Status.InvSTSFault;
//	UpsStatus2.Bit.VOLTAGE_OUT_OF_RANGE = in[15];//(!UPS.Inverter.Status.PG && UPS.Inverter.Status.InverterOnLoad);
//	UpsStatus2.Bit.BYPASS_STS_FAULT = in[16];//0;
//	UpsStatus2.Bit.OVER_TEMP = in[17];//0;
//
//
//	UpsStatus2.Bit.rsvd = 0;
//
//	UpsStatus3.Bit.rsvd = 0;
//
//	UpsStatus4.Bit.rsvd = 0;

//========================================================================================================================================================
	InputRegister[APP_PWR_R_H] = ((Uint32)sine_mainsV_Rout.Papp >> 16);//
	InputRegister[APP_PWR_R_L] = ((Uint32)sine_mainsV_Rout.Papp);//
	InputRegister[APP_PWR_S_H] = ((Uint32)sine_mainsV_Sout.Papp >> 16);//
	InputRegister[APP_PWR_S_L] = ((Uint32)sine_mainsV_Sout.Papp);//
	InputRegister[APP_PWR_T_H] = ((Uint32)sine_mainsV_Tout.Papp>> 16);//
	InputRegister[APP_PWR_T_L] = ((Uint32)sine_mainsV_Tout.Papp);//
	InputRegister[INVERTER_R_CURRENT] = (Uint16)(Irms_R);
	InputRegister[INVERTER_S_CURRENT] =	(Uint16)(Irms_S);
	InputRegister[INVERTER_T_CURRENT] = (Uint16)(Irms_T);
	InputRegister[POWER_FACTOR_R] = ((Uint16)(100.0*sine_mainsV_Rout.Cosphi) & 0x00FF);
	InputRegister[POWER_FACTOR_ST] = (((Uint16)(100.0*sine_mainsV_Sout.Cosphi) << 8) & 0xFF00) | ((Uint16)(100.0*sine_mainsV_Tout.Cosphi) & 0x00FF);
	InputRegister[SHDN_REM_TIME] = 0;//
	InputRegister[PARALLEL_CODE_12] = 0;//
	InputRegister[PARALLEL_CODE_34] = 0;//
	InputRegister[LOAD_T_VOLTAGE] = (Uint16)(Vrms_Tout);
	InputRegister[LOAD_S_VOLTAGE] = (Uint16)(Vrms_Sout);
	InputRegister[LOAD_R_VOLTAGE] = (Uint16)(Vrms_Rout);
	InputRegister[OUT_PWR_R_H] = ((Uint32)sine_mainsV_Rout.Pavg >> 16);//
	InputRegister[OUT_PWR_R_L] = ((Uint32)sine_mainsV_Rout.Pavg);//
	InputRegister[OUT_PWR_S_H] = ((Uint32)sine_mainsV_Sout.Pavg >> 16);//
	InputRegister[OUT_PWR_S_L] = ((Uint32)sine_mainsV_Sout.Pavg);//
	InputRegister[OUT_PWR_T_H] = ((Uint32)sine_mainsV_Tout.Pavg >> 16);//
	InputRegister[OUT_PWR_T_L] = ((Uint32)sine_mainsV_Tout.Pavg);//
	InputRegister[FAULT_CODE] = 0; //PROCESSOR STATE	:	000
	InputRegister[FIRMWARE_VERSION]  =  1; /* FIRMWARE VERSION */ //INVERTER	FW. VER.
	InputRegister[BYPASS_R_VOLTAGE] = 0;//(Uint16)(Vrms_Rout);
	InputRegister[INVERTER_T_VOLTAGE] =(Uint16)(Vrms_T);
	InputRegister[LOAD_T_CURRRENT] = (Uint16)(Irms_Tout);
	InputRegister[INVERTER_S_VOLTAGE] = (Uint16)(Vrms_S);
	InputRegister[LOAD_S_CURRENT] = (Uint16)(Irms_Sout);
	InputRegister[INVERTER_R_VOLTAGE] = (Uint16)(Vrms_R);
	InputRegister[LOAD_R_CURRENT] = (Uint16)(Irms_Rout);


	UpsStatus1.Bit.MANUALLY_BYPASS = 0;//; //MANUALLY BYPASS********
										        //OVER LOAD
	UpsStatus1.Bit.INVERTER_ON_LOAD = UPS.Inverter.Status.InverterOnLoad; //UPS ON LOAD
	UpsStatus1.Bit.BYPASS_ON_LOAD = UPS.Inverter.Status.BypassOnLoad; // BYPASS ON LOAD
	UpsStatus1.Bit.SYNC = UPS.Inverter.Status.Sync;
	UpsStatus1.Bit.OVER_LOAD_EXIST = UPS.Inverter.Status.OverLoadExist;//; //******************
	UpsStatus1.Bit.rsvd = 0;

	UpsStatus2.Bit.IDF1 = UPS.Inverter.Status.IDF;           //IGBT DRIVER FAULT 1
	UpsStatus2.Bit.IDF2 = 0;//UPS.Inverter.Status.IDF2;       //IGBT DRIVER FAULT 2
	UpsStatus2.Bit.IDF3 = 0;//UPS.Inverter.Status.IDF3;       //IGBT DRIVER FAULT 3

	UpsStatus2.Bit.BYPASS_FREQUENCY_FAIL = 0;//UPS.Inverter.Status.BypassFail;	// from GPIO59 says input voltage is in range ;
												  //P2->BYPASS FREQUENCY FAIL
												  //P2->BYPASS FAULT
	UpsStatus2.Bit.INVERTER_OVER_LOAD = UPS.Inverter.Status.OverLoad;//P2->OVER LOAD
	UpsStatus2.Bit.INVERTER_SHORT_CIRCUIT = (UPS.Inverter.Status.ShortCircuitR || UPS.Inverter.Status.ShortCircuitS || UPS.Inverter.Status.ShortCircuitT);//P2-> SHORT CIRCUIT
	UpsStatus2.Bit.LOAD_BREAKER = 0;//;//P2-> LOAD DISCONNECT
	UpsStatus2.Bit.INVERTER_FAIL = UPS.Inverter.Status.ComFault;//0-> P1-> INVERTER OK
																		 //1-> P1-> INVERTER FAILED P2-> INVERTER FAIL
	UpsStatus2.Bit.INV_STS_FAULT = UPS.Inverter.Status.InvSTSFault;
	UpsStatus2.Bit.VOLTAGE_OUT_OF_RANGE = (!UPS.Inverter.Status.PG && UPS.Inverter.Status.InverterOnLoad);
	UpsStatus2.Bit.BYPASS_STS_FAULT = UPS.Inverter.Status.BypassSTSFault;
	UpsStatus2.Bit.OVER_TEMP = 0;


	UpsStatus2.Bit.rsvd = 0;

	UpsStatus3.Bit.rsvd = 0;

	UpsStatus4.Bit.rsvd = 0;

	InputRegister[STATUS_1] = UpsStatus1.Reg;
	InputRegister[STATUS_2] = UpsStatus2.Reg;
	InputRegister[STATUS_3] = UpsStatus3.Reg;
	InputRegister[STATUS_4] = UpsStatus4.Reg;
	
}

Uint16 CheckSettings(Uint16 add, Uint16 val)
{
	/* add refers to address of array */
	Uint16 rtn = 0;
	Uint16 mapAdd = add + HOLDING_REG_START_ADD - 1;

	switch (mapAdd)
	{
	    case 0x0801: /* configuration */
			break;
	    case 0x0807: /* Inverter Output Voltage */
			if (val == 50 || val == 60) rtn = 1;
			break;
	    case 0x0808: /* Inverter Output Mode */
			//rtn = 1;
			//if (val == 110 || val == 220)
			break;
	    case 0x0809: /* Inverter Output Voltage */
			if (val >= 220 && val <= 225) rtn = 1;
			break;
		default:
			break;
	}

	return rtn;
}

void ModbusHoldingRegUpdate(void)
{
	Uint16 arrayAdd = ModbusEntryHoldingRegistrAddress();
	Uint16 mapAdd = arrayAdd + HOLDING_REG_START_ADD - 1;

	switch (mapAdd)
	{
		case 0x807: /* Inverter Output Voltage*/
			//inverter_frequency1 = HoldingRegister[arrayAdd];
			break;
		case 0x808: /* Inverter Output Mode*/
			break;
		case 0x809: /*Inverter Output Voltage */
			inverter_voltage1 = HoldingRegister[arrayAdd];
			break;
	}

}

void ModbusRwDataUpdate(void)
{
	


}

void Reset_System(void){
	

	IO_Setup();



	DutyR=0;
	DutyS=0;
    DutyT=0;
    Duty_STS=-0.8;
	
	
	Flag_Start=0,Counter_Start=0,Flag_Sync=2,Flag_ShutDown=0,Counter_Emergency_Shutdown=0,nsamples=0,Counter_SyncSlowly=0,Flag_PowerOk=0, Counter_Sync_OK=0;
	ChargerReady_Prev=0,ChargerReady=0,Shutdown_Counter=0,Emergency_Shutdown_Prev=0,Counter_Shutdown=0,SystemIsHot=0, Emergency_Flag = 0;
	PowerGood=0, PowerBad=0, NormalPower_Counter=0, TurnOnBypass=0,Flag_ZCD=0,Counter_Sequence_OK=0, GPIO58_prev=0,Flag_Manual_STS = 0, Flag_Manual_Bypass = 0, Flag_Auto_STS = 0;
	Counter_IDF=0, Counter_ChargerFault=0, Counter_OverVoltage=0, Counter_ShortCircuitR=0, Counter_ShortCircuitS=0, Counter_ShortCircuitT=0, Counter_BypassSTSFault=0, Counter_InvSTSFault=0, Counter_BuckFault=0;
	Flag_ShortCircuitR=0, Flag_ShortCircuitS = 0 , Flag_ShortCircuitT = 0, Counter_ShortCircuitExistR = 0, Counter_ShortCircuitExistS = 0, Counter_ShortCircuitExistT = 0;
	Counter_ShortCircuit_SoftStartR=0, Counter_ShortCircuit_SoftStartS=0, Counter_ShortCircuit_SoftStartT=0,shortcircuit_limit_nsamplesR=2,shortcircuit_limit_nsamplesS=2,shortcircuit_limit_nsamplesT=2;
	OverLoadIndex=0, OverLoadIndex1=0, OverLoadIndex2=0;
	
	V_R= 0.0, V_S=0.0, V_T=0.0, I_R=0.0, I_S=0.0, I_T=0.0,V_Rout=0.0,V_Sout=0.0,V_Tout=0.0,I_Rout=0.0,I_Sout=0.0,I_Tout=0.0,V_RCha=0.0;
	Vrms_R=0.0, Vrms_S=0.0, Vrms_T=0.0,Irms_R=0.0,Irms_S=0.0,Irms_T=0.0,Vrms_Rout=0.0,Vrms_Sout=0.0,Vrms_Tout=0.0,Irms_Rout=0.0,Irms_Sout=0.0,Irms_Tout=0.0;
	V_R_Start=0.0,V_S_Start=0.0,V_T_Start=0.0,mf_Start=0.0, mf_StartR=0.0, mf_StartS=0.0, mf_StartT=0.0;
	ipowerR=0.0, ipowerS=0.0, ipowerT=0.0;
	OverCurrentAccumulator=0.0,ChargerFrequency=0.0;
	inverter_voltage1 = 0, Counter_AfterReset = 0, Flag_Inverter_ClearTrip = 0, Emergency_Flag1 = 0, Counter_Detect_OnCommand = 0, inverter_frequency1 = 0, offset=0;
	inverter_voltage =0.0, Vref_Sin = 0.0, V_DC_BAT = 0.0, I_DC_BAT = 0.0;
	
	offset_avg_counter = 0,Flag_StratUp = 0,Counter_State1_AutoReset=0,State_AutoReset=0;
	offset= 0,offset_sum = 0,offset_avg=0;
	
	if(TEST_MODE){
		inverter_voltage1 = 22;
	}
	else{
		inverter_voltage1 = 220;
	}

	
	//------------------------------------------PLL Module Initialization---------------------------------
	SPLL_1ph_SOGI_F_init(50, ((float32)(1.0/SAMPLING_FREQ)), &spll1); 
	SPLL_1ph_SOGI_F_coeff_update(((float32)(1.0/SAMPLING_FREQ)), (float32)(2*3.14159*50), &spll1);
	
    spll1.Umax =(Max_Freq - GRID_FREQ);
    spll1.Umin= -(GRID_FREQ - Min_Freq);
	//spll1.lpf_coeff.B0_lf=(float32)(223.2095);
    //spll1.lpf_coeff.B1_lf=(float32)(-221.111);

	SPLL_1ph_SOGI_F_init(50, ((float32)(1.0/SAMPLING_FREQ)), &spll2); 
	SPLL_1ph_SOGI_F_coeff_update(((float32)(1.0/SAMPLING_FREQ)), (float32)(2*3.14159*50), &spll2);
	//spll2.lpf_coeff.B0_lf=(float32)(223.2095);
    //spll2.lpf_coeff.B1_lf=(float32)(-221.111);
    spll2.Umax=10.0;
    spll2.Umin=-10.0;		
//------------------------RMS Module Initialization-----------------------------------------
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_R);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_S);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_T);
	
	sine_mainsV_R.SampleFreq = SAMPLING_FREQ;
	sine_mainsV_S.SampleFreq = SAMPLING_FREQ;                  //ISR Frequency
	sine_mainsV_T.SampleFreq = SAMPLING_FREQ;                
	
	sine_mainsV_R.nsamplesMin=MIN_SAMPLES;
	sine_mainsV_S.nsamplesMin=MIN_SAMPLES;
	sine_mainsV_T.nsamplesMin=MIN_SAMPLES;                   
	                                               // 12000/50=240 ==> 200<240<280
	sine_mainsV_R.nsamplesMax=MAX_SAMPLES;
	sine_mainsV_S.nsamplesMax=MAX_SAMPLES;
	sine_mainsV_T.nsamplesMax=MAX_SAMPLES;
	
	sine_mainsV_R.Threshold=20.0;
	sine_mainsV_S.Threshold=20.0;
	sine_mainsV_T.Threshold=20.0;

//---------------------------------------------------------------------------------------	
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_Rout);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_Sout);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_Tout);
	
	sine_mainsV_Rout.SampleFreq = SAMPLING_FREQ;
	sine_mainsV_Sout.SampleFreq = SAMPLING_FREQ;                  //ISR Frequency
	sine_mainsV_Tout.SampleFreq = SAMPLING_FREQ;                
	
	sine_mainsV_Rout.nsamplesMin=MIN_SAMPLES;
	sine_mainsV_Sout.nsamplesMin=MIN_SAMPLES;
	sine_mainsV_Tout.nsamplesMin=MIN_SAMPLES;                   
	                                               // 12000/50=240 ==> 200<240<280
	sine_mainsV_Rout.nsamplesMax=MAX_SAMPLES;
	sine_mainsV_Sout.nsamplesMax=MAX_SAMPLES;
	sine_mainsV_Tout.nsamplesMax=MAX_SAMPLES;
	
	sine_mainsV_Rout.Threshold=20.0;
	sine_mainsV_Sout.Threshold=20.0;
	sine_mainsV_Tout.Threshold=20.0;

//------------------------------------------PI Regulators Initialization---------------------------------

	CNTL_PI_F_init(&cntl_pi1);  //PI Regulator for Phase a Voltage
	cntl_pi1.Ki =(Ki_Va/SAMPLING_FREQ) ;   //188.0/12000
	cntl_pi1.Kp =(Kp_Va) ;
	cntl_pi1.Umax=(1.5);
	cntl_pi1.Umin=(-1.5);

	CNTL_PI_F_init(&cntl_pi2);  //PI Regulator for Phase b Voltage
	cntl_pi2.Ki =((Ki_Vb/SAMPLING_FREQ)) ;   //188.0/12000
	cntl_pi2.Kp =(Kp_Vb) ;
	cntl_pi2.Umax=(1.5);
	cntl_pi2.Umin=(-1.5);
	
	CNTL_PI_F_init(&cntl_pi3);  //PI Regulator for Phase c Voltage
	cntl_pi3.Ki =(Ki_Vc/SAMPLING_FREQ) ;   //188.0/12000
	cntl_pi3.Kp =(Kp_Vc) ;
	cntl_pi3.Umax=(1.5);
	cntl_pi3.Umin=(-1.5);
	
	CNTL_PI_F_init(&cntl_pi4);   //PI Regulator for Phase a Current
	cntl_pi4.Ki =(Ki_Ia/SAMPLING_FREQ) ;   //73.0/12000 we use 0 in this situation
	cntl_pi4.Kp =(Kp_Ia) ;
	cntl_pi4.Umax=(1.0);
	cntl_pi4.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi5);   //PI Regulator for Phase b Current
	cntl_pi5.Ki =(Ki_Ib/SAMPLING_FREQ) ;   //73.0/12000 we use 0 in this situation
	cntl_pi5.Kp =(Kp_Ib) ;
	cntl_pi5.Umax=(1.0);
	cntl_pi5.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi6);   //PI Regulator for Phase c Current
	cntl_pi6.Ki =(Ki_Ic/SAMPLING_FREQ) ;   //73.0/12000 we use 0 in this situation
	cntl_pi6.Kp =(Kp_Ic) ;
	cntl_pi6.Umax=(1.0);
	cntl_pi6.Umin=(-1.0);
	
//------------------------------------------PR Regulators Initialization---------------------------------	
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h1);
	Coeff1_h1.Coeff_B2=-B2_H1;
	Coeff1_h1.Coeff_B1=B1_H1;
	Coeff1_h1.Coeff_B0=B0_H1;
	Coeff1_h1.Coeff_A2=-A2_H1;
	Coeff1_h1.Coeff_A1=A1_H1;                       
	Coeff1_h1.Min=-1;
	Coeff1_h1.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h1);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h1);
	Coeff2_h1.Coeff_B2=-B2_H1;
	Coeff2_h1.Coeff_B1=B1_H1;
	Coeff2_h1.Coeff_B0=B0_H1;
	Coeff2_h1.Coeff_A2=-A2_H1;
	Coeff2_h1.Coeff_A1=A1_H1;
	Coeff2_h1.Min=-1;
	Coeff2_h1.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h1);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h1);
	Coeff3_h1.Coeff_B2=-B2_H1;
	Coeff3_h1.Coeff_B1=B1_H1;
	Coeff3_h1.Coeff_B0=B0_H1;
	Coeff3_h1.Coeff_A2=-A2_H1;
	Coeff3_h1.Coeff_A1=A1_H1;
	Coeff3_h1.Min=-1;
	Coeff3_h1.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h1);	
	
//------------------------------------------PR Regulators Initialization---------------------------------	
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h3);
	Coeff1_h3.Coeff_B2=-B2_H3;
	Coeff1_h3.Coeff_B1=B1_H3;
	Coeff1_h3.Coeff_B0=B0_H3;
	Coeff1_h3.Coeff_A2=-A2_H3;
	Coeff1_h3.Coeff_A1=A1_H3;                       
	Coeff1_h3.Min=-1;
	Coeff1_h3.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h3);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h3);
	Coeff2_h3.Coeff_B2=-B2_H3;
	Coeff2_h3.Coeff_B1=B1_H3;
	Coeff2_h3.Coeff_B0=B0_H3;
	Coeff2_h3.Coeff_A2=-A2_H3;
	Coeff2_h3.Coeff_A1=A1_H3;
	Coeff2_h3.Min=-1;
	Coeff2_h3.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h3);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h3);
	Coeff3_h3.Coeff_B2=-B2_H3;
	Coeff3_h3.Coeff_B1=B1_H3;
	Coeff3_h3.Coeff_B0=B0_H3;
	Coeff3_h3.Coeff_A2=-A2_H3;
	Coeff3_h3.Coeff_A1=A1_H3;
	Coeff3_h3.Min=-1;
	Coeff3_h3.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h3);
	
//------------------------------------------PR Regulators Initialization---------------------------------	
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h5);
	Coeff1_h5.Coeff_B2=-B2_H5;
	Coeff1_h5.Coeff_B1=B1_H5;
	Coeff1_h5.Coeff_B0=B0_H5;
	Coeff1_h5.Coeff_A2=-A2_H5;
	Coeff1_h5.Coeff_A1=A1_H5;                       
	Coeff1_h5.Min=-1;
	Coeff1_h5.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h5);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h5);
	Coeff2_h5.Coeff_B2=-B2_H5;
	Coeff2_h5.Coeff_B1=B1_H5;
	Coeff2_h5.Coeff_B0=B0_H5;
	Coeff2_h5.Coeff_A2=-A2_H5;
	Coeff2_h5.Coeff_A1=A1_H5;
	Coeff2_h5.Min=-1;
	Coeff2_h5.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h5);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h5);
	Coeff3_h5.Coeff_B2=-B2_H5;
	Coeff3_h5.Coeff_B1=B1_H5;
	Coeff3_h5.Coeff_B0=B0_H5;
	Coeff3_h5.Coeff_A2=-A2_H5;
	Coeff3_h5.Coeff_A1=A1_H5;
	Coeff3_h5.Min=-1;
	Coeff3_h5.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h5);			
//------------------------------------------PR Regulators Initialization---------------------------------	
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h7);
	Coeff1_h7.Coeff_B2=-B2_H7;
	Coeff1_h7.Coeff_B1=B1_H7;
	Coeff1_h7.Coeff_B0=B0_H7;
	Coeff1_h7.Coeff_A2=-A2_H7;
	Coeff1_h7.Coeff_A1=A1_H7;                       
	Coeff1_h7.Min=-1;
	Coeff1_h7.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h7);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h7);
	Coeff2_h7.Coeff_B2=-B2_H7;
	Coeff2_h7.Coeff_B1=B1_H7;
	Coeff2_h7.Coeff_B0=B0_H7;
	Coeff2_h7.Coeff_A2=-A2_H7;
	Coeff2_h7.Coeff_A1=A1_H7;
	Coeff2_h7.Min=-1;
	Coeff2_h7.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h7);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h7);
	Coeff3_h7.Coeff_B2=-B2_H7;
	Coeff3_h7.Coeff_B1=B1_H7;
	Coeff3_h7.Coeff_B0=B0_H7;
	Coeff3_h7.Coeff_A2=-A2_H7;
	Coeff3_h7.Coeff_A1=A1_H7;
	Coeff3_h7.Min=-1;
	Coeff3_h7.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h7);		

//------------------------------------------PR Regulators Initialization---------------------------------		
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h9);
	Coeff1_h9.Coeff_B2=-B2_H9;
	Coeff1_h9.Coeff_B1=B1_H9;
	Coeff1_h9.Coeff_B0=B0_H9;
	Coeff1_h9.Coeff_A2=-A2_H9;
	Coeff1_h9.Coeff_A1=A1_H9;                       
	Coeff1_h9.Min=-1;
	Coeff1_h9.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h9);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h9);
	Coeff2_h9.Coeff_B2=-B2_H9;
	Coeff2_h9.Coeff_B1=B1_H9;
	Coeff2_h9.Coeff_B0=B0_H9;
	Coeff2_h9.Coeff_A2=-A2_H9;
	Coeff2_h9.Coeff_A1=A1_H9;
	Coeff2_h9.Min=-1;
	Coeff2_h9.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h9);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h9);
	Coeff3_h9.Coeff_B2=-B2_H9;
	Coeff3_h9.Coeff_B1=B1_H9;
	Coeff3_h9.Coeff_B0=B0_H9;
	Coeff3_h9.Coeff_A2=-A2_H9;
	Coeff3_h9.Coeff_A1=A1_H9;
	Coeff3_h9.Min=-1;
	Coeff3_h9.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h9);	
	
//------------------------------------------PR Regulators Initialization---------------------------------		
	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h11);
	Coeff1_h11.Coeff_B2=-B2_H11;
	Coeff1_h11.Coeff_B1=B1_H11;
	Coeff1_h11.Coeff_B0=B0_H11;
	Coeff1_h11.Coeff_A2=-A2_H11;
	Coeff1_h11.Coeff_A1=A1_H11;                       
	Coeff1_h11.Min=-1;
	Coeff1_h11.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars1_h11);
		
	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h11);
	Coeff2_h11.Coeff_B2=-B2_H11;
	Coeff2_h11.Coeff_B1=B1_H11;
	Coeff2_h11.Coeff_B0=B0_H11;
	Coeff2_h11.Coeff_A2=-A2_H11;
	Coeff2_h11.Coeff_A1=A1_H11;
	Coeff2_h11.Min=-1;
	Coeff2_h11.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars2_h11);
	
	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h11);
	Coeff3_h11.Coeff_B2=-B2_H11;
	Coeff3_h11.Coeff_B1=B1_H11;
	Coeff3_h11.Coeff_B0=B0_H11;
	Coeff3_h11.Coeff_A2=-A2_H11;
	Coeff3_h11.Coeff_A1=A1_H11;
	Coeff3_h11.Min=-1;
	Coeff3_h11.Max=1;
	CNTL_2P2Z_F_VARS_init(&Vars3_h11);				

//---------------------- Signal Generator module initialization  -----------------         
//	sgen.offset=0;
//	sgen.gain=0x7fff;	// gain=1 in Q15
//	sgen.freq=8948;	// freq = (Required Freq/Max Freq)*2^15
//		          		// = (50/183.1)*2^15 = 8948
//		          		
//	sgen.step_max=1000;      // Max Freq= (step_max * sampling freq)/65536= 183.1
//	sgen.alpha=0;           // phase_norm =(pi/4/(2*pi))*2^16=8192   
//		
//		                      /* So call step_max is normalized fmax in Q16 format             
//		                       step_max=fmax/fs*65536										
//		                        f/fmax*2^15=freq	*/	
//		                        									
//		
//
//	
//----------------------4 Channel Data Logger Initialization---------------------------------------	
//    DLOG_4CH_F_init(&dlog_4ch1); 
//    dlog_4ch1.input_ptr1 = &dval1; 
//    dlog_4ch1.input_ptr2 = &dval2; 
//    dlog_4ch1.input_ptr3 = &dval3; 
//    dlog_4ch1.input_ptr4 = &dval4; 
//    dlog_4ch1.output_ptr1 = &DBUFF_4CH1[0];
//    dlog_4ch1.output_ptr2 = &DBUFF_4CH2[0]; 
//    dlog_4ch1.output_ptr3 = &DBUFF_4CH3[0]; 
//    dlog_4ch1.output_ptr4 = &DBUFF_4CH4[0]; 
//    dlog_4ch1.size = 240; //120
//    dlog_4ch1.pre_scalar = 1;  //for observing 5 cycle of 50 hz signals
//    dlog_4ch1.trig_value = 0; 
//    dlog_4ch1.status = 2;   
	
	
//------------------------------------------------------------------------------------------

	UPS_init();	
	
   OverLoadCounterRef();
   
   
   PWM_Drive(pwm1,DutyR,DutyS,DutyT,Duty_STS);	

}












