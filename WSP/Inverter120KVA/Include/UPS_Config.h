#ifndef UPS_CONFIG_H_
#define UPS_CONFIG_H_

#define Grid_220_750


#ifdef Grid_22_80
#define RST_Inverter_ShortCircuit         1.2              
#define Nominal_Active_Power              300.0
#define Nominal_Apparent_Power			  500.0
#define RST_Inverter_OverVoltage          0.16         //(50/311)
#define Min_Freq                          48.0         //    
#define Max_Freq                          52.0 
#define GRID_FREQ                         50.0
//#define Sampling_Freq                     12000.0
#define	Step_SoftStart                    0.000016667      //1/60000
#define Gain_pu                           10.0
#define	Ki_Va                          	  188.0
#define	Kp_Va	                          0.67
#define	Ki_Vb	                          188.0
#define	Kp_Vb	                          0.67
#define	Ki_Vc	                          188.0
#define	Kp_Vc	                          0.67
#define	Ki_Ia	                          0.0
#define	Kp_Ia	                          0.36
#define	Ki_Ib	                          0.0
#define	Kp_Ib	                          0.36
#define	Ki_Ic	                          0.0
#define	Kp_Ic	                          0.36
#define PWM_MAX                           6250
#define SAMPLING_FREQ                     12000.0
#define MAX_SAMPLES                       280
#define MIN_SAMPLES                       200
#define MAX_PHASEDIFF                     0.14 //(8/360)*2*pi
#define PB_DETECTION_CYCLES               10
#define PG_DETECTION_CYCLES               100
#define BYPASS_FAULT_THRESHOLD            0.07  //10/311

#define K                                 0.0
#define TEST_MODE                         1

#define B2_H1                             0.00004166
#define B1_H1                             0.0
#define B0_H1                             0.00004166
#define A2_H1                             1.0
#define A1_H1							  1.99931465

#define B2_H3                             0.00004162
#define B1_H3                             0.0
#define B0_H3                             0.00004162
#define A2_H3                             1.0
#define A1_H3							  1.993835

#define B2_H5                             0.00004155
#define B1_H5                             0.0
#define B0_H5                             0.00004155
#define A2_H5                             1.0
#define A1_H5							  1.9828897

#define B2_H7                             0.00004143
#define B1_H7                             0.0
#define B0_H7                             0.00004143
#define A2_H7                             1.0
#define A1_H7							  1.9665098

#define B2_H9                             0.00004128
#define B1_H9                             0.0
#define B0_H9                             0.00004128
#define A2_H9                             1.0
#define A1_H9							  1.94473984

#define B2_H11                            0.00004109
#define B1_H11                            0.0
#define B0_H11                            0.00004109
#define A2_H11                            1.0
#define A1_H11							  1.91763947
#endif


#ifdef Grid_220_750
#define RST_Inverter_ShortCircuit         2.0       //100/65     
#define Nominal_Active_Power              8000.0
#define Nominal_Apparent_Power			  10000.0
#define RST_Inverter_OverVoltage          1.3             //(340/311)
#define Min_Freq                          48.0         //    
#define Max_Freq                          52.0 
#define GRID_FREQ                         50.0
//#define Sampling_Freq                   12000.0
#define	Step_SoftStart                    0.000016667       //1/60000
#define Gain_pu                           1.0
#define	Ki_Va                          	  188.0
#define	Kp_Va	                          0.67
#define	Ki_Vb	                          188.0
#define	Kp_Vb	                          0.67
#define	Ki_Vc	                          188.0
#define	Kp_Vc	                          0.67
#define	Ki_Ia	                          0.0
#define	Kp_Ia	                          0.36
#define	Ki_Ib	                          0.0
#define	Kp_Ib	                          0.36
#define	Ki_Ic	                          0.0
#define	Kp_Ic	                          0.36
#define PWM_MAX                           6250
#define SAMPLING_FREQ                     12000.0
#define MAX_SAMPLES                       280
#define MIN_SAMPLES                       200
#define MAX_PHASEDIFF                     0.14 //(8/360)*2*pi
#define PB_DETECTION_CYCLES               20
#define PG_DETECTION_CYCLES               250  //5 seconds
#define BYPASS_FAULT_THRESHOLD            0.07  //10/311

#define K                                 1.0
#define TEST_MODE                         0

#define B2_H1                             0.00004166
#define B1_H1                             0.0
#define B0_H1                             0.00004166
#define A2_H1                             1.0
#define A1_H1							  1.99931465

#define B2_H3                             0.00004162
#define B1_H3                             0.0
#define B0_H3                             0.00004162
#define A2_H3                             1.0
#define A1_H3							  1.993835

#define B2_H5                             0.00004155
#define B1_H5                             0.0
#define B0_H5                             0.00004155
#define A2_H5                             1.0
#define A1_H5							  1.9828897

#define B2_H7                             0.00004143
#define B1_H7                             0.0
#define B0_H7                             0.00004143
#define A2_H7                             1.0
#define A1_H7							  1.9665098

#define B2_H9                             0.00004128
#define B1_H9                             0.0
#define B0_H9                             0.00004128
#define A2_H9                             1.0
#define A1_H9							  1.94473984

#define B2_H11                            0.00004109
#define B1_H11                            0.0
#define B0_H11                            0.00004109
#define A2_H11                            1.0
#define A1_H11							  1.91763947
#endif



#define GAIN_CAL_VAC 1.64             //510/311 
#define GAIN_CAL_IAC 2.3             //150/65
#define GAIN_CAL_VDC 0.68             //510/750
#define BASE_VOLTAGE 311.0
#define BASE_CURRENT 65.0
 

//*********** Structure Definition ********//
struct ConverterStatus {     // bits  description

    unsigned long  ComFault:1; 		  // Inverter Common Fault.
    unsigned long  ON:1;               // if 1, Inverter is on.   
    unsigned long  EmergencyShutDown:1;     
    unsigned long  Reset:1;         // Inverter reset flag.     

    unsigned long  ShortCircuitR:1;  // if 1, a short circuit is occured at the output.
	unsigned long  ShortCircuitS:1;  // if 1, a short circuit is occured at the output.
	unsigned long  ShortCircuitT:1;  // if 1, a short circuit is occured at the output.
	 
    unsigned long  OverLoadExist:1;      // if 1, overload is occured.
    unsigned long OverLoad:1; 

    unsigned long  AC_OverVoltage:1;         
    unsigned long  AC_UnderVoltage:1;         

    unsigned long  Charger_Fault:1;     // if 1, DC input of the inverter is lower than acceptable range.    
    unsigned long  Charger_Ready:1;   // if 1, DC input of the inverter is exceeded the lowest acceptable range.

    unsigned long  IDF1:1;	 // IGBT Dirver Fault
    unsigned long  IDF2:1;	 // IGBT Dirver Fault
    unsigned long  IDF3:1;	 // IGBT Dirver Fault
    unsigned long  IDF:1;	 // IGBT Dirver Fault
    
    unsigned long  OffCommand:1;
    unsigned long  BypassOnLoad:1;
    unsigned long  BypassOnStart:1;
    unsigned long  Sync:1;
    unsigned long  PG:1;
    unsigned long  InverterOnLoad:1;
    unsigned long  BypassFail:1;
    unsigned long  SequenceOK:1;
    unsigned long  STSFault:1;
    unsigned long  BypassSTSFault:1;
    unsigned long  InvSTSFault:1;
    unsigned long  BuckFault:1;
    unsigned long  ShutDown:1;
    unsigned long  rsvd1:3;
      
  
};

struct Converter {
	struct ConverterStatus Status; 
};

struct UPS_V {
	//struct Converter    Rectifier;
	struct Converter    Inverter;
};

//*********** Function and Variables Declarations *******//

void UPS_init(void);


#endif /*UPS_CONFIG_H_*/
