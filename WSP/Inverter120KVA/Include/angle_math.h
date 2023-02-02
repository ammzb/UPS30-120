/* =================================================================================
File name:       ANGLE_MATH.H
===================================================================================*/


#ifndef __ANGLE_MATH_H__
#define __ANGLE_MATH_H__

// ***********************************************************************************
//  Angle wrap macro (wraps angle within _IQ(0.0) and _IQ(1.0)
// ***********************************************************************************
#define    ANGLE_WRAP(angle)              \
             if(angle > _IQ(1.0))         \
               angle -= _IQ(1.0);         \
             else if(angle < _IQ(0.0))    \
               angle += _IQ(1.0);

// ***********************************************************************************
//  Error Angle wrap macro (wraps error angle within _IQ(-0.5) & _IQ(0.5) (-pi to +pi)
// ***********************************************************************************
#define    ERROR_ANGLE_WRAP(angle)        \
             if(angle > _IQ(0.5))         \
               angle -= _IQ(1.0);         \
             else if(angle < _IQ(-0.5))   \
               angle += _IQ(1.0);

#endif // __ANGLE_MATH_H__

