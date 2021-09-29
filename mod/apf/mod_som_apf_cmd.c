/*
 * mod_som_apf_cmd.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <apf/mod_som_apf.h>

#ifdef RTOS_MODULE_COMMON_SHELL_AVAIL
#include <apf/mod_som_apf_cmd.h>
#include "mod_som_io.h"
#include  <common/source/shell/shell_priv.h>


//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_apf_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_apf_cmd_tbl[] =
{
        { "Daq", mod_som_apf_cmd_daq_f },
        { "Daq?", mod_som_apf_cmd_daq_status_f },
        { "FubarCal", mod_som_apf_cmd_fubar_f },
        { "FubarCal?", mod_som_apf_cmd_fubar_status_f },
        { "FwRev?", mod_som_apf_cmd_fwrev_status_f },
        { "$menu", mod_som_apf_cmd_menu_f },
        { "ok?", mod_som_apf_cmd_ok_status_f },
        { "PowerOff", mod_som_apf_cmd_poweroff_f },
        { "EpsiNo", mod_som_apf_cmd_epsi_id_f },
        { "EpsiNo?", mod_som_apf_cmd_epsi_id_status_f },
        { "ProbeNo", mod_som_apf_cmd_probe_id_f },
        { "ProbeNo?", mod_som_apf_cmd_probe_id_status_f },
        { "sleep", mod_som_apf_cmd_sleep_f },
        { "time", mod_som_apf_cmd_time_f },
        { "time?", mod_som_apf_cmd_time_status_f },
        { "comm_packet_format", mod_som_apf_cmd_comm_packet_format_f },
        { "upload", mod_som_apf_cmd_upload_f },
        { 0, 0 }
};

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, command shell
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("APF CMDs", mod_som_apf_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_ADD_CMD_TBL);
    }
    return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}
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
        SHELL_CMD_PARAM *cmd_param){
    mod_som_apf_status_t status = mod_som_apf_say_hello_world_f();
    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

	Shell_ListCmdOutput(out_put_f,cmd_param);

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

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
		SHELL_CMD_PARAM *cmd_param){

	mod_som_apf_status_t status;
	uint8_t profile_id;
	RTOS_ERR err;

	for (int i = 1; i < argc; i++) {
		if (!Str_Cmp(argv[i], "start")) {
			if (argc<3){

		        mod_som_io_print_f("daq,nak,%s.\r\n","Missing Profile ID.");
			}else{
				//ALB I do not display error if there is more than 1 profile id.
				//ALB it will the last argument as profile id
				profile_id = shellStrtol(argv[++i], &err); // Convert argument to int
				status = mod_som_apf_daq_start_f((uint64_t)profile_id);
			}
		} else if (!Str_Cmp(argv[i], "stop")) {
			status = mod_som_apf_daq_stop_f();
		} else {
	        mod_som_io_print_f("daq,nak,%s.\r\n","invalid daq cmd");
			return MOD_SOM_APF_STATUS_ERR;
		}
	}

	if(status != MOD_SOM_APF_STATUS_OK)
		return MOD_SOM_APF_STATUS_ERR;
	return MOD_SOM_APF_STATUS_ERR;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_daq_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}
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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_fubar_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_fubar_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_fwrev_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_ok_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


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
        SHELL_CMD_PARAM *cmd_param){

//    mod_som_apf_status_t status = mod_som_apf_fwrev_f();
    mod_som_apf_status_t status = mod_som_apf_poweroff_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_epsi_id_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_epsi_id_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


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
        SHELL_CMD_PARAM *cmd_param){

//    mod_som_apf_status_t status = mod_som_apf_fwrev_f();
    mod_som_apf_status_t status = mod_som_apf_probe_id_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

//    mod_som_apf_status_t status = mod_som_apf_fwrev_f();
    mod_som_apf_status_t status = mod_som_apf_probe_id_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_sleep_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_time_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_time_status_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_comm_packet_format_f(argc,argv);

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



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
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_upload_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}




//----------------------------------------------------------------------------------------------
// ALB function concerning the apf shell
// - change coma (",";44) for space (" ";32)
/*******************************************************************************
 * @brief
 *   Get text input from user.
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_shell_get_input_f(char *buf, uint32_t * buf_len){
    int c;
    int32_t i;
    RTOS_ERR err;

    Mem_Set(buf, '\0', MOD_SOM_SHELL_INPUT_BUF_SIZE); // Clear previous input
    for (i = 0; i < MOD_SOM_SHELL_INPUT_BUF_SIZE - 1; i++) {
        c = RETARGET_ReadChar();
        while (c < 0){ // Wait for valid input
            //Release for waiting tasks
            OSTimeDly(
                    (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                    (OS_OPT      )OS_OPT_TIME_DLY,
                    &err);
            APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
            c = RETARGET_ReadChar();
        }

        if (c == ASCII_CHAR_DELETE || c == 0x08) { // User inputed backspace
            if (i) {
                mod_som_io_print_f("\b \b");
                buf[--i] = '\0';
            }
            i--;
            continue;
        } else if (c == '\r' || c == '\n') {
            if (i) {
                mod_som_io_print_f("\r\n");
                break;
            } else {
                mod_som_io_print_f("\r\n$ ");
                i--;
                continue;
            }
        }else if(c == 27){
            for(i--;i>=0;i--){
                mod_som_io_print_f("\b \b");
                //Release for waiting tasks
                OSTimeDly(
                        (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                        (OS_OPT      )OS_OPT_TIME_DLY,
                        &err);
            }
            buf[0] = '\0';
            continue;
        }else if (!(c>31 && c<127)){ // check for printable characters
        	i--;
            continue;
        }
    	//ALB convert coma from apf command into space. So our micrium parser works without too much of a head ache
        if(c==44){
        	c=32;
        }
        //ALB ask Dana if we need to *not* echo the char.
        mod_som_io_putchar_f(c); // Echo to terminal
        buf[i] = c;
    }

    buf[i] = '\0';
    *(buf_len) = i;
    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
}



#endif // MOD_SOM_APF_MODULE_SHELL_AVAIL


