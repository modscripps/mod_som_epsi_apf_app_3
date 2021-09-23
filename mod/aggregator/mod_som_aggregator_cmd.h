/*******************************************************************************
 * @file mod_som_aggregator.h
 * @brief MOD SOM aggregator command shell API
 * @date Apr 20, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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

#ifndef MOD_SOM_AGGREGATOR_CMD_H_
#define MOD_SOM_AGGREGATOR_CMD_H_

#include <aggregator/mod_som_aggregator.h>
#include "rtos_description.h"
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"

#define MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U
/*******************************************************************************
 * @brief
 *   Initialize AGGREGATOR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_init_cmd_f();
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
CPU_INT16S mod_som_aggregator_cmd_format_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_aggregator_cmd_start_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_aggregator_cmd_stop_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);


#endif //RTOS_MODULE_COMMON_SHELL_AVAIL

#endif /* MOD_SOM_AGGREGATOR_CMD_H_ */
