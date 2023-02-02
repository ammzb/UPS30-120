#include "UPS_Config.h"

struct UPS_V UPS;

void UPS_init(void)
{
	UPS.Rectifier.Status.Fault=0;
	UPS.Buck.Status.Fault=0;
	UPS.Boost.Status.Fault=0;
	UPS.Rectifier.Status.On=0;
	UPS.Balancer.Status.On = 0;
	UPS.Rectifier.Status.EmergencyShutDown=0;
	UPS.Rectifier.Status.ShutDown=0;
	UPS.Rectifier.Status.ShortCircuit=0;
	UPS.Rectifier.Status.AC_OverVoltage=0;
	UPS.Rectifier.Status.AC_UnderVoltage=0;
	UPS.Rectifier.Status.DC_MinVoltage=0;
	UPS.Rectifier.Status.IDF=0;	
	UPS.Buck.Status.IDF=0;
	UPS.Boost.Status.IDF=0;
	UPS.Rectifier.Status.On_K2=0;
	UPS.Rectifier.Status.On_K3=0;
	UPS.Rectifier.Status.Freq_OutOfRange=0;
	UPS.Rectifier.Status.DC_MaxVoltage=0;
	UPS.Inverter.Status.On=0;
	UPS.Buck.Status.Bat_OverVoltage=0;
	UPS.Buck.Status.MaxCurrent=0;
	UPS.Boost.Status.OverCurrent=0;
	UPS.Buck.Status.On=0;
	UPS.Boost.Status.On=0;
	UPS.Inverter.Status.InverterFault=0;
	UPS.Inverter.Status.FaultCommand=0;
	UPS.Inverter.Status.ChargerFault=0;
	UPS.Rectifier.Status.PG=0;
	UPS.Boost.Status.VBatt_End=0;
	UPS.Boost.Status.DC_MinVoltage=0;
	UPS.Rectifier.Status.DC_UnderVoltage=0;
	UPS.Rectifier.Status.DC_UnbalanceVoltage=0;
	UPS.Boost.Status.MaxCurrent=0;
	UPS.Rectifier.Status.DC_OverVoltage=0;
	UPS.Rectifier.Status.OverCurrent=0;
	UPS.Buck.Status.OverCurrent=0;
	UPS.Balancer.Status.ShortCircuit=0;
	UPS.Boost.Status.DC_MaxVoltage=0;
	UPS.Rectifier.Status.SequenceOK=0;
	UPS.Rectifier.Status.OverTemp=0;
	UPS.Rectifier.Status.ContactorFault=0;
	UPS.Rectifier.Status.DC_MinVoltage=0;
	UPS.Rectifier.Status.ProcessorFault=0;
}
