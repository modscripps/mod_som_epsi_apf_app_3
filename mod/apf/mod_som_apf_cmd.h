/*
 * mod_som_apf_cmd.h
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */

#ifndef MOD_APF_MOD_SOM_APF_CMD_H_
#define MOD_APF_MOD_SOM_APF_CMD_H_

#include <apf/mod_som_apf.h>
#include "rtos_description.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"
#include <apf/mod_som_apf_cmd.h>


#define MOD_SOM_APF_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U
/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_init_cmd_f();
/*******************************************************************************
 * @brief
 *   command shell for hello world command
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_hello_world_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell for $menu command
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_menu_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell for $menu command
 *   it will output all the cmd in the cmd table
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_menu_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for Daq  command
 *   Data acquisition command.
 *   It should start the EFE adc master clock
 *   start the turbulence processing task
 *   and store the data
 *   it should store and return a Daq Status
 *   (e.g., Daq enable/disable)
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_daq_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for Daq?  command
 *   tell apf if Daq is enable or disable
 *   it should return an error if can not access to the information
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_daq_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for FubarCal command
 *   run FubarCal cmd with arguments arg1, arg2, ..., argn
 *   it should return a Fubar status
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_fubar_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for FubarCal? command
 *   display Fubar status
 *   it should return an error if can not access to the information

 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_fubar_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for FwRev? command
 *   display Firmware Revision ID
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_fwrev_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for ok? command
 *   wake up SOM and display apf status
 *   if nothing happens after 30 sec go back to sleep
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_ok_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for PowerOff command
 *   prepare SOM to futur power off
 *   should return an apf status.
 *   Power will turn off after reception of this status
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_poweroff_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for EpsiNo command
 *   set the SOM and EFE SN
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_epsi_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for EpsiNo? command
 *   get the SOM and EFE SN
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_epsi_id_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for ProbeNo command
 *   set the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values {type of probe, SN, Coef}
 *   "Shear,103,40"
 *   "FPO7,103,35"
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_probe_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for ProbeNo? command
 *   get the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_probe_id_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for sleep command
 *   put SOM to sleep
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_sleep_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for time command
 *   set UnixEpoch time on SOM
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_time_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   command shell for time? command
 *   get UnixEpoch time on SOM
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_time_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_comm_packet_format_f
 *   set the format of the data
 *   0 = no format (latter on called F0)
 *   1 = format 1 (F1) time pressure epsilon chi fom
 *   2 = format 2 (F2) time pressure epsilon chi fom + something
 *   3 = format 3 (F3) time pressure epsilon chi + decimated spectra
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_comm_packet_format_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);


/*******************************************************************************
 * @brief
 *   command shell for upload command
 *   start uploading data from the SD card to the apf
 *   should return an apf status.
 *
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   apf Command Status
 ******************************************************************************/
CPU_INT16S mod_som_apf_cmd_upload_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
/*******************************************************************************
 * @brief
 *   Get text input from user.
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_get_input_f(char *buf, uint32_t * buf_len);

#endif //MOD_SOM_APF_MODULE_SHELL_AVAIL
#endif /* MOD_APF_MOD_SOM_APF_CMD_H_ */
