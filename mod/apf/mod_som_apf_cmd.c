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


//ALB

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_apf_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_apf_cmd_tbl[] =
{
        { "daq",          mod_som_apf_cmd_daq_f },
        { "daq?",         mod_som_apf_cmd_daq_status_f },
        { "time",         mod_som_apf_cmd_time_f },
        { "time?",        mod_som_apf_cmd_time_status_f },
        { "$menu",        mod_som_apf_cmd_menu_f },
        { "--ok?",        mod_som_apf_cmd_ok_status_f },
        { "sleep",        mod_som_apf_cmd_sleep_f },
        { "fwrev?",       mod_som_apf_cmd_fwrev_status_f },
        { "upload",       mod_som_apf_cmd_upload_f },
        { "epsino?",      mod_som_apf_cmd_epsi_id_status_f },
        { "probeno",      mod_som_apf_cmd_probe_id_f },
        { "probeno?",     mod_som_apf_cmd_probe_id_status_f },
        { "poweroff",     mod_som_apf_cmd_poweroff_f },
        { "sd_format",    mod_som_apf_cmd_sd_format_f },
        { "process_nfft", mod_som_apf_cmd_process_nfft_f },
        { "comm_packet_format", mod_som_apf_cmd_comm_packet_format_f },
        { 0, 0 }
};
//{ "EpsiNo", mod_som_apf_cmd_epsi_id_f },

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
	uint64_t profile_id;
	RTOS_ERR err;


  // paramters for send_line_f()
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
  // get the port's fd
  uint32_t bytes_sent = 0;


	for (int i = 1; i < argc; i++) {
		if (!Str_Cmp(argv[i], "start")) {
			if (argc<3){
		        mod_som_io_print_f("daq,nak,%s.\r\n","Missing Profile ID.");
		        // save time string into the temporary local string - Mai - Nov 18, 2021
		        sprintf(apf_reply_str,"daq,nak,%s.\r\n","Missing Profile ID.");
		        reply_str_len = strlen(apf_reply_str);
		        // sending the above string to the APF port - Mai - Nov 18, 2021
		        bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
			}else{
				//ALB I do not display error if there is more than 1 profile id.
				//ALB it will the last argument as profile id
				profile_id = shellStrtol(argv[++i], &err); // Convert argument to int
				status = mod_som_apf_daq_start_f((uint64_t)profile_id);

			  if (status==0){
			      mod_som_io_print_f("daq,start,ack,%lu\r\n",(uint32_t) profile_id);
		        // save time string into the temporary local string - Mai - Nov 18, 2021
		        sprintf("daq,start,ack,%lu\r\n",(uint32_t) profile_id);
			  }else{
			      mod_som_io_print_f("daq,start,nak,%lu",status);
		        // save time string into the temporary local string - Mai - Nov 18, 2021
		        sprintf(apf_reply_str,"daq,start,nak,%lu",status);
			  } // end of (status ==0)
			  reply_str_len = strlen(apf_reply_str);
			  // sending the above string to the APF port - Mai - Nov 18, 2021
			  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
			}// end of (argc<3)
		} else if (!Str_Cmp(argv[i], "stop")) {
			status = mod_som_apf_daq_stop_f();
			//ALB display msg
			  if (status==MOD_SOM_APF_STATUS_OK){
			      mod_som_io_print_f("daq,stop,ack\r\n");
            // save time string into the temporary local string - Mai - Nov 18, 2021
            sprintf(apf_reply_str,"daq,stop,ack\r\n");
			  }else{
			      mod_som_io_print_f("daq,stop,nak,%lu\r\n",status);
            // save time string into the temporary local string - Mai - Nov 18, 2021
            sprintf(apf_reply_str,"daq,start,nak,%lu\r\n",status);
			  }
        reply_str_len = strlen(apf_reply_str);
        // sending the above string to the APF port - Mai - Nov 18, 2021
        bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
		} else {
	        mod_som_io_print_f("daq,nak,%s.\r\n","invalid daq cmd");
          // save time string into the temporary local string - Mai - Nov 18, 2021
          sprintf(apf_reply_str,"daq,nak,%s.\r\n","invalid daq cmd");
          // sending the above string to the APF port - Mai - Nov 18, 2021
          bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
			return MOD_SOM_APF_STATUS_ERR;
		}
	}

	if(status != MOD_SOM_APF_STATUS_OK)
		return MOD_SOM_APF_STATUS_ERR;
	return status;
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

//    mod_som_apf_status_t status = mod_som_apf_daq_status_f();
    mod_som_apf_status_t status;

    status=MOD_SOM_APF_STATUS_OK;
    // paramters for send_line_f()
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
    size_t reply_str_len = 0;
    LEUART_TypeDef* apf_leuart_ptr;
    apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
    uint32_t bytes_sent = 0;

//    if(mod_som_apf_ptr->daq){
//        status=mod_som_io_print_f("daq?,ack,%s","enabled");
//    }else{
//        status=mod_som_io_print_f("daq?,ack,%s","disabled");
//    }

    //ALB Dana want an error message here but I do not think there is a situation
    if (status!=MOD_SOM_APF_STATUS_OK){
        mod_som_io_print_f("daq?,nak,%lu",status);
        sprintf(apf_reply_str,"daq,nak,%s.\r\n","invalid daq cmd");
        reply_str_len = strlen(apf_reply_str);
        // sending the above string to the APF port - Mai - Nov 18, 2021
        bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
    }

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

    mod_som_apf_status_t status = mod_som_apf_poweroff_f();

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

///*******************************************************************************
// * @brief
// *   command shell for EpsiNo command
// *   set the SOM and EFE SN
// *   should return an apf status.
// *
// * @param argc
// *   argument count
// * @param argv
// *   argument values
// * @param out_put_f
// *   out_put_f (print function)
// * @param cmd_param
// *   command parameters (passing along)
// * @return
// *   apf Command Status
// ******************************************************************************/
//CPU_INT16S mod_som_apf_cmd_epsi_id_f(CPU_INT16U argc,
//        CPU_CHAR *argv[],
//        SHELL_OUT_FNCT out_put_f,
//        SHELL_CMD_PARAM *cmd_param){
//
//    mod_som_apf_status_t status = mod_som_apf_epsi_id_f();
//
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
//    return SHELL_EXEC_ERR_NONE;
//}


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
    mod_som_apf_status_t status = mod_som_apf_probe_id_f(argc,argv);

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

    mod_som_apf_status_t status = mod_som_apf_time_f(argc,argv);

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
 *   command shell for mod_som_apf_cmd_process_nfft_f
 *   set the the number of fourier coef in a segment
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
CPU_INT16S mod_som_apf_cmd_process_nfft_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = 0;

    if(status != MOD_SOM_APF_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_sd_format_f
 *   set the format of the data stored in the SD card
 *   0 = no format (latter on called SD0)
 *   1 = format 1 (SD1) time pressure epsilon chi fom dpdt kvis avg_t avg_s decimated avg spectra
 *   2 = format 2 (SD2) time pressure epsilon chi fom dpdt kvis avg_t avg_s full avg spectra
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
CPU_INT16S mod_som_apf_cmd_sd_format_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_sd_format_f(argc,argv);

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






#endif // MOD_SOM_APF_MODULE_SHELL_AVAIL


