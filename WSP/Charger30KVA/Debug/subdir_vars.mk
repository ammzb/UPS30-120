################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../DSP2833x_Headers_nonBIOS.cmd \
../F28335_FLASH_SolarInverter.cmd 

LIB_SRCS += \
../C28x_SGEN_Lib_fpu32.lib \
../rts2800_fpu32_fast_supplement.lib 

ASM_SRCS += \
../DSP2833x_ADC_cal.asm \
../DSP2833x_CodeStartBranch.asm \
../DSP2833x_usDelay.asm 

C_SRCS += \
../ADC_Setup.c \
../CNTL_2P2Z_F.c \
../CNTL_PI_F.c \
../DLOG_4CH_F.c \
../DSP2833x_CpuTimers.c \
../DSP2833x_DefaultIsr.c \
../DSP2833x_GlobalVariableDefs.c \
../DSP2833x_PieCtrl.c \
../DSP2833x_PieVect.c \
../IO_Setup.c \
../PWM_Setup.c \
../Rectifier-DevInit.c \
../Rectifier-Main.c \
../SINEANALYZER_DIFF_wPWR_F.c \
../SPLL_1ph_SOGI_F.c \
../UPS_Config.c \
../iCLARKE_F.c \
../iPARK_F.c \
../modbus-rtu.c 

C_DEPS += \
./ADC_Setup.d \
./CNTL_2P2Z_F.d \
./CNTL_PI_F.d \
./DLOG_4CH_F.d \
./DSP2833x_CpuTimers.d \
./DSP2833x_DefaultIsr.d \
./DSP2833x_GlobalVariableDefs.d \
./DSP2833x_PieCtrl.d \
./DSP2833x_PieVect.d \
./IO_Setup.d \
./PWM_Setup.d \
./Rectifier-DevInit.d \
./Rectifier-Main.d \
./SINEANALYZER_DIFF_wPWR_F.d \
./SPLL_1ph_SOGI_F.d \
./UPS_Config.d \
./iCLARKE_F.d \
./iPARK_F.d \
./modbus-rtu.d 

OBJS += \
./ADC_Setup.obj \
./CNTL_2P2Z_F.obj \
./CNTL_PI_F.obj \
./DLOG_4CH_F.obj \
./DSP2833x_ADC_cal.obj \
./DSP2833x_CodeStartBranch.obj \
./DSP2833x_CpuTimers.obj \
./DSP2833x_DefaultIsr.obj \
./DSP2833x_GlobalVariableDefs.obj \
./DSP2833x_PieCtrl.obj \
./DSP2833x_PieVect.obj \
./DSP2833x_usDelay.obj \
./IO_Setup.obj \
./PWM_Setup.obj \
./Rectifier-DevInit.obj \
./Rectifier-Main.obj \
./SINEANALYZER_DIFF_wPWR_F.obj \
./SPLL_1ph_SOGI_F.obj \
./UPS_Config.obj \
./iCLARKE_F.obj \
./iPARK_F.obj \
./modbus-rtu.obj 

ASM_DEPS += \
./DSP2833x_ADC_cal.d \
./DSP2833x_CodeStartBranch.d \
./DSP2833x_usDelay.d 

OBJS__QUOTED += \
"ADC_Setup.obj" \
"CNTL_2P2Z_F.obj" \
"CNTL_PI_F.obj" \
"DLOG_4CH_F.obj" \
"DSP2833x_ADC_cal.obj" \
"DSP2833x_CodeStartBranch.obj" \
"DSP2833x_CpuTimers.obj" \
"DSP2833x_DefaultIsr.obj" \
"DSP2833x_GlobalVariableDefs.obj" \
"DSP2833x_PieCtrl.obj" \
"DSP2833x_PieVect.obj" \
"DSP2833x_usDelay.obj" \
"IO_Setup.obj" \
"PWM_Setup.obj" \
"Rectifier-DevInit.obj" \
"Rectifier-Main.obj" \
"SINEANALYZER_DIFF_wPWR_F.obj" \
"SPLL_1ph_SOGI_F.obj" \
"UPS_Config.obj" \
"iCLARKE_F.obj" \
"iPARK_F.obj" \
"modbus-rtu.obj" 

C_DEPS__QUOTED += \
"ADC_Setup.d" \
"CNTL_2P2Z_F.d" \
"CNTL_PI_F.d" \
"DLOG_4CH_F.d" \
"DSP2833x_CpuTimers.d" \
"DSP2833x_DefaultIsr.d" \
"DSP2833x_GlobalVariableDefs.d" \
"DSP2833x_PieCtrl.d" \
"DSP2833x_PieVect.d" \
"IO_Setup.d" \
"PWM_Setup.d" \
"Rectifier-DevInit.d" \
"Rectifier-Main.d" \
"SINEANALYZER_DIFF_wPWR_F.d" \
"SPLL_1ph_SOGI_F.d" \
"UPS_Config.d" \
"iCLARKE_F.d" \
"iPARK_F.d" \
"modbus-rtu.d" 

ASM_DEPS__QUOTED += \
"DSP2833x_ADC_cal.d" \
"DSP2833x_CodeStartBranch.d" \
"DSP2833x_usDelay.d" 

C_SRCS__QUOTED += \
"../ADC_Setup.c" \
"../CNTL_2P2Z_F.c" \
"../CNTL_PI_F.c" \
"../DLOG_4CH_F.c" \
"../DSP2833x_CpuTimers.c" \
"../DSP2833x_DefaultIsr.c" \
"../DSP2833x_GlobalVariableDefs.c" \
"../DSP2833x_PieCtrl.c" \
"../DSP2833x_PieVect.c" \
"../IO_Setup.c" \
"../PWM_Setup.c" \
"../Rectifier-DevInit.c" \
"../Rectifier-Main.c" \
"../SINEANALYZER_DIFF_wPWR_F.c" \
"../SPLL_1ph_SOGI_F.c" \
"../UPS_Config.c" \
"../iCLARKE_F.c" \
"../iPARK_F.c" \
"../modbus-rtu.c" 

ASM_SRCS__QUOTED += \
"../DSP2833x_ADC_cal.asm" \
"../DSP2833x_CodeStartBranch.asm" \
"../DSP2833x_usDelay.asm" 


