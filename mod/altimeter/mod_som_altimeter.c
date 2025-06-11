/*
 * mod_som_.c
 *
 *  Created on: January 4th, 2021
 *  aleboyer@ucsd.edu
 */
#include <mod_som_altimeter.h>
#include <mod_som_altimeter_bsp.h>
#include "mod_som_io.h"
#include "mod_som_priv.h"

#ifdef  MOD_SOM_ALTIMETER_EN

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <mod_som_altimeter_cmd.h>
#include <shell_util.h>
#endif
#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif
#ifdef MOD_SOM_AGGREGATOR_EN
  #include <mod_som_aggregator.h>
#endif

#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//ALB Altimeter task
static CPU_STK altimeter_task_stk[MOD_SOM_ALTIMETER_TASK_STK_SIZE];
static OS_TCB  altimeter_task_tcb;

//ALB Sleep timer status
sl_status_t mystatus;

//ALB MAIN module structure
mod_som_altimeter_ptr_t mod_som_altimeter_ptr;

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;

/*******************************************************************************
 * @brief
 *   Initialize Altimeter, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_init_f(){
    mod_som_status_t status;
    RTOS_ERR  err;

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_altimeter_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_altimeter_encode_status_f(MOD_SOM_ALTIMETER_STATUS_FAIL_INIT_CMD);
#endif


    // ALB allocate memory for the module_ptr.
    // ALB The module_ptr is also the "scope" of the runtime_ptr
    // ALB but the module_ptr also contains the settings_ptr and the config_ptr
    // ALB The settings_ptr an config_ptr should allocated and defined during the module initialization
    mod_som_altimeter_ptr = (mod_som_altimeter_ptr_t)Mem_SegAlloc(
        "MOD SOM ALTIMETER RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_altimeter_t),
        &err);

    //SN Check error code
    //ALB WARNING: The following line hangs in a while loop -
    //ALB WARNING: - if the previous allocation fails.
    //ALB TODO change return -1 to return the an appropriate error code.
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_altimeter_ptr==DEF_NULL){
      printf("ALTIMETER not initialized\n\r");
      return -1;
    }

    //ALB Initialize the runtime flag module_ptr->initialized_flag to false.
    //ALB It will be set to true once the module is initialized at the end of mod_som_altimeter_init_f().
    mod_som_altimeter_ptr->initialized_flag = false;

    // ALB allocate memory for the settings_ptr.
    // ALB WARNING: The setup pointer CAN NOT have pointers inside.
    status |= mod_som_altimeter_allocate_settings_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("ALTIMETER not initialized\n\r");
      return status;
    }


    // ALB checking if a previous EFE setup exist, from the setup module (i.e. setup file or UserData setup)
  #ifdef MOD_SOM_SETTINGS_EN
      mod_som_settings_struct_ptr_t local_settings_ptr=mod_som_settings_get_settings_f();
      mod_som_altimeter_ptr->settings_ptr=&local_settings_ptr->mod_som_altimeter_settings;
  #else
      mod_som_altimeter_ptr->settings_ptr->initialize_flag=false;
  #endif

      // ALB If no pre-existing settings, use the default settings
      if (!mod_som_altimeter_ptr->settings_ptr->initialize_flag){
          // ALB initialize default settings_ptr.
          // TODO add the conditional download of the pre existing settings in the user data page.
          status |= mod_som_altimeter_default_settings_f(mod_som_altimeter_ptr->settings_ptr);
          if (status!=MOD_SOM_STATUS_OK){
              printf("ALTIMETER not initialized\n\r");
              return status;
          }
      }

    // ALB allocate memory for config ptr.
    status |= mod_som_altimeter_allocate_config_f();
    if (status!=MOD_SOM_STATUS_OK){
        printf("ALTIMETER not initialized\n\r");
        return status;
    }

    // ALB initialize config ptr.
    mod_som_altimeter_config_f(mod_som_altimeter_ptr->config_ptr);

    mod_som_altimeter_ptr->t0=0;
    mod_som_altimeter_ptr->t_echo=0;
    mod_som_altimeter_ptr->status=MOD_SOM_STATUS_OK;
    mod_som_altimeter_ptr->mode=0;

    // ALB initialize altimeter timers.
    mod_som_altimeter_init_timer_f();

    mod_som_altimeter_ptr->initialized_flag = false;
    printf("%s initialized\r\n",MOD_SOM_ALTIMETER_HEADER);


    return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);

}



/*******************************************************************************
 * @brief
 *   allocate memory for config_ptr
 *   uint32_t initialized_flag;
 *   mod_som_timer_handle_ptr_t tx_timer;
 *   mod_som_timer_handle_ptr_t echo_timer;
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_allocate_config_f(){
//    mod_som_status_t status;
    RTOS_ERR  err;

    //ALB alloc memory for setup pointer
    //set up default configuration
    mod_som_altimeter_ptr->config_ptr =
        (mod_som_altimeter_config_ptr_t)Mem_SegAlloc(
            "MOD SOM ALTIMETER config.",DEF_NULL,
            sizeof(mod_som_altimeter_config_t),
            &err);

    mod_som_altimeter_ptr->config_ptr->tx_timer_ptr =
        (mod_som_timer_handle_ptr_t)Mem_SegAlloc(
            "MOD SOM ALTIMETER tx.",DEF_NULL,
            sizeof(mod_som_timer_handle_t),
            &err);

    mod_som_altimeter_ptr->config_ptr->echo_timer_ptr =
        (mod_som_timer_handle_ptr_t)Mem_SegAlloc(
            "MOD SOM ALTIMETER echo.",DEF_NULL,
            sizeof(mod_som_timer_handle_t),
            &err);

    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_altimeter_ptr->config_ptr==NULL)
    {
      mod_som_altimeter_ptr = DEF_NULL;
      return MOD_SOM_ALTIMETER_CANNOT_ALLOCATE_CONFIG;
    }
    return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *   uint32_t initialized_flag;
 *   mod_som_timer_handle_ptr_t tx_timer_ptr;
 *   mod_som_timer_handle_ptr_t echo_timer;
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_config_f(mod_som_altimeter_config_ptr_t config_ptr){

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  config_ptr->initialized_flag = false;
  config_ptr->tx_pulse_start = 0; // use of CCO to start the tx pulse when cnt =0
  config_ptr->tx_timer_ptr->timer = (void *)MOD_SOM_ALTIMETER_TX_TIMER;
  config_ptr->tx_timer_ptr->top            = MOD_SOM_ALTIMETER_TX_TOP;
  config_ptr->tx_timer_ptr->compare_value  = MOD_SOM_ALTIMETER_TX_CC;
  config_ptr->tx_timer_ptr->route_location = MOD_SOM_ALTIMETER_TX_ROUTE;
  config_ptr->tx_timer_ptr->timer_pin.pin  = MOD_SOM_ALTIMETER_TX_PIN;
  config_ptr->tx_timer_ptr->timer_pin.port = MOD_SOM_ALTIMETER_TX_PORT;
  config_ptr->tx_timer_ptr->timer_clock    = MOD_SOM_ALTIMETER_TX_CLK;
// ALB I can not figure out how to initialize this
//  config_ptr->tx_timer_ptr->timer_init     = TIMER_INIT_DEFAULT;


  config_ptr->echo_timer_ptr->timer = (void *)MOD_SOM_ALTIMETER_ECHO_TIMER;
  config_ptr->echo_timer_ptr->top             = MOD_SOM_ALTIMETER_ECHO_TOP;
  config_ptr->echo_timer_ptr->compare_value   = MOD_SOM_ALTIMETER_ECHO_CC;
  config_ptr->echo_timer_ptr->route_location  = MOD_SOM_ALTIMETER_ECHO_ROUTE;
  config_ptr->echo_timer_ptr->timer_pin.pin   = MOD_SOM_ALTIMETER_ECHO_PIN;
  config_ptr->echo_timer_ptr->timer_pin.port  = MOD_SOM_ALTIMETER_ECHO_PORT;
  config_ptr->echo_timer_ptr->timer_clock     = MOD_SOM_ALTIMETER_ECHO_CLK;
  config_ptr->initialized_flag = true;

  //ALB record length
  config_ptr->header_length = MOD_SOM_ALTIMETER_SYNC_HEADER_LENGTH     +
                              MOD_SOM_ALTIMETER_TAG_LENGTH             +
                              MOD_SOM_ALTIMETER_HEXTIMESTAMP_LENGTH    +
                              MOD_SOM_ALTIMETER_HEXPAYLOAD_LENGTH      +
                              MOD_SOM_ALTIMETER_HEADER_CHECKSUM_LENGTH;

  config_ptr->record_length = config_ptr->header_length +
                              MOD_SOM_ALTIMETER_RECORD_ENDBYTES_LENGTH;

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   construct default settings_ptr
 *
 *   uint16_t size;
 *   char header[3];
 *   char rev[5];
 *   char sn[3];
 *   uint32_t tx_repetition_period; // see details above
 *   uint8_t  tx_repetition_mode;   // see details above
 *   uint32_t tx_pulse_width;       // micro-seconds
 *   uint32_t blanking_interval;    // micro-seconds
    uint32_t ping_interval;    // micrium ticks - MHA
 *   bool initialize_flag;
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_allocate_settings_f(){

  RTOS_ERR  err;

  //ALB alloc memory for setup pointer
  //set up default configuration
  mod_som_altimeter_ptr->settings_ptr =
      (mod_som_altimeter_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM ALTIMETER setup.",DEF_NULL,
          sizeof(mod_som_altimeter_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_altimeter_ptr->settings_ptr==NULL)
  {
    mod_som_altimeter_ptr = DEF_NULL;
    return MOD_SOM_ALTIMETER_CANNOT_ALLOCATE_SETUP;
  }


  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Initialize setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_default_settings_f(mod_som_altimeter_settings_ptr_t settings_ptr){

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.


  //initialize settings fields
  strcpy(settings_ptr->header,MOD_SOM_ALTIMETER_HEADER);
  strcpy(settings_ptr->rev,MOD_SOM_ALTIMETER_DEFAULT_REV);
  strcpy(settings_ptr->sn,MOD_SOM_ALTIMETER_DEFAULT_SN);

  settings_ptr->blanking_interval=MOD_SOM_ALTIMETER_DEFAULT_BLANKING_INTERVAL;
  settings_ptr->tx_pulse_width=MOD_SOM_ALTIMETER_DEFAULT_TX_PULSE;
  settings_ptr->tx_repetition_mode=MOD_SOM_ALTIMETER_DEFAULT_TX_MODE;
  settings_ptr->tx_repetition_period=MOD_SOM_ALTIMETER_DEFAULT_TX_PERIOD;
  settings_ptr->ping_interval=MOD_SOM_ALTIMETER_DELAY; //MHA

  settings_ptr->initialize_flag=true;
  settings_ptr->size=sizeof(*settings_ptr);

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * @brief  mod_som_altimeter_start_task_f
 *   start the altimeter task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_altimeter_start_task_f(){

  RTOS_ERR err;
  // Consumer Task 2
   mod_som_altimeter_ptr->data_state=not_ready;
   mod_som_altimeter_ptr->consumed_flag=true;

   OSTaskCreate(&altimeter_task_tcb,
                        "altimeter task",
                        mod_som_altimeter_task_f,
                        DEF_NULL,
                        MOD_SOM_ALTIMETER_TASK_PRIO,
            &altimeter_task_stk[0],
            (MOD_SOM_ALTIMETER_TASK_STK_SIZE / 10u),
            MOD_SOM_ALTIMETER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_altimeter_ptr->status = mod_som_altimeter_encode_status_f(MOD_SOM_ALTIMETER_STATUS_FAIL_TO_START_ALTIMETER_TASK));
  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_altimeter_task_f
 *   stop the altimeter task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_altimeter_stop_task_f(){

  RTOS_ERR err;

  OSTaskDel(&altimeter_task_tcb,
            &err);

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_altimeter_task_f
 *
 *   the altimeter works on different modes
 *   0: snapshot
 *   1: period in ms/kernel's tick
 *   2: phased on efe sampling
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

static  void  mod_som_altimeter_task_f(void  *p_arg){
    RTOS_ERR  err;
    uint32_t t_hex[2];
    char * curr_msg_ptr;
    uint8_t * local_header;

    mod_som_timer_handle_ptr_t local_tx_timer_ptr= \
                                  mod_som_altimeter_ptr->config_ptr->tx_timer_ptr;


    while (DEF_ON) {

        switch(mod_som_altimeter_ptr->data_state){
          case standby:
            mod_som_altimeter_stop_task_f();
            break;
          case collecting:
            //ALB nothing to do in stand by mode (i.e snapshot,mode 0)
            break;
          case not_ready:
            //ALB Start collecting data.
            //ALB
            mod_som_altimeter_ptr->data_state=collecting;
            //ALB CCO_CTRL 0x502: Output compare, CMOA toggle, COFOA toggle.
            local_tx_timer_ptr->timer->CC[MOD_SOM_ALTIMETER_TX_CC_CHANNEL].CTRL =0x102;
            //ALB set CMAO =0
            TIMER_CompareSet(local_tx_timer_ptr->timer,0, \
                             mod_som_altimeter_ptr->config_ptr->tx_pulse_start);
            TIMER_CounterSet(local_tx_timer_ptr->timer,0); // set CNT=0
            TIMER_IntEnable(local_tx_timer_ptr->timer, TIMER_IEN_CC0);
            break;
          case ready:
            curr_msg_ptr=(char*) mod_som_altimeter_ptr->msg;


            //ALB convert timestamp in hex
            t_hex[0] = (uint32_t) (mod_som_altimeter_ptr->timestamp>>32);
            t_hex[1] = (uint32_t) mod_som_altimeter_ptr->timestamp;

            //ALT header + hex timestamp. It will be used in the IRQ handler to send
            // a message to the IO stream or store task
            sprintf((char *) mod_som_altimeter_ptr->msg,  \
                "$%s%08x%08x%08lu*FF\r\n", \
                mod_som_altimeter_ptr->settings_ptr->header, \
                (int) t_hex[0],\
                (int) t_hex[1],
                mod_som_altimeter_ptr->t_echo/2/500);

            mod_som_altimeter_ptr->chksum=0;
            for(int i=0;i<mod_som_altimeter_ptr->config_ptr->record_length-MOD_SOM_ALTIMETER_CHCKSUM_LENGTH;i++)
              {
              mod_som_altimeter_ptr->chksum ^=\
                  *(curr_msg_ptr++);
              }

            //write checksum at the end of the steam block (record).
            local_header = (uint8_t *) &mod_som_altimeter_ptr->msg[mod_som_altimeter_ptr->config_ptr->header_length-
                                                       MOD_SOM_ALTIMETER_HEADER_CHECKSUM_LENGTH+1];
            *((uint16_t*)local_header) = \
                mod_som_int8_2hex_f(mod_som_altimeter_ptr->chksum);


            //ALB consumers mode
            switch(mod_som_altimeter_ptr->mode){
              case 0:
                //ALB direct stream
                mod_som_altimeter_ptr->consumed_flag=false;
                mod_som_io_stream_data_f((uint8_t *) &mod_som_altimeter_ptr->msg, \
                                         strlen(mod_som_altimeter_ptr->msg),   \
                                         &mod_som_altimeter_ptr->consumed_flag);
                break;
              case 1:
                //ALB SD store
                mod_som_altimeter_ptr->consumed_flag=false;
                mod_som_sdio_write_data_f((uint8_t *) &mod_som_altimeter_ptr->msg, \
                                          strlen(mod_som_altimeter_ptr->msg),   \
                                          &mod_som_altimeter_ptr->consumed_flag);
                break;
              case 2:
                //ALB Aggregator
                printf("ALTI aggreg \r\n");

//                mod_som_altimeter_ptr->consumed_flag=false;
//                mod_som_aggregator_process_data_f(
//                    (uint8_t *) &mod_som_altimeter_ptr->msg,
//                    strlen(mod_som_altimeter_ptr->msg),
//                    &mod_som_altimeter_ptr->consumed_flag);
                break;
            }

            //ALB repeat the sampling or not.
            if (mod_som_altimeter_ptr->settings_ptr->tx_repetition_mode==0){
              mod_som_altimeter_ptr->data_state=standby;
            }else{
                if(mod_som_altimeter_ptr->consumed_flag){
                    mod_som_altimeter_ptr->data_state=not_ready;
                }
            }
            break;

        }

//        // Delay Start Task execution for
//        OSTimeDly( MOD_SOM_ALTIMETER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
//                   OS_OPT_TIME_DLY,          //   from now.
//                   &err);
        //MHA: use the ping_interval (settable) rather than the compiler-flag default value
        // Delay Start Task execution for
        OSTimeDly( mod_som_altimeter_ptr->settings_ptr->ping_interval,             //   consumer delay is #define at the beginning OS Ticks
                   OS_OPT_TIME_DLY,          //   from now.
                   &err);

        //   Check error code.
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
    } // end of while (DEF_ON)

    PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.
}


/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_altimeter_settings_t mod_som_altimeter_get_settings_f(){
  return *mod_som_altimeter_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get the config pointer.
 *   to be used by other modules
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_altimeter_config_ptr_t mod_som_altimeter_get_config_f(){
return  mod_som_altimeter_ptr->config_ptr;
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
uint8_t mod_som_altimeter_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_ALTIMETER_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_altimeter_encode_status_f
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
mod_som_status_t mod_som_altimeter_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_ALTIMETER_STATUS_PREFIX, mod_som_io_status);
}



/*******************************************************************************
 * @brief
 *   enable atimeter hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_enable_hardware_f(){

  //ALB TODO
  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   disable ALTIMETER hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_disable_hardware_f(){

  //ALB TODO

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   a function to get the altimeter id
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_altimeter_id_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  if (argc==1){
    printf("ALTIMETER, %s, %s.\r\n",(char*) mod_som_altimeter_ptr->settings_ptr->rev ,(char*) mod_som_altimeter_ptr->settings_ptr->sn);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 3:
      strcpy(mod_som_altimeter_ptr->settings_ptr->rev,argv[1]);
      strcpy(mod_som_altimeter_ptr->settings_ptr->sn,argv[2]);
      break;
    default:
      printf("format: alti.id rev0 001\r\n");
      break;
    }
  }
}


/*******************************************************************************
 * @brief
 *   set the altimeter mode
 *
 *   tx_repetition_mode:
 *   0: tx one shot altimeter (default)
 *   1: units in kernel ticks
 *   2: unit of epsilon sample period
 *   3: in mili-seconds.
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t  mod_som_altimeter_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  uint8_t str_mode;
  RTOS_ERR p_err;

  if (argc==1){
      str_mode= mod_som_altimeter_ptr->mode;
      printf("ALTIMETER mode, %i.\r\n", str_mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_altimeter_ptr->mode=(uint8_t) shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: alti.mode mode (0: stream, 1: SD store, 2: aggregator)\r\n");
      break;
    }
  }

  // ALB set data_state to ready to enable CC0 in altimeter_task_f
  mod_som_altimeter_ptr->data_state=ready;


  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   set the altimeter mode
 *
 *   tx_repetition_mode:
 *   0: tx one shot altimeter (default)
 *   1: units in kernel ticks
 *   2: unit of epsilon sample period
 *   3: in mili-seconds.
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t  mod_som_altimeter_repmode_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  uint8_t str_mode;
  RTOS_ERR p_err;

  if (argc==1){
      str_mode= mod_som_altimeter_ptr->settings_ptr->tx_repetition_mode;
      printf("ALTIMETER rep mode, %i.\r\n", str_mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_altimeter_ptr->settings_ptr->tx_repetition_mode=(uint8_t) shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: alti.repmode mode (mode=0,1,2,3)\r\n");
      break;
    }
  }

  // ALB set data_state to ready to enable CC0 in altimeter_task_f
  mod_som_altimeter_ptr->data_state=ready;


  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   set the altimeter blank period
 *
 *   argv[1]= blank period in us
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t  mod_som_altimeter_set_blank_f(CPU_INT16U argc,CPU_CHAR *argv[]){
//MHA comments.  blanking_interval is in ticks.  So blanking_interval/50000000 is the interval in seconds. blanking_interval/50 is in microsecs.
  uint32_t str_mode; //MHA was unit8
  RTOS_ERR p_err;

  if (argc==1){
      str_mode= mod_som_altimeter_ptr->settings_ptr->blanking_interval;
//      printf("ALTIMETER blanking period , %i us.\r\n", str_mode/50000000);
      printf("ALTIMETER blanking period , %i us.\r\n", (uint32_t)(str_mode/50)); //MHA convert to microseconds from ticks
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_altimeter_ptr->settings_ptr->blanking_interval=
          shellStrtol(argv[1],&p_err)*50; //MHA convert to ticks from microseconds
      break;
    default:
      printf("format: alti.blank blank_period  (in micro sec).\r\n");
      break;
    }
  }

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   set the altimeter pulse period
 *
 *   argv[1]= pulse period in us
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t  mod_som_altimeter_set_pulse_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  uint32_t str_mode; //MHA was unit8
  RTOS_ERR p_err;

  if (argc==1){
      str_mode= mod_som_altimeter_ptr->settings_ptr->tx_pulse_width;
      printf("ALTIMETER pulse period , %i us.\r\n", (uint32_t)(str_mode/50)); //MHA convert to microseconds from ticks
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_altimeter_ptr->settings_ptr->tx_pulse_width=
          shellStrtol(argv[1],&p_err)*50; //MHA convert to ticks from microseconds
      break;
    default:
      printf("format: alti.pulse pulse_period  (in micro sec).\r\n");
      break;
    }
  }

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}

//MHA
/*******************************************************************************
 * @brief
 *   set the altimeter reprate
 *
 *   argv[1]= pulse period in us
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t  mod_som_altimeter_set_reprate_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  uint32_t str_mode; //MHA was unit8
  RTOS_ERR p_err;

  if (argc==1){
      str_mode= mod_som_altimeter_ptr->settings_ptr->ping_interval;
      printf("ALTIMETER rep rate , %i micrium ticks (500 ~ 1 sec).\r\n", (uint32_t)(str_mode)); //MHA
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_altimeter_ptr->settings_ptr->ping_interval=
          shellStrtol(argv[1],&p_err); //MHA
      break;
    default:
      printf("format: alti.pulse pulse_period  (in micrium ticks; 500 ~ 1 sec).\r\n");
      break;
    }
  }

  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);
}



/***************************************************************************//**
 * @brief
 *   mod_som_altimeter_init_f
 *   - initialize TX timer
 *   - initalize echo timer
 *
 *
 *
 *   Altimeter timer sequence
 *   - initialize the timer
 *          timerPrescale1 (highest resolution),
 *   - TOP is the periodicity of the altimeter sequence.
 *   - Output compare CC0 toggle up the pin (t0) -> Change CC0 inside IRQ and enable input compare CC1 -> CC0_IRQ toggle down pin.
 *   - Input capture CC1 capture t0 -> from there cc1 IRQ will check if events t1 are beyond blanking period in the CC1 IRQ.
 *   - (Maybe: use CC2 to freeze the pin until blanking period is over.)
 *   - If CC1 t1>blanking period -> get timing echo.
 *   - Wait for TOP interrupt.
 *
 *   -
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_init_timer_f()
{

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  //initialize TX timer
  //TODO change the following code to initialize the echo timer
  CMU_Clock_TypeDef altimeter_mclock_clk = MOD_SOM_ALTIMETER_TX_CLK;

  // DEFINE local tx timer handle just to save time on syntax
  mod_som_timer_handle_ptr_t local_tx_timer_ptr= \
                                mod_som_altimeter_ptr->config_ptr->tx_timer_ptr;

  mod_som_timer_handle_ptr_t local_echo_timer_ptr= \
                                mod_som_altimeter_ptr->config_ptr->echo_timer_ptr;

  /* enable TX GPIO pin  - using wide timer 0 */
  GPIO_PinModeSet(local_tx_timer_ptr->timer_pin.port, \
                  local_tx_timer_ptr->timer_pin.pin,\
                  gpioModePushPull, 0); //TODO add to bsp

  GPIO_PinModeSet(local_echo_timer_ptr->timer_pin.port, \
                  local_echo_timer_ptr->timer_pin.pin,\
                  gpioModeInput, 0); //TODO add to bsp


//  GPIO_PinModeSet(gpioPortC, 3, gpioModePushPull, 0);   // set SOM MEzz chekcing pins
//  GPIO_PinModeSet(gpioPortC, 3, gpioModePushPull, 1);   // set SOM MEzz chekcing pins


  CMU_ClockEnable(altimeter_mclock_clk, true); //TODO add to bsp


  // Configure TIMER2 cc0 Compare/Capture for output compare
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode  = timerCCModeCompare;

  // Configure TIMER2 cc1 Input/Capture
  TIMER_InitCC_TypeDef timerCCInit1 = TIMER_INITCC_DEFAULT;
  timerCCInit1.mode  = timerCCModeCapture;
  timerCCInit1.edge  = timerEdgeFalling;

  TIMER_InitCC(local_tx_timer_ptr->timer, MOD_SOM_ALTIMETER_TX_CC_CHANNEL, &timerCCInit);
  TIMER_InitCC(local_tx_timer_ptr->timer, MOD_SOM_ALTIMETER_ECHO_CC_CHANNEL, &timerCCInit1);


  // Route WTIMER2 CC0 to location 0 and enable CC0 route pin
  // TIM1_CC0 #0 is GPIO Pin PC13
  local_tx_timer_ptr->timer->ROUTELOC0 |= TIMER_ROUTELOC0_CC0LOC_LOC0|TIMER_ROUTELOC0_CC1LOC_LOC0;
  local_tx_timer_ptr->timer->ROUTEPEN  |= TIMER_ROUTEPEN_CC0PEN|TIMER_ROUTEPEN_CC1PEN;

  // Set top value to overflow at the desired PWM_FREQ frequency
  TIMER_TopSet(local_tx_timer_ptr->timer, \
               mod_som_altimeter_ptr->settings_ptr->tx_repetition_period);      // TOP 25e6 -> 0.5 sec

  TIMER_CompareSet(local_tx_timer_ptr->timer,0,\
                   mod_som_altimeter_ptr->config_ptr->tx_pulse_start);          // set pulse length into the compare register


  // Enable TIMER1 compare event interrupts to update the duty cycle
  mod_som_altimeter_ptr->altimeter_state=wait;
  mod_som_altimeter_ptr->data_state=not_ready;

  NVIC_EnableIRQ(WTIMER2_IRQn);

  uint32_t flags = TIMER_IntGet(mod_som_altimeter_ptr->config_ptr->tx_timer_ptr->timer);
  TIMER_IntClear(mod_som_altimeter_ptr->config_ptr->tx_timer_ptr->timer, flags);


//  // Initialize and start the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(local_tx_timer_ptr->timer, &timerInit);




  return mod_som_altimeter_encode_status_f(MOD_SOM_STATUS_OK);

}


/**************************************************************************//**
 * @brief
 *    Interrupt handler for WTIMER2 that changes the duty cycle
 *
 *     *   Altimeter timer sequence
 *   - initialize the timer
 *          timerPrescale1 (highest resolution),
 *   - TOP is the period of the altimeter sequence.
 *   - Output compare CC0 toggle up the pin (t0) -> Change CC0 inside IRQ and enable input compare CC1 -> CC0_IRQ toggle down pin.
 *   - Input capture CC1 capture t0 -> from there cc1 IRQ will check if events t1 are beyond blanking period in the CC1 IRQ.
 *   - (Maybe: use CC2 to freeze the pin until blanking period is over.)
 *   - If CC1 t1>blanking period -> get timing echo.
 *   - Wait for TOP interrupt.
 *
 *
 *
 * @note
 *    This handler doesn't actually dynamically change the duty cycle. Instead,
 *    it acts as a template for doing so. Simply change the dutyCyclePercent
 *    global variable here to dynamically change the duty cycle.
 *****************************************************************************/
void WTIMER2_IRQHandler(void)
{

  uint64_t tick;
  mod_som_timer_handle_ptr_t local_tx_timer_ptr= \
                                mod_som_altimeter_ptr->config_ptr->tx_timer_ptr;

  TIMER_IntDisable(local_tx_timer_ptr->timer, TIMER_IEN_CC0|TIMER_IEN_CC1|TIMER_IEN_OF);

  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(mod_som_altimeter_ptr->config_ptr->tx_timer_ptr->timer);
  TIMER_IntClear(mod_som_altimeter_ptr->config_ptr->tx_timer_ptr->timer, flags);

  switch (mod_som_altimeter_ptr->altimeter_state){
    case pulse:
      //ALB Disable thr CMOA action. Set to
      local_tx_timer_ptr->timer->CC[MOD_SOM_ALTIMETER_TX_CC_CHANNEL].CTRL =0x2; // CCO_CTRL 0x402: Output compare, CMOA none, COFOA toggle.
      TIMER_CompareSet(local_tx_timer_ptr->timer,0,\
                       mod_som_altimeter_ptr->settings_ptr->blanking_interval+\
                       mod_som_altimeter_ptr->settings_ptr->tx_pulse_width); // set the blanking period into the compare register
      mod_som_altimeter_ptr->altimeter_state=blanking;
      TIMER_IntEnable(local_tx_timer_ptr->timer, TIMER_IEN_CC0);
      break;
    case blanking:

      //Enabling both CC1 and CC0. CC1 to capture the echo, CC0 in case there is no echo.
      // I set up CC0  a 100 counts before OF.
      TIMER_CompareSet(local_tx_timer_ptr->timer,0,\
                             mod_som_altimeter_ptr->settings_ptr->tx_repetition_period-\
                             100); // set the blanking period into the compare register

      mod_som_altimeter_ptr->altimeter_state=echo;
      TIMER_IntEnable(local_tx_timer_ptr->timer, TIMER_IEN_CC1|TIMER_IEN_CC0);
      break;
    case echo:

      if (flags&TIMER_IF_CC1){
          mod_som_altimeter_ptr->t_echo=
              local_tx_timer_ptr->timer->CC[MOD_SOM_ALTIMETER_ECHO_CC_CHANNEL].CCVB;
      }
      if (flags&TIMER_IF_CC0){
          mod_som_altimeter_ptr->t_echo=
              local_tx_timer_ptr->timer->CNT;
      }
      //grab time stamp
      tick=sl_sleeptimer_get_tick_count64();
      mystatus = sl_sleeptimer_tick64_to_ms(tick,\
             &mod_som_altimeter_ptr->timestamp);

      //MHA: Now augment timestamp by poweron_offset_ms
      mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
      mod_som_altimeter_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

      //      TIMER_CompareSet(local_tx_timer_ptr->timer,0,\
//                       mod_som_altimeter_ptr->config_ptr->tx_pulse_start); // set pulse length into the compare register

      mod_som_altimeter_ptr->data_state=ready;
      mod_som_altimeter_ptr->altimeter_state=wait;
      break;
    case wait:
      //change cc0 output action to toggle when CC0 is matched
      TIMER_CompareSet(local_tx_timer_ptr->timer,0,\
                       mod_som_altimeter_ptr->settings_ptr->tx_pulse_width); // set pulse length into the compare register
      mod_som_altimeter_ptr->t0=TIMER_CounterGet(local_tx_timer_ptr->timer);
      mod_som_altimeter_ptr->altimeter_state=pulse;
      TIMER_IntEnable(local_tx_timer_ptr->timer, TIMER_IEN_CC0);
      break;
    default:
      break;
  };
}
#endif


