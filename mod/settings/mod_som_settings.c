/*
 * mod_som_settings.c
 *
 *  Created on: Sept 4, 2020
 *      Author: aleboyer
 */
#include <mod_som_settings.h>
#include "mod_som_io.h"
#include "em_msc.h"
#include "mod_som_priv.h"


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <mod_som_settings_cmd.h>
#endif

#ifdef  MOD_SOM_SDIO_EN
#include "mod_som_sdio.h"
#endif




uint32_t * userDataPage_ptr = (uint32_t *) USERDATA_BASE;
uint32_t *uphalf_userDataPage = (uint32_t *)0x0FE00800;	// write data at the uper half of page 0x0FE00000UL+2024

MSC_Status_TypeDef msc_err;
MSC_Status_TypeDef msc_ret_val;

sl_status_t mystatus;


mod_som_settings_struct_t mod_som_settings_struct;

/*******************************************************************************
 * @brief
 *   Initialize settings init, if shell is available, then the command table is added
 *
 * it should get the pointer to all the settings structure of the enabled module
 * and packetize it to get the size so it can be ready to be saved in the user data page.
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_settings_init_f(){

	mod_som_status_t status;
	uint32_t user_page_offset=0;

    //ALB read the first 2 bytes of the UserData page
	mod_som_settings_struct.size= (uint32_t) (*userDataPage_ptr);

	//ALB Work around until I am sure re-using previous settings is safe.
	//ALB right now I am ALWAYS starting from the default settings
//	mod_som_settings_struct.size=0xFFFFFFFF;

	if(mod_som_settings_struct.size==0xFFFFFFFF){
		//ALB case where the UserData page is already empty
		//ALB
		//ALB We will initialize the module with default settings and then save the the settings in the UserData page
		mod_som_settings_struct.initialized_flag=false;

    //ALB get the settings header
    strncpy(mod_som_settings_struct.header, \
            MOD_SOM_SETTINGS_DEFAULT_HEADER, \
            MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
    //ALB get the firmware id
    strncpy(mod_som_settings_struct.firmware, \
            MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_NAME, \
            MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH);
    //ALB set default efe rev number (rev3)
    strncpy(mod_som_settings_struct.rev, \
            MOD_SOM_SETTINGS_DEFAULT_CTL_REV_NAME, \
            MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
    //ALB set default efe sn (000)
    strncpy(mod_som_settings_struct.sn, \
            MOD_SOM_SETTINGS_DEFAULT_CTL_SN, \
            MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
    //ALB set default mission name
    strncpy(mod_som_settings_struct.mission_name, \
            MOD_SOM_SETTINGS_DEFAULT_MISSION_NAME, \
            MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH);
    //ALB set default vehicle name
    strncpy(mod_som_settings_struct.vehicle_name, \
            MOD_SOM_SETTINGS_DEFAULT_VEHICLE_NAME, \
            MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH);


#if defined(MOD_SOM_CALENDAR_EN)
    mod_som_settings_struct.mod_som_calendar_settings.initialize_flag=false;
#endif

#if defined(MOD_SOM_EFE_EN)
		mod_som_settings_struct.mod_som_efe_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_SBE49_EN)
		mod_som_settings_struct.mod_som_sbe49_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_SBE41_EN)
    mod_som_settings_struct.mod_som_sbe41_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_SDIO_EN)
		mod_som_settings_struct.mod_som_sdio_settings.initialize_flag=false;

#endif
#if defined(MOD_SOM_APF_EN)
		mod_som_settings_struct.mod_som_apf_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
		mod_som_settings_struct.mod_som_efe_obp_settings.initialize_flag=false;
#endif

#if defined(MOD_SOM_VOLTAGE_EN)
    mod_som_settings_struct.mod_som_voltage_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
    mod_som_settings_struct.mod_som_actuator_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
    mod_som_settings_struct.mod_som_altimeter_settings.initialize_flag=false;
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
    mod_som_settings_struct.mod_som_vec_nav_settings.initialize_flag=false;
#endif



	}
	else{
	    //ALB TODO: #define the structure offsets.
	    mod_som_settings_struct.size=(uint32_t) (*userDataPage_ptr);
	    user_page_offset++;

	    memcpy(mod_som_settings_struct.header, \
	           (char*) &userDataPage_ptr[user_page_offset],\
	           MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH % sizeof(uint32_t));

	    memcpy(mod_som_settings_struct.mission_name, \
	           (uint8_t*) &userDataPage_ptr[user_page_offset], \
	           MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH % sizeof(uint32_t));

	    memcpy(mod_som_settings_struct.vehicle_name,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH % sizeof(uint32_t));


	    memcpy(mod_som_settings_struct.firmware,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH % sizeof(uint32_t));


	    memcpy(mod_som_settings_struct.rev,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH % sizeof(uint32_t));


	    memcpy(mod_som_settings_struct.sn,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH);
	    user_page_offset+=MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH/sizeof(uint32_t)+ \
	        (MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH % sizeof(uint32_t));


	    memcpy(&mod_som_settings_struct.initialized_flag,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;


	    //here offset should 30
#if defined(MOD_SOM_CALENDAR_EN)
	    memcpy(&mod_som_settings_struct.mod_som_calendar_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_calendar_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_calendar_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_calendar_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_calendar_settings.size % sizeof(uint32_t))-1;

#endif
#if defined(MOD_SOM_EFE_EN)
	    memcpy(&mod_som_settings_struct.mod_som_efe_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_efe_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_efe_settings.size-4); //ALB -4 because is the word length in uint8_t

	    user_page_offset+=mod_som_settings_struct.mod_som_efe_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_efe_settings.size % sizeof(uint32_t))-1; //ALB -1 because is the word length in uint32_t
#endif
#if defined(MOD_SOM_SBE49_EN)
	    memcpy(&mod_som_settings_struct.mod_som_sbe49_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_sbe49_settings.data_header_text,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_sbe49_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_sbe49_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_sbe49_settings.size % sizeof(uint32_t))-1;
#endif
#if defined(MOD_SOM_SBE41_EN)

	    memcpy(&mod_som_settings_struct.mod_som_sbe41_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_sbe41_settings.data_header_text,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_sbe41_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_sbe41_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_sbe41_settings.size % sizeof(uint32_t))-1;


#endif
#if defined(MOD_SOM_SDIO_EN)

	    memcpy(&mod_som_settings_struct.mod_som_sdio_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_sdio_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_sdio_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_sdio_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_sdio_settings.size % sizeof(uint32_t))-1;


#endif
#if defined(MOD_SOM_APF_EN)

	    memcpy(&mod_som_settings_struct.mod_som_apf_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_apf_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_apf_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_apf_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_apf_settings.size % sizeof(uint32_t))-1;

#endif
#if defined(MOD_SOM_EFE_OBP_EN)

	    memcpy(&mod_som_settings_struct.mod_som_efe_obp_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_efe_obp_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_efe_obp_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_efe_obp_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_efe_obp_settings.size % sizeof(uint32_t))-1;

#endif

#if defined(MOD_SOM_VOLTAGE_EN)
	    memcpy(&mod_som_settings_struct.mod_som_voltage_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_voltage_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_voltage_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_voltage_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_voltage_settings.size % sizeof(uint32_t))-1;
#endif

#if defined(MOD_SOM_ACTUATOR_EN)

	    memcpy(&mod_som_settings_struct.mod_som_actuator_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_actuator_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_actuator_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_actuator_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_actuator_settings.size % sizeof(uint32_t))-1;

#endif
#if defined(MOD_SOM_ALTIMETER_EN)
	    memcpy(&mod_som_settings_struct.mod_som_altimeter_settings.size,\
	           (uint32_t*) &userDataPage_ptr[user_page_offset],\
	           sizeof(uint32_t));
	    user_page_offset++;

	    memcpy((uint8_t *) &mod_som_settings_struct.mod_som_altimeter_settings.header,\
	           (uint8_t*) &userDataPage_ptr[user_page_offset],\
	           mod_som_settings_struct.mod_som_altimeter_settings.size-4);

	    user_page_offset+=mod_som_settings_struct.mod_som_altimeter_settings.size/sizeof(uint32_t)+ \
	        (mod_som_settings_struct.mod_som_altimeter_settings.size % sizeof(uint32_t))-1;
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
      memcpy(&mod_som_settings_struct.mod_som_vec_nav_settings.size,\
             (uint32_t*) &userDataPage_ptr[user_page_offset],\
             sizeof(uint32_t));
      user_page_offset++;

      memcpy((uint8_t *) &mod_som_settings_struct.mod_som_vec_nav_settings.data_header_text,\
             (uint8_t*) &userDataPage_ptr[user_page_offset],\
             mod_som_settings_struct.mod_som_vec_nav_settings.size-4);

      user_page_offset+=mod_som_settings_struct.mod_som_vec_nav_settings.size/sizeof(uint32_t)+ \
          (mod_som_settings_struct.mod_som_vec_nav_settings.size % sizeof(uint32_t))-1;
#endif



//ALB We read the UserData page, parse it and use the data to initialize the modules.

//parse UserData page
		//store the UserData in the correct module_settings_ptr.

	}



#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_settings_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_settings_encode_status_f(MOD_SOM_SETTINGS_STATUS_FAIL_INIT_CMD);
#endif

    return mod_som_settings_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   a function to get the settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_settings_struct_ptr_t mod_som_settings_get_settings_f(){
    return &mod_som_settings_struct;
}


/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_settings_clear_settings_f(){
    MSC_ErasePage(userDataPage_ptr);
    return mod_som_settings_encode_status_f(MOD_SOM_STATUS_OK);
}

mod_som_status_t mod_som_settings_save_settings_f(){
	//ALB - get the settings from other module
	//ALB - write the settings_struct in the UserData page.
  mod_som_status_t status=0;

#if defined(MOD_SOM_CALENDAR_EN)
		mod_som_settings_struct.mod_som_calendar_settings=mod_som_calendar_get_settings_f();
#endif
#if defined(MOD_SOM_EFE_EN)
		mod_som_settings_struct.mod_som_efe_settings=mod_som_efe_get_settings_f();
#endif
#if defined(MOD_SOM_SDIO_EN)
		mod_som_settings_struct.mod_som_sdio_settings=mod_som_sdio_get_settings_f();
#endif
#if defined(MOD_SOM_SBE49_EN)
		mod_som_settings_struct.mod_som_sbe49_settings=mod_som_sbe49_get_settings_f();
#endif
#if defined(MOD_SOM_SBE41_EN)
		mod_som_settings_struct.mod_som_sbe41_settings=mod_som_sbe41_get_settings_f();
#endif
#if defined(MOD_SOM_APF_EN)
		mod_som_settings_struct.mod_som_apf_settings=mod_som_apf_get_settings_f();
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
		mod_som_settings_struct.mod_som_efe_obp_settings=mod_som_efe_obp_get_settings_f();
#endif
#if defined(MOD_SOM_VOLTAGE_EN)
    mod_som_settings_struct.mod_som_voltage_settings=mod_som_voltage_get_settings_f();
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
    mod_som_settings_struct.mod_som_actuator_settings=mod_som_actuator_get_settings_f();
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
    mod_som_settings_struct.mod_som_altimeter_settings=mod_som_altimeter_get_settings_f();
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
    mod_som_settings_struct.mod_som_vec_nav_settings=mod_som_vecnav_get_settings_f();
#endif

		mod_som_settings_struct.initialized_flag=true;

		mod_som_settings_struct.size=sizeof(mod_som_settings_struct);

	  //1 - Enables the MSC (i.e. flash) controller for writing
	  MSC_ExecConfig_TypeDef execConfig = MSC_EXECCONFIG_DEFAULT;
	  MSC_ExecConfigSet(&execConfig);
	  MSC_Init();


	    // copy data from dataPage to the temp

	 	if ((msc_ret_val = MSC_ErasePage(userDataPage_ptr)) == mscReturnOk)
	  {
//	 		printf("Success for erasing before writing ...\r\n");//MHA
	 		// 2. success erasing data, now write data in the upper half page at the address: from base adding 2048: 0x0FE00800UL
	 		// 2a. write back the copy data to lower half:
	 		msc_ret_val = \
	 				MSC_WriteWord(userDataPage_ptr, &mod_som_settings_struct, mod_som_settings_struct.size);
	 		if (msc_ret_val == mscReturnOk)
	 		{
//	 			printf("Success writing the copy in the lower half\r\n");//MHA
	 		}
	 		// 2b. write the settings data in the upper half: later write the settings into this up half
	 		msc_ret_val = MSC_WriteWord(uphalf_userDataPage, &mod_som_settings_struct, mod_som_settings_struct.size);
	 		if (msc_ret_val == mscReturnOk)
	 		{
//	 			printf("MSC: Write ok\n\r");
	 			// check the data just written
	 			// 3. Read to check written data
	 			//Read_PrintSetupData(w_userDataInfo_addr, settingsData, sizeof(settingsData));
	 		}
	 		else
	 		{
	 			printf("MSC: Failed to write\n\r");
	 			status|=MOD_SOM_SETTINGS_STATUS_FAIL_WRITING_USERPAGE_CMD;
	 		}  // endof if ((msc_ret_val = MSC_ErasePage(userDataPage_ptr)) == mscReturnOk)
	  }
	  else
	  {
//	  		printf("Failed to erase the page in msc\r\n");//MHA
	      status|=MOD_SOM_SETTINGS_STATUS_FAIL_WRITING_USERPAGE_CMD;
	  }
		// 4. disable the flash controller
		MSC_Deinit();
    return mod_som_settings_encode_status_f(status);
}

/*******************************************************************************
 * @brief
 *   a function to get the settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_settings_sd_settings_f(){

  //time stamp
  uint64_t tick;
  uint64_t timestamp;
  uint32_t t_hex[2];
  uint8_t * local_header;
  uint8_t header[100];
  uint8_t header_chksum;
  uint8_t payload_chksum;
  uint8_t str_payload_chksum[MOD_SOM_SETTINGS_PAYLOAD_CHECKSUM_LENGTH];
  uint8_t length_header=32;
  uint8_t * local_settings_ptr;

  mod_som_sdio_ptr_t local_sdio_ptr=mod_som_sdio_get_runtime_ptr_f();


  tick=sl_sleeptimer_get_tick_count64();
  mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                        &timestamp);

  t_hex[0] = (uint32_t) (timestamp>>32);
  t_hex[1] = (uint32_t) timestamp;

  //header  contains SBE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
      sprintf((char*) header,  \
          "$%s%08x%08x%08x*FF", \
          mod_som_settings_struct.header, \
          (int) t_hex[0],\
          (int) t_hex[1],\
          sizeof(mod_som_settings_struct));

      header_chksum=0;
      for(int i=0;i<length_header-
                    MOD_SOM_SETTINGS_HEADER_CHECKSUM_LENGTH;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
        {
          header_chksum ^=header[i];
        }


      // the curr_consumer_element_ptr should be at the right place to
      // write the checksum already
      //write checksum at the end of the steam block (record).
      local_header = &header[length_header-
                             MOD_SOM_SETTINGS_HEADER_CHECKSUM_LENGTH+1];
      *((uint16_t*)local_header) = \
          mod_som_int8_2hex_f(header_chksum);

      // compute checksum on settings payload
      payload_chksum=0;
      local_settings_ptr=(uint8_t*)&mod_som_settings_struct;
      for(int i=0;i<sizeof(mod_som_settings_struct);i++)
        {
          payload_chksum ^=*(local_settings_ptr++);
        }

      sprintf((char*) str_payload_chksum,  \
          "*%02x\r\n",payload_chksum);


      mod_som_sdio_write_config_f((uint8_t*) &header,\
                                       length_header,
                                       local_sdio_ptr->data_file_ptr);
      mod_som_sdio_write_config_f((uint8_t*) &mod_som_settings_struct,
                                       sizeof(mod_som_settings_struct),
                                       local_sdio_ptr->data_file_ptr);
      mod_som_sdio_write_config_f((uint8_t*) &str_payload_chksum,\
                                       MOD_SOM_SETTINGS_PAYLOAD_CHECKSUM_LENGTH,
                                       local_sdio_ptr->data_file_ptr);

      return mod_som_settings_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a function to  stream the settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_settings_stream_settings_f(){



  //time stamp
  uint64_t tick;
  uint64_t timestamp;
  uint32_t t_hex[2];
  uint8_t * local_header;
  uint8_t header[100];
  uint8_t header_chksum;
  uint8_t payload_chksum;
  uint8_t str_payload_chksum[MOD_SOM_SETTINGS_PAYLOAD_CHECKSUM_LENGTH];
  uint8_t length_header=32;
  uint8_t * local_settings_ptr;


  tick=sl_sleeptimer_get_tick_count64();
  mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                        &timestamp);

  t_hex[0] = (uint32_t) (timestamp>>32);
  t_hex[1] = (uint32_t) timestamp;

  //header  contains SBE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
      sprintf((char*) header,  \
          "$%s%08x%08x%08x*FF", \
          mod_som_settings_struct.header, \
          (int) t_hex[0],\
          (int) t_hex[1],\
          sizeof(mod_som_settings_struct));

      header_chksum=0;
      for(int i=0;i<length_header-
                    MOD_SOM_SETTINGS_HEADER_CHECKSUM_LENGTH;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
        {
          header_chksum ^=header[i];
        }


      // the curr_consumer_element_ptr should be at the right place to
      // write the checksum already
      //write checksum at the end of the steam block (record).
      local_header = &header[length_header-
                             MOD_SOM_SETTINGS_HEADER_CHECKSUM_LENGTH+1];
      *((uint16_t*)local_header) = \
          mod_som_int8_2hex_f(header_chksum);

      // compute checksum on settings payload
      payload_chksum=0;
      local_settings_ptr=(uint8_t*)&mod_som_settings_struct;
      for(int i=0;i<sizeof(mod_som_settings_struct);i++)
        {
          payload_chksum ^=*(local_settings_ptr++);
        }

      sprintf((char*) str_payload_chksum,  \
          "*%02x\r\n",payload_chksum);


      mod_som_io_stream_data_f((uint8_t*) &header,\
                               length_header,
                                DEF_NULL);
      mod_som_io_stream_data_f((uint8_t*) &mod_som_settings_struct,\
                            sizeof(mod_som_settings_struct),
                            DEF_NULL);
      mod_som_io_stream_data_f((uint8_t*) &str_payload_chksum,\
                               MOD_SOM_SETTINGS_PAYLOAD_CHECKSUM_LENGTH,
                               DEF_NULL);


    return mod_som_settings_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   a function to ge/set the som id
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_settings_id_f(CPU_INT16U argc,CPU_CHAR *argv[]){

	if (argc==1){
		printf("SOM, %s, %s, %s.\r\n",(char*) mod_som_settings_struct.rev ,(char*) mod_som_settings_struct.sn, \
				                      (char*) mod_som_settings_struct.firmware);
	}
	else{

		//ALB switch statement easy to handle all user input cases.
		switch (argc){
		case 4:
			strcpy(mod_som_settings_struct.rev,argv[1]);
			strcpy(mod_som_settings_struct.sn,argv[2]);
			strcpy(mod_som_settings_struct.firmware,argv[3]);
			break;
		default:
			printf("format: settings.som_id REV S/N firmwarename\r\n");
			break;
		}
	}
}


/*******************************************************************************
 * @brief
 *   a function to get/set the mission and vehicle name
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_settings_mission_f(CPU_INT16U argc,CPU_CHAR *argv[]){

	if (argc==1){
		printf("Mission: %s, Vehicle: %s.\r\n",(char*) mod_som_settings_struct.mission_name ,(char*) mod_som_settings_struct.vehicle_name);
	}
	else{

		//ALB switch statement easy to handle all user input cases.
		switch (argc){
		case 3:
			//TODO deal error if argv1>20 and arg2>20
			strcpy(mod_som_settings_struct.mission_name,argv[1]);
			strcpy(mod_som_settings_struct.vehicle_name,argv[2]);
			break;
		default:
			printf("format: settings.mission mission_name(<20char) vehicle_name(<20char)\r\n");
			break;
		}
	}
}

/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_settings_say_hello_world_f(){
    mod_som_io_print_f("[foo bar]: hello world\r\n");
    return mod_som_settings_encode_status_f(MOD_SOM_STATUS_OK);
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
uint8_t mod_som_settings_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_SETTINGS_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_settings_encode_status_f
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
mod_som_status_t mod_som_settings_encode_status_f (uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_SETTINGS_STATUS_PREFIX, mod_som_io_status);
}


