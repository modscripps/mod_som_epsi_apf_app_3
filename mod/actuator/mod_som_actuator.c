/*
 * mod_som_actuator.c
 *
 *  Created on: January 4th 2021
 */
#include <actuator/mod_som_actuator.h>

#ifdef  MOD_SOM_ACTUATOR_EN

#include <actuator/mod_som_actuator_bsp.h>
#include "mod_som_io.h"

#include "mod_som_sdio.h"
#include "mod_som_priv.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <actuator/mod_som_actuator_cmd.h>
#endif

#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;

// MAIN module structure
sl_status_t mystatus;
mod_som_actuator_ptr_t mod_som_actuator_ptr;

/*******************************************************************************
 * @brief
 *   Initialize ACTUATOR, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_actuator_init_f(){
    mod_som_status_t status;
    RTOS_ERR  err;

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_actuator_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_actuator_encode_status_f(MOD_SOM_ACTUATOR_STATUS_FAIL_INIT_CMD);
#endif

    // ALB allocate memory for the module_ptr.
    // ALB The module_ptr is also the "scope" of the runtime_ptr
    // ALB but the module_ptr also contains the settings_ptr and the config_ptr
    // ALB The settings_ptr an config_ptr should allocated and defined during the module initialization
    mod_som_actuator_ptr = (mod_som_actuator_ptr_t)Mem_SegAlloc(
        "MOD SOM ACTUATOR RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_actuator_t),
        &err);

    //SN Check error code
    //ALB WARNING: The following line hangs in a while loop -
    //ALB WARNING: - if the previous allocation fails.
    //ALB TODO change return -1 to return the an appropriate error code.
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_actuator_ptr==DEF_NULL){
//      printf("ACTUATOR not initialized\n");
      return -1;
    }

    //ALB Initialize the runtime flag module_ptr->initialized_flag to false.
    //ALB It will be set to true once the module is initialized at the end of mod_som_actuator_init_f().
    mod_som_actuator_ptr->initialized_flag = false;

    // ALB allocate memory for the settings_ptr.
    // ALB WARNING: The setup pointer CAN NOT have pointers inside.
    status |= mod_som_actuator_allocate_settings_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("ACTUATOR not initialized\n");
      return status;
    }


    // ALB allocate memory for config ptr.
    status |= mod_som_actuator_allocate_config_f();
    if (status!=MOD_SOM_STATUS_OK){
        printf("ACTUATOR not initialized\n");
        return status;
    }

    mod_som_actuator_default_settings_f(mod_som_actuator_ptr->settings_ptr);
    // ALB initialize config ptr.
    mod_som_actuator_config_f(mod_som_actuator_ptr->config_ptr);



    mod_som_actuator_ptr->actuator_position=allin;
    mod_som_actuator_ptr->ctc_extent=MOD_SOM_ACTUATOR_CTC_EXTENT;
    mod_som_actuator_ptr->ptc_extent=MOD_SOM_ACTUATOR_PTC_EXTENT;
    mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;
    mod_som_actuator_ptr->percent_extent=0;
    mod_som_actuator_ptr->nb_pulse=0;
    mod_som_actuator_ptr->error_flag=status;
    mod_som_actuator_ptr->timestamp=0;
    mod_som_actuator_ptr->mode=0;

    // ALB initialize actuator timers.
    mod_som_actuator_init_timer_f();




    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);

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
mod_som_status_t mod_som_actuator_allocate_config_f(){
//    mod_som_status_t status;
    RTOS_ERR  err;

    //ALB alloc memory for setup pointer
    //set up default configuration
    mod_som_actuator_ptr->config_ptr =
        (mod_som_actuator_config_ptr_t)Mem_SegAlloc(
            "MOD SOM ACTUATOR config.",DEF_NULL,
            sizeof(mod_som_actuator_config_t),
            &err);

    mod_som_actuator_ptr->config_ptr->timer_ptr =
        (mod_som_timer_handle_ptr_t)Mem_SegAlloc(
            "MOD SOM ACTUATOR timer handle.",DEF_NULL,
            sizeof(mod_som_timer_handle_t),
            &err);

    mod_som_actuator_ptr->config_ptr->timer_ptr->timer =
        (TIMER_TypeDef*)Mem_SegAlloc(
            "MOD SOM ACTUATOR timer.",DEF_NULL,
            sizeof(TIMER_TypeDef),
            &err);


    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_actuator_ptr->config_ptr==NULL)
    {
      mod_som_actuator_ptr = DEF_NULL;
      return MOD_SOM_ACTUATOR_CANNOT_ALLOCATE_CONFIG;
    }
    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *   uint32_t initialized_flag;
 *   mod_som_timer_handle_ptr_t timer_ptr;
 *   mod_som_timer_handle_ptr_t echo_timer;
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_actuator_config_f(mod_som_actuator_config_ptr_t config_ptr){

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  config_ptr->initialized_flag = false;
  config_ptr->timer_ptr->timer = (void *)MOD_SOM_ACTUATOR1_TIMER;
  config_ptr->timer_ptr->compare_value  = MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;
  config_ptr->timer_ptr->route_location = MOD_SOM_ACTUATOR1_ROUTE;
  config_ptr->timer_ptr->timer_pin.pin  = MOD_SOM_ACTUATOR1_CC_PIN;
  config_ptr->timer_ptr->timer_pin.port = MOD_SOM_ACTUATOR1_CC_PORT;
  config_ptr->timer_ptr->timer_clock    = MOD_SOM_ACTUATOR1_CLK;
  config_ptr->timer_ptr->top            = MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE;

  return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);

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
 *   bool initialize_flag;
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_actuator_allocate_settings_f(){

  RTOS_ERR  err;

  //ALB alloc memory for setup pointer
  //set up default configuration
  mod_som_actuator_ptr->settings_ptr =
      (mod_som_actuator_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM ACTUATOR setup.",DEF_NULL,
          sizeof(mod_som_actuator_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_actuator_ptr->settings_ptr==NULL)
  {
    mod_som_actuator_ptr = DEF_NULL;
    return MOD_SOM_ACTUATOR_CANNOT_ALLOCATE_SETUP;
  }


  return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Initialize setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_actuator_default_settings_f(mod_som_actuator_settings_ptr_t settings_ptr){

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.


  //initialize settings fields
  strcpy(settings_ptr->header,MOD_SOM_ACTUATOR_HEADER);
  strcpy(settings_ptr->rev,MOD_SOM_ACTUATOR_DEFAULT_REV);
  strcpy(settings_ptr->sn,MOD_SOM_ACTUATOR_DEFAULT_SN);
  settings_ptr->initialize_flag=true;
  settings_ptr->size=sizeof(*settings_ptr);

  return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_actuator_settings_t mod_som_actuator_get_settings_f(){
  return *mod_som_actuator_ptr->settings_ptr;
}


/***************************************************************************//**
 * @brief
 *   mod_som_actuator_init_f
 *   - initialize actuator timer
 *
 *
 *
 *   Altimeter timer sequence
 *   - initialize the timer
 *          timerPrescale1 (highest resolution),
 *   - TOP is the periodicity of the actuator sequence.
 *   - Output compare CC0 toggle up the pin (t0) -> Change CC0 inside IRQ and enable input compare CC1 -> CC0_IRQ toggle down pin.
 *   - Input capture CC1 capture t0 -> from there cc1 IRQ will check if events t1 are beyond blanking period in the CC1 IRQ.
 *   - (Maybe: use CC2 to freeze the pin until blanking period is over.)
 *   - If CC1 t1>blanking period -> get timing echo.
 *   - Wait for TOP interrupt.
 *
 *   -
 ******************************************************************************/
mod_som_status_t mod_som_actuator_init_timer_f()
{

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  //initialize TX timer
  //TODO change the following code to initialize the echo timer
  CMU_Clock_TypeDef actuator_mclock_clk = MOD_SOM_ACTUATOR1_CLK;

  // DEFINE local tx timer handle just to save time on syntax
  mod_som_timer_handle_ptr_t local_timer_ptr= \
                                mod_som_actuator_ptr->config_ptr->timer_ptr;


  /* enable TX GPIO pin  - using wide timer 0 */
  GPIO_PinModeSet(local_timer_ptr->timer_pin.port, \
                  local_timer_ptr->timer_pin.pin,\
                  gpioModePushPull, 0); //TODO add to bsp

  CMU_ClockEnable(actuator_mclock_clk, true); //TODO add to bsp


  // Configure WTIMER3 cc1 Compare/Capture for output compare
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.cmoa  = timerOutputActionNone;
  timerCCInit.mode  = timerCCModeCompare;
  TIMER_InitCC(local_timer_ptr->timer, MOD_SOM_ACTUATOR1_CC_CHANNEL, &timerCCInit);


  // Route WTIMER3 CC0 to location 0 and enable CC0 route pin
  // TIM1_CC0 #0 is GPIO Pin PC13
  local_timer_ptr->timer->ROUTELOC0 |= TIMER_ROUTELOC0_CC2LOC_LOC3;
  local_timer_ptr->timer->ROUTEPEN  |= TIMER_ROUTEPEN_CC2PEN;

  // Set top value to overflow at the desired PWM_FREQ frequency
  TIMER_TopSet(local_timer_ptr->timer, \
               mod_som_actuator_ptr->config_ptr->timer_ptr->top); // TOP 25e6 -> 0.5 sec



  GPIO_PinModeSet(MOD_SOM_ACTUATOR1_EN_PORT,MOD_SOM_ACTUATOR1_EN_PIN, gpioModePushPull, 0);   // power the ACTU

  // ALB make sure we are all in 0% extension
  mod_som_actuator_ctc_f();

  // Enable TIMER1 compare event interrupts to update the duty cycle
  mod_som_actuator_ptr->actuator_position=allin;
  mod_som_actuator_ptr->actuator_state=stop;
  NVIC_EnableIRQ(WTIMER3_IRQn);

  uint32_t flags = TIMER_IntGet(mod_som_actuator_ptr->config_ptr->timer_ptr->timer);
  TIMER_IntClear(mod_som_actuator_ptr->config_ptr->timer_ptr->timer, flags);

  //  // Initialize and start the timer
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    TIMER_Init(local_timer_ptr->timer, &timerInit);


  return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);

}





/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_hello_world_f(){
    mod_som_io_print_f("[actuator]: hello world\r\n");
    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a function to toggle the actuator
 *
 *   ALB
 *   I am using 2 state machines. 1 for the actuator position
 *   and 1 for the actuator action.
 *   The positions are allin, allout or interm.
 *   The actions are stop and go.
 *
 *   When toggling I only go use allin to allout.
 *   I have an edge case where the actuator is in interm position.
 *   If this is the case I go to allin if actuator is toggled
 *
 *   Each of the toggle is enable a sequence using CC:
 *   - Activate Compare-match toggle on CC pin
 *   - set CCO compare value to 0 (so it toggles the pin when timer->CNT=0)
 *   - Enable the CC interrupt
 *   - Set timer->CNT=0 (which should trigger the CC interrupt right away)
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_toggle_f(){
    mod_som_io_print_f("[actuator]: toggle actuator\r\n");
    mod_som_timer_handle_ptr_t local_timer_ptr= \
                                  mod_som_actuator_ptr->config_ptr->timer_ptr;

    switch(mod_som_actuator_ptr->actuator_position){
      case allin:
        //going allout
        mod_som_actuator_ptr->percent_extent=100;
//        mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT( \
//                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE
        mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE; //MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE
        mod_som_actuator_ptr->actuator_position=allout;
        break;
      case allout:
        //going allin
        mod_som_actuator_ptr->percent_extent=0;
        mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT( \
                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;
        mod_som_actuator_ptr->actuator_position=allin;
        break;
      case interm:
        //going allin
        mod_som_actuator_ptr->percent_extent=0;
        mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT( \
                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;
        mod_som_actuator_ptr->actuator_position=allin;
        break;
    }
    //ALB Enable th3 CMOA action. Set to
    local_timer_ptr->timer->CC[MOD_SOM_ACTUATOR1_CC_CHANNEL].CTRL =0x102; // CCO_CTRL 0x102: Output compare, CMOA toggle.
    TIMER_CompareSet(local_timer_ptr->timer,MOD_SOM_ACTUATOR1_CC_CHANNEL,0); // the set CC0 to toggle the pin when CNT=0.
    // Acknowledge the interrupt
    uint32_t flags = TIMER_IntGet(local_timer_ptr->timer);
    TIMER_IntClear(local_timer_ptr->timer, flags);
    TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2); // enable CC0 interrupt.
    TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.


    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   a function to toggle the actuator
 *
 *   ALB
 *   I am using 2 state machines. 1 for the actuator position
 *   and 1 for the actuator action.
 *   The positions are allin, allout or interm.
 *   The actions are stop and go.
 *
 *   When toggling I only go use allin to allout.
 *   I have an edge case where the actuator is in interm position.
 *   If this is the case I go to allin if actuator is toggled
 *
 *   Each of the toggle is enable a sequence using CC:
 *   - Activate Compare-match toggle on CC pin
 *   - set CCO compare value to 0 (so it toggles the pin when timer->CNT=0)
 *   - Enable the CC interrupt
 *   - Set timer->CNT=0 (which should trigger the CC interrupt right away)
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_ptc_f(){
    mod_som_timer_handle_ptr_t local_timer_ptr= \
                                  mod_som_actuator_ptr->config_ptr->timer_ptr;

        //going allout
        mod_som_actuator_ptr->percent_extent=mod_som_actuator_ptr->ptc_extent;
        mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT(
                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE; //MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE
        mod_som_actuator_ptr->actuator_position=allout;

        //ALB Enable th3 CMOA action. Set to
        local_timer_ptr->timer->CC[MOD_SOM_ACTUATOR1_CC_CHANNEL].CTRL =0x102; // CCO_CTRL 0x102: Output compare, CMOA toggle.
        TIMER_CompareSet(local_timer_ptr->timer,MOD_SOM_ACTUATOR1_CC_CHANNEL,0); // the set CC0 to toggle the pin when CNT=0.
        // Acknowledge the interrupt
        uint32_t flags = TIMER_IntGet(local_timer_ptr->timer);
        TIMER_IntClear(local_timer_ptr->timer, flags);
        TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2); // enable CC0 interrupt.
        TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.


    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   a function to toggle the actuator
 *
 *   ALB
 *   I am using 2 state machines. 1 for the actuator position
 *   and 1 for the actuator action.
 *   The positions are allin, allout or interm.
 *   The actions are stop and go.
 *
 *   When toggling I only go use allin to allout.
 *   I have an edge case where the actuator is in interm position.
 *   If this is the case I go to allin if actuator is toggled
 *
 *   Each of the toggle is enable a sequence using CC:
 *   - Activate Compare-match toggle on CC pin
 *   - set CCO compare value to 0 (so it toggles the pin when timer->CNT=0)
 *   - Enable the CC interrupt
 *   - Set timer->CNT=0 (which should trigger the CC interrupt right away)
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_ctc_f(){
  mod_som_timer_handle_ptr_t local_timer_ptr= \
      mod_som_actuator_ptr->config_ptr->timer_ptr;

  //going allin
  mod_som_actuator_ptr->percent_extent=mod_som_actuator_ptr->ctc_extent;
  mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT(
                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;
  mod_som_actuator_ptr->actuator_position=allin;
  //ALB Enable th3 CMOA action. Set to
  local_timer_ptr->timer->CC[MOD_SOM_ACTUATOR1_CC_CHANNEL].CTRL =0x102; // CCO_CTRL 0x102: Output compare, CMOA toggle.
  TIMER_CompareSet(local_timer_ptr->timer,MOD_SOM_ACTUATOR1_CC_CHANNEL,0); // the set CC0 to toggle the pin when CNT=0.
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(local_timer_ptr->timer);
  TIMER_IntClear(local_timer_ptr->timer, flags);
  TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2); // enable CC0 interrupt.
  TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.


    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   a function to position the actuator at a specific value
 *
 *    *   ALB
 *   I am using 2 state machines. 1 for the actuator position
 *   and 1 for the actuator action.
 *   The positions are allin, allout or interm.
 *   The actions are stop and go.
 *   When using actuator_set the position is set to interim
 *
 *   - I grab the percent_extent from the user
 *   - convert it in number of CNT for a CC value in the action-go state
 *   - Activate Compare-match toggle on CC pin
 *   - set CC value to 0 (so it toggles the pin when timer->CNT=0)
 *   - Enable the CC interrupt
 *   - Set timer->CNT=0 (which should trigger the CC interrupt right away)
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_set_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  mod_som_timer_handle_ptr_t local_timer_ptr= \
                                mod_som_actuator_ptr->config_ptr->timer_ptr;

    if (argc==1){
      printf("ACT extent %lu.\r\n",mod_som_actuator_ptr->percent_extent);
    }
    else{
      //ALB switch statement easy to handle all user input cases.
      switch (argc){
      case 2:
        mod_som_actuator_ptr->percent_extent=atoi(argv[1]);
        break;
      case 3:
        if (strcmp(argv[1],"ctc")==0){
            mod_som_actuator_ptr->ctc_extent=atoi(argv[2]);
        }
        if (strcmp(argv[1],"ptc")==0){
            mod_som_actuator_ptr->ptc_extent=atoi(argv[2]);
        }
        printf("act.set ctc:%lu and ptc:%lu.\r\n",
               mod_som_actuator_ptr->ctc_extent,
               mod_som_actuator_ptr->ptc_extent);
        break;
      default:
        printf("format: act.set percent_extent [int 0-100]\r\n");
        printf("format: act.set ctc percent_extent (set ctc or ptc extent)\r\n");
        break;
      }
    }

    mod_som_actuator_ptr->actuator_position=interm;
    mod_som_actuator_ptr->extent=MOD_SOM_ACTUATOR_CALC_EXTENT( \
                                          mod_som_actuator_ptr->percent_extent); //MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE;

//    GPIO_PinModeSet(MOD_SOM_ACTUATOR1_EN_PORT,MOD_SOM_ACTUATOR1_EN_PIN, gpioModePushPull, 1);   // power the ACTU

    //ALB Enable th3 CMOA action. Set to
    local_timer_ptr->timer->CC[MOD_SOM_ACTUATOR1_CC_CHANNEL].CTRL =0x102; // CCO_CTRL 0x102: Output compare, CMOA toggle.
    TIMER_CompareSet(local_timer_ptr->timer,MOD_SOM_ACTUATOR1_CC_CHANNEL,0); // the set CC1 to toggle the pin when CNT=0.
    TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2); // enable CC0 interrupt.
    TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.

    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}

/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_actuator_message_f()
{

  //time stamp
  uint32_t t_hex[2];
  char * curr_msg_ptr;
  curr_msg_ptr=(char*) mod_som_actuator_ptr->msg;


  t_hex[0] = (uint32_t) (mod_som_actuator_ptr->timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_actuator_ptr->timestamp;




  //ALB ACTU header + hex timestamp. It will be used in the IRQ handler to send
  //ALB 1 char ($) + 4 char tag + 16 char hex timestamp
  //ALB 8 char payload + 1 char (*) + 2 char chksum

  // a message to the IO stream or store task
  sprintf((char *) mod_som_actuator_ptr->msg,  \
      "$%s%08x%08x%08lu*FF\r\n", \
      mod_som_actuator_ptr->settings_ptr->header, \
      (int) t_hex[0],\
      (int) t_hex[1],
      mod_som_actuator_ptr->percent_extent);

  mod_som_actuator_ptr->chksum=0;
  for(int i=0;i<strlen(mod_som_actuator_ptr->msg)-MOD_SOM_ACTUATOR_CHCKSUM_LENGTH;i++)
    {
    mod_som_actuator_ptr->chksum ^=\
        *(curr_msg_ptr++);
    }

  //ALB move the pointer after the *
  curr_msg_ptr++;
  //ALB write the hex chcksum at the right place.
  uint16_t local_chksum = \
      mod_som_int8_2hex_f(mod_som_actuator_ptr->chksum);

  *(curr_msg_ptr++) = local_chksum>>8;
  *(curr_msg_ptr++) = local_chksum;


  if (mod_som_actuator_ptr->mode==0){
      mod_som_io_stream_data_f((uint8_t *) &mod_som_actuator_ptr->msg, \
                               strlen(mod_som_actuator_ptr->msg),   \
                               DEF_NULL);

  }else{
      mod_som_sdio_write_data_f((uint8_t *) &mod_som_actuator_ptr->msg, \
                                   strlen(mod_som_actuator_ptr->msg),   \
                                   DEF_NULL);

  }
}



mod_som_status_t mod_som_actuator_mode_f(CPU_INT16U argc,CPU_CHAR *argv[])
{
  RTOS_ERR  p_err;

  if (argc==1){
      printf("actu.mode %u\r\n.", mod_som_actuator_ptr->mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_actuator_ptr->mode=shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: actu.mode mode (0:stream, 1:SD store)\r\n");
      break;
    }
  }
  return MOD_SOM_STATUS_OK;
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
uint8_t mod_som_actuator_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_ACTUATOR_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_actuator_encode_status_f
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
mod_som_status_t mod_som_actuator_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_ACTUATOR_STATUS_PREFIX, mod_som_io_status);
}


/**************************************************************************//**
 * @brief
 *    Interrupt handler for WTIMER3 that changes the duty cycle
 *
 *
 *     *   Actuatorltimeter timer sequence
 *     ALB
 *     I only use CC2
 *
 *   When actuator is toggled or set, the "calling" function (toggle or set)
 *   set the mod_som_actuator_ptr->extent value. It is used here in the IR handler
 *   a compare/match value.
 *
 *   Action sate should be stop when entering here the "first time" (i.e CCO on CNT=0).
 *   - Set CC compare to mod_som_actuator_ptr->extent.
 *   - Enable CC interrupt
 *   - change action state to: go
 *
 *   Once CNT reaches mod_som_actuator_ptr->extent (end of actuator motion)
 *   - Disable CC Compare toggle
 *   - grab the time stamp
 *   - write or stream actuator message
 *   - change action state to: stop
 *
 *
 *
 * @note
 *    This handler doesn't actually dynamically change the duty cycle. Instead,
 *    it acts as a template for doing so. Simply change the dutyCyclePercent
 *    global variable here to dynamically change the duty cycle.
 *****************************************************************************/
void WTIMER3_IRQHandler(void)
{
  uint64_t tick;

  mod_som_timer_handle_ptr_t local_timer_ptr= \
                                mod_som_actuator_ptr->config_ptr->timer_ptr;

  TIMER_IntDisable(local_timer_ptr->timer, TIMER_IEN_CC2);

  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(local_timer_ptr->timer);
  TIMER_IntClear(local_timer_ptr->timer, flags);

  switch (mod_som_actuator_ptr->actuator_state){
    case stop:
      //ALB Disable thr CMOA action. Set to
      TIMER_CompareSet(local_timer_ptr->timer,MOD_SOM_ACTUATOR1_CC_CHANNEL, \
                                               mod_som_actuator_ptr->extent);
      TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2);
      TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.
      mod_som_actuator_ptr->actuator_state=cycle;
      mod_som_actuator_ptr->nb_pulse=0;
      break;
    case cycle:
      mod_som_actuator_ptr->nb_pulse++;
      if (mod_som_actuator_ptr->nb_pulse>MOD_SOM_ACTUATOR_MAX_NB_PULSE){
          mod_som_actuator_ptr->actuator_state=end_cycle;
      }
      TIMER_CounterSet(local_timer_ptr->timer,0);  // set CNT =0.
      TIMER_IntEnable(local_timer_ptr->timer, TIMER_IEN_CC2);
      break;
    case end_cycle:
      local_timer_ptr->timer->CC[MOD_SOM_ACTUATOR1_CC_CHANNEL].CTRL =0x2; // CC_CTRL 0x2: Output compare, CMOA none.
      // grab time stamp
      tick=sl_sleeptimer_get_tick_count64();
      mystatus = sl_sleeptimer_tick64_to_ms(tick,\
             &mod_som_actuator_ptr->timestamp);

      //MHA: Now augment timestamp by poweron_offset_ms
      mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
      mod_som_actuator_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

      //send msg
      mod_som_actuator_message_f();

//      GPIO_PinModeSet(MOD_SOM_ACTUATOR1_EN_PORT,MOD_SOM_ACTUATOR1_EN_PIN, gpioModePushPull, 0);   // power the ALTI
      //TODO write or stream actuator message (ACT timestamp percent_extent)
      mod_som_actuator_ptr->actuator_state=stop;
      break;
    default:
      break;
  };
}
#endif




