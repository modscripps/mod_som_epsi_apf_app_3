/*
 * mod_som_apf.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


#include <apf/mod_som_apf.h>

#ifdef MOD_SOM_APF_EN

#include "mod_som_io.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <apf/mod_som_apf_cmd.h>

#endif

#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif


mod_som_apf_ptr_t mod_som_apf_ptr;

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_init_f(){
    mod_som_apf_status_t status;
    RTOS_ERR  err;

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_apf_init_cmd_f();
    if(status != MOD_SOM_APF_STATUS_OK)
        return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_INIT_CMD);
#endif

    //ALB allocate memory for the module_ptr.
    //ALB The module_ptr is also the "scope" of the runtime_ptr
    //ALB but the module_ptr also contains the settings_ptr and the config_ptr
    //ALB The settings_ptr an config_ptr should allocated and defined
    //ALB during the module initialization
    mod_som_apf_ptr = (mod_som_apf_ptr_t)Mem_SegAlloc(
        "MOD SOM APF RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_apf_t),
        &err);

    //SN Check error code
    //ALB WARNING: The following line hangs in a while loop -
    //ALB WARNING: - if the previous allocation fails.
    //ALB TODO change return -1 to return the an appropriate error code.
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_apf_ptr==DEF_NULL){
      printf("APF not initialized\n");
      return -1;
    }

    //ALB Initialize the runtime flag module_ptr->initialized_flag to false.
    //ALB It will be set to true once the module is initialized at the
    //ALB end of mod_som_efe_init_f().
    mod_som_apf_ptr->initialize_flag = false;

    // ALB allocate memory for the settings_ptr.
    // ALB WARNING: The setup pointer CAN NOT have pointers inside.
    status |= mod_som_apf_allocate_settings_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("APF not initialized\n");
      return status;
    }


    // ALB checking if a previous EFE OBP setup exist, from the setup module
    //ALB (i.e. setup file or UserData setup)
  #ifdef MOD_SOM_SETTINGS_EN
      mod_som_settings_struct_ptr_t local_settings_ptr=
                                              mod_som_settings_get_settings_f();
      mod_som_apf_ptr->settings_ptr=
                                  &local_settings_ptr->mod_som_apf_settings;
  #else
      mod_som_apf_ptr->settings_ptr->initialize_flag=false;
  #endif

    // ALB If no pre-existing settings, use the default settings
      if (!mod_som_apf_ptr->settings_ptr->initialize_flag){
        // initialize the setup structure.
        status |= mod_som_apf_default_settings_f(
                                             mod_som_apf_ptr->settings_ptr);
        if (status!=MOD_SOM_STATUS_OK){
          printf("APF not initialized\n");
          return status;
        }
      }

      //ALB Allocate memory for the config pointer,
      //ALB using the settings_ptr variable
      status |= mod_som_apf_construct_config_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("APF not initialized\n");
      return status;
    }

    //ALB initialize mod_som_apf_ptr params
    mod_som_apf_ptr->profile_id=0;
    mod_som_apf_ptr->daq=false;


    return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   construct settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_allocate_settings_ptr_f(){

  RTOS_ERR  err;

  //ALB alloc memory for setup pointer
  //set up default configuration
  mod_som_apf_ptr->settings_ptr =
      (mod_som_apf_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM APF OBP setup.",DEF_NULL,
          sizeof(mod_som_apf_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_apf_ptr->settings_ptr==NULL)
  {
    mod_som_apf_ptr = DEF_NULL;
    return MOD_SOM_APF_OBP_CANNOT_ALLOCATE_SETUP;
  }


  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   Initialize setup struct ptr
 *   uint32_t size;
 *   char header[MOD_SOM_APF_SETTINGS_STR_lENGTH];
 *   uint32_t nfft;
 *   uint32_t record_format;
 *   uint32_t telemetry_format;
 *   uint32_t initialize_flag;
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_apf_default_settings_f(
                                    mod_som_apf_settings_ptr_t settings_ptr)
{
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  strncpy(settings_ptr->header,
          MOD_SOM_APF_HEADER,MOD_SOM_APF_SETTINGS_STR_lENGTH);
  settings_ptr->initialize_flag=true;
  settings_ptr->size=sizeof(*settings_ptr);

  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_apf_settings_t mod_som_apf_get_settings_f(){
  return *mod_som_apf_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_config_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for config pointer
  mod_som_apf_ptr->config_ptr =
      (mod_som_apf_config_ptr_t)Mem_SegAlloc(
          "MOD SOM APF config.",DEF_NULL,
          sizeof(mod_som_apf_config_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_apf_ptr->config_ptr==NULL)
  {
    mod_som_apf_ptr = DEF_NULL;
    return MOD_SOM_APF_CANNOT_OPEN_CONFIG;
  }

  mod_som_apf_config_ptr_t config_ptr = mod_som_apf_ptr->config_ptr;
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  config_ptr->initialized_flag = false;
  config_ptr->header_length    = MOD_SOM_APF_SYNC_LENGTH            +
                                 MOD_SOM_APF_TAG_LENGTH             +
                                 MOD_SOM_APF_HEXTIMESTAMP_LENGTH    +
                                 MOD_SOM_APF_PAYLOAD_LENGTH         +
                                 MOD_SOM_APF_HEADER_CHECKSUM_LENGTH;


  config_ptr->initialized_flag = true;
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @function
 *     mod_som_io_decode_status_f
 * @abstract
 *     Decode mod_som_apf_status to status of MOD SOM APF error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the bit 16-23 if the
 *     higher bits show the status code is of MOD SOM FOO BAR
 * @param       mod_som_apf_status
 *     MOD SOM general system-wide status
 * @return
 *     0xff if non-MOD SOM APF status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_apf_decode_status_f(mod_som_apf_status_t mod_som_apf_status){
    if(mod_som_apf_status==MOD_SOM_APF_STATUS_OK)
        return MOD_SOM_APF_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_apf_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_APF_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_apf_encode_status_f
 * @abstract
 *     Encode status of MOD SOM APF error code to MOD APF SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD APF SOM general status, where
 *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
 *     lower 16 bits are reserved for flags
 * @param       mod_som_io_status
 *     MOD SOM APF error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_encode_status_f(uint8_t mod_som_apf_status){
    if(mod_som_apf_status==MOD_SOM_APF_STATUS_OK)
        return MOD_SOM_APF_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_APF_STATUS_PREFIX, mod_som_apf_status);
}



//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//ALB start apf functions called by the apf commands
/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_say_hello_world_f(){
    mod_som_io_print_f("[apf]: hello world\r\n");
    return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   Daq_start function
 *   start Data acquisition.
 *   It should start the EFE adc master clock
 *   start the turbulence processing task
 *   and store the data
 *   it should store and return a Daq Status
 *   (e.g., Daq enable/disable)
 *
 * @param ProfileId
 *   argument values
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_apf_status_t mod_som_apf_daq_start_f(uint64_t profile_id){
  mod_som_status_t status;
  status=MOD_SOM_APF_STATUS_OK;
  uint32_t delay=0xFF;

	//ALB start ADC master clock timer
  mod_som_apf_ptr->profile_id=profile_id;
  mod_som_apf_ptr->daq=true;
	//ALB start turbulence processing task
  status=mod_som_efe_obp_start_fill_segment_task_f();
  status|=mod_som_efe_obp_start_cpt_spectra_task_f();
  status|=mod_som_efe_obp_start_cpt_dissrate_task_f();

  //ALB start collecting CTD
  status |= mod_som_sbe41_connect_f();
  status |= mod_som_sbe41_start_collect_data_f();

  while (delay>0){
      delay--;
  }

  status|=mod_som_efe_sampling_f();


  //ALB output good behavior
  if (status==0){
      mod_som_io_print_f("daq,ack,%lu\r\n",(uint32_t) profile_id);
  }else{
      mod_som_io_print_f("daq,nak,%lu\r\n",status);
  }

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   Daq_stop function
 *   stop Data acquisition.
 *   It should stop the EFE adc master clock
 *   stop the turbulence processing task
 *   and store the data
 *   it should store and return a Daq Status
 *   (e.g., Daq enable/disable)
 *
 * @param ProfileId
 *   argument values
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_apf_status_t mod_som_apf_daq_stop_f(){
  mod_som_status_t status;
  status=MOD_SOM_APF_STATUS_OK;

	// stop ADC master clock timer
  status|=mod_som_efe_sampling_f();


  // stop collecting CTD data
  status|=mod_som_sbe41_stop_collect_data_f();
  status|=mod_som_sbe41_disconnect_f();

	// stop turbulence processing task
  status=mod_som_efe_obp_stop_fill_segment_task_f();
  status|=mod_som_efe_obp_stop_cpt_spectra_task_f();
  status|=mod_som_efe_obp_stop_cpt_dissrate_task_f();

  //reset Daq flag
  mod_som_apf_ptr->daq=false;
//ALB display msg
  if (status==MOD_SOM_APF_STATUS_OK){
      mod_som_io_print_f("daq,stop,ack\r\n","stop");
  }else{
      mod_som_io_print_f("daq,stop,nak,%lu\r\n",status);

  }

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   command shell for Daq?  command
 *   tell apf if Daq is enable or disable
 *   it should return an error if can not access to the information
 *
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_apf_status_t mod_som_apf_daq_status_f(){
  mod_som_status_t status;
  status=MOD_SOM_APF_STATUS_OK;

  if(mod_som_apf_ptr->daq){
      status=mod_som_io_print_f("daq?,ack,%s\r\n","enabled");
  }else{
      status=mod_som_io_print_f("daq?,ack,%s\r\n","disabled");
  }

  //ALB Dana want an error message here but I do not think there is a situation
  if (status!=MOD_SOM_APF_STATUS_OK){
      mod_som_io_print_f("daq?,nak,%lu\r\n",status);
  }
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for FubarCal command
 *   run FubarCal cmd with arguments arg1, arg2, ..., argn
 *   it should return a Fubar status
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fubar_f(){

	mod_som_io_print_f("FuBar,ack,%s,%s,%s\r\n","param1","param2","param3");
	mod_som_io_print_f("FuBar,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for FubarCal? command
 *   display Fubar status
 *   it should return an error if can not access to the information
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fubar_status_f(){

	mod_som_io_print_f("FuBar?,ack,%s.\r\n","status report");
	mod_som_io_print_f("FuBar?,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for FwRev? command
 *   display Firmware Revision ID
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fwrev_status_f(){

	mod_som_io_print_f("FuBar?,ack,%s.\r\n","status report");
	mod_som_io_print_f("FuBar?,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for ok? command
 *   wake up SOM and display apf status
 *   if nothing happens after 30 sec go back to sleep
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_ok_status_f(){

	// Wake up sensor's interface.
	//
	mod_som_io_print_f("ok?,ack,%s.\r\n","status report");
	mod_som_io_print_f("ok?,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for PowerOff command
 *   prepare SOM to futur power off
 *   should return an apf status.
 *   Power will turn off after reception of this status
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_poweroff_f(){

	mod_som_io_print_f("poweroff,ack.\r\n");
	mod_som_io_print_f("poweroff,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for EpsiNo command
 *   set the SOM and EFE SN
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_epsi_id_f(CPU_CHAR *argv1,CPU_CHAR *argv2, \
		                                   CPU_CHAR *argv3,CPU_CHAR *argv4){

	//	mod_som_io_print_f("EpsiNo,ack,%s,%s,%s.\r\n",argv1,argv2,argv3,,argv4);
	mod_som_io_print_f("EpsiNo,ack,%s,%s,%s,%s.\r\n","SOM","1","EFE","1");
	mod_som_io_print_f("EpsiNo,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for EpsiNo? command
 *   get the SOM and EFE SN
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_epsi_id_status_f(){

	mod_som_io_print_f("EpsiNo,ack,%s,%s,%s,%s.\r\n","SOM","1","EFE","1");
	mod_som_io_print_f("EpsiNo?,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for ProbeNo command
 *   set the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_probe_id_f(CPU_CHAR *argv1,CPU_CHAR *argv2,CPU_CHAR *argv3){

//	mod_som_io_print_f("ProbeNo,ack,%s,%s,%s.\r\n",argv1,argv2,argv3);
	mod_som_io_print_f("ProbeNo,ack,%s,%s,%s.\r\n","shear","103","40");
	mod_som_io_print_f("ProbeNo,ack,%s,%s,%s.\r\n","FPO7","103","40");
	mod_som_io_print_f("ProbeNo,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for ProbeNo? command
 *   get the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_probe_id_status_f(){

	mod_som_io_print_f("EpsiNo?,ack,%s,%s,%s.\r\n","shear","103","40");
	mod_som_io_print_f("EpsiNo?,ack,%s,%s,%s.\r\n","FPO7","103","40");
	mod_som_io_print_f("ProbeNo?,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for sleep command
 *   put SOM to sleep
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_sleep_f(){

	//ALB put the sensor to sleep.
	//ALB If in Daq mode the sleep command will resume Daq.
	//ALB If *not* in Daq mode the SOM will enter low-power mode.



	mod_som_io_print_f("sleep,ak\r\n");
	mod_som_io_print_f("sleep,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for time command
 *   set UnixEpoch time on SOM
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_f(){

	mod_som_io_print_f("time,ak\r\n");
	mod_som_io_print_f("time,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for time? command
 *   get UnixEpoch time on SOM
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_status_f(){

	mod_som_io_print_f("time,ak,%s\r\n","UnixEpoch");
	mod_som_io_print_f("time,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for upload command
 *   start uploading data from the SD card to the apf
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_upload_f(){


	mod_som_io_print_f("upload,ak,%s\r\n","UnixEpoch");
	mod_som_io_print_f("upload,nak,%s\r\n","error message");
	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}
#endif


