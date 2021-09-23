/*
 * mod_som_sbe49_cmd.h
 *
 *  Created on: Feb 10, 2020
 *      Author: snguyen
 */

#ifndef MOD_MOD_SOM_SBE49_CMD_H_
#define MOD_MOD_SOM_SBE49_CMD_H_
#include <rtos_description.h>
#include "mod_som_sbe49.h"
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include  <common/include/shell.h>
#include "mod_som_shell.h"


#define MOD_SOM_SBE49_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U

/*******************************************************************************
 * @brief
 *   Initialize SBE49, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_init_cmd_f();


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe49_start_collect_data' command.
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
CPU_INT16S mod_som_sbe49_cmd_start_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe49_stop_collect_data' command.
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
CPU_INT16S mod_som_sbe49_cmd_stop_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_consumer_mode_f' command.
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
CPU_INT16S mod_som_sbe49_cmd_consumer_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_output_mode_f' command.
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
CPU_INT16S mod_som_sbe49_cmd_output_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_gate_f' command.
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

CPU_INT16S mod_som_sbe49_cmd_gate_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam);

CPU_INT16S mod_som_sbe49_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

//MHA declare new dcal command function
CPU_INT16S mod_som_sbe49_cmd_dcal_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_sbe49_cmd_actu_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);



#endif
#endif /* MOD_MOD_SOM_SBE49_CMD_H_ */
