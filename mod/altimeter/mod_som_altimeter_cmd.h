/*******************************************************************************
 * @file mod_som_altimeter.h
 * @brief MOD SOM altimeter command shell API
 * @date January 4th, 2021
 *
 * @description
 * This API uses micrium shell to insert shell table and execute shell commands
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_ALTIMETER_CMD_H_
#define MOD_SOM_ALTIMETER_CMD_H_

#include <altimeter/mod_som_altimeter.h>
#include "rtos_description.h"
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"

#define MOD_SOM_ALTIMETER_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U
/*******************************************************************************
 * @brief
 *   Initialize ALTIMETER, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_init_cmd_f();
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
CPU_INT16S mod_som_altimeter_cmd_hello_world_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);
#endif //RTOS_MODULE_COMMON_SHELL_AVAIL

CPU_INT16S mod_som_altimeter_cmd_enable_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_disable_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_start_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_stop_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_mode_f(CPU_INT16U argc,
       CPU_CHAR *argv[],
       SHELL_OUT_FNCT out_put_f,
       SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_repmode_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_blank_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_altimeter_cmd_pulse_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

//MHA
CPU_INT16S mod_som_altimeter_cmd_reprate_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

#endif /* MOD_SOM_ALTIMETER_CMD_H_ */
