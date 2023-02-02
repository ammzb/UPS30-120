#include "SINEANALYZER_DIFF_wPWR_F.h"
#include <math.h>
//*********** Structure Init Function ****//
void SINEANALYZER_DIFF_wPWR_F_init(SINEANALYZER_DIFF_wPWR_F *v)
{
	v->Vin=0;
	v->Vref=0;
	v->SampleFreq=0;
	v->Threshold=0;
	v->Vrms=0;
	//v->Vavg=0;
	v->Pavg=0;
	//v->Vema=0;
	v->SigFreq=0;
	v->Iin=0;
	v->Irms=0;
	v->Prms=0;
	v->Cosphi=0;
	v->ZCD=0;
	//v->sum_Vacc_avg=0;
	v->sum_Vacc_rms=0;
	//v->sum_Vacc_ema=0;
	v->sum_Iacc_rms=0;
	v->sum_Pacc_rms=0;
	v->curr_vin_norm=0;
	v->curr_iin_norm=0;
	v->prev_sign=1;
	v->curr_sign=1;
	v->nsamples=0;
	v->nsamples1=0;
	v->nsamplesMin = 0;
	v->nsamplesMax = 0;
	v->inv_nsamples=0;
	v->inv_sqrt_nsamples=0;
	v->slew_power_update=0;
	v->sum_Pacc_mul=0;
}
#pragma CODE_SECTION(SINEANALYZER_DIFF_wPWR_F_FUNC, "ramfuncs2");
//*********** Function Definition ********//
void SINEANALYZER_DIFF_wPWR_F_FUNC(SINEANALYZER_DIFF_wPWR_F *v)
{
	
	
	v->curr_vin_norm = v->Vin;
	v->curr_iin_norm = v->Iin;
	
	if ( v->Vin > v->Threshold )
	{
		v->curr_sign = 1;
	}
	else
	{
		v->curr_sign = 0;
	}
	
	
	if((v->prev_sign != v->curr_sign) && (v->curr_sign == 1) && (v->nsamples1 > v->nsamplesMin)){
		v->ZCD=1;
		v->nsamples1 = 0; 
		v->prev_sign = v->curr_sign;
	}
	
	else if(v->nsamples1 > v->nsamplesMax){
		v->ZCD=0;
		v->nsamples1 = 0;
		v->prev_sign = v->curr_sign;
	}
	else{
		v->ZCD=0;
		v->nsamples1++;
		v->prev_sign = v->curr_sign;
	}
	
	if(v->nsamples == 240)
	{
//		v->ZCD=1;
		v->inv_nsamples = (1.0)*__einvf32(v->nsamples);
		v->inv_sqrt_nsamples = (1.0)*__eisqrtf32(v->nsamples);

		//v->Vavg = (v->sum_Vacc_avg*v->inv_nsamples);
		v->Pavg = (v->sum_Pacc_rms*v->inv_nsamples);
		v->Vrms = (sqrt(v->sum_Vacc_rms)*v->inv_sqrt_nsamples);
		//v->Vema = v->sum_Vacc_ema;
		v->Irms = (sqrt(v->sum_Iacc_rms)*v->inv_sqrt_nsamples);
		v->Cosphi = v->Pavg*(1.0)*__einvf32(v->Vrms*v->Irms);
		v->slew_power_update++ ;
		if(v->slew_power_update > 100)
		{
			v->slew_power_update=0;
			v->Prms = (v->sum_Pacc_mul*(0.01));
			v->sum_Pacc_mul = 0;
		}
		else
		{
			v->sum_Pacc_mul = v->sum_Pacc_mul + (v->sum_Pacc_rms*v->inv_nsamples);
		}
		

		v->SigFreq = (v->SampleFreq*v->inv_nsamples);
//		v->prev_sign = v->curr_sign;
		//v->sum_Vacc_avg = 0;
		v->sum_Vacc_rms = 0;
		//v->sum_Vacc_ema = 0;
		v->sum_Iacc_rms = 0;
		v->sum_Pacc_rms = 0;
		v->nsamples=0;
	}
	else
	{
		v->nsamples++;
		//v->sum_Vacc_avg = v->sum_Vacc_avg+v->curr_vin_norm;
		v->sum_Vacc_rms = v->sum_Vacc_rms+(v->curr_vin_norm*v->curr_vin_norm);
		//v->sum_Vacc_ema = v->sum_Vacc_ema+(0.01*(v->curr_vin_norm - v->sum_Vacc_ema));
		v->sum_Iacc_rms = v->sum_Iacc_rms+(v->curr_iin_norm*v->curr_iin_norm);
		v->sum_Pacc_rms = v->sum_Pacc_rms+(v->curr_iin_norm*v->curr_vin_norm);
//		v->ZCD=0;
//		v->prev_sign = v->curr_sign;
	}
}
