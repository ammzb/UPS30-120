

#ifndef __SVGEN_F_H__
#define __SVGEN__F_H__

typedef struct	         {float32  Ualpha; 			// Input: reference alpha-axis phase voltage 
				  float32  Ubeta;			// Input: reference beta-axis phase voltage 
				  float32  Ta;				// Output: reference phase-a switching function		
				  float32  Tb;				// Output: reference phase-b switching function 
				  float32  Tc;				// Output: reference phase-c switching function
				  float32  tmp1;			// Variable: temp variable
				  float32  tmp2;			// Variable: temp variable
				  float32  tmp3;			// Variable: temp variable
				  Uint32 VecSector;		// Space vector sector
				}SVGEN_F ;
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																				

/*-----------------------------------------------------------------------------
Default initalizer for the SVGEN object.
-----------------------------------------------------------------------------*/                     
#define SVGEN_DEFAULTS  { 0,0,0,0,0}   



/*------------------------------------------------------------------------------
	Space Vector  Generator (SVGEN) Macro Definition
------------------------------------------------------------------------------*/


#define SVGEN_MACRO(v)														\
	v.tmp1= v.Ubeta;															\
	v.tmp2= (0.5*v.Ubeta)+(0.866*v.Ualpha);					\
    v.tmp3= v.tmp2 - v.tmp1;													\
																				\
	v.VecSector=3;																\
	v.VecSector=(v.tmp2> 0)?( v.VecSector-1):v.VecSector;						\
	v.VecSector=(v.tmp3> 0)?( v.VecSector-1):v.VecSector;						\
	v.VecSector=(v.tmp1< 0)?(7-v.VecSector) :v.VecSector;						\
																				\
	if     (v.VecSector==1 || v.VecSector==4)                                   \
      {     v.Ta= v.tmp2; 														\
      		v.Tb= v.tmp1-v.tmp3; 												\
      		v.Tc=-v.tmp2;														\
      }								    										\
   																				\
    else if(v.VecSector==2 || v.VecSector==5)                                   \
      {     v.Ta= v.tmp3+v.tmp2; 												\
      		v.Tb= v.tmp1; 														\
      		v.Tc=-v.tmp1;														\
      }																	   		\
   																				\
    else                                                                        \
      {     v.Ta= v.tmp3; 														\
      		v.Tb=-v.tmp3; 														\
      		v.Tc=-(v.tmp1+v.tmp2);												\
      }																	   		
																				

#endif // __SVGEN_H__

