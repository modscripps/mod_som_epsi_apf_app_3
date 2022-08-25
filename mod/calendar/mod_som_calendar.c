/*
 * mod_som_food_bar.c
 *
 *  Created on: August 25, 2020
 *      Author: aleboyer
 */
#include "mod_som_calendar.h"
#include "mod_som_io.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_calendar_cmd.h"
#endif

#include "sl_sleeptimer.h"

#ifdef MOD_SOM_SETTINGS_EN
#include "mod_som_settings.h"
#endif


mod_som_calendar_ptr_t mod_som_calendar_ptr;
// char start_date: "%Y-%m-%d %H:%M:%S"

/*******************************************************************************
 * @brief
 *   Initialize calendar module, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_calendar_init_f(){
	sl_status_t status;
	RTOS_ERR  err;

	//ALB initialize the shell cmds
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_calendar_init_cmd_f();
    status = 0;
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_calendar_encode_status_f(
                                         MOD_SOM_CALENDAR_STATUS_FAIL_INIT_CMD);
#endif


	//ALB allocate memory of the calendar module ptr
	mod_som_calendar_ptr = (mod_som_calendar_ptr_t)Mem_SegAlloc(
			"MOD SOM CALEND Memory",DEF_NULL,
			sizeof(mod_som_calendar_t),
			&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_calendar_ptr==DEF_NULL)
		return -1;

	//ALB allocate memory of the calendar module setup ptr
	mod_som_calendar_ptr->mod_som_calendar_settings_ptr =
	    (mod_som_calendar_settings_ptr_t)Mem_SegAlloc(
			"MOD SOM CALEND SETUP Memory",DEF_NULL,
			sizeof(mod_som_calendar_settings_t),
			&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_calendar_ptr->mod_som_calendar_settings_ptr==DEF_NULL)
		return -1;

	mod_som_calendar_ptr->mod_som_calendar_settings_ptr->poweron_offset_ms=0; //MHA: default offset is 0
	mod_som_calendar_ptr->initialized_flag=false;


    mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initialize_flag=false;

#ifdef MOD_SOM_SETUP_EN
    mod_som_settings_struct_ptr_t local_settings_ptr=
                                              mod_som_settings_get_settings_f();
    mod_som_calendar_ptr->mod_som_calendar_settings_ptr=
                                    &local_settings_ptr->mod_som_calendar_setup;
#endif

    if (!mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initialize_flag){
    	mod_som_calendar_default_settings_f();
    }

     mod_som_calendar_ptr->initialized_flag=1;

    return mod_som_calendar_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   set the default setup struct.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_calendar_default_settings_f(){

	sl_status_t status;

	sprintf(mod_som_calendar_ptr->mod_som_calendar_settings_ptr->header,
	        "%s",
	        "CAL");

	status=sl_sleeptimer_build_datetime(
	          &mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date,\
                                                              1970,0,0,0,0,0,0);
	// initialization of the start date
    status=sl_sleeptimer_set_datetime(
           & mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date);

    mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initialize_flag=true;
    mod_som_calendar_ptr->mod_som_calendar_settings_ptr->size=
                   sizeof(*mod_som_calendar_ptr->mod_som_calendar_settings_ptr);

    return mod_som_calendar_encode_status_f(status);
}

/*******************************************************************************
 * @brief
 *   set the calendar setup struct.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_calendar_settings_f(){

	sl_status_t status;

	sprintf( mod_som_calendar_ptr->mod_som_calendar_settings_ptr->header,"%s",
	                                                                  "CALENDAR");

	status=sl_sleeptimer_build_datetime(
	          &mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date,\
                                                              1970,0,0,0,0,0,0);
    // initialization of the start date
    status=sl_sleeptimer_set_datetime(
            &mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date);
    mod_som_calendar_ptr->mod_som_calendar_settings_ptr->size=
                   sizeof( mod_som_calendar_ptr->mod_som_calendar_settings_ptr);

    return mod_som_calendar_encode_status_f(status);
}


/*******************************************************************************
 * @brief
 *   get calendar setup struct pointer
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_calendar_settings_t mod_som_calendar_get_settings_f(){
	return *mod_som_calendar_ptr->mod_som_calendar_settings_ptr;
}
/*******************************************************************************
 * @brief
 *   get calendar setup struct pointer
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_calendar_settings_ptr_t mod_som_calendar_get_settings_ptr_f(){
  return mod_som_calendar_ptr->mod_som_calendar_settings_ptr;
}

/*******************************************************************************
 * @brief
 *   set calendar setup struct pointer
 *   the setup module gets the new calendar_setup from SD or other com port
 *   this function should memcopy the data from that new setup to the
 *   mod_som_calendar_settings_t
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_calendar_set_settings_f(mod_som_calendar_settings_ptr_t setup_ptr){
	//mod_som_calendar_ptr->mod_som_calendar_settings_ptr=setup_ptr;
}


/*******************************************************************************
 * @brief
 *   get wall clock time stamp and
 *   convert it in char * with format "%Y%m%d_%H%M%S"
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
char * mod_som_calendar_get_datetime_f(){
//	char buff[20];
	char * format="%Y%m%d_%H%M%S";
	sl_sleeptimer_get_datetime(&mod_som_calendar_ptr->sl_current_date);
	sl_sleeptimer_convert_date_to_str(mod_som_calendar_ptr->buff,20,
	                                  (uint8_t*) format,
									                  &mod_som_calendar_ptr->sl_current_date);
	return mod_som_calendar_ptr->buff;
}
/*******************************************************************************
 * @brief
 *   get wall clock time stamp in sec.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
sl_sleeptimer_timestamp_t mod_som_calendar_get_time_f(){
//	char buff[20];
	sl_sleeptimer_timestamp_t time;
//	sl_sleeptimer_date_t      adate;//,adate_1970;
	time = sl_sleeptimer_get_time();
//	//get timestamp for 1970
//      sl_sleeptimer_build_datetime(
//                  &adate,\
//                   1970,0,1,0,0,0,0); //MHA: month starts with 0, day starts with 1, HMS all start with 0
//  sl_sleeptimer_convert_date_to_time(&adate, &atime);
//
//  time-=atime;

	return time;
}

/*******************************************************************************
 * @brief
 *   set the initial date
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
sl_status_t  mod_som_calendar_set_time_f(sl_sleeptimer_timestamp_t time){

  sl_status_t status = SL_STATUS_OK;

  //MHA month is defined as zero upwards, so add one to it so user enters dates in familiar format
//  sl_status_t status=sl_sleeptimer_build_datetime(
//      &mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date,\
//      year,month,month_day,hour,min,sec,tz_offset);
  // initialization of the start date
//  status=sl_sleeptimer_set_datetime(
//      &mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date);
  status = sl_sleeptimer_set_time(time);
  sl_sleeptimer_get_datetime(&mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date);
  //***************************************************************
  //MHA New timestamp convention 6/21/2021.  Previously, timestamp was the number of milliseconds
  // since power on.  We now compute the number of milliseconds at power on since 1/1/1970, 0:0:0,
  // and add that offset to the time stamp.  Hence timestamp is now 1000*unix time.

  //MHA Now we need to compute the number of seconds since power on.
  //MHA The offset we add then to the timestamp will be that times 1000.
  uint64_t tick,settime_delay,offset_ms; //MHA: tick is 50-MHz ticks since power on; settime_delay is ms since power on.
  //MHA: offset_ms is the number of millisec of power on since 1/1/1970 (1000*unix time).
//  sl_status_t mystatus;
  //&mod_som_calendar_ptr->poweron_offset_ms


  tick=sl_sleeptimer_get_tick_count64();
  status = sl_sleeptimer_tick64_to_ms(tick,\
                                      &settime_delay);

  //MHA settime_delay now contains the number of ms since poweron.

  //Next get the number of ms since Jan 1, 1970.

  //First make variables for the timestamps of the
//  sl_sleeptimer_timestamp_t atime = 0u;//,time_1970=0u;
//  sl_status_t err_code = SL_STATUS_OK;
  sl_sleeptimer_date_t adate;//,adate_1970;
  //     if (!is_valid_date(date)) {
  //       return SL_STATUS_INVALID_PARAMETER;
  //     }

//  adate=mod_som_calendar_ptr->mod_som_calendar_settings_ptr->initial_date;

  //get timestamp for 1970
  //status=sl_sleeptimer_build_datetime(
  //            &adate_1970,\
  //                                   1970,0,1,0,0,0,0); //MHA: month starts with 0, day starts with 1, HMS all start with 0

//  status = sl_sleeptimer_convert_date_to_time(&adate, &atime);
  //err_code = sl_sleeptimer_convert_date_to_time(&adate_1970, &time_1970);

  //MHA: OK, confirmed that the timestamp is in fact relative to Jan 1, 1970 0:0:0 which is month 0, day 1, 0:0:0.

  //So the next step is to compute the timestamp in secs at power on.
  offset_ms=(uint64_t)1000 * time - settime_delay;

  //MHA: great, now the offset is stored in poweron_offset_ms in the calendar structure.
  mod_som_calendar_ptr->mod_som_calendar_settings_ptr->poweron_offset_ms=offset_ms;

  // debug - maibui
  printf(".... offset_ms = %d\n", mod_som_calendar_ptr->mod_som_calendar_settings_ptr->poweron_offset_ms);
  return status;
}


/*******************************************************************************
 * @function
 *     mod_som_io_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the bit 16-23 if the
 *     higher bits show the status code is of MOD SOM FOO BAR
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_calendar_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_CALENDAR_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_foo_bar_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
 *     lower 16 bits are reserved for flags
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_calendar_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_CALENDAR_STATUS_PREFIX,
                                 mod_som_io_status);
}


