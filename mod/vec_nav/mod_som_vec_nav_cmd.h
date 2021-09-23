/*
 * mod_som_vec_nav_cmd.h
 *
 *
 * @date Mar 12, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 */

#ifndef MOD_MOD_SOM_VECNAV_CMD_H_
#define MOD_MOD_SOM_VECNAV_CMD_H_
#include <mod_som_vec_nav.h>
#include <rtos_description.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include  <common/include/shell.h>
#include "mod_som_shell.h"


#define MOD_SOM_VECNAV_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U

/*******************************************************************************
 * @brief
 *   Initialize VECNAV, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_init_cmd_f();


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'vecnav_start_collect_data' command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_vecnav_cmd_start_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'vecnav_stop_collect_data' command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_vecnav_cmd_stop_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_consumer_mode_f' command.
 *   - it configure the SOM to either STREAM or SD STORE the SBE data
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_vecnav_cmd_consumer_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_output_mode_f' command.
 *   - it configure the output format of the sbe
 *   - it also configure the SOM to receive the newly defined outputformat
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_vecnav_cmd_output_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_gate_f' command.
 *
 *   it opens a direct access to the SBE user interface
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/

CPU_INT16S mod_som_vecnav_cmd_gate_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);

CPU_INT16S mod_som_vecnav_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);


#endif
#endif /* MOD_MOD_SOM_VECNAV_CMD_H_ */
