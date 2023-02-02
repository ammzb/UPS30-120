/* ==================================================================================
File name:        PWM_Setup.c                    
                    
Originator:	 Digital Control Systems Group
			 Texas Instruments
Description: Header file containing data type, object, macro definitions and 
			 initializers. 
			 Configures PWM1-3 for three phase inverters

Target: TMS320F2833x family
              
=====================================================================================
History:
-------------------------------------------------------------------------------------
 04-15-2010	Version 1.0
------------------------------------------------------------------------------------*/



#include "PWM_DefineVariables.h"
#include "DSP28x_Project.h"


/*------------------------------------------------------------------------------
	PWM Init & PWM Update Macro Definitions
------------------------------------------------------------------------------*/


void PWM_INIT(PWMGEN *v){
		 /* Init Timer-Base Control Register for EPWM1-EPWM3*/				
         EPwm1Regs.TBCTL.all = PWM_INIT_STATE;								
         EPwm2Regs.TBCTL.all = PWM_INIT_STATE;								
         EPwm3Regs.TBCTL.all = PWM_INIT_STATE;								
		 EPwm5Regs.TBCTL.all = PWM_INIT_STATE;														
	     /* Setup Sync*/													
         EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;      			
         EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			
         EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;       /* Pass through*/			
         EPwm5Regs.TBCTL.bit.SYNCOSEL = 0;		 /* Pass through*/							
																			
         /* Allow each timer to be sync'ed*/								
         EPwm1Regs.TBCTL.bit.PHSEN = 0;										
         EPwm2Regs.TBCTL.bit.PHSEN = 1;										
         EPwm3Regs.TBCTL.bit.PHSEN = 1;										
		 EPwm5Regs.TBCTL.bit.PHSEN = 1;										
																			
         /* Init Timer-Base Period Register for EPWM1-EPWM3*/				
         EPwm1Regs.TBPRD = v->PeriodMax;										
         EPwm2Regs.TBPRD = v->PeriodMax;										
         EPwm3Regs.TBPRD = v->PeriodMax;										
		 EPwm5Regs.TBPRD = v->PeriodMax;											
																			
         /* Init Timer-Base Phase Register for EPWM1-EPWM3*/				
         EPwm1Regs.TBPHS.half.TBPHS = 0;									
         EPwm2Regs.TBPHS.half.TBPHS = 0;									
         EPwm3Regs.TBPHS.half.TBPHS = 0;									
		 EPwm5Regs.TBPHS.half.TBPHS = 0;			
		 
		 	
		 EPwm1Regs.CMPA.half.CMPA=1000;	// PWM 1A - PhaseA 
	 	 EPwm2Regs.CMPA.half.CMPA=1000;	// PWM 2A - PhaseB
	     EPwm3Regs.CMPA.half.CMPA=1000;	// PWM 3A - PhaseC 
	     EPwm5Regs.CMPA.half.CMPA=1000;	// PWM 5A - Leg4
		 									     			
         /* Init Compare Control Register for EPWM1-EPWM3*/					
         EPwm1Regs.CMPCTL.all = CMPCTL_INIT_STATE;							
         EPwm2Regs.CMPCTL.all = CMPCTL_INIT_STATE;							
         EPwm3Regs.CMPCTL.all = CMPCTL_INIT_STATE;							
		 EPwm5Regs.CMPCTL.all = CMPCTL_INIT_STATE;																	
         /* Init Action Qualifier Output A Register for EPWM1-EPWM3*/		
         EPwm1Regs.AQCTLA.all = AQCTLA_INIT_STATE;							
         EPwm2Regs.AQCTLA.all = AQCTLA_INIT_STATE;							
         EPwm3Regs.AQCTLA.all = AQCTLA_INIT_STATE;							
		 EPwm5Regs.AQCTLA.all = (CAU_SET + CAD_CLEAR);
		 /* Init Action Qualifier Output B Register for EPWM5*/								
		 EPwm5Regs.AQCTLB.all = (CAU_SET + CAD_CLEAR);																		
         /* Init Dead-Band Generator Control Register for EPWM1-EPWM3*/		
         EPwm1Regs.DBCTL.all = DBCTL_INIT_STATE;							
         EPwm2Regs.DBCTL.all = DBCTL_INIT_STATE;							
         EPwm3Regs.DBCTL.all = DBCTL_INIT_STATE;							
		 EPwm5Regs.DBCTL.all = BP_DISABLE;	
//		 EPwm5Regs.DBCTL.bit.IN_MODE = 1;	
//		 EPwm5Regs.DBCTL.bit.POLSEL = 3;
//		 EPwm5Regs.DBCTL.bit.OUT_MODE = 3;												
         /* Init Dead-Band Generator for EPWM1-EPWM3*/						
         EPwm1Regs.DBFED = DBCNT_INIT_STATE;	//4 usec Delay							
         EPwm1Regs.DBRED = DBCNT_INIT_STATE;	//4 usec Delay							
         EPwm2Regs.DBFED = DBCNT_INIT_STATE;	//4 usec Delay							
         EPwm2Regs.DBRED = DBCNT_INIT_STATE;	//4 usec Delay							
         EPwm3Regs.DBFED = DBCNT_INIT_STATE;	//4 usec Delay							
         EPwm3Regs.DBRED = DBCNT_INIT_STATE;	//4 usec Delay
         EPwm5Regs.DBFED = 0;	//2 usec Delay							
         EPwm5Regs.DBRED = 0;	//2 usec Delay						
																			
         /* Init PWM Chopper Control Register for EPWM1-EPWM3*/				
         EPwm1Regs.PCCTL.all = PCCTL_INIT_STATE;							
         EPwm2Regs.PCCTL.all = PCCTL_INIT_STATE;							
         EPwm3Regs.PCCTL.all = PCCTL_INIT_STATE;
         EPwm5Regs.PCCTL.all = PCCTL_INIT_STATE;							
          																	
         EALLOW;     
                           /* Enable EALLOW */																				
         /* Init Trip Zone Select Register*/								
         EPwm1Regs.TZSEL.all = TZSEL_INIT_STATE;							
         EPwm2Regs.TZSEL.all = TZSEL_INIT_STATE;							
         EPwm3Regs.TZSEL.all = TZSEL_INIT_STATE;							
		// EPwm5Regs.TZSEL.all = TZSEL_INIT_STATE;
		 																
         /* Init Trip Zone Control Register*/									
		 EPwm1Regs.TZCTL.bit.TZA = 1;	
		 EPwm1Regs.TZCTL.bit.TZB = 1;
		 EPwm2Regs.TZCTL.bit.TZA = 1;	
		 EPwm2Regs.TZCTL.bit.TZB = 1;
		 EPwm3Regs.TZCTL.bit.TZA = 1;	
		 EPwm3Regs.TZCTL.bit.TZB = 1;	
//		 EPwm5Regs.TZCTL.bit.TZA = 1;
//		 EPwm5Regs.TZCTL.bit.TZB = 1;
		 													
         EDIS;                         /* Disable EALLOW*/			
         
	     // Force a one-shot trip event for PWM
	     EALLOW;
		 EPwm1Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM1
		 EPwm2Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM2
	     EPwm3Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM3
	     //EPwm5Regs.TZFRC.bit.OST = 1 ; // Forces a one-shot trip event for EPWM5
	     EPwm5Regs.AQSFRC.bit.RLDCSF=3;
	     EPwm5Regs.AQCSFRC.bit.CSFB=2;
	     EPwm5Regs.AQCSFRC.bit.CSFA=2;
	     EDIS;
	
   		 EALLOW;
         SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
         EDIS;

}





void PWM_Drive(PWMGEN *v,float32 DutyR,float32 DutyS,float32 DutyT,float32 Duty_Leg4)
{																		
																							
	float32 MPeriod;
	float32 Tmp;	
				
	v->MfuncC1=DutyR;			
	v->MfuncC2=DutyS;
	v->MfuncC3=DutyT;
	v->MfuncC5=Duty_Leg4;
																						
	MPeriod = (float32)(v->PeriodMax>>1);	                                                
																							
	EPwm1Regs.TBPRD = v->PeriodMax;													    	
	EPwm2Regs.TBPRD = v->PeriodMax;															
	EPwm3Regs.TBPRD = v->PeriodMax;	
	EPwm5Regs.TBPRD = v->PeriodMax;															
																							
/*Compute the compare A from the related duty cycle ratio */						
																							
	Tmp = MPeriod*v->MfuncC1;				                            					
	v->PWM1out = (Uint16)(Tmp+MPeriod);			                                            
																							
/*Compute the compare B from the related duty cycle ratio */						
																							
	Tmp = MPeriod*v->MfuncC2;				                            					
	v->PWM2out = (Uint16)(Tmp+MPeriod);			                                            
																							
/*Compute the compare from the related duty cycle ratio*/						
																							
	Tmp = MPeriod*v->MfuncC3;				                            					
	v->PWM3out = (Uint16)(Tmp+MPeriod);	
	
	Tmp = MPeriod*v->MfuncC5;				                            					
	v->PWM5out = (Uint16)(Tmp+MPeriod);	
	
	EPwm1Regs.CMPA.half.CMPA=v->PWM1out;	// PWM 1A - PhaseA 
	EPwm2Regs.CMPA.half.CMPA=v->PWM2out;	// PWM 2A - PhaseB
	EPwm3Regs.CMPA.half.CMPA=v->PWM3out;	// PWM 3A - PhaseC  
	EPwm5Regs.CMPA.half.CMPA=v->PWM5out;
}

