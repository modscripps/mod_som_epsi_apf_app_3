/*******************************************************************************
 * @file mod_som_calendar.h
 * @brief MOD SOM calendar command shell API Implementation
 * @date Apr 01, 2020
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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
#include "mod_som_calendar_cmd.h"
#include "shell_util.h"
//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_calendar_cmd_tbl predefined command table of fixed size        */
//MHA change time.start to time.set
static SHELL_CMD  mod_som_calendar_cmd_tbl[] =
{
        { "time.set", mod_som_calendar_set_time_cmd_f },
        { "time.get",   mod_som_calendar_get_time_cmd_f },
        { 0, 0 }
};

/*******************************************************************************
 * @brief
 *   Initialize calendar  command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_calendar_init_cmd_f(){
    RTOS_ERR err;

    Shell_CmdTblAdd("CAL CMDs", mod_som_calendar_cmd_tbl, &err);

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_calendar_encode_status_f(MOD_SOM_CALENDAR_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_calendar_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   command set the time
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
CPU_INT16S mod_som_calendar_set_time_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

	RTOS_ERR  p_err;
    mod_som_status_t status = 0;

//    uint16_t year;
//    uint8_t month;
//    uint8_t month_day;
//    uint8_t hour;
//    uint8_t min;
//    uint8_t sec;

    sl_sleeptimer_timestamp_t time;
//    sl_sleeptimer_time_zone_offset_t time_zone;
//    sl_sleeptimer_date_t date;
    switch (argc){
    case 2:
    	time= shellStrtol(argv[1],&p_err);

//        time_zone=0;
//
//    	sl_sleeptimer_convert_time_to_date((uint32_t) time,time_zone,&date);
//
//    	  year=date.year;
//    	  month=date.month;
//    	  month_day=date.month_day;
//    	  hour=date.hour;
//    	  min=date.min;
//    	  sec=date.sec;

    	break;
//    case 7:
//    	year=shellStrtol(argv[1],&p_err);
//    	month=shellStrtol(argv[2],&p_err)-1;//month is defined as zero upward
//    	month_day=shellStrtol(argv[3],&p_err);
//    	hour=shellStrtol(argv[4],&p_err);
//    	min=shellStrtol(argv[5],&p_err);
//    	sec=shellStrtol(argv[6],&p_err);
//
//    	break;
    default:
    	printf("..Wrong format either posix time (second since 1970)\r");
    	break;
    }
    mod_som_calendar_set_time_f(time);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command get the time
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
CPU_INT16S mod_som_calendar_get_time_cmd_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_status_t status = 0;
    sl_sleeptimer_date_t date;
    char *format="%Y-%m-%d %H:%M:%S";
    char str_date[50];

    switch (argc){
    case 1:
      sl_sleeptimer_get_datetime(&date);
      sl_sleeptimer_convert_date_to_str(str_date,50,(uint8_t*)format,&date);
      mod_som_io_print_f(" %s",str_date);

      break;
    default:
      printf("..format cal.get  ... it will give the current data \r\n");
      break;
    }

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

