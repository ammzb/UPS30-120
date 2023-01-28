

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
extern void IO_Setup(void);

void Turn_on_system(void);


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
SINEANALYZER_DIFF_wPWR_F sine_mainsJ44;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ46;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ48;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ50;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ53;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ55;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ56;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ57;


SINEANALYZER_DIFF_wPWR_F sine_mainsJ21;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ19;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ17;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ15;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ13;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ11;


//-------------------------------Global Parameters Declaration----------------------------

extern struct UPS_V UPS;

float32 DutyR=0.6;
float32 DutyS=0.0;
float32 DutyT=-0.4;
float32 Duty_Leg4=0.0;


float32 J44, J46, J48, J50, J53, J55, J56, J57;
float32 J21, J19, J17, J15, J13, J11;

float32 Vrms_J44, Vrms_J46, Vrms_J48, Vrms_J50, Vrms_J53, Vrms_J55, Vrms_J56, Vrms_J57;
float32 Irms_J21, Irms_J19, Irms_J17, Irms_J15, Irms_J13, Irms_J11;



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




	IO_Setup();
	//------------------------RMS Module Initialization-----------------------------------------
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ44);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ46);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ48);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ50);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ53);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ55);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ56);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ57);

	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ21);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ19);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ17);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ15);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ13);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ11);

	sine_mainsJ44.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ46.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ48.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ50.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ53.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ55.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ56.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ57.SampleFreq = SAMPLING_FREQ;

	sine_mainsJ21.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ19.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ17.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ15.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ13.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ11.SampleFreq = SAMPLING_FREQ;

	sine_mainsJ44.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ46.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ48.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ50.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ53.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ55.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ56.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ57.nsamplesMin=MIN_SAMPLES;

	sine_mainsJ21.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ19.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ17.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ15.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ13.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ11.nsamplesMin=MIN_SAMPLES;
	                                               // 12000/50=240 ==> 200<240<280
	sine_mainsJ44.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ46.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ48.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ50.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ53.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ55.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ56.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ57.nsamplesMax=MAX_SAMPLES;

	sine_mainsJ21.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ19.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ17.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ15.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ13.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ11.nsamplesMax=MAX_SAMPLES;

	sine_mainsJ44.Threshold=20.0;
	sine_mainsJ46.Threshold=20.0;
	sine_mainsJ48.Threshold=20.0;
	sine_mainsJ50.Threshold=20.0;
	sine_mainsJ53.Threshold=20.0;
	sine_mainsJ55.Threshold=20.0;
	sine_mainsJ56.Threshold=20.0;
	sine_mainsJ57.Threshold=20.0;

	sine_mainsJ21.Threshold=20.0;
	sine_mainsJ19.Threshold=20.0;
	sine_mainsJ17.Threshold=20.0;
	sine_mainsJ15.Threshold=20.0;
	sine_mainsJ13.Threshold=20.0;
	sine_mainsJ11.Threshold=20.0;



   
   
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



	Turn_on_system();
      
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

     

	
	
	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C2
	C_Task_Ptr = &C2;	
	//-----------------

}

//----------------------------------------
void C2(void) //  SPARE
//----------------------------------------
{







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
	
	

 
//-----------------------Reading ADC Chanels----------------------------------
 	//offset=(int16)AdcMirror.ADCRESULT11 - 2048;
		J44=(AdcMirror.ADCRESULT0*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J46=(AdcMirror.ADCRESULT1*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J48=(AdcMirror.ADCRESULT2*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J50=(AdcMirror.ADCRESULT3*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J53=(AdcMirror.ADCRESULT4*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J55=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_VAC;
		J56=(AdcMirror.ADCRESULT6*0.00024414-0.5)*2*GAIN_CAL_VDC;
		J57=(AdcMirror.ADCRESULT7*0.00024414-0.5)*2*GAIN_CAL_VDC;

		J21=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_IAC;
		J19=(AdcMirror.ADCRESULT6*0.00024414-0.5)*2*GAIN_CAL_IAC;
		J17=(AdcMirror.ADCRESULT7*0.00024414-0.5)*2*GAIN_CAL_IAC;
		J15=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_IAC;
		J13=(AdcMirror.ADCRESULT6*0.00024414-0.5)*2*GAIN_CAL_IAC;
		J11=(AdcMirror.ADCRESULT7*0.00024414-0.5)*2*GAIN_CAL_IAC;


		//--------------------RMS Module Implementation--------------------------------------
		    sine_mainsJ44.Vin=J44*311.0;
		    sine_mainsJ46.Vin=J46*311.0;
		    sine_mainsJ48.Vin=J48*311.0;
		    sine_mainsJ50.Vin=J50*311.0;
		    sine_mainsJ53.Vin=J53*311.0;
		    sine_mainsJ55.Vin=J55*311.0;
		    sine_mainsJ56.Vin=J56*311.0;
		    sine_mainsJ57.Vin=J57*311.0;

		    sine_mainsJ21.Vin=J21*311.0;
		    sine_mainsJ19.Vin=J19*311.0;
		    sine_mainsJ17.Vin=J17*311.0;
		    sine_mainsJ15.Vin=J15*311.0;
		    sine_mainsJ13.Vin=J13*311.0;
		    sine_mainsJ11.Vin=J11*311.0;


		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ44);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ46);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ48);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ50);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ53);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ55);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ56);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ57);

		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ21);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ19);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ17);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ15);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ13);
		    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ11);


		    Vrms_J44=sine_mainsJ44.Vrms;
		    Vrms_J46=sine_mainsJ46.Vrms;
		    Vrms_J48=sine_mainsJ48.Vrms;
		    Vrms_J50=sine_mainsJ50.Vrms;
		    Vrms_J53=sine_mainsJ53.Vrms;
		    Vrms_J55=sine_mainsJ55.Vrms;
		    Vrms_J56=sine_mainsJ56.Vrms;
		    Vrms_J57=sine_mainsJ57.Vrms;

		    Irms_J21=sine_mainsJ21.Vrms;
		    Irms_J19=sine_mainsJ19.Vrms;
		    Irms_J17=sine_mainsJ17.Vrms;
		    Irms_J15=sine_mainsJ15.Vrms;
		    Irms_J13=sine_mainsJ13.Vrms;
		    Irms_J11=sine_mainsJ11.Vrms;
 
	





    
    


	GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;



//-----------------------PWM Module Implementation-----------------------------------------
 	         
	PWM_Drive(pwm1,DutyR,DutyS,DutyT,Duty_Leg4);	// Calculate the new PWM compare values	
	
		
//-----------------------------------------------------------------------------------------


	EPwm1Regs.ETCLR.bit.INT = 1;	// Clear INT flag for this timer


	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;	// Acknowledge this interrupt to receive more interrupts from group 3
    
}
	


void Turn_on_system(void){
 	GpioDataRegs.GPACLEAR.bit.GPIO10 = 1; // Enable RST PWM
	EALLOW;
		EPwm1Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM1
		EPwm2Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM2
		EPwm3Regs.TZCLR.bit.OST = 1 ; // Clear one-shot trip event for EPWM3
	EDIS;


	GpioDataRegs.GPACLEAR.bit.GPIO11 = 1; //Enable Leg4 PWM
	EALLOW;
		//EPwm5Regs.AQCSFRC.bit.CSFB=3; //Software Forcing is Disabled
		EPwm5Regs.AQCSFRC.bit.CSFA=3; //Software Forcing is Disabled
	EDIS;

 }

 

































