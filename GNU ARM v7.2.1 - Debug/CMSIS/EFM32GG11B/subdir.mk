################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Applications/Simplicity\ Studio\ 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32GG11B/Source/system_efm32gg11b.c 

S_UPPER_SRCS += \
/Applications/Simplicity\ Studio\ 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32GG11B/Source/GCC/startup_efm32gg11b.S 

OBJS += \
./CMSIS/EFM32GG11B/startup_efm32gg11b.o \
./CMSIS/EFM32GG11B/system_efm32gg11b.o 

C_DEPS += \
./CMSIS/EFM32GG11B/system_efm32gg11b.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/EFM32GG11B/startup_efm32gg11b.o: /Applications/Simplicity\ Studio\ 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32GG11B/Source/GCC/startup_efm32gg11b.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Assembler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/service" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/cfg" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32GG11B/Include" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/micrium_os" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/micrium_os/bsp/siliconlabs/generic/include" '-DEFM32GG11B420F2048GQ100=1' -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -c -x assembler-with-cpp -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

CMSIS/EFM32GG11B/system_efm32gg11b.o: /Applications/Simplicity\ Studio\ 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32GG11B/Source/system_efm32gg11b.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DEFM32GG11B420F2048GQ100=1' -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/DSP/Includes" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/service" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/cfg" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/sdio/HBLib/SDIO_Driver/inc" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/sdio/HBLib/FatFS/inc" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/FatFS_inc" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/config" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/inc" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/src" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/io" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/calendar" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/efe" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/sbe49" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/sdio" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/shell" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/Drivers" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/common/inc" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/altimeter" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/settings" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/actuator" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/vec_nav" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/voltage" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/aggregator" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/sbe41" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/apf" -I"/Users/aleboyer/SimplicityStudio/v5_workspace/mod_som_epsi_apf_app_3/mod/efe_obp" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32GG11B/Include" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/micrium_os" -I"/Applications/Simplicity Studio 2.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v3.0//platform/micrium_os/bsp/siliconlabs/generic/include" -O0 -Wall -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -c -fmessage-length=0 -MMD -MP -MF"CMSIS/EFM32GG11B/system_efm32gg11b.d" -MT"CMSIS/EFM32GG11B/system_efm32gg11b.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


