

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
void Turn_on_bypass(void);
void Turn_on_STS(void);
extern void IO_Setup(void);




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
SINEANALYZER_DIFF_wPWR_F sine_mainsJ45;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ47;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ49;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ51;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ52;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ54;



SINEANALYZER_DIFF_wPWR_F sine_mainsJ22;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ20;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ18;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ16;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ14;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ12;
SINEANALYZER_DIFF_wPWR_F sine_mainsJ23;

extern struct UPS_V UPS;

float32 DutyR=0.6;
float32 DutyS=0;
float32 DutyT=-0.4;
float32 Duty_STS=-0.8;




float32 J45, J47, J49, J51, J52, J54;
float32 J22, J20, J18, J16, J14, J12, J23;

float32 Vrms_J45, Vrms_J47, Vrms_J49, Vrms_J51, Vrms_J52, Vrms_J54;
float32 Irms_J22, Irms_J20, Irms_J18, Irms_J16, Irms_J14, Irms_J12, Irms_J23;

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


	IO_Setup();
	//------------------------RMS Module Initialization-----------------------------------------
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ45);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ47);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ49);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ51);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ52);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ54);

	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ22);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ20);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ18);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ16);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ14);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ12);
	SINEANALYZER_DIFF_wPWR_F_init(&sine_mainsJ23);

	sine_mainsJ45.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ47.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ49.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ51.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ52.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ54.SampleFreq = SAMPLING_FREQ;

	sine_mainsJ22.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ20.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ18.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ16.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ14.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ12.SampleFreq = SAMPLING_FREQ;
	sine_mainsJ23.SampleFreq = SAMPLING_FREQ;

	sine_mainsJ45.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ47.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ49.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ51.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ52.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ54.nsamplesMin=MIN_SAMPLES;

	sine_mainsJ22.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ20.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ18.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ16.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ14.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ12.nsamplesMin=MIN_SAMPLES;
	sine_mainsJ23.nsamplesMin=MIN_SAMPLES;
	                                               // 12000/50=240 ==> 200<240<280
	sine_mainsJ45.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ47.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ49.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ51.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ52.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ54.nsamplesMax=MAX_SAMPLES;

	sine_mainsJ22.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ20.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ18.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ16.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ14.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ12.nsamplesMax=MAX_SAMPLES;
	sine_mainsJ23.nsamplesMax=MAX_SAMPLES;

	sine_mainsJ45.Threshold=20.0;
	sine_mainsJ47.Threshold=20.0;
	sine_mainsJ49.Threshold=20.0;
	sine_mainsJ51.Threshold=20.0;
	sine_mainsJ52.Threshold=20.0;
	sine_mainsJ54.Threshold=20.0;

	sine_mainsJ22.Threshold=20.0;
	sine_mainsJ20.Threshold=20.0;
	sine_mainsJ18.Threshold=20.0;
	sine_mainsJ16.Threshold=20.0;
	sine_mainsJ14.Threshold=20.0;
	sine_mainsJ12.Threshold=20.0;
	sine_mainsJ23.Threshold=20.0;


   
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


	Turn_on_system();
//	Turn_on_bypass();
	Turn_on_STS(); //baraye inverter

      
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
	
	
	



		
		

//-----------------------Reading ADC Chanels----------------------------------

	J45=(AdcMirror.ADCRESULT0*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J47=(AdcMirror.ADCRESULT1*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J49=(AdcMirror.ADCRESULT2*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J51=(AdcMirror.ADCRESULT3*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J52=(AdcMirror.ADCRESULT4*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J54=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_VAC;

	J22=(AdcMirror.ADCRESULT0*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J20=(AdcMirror.ADCRESULT1*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J18=(AdcMirror.ADCRESULT2*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J16=(AdcMirror.ADCRESULT3*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J14=(AdcMirror.ADCRESULT4*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J12=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_VAC;
	J23=(AdcMirror.ADCRESULT5*0.00024414-0.5)*2*GAIN_CAL_VAC;


	//--------------------RMS Module Implementation--------------------------------------
	    sine_mainsJ45.Vin=J45*311.0;
	    sine_mainsJ47.Vin=J47*311.0;
	    sine_mainsJ49.Vin=J49*311.0;
	    sine_mainsJ51.Vin=J51*311.0;
	    sine_mainsJ52.Vin=J52*311.0;
	    sine_mainsJ54.Vin=J54*311.0;

	    sine_mainsJ22.Vin=J22*311.0;
	    sine_mainsJ20.Vin=J20*311.0;
	    sine_mainsJ18.Vin=J18*311.0;
	    sine_mainsJ16.Vin=J16*311.0;
	    sine_mainsJ14.Vin=J14*311.0;
	    sine_mainsJ12.Vin=J12*311.0;
	    sine_mainsJ23.Vin=J23*311.0;


	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ45);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ47);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ49);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ51);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ52);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ54);

	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ22);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ20);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ18);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ16);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ14);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ12);
	    SINEANALYZER_DIFF_wPWR_F_FUNC(&sine_mainsJ23);

	    Vrms_J45=sine_mainsJ45.Vrms;
	    Vrms_J47=sine_mainsJ47.Vrms;
	    Vrms_J49=sine_mainsJ49.Vrms;
	    Vrms_J51=sine_mainsJ51.Vrms;
	    Vrms_J52=sine_mainsJ52.Vrms;
	    Vrms_J54=sine_mainsJ54.Vrms;

	    Irms_J22=sine_mainsJ22.Vrms;
	    Irms_J20=sine_mainsJ20.Vrms;
	    Irms_J18=sine_mainsJ18.Vrms;
	    Irms_J16=sine_mainsJ16.Vrms;
	    Irms_J14=sine_mainsJ14.Vrms;
	    Irms_J12=sine_mainsJ12.Vrms;
	    Irms_J23=sine_mainsJ23.Vrms;

    	


	
	


 

    


 





    		        	                                  
	    





	    	









	    	
	 

	   	








    


//-----------------------PWM Module Implementation-----------------------------------------	         
		PWM_Drive(pwm1,DutyR,DutyS,DutyT,Duty_STS);	// Calculate the new PWM compare values	
		
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













