################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LIB_SRCS += \
../C28x_SGEN_Lib_fpu32.lib \
../rts2800_fpu32_fast_supplement.lib 

C_SRCS += \
../ABC_DQ0_POS_F.c \
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
../Inverter-Main.c \
../PWM_Setup.c \
../Rectifier-DevInit.c \
../SINEANALYZER_DIFF_wPWR_F.c \
../SPLL_1ph_SOGI_F.c \
../UPS_Config.c \
../modbus-rtu.c 

ASM_SRCS += \
../DSP2833x_ADC_cal.asm \
../DSP2833x_CodeStartBranch.asm \
../DSP2833x_usDelay.asm 

CMD_SRCS += \
../DSP2833x_Headers_nonBIOS.cmd \
../F28335_FLASH_SolarInverter.cmd 

ASM_DEPS += \
./DSP2833x_ADC_cal.pp \
./DSP2833x_CodeStartBranch.pp \
./DSP2833x_usDelay.pp 

OBJS += \
./ABC_DQ0_POS_F.obj \
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
./Inverter-Main.obj \
./PWM_Setup.obj \
./Rectifier-DevInit.obj \
./SINEANALYZER_DIFF_wPWR_F.obj \
./SPLL_1ph_SOGI_F.obj \
./UPS_Config.obj \
./modbus-rtu.obj 

C_DEPS += \
./ABC_DQ0_POS_F.pp \
./ADC_Setup.pp \
./CNTL_2P2Z_F.pp \
./CNTL_PI_F.pp \
./DLOG_4CH_F.pp \
./DSP2833x_CpuTimers.pp \
./DSP2833x_DefaultIsr.pp \
./DSP2833x_GlobalVariableDefs.pp \
./DSP2833x_PieCtrl.pp \
./DSP2833x_PieVect.pp \
./IO_Setup.pp \
./Inverter-Main.pp \
./PWM_Setup.pp \
./Rectifier-DevInit.pp \
./SINEANALYZER_DIFF_wPWR_F.pp \
./SPLL_1ph_SOGI_F.pp \
./UPS_Config.pp \
./modbus-rtu.pp 

OBJS__QTD += \
".\ABC_DQ0_POS_F.obj" \
".\ADC_Setup.obj" \
".\CNTL_2P2Z_F.obj" \
".\CNTL_PI_F.obj" \
".\DLOG_4CH_F.obj" \
".\DSP2833x_ADC_cal.obj" \
".\DSP2833x_CodeStartBranch.obj" \
".\DSP2833x_CpuTimers.obj" \
".\DSP2833x_DefaultIsr.obj" \
".\DSP2833x_GlobalVariableDefs.obj" \
".\DSP2833x_PieCtrl.obj" \
".\DSP2833x_PieVect.obj" \
".\DSP2833x_usDelay.obj" \
".\IO_Setup.obj" \
".\Inverter-Main.obj" \
".\PWM_Setup.obj" \
".\Rectifier-DevInit.obj" \
".\SINEANALYZER_DIFF_wPWR_F.obj" \
".\SPLL_1ph_SOGI_F.obj" \
".\UPS_Config.obj" \
".\modbus-rtu.obj" 

ASM_DEPS__QTD += \
".\DSP2833x_ADC_cal.pp" \
".\DSP2833x_CodeStartBranch.pp" \
".\DSP2833x_usDelay.pp" 

C_DEPS__QTD += \
".\ABC_DQ0_POS_F.pp" \
".\ADC_Setup.pp" \
".\CNTL_2P2Z_F.pp" \
".\CNTL_PI_F.pp" \
".\DLOG_4CH_F.pp" \
".\DSP2833x_CpuTimers.pp" \
".\DSP2833x_DefaultIsr.pp" \
".\DSP2833x_GlobalVariableDefs.pp" \
".\DSP2833x_PieCtrl.pp" \
".\DSP2833x_PieVect.pp" \
".\IO_Setup.pp" \
".\Inverter-Main.pp" \
".\PWM_Setup.pp" \
".\Rectifier-DevInit.pp" \
".\SINEANALYZER_DIFF_wPWR_F.pp" \
".\SPLL_1ph_SOGI_F.pp" \
".\UPS_Config.pp" \
".\modbus-rtu.pp" 

C_SRCS_QUOTED += \
"../ABC_DQ0_POS_F.c" \
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
"../Inverter-Main.c" \
"../PWM_Setup.c" \
"../Rectifier-DevInit.c" \
"../SINEANALYZER_DIFF_wPWR_F.c" \
"../SPLL_1ph_SOGI_F.c" \
"../UPS_Config.c" \
"../modbus-rtu.c" 

ASM_SRCS_QUOTED += \
"../DSP2833x_ADC_cal.asm" \
"../DSP2833x_CodeStartBranch.asm" \
"../DSP2833x_usDelay.asm" 


# Each subdirectory must supply rules for building sources it contributes
ABC_DQ0_POS_F.obj: ../ABC_DQ0_POS_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="ABC_DQ0_POS_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

ADC_Setup.obj: ../ADC_Setup.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="ADC_Setup.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

CNTL_2P2Z_F.obj: ../CNTL_2P2Z_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="CNTL_2P2Z_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

CNTL_PI_F.obj: ../CNTL_PI_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="CNTL_PI_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DLOG_4CH_F.obj: ../DLOG_4CH_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DLOG_4CH_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_ADC_cal.obj: ../DSP2833x_ADC_cal.asm $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_ADC_cal.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_CodeStartBranch.obj: ../DSP2833x_CodeStartBranch.asm $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_CodeStartBranch.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_CpuTimers.obj: ../DSP2833x_CpuTimers.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_CpuTimers.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_DefaultIsr.obj: ../DSP2833x_DefaultIsr.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_DefaultIsr.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_GlobalVariableDefs.obj: ../DSP2833x_GlobalVariableDefs.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_GlobalVariableDefs.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_PieCtrl.obj: ../DSP2833x_PieCtrl.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_PieCtrl.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_PieVect.obj: ../DSP2833x_PieVect.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_PieVect.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_usDelay.obj: ../DSP2833x_usDelay.asm $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="DSP2833x_usDelay.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

IO_Setup.obj: ../IO_Setup.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="IO_Setup.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

Inverter-Main.obj: ../Inverter-Main.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="Inverter-Main.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

PWM_Setup.obj: ../PWM_Setup.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="PWM_Setup.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

Rectifier-DevInit.obj: ../Rectifier-DevInit.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="Rectifier-DevInit.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SINEANALYZER_DIFF_wPWR_F.obj: ../SINEANALYZER_DIFF_wPWR_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="SINEANALYZER_DIFF_wPWR_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

SPLL_1ph_SOGI_F.obj: ../SPLL_1ph_SOGI_F.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="SPLL_1ph_SOGI_F.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

UPS_Config.obj: ../UPS_Config.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="UPS_Config.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

modbus-rtu.obj: ../modbus-rtu.c $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" --silicon_version=28 -g --include_path="C:/Program Files (x86)/Texas Instruments/ccsv4/tools/compiler/c2000/include" --include_path="D:/New/LastTries/WSP2/InverterFinal15-05-99/include" --diag_warning=225 --large_memory_model --unified_memory --float_support=fpu32 --preproc_with_compile --preproc_dependency="modbus-rtu.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


