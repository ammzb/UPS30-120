

#ifndef PWM_DEFINEVARIABLES_H_
#define PWM_DEFINEVARIABLES_H_


#include "f2833xbmsk.h"
/*----------------------------------------------------------------------------
Initialization constant for the F280X Time-Base Control Registers for PWM Generation. 
Sets up the timer to run free upon emulation suspend, count up-down mode
prescaler 1.
----------------------------------------------------------------------------*/
#define PWM_INIT_STATE ( SOFT_STOP_FLAG  +         \
                         PRDLD_SHADOW  +       \
                         TIMER_CNT_UPDN +         \
                         HSPCLKDIV_PRESCALE_X_1 + \
                         CLKDIV_PRESCALE_X_1  +   \
                         PHSDIR_CNT_UP    +       \
                         CNTLD_DISABLE )

/*----------------------------------------------------------------------------
Initialization constant for the F280X Compare Control Register. 
----------------------------------------------------------------------------*/
#define CMPCTL_INIT_STATE ( LOADAMODE_PRD + \
                            LOADBMODE_PRD + \
                            SHDWAMODE_SHADOW + \
                            SHDWBMODE_SHADOW )

/*----------------------------------------------------------------------------
Initialization constant for the F280X Action Qualifier Output A Register. 
----------------------------------------------------------------------------*/
#define AQCTLA_INIT_STATE ( CAD_SET + CAU_CLEAR )

/*----------------------------------------------------------------------------
Initialization constant for the F280X Dead-Band Generator registers for PWM Generation. 
Sets up the dead band for PWM and sets up dead band values.
----------------------------------------------------------------------------*/
#define DBCTL_INIT_STATE  (BP_ENABLE + POLSEL_ACTIVE_LO_CMP )

#define DBCNT_INIT_STATE  600  // 200 counts = 1.33 usec (delay)  (for TBCLK = SYSCLK/1)

/*----------------------------------------------------------------------------
Initialization constant for the F2833X PWM Chopper Control register for PWM Generation. 
----------------------------------------------------------------------------*/
#define  PCCTL_INIT_STATE  CHPEN_DISABLE

/*----------------------------------------------------------------------------
Initialization constant for the F2833X Trip Zone Select Register 
----------------------------------------------------------------------------*/
#define  TZSEL_INIT_STATE  (ENABLE_TZ1_OST + ENABLE_TZ2_OST + ENABLE_TZ3_OST) //80KVA
              
/*----------------------------------------------------------------------------
Initialization constant for the F2833X Trip Zone Control Register 
----------------------------------------------------------------------------*/
#define  TZCTL_INIT_STATE ( TZA_FORCE_LO + TZB_FORCE_LO )
                            
                            
                                                                 
/*-----------------------------------------------------------------------------
	Define the structure of the PWM Driver Object 
-----------------------------------------------------------------------------*/
typedef struct {   
        unsigned int PeriodMax;     // Parameter: PWM Half-Period in CPU clock cycles (Q0)
        int MfuncPeriod;    // Input: Period scaler (Q15) 
        float MfuncC1;        // Input: EPWM1 A&B Duty cycle ratio (Q15)
        float MfuncC2;        // Input: EPWM2 A&B Duty cycle ratio (Q15) 
        float MfuncC3;        // Input: EPWM3 A&B Duty cycle ratio (Q15)
        float MfuncC5;
		unsigned int PWM1out;
		unsigned int PWM2out;
		unsigned int PWM3out;
		unsigned int PWM5out;
        } PWMGEN ;    

/*-----------------------------------------------------------------------------
	Define a PWMGEN_handle
-----------------------------------------------------------------------------*/
typedef PWMGEN *PWMGEN_handle;

/*------------------------------------------------------------------------------
	Default Initializers for the F2833X PWMGEN Object 
------------------------------------------------------------------------------*/
#define F2833X_FC_PWM_GEN    { 1000,   \
                              0x7FFF, \
                              0x4000, \
                              0x4000, \
                              0x4000, \
							  0x4000, \
							  0x4000, \
							  0x4000, \
							  0x4000, \
							  0x4000, \
                             }
                             

#define PWMGEN_DEFAULTS 	F2833X_FC_PWM_GEN



#endif /*PWM_DEFINEVARIABLES_H_*/
