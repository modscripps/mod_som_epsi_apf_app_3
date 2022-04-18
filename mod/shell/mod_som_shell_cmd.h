/*
 * mod_som_shell_cmd.h
 *
 *  Created on: Jun 4, 2020
 *      Author: aleboyer
 */

#ifndef MOD_SHELL_MOD_SOM_SHELL_CMD_H_
#define MOD_SHELL_MOD_SOM_SHELL_CMD_H_


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"

#define MOD_SOM_STATUS_FAIL_TO_ADD_CMD_TBL 01U

mod_som_status_t mod_som_init_shellcmd_f(void);

// place holder for functions
CPU_INT16S mod_som_id_cmd_f (CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam);

CPU_INT16S mod_som_firmware_id_cmd_f (CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam);

CPU_INT16S mod_som_void_cmd_f (CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam);



#endif /* RTOS_MODULE_COMMON_SHELL_AVAIL */

#endif /* MOD_SHELL_MOD_SOM_SHELL_CMD_H_ */
