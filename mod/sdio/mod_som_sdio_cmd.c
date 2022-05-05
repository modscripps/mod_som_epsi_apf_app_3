/*******************************************************************************
 * @file mod_som_sdio.h
 * @brief MOD SOM foo bar command shell API Implementation
 * @date Apr 01, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API uses micrium shell to insert shell table and execute shell commands
 * Make sure the SHELL_CMD table is static so that when you call your custom
 * command you don't get memory access error.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <sdio/mod_som_sdio_cmd.h>
#include <sdio/mod_som_sdio.h>
#include <sdio/mod_som_sdio_priv.h>
#include "mod_som_io.h"

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_sdio_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_sdio_cmd_tbl[] =
{
        { "sdio.filename",  mod_som_sdio_filename_cmd_f    },
        { "sdio.getdata",   mod_som_sdio_get_data_cmd_f    },
        { "sdio.getconfig", mod_som_sdio_get_config_cmd_f  },
        { "sdio.ls",        mod_som_sdio_ls_cmd_f          },
        { "sdio.rm",        mod_som_sdio_rm_cmd_f          },
        { "sdio.enable",    mod_som_sdio_enable_cmd_f          },
        { "sdio.disable",   mod_som_sdio_disable_cmd_f          },
        { 0, 0 }
};

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sdio_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("SDIO CMDs", mod_som_sdio_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}
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
        SHELL_CMD_PARAM *cmd_param){

	CPU_CHAR * filename;
	mod_som_status_t status;

	switch (argc){
	case 2:
		filename = argv[1]; // Convert argument to long
		status = mod_som_sdio_define_filename_f(filename);
		break;
	default:
		mod_som_io_print_f("sdio_filename, wrong argument \r\n");
		break;
	}
	if(status != MOD_SOM_SDIO_STATUS_OK){
		mod_som_io_print_f("sdio_filename,error %lu\r\n",status);
	}
	return status;
}

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
        SHELL_CMD_PARAM *cmd_param){

		mod_som_status_t status = MOD_SOM_STATUS_OK;
	    char * filename;
		uint32_t number_of_files;

		switch (argc){
			case 3:
				filename=argv[1];
				number_of_files=strtol(argv[2],DEF_NULL,10); //strtol convert a str in a long. We use base 10.
				status = mod_som_sdio_read_data_sd_f(filename,number_of_files);
				break;
			default:
				printf("format: sdio.getdata filename number_of_files\r\n");
				break;
		}


	if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

	mod_som_status_t status = MOD_SOM_STATUS_OK;
    char * filename;

	switch (argc){
		case 2:
			filename=argv[1];
			status = mod_som_sdio_read_config_sd_f(filename);
			break;
		default:
			printf("format: sdio.getconfig filename \r\n");
			break;
	}


if(status != MOD_SOM_STATUS_OK)
    return SHELL_EXEC_ERR;
return SHELL_EXEC_ERR_NONE;


	if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

	mod_som_status_t status = mod_som_sdio_ls_sd_f();

	if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to remove one file on the Volume.
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
CPU_INT16S mod_som_sdio_rm_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
		mod_som_status_t status=MOD_SOM_STATUS_OK;
	switch (argc){
		case 2:
			status = mod_som_sdio_rm_sd_f(argv[1]);
			break;
		default:
			printf("format: sdio.rm filename\r\n");
			break;
	}

	if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

CPU_INT16S mod_som_sdio_enable_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status=MOD_SOM_STATUS_OK;

    status = mod_som_sdio_enable_hardware_f();

    return status;
}

CPU_INT16S mod_som_sdio_disable_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status=MOD_SOM_STATUS_OK;

    status = mod_som_sdio_disable_hardware_f();

    return status;
}


