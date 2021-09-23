################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include service/subdir.mk
-include ports/source/gnu/subdir.mk
-include ports/source/generic/subdir.mk
-include mod/voltage/subdir.mk
-include mod/vec_nav/subdir.mk
-include mod/shell/subdir.mk
-include mod/settings/subdir.mk
-include mod/sdio/HBLib/SDIO_Driver/src/subdir.mk
-include mod/sdio/HBLib/FatFS/src/subdir.mk
-include mod/sdio/subdir.mk
-include mod/sbe41/subdir.mk
-include mod/io/subdir.mk
-include mod/efe_obp/subdir.mk
-include mod/efe/subdir.mk
-include mod/calendar/subdir.mk
-include mod/apf/subdir.mk
-include mod/altimeter/subdir.mk
-include mod/aggregator/subdir.mk
-include mod/actuator/subdir.mk
-include mod/DSP/TransformFunctions/subdir.mk
-include mod/DSP/StatisticsFunctions/subdir.mk
-include mod/DSP/FastMathFunctions/subdir.mk
-include mod/DSP/ComplexMathFunctions/subdir.mk
-include mod/DSP/CommonTables/subdir.mk
-include mod/DSP/subdir.mk
-include mod/subdir.mk
-include kernel/source/subdir.mk
-include emlib/subdir.mk
-include cpu/source/subdir.mk
-include common/source/shell/subdir.mk
-include common/source/rtos/subdir.mk
-include common/source/ring_buf/subdir.mk
-include common/source/platform_mgr/subdir.mk
-include common/source/op_lock/subdir.mk
-include common/source/logging/subdir.mk
-include common/source/lib/subdir.mk
-include common/source/kal/subdir.mk
-include common/source/common/subdir.mk
-include common/source/collections/subdir.mk
-include common/source/auth/subdir.mk
-include FatFS/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32GG11B/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: mod_som_epsi_apf_app_3.axf

# Tool invocations
mod_som_epsi_apf_app_3.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "mod_som_epsi_apf_app_3.ld" -Xlinker --gc-sections -Xlinker -Map="mod_som_epsi_apf_app_3.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -u _printf_float -o mod_som_epsi_apf_app_3.axf "./CMSIS/EFM32GG11B/startup_efm32gg11b.o" "./CMSIS/EFM32GG11B/system_efm32gg11b.o" "./Drivers/retargetio.o" "./Drivers/retargetserial.o" "./FatFS/ff.o" "./FatFS/unicode.o" "./common/source/auth/auth.o" "./common/source/collections/bitmap.o" "./common/source/collections/map.o" "./common/source/collections/slist.o" "./common/source/common/common.o" "./common/source/kal/kal_kernel.o" "./common/source/lib/lib_ascii.o" "./common/source/lib/lib_math.o" "./common/source/lib/lib_mem.o" "./common/source/lib/lib_mem_cmd.o" "./common/source/lib/lib_str.o" "./common/source/logging/logging.o" "./common/source/op_lock/op_lock.o" "./common/source/platform_mgr/platform_mgr.o" "./common/source/ring_buf/ring_buf.o" "./common/source/rtos/rtos_err_str.o" "./common/source/rtos/rtos_utils.o" "./common/source/shell/shell.o" "./common/source/shell/shell_cmd.o" "./cpu/source/cpu_core.o" "./emlib/em_adc.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_cryotimer.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_ldma.o" "./emlib/em_leuart.o" "./emlib/em_msc.o" "./emlib/em_rtc.o" "./emlib/em_rtcc.o" "./emlib/em_system.o" "./emlib/em_timer.o" "./emlib/em_usart.o" "./kernel/source/os_cfg_app.o" "./kernel/source/os_core.o" "./kernel/source/os_dbg.o" "./kernel/source/os_flag.o" "./kernel/source/os_mon.o" "./kernel/source/os_msg.o" "./kernel/source/os_mutex.o" "./kernel/source/os_prio.o" "./kernel/source/os_q.o" "./kernel/source/os_sem.o" "./kernel/source/os_stat.o" "./kernel/source/os_task.o" "./kernel/source/os_time.o" "./kernel/source/os_tmr.o" "./kernel/source/os_var.o" "./mod/DSP/CommonTables/arm_common_tables.o" "./mod/DSP/CommonTables/arm_const_structs.o" "./mod/DSP/CommonTables/arm_mve_tables.o" "./mod/DSP/ComplexMathFunctions/arm_cmplx_mag_f32.o" "./mod/DSP/ComplexMathFunctions/arm_cmplx_mag_squared_f32.o" "./mod/DSP/FastMathFunctions/arm_cos_f32.o" "./mod/DSP/FastMathFunctions/arm_cos_q15.o" "./mod/DSP/FastMathFunctions/arm_cos_q31.o" "./mod/DSP/FastMathFunctions/arm_sin_f32.o" "./mod/DSP/FastMathFunctions/arm_sin_q15.o" "./mod/DSP/FastMathFunctions/arm_sin_q31.o" "./mod/DSP/FastMathFunctions/arm_sqrt_q15.o" "./mod/DSP/FastMathFunctions/arm_sqrt_q31.o" "./mod/DSP/FastMathFunctions/arm_vexp_f32.o" "./mod/DSP/FastMathFunctions/arm_vlog_f32.o" "./mod/DSP/StatisticsFunctions/arm_max_f32.o" "./mod/DSP/StatisticsFunctions/arm_mean_f32.o" "./mod/DSP/StatisticsFunctions/arm_power_f32.o" "./mod/DSP/TransformFunctions/arm_bitreversal.o" "./mod/DSP/TransformFunctions/arm_bitreversal2.o" "./mod/DSP/TransformFunctions/arm_cfft_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_f64.o" "./mod/DSP/TransformFunctions/arm_cfft_init_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_init_f64.o" "./mod/DSP/TransformFunctions/arm_cfft_init_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_init_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_init_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_init_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_init_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_radix2_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_init_f32.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_init_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_init_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_q15.o" "./mod/DSP/TransformFunctions/arm_cfft_radix4_q31.o" "./mod/DSP/TransformFunctions/arm_cfft_radix8_f32.o" "./mod/DSP/TransformFunctions/arm_dct4_f32.o" "./mod/DSP/TransformFunctions/arm_dct4_init_f32.o" "./mod/DSP/TransformFunctions/arm_dct4_init_q15.o" "./mod/DSP/TransformFunctions/arm_dct4_init_q31.o" "./mod/DSP/TransformFunctions/arm_dct4_q15.o" "./mod/DSP/TransformFunctions/arm_dct4_q31.o" "./mod/DSP/TransformFunctions/arm_rfft_f32.o" "./mod/DSP/TransformFunctions/arm_rfft_fast_f32.o" "./mod/DSP/TransformFunctions/arm_rfft_fast_f64.o" "./mod/DSP/TransformFunctions/arm_rfft_fast_init_f32.o" "./mod/DSP/TransformFunctions/arm_rfft_fast_init_f64.o" "./mod/DSP/TransformFunctions/arm_rfft_init_f32.o" "./mod/DSP/TransformFunctions/arm_rfft_init_q15.o" "./mod/DSP/TransformFunctions/arm_rfft_init_q31.o" "./mod/DSP/TransformFunctions/arm_rfft_q15.o" "./mod/DSP/TransformFunctions/arm_rfft_q31.o" "./mod/DSP/arm_mult_f32.o" "./mod/actuator/mod_som_actuator.o" "./mod/actuator/mod_som_actuator_cmd.o" "./mod/aggregator/mod_som_aggregator.o" "./mod/aggregator/mod_som_aggregator_cmd.o" "./mod/altimeter/mod_som_altimeter.o" "./mod/altimeter/mod_som_altimeter_cmd.o" "./mod/apf/mod_som_apf.o" "./mod/apf/mod_som_apf_cmd.o" "./mod/calendar/mod_som_calendar.o" "./mod/calendar/mod_som_calendar_cmd.o" "./mod/efe/mod_som_efe.o" "./mod/efe/mod_som_efe_cmd.o" "./mod/efe_obp/mod_som_efe_obp.o" "./mod/efe_obp/mod_som_efe_obp_calculus.o" "./mod/efe_obp/mod_som_efe_obp_cmd.o" "./mod/io/mod_som_io.o" "./mod/mod_som.o" "./mod/sbe41/mod_som_sbe41.o" "./mod/sbe41/mod_som_sbe41_cmd.o" "./mod/sdio/HBLib/FatFS/src/diskio.o" "./mod/sdio/HBLib/SDIO_Driver/src/sdio.o" "./mod/sdio/mod_som_sdio.o" "./mod/sdio/mod_som_sdio_cmd.o" "./mod/settings/mod_som_settings.o" "./mod/settings/mod_som_settings_cmd.o" "./mod/shell/mod_som_shell.o" "./mod/shell/mod_som_shell_cmd.o" "./mod/shell/shell_util.o" "./mod/vec_nav/mod_som_vec_nav.o" "./mod/vec_nav/mod_som_vec_nav_cmd.o" "./mod/voltage/mod_som_voltage.o" "./mod/voltage/mod_som_voltage_cmd.o" "./ports/source/generic/arm_cpu_dwt_ts.o" "./ports/source/gnu/armv7m_cpu_a.o" "./ports/source/gnu/armv7m_cpu_c.o" "./ports/source/gnu/armv7m_os_cpu_a.o" "./ports/source/gnu/armv7m_os_cpu_c.o" "./service/sl_sleeptimer.o" "./service/sl_sleeptimer_hal_rtc.o" "./service/sl_sleeptimer_hal_rtcc.o" "./src/mod_som_main.o" -lm -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: mod_som_epsi_apf_app_3.hex'
	arm-none-eabi-objcopy -O ihex "mod_som_epsi_apf_app_3.axf" "mod_som_epsi_apf_app_3.hex"
	@echo ' '

	@echo 'Building bin file: mod_som_epsi_apf_app_3.bin'
	arm-none-eabi-objcopy -O binary "mod_som_epsi_apf_app_3.axf" "mod_som_epsi_apf_app_3.bin"
	@echo ' '

	@echo 'Building s37 file: mod_som_epsi_apf_app_3.s37'
	arm-none-eabi-objcopy -O srec "mod_som_epsi_apf_app_3.axf" "mod_som_epsi_apf_app_3.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "mod_som_epsi_apf_app_3.axf" -A
	@echo ' '

# Other Targets
clean:
	-@$(foreach FILE, $(EXECUTABLES), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(OBJS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_DEPS), $(shell $(RM) $(FILE)) )
	-$(RM) mod_som_epsi_apf_app_3.axf
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
