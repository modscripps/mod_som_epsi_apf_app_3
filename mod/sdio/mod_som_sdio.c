/*
 * mod_som_sdio.c
 *
 *  Created on: June 28, 2020
 *      Author: alboyer
 */
#include "mod_som_io.h"
#include "ff.h"
#include "diskio.h"
#include "mod_som_sdio.h"
#include "mod_som_sdio_priv.h"
#include <sl_sleeptimer.h>
#include "mod_som_priv.h"


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <sdio/mod_som_sdio_cmd.h>
#endif

#ifdef MOD_SOM_CALENDAR_EN
	#include "mod_som_calendar.h"
#endif
#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif


// ALB Task related variables
#define  MOD_SOM_SDIO_WRITE_TASK_PRIO             18u
#define  MOD_SOM_SDIO_WRITE_TASK_STK_SIZE        512u

#define MOD_SOM_SDIO_WRITE_PADDING 1		// number of elements for padding for consumer 2

mod_som_sdio_t mod_som_sdio_struct;
sl_status_t mystatus;


/***************************************************************************//**
 * @brief
 *   This function is required by the FAT file system in order to provide
 *   timestamps for created files. Since we do not have a reliable clock we
 *   hardcode a value here.
 *
 *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
 * @return
 *    A DWORD containing the current time and date as a packed datastructure.
 ******************************************************************************/
DWORD get_fattime(void)
{
  return (28 << 25) | (2 << 21) | (1 << 16);
}


/*******************************************************************************
 * @brief
 *   Initialize SDIO module, if shell is available, then the command table is added
 *   starts withs allocating memory
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sdio_init_f(){
    mod_som_status_t status;
    RTOS_ERR  err;
//    mod_som_sdio_ptr_t mod_som_sdio_struct_ptr=&mod_som_sdio_struct;

    //ALB initialize the SDIO module commands
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    if (!mod_som_sdio_struct.initialized_flag){
        status = mod_som_sdio_init_cmd_f();
        //ALB return error if cmd not initialized
        if(status != MOD_SOM_STATUS_OK)
            return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_FAIL_INIT_CMD);
    }
#endif


    //ALB allocate memory for the module settings ptr.
    //ALB SDIO module have the a module struct directly defined (line 28)
    //ALB it differs from the other modules for which I define a module_struct_ptr
    //ALB I have no reason to do this beside keeping close to San's first version of the module
    mod_som_sdio_struct.mod_som_sdio_settings_ptr =
    		(mod_som_sdio_settings_ptr_t)Mem_SegAlloc(
    				"MOD SOM SDIO SETUP.",DEF_NULL,
					sizeof(mod_som_sdio_settings_t),
					&err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_struct.mod_som_sdio_settings_ptr==NULL)
    {
    	mod_som_sdio_struct.mod_som_sdio_settings_ptr = DEF_NULL;
    	return MOD_SOM_SDIO_CANNOT_SAVE_SETUP;
    }


    //ALB initialize runtime param
    mod_som_sdio_struct.initialized_flag = false;
    mod_som_sdio_struct.started_flag = false;
    mod_som_sdio_struct.file_number=0;
    mod_som_sdio_struct.new_file_flag=false;
    mod_som_sdio_struct.listoverflow_flag=false;
    mod_som_sdio_struct.enable_flag=false;

    //ALB What is that?
    if(mod_som_sdio_struct.max_size_of_complete_data_block == 0)
    	mod_som_sdio_struct.max_size_of_complete_data_block = MOD_SOM_SDIO_DEFAULT_SIZE_LARGEST_BLOCK;


    //define settings
    mod_som_sdio_default_settings_f();
    //allocate memory
    mod_som_sdio_allocate_memory_f();
    //ALB enable hardware
    mod_som_sdio_enable_hardware_f();
    //mount fatFS memory.
    // ALB the SDIO software does not have a disk_initialization working correctly.
    // ALB consequently we can mount a volume but not check if an physical SD is in the slot.
    // ALB I am moving mount fatfs inside enable hardware
    // mod_som_sdio_mount_fatfs_f();

    //ALB initialize runtime param initialized flag
    mod_som_sdio_struct.initialized_flag = true;

    //ALB start the SDIO task.
    mod_som_sdio_start_f();

    //ALB disable hardware
    mod_som_sdio_disable_hardware_f();

    // return default mod som OK.
    //TODO handle error from the previous calls.
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   enable hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_enable_hardware_f(){
//  int delay=833333; //ALB 60s at 50e6 Hz
  int delay=1000; //ALB 1sec

  if(!mod_som_sdio_struct.enable_flag){
      mod_som_sdio_struct.enable_flag=true;
  CMU_ClockEnable(cmuClock_GPIO, true);


  // Soldered sdCard slot
    GPIO_PinModeSet(gpioPortD, 6u, gpioModePushPull, 1); //SD_EN
    GPIO_PinOutSet(gpioPortD, 6u);

    sl_sleeptimer_delay_millisecond(delay);

//ALB TODO Make CD works
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 1);              // SDIO_CD
    GPIO_PinModeSet(gpioPortE, 15, gpioModePushPullAlternate, 0); // SDIO_CMD
    GPIO_PinModeSet(gpioPortE, 14, gpioModePushPullAlternate, 1); // SDIO_CLK
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPullAlternate, 1);  // SDIO_DAT0
    GPIO_PinModeSet(gpioPortA, 1, gpioModePushPullAlternate, 1);  // SDIO_DAT1
    GPIO_PinModeSet(gpioPortA, 2, gpioModePushPullAlternate, 1);  // SDIO_DAT2
    GPIO_PinModeSet(gpioPortA, 3, gpioModePushPullAlternate, 1);  // SDIO_DAT3

    sl_sleeptimer_delay_millisecond(delay);


    mod_som_sdio_mount_fatfs_f();
//    //ALB CDSIGDET should trigger the toggling of the register CDTSTLLVL
//    SDIO->HOSTCTRL1|=(_SDIO_HOSTCTRL1_CDSIGDET_MASK & SDIO_HOSTCTRL1_CDSIGDET);


  }

    // return default mod som OK.
    // TODO handle error from the previous calls.
	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   disable hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_disable_hardware_f(){
int delay=1000; //ALB 60s at 50e6 Hz
FRESULT res;
mod_som_status_t status=0;

  if(mod_som_sdio_struct.enable_flag){
      mod_som_sdio_struct.enable_flag=false;

      //ALB if loop insde to check if the file is open
      if(mod_som_sdio_struct.data_file_ptr->is_open_flag==1){
          mod_som_sdio_close_file_f(mod_som_sdio_struct.data_file_ptr);
      }
//      //ALB unmount fatfs
//      f_mount(0, NULL);

      res=f_mount(NULL,(TCHAR *) "" ,1);
      SDIO->CLOCKCTRL|=(_SDIO_CLOCKCTRL_SFTRSTA_MASK & SDIO_CLOCKCTRL_SFTRSTA);

      if (res!=FR_OK){
        status=1;
      }
      sl_sleeptimer_delay_millisecond(delay);

    //TODO use bsp_som variables to initialize the SD card.
    // Soldered sdCard slot
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);             // SDIO_CD
    GPIO_PinModeSet(gpioPortE, 15, gpioModePushPullAlternate, 0); // SDIO_CMD
    GPIO_PinModeSet(gpioPortE, 14, gpioModePushPullAlternate, 0); // SDIO_CLK
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPullAlternate, 0);  // SDIO_DAT0
    GPIO_PinModeSet(gpioPortA, 1, gpioModePushPullAlternate, 0);  // SDIO_DAT1
    GPIO_PinModeSet(gpioPortA, 2, gpioModePushPullAlternate, 0);  // SDIO_DAT2
    GPIO_PinModeSet(gpioPortA, 3, gpioModePushPullAlternate, 0);  // SDIO_DAT3


    sl_sleeptimer_delay_millisecond(delay);
    GPIO_PinModeSet(gpioPortD, 6u, gpioModePushPull, 0); //SD_EN

  }else{
      status=1;
  }
    // return default mod som OK.
    // TODO handle error from the previous calls.
  return status;

}


/*******************************************************************************
 * @brief
 *   mount fatFS hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_mount_fatfs_f(){

    //mount volume
    FRESULT res;
    if(mod_som_sdio_struct.initialized_flag){
        uint8_t * local_fat_fs_ptr= (uint8_t *) &mod_som_sdio_struct.fat_fs;
        for (int i=1;i<sizeof(FATFS);i++){
            local_fat_fs_ptr[i]=0;
        }
    }
    res=f_mount(&mod_som_sdio_struct.fat_fs,(TCHAR *) "" ,0);

    //    res = f_mount(VOLUME_ADDRESS, &Fatfs);
    if (res != FR_OK)
    {
    	printf("FAT-mount failed: %d\r\n", res);
    }
    else
    {
    	printf("FAT-mount successful.\r\n Watch out: It does NOT imply there is an SD card. \r\n");//MHA
    }

    // return default mod som OK.
    //TODO handle error from the previous calls.
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   enable hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_allocate_memory_f(){
	RTOS_ERR err;

    OSQCreate(&mod_som_sdio_struct.msg_queue,           /*   Pointer to user-allocated message queue.          */
            "MOD SOM SDIO Message Queue",          		/*   Name used for debugging.                          */
            MOD_SOM_SDIO_MSG_QUEUE_COUNT,               /*   Queue will have 10 messages maximum.              */
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE);

    OSQFlush(&mod_som_sdio_struct.msg_queue,&err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_ALREADY_STARTED);

    Mem_DynPoolCreate(
            "MOD SOM SDIO Dynamic Memory Pool",
            &mod_som_sdio_struct.dyn_mem_pool,
            DEF_NULL,
            sizeof(mod_som_sdio_xfer_t)+mod_som_sdio_struct.max_size_of_complete_data_block,
            sizeof(CPU_ALIGN),//LIB_MEM_BUF_ALIGN_AUTO,
            MOD_SOM_SDIO_MSG_QUEUE_COUNT,
            2*MOD_SOM_SDIO_MSG_QUEUE_COUNT,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY);


    // ALB save memory space for the data file
    mod_som_sdio_struct.data_file_ptr =
            (void *)Mem_SegAlloc(
                    "MOD SOM SDIO data file handle",DEF_NULL,
                    sizeof(mod_som_sdio_file_t),
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_struct.data_file_ptr==DEF_NULL)
    {
    	mod_som_sdio_struct.data_file_ptr = DEF_NULL;
        return MOD_SOM_SDIO_CANNOT_OPEN_DATA_FILE;
    }

    mod_som_sdio_struct.data_file_ptr->file_name =
            (char *)Mem_SegAlloc(
                    "MOD SOM SDIO data filename handle",DEF_NULL,
                    MOD_SOM_SDIO_MAX_FILENAME_LENGTH,
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_struct.data_file_ptr->file_name==DEF_NULL)
    {
    	mod_som_sdio_struct.data_file_ptr->file_name = DEF_NULL;
        return MOD_SOM_SDIO_CANNOT_OPEN_CONF_FILE;
    }

    mod_som_sdio_struct.data_file_ptr->fp =
            (FIL *)Mem_SegAlloc(
                    "MOD SOM SDIO data fp handle",DEF_NULL,
                    sizeof(FIL),
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_struct.data_file_ptr->fp==DEF_NULL)
    {
    	mod_som_sdio_struct.data_file_ptr->fp = DEF_NULL;
        return MOD_SOM_SDIO_CANNOT_OPEN_CONF_FILE;
    }

    mod_som_sdio_struct.data_file_ptr->fp->dir_ptr =
            (BYTE *)Mem_SegAlloc(
                    "MOD SOM SDIO fp dir handle",DEF_NULL,
                    sizeof(BYTE),
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_struct.data_file_ptr->fp->dir_ptr==DEF_NULL)
    {
    	mod_som_sdio_struct.data_file_ptr->fp->dir_ptr = DEF_NULL;
        return MOD_SOM_SDIO_CANNOT_OPEN_CONF_FILE;
    }



	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   default settings
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_default_settings_f(){

//	CPU_CHAR* filename = MOD_SOM_SDIO_DEFAULT_FILENAME_HEADER;
	mod_som_sdio_struct.mod_som_sdio_settings_ptr->initialize_flag=false;
	mod_som_sdio_struct.mod_som_sdio_settings_ptr->file_duration=MOD_SOM_SDIO_DEFAULT_FILE_DURATION; //default 3600 sec
	strncpy(mod_som_sdio_struct.mod_som_sdio_settings_ptr->header,MOD_SOM_SDIO_DEFAULT_SETUP_HEADER,4);
	mod_som_sdio_struct.mod_som_sdio_settings_ptr->size   = sizeof(*mod_som_sdio_struct.mod_som_sdio_settings_ptr);
	mod_som_sdio_struct.mod_som_sdio_settings_ptr->initialize_flag=true;

	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   get settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_sdio_settings_t mod_som_sdio_get_settings_f(){
	return *mod_som_sdio_struct.mod_som_sdio_settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get sdio run time _ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_sdio_ptr_t mod_som_sdio_get_runtime_ptr_f(){
  return &mod_som_sdio_struct;
}

/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_define_filename_f(CPU_CHAR* filename){

	mod_som_status_t status;
	mod_som_status_t status_data;
//	mod_som_status_t status_config;
	FRESULT res_sync;
//	char * time_buff;

	CPU_CHAR data_file_buf[100];   //ALB with this version of ff.c the filename can *not* be more than 8

	if(!mod_som_sdio_struct.enable_flag){
	    mod_som_sdio_enable_hardware_f();
	}

	//ALB store file prefix
	memcpy(mod_som_sdio_struct.mod_som_sdio_settings_ptr->prefix_file, filename,strlen(filename));

	sprintf(data_file_buf, "%s_%lu.modraw",filename,mod_som_sdio_struct.file_number);
//	sprintf(config_file_buf, "%s_config",filename);

    mod_som_sdio_struct.data_file_ptr->len_filename=strlen(data_file_buf);
//    mod_som_sdio_struct.config_file_ptr->len_filename=strlen(config_file_buf);

    //ALB initialize the filename memory to erase previous str
	memset(mod_som_sdio_struct.data_file_ptr->file_name, 0,strlen(mod_som_sdio_struct.data_file_ptr->file_name));
//	memset(mod_som_sdio_struct.config_file_ptr->file_name, 0,strlen(mod_som_sdio_struct.config_file_ptr->file_name));

	memcpy(mod_som_sdio_struct.data_file_ptr->file_name, data_file_buf,mod_som_sdio_struct.data_file_ptr->len_filename);
//	memcpy(mod_som_sdio_struct.config_file_ptr->file_name, config_file_buf,mod_som_sdio_struct.config_file_ptr->len_filename);

//	mod_som_sdio_struct.config_file_ptr->initialized_flag=0;
  mod_som_sdio_struct.data_file_ptr->initialized_flag=0;

	status_data=mod_som_sdio_open_file_f(mod_som_sdio_struct.data_file_ptr);
	mod_som_sdio_struct.open_file_time=mod_som_calendar_get_time_f();
	//store the date time when we open the file
//	time_buff=mod_som_calendar_get_datetime_f();
  mod_som_settings_struct_ptr_t local_settings_ptr=
                                          mod_som_settings_get_settings_f();

	status_data=mod_som_sdio_write_config_f((uint8_t *) local_settings_ptr,\
	                                        local_settings_ptr->size,
												   mod_som_sdio_struct.data_file_ptr);



	//ALB sync the file to actually write on the SD card
    res_sync = f_sync(mod_som_sdio_struct.data_file_ptr->fp);
	mod_som_sdio_struct.data_file_ptr->initialized_flag=1;

	status=status_data & res_sync;
	if (status!=MOD_SOM_SDIO_STATUS_OK){
		return MOD_SOM_STATUS_NOT_OK;
	}
	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_open_next_datafile(){

	mod_som_status_t status_data;
    FRESULT res,res_sync;
    char * time_buff;
    UINT byteswritten;

	CPU_CHAR data_file_buf[100];   //ALB with this version of ff.c the filename can *not* be more than 8

	mod_som_sdio_struct.file_number++;
	sprintf(data_file_buf, "%s_%lu.modraw",mod_som_sdio_struct.mod_som_sdio_settings_ptr->prefix_file,mod_som_sdio_struct.file_number);



//close the previous data file
	mod_som_sdio_close_file_f(mod_som_sdio_struct.data_file_ptr);
//change the data file ptr with the new name
    mod_som_sdio_struct.data_file_ptr->len_filename=strlen(data_file_buf);
	memset(mod_som_sdio_struct.data_file_ptr->file_name, 0,mod_som_sdio_struct.data_file_ptr->len_filename);
	memcpy(mod_som_sdio_struct.data_file_ptr->file_name, data_file_buf,mod_som_sdio_struct.data_file_ptr->len_filename);

	mod_som_sdio_struct.data_file_ptr->initialized_flag=0;
//change the data file ptr with the new name
//    mod_som_sdio_struct.data_file_ptr->fp = &mod_som_sdio_struct.fstrc_data;
//open the new file
    status_data=mod_som_sdio_open_file_f(mod_som_sdio_struct.data_file_ptr);
    mod_som_sdio_struct.open_file_time=mod_som_calendar_get_time_f();
	time_buff=mod_som_calendar_get_datetime_f();
    res = f_write(mod_som_sdio_struct.data_file_ptr->fp,\
    		time_buff,strlen(time_buff), &byteswritten); /* buffptr = pointer to the data to be written */
    res_sync=f_sync(mod_som_sdio_struct.data_file_ptr->fp);

    if ((res|res_sync)!=FR_OK){
    	printf("can not write date on %s",data_file_buf);
    }

	mod_som_sdio_struct.data_file_ptr->initialized_flag=1;


	if (status_data!=MOD_SOM_SDIO_STATUS_OK){
		return MOD_SOM_STATUS_NOT_OK;
	}
	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_open_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr){

	mod_som_io_print_f("\n[sdio open file]:%s \r\n",(char *) mod_som_sdio_file_ptr->file_name);
    mod_som_sdio_file_ptr->is_open_flag=0;

    TCHAR tchar_filename[100];

    int idx;
    //mod_som_sdio_file_ptr->file_name is ASCII char, we have to convert into ANSII
    for (idx = 0; idx < strlen (mod_som_sdio_file_ptr->file_name); ++idx){
    	tchar_filename[idx] = ff_convert (mod_som_sdio_file_ptr->file_name[idx], 1);
    }
    tchar_filename[idx] = '\0';

    FRESULT res;
    res = f_open(mod_som_sdio_file_ptr->fp, \
    		tchar_filename,\
			FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res == FR_OK)
    {
    	mod_som_io_print_f("\nopened %s\n",(char *) mod_som_sdio_file_ptr->file_name);

    }else {
    	mod_som_io_print_f("\nFailed to open %s,error %i \n", \
    			(char *) mod_som_sdio_file_ptr->file_name, res);
    	return MOD_SOM_SDIO_STATUS_FAIL_OPENFILE;
    }
    mod_som_sdio_file_ptr->is_open_flag=1;
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   a function to close a file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_close_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr){

	if(mod_som_sdio_file_ptr->is_open_flag==1){

	    FRESULT res;

	    //ALB Add a delay because sometime the closing process get hangup
	    //ALB because (i think) the file close while trying to write in the writing task
	    // close the file
	    res=f_close(mod_som_sdio_file_ptr->fp);
	    //ALB clear fp
//	    for (int i=1;i<sizeof(FIL);i++){
//	        mod_som_sdio_file_ptr->fp[i]=0;
//	    }

		mod_som_sdio_file_ptr->is_open_flag=0;
		if (res == FR_OK)
		{
			mod_som_io_print_f("closed %s\r\n",mod_som_sdio_file_ptr->file_name);
		} else{
			mod_som_io_print_f("Failed to close %s, error %i\r\n", \
					mod_som_sdio_file_ptr->file_name,res);
			return MOD_SOM_SDIO_STATUS_FAIL_CLOSEFILE;
		}
	} else {
		printf("Close error file not open\r\n");
	}
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   a function to write config in file_ptr
 *   - if file is closed re-open it in F!_OPEN_APPEND mode
 *   - write the config in the file
 *   - if the file was closed at the beginning, close it again
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_write_config_f(uint8_t *data_ptr,
                                             uint32_t data_length,
                                             mod_som_sdio_file_ptr_t file_ptr){

	FRESULT res;
	UINT byteswritten=0;
    int remaining_bytes;
    int bytes_to_send=MOD_SOM_SDIO_BLOCK_LENGTH;

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
//    uint8_t * local_settings_ptr;


	if(file_ptr->is_open_flag==0){

		TCHAR tchar_filename[100];   //ALB with this version of ff.c the filename can *not* be more than 8

		int idx;
		for (idx = 0; idx < strlen (file_ptr->file_name); ++idx){
			tchar_filename[idx] = ff_convert (file_ptr->file_name[idx], 1);
		}
		tchar_filename[idx] = '\0';

		res = f_open(file_ptr->fp, \
				tchar_filename,\
				FA_OPEN_APPEND);
		if (res!=FR_OK){
			printf("\n can not open %s",file_ptr->file_name);
		}
	}

	//Header
	tick=sl_sleeptimer_get_tick_count64();
	mystatus = sl_sleeptimer_tick64_to_ms(tick,\
	                                      &timestamp);

	t_hex[0] = (uint32_t) (timestamp>>32);
	t_hex[1] = (uint32_t) timestamp;

	//header  contains SBE,flags. Currently flags are hard coded to 0x1e
	//time stamp will come at the end of header
	sprintf((char*) header,  \
	        "$%s%08x%08x%08x*FF", \
	        "SOM3", \
	        (int) t_hex[0],\
	        (int) t_hex[1],\
	        (int) data_length);

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
	for(int i=0;i<data_length;i++)
	  {
//	    payload_chksum ^=*(data_ptr++);
      payload_chksum ^=data_ptr[i];
	  }

	sprintf((char*) str_payload_chksum,  \
	        "*%02x\r\n",payload_chksum);

    res = f_write(file_ptr->fp,(uint8_t*) header,length_header,&byteswritten);  /* buffptr = pointer to the data to be written */

    //write the settings
    byteswritten=0;
	  remaining_bytes=data_length;
    while(remaining_bytes>0){
        if (remaining_bytes<MOD_SOM_SDIO_BLOCK_LENGTH){
            bytes_to_send=remaining_bytes;
        }
    	res = f_write(file_ptr->fp,data_ptr+byteswritten,bytes_to_send,&byteswritten);	/* buffptr = pointer to the data to be written */
        remaining_bytes=remaining_bytes-byteswritten;
        if (res != FR_OK){
            //TODO write an error code
        }
    }
    byteswritten=0;
    res = f_write(file_ptr->fp,str_payload_chksum,5,&byteswritten);  /* buffptr = pointer to the data to be written */
    f_sync(mod_som_sdio_struct.data_file_ptr->fp);

    if(file_ptr->is_open_flag==0){
        mod_som_sdio_close_file_f(file_ptr);
    }

	if (res != FR_OK){
		printf("can not write config in %s",file_ptr->file_name);
		return MOD_SOM_SDIO_STATUS_FAIL_WRITEFILE;
	}
	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a function to write config file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_stop_store_f(){

  //TODO Handle status update.
	mod_som_sdio_close_file_f(mod_som_sdio_struct.data_file_ptr);
	return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   a function read the config file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_config_sd_f(char * filename){

    mod_som_status_t status=MOD_SOM_SDIO_STATUS_OK;
	CPU_CHAR data_file_buf[100];

	sprintf(data_file_buf, "%s",filename);
	mod_som_sdio_struct.data_file_ptr->len_filename=strlen(data_file_buf);
    memset(mod_som_sdio_struct.data_file_ptr->file_name,0,strlen(mod_som_sdio_struct.data_file_ptr->file_name));
    memcpy(mod_som_sdio_struct.data_file_ptr->file_name, data_file_buf,mod_som_sdio_struct.data_file_ptr->len_filename);

	status=mod_som_sdio_read_file_f(mod_som_sdio_struct.data_file_ptr);

    return status;
}
/*******************************************************************************
 * @brief
 *   a function read  the data file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_data_sd_f(char * filename,uint32_t number_of_files){

	mod_som_status_t status=0;
	CPU_CHAR data_file_buf[100];

	//close the current file if open
	mod_som_sdio_close_file_f(mod_som_sdio_struct.data_file_ptr);
	//loop through the files,read and stream data
	for (uint32_t i=0;i<number_of_files+1;i++){
		sprintf(data_file_buf, "%s%lu",filename,i);
	    mod_som_sdio_struct.data_file_ptr->len_filename=strlen(data_file_buf);
	    //clear file_name. if i<10, tt avoids the 0 in filename07.
	    memset(mod_som_sdio_struct.data_file_ptr->file_name,0,strlen(mod_som_sdio_struct.data_file_ptr->file_name));
	    memcpy(mod_som_sdio_struct.data_file_ptr->file_name, data_file_buf,mod_som_sdio_struct.data_file_ptr->len_filename);

		status=mod_som_sdio_read_file_f(mod_som_sdio_struct.data_file_ptr);
	}


    return status;
}


mod_som_status_t mod_som_sdio_read_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr){

        UINT byte_read;
        FRESULT res;
        TCHAR tchar_filename[100];

        int idx;

        for (idx = 0; idx < strlen (mod_som_sdio_file_ptr->file_name); ++idx){
        	tchar_filename[idx] = ff_convert (mod_som_sdio_file_ptr->file_name[idx], 1);
        }
        tchar_filename[idx] = '\0';


        res = f_open(mod_som_sdio_file_ptr->fp, \
        		tchar_filename,\
    			FA_READ);

        if(mod_som_sdio_file_ptr->is_open_flag==0){
        	mod_som_io_print_f("\nReading file %s...\n", \
        			mod_som_sdio_file_ptr->file_name);

        	res = f_lseek (mod_som_sdio_file_ptr->fp, 0);
        	int done_reading=0;
        	while(done_reading==0){
        	    res = f_read(mod_som_sdio_file_ptr->fp, mod_som_sdio_struct.read_buff, \
        	            MOD_SOM_SDIO_BLOCK_LENGTH, &byte_read);
        		if (res == FR_OK){
        			mod_som_io_stream_data_f((uint8_t*)mod_som_sdio_struct.read_buff,byte_read,DEF_NULL);
        		} else{
        			mod_som_io_print_f("\nRead Failure: %d\n", res);
        		}
            //ALB feed the WDOG coz sending long files tiggers the WDOG.
            WDOG_Feed();
            //ALB check If I am at the end of the file. if yes done_reading=1
            done_reading=f_eof(mod_som_sdio_file_ptr->fp);
        	}
        } else
        {
        	mod_som_io_print_f("\nRead error data file still open\n");
        }
        res = f_close(mod_som_sdio_file_ptr->fp);
		mod_som_io_print_f("\nDone reading %s\r\n",mod_som_sdio_file_ptr->file_name);

    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a scan files on a SD Volume
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    int idx;
    static FILINFO fno;
    char buf[256];
    TCHAR str[256];

    for (idx = 0; idx < strlen(path); ++idx){
    	str[idx] = ff_convert (path[idx], 1);
    }
    str[idx] = '\0';

    res = f_opendir(&dir,str);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                for (idx = 0; idx < 256; ++idx){
                	buf[idx] = ff_convert (fno.fname[idx], 0);
                }


                sprintf(&path[i], "/%s", buf);
//                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                int idx;
                for (idx = 0; idx < 256; ++idx){
                	buf[idx] = ff_convert (fno.fname[idx], 0);
                }
            	mod_som_io_print_f("%s/%s\r\n", path, buf);//MHA added \r
            }
        }
        f_closedir(&dir);
    }

    return res;
}


/*******************************************************************************
 * @brief
 *   a function to list the files
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_ls_sd_f(){


	    char buff[256];
	    strcpy(buff, "");
	    FRESULT res;
	    res = scan_files(buff);
	    if (res != FR_OK) {
	    	mod_som_io_print_f("can not read Volume");
		    return MOD_SOM_SDIO_STATUS_FAIL_READVOLUME;
	    }
	    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   a function to remove a file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_rm_sd_f(char* path){


	    FRESULT res;
	    int idx;

	    TCHAR str[256];
	    for (idx = 0; idx < strlen(path); ++idx){
	    	str[idx] = ff_convert (path[idx], 1);
	    }
	    str[idx] = '\0';
	    res=f_unlink (str);
	    if (res != FR_OK) {
	    	mod_som_io_print_f("can not remove %s\r\n", path);
		    return MOD_SOM_SDIO_STATUS_FAIL_READVOLUME;
	    }
	    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @function
 *     mod_som_sdio_decode_status_f
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
uint8_t mod_som_sdio_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_SDIO_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_sdio_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
 *     lower 16 bits are reserved for flags
 * @param       mod_som_sdio_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_sdio_encode_status_f(uint8_t mod_som_sdio_status){
    if(mod_som_sdio_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_SDIO_STATUS_PREFIX, mod_som_sdio_status);
}


/*******************************************************************************
 * @function
 *     mod_som_sdio_set_max_size_of_complete_data_block_f
 * @abstract
 *     Set maximum size in bytes of the largest data block
 * @discussion
 *     Set maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O, this must be set before initialization
 * @return
 *     status would indicate error if set after initialization
 ******************************************************************************/
mod_som_status_t mod_som_sdio_set_max_size_of_complete_data_block_f(uint32_t max_size){
    if(mod_som_sdio_struct.initialized_flag){
        return mod_som_sdio_encode_status_f(
                MOD_SOM_SDIO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT);
    }
    mod_som_sdio_struct.max_size_of_complete_data_block = max_size;
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @function
 *     mod_som_sdio_get_max_size_of_complete_data_block_f
 * @abstract
 *     Get maximum size in bytes of the largest data block
 * @discussion
 *     Get maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O
 * @return
 *     size as a unsigned 32 bit number
 ******************************************************************************/
uint32_t mod_som_sdio_get_max_size_of_complete_data_block_f(void){
    return mod_som_sdio_struct.max_size_of_complete_data_block;
}

///*******************************************************************************
// * @function
// *     mod_som_sdio_init_f
// * @abstract
// *     Initializes MOD SOM I/O and start MOD SOM I/O Task for data piping
// * @discussion
// *     The initialization function creates a dynamic memory pool of memory block
// *     of the size of the largest data block to transfer/pipe. It also
// *     initialized the queue for transfer. And it create and OS task via
// *     OSTaskCreate to run in parallels with the main task
// *     TODO we need to work out the MUTEX part of things
// * @return
// *     status would indicate error in initialization
// ******************************************************************************/
//mod_som_status_t mod_som_sdio_init_f(void){
//    mod_som_sdio_struct.initialized_flag = false;
//    mod_som_sdio_struct.started_flag = false;
//    if(mod_som_sdio_struct.max_size_of_complete_data_block == 0)
//        mod_som_sdio_struct.max_size_of_complete_data_block = MOD_SOM_SDIO_DEFAULT_SIZE_LARGEST_BLOCK;
////    if(mod_som_sdio_struct.started_flag)
////        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_ALREADY_STARTED);
//    RTOS_ERR err;
//
//    OSQCreate(&mod_som_sdio_struct.msg_queue,           /*   Pointer to user-allocated message queue.          */
//            "MOD SOM Message Queue",          /*   Name used for debugging.                          */
//            MOD_SOM_SDIO_MSG_QUEUE_COUNT,                     /*   Queue will have 10 messages maximum.              */
//            &err);
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE);
//
//    OSQFlush(&mod_som_sdio_struct.msg_queue,&err);
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_ALREADY_STARTED);
//
//    Mem_DynPoolCreate(
//            "MOD SOM IO Dynamic Memory Pool",
//            &mod_som_sdio_struct.dyn_mem_pool,
//            DEF_NULL,
//            sizeof(mod_som_sdio_xfer_t)+mod_som_sdio_struct.max_size_of_complete_data_block,
//            sizeof(CPU_ALIGN),//LIB_MEM_BUF_ALIGN_AUTO,
//            MOD_SOM_SDIO_MSG_QUEUE_COUNT,
//            2*MOD_SOM_SDIO_MSG_QUEUE_COUNT,
//            &err);
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY);
//
//
////    Mem_DynPoolCreate(
////            "MOD SOM IO circular buffer list Dynamic Memory Pool",
////            &mod_som_sdio_struct.cir_buff_dyn_mem_pool_ptr,
////            DEF_NULL,
////            sizeof(mod_som_sdio_cir_buff_list_item_t),
////            LIB_MEM_BUF_ALIGN_AUTO,
////            16,
////            128,
////            &err);
////    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
////    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
////        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);
////    mod_som_sdio_struct.cir_buff_list_head_ptr = DEF_NULL;
////    mod_som_sdio_struct.cir_buff_list_head_ptr = DEF_NULL;
//
//    mod_som_sdio_struct.initialized_flag = true;
//    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
//}

/*******************************************************************************
 * @function
 *     mod_som_sdio_start_f
 * @abstract
 *     Start the background task for MOD SOM I/O
 * @discussion
 *
 * @return
 *     status would indicate error in initialization
 ******************************************************************************/
mod_som_status_t mod_som_sdio_start_f(void){

    if(!mod_som_sdio_struct.initialized_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
    RTOS_ERR err;

    OSTaskCreate(&mod_som_sdio_struct.print_task_tcb, // Create the Start Print Task
            "MOD SOM SDIO Print Task",
            (OS_TASK_PTR)mod_som_sdio_print_task_f,
            DEF_NULL,
            MOD_SOM_SDIO_PRINT_TASK_PRIORITY,
            &mod_som_sdio_struct.print_task_stack[0],
            (MOD_SOM_SDIO_TASK_STK_SIZE / 10u),
            MOD_SOM_SDIO_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_TASK);

    mod_som_sdio_struct.started_flag = true;
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_sdio_write_data_f
 * @abstract
 *     queue up data to write on the SD card
 * @discussion
 *     this function copy data over, queue it up, and stream it out
 *     once it done, it would indicate in the parameter done_streaming_flag_ptr
 * @param data_ptr
 *     pointer to data to stream
 * @param data_length
 *     length of data to stream
 * @param done_streaming_flag_ptr
 *     pointer to a boolean flag to indicate when streaming is done
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_sdio_write_data_f(const uint8_t *data_ptr, uint32_t data_length,
        volatile bool * done_streaming_flag_ptr){
    if(!mod_som_sdio_struct.initialized_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_sdio_struct.started_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED);

    mod_som_sdio_xfer_ptr_t mod_som_sdio_xfer_item_ptr;
    mod_som_status_t mod_som_status;
	  mod_som_status = MOD_SOM_STATUS_OK;

    mod_som_sdio_xfer_item_ptr = mod_som_sdio_new_xfer_item_f();

    if(mod_som_sdio_xfer_item_ptr == DEF_NULL)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);

    mod_som_sdio_xfer_item_ptr->done_streaming_flag_ptr = done_streaming_flag_ptr;

    if(done_streaming_flag_ptr != DEF_NULL)
        *(mod_som_sdio_xfer_item_ptr->done_streaming_flag_ptr)  = false;

    mod_som_sdio_xfer_item_ptr->data_ptr = data_ptr;
    mod_som_sdio_xfer_item_ptr->data_length = data_length;

    //ALB add to queue the  new xfer item
    mod_som_status = mod_som_sdio_add_to_queue_f(mod_som_sdio_xfer_item_ptr);

    // Free the xfer item memory if add to queue fails
    if(mod_som_status != MOD_SOM_STATUS_OK){
        mod_som_sdio_free_xfer_item_f(mod_som_sdio_xfer_item_ptr);
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ENQUEUE);
    }

    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_sdio_print_task_f
 * @abstract
 *     MOD SOM I/O OS task that would run in parallel with the main task
 * @discussion
 *     This task would dequeue the MOD SOM I/O transfer as the data is piped out
 * @param       p_arg
 *     argument passed in by OSTaskCreate (not used)
 ******************************************************************************/
static void mod_som_sdio_print_task_f(void *p_arg)
{

    (void)p_arg; // Deliberately unused argument
    RTOS_ERR err;

    mod_som_sdio_xfer_ptr_t tmp_mod_som_sdio_xfer_item_ptr = DEF_NULL;
    OS_MSG_SIZE tmp_mod_som_sdio_xfer_item_size;
    CPU_TS time_passed_msg_pend;
    uint32_t i;

//    CORE_DECLARE_IRQ_STATE;

//    uint32_t n_count = 0;
    while(DEF_ON){
        //necessary for every task
        tmp_mod_som_sdio_xfer_item_ptr =
                (mod_som_sdio_xfer_ptr_t)OSQPend(&mod_som_sdio_struct.msg_queue,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        &tmp_mod_som_sdio_xfer_item_size,
                        &time_passed_msg_pend,
                        &err);

        if (mod_som_sdio_struct.new_file_flag)
          {
            mod_som_sdio_open_next_datafile();
            mod_som_sdio_struct.new_file_flag=false;
          }

        //TODO track print
        if(tmp_mod_som_sdio_xfer_item_ptr->is_printf_mode)
          {
            //body of data
            for(i=0;i<tmp_mod_som_sdio_xfer_item_ptr->printf_str_length;i++)
              {
                putchar(tmp_mod_som_sdio_xfer_item_ptr->printf_str_ptr[i]);
              }
          }else
            {

            if(mod_som_sdio_struct.data_file_ptr->is_open_flag==1)
              {
                int remaining_bytes;
                int bytes_to_send=MOD_SOM_SDIO_BLOCK_LENGTH;
                UINT byteswritten;
                FRESULT res;
                remaining_bytes=tmp_mod_som_sdio_xfer_item_ptr->data_length;

                while(remaining_bytes>0)
                  {
                    if (remaining_bytes<MOD_SOM_SDIO_BLOCK_LENGTH)
                      {
                        bytes_to_send=remaining_bytes;
                      }
                    res = f_write(mod_som_sdio_struct.data_file_ptr->fp,\
                            &tmp_mod_som_sdio_xfer_item_ptr->data_ptr[tmp_mod_som_sdio_xfer_item_ptr->data_length-remaining_bytes],bytes_to_send, &byteswritten); /* buffptr = pointer to the data to be written */
                    remaining_bytes=remaining_bytes-byteswritten;
                    if (res != FR_OK)
                      {
                        //TODO write an error code
                      }
                  }
                  f_sync(mod_som_sdio_struct.data_file_ptr->fp);

                  if (mod_som_calendar_get_time_f()-
                    mod_som_sdio_struct.open_file_time>=
                    mod_som_sdio_struct.mod_som_sdio_settings_ptr->file_duration)
                    {
                      //ALB suspend the task.
                      mod_som_sdio_struct.new_file_flag=true;
                    }

              } else{
                  mod_som_io_print_f("Write error data file not open\n");
              }
              if(tmp_mod_som_sdio_xfer_item_ptr->done_streaming_flag_ptr!=DEF_NULL){
                  *(tmp_mod_som_sdio_xfer_item_ptr->done_streaming_flag_ptr)  = true;
              }
            }//end if printf
          //free memory
          mod_som_sdio_free_xfer_item_f(tmp_mod_som_sdio_xfer_item_ptr);
      }//end while DEF_ON
  }//end function

/*******************************************************************************
 * @function
 *     mod_som_sdio_new_xfer_item_f
 * @abstract
 *     Create new I/O transfer item
 * @discussion
 *     Using dynamic memory pool for IO transfer item to create new memory
 *     pointer for a new I/O transfer item
 * @return
 *     Pointer to new I/O transfer item, if pointer is null, this indicates
 *     error in allocating memory
 ******************************************************************************/
mod_som_sdio_xfer_ptr_t mod_som_sdio_new_xfer_item_f(void){
    if(!mod_som_sdio_struct.initialized_flag)
        return DEF_NULL;
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_sdio_struct.started_flag)
        return DEF_NULL;
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED);
    RTOS_ERR err;
    mod_som_sdio_xfer_ptr_t mod_som_sdio_xfer_item_ptr =
            (mod_som_sdio_xfer_ptr_t)Mem_DynPoolBlkGet(
                    &mod_som_sdio_struct.dyn_mem_pool,
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sdio_xfer_item_ptr==DEF_NULL)
        return DEF_NULL;

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return DEF_NULL;

    Mem_Set(mod_som_sdio_xfer_item_ptr,0,sizeof(mod_som_sdio_xfer_t));
    //    mod_som_sdio_xfer_item_ptr->next_item_ptr = DEF_NULL;
    mod_som_sdio_xfer_item_ptr->printf_str_ptr = ((uint8_t *)mod_som_sdio_xfer_item_ptr) + sizeof(mod_som_sdio_xfer_t);
    mod_som_sdio_xfer_item_ptr->printf_str_length = 0;
    mod_som_sdio_xfer_item_ptr->data_length = 0;
    mod_som_sdio_xfer_item_ptr->is_printf_mode = false;
    mod_som_sdio_xfer_item_ptr->size = sizeof(mod_som_sdio_xfer_t);
    //this is so we can find the wrapper class in case we need to clear memory
    return mod_som_sdio_xfer_item_ptr;
}

/*******************************************************************************
 * @function
 *     mod_som_sdio_free_xfer_item_f
 * @abstract
 *     Free memory of I/O transfer item
 * @discussion
 *     Since memory of I/O transfer item is in a dynamic memory pool, item freed
 *     allows new items to be created
 * @param       xfer_item_ptr
 *     Item to free
 * @return
 *     Status of success or failure of freeing transfer item
 ******************************************************************************/
mod_som_status_t mod_som_sdio_free_xfer_item_f(mod_som_sdio_xfer_ptr_t xfer_item_ptr){
    RTOS_ERR err;
    if(!mod_som_sdio_struct.initialized_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_sdio_struct.started_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED);

    Mem_DynPoolBlkFree(&mod_som_sdio_struct.dyn_mem_pool,
            (void *)xfer_item_ptr,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_FREE_MEMORY);
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @function
 *     mod_som_sdio_add_to_queue_f
 * @abstract
 *     Add I/O transfer item to queue
 * @discussion
 *     Add I/O transfer item to queue (FIFO), update current tail next item, and
 *     tail to queue to current item
 * @param       xfer_item_ptr
 *     Item to add
 * @return
 *     Status of adding to queue
 ******************************************************************************/
mod_som_status_t mod_som_sdio_add_to_queue_f(mod_som_sdio_xfer_ptr_t xfer_item_ptr){
    if(!mod_som_sdio_struct.initialized_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_sdio_struct.started_flag)
        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED);
    RTOS_ERR err;

    //ALB check the Nb of entries and pass if it the list is full.
    //ALB TODO change 60 to a
    if(mod_som_sdio_struct.msg_queue.MsgQ.NbrEntries>SDIO_OVF_MSG_LIST_THERSHOLD){
    	mod_som_sdio_struct.listoverflow_flag=true;
    	//ALB TODO say How I want to handle the overflow?
    	//ALB TODO check for ovf.
    	//ALB TODO handle ovf.
    	//ALB TODO flag the ovf.
    	//ALB TODO So the user knows how much time or sample we missed
    }else{
    	mod_som_sdio_struct.listoverflow_flag=false;
    }

    if (!mod_som_sdio_struct.listoverflow_flag){
    	OSQPost (&mod_som_sdio_struct.msg_queue,
    			xfer_item_ptr,
				xfer_item_ptr->size,
				OS_OPT_POST_FIFO,
				&err);

    	if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)){
    		mod_som_sdio_free_xfer_item_f(xfer_item_ptr);
    		return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE);
    	}
//    	printf("\n add to queue Nb entries %u \n",mod_som_sdio_struct.msg_queue.MsgQ.NbrEntries);

    }else{
		mod_som_sdio_free_xfer_item_f(xfer_item_ptr);
    	printf("\n Not adding to SD queue Nb entries %u \n",mod_som_sdio_struct.msg_queue.MsgQ.NbrEntries);
    }
    return mod_som_sdio_encode_status_f(MOD_SOM_STATUS_OK);
}

