
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#define ADC_usDELAY 5300L


void ADC_Setup(void)
	{
	
	 																								
	EALLOW;																						
	SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;														
	ADC_cal();																					
	EDIS;																						
																								
					
    																							
 	AdcRegs.ADCTRL1.bit.ACQ_PS = 3;
	AdcRegs.ADCTRL1.bit.CPS = 1;																
	AdcRegs.ADCTRL3.bit.ADCCLKPS =  1;
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;//0        /* 0x0 Dual Sequencer Mode, 0x1 Cascaded Mode*/	
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 0x0;														
	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 0x1;															
	//AdcRegs.ADCTRL2.bit.RST_SEQ2 = 0x1;															
	AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1=0x1; /* enable SOC from EPWMA trigger*/  
	
	AdcRegs.ADCTRL3.bit.ADCPWDN=0;
	AdcRegs.ADCTRL3.bit.ADCBGRFDN=0;
	AdcRegs.ADCOFFTRIM.bit.OFFSET_TRIM-=30;
	AdcRegs.ADCREFSEL.bit.REF_SEL= 1;        //External reference, 2.048 V on ADCREFIN


	AdcRegs.ADCTRL3.bit.ADCPWDN=1;
	AdcRegs.ADCTRL3.bit.ADCBGRFDN=3;
	DELAY_US(ADC_usDELAY);         /* Delay before converting ADC channels*/               
																								
	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0;	/* ChSelect: ADC B4-> Phase A Current */            
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 2;	/* ChSelect: ADC B6-> Phase B Current */
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 4;	/* ChSelect: ADC A1-> DC Bus Voltage */
	AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 6;	/* ChSelect: ADC B3-> V Phase A */
	AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 1;	/* ChSelect: ADC B2-> V Phase B */
	AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 3;	/* ChSelect: ADC B1-> V Phase C */
	AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 5;    /* ChSelect: ADC A4-> Low Side DC Bus Return Cur.*/
	AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 7;
	AdcRegs.ADCCHSELSEQ3.bit.CONV08 = 5;	/* ChSelect: ADC A5-> Battery Voltage */
	AdcRegs.ADCCHSELSEQ3.bit.CONV09 = 7;	/* ChSelect: ADC A7->  Buck output voltage Voltage */			
	AdcRegs.ADCCHSELSEQ3.bit.CONV10 = 15;	/* ChSelect: ADC B7-> Buck Output Current */				
	AdcRegs.ADCCHSELSEQ3.bit.CONV11 = 9;	/* ChSelect: ADC B3-> V Phase A */
	AdcRegs.ADCCHSELSEQ4.bit.CONV12 = 15;	/* ChSelect: ADC B2-> V Phase B */					
	AdcRegs.ADCCHSELSEQ4.bit.CONV13 = 15;	/* ChSelect: ADC B1-> V Phase C */					
	AdcRegs.ADCCHSELSEQ4.bit.CONV14 = 15;    /* ChSelect: ADC A4-> Low Side DC Bus Return Cur.*/	
	AdcRegs.ADCCHSELSEQ4.bit.CONV15 = 15;													
	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 15;
	EDIS;


																								
    /* Set up Event Trigger with CNT_zero enable for Time-base of EPWM1 */						
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;     /* Enable SOCA */										
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;    /* Enable CNT_zero event for SOCA */					
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     /* Generate SOCA on the 1st event */					
	EPwm1Regs.ETCLR.bit.SOCA = 1;       /* Clear SOCA flag */
	
	}
	
	



