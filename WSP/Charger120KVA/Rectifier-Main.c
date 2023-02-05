

#include "SolarInverter-Includes.h"
#include "SPLL_1ph_SOGI_F.h"
#include "C28x_FPU_FastRTS.h"
#include "sgen.h"
#include "CNTL_PI_F.h"
#include <math.h> 
//#include "svgen_F.h"
#include "DLOG_4CH_F.h"
#include "CNTL_2P2Z_F.h"
#include "UPS_Config.h"
/* MODBUS include header */
#include "modbus-rtu.h"
/* MODBUS include header END */


#define DEADTIME_BALANCER	1000//for 20% DutyCycle Max
#define DEADTIME_RECTIFIER	600 //4 us Deadtime   

//lines 100
#define	DC_BUS_VOLTAGE_P 0x0016
#define	DC_BUS_VOLTAGE_N 0x0017
#define	FAULT_CODE 0x0018
#define	FIRMWARE_VERSION 0X0019
#define	MAINS_R_VOLTAGE 0X001A
#define	MAINS_S_VOLTAGE 0X001B
#define	MAINS_T_VOLTAGE 0X001C
#define	MAINS_R_CURRENT 0X001D
#define	MAINS_S_CURRENT 0X001E
#define	MAINS_T_CURRENT 0X001F
#define	CHA_OUTPUT_VOLTAGE 0X0020
#define	BATTERY_VOLTAGE 0X0021
#define	BATTERY_CURRENT 0X0022
#define	STATUS_1 0X0023
#define	STATUS_2 0X0024
#define	STATUS_3 0X0025
#define	STATUS_4 0X0026



struct UPS_STATUS_1
{
	Uint16 OnOff:1;
	Uint16 FloatChargingMode:1;
	Uint16 BatCurLimit:1;
	Uint16 BatDischargeMode:1;
	Uint16 rsvd:12;

};


struct UPS_STATUS_2
{
	Uint16 ChargerFail:1;
	Uint16 RecDCOverVoltage:1;
	Uint16 DriverFail:1;
	Uint16 ACFail:1;
	Uint16 BoostDCOverVoltage:1;
	Uint16 RecDCUnderVoltage:1;
	Uint16 DeepDischarge:1;
	Uint16 BatteryBreaker:1;
	Uint16 InputContactorBreaker:1;
	Uint16 EmergencyShutdown:1;
	Uint16 OverTemp:1;
	Uint16 RectifierShortCircuit:1;
	Uint16 BoostDCUnderVoltage:1;
	Uint16 BatteryOverVoltage:1;
	Uint16 BatteryOverCurrent:1;
	Uint16 ProcessorFault:1;

};

struct UPS_STATUS_3
{
	Uint16 BatteryLowVoltage:1;
	Uint16 BoostOverCurrent:1;
	Uint16 RectifierSequenceFail:1;
	Uint16 VoltageOutOfRange:1;
	Uint16 DCUnbalanceVoltage:1;
	Uint16 BalancerOverCurrent:1;
	Uint16 rsvd:10;
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
void Shut_down_system(void);
void Shut_down_buck(void);
void Turn_on_boost(void);
void Turn_on_power(void);
void Protection(void);
void Reset_System(void);
void Initial_Settings(void);
extern void IO_Setup(void);

Uint16 CheckSettings(Uint16 add, Uint16 val);

/* MODBUS local var */
//Uint16 entryHoldingRegQty;
//	Uint16 entryHoldingRegAdd;
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
//===========================================================================
//SINEANALYZER_DIFF_F sine_mainsI_R;
//SINEANALYZER_DIFF_F sine_mainsI_S;
//SINEANALYZER_DIFF_F sine_mainsI_T;
//===============================================================================
//SINEANALYZER_DIFF_wPWR_F sine_spllV_R;

//-------------------------------PLL Handle Declaration--------------------------------
SPLL_1ph_SOGI_F spll1; 
//SPLL_1ph_SOGI_F spll2; 
//SPLL_1ph_SOGI_F spll3; 

//--------------------------------PI Handle Declaration----------------------------------
CNTL_PI_F cntl_pi1;     //DC Link Voltage Regulator
CNTL_PI_F cntl_pi2; 	//Phase R Regulator
CNTL_PI_F cntl_pi3; 	//Phase S Regulator
CNTL_PI_F cntl_pi4; 	//Phase T Regulator
CNTL_PI_F cntl_pi5;     //Buck Voltage Loop Regulator
CNTL_PI_F cntl_pi6;     //Buck Current Loop Regulator
CNTL_PI_F cntl_pi7;     //Boost Voltage Loop Regulator
CNTL_PI_F cntl_pi8;     //Boost Current Loop Regulator
CNTL_PI_F cntl_pi9;     //Balancing Regulator
CNTL_PI_F cntl_pi10;     //Battery Currnet Regulator
//--------------------------------PR Handle Declaration----------------------------------
CNTL_2P2Z_F_COEFFS Coeff1_h1;
CNTL_2P2Z_F_VARS  Vars1_h1;

CNTL_2P2Z_F_COEFFS Coeff2_h1;
CNTL_2P2Z_F_VARS  Vars2_h1;

CNTL_2P2Z_F_COEFFS Coeff3_h1;
CNTL_2P2Z_F_VARS  Vars3_h1;
//--------------------------------PR Handle Declaration----------------------------------
//CNTL_2P2Z_F_COEFFS Coeff1_h2;
//CNTL_2P2Z_F_VARS  Vars1_h2;
//
//CNTL_2P2Z_F_COEFFS Coeff2_h2;
//CNTL_2P2Z_F_VARS  Vars2_h2;
//
//CNTL_2P2Z_F_COEFFS Coeff3_h2;
//CNTL_2P2Z_F_VARS  Vars3_h2;
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
//---------------------- instance of Signal generator module------------------------------    
//SGENT_3 sgen = SGENT_3_DEFAULTS;



//------------------------------------------Space Vector Modul Initialization---------------------------------
//SVGEN_F S={0,0,0,0,0};		


//----------------------------4 Channel Data Logger Declaration----------------------------
//DLOG_4CH_F dlog_4ch1; 
//float32 DBUFF_4CH1[120]; 
//float32 DBUFF_4CH2[120]; 
//float32 DBUFF_4CH3[120]; 
//float32 DBUFF_4CH4[120]; 
//float32 dval1; 
//float32 dval2; 
//float32 dval3; 
//float32 dval4; 


//-------------------------------Global Parameters Declaration----------------------------

extern struct UPS_V UPS;

float32 DutyR=0;
float32 DutyS=0;
float32 DutyT=0;
float32 Duty_Leg4=-1.0;

Uint32 Flag_Start=0,Flag_OverTemp=0;
Uint16 Counter_Start=0,Counter_DC_OK=0,Counter_Buck_Start=0,Counter_Boost_Start=0,Counter_Boost_Off=0,Flag_BoostFault=0,Flag_BoostDanger=0 ,PowerGood=0, Flag_Auto=0;
Uint16 Flag_BatDisConnected=0, Flag_BatConnected=0, Flag_MainsOff1=0, Flag_MainsOff2=0, Flag_MainsOff3=0, Flag_BuckFault=0, Flag_BuckDanger=0, Flag_RecFault=0, Flag_InvFault=0, Flag_RecDanger=0, Emergency_Flag=0,Flag_IDF=0,Restart_System=0,PowerBad=0;
Uint16 Flag_ZCD=0, Counter_Sequence_OK = 0, Flag_ContactorFault = 0,SystemIsReady = 0,Flag_PowerOk=0;
float32 V_R, V_S, V_T, I_R, I_S, I_T, V_DC, V_DCP,V_DCN,V_DC_OUT,V_DC_BAT,I_DC,K=0.0;
float32 Vrms_R, Vrms_S, Vrms_T, Irms_R, Irms_S, Irms_T, V_RCha;
float32 VDC_Start=0,Step_SoftStart=0,VBuck_Start=0,SoftStart_Buck=0,SoftStart_Boost=0;
Uint16 ref_float_voltage1 = 0, n_of_battery_cells1 = 0, battery_capacity1 = 0, mains_voltage_tolerance1 = 0;
float32 ref_float_voltage = 0.0, vrms_mains_min = 0.0, vrms_mains_max = 0.0, battery_overvoltage = 0.0;
Uint16 mains_voltage1 = 0, battery_type1 = 0, current_limit_rate1 = 0, mains_nominal_voltage1 = 0, ac_fail_time1 = 0, quick_time1 = 0, deep_dischage_voltage1 = 0, low_voltage_alarm1 = 0, quick_voltage1 = 0;
float32 current_limit_rate = 0.0, mains_voltage = 0.0, mains_nominal_voltage = 0.0, deep_dischage_voltage = 0.0;


Uint16 Flag_Boost_CLearTrip=0, Flag_Rectifier_ClearTrip=0, Flag_Buck_ClearTrip=0, Flag_Balancer_ClearTrip=0, Counter_AfterReset = 0, Delay_Null_Contactor=0 , Counter_TurnOn=0, Flag_ManualShutDown = 0;
Uint16 OnCommand = 0, SystemIsHot = 0, Counter_Delay_Contactor=0, Counter_Delay_Reset=0, Delay_Mains_Contactor=0, test1 = 0,Emergency_Flag1=0, Counter_Toggle = 0, Flag_Battery_Ok = 0;
Uint16 Flag_offset_error=0, Flag_UnbalanceVoltageFault=0, Flag_SystemStart = 0,offset_avg_counter = 0,Flag_StratUp = 0, State_AutoReset = 0, Counter_State1_AutoReset =0, Counter_State2_AutoReset=0,Counter_State3_AutoReset=0;
int16 offset= 0,offset_sum = 0,offset_avg=0;

float32 Kp_test = 1.0, Ki_test = 220.0;

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
	pwm1->PeriodMax = PWM_MAX;
	PWM_INIT(pwm1);

	Reset_System();
	Flag_StratUp=1;

	/* MODBUS initial */
	modbus_rtu_init(
			HoldingRegister, HOLDING_REG_QTY, HOLDING_REG_START_ADD,
			InputRegister, INPUT_REG_QTY, INPUT_REG_START_ADD,
			Coil, COIL_QTY, COIL_START_ADD);
	/* MODBUS initial END */



	UPS.Rectifier.Status.ShutDown = 1;
	Initial_Settings();
   
   
//   GpioDataRegs.GPASET.bit.GPIO17 = 1;
//   asm("   RPT #200 || NOP"); 
//   GpioDataRegs.GPACLEAR.bit.GPIO17 = 1; 
   
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
    EPwm1Regs.ETSEL.bit.INTSEL = 2;  // Enable interrupt CNT_zero event
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
//   EDIS;

// Reset the watchdog counter
//   ServiceDog();

	
	// IDLE loop. Just sit and loop forever:	
	for(;;)  //infinite loop
	{
		
		
				
	

 		
		// State machine entry & exit point
		//===========================================================
		(*Alpha_State_Ptr)();	// jump to an Alpha state (A0,B0,...)
		//===========================================================
		
		

 		
	}
	
	
}



//void A0(void)
//{
//	// loop rate synchronizer for A-tasks
//	if(CpuTimer0Regs.TCR.bit.TIF == 1)
//	{
//		CpuTimer0Regs.TCR.bit.TIF = 1;	// clear flag
//
//		//-----------------------------------------------------------
//		(*A_Task_Ptr)();		// jump to an A Task (A1,A2,A3,...)
//		//-----------------------------------------------------------
//
//		VTimer0[0]++;			// virtual timer 0, instance 0 (spare)
//	}
//
//	Alpha_State_Ptr = &B0;		// Comment out to allow only A tasks
//}

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
 					    case 0x0001: /* off cmd */

 							if (Coil[modbusAdd] == 0xFF00){
 								Flag_ManualShutDown = 1;
 								Shut_down_system();
 							}
 							break;
 							
 						case 0x0002: /* on cmd */
 							if (Coil[modbusAdd] == 0xFF00 && UPS.Rectifier.Status.ShutDown == 1 && Counter_TurnOn == 0 && UPS.Rectifier.Status.PG == 1){
 								Reset_System();	
 								Counter_TurnOn = 1;
 								Flag_SystemStart = 1;
 							}
 							break;
 						case 0x0003: /* ups on load */
 							break;
 							
 						case 0x0004: /* bypass on load */
 							//if (Coil[modbusAdd] == 0xFF00) Shut_up_system();
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
//	A - TASKS (executed in every 1 msec)
//=================================================================================
//--------------------------------------------------------
//void A1(void) // SPARE (not used)
//--------------------------------------------------------
//{

	
	//-------------------
	//the next time CpuTimer0 'counter' reaches Period value go to A2
//	A_Task_Ptr = &A2;
	//-------------------
//}

//-----------------------------------------------------------------
//void A2(void) // SPARE (not used)
//-----------------------------------------------------------------
//{	

	//-------------------
	//the next time CpuTimer0 'counter' reaches Period value go to A3
	//A_Task_Ptr = &A3;
	//-------------------
//}

//-----------------------------------------
//void A3(void) // SPARE (not used)
//-----------------------------------------
//{

	//-----------------
	//the next time CpuTimer0 'counter' reaches Period value go to A1
//	A_Task_Ptr = &A1;
	//-----------------
//}


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

     
    if(UPS.Rectifier.Status.On == 1 && Flag_Start==1 && V_DC > VDC_Min_Start){
    	Flag_Start=2;     //Spare
    }
    
    if(UPS.Rectifier.Status.On == 1 && Flag_Start==2){
    	VTimer2[0]=0;
    	Flag_Start=3;
    }
    
	if(UPS.Rectifier.Status.On == 1 && Flag_Start==3 && VTimer2[0] > 40 ){   //40 f0r 2 sec.
		//GpioDataRegs.GPCCLEAR.bit.GPIO79 = 1;   //Turn off relays on board
		Flag_Start=5;
	}
	if(UPS.Rectifier.Status.On == 1 && Flag_Start==4 && VTimer2[0] > 420 && UPS.Rectifier.Status.On_K3==0 && V_DC > VDC_Min_Start){
		GpioDataRegs.GPCSET.bit.GPIO78 = 1;  //Turn on K2 Contactor
		Flag_Start=5;
	}
	
		if(UPS.Rectifier.Status.On == 1 && Flag_Start==5){
		Flag_Start=6; //Delay for DC Voltage Set
	}
	
	
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

	if(GpioDataRegs.GPCDAT.bit.GPIO66 == 0)
		UPS.Rectifier.Status.OverTemp=1;
	else
		UPS.Rectifier.Status.OverTemp=0;

	if(State_AutoReset == 1 && UPS.Rectifier.Status.PG == 1 && Counter_State1_AutoReset < 400)
		Counter_State1_AutoReset++;
	else
		Counter_State1_AutoReset = 0;

	if(State_AutoReset == 2 && UPS.Rectifier.Status.OverTemp == 0 && UPS.Rectifier.Status.PG == 1 && Counter_State2_AutoReset < 400)
		Counter_State2_AutoReset++;
	else
		Counter_State2_AutoReset = 0;

	if(GpioDataRegs.GPADAT.bit.GPIO24 == 0 && UPS.Rectifier.Status.PG == 1 && Counter_State3_AutoReset < 10) //Inverter OverLoad
		Counter_State3_AutoReset++;
	else
		Counter_State3_AutoReset = 0;


	 if((Counter_State1_AutoReset == 400 || Counter_State2_AutoReset == 400 || Counter_State3_AutoReset == 10)  && UPS.Rectifier.Status.ShutDown == 1 && Counter_TurnOn == 0){
		 State_AutoReset = 0;
		 Counter_State1_AutoReset = 0;
		 Counter_State2_AutoReset = 0;
		 Counter_State3_AutoReset = 0;

		 Reset_System();
		 Counter_TurnOn = 1;
		 Flag_SystemStart = 1;
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

	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C1
	C_Task_Ptr = &C1;	
	//-----------------
}

//Inverter    charger
//GPIO57 ---> GPIO57 Command From Charger To Turn on Inverter
//GPIO56 ---> GPIO56 Command From Charger to Turn off Inverter
//GPIO54 ---> GPIO54 Inverter Say to Charger Battery Contactor is Connected
//GPIO55 ---> GPIO55 Command From Charger To Emergency Shutsown Inverter
//GPIO22 ---> GPIO58 Command From Inverter to Turn off Charger
//GPIO23 ---> GPIO59 Charger Says to Inverter that Bypass is in Range
//GPIO58 ---> GPIO23 Charger Says to Inverter that Battery is Reverse
//GPIO59 ---> GPIO24 Charger Says to Inverter that open the battery Contactor


//Inverter    charger
//GPIO57 ---> GPIO57 Charger Says to Inverter that Turn on Inverter*
//GPIO56 ---> GPIO56 Charger Fault
//GPIO54 ---> GPIO54 Inverter Fault
//GPIO55 ---> GPIO55 Inverter says to charger that Inverter reset is complete*
//GPIO22 ---> GPIO58 Manual ShutDown
//GPIO23 ---> GPIO59 Charger Says to Inverter that Bypass is in Range*
//GPIO58 ---> GPIO23 Charger Says to Inverter that Turn on Inverter*
//GPIO59 ---> GPIO24 Inverter OverLoad



interrupt void MainISR(void)
{ 		
	
	GpioDataRegs.GPBSET.bit.GPIO33 = 1;
	asm(" NOP");
	
	
	if(Flag_Boost_CLearTrip == 1){
		EPwm5Regs.AQCSFRC.bit.CSFB=3;   //turn on IGBT related to boost converter
		Flag_Boost_CLearTrip = 0;
	}

	if(Flag_Rectifier_ClearTrip == 1){
		Turn_on_system();
		Flag_Rectifier_ClearTrip = 0;
	}	  	
	
	if(Flag_Buck_ClearTrip == 1){
		GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;  //Enable Leg4 PWM
    	EPwm5Regs.AQCSFRC.bit.CSFA=3;    //enable PWM for Buck IGBT
    	
    	Flag_Buck_ClearTrip = 0;
	}
	

	
	if(Counter_Delay_Contactor > 0 && Counter_Delay_Contactor < 500){
		Counter_Delay_Contactor++;
	}
	
	if(Counter_Delay_Contactor == 500){
		
		GpioDataRegs.GPCCLEAR.bit.GPIO78 = 1;  //Turn off K2 Contactor
		GpioDataRegs.GPCCLEAR.bit.GPIO76 = 1;  // Turn off K3 Contactor	
		
		Counter_Delay_Contactor++;
	}
	
	if(Counter_Delay_Contactor > 500 && Counter_Delay_Contactor < 36000){
		Counter_Delay_Contactor++;	
	}
	if(Counter_Delay_Contactor == 36000){
		UPS.Rectifier.Status.ShutDown = 1;
		Counter_TurnOn = 0;
		Flag_SystemStart = 0;
		Counter_Delay_Contactor++;	
	}
    
//------------------Turn on System Routine----------------------------------------- 

//	if (Counter_AfterReset == 0 && Counter_TurnOn < 100 && PowerGood > PG_START_REC_CYCLES){
//		if(GpioDataRegs.GPCDAT.bit.GPIO66 == 0) 
//			Counter_TurnOn++; 								
//		else
//			Counter_TurnOn = 0;
//	}
	
	if (Counter_AfterReset == 0 && Counter_TurnOn == 1){
		Counter_TurnOn = 100;
	}	
	
	if(Counter_TurnOn == 100 && Counter_AfterReset == 0){	
		OnCommand=1;
//        Shut_down_system();
        Counter_TurnOn++;
        Counter_AfterReset++;
	}
 
    if(OnCommand == 1 && SystemIsHot == 0 && Counter_Delay_Reset < 12000 && Counter_AfterReset == 1){
    		
    	Counter_Delay_Reset++;
    }

     
     if(Counter_Delay_Reset == 12000 && Counter_AfterReset == 1){
         // Enable the watchdog
  //       EALLOW;
 //        SysCtrlRegs.WDCR = 0x0028;   
 //        EDIS;

        // Reset the watchdog counter
 //         ServiceDog(); 

        // Reset DSP  
 //        EALLOW;
 //        SysCtrlRegs.WDCR = 0x0038;   
 //        EDIS;	
         
         OnCommand=0;	

         Counter_AfterReset++;
         
         Counter_Delay_Reset++;							    								
     }     
     



	if(Counter_AfterReset == 2 && SystemIsHot == 0 && Counter_Toggle < 12){
        if(Counter_Toggle % 2 == 0)
      		GpioDataRegs.GPATOGGLE.bit.GPIO23 = 1; //command to reset inverter DSP 
		
		Counter_Toggle++;
	}	
	
	if(Counter_AfterReset == 2 && SystemIsHot == 0 && Counter_Toggle == 12){
		SystemIsHot = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1; //command to Start turn on routine
		Counter_Toggle++;	
		Counter_AfterReset++;
	}	


	if(GpioDataRegs.GPBDAT.bit.GPIO55 == 0 && Counter_AfterReset == 3){
		
		Counter_Start = 1;
 		Flag_Auto = 1;
 		Counter_AfterReset++;
	}
    
    
		
	if(Counter_Start == 1 && UPS.Rectifier.Status.SequenceOK == 1 && Flag_Start == 0 && GpioDataRegs.GPBDAT.bit.GPIO55 == 0 && Counter_AfterReset == 4){
		if(UPS.Rectifier.Status.On == 0){	  
			Turn_on_power();
			GpioDataRegs.GPBSET.bit.GPIO57=1;  // command to turn on Bypass
			Counter_Start=2;
			Counter_AfterReset++;
		}	
	}



	if(Counter_Buck_Start == 60001 && GpioDataRegs.GPBDAT.bit.GPIO55 == 1 && Counter_AfterReset == 5){
		GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1; //command to turn on inverter
		Counter_AfterReset++;
	}
	
	if(Counter_AfterReset == 6 &&  GpioDataRegs.GPBDAT.bit.GPIO55 == 0){
		UPS.Inverter.Status.On = 1;
		GpioDataRegs.GPBSET.bit.GPIO57 = 1;
		Counter_AfterReset++;
	}
	
//---------------------------------------------Update Modbus------------------------------    
 	ModbusInputRegUpdate();




 
//-----------------------Reading ADC Chanels----------------------------------
 	offset=(int16)AdcMirror.ADCRESULT11 - 2048;
		V_R=(AdcMirror.ADCRESULT0*0.00024414-0.5)*2*GAIN_CAL_VAC;
		V_S=(AdcMirror.ADCRESULT1*0.00024414-0.5)*2*GAIN_CAL_VAC;
		V_T=(AdcMirror.ADCRESULT2*0.00024414-0.5)*2*GAIN_CAL_VAC;
		V_DCP=(AdcMirror.ADCRESULT3*0.00024414-0.5)*2*GAIN_CAL_VDC * 0.875;
		V_DCN=(AdcMirror.ADCRESULT4*0.00024414-0.5)*2*GAIN_CAL_VDC * 0.875;
		I_R=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_IAC;
		I_S=(AdcMirror.ADCRESULT6*0.00024414-0.5)*2*GAIN_CAL_IAC;
		I_T=(AdcMirror.ADCRESULT7*0.00024414-0.5)*2*GAIN_CAL_IAC;
		V_DC_BAT=(AdcMirror.ADCRESULT8*0.00024414-0.5)*2*GAIN_CAL_VDC*1.468;
		V_DC_OUT=(AdcMirror.ADCRESULT9*0.00024414-0.5)*2*GAIN_CAL_VDC*1.468;
		I_DC=(AdcMirror.ADCRESULT10*0.00024414-0.5)*2*GAIN_CAL_IAC;
		V_DC=V_DCP+V_DCN;
    
//    if(offset_avg_counter <= 100 && Flag_StratUp == 1){
//		if(offset < 200 && offset > -200 ){
//			offset_avg_counter++;
//			if(offset_avg_counter > 100){
//			   offset_avg=(offset_sum/100);
//			   AdcRegs.ADCOFFTRIM.bit.OFFSET_TRIM -= offset_avg;
//			}
//			else{
//				offset_sum = offset_sum + offset;
//			}
//		}
//    }

    
    	
//-----------------------Signal Generator Module Implementation----------------------    	
    //sgen.calc(&sgen);
    //x1=sgen.out1; 
    //x2=sgen.out2;
    //x3=sgen.out3;
     
    //DutyR=((float)x1/32768);      //Q15 to Float32
    //DutyS=((float)x2/32768);      //Q15 to Float32
    //DutyT=((float)x3/32768);      //Q15 to Float32
 
	
//----------------------PLL Module Implementation--------------------------------------	
	if(UPS.Rectifier.Status.PG == 0){
		V_RCha=0.0;
		spll1.Umax = 0.0;
		spll1.Umin = 0.0;
	}
	else{
		V_RCha=V_R;
		spll1.Umax =2.0;
	    spll1.Umin=-2.0;
	
	}
		
		
	spll1.u[0]=(float32)(V_RCha*Gain_pu); 
	SPLL_1ph_SOGI_F_FUNC(&spll1);
	
//	spll2.u[0]=(float32)(V_S); 
//	SPLL_1ph_SOGI_F_FUNC(&spll2);
	
//	spll3.u[0]=(float32)(V_T); 
//	SPLL_1ph_SOGI_F_FUNC(&spll3);
	
//--------------------RMS Module Implementation--------------------------------------				
    sine_mainsV_R.Vin=V_R*311.0;
    sine_mainsV_S.Vin=V_S*311.0;
    sine_mainsV_T.Vin=V_T*311.0;
    
//    sine_mainsV_R.Vref=spll1.sin;
//    sine_mainsV_S.Vref=spll1.sin;
//    sine_mainsV_T.Vref=spll1.sin;
    
    sine_mainsV_R.Iin=I_R*65.0;
    sine_mainsV_S.Iin=I_S*65.0;
    sine_mainsV_T.Iin=I_T*65.0;
    	
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_R);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_S);
    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsV_T);
    	
    Vrms_R=sine_mainsV_R.Vrms;
    Vrms_S=sine_mainsV_S.Vrms;
    Vrms_T=sine_mainsV_T.Vrms;
	
    Irms_R=sine_mainsV_R.Irms;
    Irms_S=sine_mainsV_S.Irms;
    Irms_T=sine_mainsV_T.Irms;

//========================================================================   
//    sine_spllV_R.Vin=spll1.sin;
//    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_spllV_R);

//=======================================================================    


        if(battery_type1 == 0)
            ref_float_voltage = (float32)(0.00008*ref_float_voltage1*n_of_battery_cells1); //0.00008=6*0.01*(1/750)
        else
            ref_float_voltage = (float32)(0.00001333*ref_float_voltage1*n_of_battery_cells1); //0.0000133=0.01*(1/750)


        current_limit_rate = (float32)(0.000153846*current_limit_rate1*battery_capacity1);//0.000153846 = 0.01*(1/65)
        mains_nominal_voltage = (float32)(mains_nominal_voltage1);
        mains_voltage= (float32)(mains_voltage1);
        vrms_mains_min = (float32)(0.01*(100 - mains_voltage_tolerance1))*mains_voltage;
        vrms_mains_max = (float32)(0.01*(100 + mains_voltage_tolerance1))*mains_voltage;

        if(battery_type1 == 0)
            battery_overvoltage = (float32)(0.02*n_of_battery_cells1);//0.02=2.45*6*(1/750)
        else
            battery_overvoltage = (float32)(0.0022*n_of_battery_cells1);//0.0022=1.65*(1/750)

        if(battery_type1 == 0)
            deep_dischage_voltage = 0.00008*deep_dischage_voltage1*n_of_battery_cells1; //0.0008=(0.01*6)/750
        else
            deep_dischage_voltage = 0.00001333*deep_dischage_voltage1*n_of_battery_cells1; //0.0008=(0.01)/750


        if(current_limit_rate > MAX_CL)
            current_limit_rate = MAX_CL;


	if(UPS.Rectifier.Status.PG == 0){
		vrms_mains_min *= 1.02;
		vrms_mains_max *= 0.98;
	}




//------------------------- Sync inverter output with bypass----------------------------    	
    

    if(sine_mainsV_R.nsamples == 0){
    	Flag_PowerOk=0;
    	if( Vrms_R > vrms_mains_min && Vrms_S > vrms_mains_min && Vrms_T > vrms_mains_min && Vrms_R < vrms_mains_max && Vrms_S < vrms_mains_max && Vrms_T < vrms_mains_max){
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
    	else
    		PowerBad=0;

    	if(PowerBad == PB_DETECTION_CYCLES || Flag_MainsOff1 == QSAMPLES || Flag_MainsOff2 == QSAMPLES || Flag_MainsOff3 == QSAMPLES){//
    		PowerGood=0; 
    		UPS.Rectifier.Status.PG=0;
    		PowerBad=0; 
    		Flag_MainsOff1=0;
    		Flag_MainsOff2=0;
    		Flag_MainsOff3=0;	
    	}
    	   	
    }

    
    if(PowerGood > PG_BOOST_OFF_CYCLES){        //30 sec to decide about stable input power
    	UPS.Rectifier.Status.PG=1;
    }
    
    if(PowerGood > PG_START_REC_CYCLES && Flag_SystemStart == 0){
    	UPS.Rectifier.Status.PG=1;
    }


    if(PowerGood > PG_SYNC_INV_CYCLES){        
    	PowerGood=PG_SYNC_INV_CYCLES;			
    	GpioDataRegs.GPBCLEAR.bit.GPIO59=1;// say to inverter that sync output with input
    }
    
//============================================================================================================ 	
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
 		UPS.Rectifier.Status.SequenceOK = 0;		
 	}
 		
 	if(Counter_Sequence_OK > 10){
 		Counter_Sequence_OK = 10;
 		UPS.Rectifier.Status.SequenceOK = 1;
 	}
 		 
//=====================================================================================    	   
//--------------------------4 Channel Data Logger Implementation----------------------
//   dval1= (spll1.sin); 
//	dval2= (spll2.sin); 
//	dval3= (spll3.sin); 
//	dval4= (spll1.theta[0]); 
//	DLOG_4CH_F_FUNC(&dlog_4ch1); 
//=====================================================================================	

//------------------------------Emergency shut down	--------------------------------------------		
	if(GpioDataRegs.GPCDAT.bit.GPIO64 == 0)
		UPS.Rectifier.Status.EmergencyShutDown = 1;
	else
		UPS.Rectifier.Status.EmergencyShutDown = 0;	
		
	if(Emergency_Flag < 10){
		if(UPS.Rectifier.Status.EmergencyShutDown)
			Emergency_Flag++;
		else
			Emergency_Flag=0;	
	}	
	
	if(Emergency_Flag == 10){
		Shut_down_system();
		Emergency_Flag++;
	}
		
		
	if(SystemIsHot == 1){		
		Protection();
	


	

//=================================================================================================================	
//---------------------------Wait for Turn on System Command----------------------------
	if(Flag_Start==6 && UPS.Rectifier.Status.On_K2==1 && Flag_Auto==1){	
		Flag_Start++;
	}
	
	if(Flag_Start > 6 && Flag_Start < 17 && V_DC > VDC_Min_Start){
    	Flag_Start++;    	
    }

      
    if(Flag_Start==17 && V_DC > VDC_Min_Start && spll1.theta[0] <0.001){
    	
    	EPwm5Regs.AQCSFRC.bit.CSFB=2;      //Turn off boost
		Counter_Boost_Start=0;
		UPS.Boost.Status.On=0;

		UPS.Rectifier.Status.On=1;
    	VDC_Start=V_DC;		

    	CNTL_PI_F_init(&cntl_pi10); //PI Regulator Related to Battery Current
		cntl_pi10.Ki = (Ki_IDC/SAMPLING_FREQ);
		cntl_pi10.Kp = (Kp_IDC);
		cntl_pi10.Umax=(VDC_Start);
		cntl_pi10.Umin=(-VDC_Start);
    		
    	CNTL_PI_F_init(&cntl_pi1); //PI Regulator Related to DC Link Voltage
		cntl_pi1.Ki = (Ki_DC/SAMPLING_FREQ);    //            177.0/12000
		cntl_pi1.Kp = (Kp_DC);
		cntl_pi1.Umax=(1.2);
		cntl_pi1.Umin=(-1.2);	
		
		CNTL_PI_F_init(&cntl_pi2);  //PI Regulator for phase R
		cntl_pi2.Ki = (Ki_a/SAMPLING_FREQ);    // 47/12000 or 0.0
		cntl_pi2.Kp = (Kp_a);
		cntl_pi2.Umax=(1.0);
		cntl_pi2.Umin=(-1.0);
		
		CNTL_PI_F_init(&cntl_pi3);  //PI Regulator for phase S
		cntl_pi3.Ki = (Ki_b/SAMPLING_FREQ);    //47.0/12000
		cntl_pi3.Kp = (Kp_b );
		cntl_pi3.Umax=(1.0);
		cntl_pi3.Umin=(-1.0);
		
		CNTL_PI_F_init(&cntl_pi4);  //PI Regulator for phase T
		cntl_pi4.Ki = (Ki_c/SAMPLING_FREQ);    //47/12000
		cntl_pi4.Kp = (Kp_c);
		cntl_pi4.Umax=(1.0);
		cntl_pi4.Umin=(-1.0);		

    	CNTL_2P2Z_F_VARS_init(&Vars1_h3);
		CNTL_2P2Z_F_VARS_init(&Vars2_h3);
		CNTL_2P2Z_F_VARS_init(&Vars3_h3);
		CNTL_2P2Z_F_VARS_init(&Vars1_h5);
		CNTL_2P2Z_F_VARS_init(&Vars2_h5);
		CNTL_2P2Z_F_VARS_init(&Vars3_h5);
		
    	Flag_Rectifier_ClearTrip = 1; 
    	
    	Flag_Start++;	
    }
    
    if(Flag_Start == 18 && UPS.Inverter.Status.On == 0 && UPS.Rectifier.Status.On == 1){
		Step_SoftStart=0.0000027778;           //   1/(120000*3)
		Flag_Start++;
    }
     
   if(Flag_Start == 18 && UPS.Inverter.Status.On == 1 && UPS.Rectifier.Status.On == 1){
		Step_SoftStart=0.0000005556;           //   1/(120000*3*5)
		Flag_Start++;
    } 
	if(Flag_Start > 18 && Flag_Start < 120000 && V_DC > VDC_Min_Start && UPS.Rectifier.Status.On == 1){
    	VDC_Start+=Step_SoftStart;                                    //Soft Start  
    	Flag_Start++;
    }
    
    	
	if(VDC_Start > VDC_Ref && Flag_Start > 18)	
		VDC_Start=VDC_Ref;
		



	if(Flag_Start > 16)
    { 	

	    cntl_pi10.Umax=(VDC_Start);
	    cntl_pi10.Umin=(-VDC_Start);

    	if(Counter_DC_OK  < 60000 && UPS.Rectifier.Status.On == 1 && Flag_Start == 120000){
    		if(V_DC > 0.95*VDC_Ref)
    			Counter_DC_OK++;		
    	}
    	
    	if(Counter_DC_OK==60000 && UPS.Rectifier.Status.On == 1){

	  		//EPwm5Regs.AQCSFRC.bit.CSFB=2;      //Turn off boost
			//UPS.Boost.Status.On=0;
			//Counter_Boost_Off=0;
			Counter_Buck_Start=0;
			SoftStart_Buck=0;
			//Counter_Boost_Off++;
	    		
	    	Counter_DC_OK++;
    	}



 
//-------------------------------------Buck Start----------------------------------------------------------------------------------------------------  
	if(UPS.Buck.Status.Fault == 0 && UPS.Rectifier.Status.On == 1){
	    if(Counter_DC_OK == 60001 && Counter_Buck_Start < 36000 && UPS.Boost.Status.On == 0 && UPS.Buck.Status.On == 0)
	    	Counter_Buck_Start++;
		
		if(Counter_Buck_Start == 36000 && UPS.Boost.Status.On == 0){
	
	    	CNTL_PI_F_init(&cntl_pi5);  //PI Regulator for Buck Voltage Loop
			cntl_pi5.Ki =(Ki_Vbuck/SAMPLING_FREQ);
			cntl_pi5.Kp =(Kp_Vbuck) ;
			cntl_pi5.Umax=(current_limit_rate);  //1.7/65;
			cntl_pi5.Umin=(-current_limit_rate);  //1.7/65
			
			CNTL_PI_F_init(&cntl_pi6); //PI Regulator for Buck Current Loop
			cntl_pi6.Ki =(Ki_Ibuck/SAMPLING_FREQ) ;
			cntl_pi6.Kp =(Kp_Ibuck) ;
			cntl_pi6.Umax=(1.0);
			cntl_pi6.Umin=(-1.0);
			cntl_pi6.i1=(-1.0);
			
			if(V_DC_OUT > deep_dischage_voltage){
				Flag_Battery_Ok = 1;
			}
		
			Flag_Buck_ClearTrip = 1;

			SoftStart_Buck = V_DC_OUT;
	    	 
	    	UPS.Buck.Status.On=1;
	    	
	    	Counter_Buck_Start++;
	    	
		}
		
		if(Counter_Buck_Start > 36000  && Counter_Buck_Start <= 60000){
			Counter_Buck_Start++;
		}
		
		if(Counter_Buck_Start == 60001 && (V_DC_OUT < battery_overvoltage) ){
			if((ref_float_voltage - SoftStart_Buck) >  2*0.0000416667)
				SoftStart_Buck+=0.0000416667;
			
			else if((ref_float_voltage - SoftStart_Buck) <  -2*0.0000416667)
				SoftStart_Buck-=0.0000416667;
		
			else
				SoftStart_Buck = ref_float_voltage;	
		}
	}
	
//=====================================================================================
//---------------------------------------Turn on boost-----------------------------------------

		if(TEST_MODE){
			if(UPS.Rectifier.Status.PG==0 && Counter_Boost_Start == 0  && Flag_SystemStart == 1){
				if(UPS.Boost.Status.On == 0 && UPS.Rectifier.Status.On == 1 && UPS.Inverter.Status.On == 1){	//Turn on boost
					Turn_on_boost();
					Flag_RecFault++;
				}

				else{	//ShutDown System
                    Shut_down_system();
                    UPS.Rectifier.Status.Fault=1;
                    Flag_RecFault++;
					}
			}
		}



	else{
			if((Flag_RecFault == 2 || UPS.Rectifier.Status.PG==0) && Counter_Boost_Start == 0  && Flag_SystemStart == 1){//
				if(UPS.Boost.Status.On == 0 && UPS.Rectifier.Status.On == 1 && UPS.Inverter.Status.On == 1){
						Turn_on_boost(); //Turn on boost
						Flag_RecFault++;
				}

				else{	//ShutDown System
						Shut_down_system();
						UPS.Rectifier.Status.Fault=1;
						Flag_RecFault++;
					}
		  }
	}

//======================================================================================================================
	
	
    if(Counter_Boost_Start == 121){
    	CNTL_PI_F_init(&cntl_pi7);  //PI Regulator for Boost Voltage Loop
		cntl_pi7.Ki =(Ki_Vboost/SAMPLING_FREQ);
		cntl_pi7.Kp =(Kp_Vboost) ;
		cntl_pi7.Umax=(1.5);
		cntl_pi7.Umin=(-1.5);
		
		CNTL_PI_F_init(&cntl_pi8);  //PI Regulator for Boost Current Loop
		cntl_pi8.Ki =(Ki_Iboost/SAMPLING_FREQ) ;
		cntl_pi8.Kp =(Kp_Iboost) ;
		cntl_pi8.Umax=(1.0);
		cntl_pi8.Umin=(-1.0);
		
		Flag_Boost_CLearTrip = 1;
		
    	Counter_Boost_Start++;
    }
    
    if(Counter_Boost_Start == 120){
    	EALLOW;
    	EPwm1Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM1
		EPwm2Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM2
		EPwm3Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM3
		EDIS;	
		
    	EPwm5Regs.AQCSFRC.bit.CSFA=2;   //turn off IGBT related to buck converter
    	UPS.Rectifier.Status.On=0;
    	UPS.Buck.Status.On=0; 
    	UPS.Boost.Status.On=1;
    	Flag_RecFault=0;  
    	Flag_MainsOff1=0; 
    	Flag_MainsOff2=0; 
    	Flag_MainsOff3=0;                                                                        
    	UPS.Rectifier.Status.Fault=0;
		UPS.Rectifier.Status.DC_UnderVoltage=0;
		UPS.Rectifier.Status.DC_OverVoltage=0;
		
    	Counter_DC_OK=1; 
    	SoftStart_Boost=0.9*VDC_Ref_Boost; 
    	PowerGood=0;
    	UPS.Rectifier.Status.PG = 0;
    	GpioDataRegs.GPBSET.bit.GPIO59=1; //say to inverter that don't sync output with input
    	
    	Counter_Boost_Start++;
    }
  
   
    if(Counter_Boost_Start > 121 && Counter_Boost_Start < 220){	  	
    	SoftStart_Boost+=0.001;   //1.0/100
    	Counter_Boost_Start++;
    }
    
   if(Counter_Boost_Start == 220){	  	
    	GpioDataRegs.GPCCLEAR.bit.GPIO78 = 1;  //Turn off K2 Contactor
    	Counter_Boost_Start++;
    }
        
    if(SoftStart_Boost > VDC_Ref_Boost)
		SoftStart_Boost = VDC_Ref_Boost;
		
		
    if(Counter_Boost_Start == 221){

    		Counter_Boost_Start = 225;
    }

    
    if(Counter_Boost_Start == 225){
    	Counter_Boost_Start++; 
    }
    
    if(Counter_Boost_Start == 226){
    		Counter_Boost_Start =230;
    }

    if(Counter_Boost_Start==230){
    	CNTL_PI_F_init(&cntl_pi9);  //PI Regulator for Balancing DC Link
		cntl_pi9.Ki =(Ki_Balancing/SAMPLING_FREQ) ;
		cntl_pi9.Kp =(Kp_Balancing) ;
		cntl_pi9.Umax=(1.0);
		cntl_pi9.Umin=(-1.0);
		
		

		
		Counter_Boost_Off=0;

		

		
    	Counter_Boost_Start++;
    	
    }




 //=========================================================================================   
 //----------------------------Turn off boost--------------------------------------------
    	if(Counter_Boost_Start == 231){

    		if(UPS.Rectifier.Status.PG == 1 && UPS.Rectifier.Status.SequenceOK == 1 && UPS.Boost.Status.On == 1 && Counter_Boost_Off < 800){
    				Counter_Boost_Off++;
    		}

    		if(Counter_Boost_Off == 800){
    			GpioDataRegs.GPASET.bit.GPIO10 = 1; // Disable RST PWM





    			Counter_Boost_Off++;
    		}

    		if(Counter_Boost_Off > 800 && Counter_Boost_Off < 1000){
    			Counter_Boost_Off++;
    		}

    		if(Counter_Boost_Off == 1000){
    			Counter_Boost_Off++;
    		}


    		if(Counter_Boost_Off == 1001){

    				Counter_Boost_Off = 1005;
    		}

    		if(Counter_Boost_Off == 1005 && V_DC > VDC_Min_Start){
    			VDC_Start = 0;
    			EPwm5Regs.AQCSFRC.bit.CSFA=2; //turn off IGBT related to buck converter
    			Counter_DC_OK = 1;
    			Counter_Boost_Off = 1010;
    		}


    		if(Counter_Boost_Off == 1010){
    			Flag_Start = 17;
    			Counter_Boost_Off++;
    		}
    	}
//=============================================================================================
	
//---------------------PI Regulator Battery Current--------------------------------------
	cntl_pi10.Ref =current_limit_rate;
	cntl_pi10.Fbk =I_DC;
	CNTL_PI_F_FUNC(&cntl_pi10);
	//---------------------PI Regulator for DC Link Voltage--------------------------------------
	cntl_pi1.Ref =cntl_pi10.Out;
   	cntl_pi1.Fbk =V_DC; 
    CNTL_PI_F_FUNC(&cntl_pi1);		

//---------------------PI Regulator for Phase R--------------------------------------  
	cntl_pi2.Ref =cntl_pi1.Out*spll1.sin; 
   	cntl_pi2.Fbk = I_R; 
    CNTL_PI_F_FUNC(&cntl_pi2);		
    
    
//---------------------PI Regulator for Phase S--------------------------------------  
	cntl_pi3.Ref =cntl_pi1.Out*spll1.sin240;  
   	cntl_pi3.Fbk = I_S; 
    CNTL_PI_F_FUNC(&cntl_pi3);  
    
 
//---------------------PI Regulator for Phase T--------------------------------------  
	cntl_pi4.Ref =cntl_pi1.Out*spll1.sin120;  
   	cntl_pi4.Fbk = I_T; 
    CNTL_PI_F_FUNC(&cntl_pi4);  
       
//---------------------PI Regulator for Buck Output Voltage--------------------------------------  
	cntl_pi5.Ref =SoftStart_Buck; //67/750 5*13.38 float voltage for battery bank
   	cntl_pi5.Fbk =V_DC_OUT; 
    CNTL_PI_F_FUNC(&cntl_pi5);
    cntl_pi5.Umax=(current_limit_rate);  //1.7/65;
	cntl_pi5.Umin=(-current_limit_rate);  //1.7/65
    
//---------------------PI Regulator for Buck Output Current--------------------------------------  
	cntl_pi6.Ref =cntl_pi5.Out; 
   	cntl_pi6.Fbk = I_DC; 
    CNTL_PI_F_FUNC(&cntl_pi6); 
       
    
//---------------------PI Regulator for Boost Output Voltage--------------------------------------  
	cntl_pi7.Ref =SoftStart_Boost; //70/750
   	cntl_pi7.Fbk = V_DC; 
    CNTL_PI_F_FUNC(&cntl_pi7);     
  
//---------------------PI Regulator for Boost Output Current--------------------------------------  
	cntl_pi8.Ref =cntl_pi7.Out; 
   	cntl_pi8.Fbk =-1.0*I_DC; 
    CNTL_PI_F_FUNC(&cntl_pi8);   
    
//---------------------PI Regulator for Balancing DC Link--------------------------------------  
	cntl_pi9.Ref =0.0; //70/750
   	cntl_pi9.Fbk = (V_DCN - V_DCP); 
    CNTL_PI_F_FUNC(&cntl_pi9); 
        
//---------------------PR Regulator------------------------------------------  
	Vars1_h1.Ref = cntl_pi1.Out*spll1.sin;
	Vars1_h1.Fdbk = I_R;
	CNTL_2P2Z_F_FUNC( &Coeff1_h1,&Vars1_h1);

//	Vars1_h2.Ref = cntl_pi1.Out*spll1.sin;
//	Vars1_h2.Fdbk = I_R;
//	CNTL_2P2Z_F_FUNC( &Coeff1_h2,&Vars1_h2);
	
	Vars1_h3.Ref = cntl_pi1.Out*spll1.sin;
	Vars1_h3.Fdbk = I_R;
	CNTL_2P2Z_F_FUNC( &Coeff1_h3,&Vars1_h3);
	
	Vars1_h5.Ref = cntl_pi1.Out*spll1.sin;
	Vars1_h5.Fdbk = I_R;
	CNTL_2P2Z_F_FUNC( &Coeff1_h5,&Vars1_h5);
	
	Vars2_h1.Ref = cntl_pi1.Out*spll1.sin240;
	Vars2_h1.Fdbk = I_S;
	CNTL_2P2Z_F_FUNC( &Coeff2_h1,&Vars2_h1);

//	Vars2_h2.Ref = cntl_pi1.Out*spll1.sin240;
//	Vars2_h2.Fdbk = I_S;
//	CNTL_2P2Z_F_FUNC( &Coeff2_h2,&Vars2_h2);
	
	Vars2_h3.Ref = cntl_pi1.Out*spll1.sin240;
	Vars2_h3.Fdbk = I_S;
	CNTL_2P2Z_F_FUNC( &Coeff2_h3,&Vars2_h3);
	
	Vars2_h5.Ref = cntl_pi1.Out*spll1.sin240;
	Vars2_h5.Fdbk = I_S;
	CNTL_2P2Z_F_FUNC( &Coeff2_h5,&Vars2_h5);
	
	Vars3_h1.Ref = cntl_pi1.Out*spll1.sin120;
	Vars3_h1.Fdbk = I_T;
	CNTL_2P2Z_F_FUNC( &Coeff3_h1,&Vars3_h1);

//	Vars3_h2.Ref = cntl_pi1.Out*spll1.sin120;
//	Vars3_h2.Fdbk = I_T;
//	CNTL_2P2Z_F_FUNC( &Coeff3_h2,&Vars3_h2); 
	
	Vars3_h3.Ref = cntl_pi1.Out*spll1.sin120;
	Vars3_h3.Fdbk = I_T;
	CNTL_2P2Z_F_FUNC( &Coeff3_h3,&Vars3_h3);    
	
	Vars3_h5.Ref = cntl_pi1.Out*spll1.sin120;
	Vars3_h5.Fdbk = I_T;
	CNTL_2P2Z_F_FUNC( &Coeff3_h5,&Vars3_h5);
	
	DutyR=0;
	DutyS=0;
	DutyT=0;
	Duty_Leg4=-1;
         
	if(UPS.Rectifier.Status.On == 1){
		DutyR=cntl_pi2.Out - 1.0*Vars1_h1.Out - 100.0*Vars1_h3.Out - 100.0*Vars1_h5.Out;//100
		DutyS=cntl_pi3.Out - 1.0*Vars2_h1.Out - 100.0*Vars2_h3.Out - 100.0*Vars2_h5.Out;
		DutyT=cntl_pi4.Out - 1.0*Vars3_h1.Out - 100.0*Vars3_h3.Out - 100.0*Vars3_h5.Out;
	}

	if(Counter_Buck_Start > 36000 && UPS.Buck.Status.On == 1){					
    		Duty_Leg4=cntl_pi6.Out; 	
	}
	
	if(Counter_Boost_Start > 121 && UPS.Boost.Status.On == 1){			
    		Duty_Leg4=cntl_pi8.Out;   	 
    }
    

    
    
    



       
//    if((V_DC < V_DC_OFFSET ) && UPS.Buck.Status.On==1 && Counter_Boost_Start <120 && Counter_Buck_Start==60001) 	
//    	Counter_Boost_Start++;	
//    	
//    if(V_DC > V_DC_OFFSET && UPS.Buck.Status.On==1 && Counter_Boost_Start <120 && Counter_Buck_Start==60001)
//    	Counter_Boost_Start=0;
    	

//---------------------Saturation Limits--------------------------------------  

	if(DutyR > 0.95)
		DutyR=0.95;
	if(DutyS > 0.95)
		DutyS=0.95;
	if(DutyT > 0.95)
		DutyT=0.95;
    if(Duty_Leg4 > 0.9)
		Duty_Leg4=0.9;			
	if(DutyR < -0.95)
		DutyR=-0.95;
	if(DutyS < -0.95)
		DutyS=-0.95;
	if(DutyT <- 0.95)
		DutyT=-0.95;
	if(Duty_Leg4 <- 1.0)
		Duty_Leg4=-1.0;
		
	if(UPS.Boost.Status.On==1 && Duty_Leg4 > Max_Duty_Leg4)
		Duty_Leg4=Max_Duty_Leg4;			
	

//-----------------------PWM Module Implementation-----------------------------------------
 	         
	PWM_Drive(pwm1,DutyR,DutyS,DutyT,Duty_Leg4);	// Calculate the new PWM compare values	
	
		
//-----------------------------------------------------------------------------------------		
    }
}
	
	//GpioDataRegs.GPCCLEAR.bit.GPIO76 = 1; //Turn on K3 Contctor



//	Turn_on_system();
//	PWM_Drive(pwm1,-0.5,0.0,0.5,0.0);

//	GpioDataRegs.GPACLEAR.bit.GPIO11 = 1; //Disable Leg4 PWM
//	EALLOW;
//	EPwm5Regs.AQCSFRC.bit.CSFB=3; //turn off IGBT related to boost converter
//	EPwm5Regs.AQCSFRC.bit.CSFA=3; //turn off IGBT related to buck converter
//	EDIS;

//	PWM_Drive(pwm1,0.5,0.0,-0.5,0.5);
    

	EPwm1Regs.ETCLR.bit.INT = 1;	// Clear INT flag for this timer
	
	test1= EPwm1Regs.TBCTR;
	GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;
		
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;	// Acknowledge this interrupt to receive more interrupts from group 3
 
}

void Turn_on_power(void){	
		GpioDataRegs.GPCSET.bit.GPIO79 = 1; //Connect R & S wire  to Legs 1 & 2.
		UPS.Rectifier.Status.On=1;
		Flag_Start=1;
}

void Shut_down_system(void){// Shut down system completely
//	GpioDataRegs.GPBCLEAR.bit.GPIO56=1;//Command to turn off Inverter
//	GpioDataRegs.GPACLEAR.bit.GPIO29=1; //turn on D19
//	GpioDataRegs.GPASET.bit.GPIO28=1;   //turn off D17	
	GpioDataRegs.GPASET.bit.GPIO10 = 1; // Disable RST PWM
	GpioDataRegs.GPASET.bit.GPIO11 = 1; //Disable Leg4 PWM
	GpioDataRegs.GPCCLEAR.bit.GPIO79 = 1; 
	
	
	EALLOW;
	EPwm1Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM1
	EPwm2Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM2
	EPwm3Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM3
	EPwm5Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM5
	EPwm5Regs.AQCSFRC.bit.CSFB=2; //turn off IGBT related to boost converter
	EPwm5Regs.AQCSFRC.bit.CSFA=2; //turn off IGBT related to buck converter 
	EDIS;
	
	Counter_Delay_Contactor = 1;
	Counter_AfterReset = 10;
	if(Flag_ManualShutDown == 1)
		GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1;
	else
		GpioDataRegs.GPBCLEAR.bit.GPIO56=1;//Command to turn off Inverter

	

	
	UPS.Rectifier.Status.On=0;
	UPS.Boost.Status.On=0;
	UPS.Buck.Status.On=0;

	SystemIsHot = 0;

	//Initial_Settings();

	if(UPS.Boost.Status.VBatt_End == 1 && UPS.Inverter.Status.On == 1)
		State_AutoReset = 1;
	else if(UPS.Rectifier.Status.OverTemp == 1 && UPS.Inverter.Status.On == 1)
		State_AutoReset = 2;
	
	UPS.Inverter.Status.On =0;


}


void Shut_down_buck(void){ // Shut down buck  
	GpioDataRegs.GPASET.bit.GPIO11 = 1; //Disable Leg4 PWM
	EALLOW;
	EPwm5Regs.AQCSFRC.bit.CSFB=2; //turn off IGBT related to boost converter
	EPwm5Regs.AQCSFRC.bit.CSFA=2; //turn off IGBT related to buck converter 
	EDIS;
	UPS.Buck.Status.On=0;
	UPS.Buck.Status.Fault=1;
	
}

void Turn_on_boost(void){
	Counter_Boost_Start=120;	
    UPS.Boost.Status.On=0;
   	EPwm5Regs.AQCSFRC.bit.CSFB=2;      //Turn off boost
}

void Turn_on_system(void){
 	GpioDataRegs.GPACLEAR.bit.GPIO10 = 1; // Enable RST PWM	
	EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM1
	EPwm2Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM2
	EPwm3Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM3
	EDIS;
 }

#pragma CODE_SECTION(ModbusInputRegUpdate,"ramfuncs2");
void ModbusInputRegUpdate(void)
{

	//InputRegister[j++] = ((unsigned int *) &ReadOnlyData[i])[0];
	//InputRegister[j++] = ((unsigned int *) &ReadOnlyData[i])[1];
	InputRegister[DC_BUS_VOLTAGE_P] = (Uint16)(V_DCP * VDC_NOM);
	InputRegister[DC_BUS_VOLTAGE_N] = (Uint16)(V_DCN * VDC_NOM);
	InputRegister[FAULT_CODE] = test1; //PROCESSOR STATE :	000
	InputRegister[FIRMWARE_VERSION] = 1;  //CHARGER FW. VER	: 00
	InputRegister[MAINS_R_VOLTAGE] = (Uint16)(Vrms_R);
	InputRegister[MAINS_S_VOLTAGE] = (Uint16)(Vrms_S);
	InputRegister[MAINS_T_VOLTAGE] = (Uint16)(Vrms_T);//(Uint16)(Vrms_T);
	InputRegister[MAINS_R_CURRENT] = (Uint16)(Irms_R);
	InputRegister[MAINS_S_CURRENT] = (Uint16)(Irms_S);
	InputRegister[MAINS_T_CURRENT] = (Uint16)(Irms_T);
	InputRegister[CHA_OUTPUT_VOLTAGE] = (Uint16)(V_DC * VDC_NOM); 
	InputRegister[BATTERY_VOLTAGE] = (Uint16)(V_DC_BAT * VDC_NOM);
	InputRegister[BATTERY_CURRENT] = (Uint16)(fabs(I_DC) * IDC_NOM);
	
	UpsStatus1.Bit.OnOff =UPS.Rectifier.Status.On;  //CHARGER STATUS: ON    0->CHARGER STATUS:OFF
	UpsStatus1.Bit.FloatChargingMode = UPS.Buck.Status.On; // FLOATING MODE
	UpsStatus1.Bit.BatCurLimit =(I_DC > 0.95*current_limit_rate && UPS.Buck.Status.On == 1);
	UpsStatus1.Bit.BatDischargeMode = UPS.Boost.Status.On ;//*******
	UpsStatus1.Bit.rsvd = 0;
	
	UpsStatus2.Bit.ChargerFail = UPS.Rectifier.Status.Fault;
	UpsStatus2.Bit.RecDCOverVoltage =(UPS.Rectifier.Status.DC_MaxVoltage);// UPS.Boost.Status.DC_MaxVoltage
	UpsStatus2.Bit.DriverFail = UPS.Rectifier.Status.IDF;
	UpsStatus2.Bit.ACFail = (!UPS.Rectifier.Status.PG);
	UpsStatus2.Bit.BoostDCOverVoltage = UPS.Boost.Status.DC_MaxVoltage;// (UPS.Buck.Status.Bat_OverVoltage || UPS.Buck.Status.OverCurrent); //P2->DC/DC FAIL=========================dc boost over voltage
	UpsStatus2.Bit.RecDCUnderVoltage = (UPS.Rectifier.Status.DC_MinVoltage);// UPS.Rectifier.Status.DC_UnderVoltage;//P2-> CHARGER UNDER VOLTAGE =========================Rec dc under voltage
	UpsStatus2.Bit.DeepDischarge = UPS.Boost.Status.VBatt_End; //P2-> BATTERY DEEP DISCHARGE
	UpsStatus2.Bit.BatteryBreaker = 0;//!GpioDataRegs.GPBDAT.bit.GPIO54;  //1->Disconnected 0->connected 6.0/750=0.008//P2-> BATTERY DISCONNECTED
	UpsStatus2.Bit.InputContactorBreaker = 0;//!UPS.Rectifier.Status.On_K2;  //1->Disconnected 0->connected //P2->CHARGER CONTACTOR DISCONNECTED
	UpsStatus2.Bit.EmergencyShutdown = 0; //UPS.Rectifier.Status.ShortCircuit; ///P2->/EMERGENCY SHUTDOWN
	UpsStatus2.Bit.OverTemp = UPS.Rectifier.Status.OverTemp;
	UpsStatus2.Bit.RectifierShortCircuit = UPS.Rectifier.Status.ShortCircuit;
	UpsStatus2.Bit.BoostDCUnderVoltage = UPS.Boost.Status.DC_MinVoltage;// UPS.Boost.Status.DC_MaxVoltage; ==========================boost dc under voltage
	UpsStatus2.Bit.BatteryOverVoltage = UPS.Buck.Status.Bat_OverVoltage;// UPS.Buck.Status.Bat_OverVoltage;========================= battery over voltage
	UpsStatus2.Bit.BatteryOverCurrent =(UPS.Buck.Status.OverCurrent || UPS.Buck.Status.MaxCurrent);// UPS.Buck.Status.MaxCurrent;=====================battery over current
	UpsStatus2.Bit.ProcessorFault = UPS.Rectifier.Status.ProcessorFault;// 0;
	
	UpsStatus3.Bit.BatteryLowVoltage = UPS.Boost.Status.VBatt_End;// 0; ===================battery low voltage
	UpsStatus3.Bit.BoostOverCurrent = (UPS.Boost.Status.OverCurrent || UPS.Boost.Status.MaxCurrent);
	UpsStatus3.Bit.RectifierSequenceFail = !UPS.Rectifier.Status.SequenceOK;
	UpsStatus3.Bit.VoltageOutOfRange = !UPS.Rectifier.Status.PG;
	UpsStatus3.Bit.DCUnbalanceVoltage = UPS.Rectifier.Status.DC_UnbalanceVoltage;
	UpsStatus3.Bit.BalancerOverCurrent = UPS.Balancer.Status.ShortCircuit;
	UpsStatus3.Bit.rsvd = 12;
	
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
		case 0x0817: /* AC fail time */
			if (Flag_Start == 0 && val >= 30 && val <= 1200) rtn = 1;
			break;
		case 0x0818: /* quick_time */
			if (val >= 30 && val <= 900) rtn = 1;
			break;
		case 0x0819: /* battery type */
			if (val == 0 || val == 1) rtn = 1;
			break;
		case 0x081A: /* mains nominal voltage */
			if (val == 220) rtn = 1;
			break;
		case 0x081B: /* number of battery cells */
			if ((val >= 38 && val <= 50) || (val >= 370 && val <= 470) ) rtn = 1;
			break;	
		case 0x081C:  /* battery capacity */
			if (val >= 7 && val <=200) rtn = 1;
			break;			
		case 0x081D: /* current limit rate */
			if ((val >= 10 && val <= 15) || (val >= 20 && val <= 40)) rtn = 1;
			break;		
		case 0x081E: /* deep discharge voltage */
			if ((val >= 100 && val <= 114) || (val >= 175 && val <= 185)) rtn = 1;
			break;
		case 0x081F: /*low voltage alarm */
			if (val >= 20 && val <= 80) rtn = 1;
			break;
		case 0x0820: /*quick voltage */
			if ((val >= 140 && val <= 155) || (val >= 220 && val <= 245)) rtn = 1;
			break;
	    case 0x0821: /* float voltage*/
			if ((val >= 210 && val <= 235) || (val >= 140 && val <= 145) ) rtn = 1;
			break;
		case 0x0822: /* mains volt tol */
			if (val >= 5 && val <= 15) rtn = 1;
			break;	
		case 0x0823: /* mains voltage */
			if (val >= 220 && val <= 240) rtn = 1;
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
		case 0x817: /* ac_fail_time */
			ac_fail_time1 = HoldingRegister[arrayAdd];
			break;
		case 0x818: /* quick_time */
			quick_time1 = HoldingRegister[arrayAdd];
			break;
		case 0x819: /* battery type */
			battery_type1 = HoldingRegister[arrayAdd];
			break;
		case 0x81A: /* mains nominal voltage*/
			mains_nominal_voltage1 = HoldingRegister[arrayAdd];
			break;
		case 0x81B: /* number of battery cells */
			n_of_battery_cells1 = HoldingRegister[arrayAdd];
			break;
		case 0x81C: /* battery capacity */
			battery_capacity1 = HoldingRegister[arrayAdd];
			break;
		case 0x81D: /* current limit rate */
			current_limit_rate1 = HoldingRegister[arrayAdd];
			break;		
		case 0x81E: /* deep dischage voltage*/
			deep_dischage_voltage1 = HoldingRegister[arrayAdd];
			break;
		case 0x81F: /* low voltage alarm*/
			low_voltage_alarm1 = HoldingRegister[arrayAdd];
			break;
		case 0x820: /* low voltage alarm*/
			quick_voltage1 = HoldingRegister[arrayAdd];
			break;
		case 0x821: /* float voltage */
			ref_float_voltage1 = HoldingRegister[arrayAdd];
			break;
		case 0x822: /* mains volt tol */
			mains_voltage_tolerance1 = HoldingRegister[arrayAdd];
			break;
		case 0x823: /* mains volt */
			mains_voltage1 = HoldingRegister[arrayAdd];
			break;
	}

}

void ModbusRwDataUpdate(void)
{
	


}


#pragma CODE_SECTION(Protection,"ramfuncs2");
void Protection(void){
	//  IDFR                                   IDFS                                   IDFT                                   IDF4
	//if(GpioDataRegs.GPADAT.bit.GPIO12 == 0  || GpioDataRegs.GPADAT.bit.GPIO13 == 0 || GpioDataRegs.GPADAT.bit.GPIO14 == 0 || GpioDataRegs.GPADAT.bit.GPIO15 == 0)
	if(EPwm5Regs.TZFLG.bit.OST == 1)
		UPS.Rectifier.Status.IDF = 1;
	else
		UPS.Rectifier.Status.IDF = 0;
	
	if(GpioDataRegs.GPCDAT.bit.GPIO67==0)
        UPS.Rectifier.Status.On_K3=1;	//k3 connects neutral wire to line wire
    else
        UPS.Rectifier.Status.On_K3=0;	 //k3 connects neutral wire to line wire

    if(GpioDataRegs.GPCDAT.bit.GPIO68==0)
        UPS.Rectifier.Status.On_K2=1;	//K2 bypasas softstart
    else
        UPS.Rectifier.Status.On_K2=1;	//k2 bypass softstart

//    if(UPS.Rectifier.Status.On_K2==1 && UPS.Rectifier.Status.On_K3==1)
//        UPS.Rectifier.Status.ContactorFault=1;
//    else
//        UPS.Rectifier.Status.ContactorFault=0;


    if(GpioDataRegs.GPBDAT.bit.GPIO54 == 0) 
        UPS.Inverter.Status.FaultCommand=1;	
    else
        UPS.Inverter.Status.FaultCommand=0;
				
	if(GpioDataRegs.GPCDAT.bit.GPIO66 == 0)
		UPS.Rectifier.Status.OverTemp=1;
	else
		UPS.Rectifier.Status.OverTemp=0;
		
	if(fabs(V_DCP-V_DCN) > MAX_UNBALANCE_VOLTAGE)
		UPS.Rectifier.Status.DC_UnbalanceVoltage=1;
	else	
		UPS.Rectifier.Status.DC_UnbalanceVoltage=0;		
	
	if(AdcMirror.ADCRESULT11 > 2300 || AdcMirror.ADCRESULT11 < 1700)
		UPS.Rectifier.Status.ProcessorFault=1;
	else	
		UPS.Rectifier.Status.ProcessorFault=0;		
		
		
			
		
//--------------------------------------------------Rectifire Faults----------------------------------------------------------------------		
	if(UPS.Rectifier.Status.On==1){
			
		
		if(Flag_Start > 5 && ((I_R > RST_Rectifier_ShortCircuit) || (I_S > RST_Rectifier_ShortCircuit) || (I_T > RST_Rectifier_ShortCircuit) || (I_R < -RST_Rectifier_ShortCircuit) || (I_S < -RST_Rectifier_ShortCircuit) || (I_T < -RST_Rectifier_ShortCircuit)))
			UPS.Rectifier.Status.ShortCircuit=1;
		else
			UPS.Rectifier.Status.ShortCircuit=0;
		
		
		if(V_DC > DC_Rectifier_OverVoltage && UPS.Inverter.Status.On == 1)
			UPS.Rectifier.Status.DC_OverVoltage=1;
		else	
			UPS.Rectifier.Status.DC_OverVoltage=0;
			

		if(V_DC < 0.95*VDC_Start && UPS.Inverter.Status.On == 1)
			UPS.Rectifier.Status.DC_UnderVoltage=1;
		else 
			UPS.Rectifier.Status.DC_UnderVoltage=0;	

		if(V_DC > DC_MAX_VOLTAGE)
			UPS.Rectifier.Status.DC_MaxVoltage=1;
		else
			UPS.Rectifier.Status.DC_MaxVoltage=0;
			
		if(V_DC < DC_LowVoltage && UPS.Inverter.Status.On == 1)
			UPS.Rectifier.Status.DC_MinVoltage=1;
		else	
			UPS.Rectifier.Status.DC_MinVoltage=0;		
	}	
			
//================================================================================================================================================

//--------------------------------------------------------------Buck Faults-----------------------------------------------------------------			
	if(UPS.Buck.Status.On==1){						
		if(V_DC_BAT > battery_overvoltage)
			UPS.Buck.Status.Bat_OverVoltage=1;
		else 
			UPS.Buck.Status.Bat_OverVoltage=0;		

		if(I_DC > I_Buck_OverCurrent  || I_DC < -I_Buck_OverCurrent)	
			UPS.Buck.Status.OverCurrent=1;
		else
			UPS.Buck.Status.OverCurrent=0;
				
		if(I_DC > I_Buck_MaxCurrent || I_DC < -I_Buck_MaxCurrent)	
			UPS.Buck.Status.MaxCurrent=1;
		else
			UPS.Buck.Status.MaxCurrent=0;					
	}	
//===========================================================================================================================================
//-----------------------------------------------------Boost Faults-----------------------------------------------------------------		
	if(UPS.Boost.Status.On==1){

		if(I_DC > I_Boost_OverCurrent || I_DC < -I_Boost_OverCurrent)	
			UPS.Boost.Status.OverCurrent=1;
		else
			UPS.Boost.Status.OverCurrent=0;	
			
		if(I_DC > I_Boost_MaxCurrent || I_DC < -I_Boost_MaxCurrent)	
			UPS.Boost.Status.MaxCurrent=1;
		else
			UPS.Boost.Status.MaxCurrent=0;		
	
		if(V_DC < deep_dischage_voltage)
			UPS.Boost.Status.VBatt_End=1;
		else	
			UPS.Boost.Status.VBatt_End=0;	
		
		if(V_DC < DC_LowVoltage)
			UPS.Boost.Status.DC_MinVoltage=1;
		else	
			UPS.Boost.Status.DC_MinVoltage=0;	
			
		if((V_DC > DC_MAX_VOLTAGE))
			UPS.Boost.Status.DC_MaxVoltage=1;
		else	
			UPS.Boost.Status.DC_MaxVoltage=0;	
	}


	
//-----------------------------------------------Rectifier Protection Counters------------------------------------------------	
	
	if(UPS.Rectifier.Status.On == 1 && Flag_RecDanger < 1){ 
		if(UPS.Rectifier.Status.ShortCircuit || UPS.Rectifier.Status.DC_MaxVoltage || UPS.Rectifier.Status.DC_MinVoltage)
			Flag_RecDanger++;  
		else
			Flag_RecDanger=0;	  	
	}
	
	if(UPS.Rectifier.Status.On == 1 && Flag_RecFault < 2){
		if(UPS.Rectifier.Status.DC_UnderVoltage || UPS.Rectifier.Status.DC_OverVoltage)
			Flag_RecFault++;  
		else
			Flag_RecFault=0;	  	
	}
	
	
	if(UPS.Rectifier.Status.On == 1 && Flag_MainsOff1 < QSAMPLES ){ 
		if(fabs(V_R) < THRESHOLD_MAIN_OFF)
			 Flag_MainsOff1++;  
		else
			 Flag_MainsOff1=0;	  	
	}
	
	if(UPS.Rectifier.Status.On == 1 && Flag_MainsOff2 < QSAMPLES ){ 
		if(fabs(V_S) < THRESHOLD_MAIN_OFF)
			 Flag_MainsOff2++;  
		else
			 Flag_MainsOff2=0;	  	
	}
	
	if(UPS.Rectifier.Status.On == 1 && Flag_MainsOff3 < QSAMPLES ){ 
		if(fabs(V_T) < THRESHOLD_MAIN_OFF)
			 Flag_MainsOff3++;  
		else
			 Flag_MainsOff3=0;	  	
	}		
//===================================================================================================================================================	
//---------------------------------------------------------------------Boost Protection Counters-----------------------------------------------------------		
	if(UPS.Boost.Status.On == 1 && Flag_BoostDanger < 1){
		if(UPS.Boost.Status.MaxCurrent || UPS.Boost.Status.DC_MaxVoltage || UPS.Boost.Status.DC_MinVoltage || UPS.Balancer.Status.ShortCircuit)
			Flag_BoostDanger++;   	
		else 
			Flag_BoostDanger=0;
	}
	
	if(UPS.Boost.Status.On == 1 && Flag_BoostFault < 12000){
		if(UPS.Boost.Status.OverCurrent || UPS.Boost.Status.VBatt_End)
			Flag_BoostFault++;   	
		else 
			Flag_BoostFault=0;
	}
//=======================================================================================================================		
//-----------------------------------------------Buck Protection Counters------------------------------------------------	
	
	if(UPS.Buck.Status.On == 1 && Flag_BuckDanger < 3){
		if(UPS.Buck.Status.MaxCurrent)
			Flag_BuckDanger=0;  //Shut down Buck 	
		else
			Flag_BuckDanger=0;				
	}

	if(UPS.Buck.Status.On == 1 && Flag_BuckFault < 12000){
		if(UPS.Buck.Status.OverCurrent || UPS.Buck.Status.Bat_OverVoltage)
			Flag_BuckFault++;  //Shut down Buck 	
		else
			Flag_BuckFault=0;				
	}
//=======================================================================================================================		

	
//=======================================================================================================================		
//-----------------------------------------------Common Protection Counters------------------------------------------------			
	if(Flag_InvFault < 1){ 
		if(UPS.Inverter.Status.FaultCommand)
			Flag_InvFault++;  
		else
			Flag_InvFault=0;	  	
	}
	
	if(Flag_IDF < 1){ 
		if(UPS.Rectifier.Status.IDF)
			Flag_IDF++;  
		else
			Flag_IDF=0;	  	
	}
	  
    if(Flag_OverTemp < 60000){
    	if(UPS.Rectifier.Status.OverTemp)
			Flag_OverTemp++;
		else
			Flag_OverTemp=0;	
	}
	if(Flag_ContactorFault < 10){
		if(UPS.Rectifier.Status.ContactorFault)
			Flag_ContactorFault++;
		else
			Flag_ContactorFault=0;		
	}
	
	if(Flag_UnbalanceVoltageFault < 2){
		if(UPS.Rectifier.Status.DC_UnbalanceVoltage)
			Flag_UnbalanceVoltageFault++;
		else
			Flag_UnbalanceVoltageFault=0;	
	}
	
	if(Flag_offset_error < 1){
		if(UPS.Rectifier.Status.ProcessorFault)
			Flag_offset_error++;
		else
			Flag_offset_error=0;	
	}	
	
	
					
//---------------------------------------------------------Shutdown System-------------------------------------------
 
	if(Flag_BoostDanger >= 1 || Flag_BoostFault >= 12000){	//Shut down system completely
		UPS.Boost.Status.Fault=1;
		Shut_down_system();
	}
	
	if(Flag_BuckDanger >= 3 || Flag_BuckFault >= 12000){	//Shut down buck
		UPS.Buck.Status.Fault=1;
		Shut_down_buck();
	}	
	
	if(Flag_RecDanger >= 1 || Flag_OverTemp >= 60000 || Flag_ContactorFault >= 10 || Flag_IDF >= 1 || Flag_UnbalanceVoltageFault >= 2 || Flag_offset_error >= 1){
		UPS.Rectifier.Status.Fault=1;
		Shut_down_system();
	}

	if(UPS.Inverter.Status.On == 0 && UPS.Rectifier.Status.PG == 0){
//		UPS.Rectifier.Status.Fault=1;
//		Shut_down_system();
	}
	
	if(Flag_InvFault >= 1 ){	//Shutdown System
		UPS.Inverter.Status.InverterFault=1;
		Shut_down_system();
	}
	


 }


void Reset_System(void){
	
	IO_Setup(); 
	

	 
	DutyR=0;
	DutyS=0;
	DutyT=0;
	Duty_Leg4=-1.0;
	
	Flag_Start=0,Flag_OverTemp=0;
	Counter_Start=0,Counter_DC_OK=0,Counter_Buck_Start=0,Counter_Boost_Start=0,Counter_Boost_Off=0,Flag_BoostFault=0,Flag_BoostDanger=0 ,PowerGood=0, Flag_Auto=0;
	Flag_BatDisConnected=0, Flag_BatConnected=0, Flag_MainsOff1=0, Flag_MainsOff2=0, Flag_MainsOff3=0, Flag_BuckFault=0, Flag_BuckDanger=0, Flag_RecFault=0, Flag_InvFault=0, Flag_RecDanger=0, Emergency_Flag=0,Flag_IDF=0,Restart_System=0,PowerBad=0;
	Flag_ZCD=0, Counter_Sequence_OK = 0, Flag_ContactorFault = 0,SystemIsReady = 0,Flag_PowerOk=0;
	V_R=0.0, V_S=0.0, V_T=0.0, I_R=0.0, I_S=0.0, I_T=0.0, V_DC=0.0, V_DCP=0.0,V_DCN=0.0,V_DC_OUT=0.0,V_DC_BAT=0.0,I_DC =0.0,K=0.0;
	Vrms_R=0.0, Vrms_S=0.0, Vrms_T=0.0, Irms_R=0.0, Irms_S=0.0, Irms_T=0.0, V_RCha=0.0;
	VDC_Start=0,Step_SoftStart=0,VBuck_Start=0,SoftStart_Buck=0,SoftStart_Boost=0;
	vrms_mains_min = 0.0, vrms_mains_max = 0.0, battery_overvoltage = 0.0;
	ac_fail_time1 = 0, quick_time1 = 0, low_voltage_alarm1 = 0, quick_voltage1 = 0;
	
	Flag_Boost_CLearTrip=0, Flag_Rectifier_ClearTrip=0, Flag_Buck_ClearTrip=0, Flag_Balancer_ClearTrip=0, Counter_AfterReset = 0, Delay_Null_Contactor=0 , Counter_TurnOn=0, Flag_ManualShutDown = 0;
	OnCommand = 0, SystemIsHot = 0, Counter_Delay_Contactor=0, Counter_Delay_Reset=0, Delay_Mains_Contactor=0, test1 = 0,Emergency_Flag1=0, Counter_Toggle = 0, Flag_Battery_Ok = 0;
	Flag_offset_error=0, Flag_UnbalanceVoltageFault =0, Flag_SystemStart = 0,offset_avg_counter = 0,Flag_StratUp = 0, State_AutoReset = 0, Counter_State1_AutoReset =0, Counter_State2_AutoReset=0,Counter_State3_AutoReset=0;
	offset= 0,offset_sum = 0,offset_avg=0;

	Kp_test = 1.0, Ki_test = 220.0;
	//---------------------Mimic Commands Initialization------------------------------------




	//------------------------RMS Module Initialization-----------------------------------------
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_R);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_S);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsV_T);
	
	sine_mainsV_R.SampleFreq = SAMPLING_FREQ;
	sine_mainsV_S.SampleFreq = SAMPLING_FREQ;                  
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
	

//===================================================================================
//	SINEANALYZER_DIFF_wPWR_F_init(&sine_spllV_R);
//	sine_spllV_R.SampleFreq = SAMPLING_FREQ;
//	sine_spllV_R.nsamplesMin=MIN_SAMPLES;
//	sine_spllV_R.nsamplesMax=MAX_SAMPLES;

//------------------------------------------PLL Module Initialization---------------------------------
	SPLL_1ph_SOGI_F_init(50, ((float32)(1.0/SAMPLING_FREQ)), &spll1); 
	SPLL_1ph_SOGI_F_coeff_update(((float32)(1.0/SAMPLING_FREQ)), (float32)(2*3.14159*50), &spll1);
	//spll1.lpf_coeff.B0_lf=(float32)(223.2095);
    //spll1.lpf_coeff.B1_lf=(float32)(-221.111);
    
    spll1.Umax =2.0;
    spll1.Umin= -2.0;

//	SPLL_1ph_SOGI_F_init(50, ((float32)(1.0/SAMPLING_FREQ)), &spll2); 
//	SPLL_1ph_SOGI_F_coeff_update(((float32)(1.0/SAMPLING_FREQ)), (float32)(2*3.14159*50), &spll2);
	//spll2.lpf_coeff.B0_lf=(float32)(223.2095);
    //spll2.lpf_coeff.B1_lf=(float32)(-221.111);	
	
//	SPLL_1ph_SOGI_F_init(50, ((float32)(1.0/SAMPLING_FREQ)), &spll3); 
//	SPLL_1ph_SOGI_F_coeff_update(((float32)(1.0/SAMPLING_FREQ)), (float32)(2*3.14159*50), &spll3);
	//spll3.lpf_coeff.B0_lf=(float32)(223.2095);
    //spll3.lpf_coeff.B1_lf=(float32)(-221.111);
//------------------------------------------PI Regulators Initialization---------------------------------
	CNTL_PI_F_init(&cntl_pi10); //PI Regulator Related to Battery Current
	cntl_pi10.Ki = (Ki_IDC/SAMPLING_FREQ);    //            177.0/12000
	cntl_pi10.Kp = (Kp_IDC);
	cntl_pi10.Umax=(1.2);
	cntl_pi10.Umin=(-1.2);

	CNTL_PI_F_init(&cntl_pi1); //PI Regulator Related to DC Link Voltage
	cntl_pi1.Ki = (Ki_DC/SAMPLING_FREQ);    //            177.0/12000
	cntl_pi1.Kp = (Kp_DC);
	cntl_pi1.Umax=(1.2);
	cntl_pi1.Umin=(-1.2);
	
	CNTL_PI_F_init(&cntl_pi2);  //PI Regulator for phase R
	cntl_pi2.Ki = (Ki_a/SAMPLING_FREQ);    // 47/12000 or 0.0
	cntl_pi2.Kp = (Kp_a);
	cntl_pi2.Umax=(1.0);
	cntl_pi2.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi3);  //PI Regulator for phase S
	cntl_pi3.Ki = (Ki_b/SAMPLING_FREQ);    //47.0/12000
	cntl_pi3.Kp = (Kp_b );
	cntl_pi3.Umax=(1.0);
	cntl_pi3.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi4);  //PI Regulator for phase T
	cntl_pi4.Ki = (Ki_c/SAMPLING_FREQ);    //47/12000
	cntl_pi4.Kp = (Kp_c);
	cntl_pi4.Umax=(1.0);
	cntl_pi4.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi5);  //PI Regulator for Buck Voltage Loop
	cntl_pi5.Ki =(Ki_Vbuck/SAMPLING_FREQ);
	cntl_pi5.Kp =(Kp_Vbuck) ;
	cntl_pi5.Umax=(current_limit_rate);  //1.7/65;
	cntl_pi5.Umin=(-current_limit_rate);  //1.7/65
	
	CNTL_PI_F_init(&cntl_pi6); //PI Regulator for Buck Current Loop
	cntl_pi6.Ki =(Ki_Ibuck/SAMPLING_FREQ) ;
	cntl_pi6.Kp =(Kp_Ibuck) ;
	cntl_pi6.Umax=(1.0);
	cntl_pi6.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi7);  //PI Regulator for Boost Voltage Loop
	cntl_pi7.Ki =(Ki_Vboost/SAMPLING_FREQ);
	cntl_pi7.Kp =(Kp_Vboost) ;
	cntl_pi7.Umax=(1.5);
	cntl_pi7.Umin=(-1.5);
	
	CNTL_PI_F_init(&cntl_pi8);  //PI Regulator for Boost Current Loop
	cntl_pi8.Ki =(Ki_Iboost/SAMPLING_FREQ) ;
	cntl_pi8.Kp =(Kp_Iboost) ;
	cntl_pi8.Umax=(1.0);
	cntl_pi8.Umin=(-1.0);
	
	CNTL_PI_F_init(&cntl_pi9);  //PI Regulator for Balancing DC Link
	cntl_pi9.Ki =(Ki_Balancing/SAMPLING_FREQ) ;
	cntl_pi9.Kp =(Kp_Balancing) ;
	cntl_pi9.Umax=(1.0);
	cntl_pi9.Umin=(-1.0);
	
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
	
//	CNTL_2P2Z_F_COEFFS_init(&Coeff1_h2);
//	Coeff1_h2.Coeff_B2=-0.0000416595;
//	Coeff1_h2.Coeff_B1=0.0;
//	Coeff1_h2.Coeff_B0=0.0000416595;
//	Coeff1_h2.Coeff_A2=-1.0;
//	Coeff1_h2.Coeff_A1=1.99726;                       
//	Coeff1_h2.Min=-1;
//	Coeff1_h2.Max=1;
//	CNTL_2P2Z_F_VARS_init(&Vars1_h2);
//		
//	CNTL_2P2Z_F_COEFFS_init(&Coeff2_h2);
//	Coeff2_h2.Coeff_B2=-0.0000416595;
//	Coeff2_h2.Coeff_B1=0.0;
//	Coeff2_h2.Coeff_B0=0.0000416595;
//	Coeff2_h2.Coeff_A2=-1.0;
//	Coeff2_h2.Coeff_A1=1.99726;
//	Coeff2_h2.Min=-1;
//	Coeff2_h2.Max=1;
//	CNTL_2P2Z_F_VARS_init(&Vars2_h2);
//	
//	CNTL_2P2Z_F_COEFFS_init(&Coeff3_h2);
//	Coeff3_h2.Coeff_B2=-0.0000416595;
//	Coeff3_h2.Coeff_B1=0.0;
//	Coeff3_h2.Coeff_B0=0.0000416595;
//	Coeff3_h2.Coeff_A2=-1.0;
//	Coeff3_h2.Coeff_A1=1.99726;
//	Coeff3_h2.Min=-1;
//	Coeff3_h2.Max=1;
//	CNTL_2P2Z_F_VARS_init(&Vars3_h2);	
	
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

//---------------------- Signal Generator module initialization  -----------------         
//	sgen.offset=0;
//	sgen.gain=0x7fff;	// gain=1 in Q15
//	sgen.freq=3579;	// freq = (Required Freq/Max Freq)*2^15
		          		// = (50/137.33)*2^15 = 11930 
		          		
//	sgen.step_max=3000;      // Max Freq= (step_max * sampling freq)/65536 
//	sgen.alpha=0;           // phase_norm =(pi/4/(2*pi))*2^16=8192   
		
		                      /* So call step_max is normalized fmax in Q16 format             
		                       step_max=fmax/fs*65536										
		                        f/fmax*2^15=freq	*/	
		                        									
		

	
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
//    dlog_4ch1.size = 120;//120 
//    dlog_4ch1.pre_scalar =20;//20  //for observing 5 cycle of 50 hz signals
//    dlog_4ch1.trig_value = 0; 
//    dlog_4ch1.status = 1;   
	
	
//------------------------------------------------------------------------------------------	

//-------------------------MIMIC Setting Initialization---------------------------------------












//------------------------------------------------------------------------------------------
   UPS_init();
   UPS.Rectifier.Status.PG=1;
   
   	EALLOW;
		EPwm5Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM5
    EDIS;
                    
	PWM_Drive(pwm1,0.0,0.0,0.0,0.0); 
}

void Initial_Settings(void)
{
	if(TEST_MODE){
		battery_type1 = 0;
		n_of_battery_cells1 = 6;
		current_limit_rate1 = 10;
		battery_capacity1 = 100;
		ref_float_voltage1 = 220;
		mains_voltage_tolerance1 = 60;
		mains_voltage1= 22;
		mains_nominal_voltage1 = 22;
		deep_dischage_voltage1 = 160;

	}
	else{
		battery_type1 = 0;
		n_of_battery_cells1 = 40;
		current_limit_rate1 = 10;
		battery_capacity1 = 100;
		ref_float_voltage1 = 220;
		mains_voltage_tolerance1 = 20;
		mains_voltage1= 220;
		mains_nominal_voltage1 = 220;
		deep_dischage_voltage1 = 185;
	}





}








