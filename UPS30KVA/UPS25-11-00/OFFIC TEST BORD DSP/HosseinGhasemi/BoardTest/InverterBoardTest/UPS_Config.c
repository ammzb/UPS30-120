#include "UPS_Config.h"

struct UPS_V UPS;

void UPS_init(void)
{
	UPS.Inverter.Status.ComFault=0;
	UPS.Inverter.Status.ON=0;
	UPS.Inverter.Status.EmergencyShutDown=0;
	UPS.Inverter.Status.Reset=0;
	UPS.Inverter.Status.ShortCircuitR=0;
	UPS.Inverter.Status.ShortCircuitS=0;
	UPS.Inverter.Status.ShortCircuitT=0;
	UPS.Inverter.Status.OverLoadExist=0;
	UPS.Inverter.Status.OverLoad=0;
	UPS.Inverter.Status.Charger_Fault=0;
	UPS.Inverter.Status.Charger_Ready=0;
	UPS.Inverter.Status.AC_OverVoltage=0;
	UPS.Inverter.Status.IDF1=0;
	UPS.Inverter.Status.IDF2=0;
	UPS.Inverter.Status.IDF3=0;	
	UPS.Inverter.Status.IDF=0;	
	UPS.Inverter.Status.OffCommand=0;
//	UPS.Inverter.Status.BypassOnLoad=0;
	UPS.Inverter.Status.BypassOnStart=0;
	UPS.Inverter.Status.Sync=0;
	UPS.Inverter.Status.SequenceOK=0; 
	UPS.Inverter.Status.PG=0;
	UPS.Inverter.Status.InverterOnLoad=0;
	UPS.Inverter.Status.BypassFail = 0;
	UPS.Inverter.Status.InvSTSFault = 0;
	UPS.Inverter.Status.BypassSTSFault = 0;
	UPS.Inverter.Status.BuckFault = 0;
	UPS.Inverter.Status.ShutDown = 0;
}
