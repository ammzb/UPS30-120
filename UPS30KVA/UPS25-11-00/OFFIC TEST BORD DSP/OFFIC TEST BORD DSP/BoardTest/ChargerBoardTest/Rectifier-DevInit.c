// TI File $Revision: /main/8 $
// Checkin $Date: April 15, 2009   09:54:05 $
//###########################################################################
//
// FILE:   DSP2833x_SysCtrl.c
//
// TITLE:  DSP2833x Device System Control Initialization & Support Functions.
//
// DESCRIPTION:
//
//         Example initialization of system resources.
//
//###########################################################################
// $TI Release: DSP2833x/DSP2823x C/C++ Header Files V1.31 $
// $Release Date: August 4, 2009 $
//###########################################################################


#include "DSP28x_Project.h"     // Headerfile Include File


//extern void IO_Setup(void);
//void DisableDog(void);
//void InitPll(Uint16 pllcr, Uint16 clkindiv);
//void InitPieCtrl(void);
//void InitPieVectTable(void);
//void InitPeripheralClocks(void);
///void ServiceDog(void);
//void ADC_cal(void);

// Functions that will be run from RAM need to be assigned to
// a different section.  This section will then be mapped to a load and
// run address using the linker cmd file.

#pragma CODE_SECTION(InitFlash, "ramfuncs");

//---------------------------------------------------------------------------
// InitSysCtrl:
//---------------------------------------------------------------------------
// This function initializes the System Control registers to a known state.
// - Disables the watchdog
// - Set the PLLCR for proper SYSCLKOUT frequency
// - Set the pre-scaler for the high and low frequency peripheral clocks
// - Enable the clocks to the peripherals

void InitSysCtrl(void)
{

   // Disable the watchdog
    DisableDog();
    DINT;			// Global Disable all Interrupts
	IER = 0x0000;	// Disable CPU interrupts
	IFR = 0x0000;	// Clear all CPU interrupt flags
   

   // Initialize the PLL control: PLLCR and DIVSEL
   // DSP28_PLLCR and DSP28_DIVSEL are defined in DSP2833x_Examples.h
    InitPll(DSP28_PLLCR,DSP28_DIVSEL); 
    // Initialise interrupt controller and Vector Table
    // to defaults for now. Application ISR mapping done later.
	InitPieCtrl();		
	InitPieVectTable();
   // Initialize the peripheral clocks
    InitPeripheralClocks();
    ADC_cal();
    //IO_Setup();
}

void InitFlash(void)
{
   EALLOW;
   //Enable Flash Pipeline mode to improve performance
   //of code executed from Flash.
   FlashRegs.FOPT.bit.ENPIPE = 1;

   //                CAUTION
   //Minimum waitstates required for the flash operating
   //at a given CPU rate must be characterized by TI.
   //Refer to the datasheet for the latest information.
#if CPU_FRQ_150MHZ
   //Set the Paged Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 5;

   //Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 5;

   //Set the Waitstate for the OTP
   FlashRegs.FOTPWAIT.bit.OTPWAIT = 8;
#endif

#if CPU_FRQ_100MHZ
   //Set the Paged Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 3;

   //Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 3;

   //Set the Waitstate for the OTP
   FlashRegs.FOTPWAIT.bit.OTPWAIT = 5;
#endif
   //                CAUTION
   //ONLY THE DEFAULT VALUE FOR THESE 2 REGISTERS SHOULD BE USED
   FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;
   FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;
   EDIS;

   //Force a pipeline flush to ensure that the write to
   //the last register configured occurs before returning.

   asm(" RPT #8 || NOP");
}


//---------------------------------------------------------------------------
// Example: ServiceDog:
//---------------------------------------------------------------------------
// This function resets the watchdog timer.
// Enable this function for using ServiceDog in the application

void ServiceDog(void)
{
    EALLOW;
    SysCtrlRegs.WDKEY = 0x0055;
    SysCtrlRegs.WDKEY = 0x00AA;
    EDIS;
}

//---------------------------------------------------------------------------
// Example: DisableDog:
//---------------------------------------------------------------------------
// This function disables the watchdog timer.

void DisableDog(void)
{
    EALLOW;
    SysCtrlRegs.WDCR= 0x0068;
    EDIS;
}

//---------------------------------------------------------------------------
// Example: InitPll:
//---------------------------------------------------------------------------
// This function initializes the PLLCR register.

void InitPll(Uint16 val, Uint16 divsel)
{

   // Make sure the PLL is not running in limp mode
   if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 0)
   {
      // Missing external clock has been detected
      // Replace this line with a call to an appropriate
      // SystemShutdown(); function.
      asm("        ESTOP0");
   }

   // DIVSEL MUST be 0 before PLLCR can be changed from
   // 0x0000. It is set to 0 by an external reset XRSn
   // This puts us in 1/4
   if (SysCtrlRegs.PLLSTS.bit.DIVSEL != 0)
   {
       EALLOW;
       SysCtrlRegs.PLLSTS.bit.DIVSEL = 0;
       EDIS;
   }

   // Change the PLLCR
   if (SysCtrlRegs.PLLCR.bit.DIV != val)
   {

      EALLOW;
      // Before setting PLLCR turn off missing clock detect logic
      SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;
      SysCtrlRegs.PLLCR.bit.DIV = val;
      EDIS;

      // Optional: Wait for PLL to lock.
      // During this time the CPU will switch to OSCCLK/2 until
      // the PLL is stable.  Once the PLL is stable the CPU will
      // switch to the new PLL value.
      //
      // This time-to-lock is monitored by a PLL lock counter.
      //
      // Code is not required to sit and wait for the PLL to lock.
      // However, if the code does anything that is timing critical,
      // and requires the correct clock be locked, then it is best to
      // wait until this switching has completed.

      // Wait for the PLL lock bit to be set.

      // The watchdog should be disabled before this loop, or fed within
      // the loop via ServiceDog().

	  // Uncomment to disable the watchdog
      DisableDog();

      while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1)
      {
	      // Uncomment to service the watchdog
          // ServiceDog();
      }

      EALLOW;
      SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0;
      EDIS;
    }

    // If switching to 1/2
	if((divsel == 1)||(divsel == 2))
	{
		EALLOW;
	    SysCtrlRegs.PLLSTS.bit.DIVSEL = divsel;
	    EDIS;
	}


	// NOTE: ONLY USE THIS SETTING IF PLL IS BYPASSED (I.E. PLLCR = 0) OR OFF
	// If switching to 1/1
	// * First go to 1/2 and let the power settle
	//   The time required will depend on the system, this is only an example
	// * Then switch to 1/1
	if(divsel == 3)
	{
		EALLOW;
	    SysCtrlRegs.PLLSTS.bit.DIVSEL = 2;
	    DELAY_US(50L);
	    SysCtrlRegs.PLLSTS.bit.DIVSEL = 3;
	    EDIS;
    }
}

//--------------------------------------------------------------------------
// Example: InitPeripheralClocks:
//---------------------------------------------------------------------------
// This function initializes the clocks to the peripheral modules.
// First the high and low clock prescalers are set
// Second the clocks are enabled to each peripheral.
// To reduce power, leave clocks to unused peripherals disabled
//
// Note: If a peripherals clock is not enabled then you cannot
// read or write to the registers for that peripheral
void InitPeripheralClocks(void)
{
   EALLOW;

// HISPCP/LOSPCP prescale register settings, normally it will be set to default values
   SysCtrlRegs.HISPCP.all = 0x0003;
   SysCtrlRegs.LOSPCP.all = 0x0002;

// XCLKOUT to SYSCLKOUT ratio.  By default XCLKOUT = 1/4 SYSCLKOUT
   // XTIMCLK = SYSCLKOUT/2
   XintfRegs.XINTCNF2.bit.XTIMCLK = 1;
   // XCLKOUT = XTIMCLK/2
   XintfRegs.XINTCNF2.bit.CLKMODE = 1;
   // Enable XCLKOUT
   XintfRegs.XINTCNF2.bit.CLKOFF = 0;

// Peripheral clock enables set for the selected peripherals.
// If you are not using a peripheral leave the clock off
// to save on power.
//
// Note: not all peripherals are available on all 2833x derivates.
// Refer to the datasheet for your particular device.
//
// This function is not written to be an example of efficient code.

   SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;    // ADC

   // *IMPORTANT*
   // The ADC_cal function, which  copies the ADC calibration values from TI reserved
   // OTP into the ADCREFSEL and ADCOFFTRIM registers, occurs automatically in the
   // Boot ROM. If the boot ROM code is bypassed during the debug process, the
   // following function MUST be called for the ADC to function according
   // to specification. The clocks to the ADC MUST be enabled before calling this
   // function.
   // See the device data manual and/or the ADC Reference
   // Manual for more information.

   //ADC_cal();


   SysCtrlRegs.PCLKCR0.bit.I2CAENCLK = 1;   // I2C
   SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;   // SCI-A
   SysCtrlRegs.PCLKCR0.bit.SCIBENCLK = 1;   // SCI-B
   SysCtrlRegs.PCLKCR0.bit.SCICENCLK = 1;   // SCI-C
   SysCtrlRegs.PCLKCR0.bit.SPIAENCLK = 1;   // SPI-A
   SysCtrlRegs.PCLKCR0.bit.MCBSPAENCLK = 1; // McBSP-A
   SysCtrlRegs.PCLKCR0.bit.MCBSPBENCLK = 1; // McBSP-B
   SysCtrlRegs.PCLKCR0.bit.ECANAENCLK=1;    // eCAN-A
   SysCtrlRegs.PCLKCR0.bit.ECANBENCLK=1;    // eCAN-B

   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;   // Disable TBCLK within the ePWM
   SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;  // ePWM1
   SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;  // ePWM2
   SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;  // ePWM3
   SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;  // ePWM4
   SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;  // ePWM5
   SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 1;  // ePWM6
   //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;   // Enable TBCLK within the ePWM

   SysCtrlRegs.PCLKCR1.bit.ECAP3ENCLK = 1;  // eCAP3
   SysCtrlRegs.PCLKCR1.bit.ECAP4ENCLK = 1;  // eCAP4
   SysCtrlRegs.PCLKCR1.bit.ECAP5ENCLK = 1;  // eCAP5
   SysCtrlRegs.PCLKCR1.bit.ECAP6ENCLK = 1;  // eCAP6
   SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1;  // eCAP1
   SysCtrlRegs.PCLKCR1.bit.ECAP2ENCLK = 1;  // eCAP2
   SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;  // eQEP1
   SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;  // eQEP2

   SysCtrlRegs.PCLKCR3.bit.CPUTIMER0ENCLK = 1; // CPU Timer 0
   SysCtrlRegs.PCLKCR3.bit.CPUTIMER1ENCLK = 1; // CPU Timer 1
   SysCtrlRegs.PCLKCR3.bit.CPUTIMER2ENCLK = 1; // CPU Timer 2

   SysCtrlRegs.PCLKCR3.bit.DMAENCLK = 1;       // DMA Clock
   SysCtrlRegs.PCLKCR3.bit.XINTFENCLK = 1;     // XTIMCLK
   SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1;    // GPIO input clock

   EDIS;
}
// This function will copy the specified memory contents from
// one location to another. 
// 
//	Uint16 *SourceAddr        Pointer to the first word to be moved
//                          SourceAddr < SourceEndAddr
//	Uint16* SourceEndAddr     Pointer to the last word to be moved
//	Uint16* DestAddr          Pointer to the first destination word
//
// No checks are made for invalid memory locations or that the
// end address is > then the first start address.

void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr)
{
    while(SourceAddr < SourceEndAddr)
    { 
       *DestAddr++ = *SourceAddr++;
    }
    return;
}
	
//===========================================================================
// End of file.
//===========================================================================



















