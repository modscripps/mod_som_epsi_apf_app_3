/*
 * mod_som_apf_cmd.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


// testing for github - maibui 10 May 2022

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <apf/mod_som_apf.h>


#ifdef RTOS_MODULE_COMMON_SHELL_AVAIL
#include <apf/mod_som_apf_cmd.h>
#include "mod_som_io.h"
#include  <common/source/shell/shell_priv.h>

#include <ctype.h>
#include <string.h>
//ALB

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_apf_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_apf_cmd_tbl[] =
{
        { MOD_SOM_APF_DAQ_STR,               mod_som_apf_cmd_daq_f },
        { MOD_SOM_APF_DAQSTAT_STR,           mod_som_apf_cmd_daq_status_f },
        { MOD_SOM_APF_TIME_STR,              mod_som_apf_cmd_time_f },
        { MOD_SOM_APF_TIMESTAT_STR,          mod_som_apf_cmd_time_status_f },
        { MOD_SOM_APF_OKSTAT_STR,            mod_som_apf_cmd_ok_status_f },
        { MOD_SOM_APF_SLEEP_STR,             mod_som_apf_cmd_sleep_f },
        { MOD_SOM_APF_GATE_STR,              mod_som_apf_cmd_gate_f }, // turn on/off the main menu
        { MOD_SOM_APF_FWREV_STAT_STR,        mod_som_apf_cmd_fwrev_status_f },
        { MOD_SOM_APF_UPLOAD_STR,            mod_som_apf_cmd_upload_f },
        { MOD_SOM_APF_EPSINO_STAT_STR,       mod_som_apf_cmd_epsi_id_status_f },
        { MOD_SOM_APF_PROBENO_STR,           mod_som_apf_cmd_probe_id_f },
        { MOD_SOM_APF_PROBENO_STAT_STR,      mod_som_apf_cmd_probe_id_status_f },
        { MOD_SOM_APF_POWEROFF_STR,          mod_som_apf_cmd_poweroff_f },
        { MOD_SOM_APF_SDFORMAT_STAT_STR,     mod_som_apf_cmd_sd_format_status_f },
        { MOD_SOM_APF_SDFORMAT_STR,          mod_som_apf_cmd_sd_format_f },
        { MOD_SOM_APF_PACKETFORMAT_STAT_STR, mod_som_apf_cmd_packet_format_status_f },
        { MOD_SOM_APF_PACKETFORMAT_STR,      mod_som_apf_cmd_packet_format_f },
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
mod_som_status_t mod_som_apf_init_cmd_f(){

    RTOS_ERR err;
    Shell_CmdTblAdd("APF CMDs", mod_som_apf_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_ADD_CMD_TBL);
    }
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
#define MOD_SOM_APF_DAQ_PROFILE_LIMIT 65534
//uint64_t last_profile_id = 0;

CPU_INT16S mod_som_apf_cmd_daq_f(CPU_INT16U argc,
    CPU_CHAR *argv[],
    SHELL_OUT_FNCT out_put_f,
    SHELL_CMD_PARAM *cmd_param){

  mod_som_apf_status_t status = MOD_SOM_APF_STATUS_OK;
  mod_som_apf_status_t return_status;
  uint64_t profile_id = 0;

  // paramters for send_line_f()
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
  // get the port's fd
  uint32_t bytes_sent = 0;
  CPU_INT16U i = argc;

  char third_arg[125] = "\0";
//  char daq_valid_cmd[] = "valid command: \"daq,stop\" or \"daq,start,profile_id_positive_int_below_65534\"";
//  char daq_stop_valid_cmd[] = "valid command: \"daq,stop\"r\n";
//  char daq_start_valid_cmd[] = "valid command: \"daq,start,profile_id_positive_int_below_65534\"";
//  char input_cmd[125] = "\0";
//  char err_str[125] = "\0";

  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else{

  switch(argc)
  {
    case 1: // input: "daq" -- invalid command, not enough information
      // save time string into the temporary local string - Mai - Nov 18, 2021
      //ALB changing to %s,%s
      sprintf(apf_reply_str,"%s,%s,not enough arguments\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
    case 2: // input: "daq,stop", if "daq,start" or "daq,somethingelse" -> error
      i = 1; // get the second argument
      if (!Str_Cmp(argv[i], "stop"))  // "daq,stop" ==> execute mod_som_apf_daq_stop_f() routine
      {
         return_status = mod_som_apf_daq_stop_f();
         //ALB display msg
         if (return_status == MOD_SOM_APF_STATUS_OK)
         {
             // save time string into the temporary local string - Mai - Nov 18, 2021
             //ALB changing to %s,%s
             sprintf(apf_reply_str,"%s,stop,%s\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_ACK_STR);
             status |= return_status;
             break;
         }
      }
      else if (!Str_Cmp(argv[i], "start"))  // "daq,start" => missing profile id
      {
          // save time string into the temporary local string - Mai - Dec 3, 2021
          sprintf(apf_reply_str,"%s,start,%s,not enough arguments\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
          break;
      } // end of if (!Str_Cmp(argv[i], "start"))
      else  // not "daq stop" nor "daq start"
      {
              // save time string into the temporary local string - Mai - Nov 18, 2021
              sprintf(apf_reply_str,"%s,%s,invalid input(s)\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
              status |= MOD_SOM_APF_STATUS_ERR;
              break;
      }
      break;
    case 3: // command: "daq,start,proid"
      i = 1; // get the second argument
      if (!Str_Cmp(argv[i], "stop"))  // daq stop arg => wrong stop command
      {
          // save time string into the temporary local string - Mai - Nov 18, 2021
          sprintf(apf_reply_str,"%s,stop,%s,too many arguments\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_ERR;
          break;
      }
      else if (!Str_Cmp(argv[i], "start"))  // "daq start ..." commnand
      {
          // copy to the temp
          strcpy(third_arg,argv[i+1]);
          // detect for not interger, only need check the first element of the third argument
          if(isalpha(third_arg[0])) // daq start not_integer
          {
              // save time string into the temporary local string - Mai - 11 May, 2022
              sprintf(apf_reply_str,"%s,start,%s,invalid input(s)\r\n",
                      MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
              status |= MOD_SOM_APF_STATUS_WRONG_ARG;
              break;
          }
//        profile_id = shellStrtol(argv[i+1], &err); // Convert the third argument to int to get proid
          // get value of argv
          profile_id = strtol(argv[i+1],NULL,10); // get value of the third arg
          //printf("profile id: %s\n",argv[i+1]);

          // profile id is out of the range
          //if (profile_id < 0 |  | profile_id > MOD_SOM_APF_DAQ_PROFILE_LIMIT)
          // 2023 12 13 add to make sure we don't have profile id 0 problem
          if (profile_id < 1 || profile_id > MOD_SOM_APF_DAQ_PROFILE_LIMIT)
          {
                 // save time string into the temporary local string - Mai - Nov 18, 2021
                 sprintf(apf_reply_str,"%s,start,%s,invalid input(s)\r\n",
                         MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
                 status |= MOD_SOM_APF_STATUS_WRONG_ARG;
                 break;
          }

          // CAP removing the continuous profile id condition
          // check the profile_id is continous
//          if (profile_id - local_apf_runtime_ptr->profile_id != 1)
//            {
//               // save time string into the temporary local string - Mai - Nov 18, 2021
//              sprintf(apf_reply_str,"%s,start,%s,wrong profile id,input profile is not continous,last profile id is %lu, must be %lu\r\n",
//                      MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR,
//                      (uint32_t) local_apf_runtime_ptr->profile_id,
//                      (uint32_t) local_apf_runtime_ptr->profile_id+1);
//              status |= MOD_SOM_APF_STATUS_WRONG_ARG;
//              break;
//            }
          // all the bad input are detected, now only have valid command - maibui Aprl 28, 2022
//          last_profile_id = profile_id;  // update the frofile id
          //ALB STARTING THE PROFILE HERE
          return_status = mod_som_apf_daq_start_f((uint32_t)profile_id); // execute start()

          if (return_status == MOD_SOM_APF_STATUS_NO_CTD_DATA)
          {
              // save time string into the temporary local string - Mai - Nov 18, 2021
              sprintf(apf_reply_str,"%s,start,%s,no valid CTD data\r\n",
                      MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
              mod_som_apf_status_t temp_status = mod_som_apf_daq_stop_f();
              status |= return_status;
              break;
          }
          // save time string into the temporary local string - Mai - Nov 18, 2021
          //ALB THIS THE ACK ANSWER
          sprintf(apf_reply_str,"%s,start,%s,%lu\r\n",
                  MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_ACK_STR,
                  profile_id);
          status |= MOD_SOM_APF_STATUS_OK;
        } // if (!Str_Cmp(argv[argc], "start"))
      else  // not "daq stop" nor "daq start"
        {
          // save time string into the temporary local string - Mai - Nov 18, 2021
          sprintf(apf_reply_str,"%s,%s,invalid input(s)\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_ERR;
          break;
        }
      break;
    default:  // more than 3 argc
      i = 1;
      /*
      // save time string into the temporary local string - Mai - Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,invalid input(s)\r\n",
              MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
             // */
      // 2024 01 22 SAN update to have proper replies to different cases
      if (!Str_Cmp(argv[i], "stop"))  // daq stop arg => wrong stop command
        {
          // save time string into the temporary local string - Mai - Nov 18, 2021
          sprintf(apf_reply_str,"%s,stop,%s,too many arguments\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_ERR;
          break;
        }
      else if (!Str_Cmp(argv[i], "start"))  // "daq,start" => missing profile id
        {
          // save time string into the temporary local string - Mai - Dec 3, 2021
          sprintf(apf_reply_str,"%s,start,%s,too many arguments\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
          break;
        } // end of if (!Str_Cmp(argv[i], "start"))
      else  // not "daq stop" nor "daq start"
        {
          // save time string into the temporary local string - Mai - Nov 18, 2021
          sprintf(apf_reply_str,"%s,%s,invalid input(s)\r\n",MOD_SOM_APF_DAQ_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_ERR;
          break;
        }
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
  }
    // sending to the screen - Mai- May 11, 2022
   return_status = mod_som_io_print_f("%s",apf_reply_str);
   if (return_status != MOD_SOM_STATUS_OK)
   {
       // save time string into the temporary local string - Mai - Nov 18, 2021
       mod_som_io_print_f("Failed from mod_som_io_print_f()\r\n");
       status |= MOD_SOM_APF_STATUS_ERR;
   }
   reply_str_len = strlen(apf_reply_str);
   // sending the above string to the APF port - Mai - Nov 18, 2021
   bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
   if (bytes_sent==0)
   {
     mod_som_io_print_f("Failed on mod_som_apf_send_line_f\r\n");
     status |= MOD_SOM_APF_STATUS_ERR;
   }
//   if(status != MOD_SOM_APF_STATUS_OK)
//       return MOD_SOM_APF_STATUS_ERR;
  }
  return SHELL_EXEC_ERR_NONE;
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

    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,too many arguments\r\n",
                MOD_SOM_APF_DAQSTAT_STR,MOD_SOM_APF_NACK_STR);
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{

    if(mod_som_apf_get_daq_f()){
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_DAQSTAT_STR,MOD_SOM_APF_ACK_STR,"enabled");
    }else{
         sprintf(apf_reply_str,"%s,%s,%s\r\n",
                 MOD_SOM_APF_DAQSTAT_STR,MOD_SOM_APF_ACK_STR,"disabled");
   }  // mnbui Nov 29, 2021

    //ALB Dana want an error message here but I do not think there is a situation
    if (status!=MOD_SOM_APF_STATUS_OK){
         sprintf(apf_reply_str,"%s,%s,%lu.\r\n",
                 MOD_SOM_APF_DAQSTAT_STR,MOD_SOM_APF_NACK_STR,status);
     }

    status=mod_som_io_print_f("%s",apf_reply_str);
    reply_str_len = strlen(apf_reply_str);
    // sending the above string to the APF port - Mai - Nov 18, 2021
    bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
    if (bytes_sent==reply_str_len){
        status = MOD_SOM_APF_STATUS_OK;
    }
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  //uint32_t bytes_sent;
  LEUART_TypeDef* apf_leuart_ptr;

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_FWREV_STAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
      reply_str_len = strlen(apf_reply_str);
      apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      //            if (bytes_sent==reply_str_len){
      //                status = MOD_SOM_APF_STATUS_OK;
      //            }
      return SHELL_EXEC_ERR_NONE;
  }else{

    status = mod_som_apf_fwrev_status_f();
  }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

    mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
    size_t reply_str_len = 0;
    //uint32_t bytes_sent;
    LEUART_TypeDef* apf_leuart_ptr;

    if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_OKSTAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }
    status = mod_som_apf_ok_status_f();
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell for PowerOff command
 *   prepare SOM to futur power off
 *    stop efe sampling
 *    stop sbe41 communication
 *    stop efeobp fill segment task
 *    stop efeobp compute spectra task
 *    stop efeobp compute dissrate task
 *    stop afp    producer task
 *    stop afp    consumer task
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

  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  //uint32_t bytes_sent;
  LEUART_TypeDef* apf_leuart_ptr;

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_POWEROFF_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{
        status = mod_som_apf_poweroff_f();
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
    size_t reply_str_len = 0;
    //uint32_t bytes_sent;
    LEUART_TypeDef* apf_leuart_ptr;

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_EPSINO_STAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{
        status = mod_som_apf_epsi_id_status_f();
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else{
        status = mod_som_apf_probe_id_f(argc,argv);
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
        size_t reply_str_len = 0;
        //uint32_t bytes_sent;
        LEUART_TypeDef* apf_leuart_ptr;

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_PROBENO_STAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
      reply_str_len = strlen(apf_reply_str);
      apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      //            if (bytes_sent==reply_str_len){
      //                status = MOD_SOM_APF_STATUS_OK;
      //            }
      return SHELL_EXEC_ERR_NONE;
  }else{
      status = mod_som_apf_probe_id_status_f();
  }
//  if(status != MOD_SOM_APF_STATUS_OK)
//    return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  //uint32_t bytes_sent;
  LEUART_TypeDef* apf_leuart_ptr;

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_SLEEP_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
      reply_str_len = strlen(apf_reply_str);
      apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      //            if (bytes_sent==reply_str_len){
      //                status = MOD_SOM_APF_STATUS_OK;
      //            }
      return SHELL_EXEC_ERR_NONE;
  }else{
      status = mod_som_apf_sleep_f();
  }
//  if(status != MOD_SOM_APF_STATUS_OK)
//    return SHELL_EXEC_ERR;
  return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell for gate command
 *   start or stop MOD shell and turn off 232 driver
 *   should return an apf status.
 *   gate,on\r\n
 *   gate,off\r\n
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
CPU_INT16S mod_som_apf_cmd_gate_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_apf_status_t status = mod_som_apf_gate_f(argc,argv);

//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else{
      status = mod_som_apf_time_f(argc,argv);
  }
//  if(status != MOD_SOM_APF_STATUS_OK)
//    return SHELL_EXEC_ERR;
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


  mod_som_apf_status_t status;
  mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
        size_t reply_str_len = 0;
        //uint32_t bytes_sent;
        LEUART_TypeDef* apf_leuart_ptr;

  if (local_apf_runtime_ptr->sleep_flag){
      status=MOD_SOM_APF_STATUS_OK;
  }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_TIMESTAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
      reply_str_len = strlen(apf_reply_str);
      apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      //            if (bytes_sent==reply_str_len){
      //                status = MOD_SOM_APF_STATUS_OK;
      //            }
      return SHELL_EXEC_ERR_NONE;
  }else{
      status = mod_som_apf_time_status_f();
  }
//  if(status != MOD_SOM_APF_STATUS_OK)
//    return SHELL_EXEC_ERR;
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
CPU_INT16S mod_som_apf_cmd_packet_format_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else{
    status = mod_som_apf_packet_format_f(argc,argv);
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

CPU_INT16S mod_som_apf_cmd_packet_format_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_apf_status_t status = SHELL_EXEC_ERR_NONE;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
          size_t reply_str_len = 0;
          //uint32_t bytes_sent;
          LEUART_TypeDef* apf_leuart_ptr;

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_PACKETFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{
        status = mod_som_apf_packet_format_status_f(argc,argv);
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//      return SHELL_EXEC_ERR;
    return status;
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

//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else{
        status = mod_som_apf_sd_format_f(argc,argv);
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

CPU_INT16S mod_som_apf_cmd_sd_format_status_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
      size_t reply_str_len = 0;
      //uint32_t bytes_sent;
      LEUART_TypeDef* apf_leuart_ptr;


    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                         MOD_SOM_APF_SDFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{
    status = mod_som_apf_sd_format_status_f(argc,argv);
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
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

  mod_som_apf_status_t status;
    mod_som_apf_ptr_t local_apf_runtime_ptr = mod_som_apf_get_runtime_ptr_f();
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
      size_t reply_str_len = 0;
      //uint32_t bytes_sent;
      LEUART_TypeDef* apf_leuart_ptr;

    if (local_apf_runtime_ptr->sleep_flag){
        status=MOD_SOM_APF_STATUS_OK;
    }else if(argc>1){ //SAN 2023 02 16 added to ensure zero arguments
        sprintf(apf_reply_str,"%s,%s,%s\r\n",
                MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,"too many arguments");
        reply_str_len = strlen(apf_reply_str);
        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        //            if (bytes_sent==reply_str_len){
        //                status = MOD_SOM_APF_STATUS_OK;
        //            }
        return SHELL_EXEC_ERR_NONE;
    }else{
        status = mod_som_apf_upload_f();
    }
//    if(status != MOD_SOM_APF_STATUS_OK)
//        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


#endif // MOD_SOM_APF_MODULE_SHELL_AVAIL


