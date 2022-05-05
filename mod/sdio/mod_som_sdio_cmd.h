/*******************************************************************************
 * @file mod_som_sdio.h
 * @brief MOD SOM foo bar command shell API
 * @date Apr 01, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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

#ifndef MOD_SOM_SDIO_CMD_H_
#define MOD_SOM_SDIO_CMD_H_

#include "rtos_description.h"
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"

#define MOD_SOM_SDIO_STATUS_FAIL_TO_ADD_CMD_TBL 0x12U
/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sdio_init_cmd_f();

/*******************************************************************************
 * @brief
 *   command shell to define the filename of the data and config files
 *   Create a file with a name given by the in user in arguments
 *   It d be smart to have the name of the cruise and date in the name.
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
CPU_INT16S mod_som_sdio_filename_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);


/*******************************************************************************
 * @brief
 *   command shell to read from the sd card and stream on the main com port the data on the data_file .
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
CPU_INT16S mod_som_sdio_get_data_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell to read a file on the sd card.
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
CPU_INT16S mod_som_sdio_get_config_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell to list the file on the Volume.
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
CPU_INT16S mod_som_sdio_ls_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

/*******************************************************************************
 * @brief
 *   command shell to rm the file on the Volume.
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 *   argument is the name of the file to remove
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_sdio_rm_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_sdio_enable_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);

CPU_INT16S mod_som_sdio_disable_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param);



#endif //RTOS_MODULE_COMMON_SHELL_AVAIL

#endif /* MOD_SOM_SDIO_CMD_H_ */
