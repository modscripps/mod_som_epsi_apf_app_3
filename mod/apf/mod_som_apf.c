/*
 * mod_som_apf.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


#include <apf/mod_som_apf.h>
#include <apf/mod_som_apf_bsp.h>

#ifdef MOD_SOM_APF_EN

#include "mod_som_io.h"
#include "math.h"


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <apf/mod_som_apf_cmd.h>

#endif

#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif


#include <efe_obp/mod_som_efe_obp.h>



mod_som_apf_ptr_t mod_som_apf_ptr;

// producer task
static CPU_STK mod_som_apf_producer_task_stk[MOD_SOM_APF_PRODUCER_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_producer_task_tcb;

// consumer task
static CPU_STK mod_som_apf_consumer_task_stk[MOD_SOM_APF_CONSUMER_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_consumer_task_tcb;

// apf shell task
static CPU_STK mod_som_apf_shell_task_stk[MOD_SOM_APF_PRODUCER_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_shell_task_tcb;


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

    //ALB gittest#2

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

    //ALB Allocate memory for the producer pointer,
    //ALB using the settings_ptr variable
    status |= mod_som_apf_construct_producer_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
        printf("APF not initialized\n");
        return status;
    }
// enable the reading and parsing the input command/string through the LEUART - Arnaud&Mai - Nov 10, 2021
//    //ALB Allocate memory for the producer pointer,
//    //ALB using the settings_ptr variable
    status |= mod_som_apf_construct_com_prf_f();
    if (status!=MOD_SOM_STATUS_OK){
        printf("APF not initialized\n");
        return status;
    }



    //ALB initialize mod_som_apf_ptr params
    mod_som_apf_ptr->profile_id=0;
    mod_som_apf_ptr->daq=false;
    mod_som_apf_ptr->dacq_start_pressure=0;
    mod_som_apf_ptr->dacq_pressure=0;
    mod_som_apf_ptr->dacq_dz=0;


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
    return MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_SETUP;
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

  settings_ptr->comm_telemetry_packet_format=F1;
  settings_ptr->sd_packet_format=SD0;

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
 *   get the daq's value
 *
 * @param
 *   return daq's value
 ******************************************************************************/
bool mod_som_apf_get_daq_f(){
  return mod_som_apf_ptr->daq;
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
void* mod_som_apf_get_port_ptr_f(){
  return mod_som_apf_ptr->com_prf_ptr->handle_port;
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
    return MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_CONFIG;
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

  config_ptr->vibration_cut_off=MOD_SOM_APF_VIBRATION_CUT_OFF;

  config_ptr->port.com_port   = (void *)MOD_SOM_APF_USART;
  config_ptr->port.route      = MOD_SOM_APF_TX_LOC;
  config_ptr->port.tx_port    = MOD_SOM_APF_TX_PORT;
  config_ptr->port.tx_pin     = MOD_SOM_APF_TX_PIN;

  config_ptr->port.rx_port    = MOD_SOM_APF_RX_PORT;
  config_ptr->port.rx_pin     = MOD_SOM_APF_RX_PIN;

  config_ptr->port.en_port    = MOD_SOM_APF_EN_PORT;
  config_ptr->port.en_pin     = MOD_SOM_APF_EN_PIN;

  config_ptr->port.parity    = mod_som_uart_parity_none;
  config_ptr->port.data_bits = mod_som_uart_data_bits_8;
  config_ptr->port.stop_bits = mod_som_uart_stop_bits_1;

  config_ptr->baud_rate      = MOD_SOM_APF_RX_DEFAULT_BAUD_RATE;


  config_ptr->initialized_flag = true;
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct producer_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_producer_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for config pointer
  mod_som_apf_ptr->producer_ptr =
      (mod_som_apf_producer_ptr_t)Mem_SegAlloc(
          "MOD SOM APF producer.",DEF_NULL,
          sizeof(mod_som_apf_producer_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_apf_ptr->config_ptr==NULL)
  {
    mod_som_apf_ptr = DEF_NULL;
    return MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_PRODUCER;
  }

//ALB I need efe_obp_ptr to the nfft so I can compute nfft_diag;
  mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr=
                                          mod_som_efe_obp_get_runtime_ptr_f();



  mod_som_apf_ptr->producer_ptr->initialized_flag     = false;
  mod_som_apf_ptr->producer_ptr->started_flg          = false;
  mod_som_apf_ptr->producer_ptr->collect_flg          = false;
  mod_som_apf_ptr->producer_ptr->dacq_full            = false;
  mod_som_apf_ptr->producer_ptr->dissrates_cnt        = 0;
  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt = 0;
  mod_som_apf_ptr->producer_ptr->nfft_diag=
               mod_som_efe_obp_ptr->settings_ptr->nfft/
               MOD_SOM_APF_DACQ_F3_NFFT_DECIM_COEF;

  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dacq_size    =
                           mod_som_apf_ptr->producer_ptr->dacq_ptr-
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];


  mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit =
                                       MOD_SOM_APF_PRODUCER_DISSRATE_RANGE/
                                       (log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE)
                                       -log10(MOD_SOM_APF_PRODUCER_MIN_DISSRATE));
                                       ;
  mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin =
      - (mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit*
        log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE)) +
        MOD_SOM_APF_PRODUCER_DISSRATE_RANGE;

  mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit =
                                       MOD_SOM_APF_PRODUCER_FOM_RANGE/
                                       (MOD_SOM_APF_PRODUCER_MAX_FOM
                                       -MOD_SOM_APF_PRODUCER_MIN_FOM);
                                       ;
  mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin =
      - (mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit*
        MOD_SOM_APF_PRODUCER_MAX_FOM) +
        MOD_SOM_APF_PRODUCER_FOM_RANGE;

  mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit =
                                       MOD_SOM_APF_PRODUCER_FOCO_RANGE/
                                       (log10(MOD_SOM_APF_PRODUCER_MAX_FOCO)
                                       -log10(MOD_SOM_APF_PRODUCER_MIN_FOCO));
                                       ;
  mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin =
      - (mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit*
        log10(MOD_SOM_APF_PRODUCER_MAX_FOCO)) +
        MOD_SOM_APF_PRODUCER_FOCO_RANGE;



  mod_som_apf_ptr->producer_ptr->initialized_flag = true;
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct consumer_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_consumer_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for config pointer
  mod_som_apf_ptr->consumer_ptr =
      (mod_som_apf_consumer_ptr_t)Mem_SegAlloc(
          "MOD SOM APF consumer.",DEF_NULL,
          sizeof(mod_som_apf_consumer_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_apf_ptr->config_ptr==NULL)
  {
    mod_som_apf_ptr = DEF_NULL;
    return MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_PRODUCER;
  }

  mod_som_apf_ptr->consumer_ptr->initialized_flag = false;
  mod_som_apf_ptr->consumer_ptr->dacq_size=0;
  mod_som_apf_ptr->consumer_ptr->stored_dissrates_cnt = 0;

  mod_som_apf_ptr->consumer_ptr->dacq_ptr =
      (uint8_t*)Mem_SegAlloc(
          "MOD SOM APF consumer dacq.",DEF_NULL,
          sizeof(uint8_t)*MOD_SOM_EFE_OBP_DEFAULT_NFFT,
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_apf_ptr->consumer_ptr->initialized_flag = true;
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct apf com_prf
 *   ALB We should be able to choose between RETARGET_SERIAL and any other port
 *   (e.g., LEUART)
 *
 * @param mod_som_apf_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_com_prf_f(){

  RTOS_ERR  err;

  //TODO I am hardcoding the leuart port but MN should find a solution to be
  //TODO not port dependent.
  LEUART_TypeDef          *leuart_ptr;
  LEUART_Init_TypeDef     leuart_init = LEUART_INIT_DEFAULT;

  //device communication peripheral pointer
  mod_som_apf_ptr->com_prf_ptr =
       (mod_som_apf_prf_ptr_t)Mem_SegAlloc(
           "MOD SOM apf communication prf",DEF_NULL,
           sizeof(mod_som_apf_ptr),
           &err);
   // Check error code
   APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  mod_som_apf_ptr->com_prf_ptr->portID=apfleuart;

  switch (mod_som_apf_ptr->com_prf_ptr->portID){
    case apfleuart:


      #if defined(_CMU_HFPERCLKEN0_MASK)
        CMU_ClockEnable(cmuClock_HFPER, true);
      #endif
        CMU_ClockEnable(cmuClock_GPIO, true);

        /* Enable CORE LE clock in order to access LE modules */
        CMU_ClockEnable(cmuClock_HFLE, true);
        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_1);
        CMU_ClockEnable(MOD_SOM_APF_USART_CLK, true);    /* Enable device clock */

        //device communication peripheral pointer
        mod_som_apf_ptr->com_prf_ptr =
            (mod_som_apf_prf_ptr_t)Mem_SegAlloc(
                "MOD SOM APF communication prf",DEF_NULL,
                sizeof(mod_som_apf_prf_t),
                &err);
        // Check error code
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
        if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
          return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_ALLOCATE_MEMORY));
        if(mod_som_apf_ptr->com_prf_ptr==DEF_NULL){
            mod_som_apf_ptr = DEF_NULL;
            return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_ALLOCATE_MEMORY));
        }

        // define com port in the com struct.
        // com_port is the same with handle_port: it should be named the same - Arnaud Nov 11, 2021
        mod_som_apf_ptr->com_prf_ptr->handle_port = \
                                    mod_som_apf_ptr->config_ptr->port.com_port;

        // making sure we have UART types defined
        // ALB I remove all the UART cases a kept only LEUART0 to match the SOM
        // ALB if needed it one can add another com port here
//        mod_som_apf_ptr->com_prf_ptr->irqn = MOD_SOM_APF_USART_IRQ;

        leuart_ptr  = \
            (LEUART_TypeDef *) mod_som_apf_ptr->com_prf_ptr->handle_port;
        leuart_init.baudrate   = mod_som_apf_ptr->config_ptr->baud_rate;

        //parity set
        //ALB the switch statements are a legacy from the previous APF module
        switch(mod_som_apf_ptr->config_ptr->port.parity){
          case mod_som_uart_parity_none:
            leuart_init.parity = leuartNoParity;
            break;
          case mod_som_uart_parity_even:
            leuart_init.parity = leuartEvenParity;
            break;
          case mod_som_uart_parity_odd:
            leuart_init.parity = leuartOddParity;
            break;
        }
        //data bits
        switch(mod_som_apf_ptr->config_ptr->port.data_bits){
          case mod_som_uart_data_bits_8:
            leuart_init.databits = leuartDatabits8;
            break;
          case mod_som_uart_data_bits_9:
            leuart_init.databits = leuartDatabits9;
            break;
        }
        //stop bits
        switch(mod_som_apf_ptr->config_ptr->port.stop_bits){
          case mod_som_uart_stop_bits_1:
            leuart_init.stopbits = leuartStopbits1;
            break;
          case mod_som_uart_stop_bits_2:
            leuart_init.stopbits = leuartStopbits2;
            break;
        }


        //reset leuart driver before initialization
        LEUART_Reset(leuart_ptr);
        leuart_init.enable = leuartDisable;
        LEUART_Init(leuart_ptr, &leuart_init);

        //ALB define the LEUART ROUTE.
        leuart_ptr->ROUTELOC0 = (leuart_ptr->ROUTELOC0
                                 & ~(_LEUART_ROUTELOC0_TXLOC_MASK
                                 | _LEUART_ROUTELOC0_RXLOC_MASK))
                                 | (mod_som_apf_ptr->config_ptr->port.route
                                 << _LEUART_ROUTELOC0_TXLOC_SHIFT)
                                 | (mod_som_apf_ptr->config_ptr->port.route
                                 << _LEUART_ROUTELOC0_RXLOC_SHIFT);

        //ALB enable the LEUART ROUTE.
        leuart_ptr->ROUTEPEN = LEUART_ROUTEPEN_TXPEN
                               | LEUART_ROUTEPEN_RXPEN;

//        //ALB This is important!!
//        //ALB It defines the last char of a SBE sample.
//        //ALB LEUART will scan it and trigger a SIGFRAME interrupt.
//        //ALB I use this interrupt to store the SBE data.
//        leuart_ptr->SIGFRAME = MOD_SOM_APF_SAMPLE_LASTCHAR;
//
//        //flush data from RX and TX.
//        leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
//
//        /* Clear previous RX interrupts. */
//        LEUART_IntClear(leuart_ptr, ~0x0);


      break;
      default:
        break;
  }

  //    /* SBE Enable: configure the LEUART pins and SBE EN (send power to the SBE)*/
  GPIO_PinModeSet(mod_som_apf_ptr->config_ptr->port.en_port, mod_som_apf_ptr->config_ptr->port.en_pin,
                  gpioModePushPull, 1);

  // configuration for RX & TX port, set "out"=0 - Arnaud - Nov 10, 2021
  GPIO_PinModeSet(mod_som_apf_ptr->config_ptr->port.rx_port, mod_som_apf_ptr->config_ptr->port.rx_pin,
                  gpioModeInput, 0);

  GPIO_PinModeSet(mod_som_apf_ptr->config_ptr->port.tx_port, mod_som_apf_ptr->config_ptr->port.tx_pin,
                  gpioModePushPull, 1);

  GPIO_PinModeSet(gpioPortC,0,gpioModePushPull, 1);

  LEUART_Enable((LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port, leuartEnable);


  //ALB start the shell tasks
  // Consumer Task 2
  //ALB initialize all parameters. They should be reset right before
  //ALB fill_segment task is starts running.

   OSTaskCreate(&mod_som_apf_shell_task_tcb,
                        "apf shell task",
                        mod_som_apf_shell_task_f,
                        DEF_NULL,
                        MOD_SOM_APF_SHELL_TASK_PRIO,
            &mod_som_apf_shell_task_stk[0],
            (MOD_SOM_APF_SHELL_TASK_STK_SIZE / 10u),
            MOD_SOM_APF_SHELL_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_START_PRODUCER_TASK));
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}





/*******************************************************************************
 * @brief
 *   start producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_start_producer_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
  //ALB initialize all parameters. They should be reset right before
  //ALB fill_segment task is starts running.
  mod_som_apf_ptr->producer_ptr->avg_timestamp        = 0;
  mod_som_apf_ptr->producer_ptr->dissrate_skipped     = 0;
  mod_som_apf_ptr->producer_ptr->dissrates_cnt        = 0;
  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt = 0;
  mod_som_apf_ptr->producer_ptr->dissrates_cnt        = 0;

  mod_som_apf_ptr->producer_ptr->initialized_flag     = false;
  mod_som_apf_ptr->producer_ptr->collect_flg          = false;
  mod_som_apf_ptr->producer_ptr->dacq_full            = false;

  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dacq_size    =
                           mod_som_apf_ptr->producer_ptr->dacq_ptr-
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

  mod_som_apf_ptr->producer_ptr->started_flg      = true;



   OSTaskCreate(&mod_som_apf_producer_task_tcb,
                        "apf producer task",
                        mod_som_apf_producer_task_f,
                        DEF_NULL,
                        MOD_SOM_APF_PRODUCER_TASK_PRIO,
            &mod_som_apf_producer_task_stk[0],
            (MOD_SOM_APF_PRODUCER_TASK_STK_SIZE / 10u),
            MOD_SOM_APF_PRODUCER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_START_PRODUCER_TASK));
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   stop producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_stop_producer_task_f(){


  RTOS_ERR err;
  OSTaskDel(&mod_som_apf_producer_task_tcb,
             &err);


  //ALB update the Meta Data sample cnt.
  mod_som_apf_ptr->producer_ptr->
  acq_profile.mod_som_apf_meta_data.sample_cnt=
      mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt;

  mod_som_apf_ptr->producer_ptr->started_flg=false;


  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_STOP_PRODUCER_TASK));

  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   start consumer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_start_consumer_task_f(){


  RTOS_ERR err;
  // Consumer Task

  mod_som_apf_ptr->consumer_ptr->dacq_size=0;
  mod_som_apf_ptr->consumer_ptr->stored_dissrates_cnt = 0;

  mod_som_apf_ptr->consumer_ptr->initialized_flag = true;


   OSTaskCreate(&mod_som_apf_consumer_task_tcb,
                        "apf consumer task",
                        mod_som_apf_consumer_task_f,
                        DEF_NULL,
                        MOD_SOM_APF_CONSUMER_TASK_PRIO,
            &mod_som_apf_consumer_task_stk[0],
            (MOD_SOM_APF_CONSUMER_TASK_STK_SIZE / 10u),
            MOD_SOM_APF_CONSUMER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   stop fill segment task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_stop_consumer_task_f(){


  RTOS_ERR err;
  OSTaskDel(&mod_som_apf_consumer_task_tcb,
             &err);

  mod_som_apf_ptr->consumer_ptr->started_flg=false;


  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_apf_ptr->status = mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_FAIL_TO_STOP_CONSUMER_TASK));

  return mod_som_apf_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   apf producer task
 *
 *   collect data and store them in the daq structure
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_producer_task_f(void  *p_arg){
  RTOS_ERR  err;

  (void)p_arg; // Deliberately unused argument



  int dissrate_avail=0, reset_dissrate_cnt=0;
  int dissrate_elmnts_offset=0;
  int avg_spectra_offset=0;

  uint64_t * curr_avg_timestamp_ptr;
  float    * curr_temp_avg_spectra_ptr;
  float    * curr_shear_avg_spectra_ptr;
  float    * curr_accel_avg_spectra_ptr;
  float    * curr_avg_pressure_ptr;
  float    * curr_avg_dpdt_ptr;
  float    * curr_epsilon_ptr;
  float    * curr_chi_ptr;
  float    * curr_epsi_fom_ptr;
  float    * curr_chi_fom_ptr;

  // parameters for send commands out to APF - mai - Nov 22, 2021
  uint32_t bytes_sent = 0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  uint32_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


  int padding = 1; // the padding should be big enough to include the time variance.


  mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr=
                                          mod_som_efe_obp_get_runtime_ptr_f();


  while (DEF_ON) {



      /************************************************************************/
      //ALB APF producer phase 1
      //ALB check if producer is started, and if the dacp_profile is NOT full
      if (mod_som_apf_ptr->producer_ptr->started_flg &
          !mod_som_apf_ptr->producer_ptr->dacq_full){

          dissrate_avail = mod_som_efe_obp_ptr->sample_count -
              mod_som_apf_ptr->producer_ptr->dissrates_cnt;  //calculate number of elements available have been produced

          //ALB User stopped efe. I need to reset the obp producers count
          if(dissrate_avail<0){
              mod_som_apf_ptr->producer_ptr->dissrates_cnt = 0;
          }
          // LOOP without delay until caught up to latest produced element
          while (dissrate_avail > 0)
            {
              // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
              // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
              // calculate the number of data we skipped, report number of elements skipped.
              // Reset the consumers cnt equal with producer data plus padding
              if (dissrate_avail>MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD){ // checking over flow. TODO check adding padding is correct.
                  // reset the consumer count less one buffer than producer count plus padding
                  //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                  reset_dissrate_cnt = mod_som_efe_obp_ptr->sample_count -
                      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD +
                      padding;
                  // calculate the number of skipped elements
                  mod_som_apf_ptr->producer_ptr->dissrate_skipped = reset_dissrate_cnt -
                      mod_som_apf_ptr->producer_ptr->dissrates_cnt;

                  mod_som_io_print_f("\n apf obp prod task: CB overflow: sample count = %lu,"
                      "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                      (uint32_t)mod_som_efe_obp_ptr->sample_count, \
                      (uint32_t)mod_som_apf_ptr->producer_ptr->dissrates_cnt, \
                      (uint32_t)mod_som_apf_ptr->producer_ptr->dissrate_skipped);

                  // save time string into the temporary local string - Mai - Nov 18, 2021
                  sprintf(apf_reply_str,"\n apf obp prod task: CB overflow: sample count = %lu,"
                          "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                          (uint32_t)mod_som_efe_obp_ptr->sample_count, \
                          (uint32_t)mod_som_apf_ptr->producer_ptr->dissrates_cnt, \
                          (uint32_t)mod_som_apf_ptr->producer_ptr->dissrate_skipped);
                  reply_str_len = strlen(apf_reply_str);
                  // sending the above string to the APF port - Mai - Nov 18, 2021
                  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
                  if (bytes_sent==0){
                      //TODO handle error
                  }

                  mod_som_apf_ptr->producer_ptr->dissrates_cnt = reset_dissrate_cnt;
              }

              //ALB calculate the offset for current pointer
              dissrate_elmnts_offset = mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                                       MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD;

              //ALB update avg pressure
              curr_avg_pressure_ptr =
                  &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure+
                  dissrate_elmnts_offset;

              //ALB check if the updated avg pressure is lower than
              //ALB the previous pressure + dz.
//              if (*curr_avg_pressure_ptr<
//                   mod_som_apf_ptr->dacq_pressure+mod_som_apf_ptr->dacq_dz)
//                {
              //ALB fake if loop to test the dacq
              if (dissrate_avail>0)
                {

                  mod_som_apf_ptr->dacq_pressure=*curr_avg_pressure_ptr;

                  avg_spectra_offset     = (mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                      MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)*
                      mod_som_efe_obp_ptr->settings_ptr->nfft/2;

                  //ALB update the current avg_spectra pointers
                  curr_temp_avg_spectra_ptr   =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr+
                      avg_spectra_offset;
                  curr_shear_avg_spectra_ptr   =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr+
                      avg_spectra_offset;
                  curr_accel_avg_spectra_ptr   =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr+
                      avg_spectra_offset;

                  //ALB update avg timestamp
                  curr_avg_timestamp_ptr =
                      (uint64_t *) (&mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp+
                         dissrate_elmnts_offset);

                  //ALB update avg pressure
                  curr_avg_dpdt_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt+
                      dissrate_elmnts_offset;

                  //ALB update the dissrate pointers
                  curr_epsilon_ptr =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon+
                                    dissrate_elmnts_offset;
                  curr_chi_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi+
                                    dissrate_elmnts_offset;
                  curr_epsi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom+
                                    dissrate_elmnts_offset;
                  curr_chi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom+
                                    dissrate_elmnts_offset;

                  switch (mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format){
                    case F0:
                      mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
                      break;
                    case F1:
                      //ALB convert and store the current dissrate into the MOD format
                      // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      mod_som_apf_copy_F1_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_epsi_fom_ptr,
                                                     curr_chi_fom_ptr,
                                                     mod_som_apf_ptr->producer_ptr->dacq_ptr
                                                 );
                      break;
                    case F2:
                      //ALB convert and store the current dissrate and FFT into the MOD format
                      // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      //  log10(FFT_shear),log10(FFT_temp),log10(FFT_accel)
                      mod_som_apf_copy_F2_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_avg_dpdt_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_temp_avg_spectra_ptr,
                                                     curr_shear_avg_spectra_ptr,
                                                     curr_accel_avg_spectra_ptr,
                                                     mod_som_apf_ptr->producer_ptr->dacq_ptr
                                                 );
                      break;
                  }//end switch format

                  //ALB update dacq_size
                  mod_som_apf_ptr->producer_ptr->dacq_size=
                               mod_som_apf_ptr->producer_ptr->dacq_ptr-
                              &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

                  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt++;
              }//end if current P<previous P +dz

              //ALB increment cnsmr count
              mod_som_apf_ptr->producer_ptr->dissrates_cnt++;

              //ALB update dissrate available
              dissrate_avail = mod_som_efe_obp_ptr->sample_count -
                  mod_som_apf_ptr->producer_ptr->dissrates_cnt; //elements available have been produced


              //ALB raise flag and stop producer if profile is full (size(dacq)>=25kB)
              //ALB update the number of sample
              //ALB I should also update mod_som_apf_meta_data.sample_cnt in stop producer task i.e. after
              if (mod_som_apf_ptr->producer_ptr->dacq_size+
                  mod_som_apf_ptr->producer_ptr->dacq_element_size>=
                  MOD_SOM_APF_DACQ_STRUCT_SIZE)
                {
                  mod_som_apf_ptr->producer_ptr->dacq_full=true;
                  mod_som_apf_ptr->producer_ptr->
                  acq_profile.mod_som_apf_meta_data.sample_cnt=
                      mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt;
                }
            }  // end of while (dissrate_avail > 0)
          // ALB done with segment storing.
          mod_som_apf_ptr->producer_ptr->dissrate_skipped = 0;
      } //end if started

      // Delay Start Task execution for
      OSTimeDly( MOD_SOM_APF_PRODUCER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      //   Check error code.
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//      printf("EFEOBP efe: %lu\r\n",(uint32_t) mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt);
//      printf("EFEOBP seg: %lu\r\n",(uint32_t) mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}




/*******************************************************************************
 * @brief
 *   apf consumer task
 *
 *   1 - Always write time pressure epsi chi fom on the sd card
 *   1bis - strore the decimated avg spectra
 *   2 - store the meta data on the sd when dacq_profile is full
 *   3 - close SD file after meta data are written.
 *   2- when asked stream out the data to the apf with the afp format
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_consumer_task_f(void  *p_arg){
  RTOS_ERR  err;

  (void)p_arg; // Deliberately unused argument

  uint8_t * curr_dacq_ptr;

  int dissrate_avail=0, reset_dissrate_cnt=0;
  int dissrate_elmnts_offset=0;
  int avg_spectra_offset=0;

  uint64_t * curr_avg_timestamp_ptr;
  float * curr_temp_avg_spectra_ptr;
  float * curr_shear_avg_spectra_ptr;
  float * curr_accel_avg_spectra_ptr;
  float * curr_avg_pressure_ptr;
  float * curr_avg_dpdt_ptr;
  float * curr_epsilon_ptr;
  float * curr_chi_ptr;
//  float * curr_epsi_fom_ptr;
//  float * curr_chi_fom_ptr;

  // parameters for send commands out to APF - mai - Nov 22, 2021
  uint32_t bytes_send;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  uint32_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  int padding = 1; // the padding should be big enough to include the time variance.

  mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr=
                                          mod_som_efe_obp_get_runtime_ptr_f();



  while (DEF_ON) {

      if (mod_som_apf_ptr->consumer_ptr->started_flg){
          /************************************************************************/
          //ALB APF producer phase 1
          //ALB check if producer is started, and if the dacp_profile is NOT full
              dissrate_avail = mod_som_efe_obp_ptr->sample_count -
                  mod_som_apf_ptr->consumer_ptr->dissrates_cnt;  //calculate number of elements available have been produced

              //ALB User stopped efe. I need to reset the obp producers count
              if(dissrate_avail<0){
                  mod_som_apf_ptr->consumer_ptr->dissrates_cnt = 0;
              }
              // LOOP without delay until caught up to latest produced element
              while (dissrate_avail > 0)
                {
                  curr_dacq_ptr=mod_som_apf_ptr->consumer_ptr->dacq_ptr;
                  // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
                  // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
                  // calculate the number of data we skipped, report number of elements skipped.
                  // Reset the consumers cnt equal with producer data plus padding
                  if (dissrate_avail>MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD){ // checking over flow. TODO check adding padding is correct.
                      // reset the consumer count less one buffer than producer count plus padding
                      //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                      reset_dissrate_cnt = mod_som_efe_obp_ptr->sample_count -
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD +
                          padding;
                      // calculate the number of skipped elements
                      mod_som_apf_ptr->consumer_ptr->dissrate_skipped = reset_dissrate_cnt -
                          mod_som_apf_ptr->consumer_ptr->dissrates_cnt;

                      mod_som_io_print_f("\n apf obp consumer task: CB overflow: sample count = %lu,"
                          "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                          (uint32_t)mod_som_efe_obp_ptr->sample_count, \
                          (uint32_t)mod_som_apf_ptr->consumer_ptr->dissrates_cnt, \
                          (uint32_t)mod_som_apf_ptr->consumer_ptr->dissrate_skipped);

                      // save time string into the temporary local string - Mai - Nov 18, 2021
                      sprintf(apf_reply_str,"\n apf obp consumer task: CB overflow: sample count = %lu,"
                              "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                              (uint32_t)mod_som_efe_obp_ptr->sample_count, \
                              (uint32_t)mod_som_apf_ptr->consumer_ptr->dissrates_cnt, \
                              (uint32_t)mod_som_apf_ptr->consumer_ptr->dissrate_skipped);
                      reply_str_len = strlen(apf_reply_str);
                      // sending the above string to the APF port - Mai - Nov 18, 2021
                      bytes_send = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
                      if (bytes_send==0){
                          //TODO handle the error
                      }


                      mod_som_apf_ptr->consumer_ptr->dissrates_cnt = reset_dissrate_cnt;
                  }

                  //ALB calculate the offset for current pointer
                  dissrate_elmnts_offset = mod_som_apf_ptr->consumer_ptr->dissrates_cnt %
                                           MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD;

                  //ALB update avg pressure
                  curr_avg_pressure_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure+
                      dissrate_elmnts_offset;

                  //ALB check if the updated avg pressure is lower than
                  //ALB the previous pressure + dz.
                  //ALB
//                  if (*curr_avg_pressure_ptr>
//                       mod_som_apf_ptr->dacq_pressure+mod_som_apf_ptr->dacq_dz)
//                    {
                  //ALB fake if loop to test apf sdwrite
                      if(dissrate_avail>0)
                        {
                      mod_som_apf_ptr->dacq_pressure=*curr_avg_pressure_ptr;

                      avg_spectra_offset     = (mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                          MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)*
                          mod_som_efe_obp_ptr->settings_ptr->nfft/2;

                      //ALB update the current avg_spectra pointers
                      curr_temp_avg_spectra_ptr   =
                          mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr+
                          avg_spectra_offset;
                      curr_shear_avg_spectra_ptr   =
                          mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr+
                          avg_spectra_offset;
                      curr_accel_avg_spectra_ptr   =
                          mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr+
                          avg_spectra_offset;

                      //ALB update avg timestamp
                      curr_avg_timestamp_ptr =
                          (uint64_t *) (&mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp+
                             dissrate_elmnts_offset);

                      //ALB update avg pressure
                      curr_avg_dpdt_ptr =
                          &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt+
                          dissrate_elmnts_offset;

                      //ALB update the dissrate pointers
                      curr_epsilon_ptr =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon+
                                        dissrate_elmnts_offset;
                      curr_chi_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi+
                                        dissrate_elmnts_offset;
//                      curr_epsi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom+
//                                        dissrate_elmnts_offset;
//                      curr_chi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom+
//                                        dissrate_elmnts_offset;

                      //ALB convert and store the current dissrate and FFT into the MOD format
                      // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      //  log10(FFT_shear),log10(FFT_temp),log10(FFT_accel)
                      mod_som_apf_copy_sd_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_avg_dpdt_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_temp_avg_spectra_ptr,
                                                     curr_shear_avg_spectra_ptr,
                                                     curr_accel_avg_spectra_ptr,
                                                     curr_dacq_ptr
                      );

                      //ALB update dacq_size
                      mod_som_apf_ptr->consumer_ptr->dacq_size=
                          curr_dacq_ptr-mod_som_apf_ptr->consumer_ptr->dacq_ptr;

                      mod_som_apf_ptr->consumer_ptr->stored_dissrates_cnt++;

                      mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
                      mod_som_sdio_write_data_f(
                          mod_som_apf_ptr->consumer_ptr->dacq_ptr,
                          mod_som_apf_ptr->consumer_ptr->dacq_size,
                          &mod_som_apf_ptr->consumer_ptr->consumed_flag);

                  }//end if current P<previous P +dz

                  //ALB increment cnsmr count
                  mod_som_apf_ptr->consumer_ptr->dissrates_cnt++;

                  //ALB update dissrate available
                  dissrate_avail = mod_som_efe_obp_ptr->sample_count -
                      mod_som_apf_ptr->consumer_ptr->dissrates_cnt; //elements available have been produced


//                  //ALB raise flag and stop producer if profile is full (size(dacq)>=25kB)
//                  //ALB update the number of sample
//                  //ALB I should also update mod_som_apf_meta_data.sample_cnt in stop producer task i.e. after
//                  if (mod_som_apf_ptr->producer_ptr->dacq_size+
//                      mod_som_apf_ptr->producer_ptr->dacq_element_size>=
//                      MOD_SOM_APF_DACQ_STRUCT_SIZE)
//                    {
//                      mod_som_apf_ptr->producer_ptr->dacq_full=true;
//                      mod_som_apf_ptr->producer_ptr->
//                      acq_profile.mod_som_apf_meta_data.sample_cnt=
//                          mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt;
//                    }
                }  // end of while (dissrate_avail > 0)
              // ALB done with segment storing.
              mod_som_apf_ptr->consumer_ptr->dissrate_skipped = 0;
          } //end if started

      // Delay Start Task execution for
      OSTimeDly( MOD_SOM_APF_CONSUMER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      //   Check error code.
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}

/*******************************************************************************
 * @brief
 *   apf producer task
 *
 *   shell task
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_shell_task_f(void  *p_arg){
  (void)p_arg; // Deliberately unused argument
  RTOS_ERR err;
  char     input_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE];
  char     output_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE];
  char     apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE];
  uint32_t bytes_sent;
  uint32_t reply_str_len;
  uint32_t input_buf_len;

  mod_som_status_t status=0;
  //TODO This is very hardware dependent
  //TODO Be careful if you want to change serial port
  //TODO This will
  LEUART_TypeDef* apf_leuart_ptr;
  apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();


  while (DEF_ON) {
      OSTimeDly(
              (OS_TICK     )MOD_SOM_APF_SHELL_DELAY,
              (OS_OPT      )OS_OPT_TIME_DLY,
              &err);
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
      // get the whole string from the port
      for (int i=0;i<MOD_SOM_SHELL_INPUT_BUF_SIZE;i++){
          input_buf[i]='\0';
          output_buf[i]='\0';
          apf_reply_str[i]='\0';
      }
      status = mod_som_apf_shell_get_line_f(input_buf,&input_buf_len); // get the whole string from the port (i.e LEUART)

      // send "NAK,<expression>\r\n"
      if (status > 0) // if not success: send a message to APF (LEUART port)
      {
//          mod_som_apf_send_line_f(mod_som_apf_ptr->com_prf_ptr->handle_port, temp_str,temp_str_len);
          //      status = mod_som_apf_send_line_f(apf_leuart_ptr, "NAK,<ReceivedCmd>,\r\n",input_buf_len);
      }

      if (!Str_Cmp(input_buf, "exit"))
      {
          break;
      }
      // we have the whole string, we would convert the string to the right format string we want
      status = mod_som_apf_convert_string_f(input_buf, output_buf);   // convert the input string to the right format: cap -> uncap, coma -> space
      status = mod_som_shell_execute_input_f(output_buf,input_buf_len);   // execute the appropriate routine base on the command. Return the mod_som_status

      if (status>0){
          sprintf(apf_reply_str,"nak,%s.\r\n",output_buf);
          reply_str_len = strlen(apf_reply_str);
          // sending the above string to the APF port - Mai - Nov 18, 2021
          bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
          if(bytes_sent==0){
              //TODO handle the error
          }
      }
  }
}

//TODO Modify the string to match the APF shell rules
/*
 * Function name: mod_som_apf_convert_string_f(char* input_str, char* output_str)
 * Description: parse a input string and make change:
 *              1. convert upercase to lowercase
 *              2. convert coma ',' to a space character ' '
 * Passing parameters: input_str: the string would like to convert
 *                     output_str: return the right format string
 */
uint32_t mod_som_apf_convert_string_f(char* data_str_input, char* data_str_output)
{
    uint32_t retval = 0;
    char* local_str_input_ptr = data_str_input;
    char* local_str_output_ptr = data_str_output;

    while(*local_str_input_ptr!='\0')
    {
        // if the character is upercase -> change to lowercase
        if (*local_str_input_ptr >= 'A' && *local_str_input_ptr <= 'Z')
          {
              *local_str_output_ptr = *local_str_input_ptr+32;
              printf("outptstr: %c\n", *local_str_output_ptr);
          }
        else if(*local_str_input_ptr==',')
          {
            printf("outptstr: %c\n", *local_str_output_ptr);
            *local_str_output_ptr = ' ';
          }
        else
          *local_str_output_ptr = *local_str_input_ptr;
        local_str_input_ptr++;
        local_str_output_ptr++;
    }
    printf("outptstr: %s\n", data_str_output);

    return retval;
}
/*******************************************************************************
 * @brief
 *   Execute user's input when a carriage return is pressed.
 *
 * @param input
 *   The string entered at prompt.
 * @param input_len
 *   Length of string input
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_execute_input_f(char* input,uint32_t input_len){

  //TODO Execute apf functions
  //TODO Return  error message if bad command.

  //TODO switch

    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 * Function Name:
mod_som_status_t mod_som_apf_shell_get_line_f(char *buf, uint32_t * buf_len)
 * Description:
 *   Get string (with max len = MOD_SOM_SHELL_INPUT_BUF_SIZE) input from user.
 *
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_get_line_f(char *buf, uint32_t * bytes_read){
  mod_som_status_t status;
    int32_t i=0;
    char read_char;
    LEUART_TypeDef  *apf_leuart_ptr;

    Mem_Set(buf, '\0', MOD_SOM_SHELL_INPUT_BUF_SIZE); // Clear previous input

    // get the fd of LEUART port
    apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

    //ALB Clear RX buffer
    //TODO this hardware dependent.
    //TODO make it not hardware dependent
    apf_leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;


    // use for to read input from APF until geting '\r' character
    // it would be ended either reach the max characters need to read or '\r'

    for (i=0;i<MOD_SOM_SHELL_INPUT_BUF_SIZE;i++)
    {
        status = mod_som_apf_get_char_f(apf_leuart_ptr, &read_char); // call for getting char from LEUART
        buf[i] = read_char;// save the read character into the buffer
        if (read_char == '\r')
        {
            //MN We do not save \r and \n in the output str
            buf[i] = '\0';
            break;
        }
    }
    *bytes_read = i;
    return status;
}

// get the string from APF (universal)
// mod_som_status_t mod_som_get_line_f(char *buf, uint32_t * bytes_read, uint32_t max_len, LEUART_TypeDef  *apf_leuart_ptr)

//TODO Modify the string to match the APF shell rules
//TODO Not case sensitive
//TODO coma

/*******************************************************************************
 * Function Name: mod_som_status_t mod_som_apf_get_char_f(LEUART_TypeDef *leuart)
 * @brief
 *   Get char input from a PORT. (LEUART)
 *
 * @param leuart
 *   port
 * @param get_char
 *  character as the user is typing
 *  LEUART() is always waiting the input character -> this funct is always return 0 - Arnaud & Mai
 ******************************************************************************/
mod_som_status_t mod_som_apf_get_char_f(LEUART_TypeDef *leuart_ptr, char* read_char)
{
  //Get one bytes from the select port
  *read_char = LEUART_Rx(leuart_ptr);

  // if not succeed, it return error
  if (*read_char==0)
    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_NOT_OK);
  // otherwise, return ok
  return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Send a line from a PORT.
 *
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 *  edit: Nov 19, 2021
 ******************************************************************************/
uint32_t mod_som_apf_send_line_f(LEUART_TypeDef *leuart_ptr,char * buf, uint32_t nb_of_char_to_send)
{
  uint32_t nb_char_sent=0;
  uint8_t *send_char = (uint8_t*) buf;
  int i;

  for (i=0; i<nb_of_char_to_send; i++)
  {
    LEUART_Tx(leuart_ptr,  *send_char);
    send_char++;
    nb_char_sent++;
  }
  //TODO send one line to the port
  return nb_char_sent;

}


/*******************************************************************************
 * @brief
 * convert the dissrates into MOD format
 *    MOD epsilon is 3 bytes ranging from log10(1e-12) and log10(1e-3) V^2/Hz
 *    MOD chi is 3 bytes ranging from log10(1e-12) and log10(1e-3) V^2/Hz
 *    MOD fom is 1 byte
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_copy_F1_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                    float * curr_avg_pressure_ptr,
                                    float * curr_epsilon_ptr,
                                    float * curr_chi_ptr,
                                    float * curr_fom_epsi_ptr,
                                    float * curr_fom_chi_ptr,
                                    uint8_t * dacq_ptr)
{


  uint32_t mod_epsilon, mod_chi;
  uint8_t mod_epsi_fom,mod_chi_fom;
  uint16_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq


  float local_epsilon   = log10(*curr_epsilon_ptr);
  float local_chi       = log10(*curr_chi_ptr);
  float local_epsi_fom  = *curr_fom_epsi_ptr;
  float local_chi_fom   = *curr_fom_chi_ptr;


  uint8_t  mod_bit_dissrates[MOD_SOM_APF_PRODUCER_DISSRATE_RES] = {0};
  uint8_t  mod_bit_fom;

  float min_dissrate=log10(MOD_SOM_APF_PRODUCER_MIN_DISSRATE);
  float max_dissrate=log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE);

  float min_fom=MOD_SOM_APF_PRODUCER_MIN_FOM;
  float max_fom=MOD_SOM_APF_PRODUCER_MAX_FOM;



  //ALB decimate timestamps and store it
  // dissrate timestamp - dacq start timestamp -> 2bytes (65000 sec)
  local_avg_dissrate_timestamp =(uint16_t)
                  (uint64_t)(mod_som_apf_ptr->producer_ptr->acq_profile.
                             mod_som_apf_meta_data.daq_timestamp) -
                              *curr_avg_timestamp_ptr;


  //ALB min out the local epsilon and chi to -12
  local_epsilon = MAX(local_epsilon,min_dissrate);
  local_chi     = MAX(local_chi,min_dissrate);

  //ALB max out the local epsilon and chi to -3
  local_epsilon = MIN(local_epsilon,max_dissrate);
  local_chi     = MIN(local_chi,max_dissrate);

  //ALB min out the local fom to 0
  local_epsi_fom     = MAX(local_epsi_fom,min_fom);
  local_chi_fom      = MAX(local_chi_fom,min_fom);

  //ALB max out the local fom to 10
  local_epsi_fom     = MIN(local_epsi_fom,max_fom);
  local_chi_fom      = MIN(local_chi_fom,max_fom);

  //ALB decimate log10 epsilon with 12 bits (3 bytes)
  mod_epsilon  = (uint32_t) ceil(local_epsilon*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  //ALB decimate log10 chi with 12 bits (3 bytes)
  mod_chi      = (uint32_t) ceil(local_chi*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  //ALB decimate fom with 8 bits (1 bytes)
  mod_epsi_fom  = (uint8_t) ceil(local_epsi_fom*
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;
  mod_chi_fom   = (uint8_t) ceil(local_chi_fom*
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;

  //ALB bit shifting to build mod_bit_fom.
  mod_bit_fom = (mod_epsi_fom<<4) + mod_chi_fom;

  //ALB bit shifting to mod_bit_dissrates.
  mod_bit_dissrates[0]= (uint8_t) (mod_epsilon>>4);
  mod_bit_dissrates[1]= (uint8_t) (mod_epsilon<<4);
  mod_bit_dissrates[1]= (uint8_t) (mod_bit_dissrates[1] & (mod_chi>>8));
  mod_bit_dissrates[2]= (uint8_t) mod_chi;


  //ALB copy the data in the acq profile structure
  //ALB TODO check the dacq_ptr update and its value when out of that function

  memcpy(dacq_ptr,
         &local_avg_dissrate_timestamp,
         MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
  memcpy(dacq_ptr,
         curr_avg_pressure_ptr,
         MOD_SOM_APF_DACQ_PRESSURE_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_dissrates,
         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  memcpy(dacq_ptr,
         &mod_bit_fom,
         MOD_SOM_APF_PRODUCER_FOM_RES);
}


/*******************************************************************************
 * @brief
 * downgrade avg spectra into MOD format
 *
 * We decimate the spectra from nfft/2 Fourier coef to nfft/8 (e.g., 4096 to 512)
 * In frequency it means that the decimated spectrum will a max freq=  320Hz/8 = 40Hz
 * Each Fourier (foco) is digitized in 2bytes, assuming a log10-range of -12 to 1.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_copy_F2_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                float * curr_avg_pressure_ptr,
                                float * curr_avg_dpdt_ptr,
                                float * curr_epsilon_ptr,
                                float * curr_chi_ptr,
                                float * curr_temp_avg_spectra_ptr,
                                float * curr_shear_avg_spectra_ptr,
                                float * curr_accel_avg_spectra_ptr,
                                uint8_t * dacq_ptr)
{

  //ALB declare the local parameters
  uint32_t mod_epsilon, mod_chi;
  uint16_t mod_shear_foco, mod_temp_foco, mod_accel_foco;
  uint16_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq
  uint8_t  mod_bit_dissrates[MOD_SOM_APF_PRODUCER_DISSRATE_RES] = {0};

  //ALB store the dissrate and foco values in local params.
  float local_epsilon        = log10(*curr_epsilon_ptr);
  float local_chi            = log10(*curr_chi_ptr);
  float local_shear_avg_fft  = log10(*curr_shear_avg_spectra_ptr);
  float local_temp_avg_fft   = log10(*curr_temp_avg_spectra_ptr);
  float local_accel_avg_fft  = log10(*curr_accel_avg_spectra_ptr);

  //ALB store the max min dissrate and foco values in local params.
  //ALB TODO this could be done with macro and not waste CPU time
  float min_dissrate = log10(MOD_SOM_APF_PRODUCER_MIN_DISSRATE);
  float max_dissrate = log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE);
  float min_foco     = log10(MOD_SOM_APF_PRODUCER_MIN_FOCO);
  float max_foco     = log10(MOD_SOM_APF_PRODUCER_MAX_FOCO);

  //ALB decimate timestamps and store it
  //ALB dissrate timestamp - dacq start timestamp -> 2bytes (65000 sec)
  local_avg_dissrate_timestamp =(uint16_t)
                  (uint64_t)(mod_som_apf_ptr->producer_ptr->acq_profile.
                             mod_som_apf_meta_data.daq_timestamp) -
                              *curr_avg_timestamp_ptr;

  //ALB decimate dissrates,
  //ALB first make sure it is above the min values
  local_epsilon = MAX(local_epsilon,min_dissrate);
  local_chi     = MAX(local_chi,min_dissrate);

  //ALB then make sure it is below the max values
  local_epsilon = MIN(local_epsilon,max_dissrate);
  local_chi     = MIN(local_chi,max_dissrate);

  //ALB then digitize on a 12bits number
  mod_epsilon  = (uint32_t) ceil(local_epsilon*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  mod_chi      = (uint32_t) ceil(local_chi*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  //ALB then store epsi and chi in a 3 bytes array
  mod_bit_dissrates[0]= (uint8_t) (mod_epsilon>>4);
  mod_bit_dissrates[1]= (uint8_t) (mod_epsilon<<4);
  mod_bit_dissrates[1]= (uint8_t) (mod_bit_dissrates[1] & (mod_chi>>8));
  mod_bit_dissrates[2]= (uint8_t) mod_chi;

  //ALB copy the data in the acq profile structure
  //ALB TODO check the dacq_ptr update and its value when out of that function
  memcpy(dacq_ptr,
         &local_avg_dissrate_timestamp,
         MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
  memcpy(dacq_ptr,
         curr_avg_pressure_ptr,
         MOD_SOM_APF_DACQ_PRESSURE_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_dissrates,
         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  memcpy(dacq_ptr,
         curr_avg_dpdt_ptr,
         MOD_SOM_APF_DACQ_DPDT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_DPDT_SIZE;

  //ALB decimate fourier coef (foco),
  for (int i=0;i<mod_som_apf_ptr->producer_ptr->nfft_diag;i++)
    {
      //ALB first make sure it is above the min/max values
      local_shear_avg_fft = MAX(local_shear_avg_fft,min_foco);
      local_shear_avg_fft = MIN(local_shear_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_shear_foco  = (uint16_t) ceil(local_shear_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);

      //ALB first make sure it is above the min/max values
      local_temp_avg_fft = MAX(local_temp_avg_fft,min_foco);
      local_temp_avg_fft = MIN(local_temp_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_temp_foco  = (uint16_t) ceil(local_temp_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);

      //ALB first make sure it is above the min/max values
      local_accel_avg_fft = MAX(local_accel_avg_fft,min_foco);
      local_accel_avg_fft = MIN(local_accel_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_accel_foco  = (uint16_t) ceil(local_accel_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);


      memcpy(dacq_ptr,
             &mod_shear_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;

      memcpy(dacq_ptr,
             &mod_temp_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;
      memcpy(dacq_ptr,
             &mod_accel_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;

      local_shear_avg_fft   = log10(*(curr_shear_avg_spectra_ptr+i));
      local_temp_avg_fft    = log10(*(curr_temp_avg_spectra_ptr+i));
      local_accel_avg_fft   = log10(*(curr_accel_avg_spectra_ptr+i));


  }

}

/*******************************************************************************
 * @brief
 * downgrade avg spectra into MOD format
 *
 * We decimate the spectra from nfft/2 Fourier coef to nfft/8 (e.g., 4096 to 512)
 * In frequency it means that the decimated spectrum will a max freq=  320Hz/8 = 40Hz
 * Each Fourier (foco) is digitized in 2bytes, assuming a log10-range of -12 to 1.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_copy_sd_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                float * curr_avg_pressure_ptr,
                                float * curr_avg_dpdt_ptr,
                                float * curr_epsilon_ptr,
                                float * curr_chi_ptr,
                                float * curr_temp_avg_spectra_ptr,
                                float * curr_shear_avg_spectra_ptr,
                                float * curr_accel_avg_spectra_ptr,
                                uint8_t * dacq_ptr)
{

  //ALB declare the local parameters
  uint32_t mod_epsilon, mod_chi;
  uint16_t mod_shear_foco, mod_temp_foco, mod_accel_foco;
  uint16_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq
  uint8_t  mod_bit_dissrates[MOD_SOM_APF_PRODUCER_DISSRATE_RES] = {0};

  //ALB store the dissrate and foco values in local params.
  float local_epsilon        = log10(*curr_epsilon_ptr);
  float local_chi            = log10(*curr_chi_ptr);
  float local_shear_avg_fft  = log10(*curr_shear_avg_spectra_ptr);
  float local_temp_avg_fft   = log10(*curr_temp_avg_spectra_ptr);
  float local_accel_avg_fft  = log10(*curr_accel_avg_spectra_ptr);

  //ALB store the max min dissrate and foco values in local params.
  //ALB TODO this could be done with macro and not waste CPU time
  float min_dissrate = log10(MOD_SOM_APF_PRODUCER_MIN_DISSRATE);
  float max_dissrate = log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE);
  float min_foco     = log10(MOD_SOM_APF_PRODUCER_MIN_FOCO);
  float max_foco     = log10(MOD_SOM_APF_PRODUCER_MAX_FOCO);

  //ALB decimate timestamps and store it
  //ALB dissrate timestamp - dacq start timestamp -> 2bytes (65000 sec)
  local_avg_dissrate_timestamp =(uint16_t)
                  (uint64_t)(mod_som_apf_ptr->producer_ptr->acq_profile.
                             mod_som_apf_meta_data.daq_timestamp) -
                              *curr_avg_timestamp_ptr;

  //ALB decimate dissrates,
  //ALB first make sure it is above the min values
  local_epsilon = MAX(local_epsilon,min_dissrate);
  local_chi     = MAX(local_chi,min_dissrate);

  //ALB then make sure it is below the max values
  local_epsilon = MIN(local_epsilon,max_dissrate);
  local_chi     = MIN(local_chi,max_dissrate);

  //ALB then digitize on a 12bits number
  mod_epsilon  = (uint32_t) ceil(local_epsilon*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  mod_chi      = (uint32_t) ceil(local_chi*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  //ALB then store epsi and chi in a 3 bytes array
  mod_bit_dissrates[0]= (uint8_t) (mod_epsilon>>4);
  mod_bit_dissrates[1]= (uint8_t) (mod_epsilon<<4);
  mod_bit_dissrates[1]= (uint8_t) (mod_bit_dissrates[1] & (mod_chi>>8));
  mod_bit_dissrates[2]= (uint8_t) mod_chi;

  //ALB copy the data in the acq profile structure
  //ALB TODO check the dacq_ptr update and its value when out of that function
  memcpy(dacq_ptr,
         &local_avg_dissrate_timestamp,
         MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
  memcpy(dacq_ptr,
         curr_avg_pressure_ptr,
         MOD_SOM_APF_DACQ_PRESSURE_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_dissrates,
         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  memcpy(dacq_ptr,
         curr_avg_dpdt_ptr,
         MOD_SOM_APF_DACQ_DPDT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_DPDT_SIZE;

  //ALB decimate fourier coef (foco),
  for (int i=0;i<mod_som_apf_ptr->producer_ptr->nfft_diag;i++)
    {
      //ALB first make sure it is above the min/max values
      local_shear_avg_fft = MAX(local_shear_avg_fft,min_foco);
      local_shear_avg_fft = MIN(local_shear_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_shear_foco  = (uint16_t) ceil(local_shear_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);

      //ALB first make sure it is above the min/max values
      local_temp_avg_fft = MAX(local_temp_avg_fft,min_foco);
      local_temp_avg_fft = MIN(local_temp_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_temp_foco  = (uint16_t) ceil(local_temp_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);

      //ALB first make sure it is above the min/max values
      local_accel_avg_fft = MAX(local_accel_avg_fft,min_foco);
      local_accel_avg_fft = MIN(local_accel_avg_fft,max_foco);
      //ALB then digitize on a 16bits number
      mod_accel_foco  = (uint16_t) ceil(local_accel_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);


      memcpy(dacq_ptr,
             &mod_shear_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;

      memcpy(dacq_ptr,
             &mod_temp_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;
      memcpy(dacq_ptr,
             &mod_accel_foco,
             MOD_SOM_APF_PRODUCER_FOCO_RES);
      dacq_ptr+=MOD_SOM_APF_PRODUCER_FOCO_RES;

      local_shear_avg_fft   = log10(*(curr_shear_avg_spectra_ptr+i));
      local_temp_avg_fft    = log10(*(curr_temp_avg_spectra_ptr+i));
      local_accel_avg_fft   = log10(*(curr_accel_avg_spectra_ptr+i));


  }
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
  mod_som_status_t status=0;
  RTOS_ERR  err;

  status=MOD_SOM_APF_STATUS_OK;
  uint32_t delay=0xFF;
  CPU_CHAR filename[100];

  //ALB start collecting CTD.
  status |= mod_som_sbe41_connect_f();
  status |= mod_som_sbe41_start_collect_data_f();

  //ALB enable SDIO hardware
  mod_som_sdio_enable_hardware_f();

  mod_som_apf_ptr->profile_id=profile_id;
  //ALB Open SD file,
  sprintf(filename, "Profile%lu",(uint32_t) mod_som_apf_ptr->profile_id);
  mod_som_sdio_define_filename_f(filename);
  //ALB write MODSOM settings on the SD file
//  mod_som_settings_sd_settings_f();
  //ALB initialize Meta_Data Structure, TODO
  mod_som_apf_init_meta_data(mod_som_apf_ptr->producer_ptr->
                             acq_profile.mod_som_apf_meta_data);


	//ALB start ADC master clock timer
  mod_som_apf_ptr->profile_id=profile_id;
  mod_som_apf_ptr->daq=true;
	//ALB start turbulence processing task
 status|= mod_som_efe_obp_start_fill_segment_task_f();
 status|= mod_som_efe_obp_start_cpt_spectra_task_f();
 status|= mod_som_efe_obp_start_cpt_dissrate_task_f();


  // Delay Start Task execution for
  OSTimeDly( MOD_SOM_APF_DACQ_CTD_DELAY,             //   consumer delay is #define at the beginning OS Ticks
             OS_OPT_TIME_DLY,          //   from now.
             &err);


  //ALB get a P reading and define the dz to get 25kB in the producer->dacq_profile
  //ALB the dz will depends on the comm_packet_format.
  //TODO
  mod_som_apf_ptr->producer_ptr->
  acq_profile.mod_som_apf_meta_data.sample_cnt =
      floor(MOD_SOM_APF_DACQ_STRUCT_SIZE/
            (MOD_SOM_APF_METADATA_STRUCT_SIZE+
      mod_som_apf_ptr->producer_ptr->dacq_element_size));

  //ALB I should have a delay here. So the system has time to get a pressure sample
  //ALB I think OS_TIME_delay would the same and free the CPU for other tasks.
  //ALB get a pressure sample
  mod_som_apf_ptr->dacq_start_pressure = mod_som_sbe41_get_pressure_f();
  //ALB the evolving pressure is initialized with the start pressure.
  mod_som_apf_ptr->dacq_pressure=mod_som_apf_ptr->dacq_start_pressure;
  //ALB define dz
  mod_som_apf_ptr->dacq_dz = (mod_som_apf_ptr->dacq_start_pressure-
                              MOD_SOM_APF_DACQ_MINIMUM_PRESSURE)/
                              mod_som_apf_ptr->producer_ptr->
                              acq_profile.mod_som_apf_meta_data.sample_cnt;




//  //ALB start APF producer task
  status |= mod_som_apf_start_producer_task_f();
//  //ALB start APF consumer task
//  status |= mod_som_apf_start_consumer_task_f();

  //ALB poor practice delay I think OS_TIME_delay would the same and free the CPU for other tasks.
  while (delay>0){
      delay--;
  }

  status|=mod_som_efe_sampling_f();

	return status;
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
  status|= mod_som_efe_stop_sampling_f();



  // stop collecting CTD data
  status|= mod_som_sbe41_stop_collect_data_f();
  status|= mod_som_sbe41_disconnect_f();

	// stop turbulence processing task
  status = mod_som_efe_obp_stop_fill_segment_task_f();
  status|= mod_som_efe_obp_stop_cpt_spectra_task_f();
  status|= mod_som_efe_obp_stop_cpt_dissrate_task_f();

  //ALB stop APF producer task
  status |= mod_som_apf_stop_producer_task_f();
  //ALB stop APF consumer task
  status |= mod_som_apf_stop_consumer_task_f();


  //ALB disable SDIO hardware
  mod_som_sdio_disable_hardware_f();

  //reset Daq flag
  mod_som_apf_ptr->daq=false;

	return status;
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

//mod_som_apf_status_t mod_som_apf_daq_status_f(){
//  mod_som_status_t status;
//  status=MOD_SOM_APF_STATUS_OK;
//
//  if(mod_som_apf_ptr->daq){
//      status=mod_som_io_print_f("daq?,ack,%s","enabled");
//  }else{
//      status=mod_som_io_print_f("daq?,ack,%s","disabled");
//  }
//
//  //ALB Dana want an error message here but I do not think there is a situation
//  if (status!=MOD_SOM_APF_STATUS_OK){
//      mod_som_io_print_f("daq?,nak,%lu",status);
//  }
//	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
//}

/*******************************************************************************
 * @brief
 *    initialize dacq Meta_Data
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_apf_init_meta_data(mod_som_apf_meta_data_t mod_som_apf_meta_data)
{


  mod_som_efe_obp_ptr_t local_efe_obp=mod_som_efe_obp_get_runtime_ptr_f();
  mod_som_settings_struct_ptr_t local_settings_ptr=
                                              mod_som_settings_get_settings_f();

  mod_som_apf_meta_data.nfft=local_efe_obp->settings_ptr->nfft;
  mod_som_apf_meta_data.comm_telemetry_packet_format=
      mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format;
  mod_som_apf_meta_data.sd_format=
      mod_som_apf_ptr->settings_ptr->sd_packet_format;

  mod_som_apf_meta_data.efe_sn=
                      strtol(local_efe_obp->efe_settings_ptr->sn,NULL,10);
  mod_som_apf_meta_data.firmware_rev=
                      strtol(local_settings_ptr->firmware, NULL, 16);
  mod_som_apf_meta_data.modsom_sn=
                      strtol(local_settings_ptr->sn,NULL,10);

  //ALB TODO local_efe_obp->efe_settings_ptr->sensors[0].name;
  mod_som_apf_meta_data.probe1.type=temp;
  mod_som_apf_meta_data.probe1.sn=
   (uint16_t)  strtol(local_efe_obp->efe_settings_ptr->sensors[0].sn, NULL, 10);
  mod_som_apf_meta_data.probe1.cal=
   (uint16_t) ceil(local_efe_obp->efe_settings_ptr->sensors[0].cal);

  mod_som_apf_meta_data.probe2.type=shear;
  mod_som_apf_meta_data.probe2.sn=
  (uint16_t)  strtol(local_efe_obp->efe_settings_ptr->sensors[1].sn, NULL, 10);
  mod_som_apf_meta_data.probe2.cal=
      (uint16_t) ceil(local_efe_obp->efe_settings_ptr->sensors[1].cal);

  mod_som_apf_meta_data.profile_id=mod_som_apf_ptr->profile_id;

  //ALB get some values
  sl_sleeptimer_timestamp_t toto =sl_sleeptimer_get_time();
  mod_som_apf_meta_data.daq_timestamp=toto;

  mod_som_apf_meta_data.sample_cnt=0;
  mod_som_apf_meta_data.end_metadata=0xFFFF;
}



/*******************************************************************************
 * @brief
 *   command shell for FwRev? command
 *   display Firmware Revision ID
 *
 * ALB comment #1
 * ALB Hello mai
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fwrev_status_f(){

  mod_som_apf_status_t status=0;
  uint32_t bytes_send;

  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  uint32_t reply_str_len = 0;

  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  mod_som_settings_struct_ptr_t local_settings_ptr=
                                          mod_som_settings_get_settings_f();


  mod_som_io_print_f("fwrev?,ack,%s\r\n",local_settings_ptr->firmware);

  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"fwrev?,ack,%s\r\n",local_settings_ptr->firmware);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_send = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  if (bytes_send!=reply_str_len){
      status=MOD_SOM_APF_STATUS_FAIL_SEND_MS;
  }

	return status;
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
  mod_som_apf_status_t status = 0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;
  uint32_t bytes_sent = 0;

	// Wake up sensor's interface.
	// mod_som_apf_status_t status mod_som_apf_io_send_line_f(char* buf)
  // 1. sprint
  // 2. io_print
  // 3. send_line
  // 4. use ret_val from send_line() to update status and return status
  // ie:
  // sprintf(str,"ok?,ack,%s\r\n","status report");
  // called: mod_som_apf_io_send_line_f(str)
  //
  // mod_som_io_xfer_item_ptr->printf_str_ptr

	mod_som_io_print_f("ok?,ack,%s\r\n","status report");

	// can it see this string: mod_som_io_xfer_item_ptr->printf_str_ptr

  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"ok?,ack,%s\r\n","status report");
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  if(bytes_sent==0){
      //TODO handle the error
  }

//	mod_som_io_print_f("ok?,nak,%s\r\n","error message");
	return status;
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
  mod_som_apf_status_t status=0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;
  uint32_t bytes_sent = 0;

  //make sure we are not in daq mode
  mod_som_apf_daq_stop_f();
  //ALB save settings in the UserData page
	mod_som_io_print_f("poweroff,ack\r\n");

  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"poweroff,ack\r\n");
  reply_str_len = strlen(apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  if(bytes_sent==0){
      //TODO handle the error
  }


	return status;
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
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;
  uint32_t bytes_sent = 0;

  mod_som_apf_status_t status = 0;
  mod_som_settings_struct_ptr_t local_settings_ptr=
                                          mod_som_settings_get_settings_f();


  status|=mod_som_io_print_f("EpsiNo,ack,SOM%s,%s,EFE%s,%s\r\n",
	                   local_settings_ptr->rev,local_settings_ptr->sn,
	                   local_settings_ptr->mod_som_efe_settings.rev,
	                   local_settings_ptr->mod_som_efe_settings.sn);
  // save the string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"EpsiNo,ack,SOM%s,%s,EFE%s,%s\r\n",
          local_settings_ptr->rev,local_settings_ptr->sn,
          local_settings_ptr->mod_som_efe_settings.rev,
          local_settings_ptr->mod_som_efe_settings.sn);
  reply_str_len = strlen(apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  status = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

if (status>0)
{
    mod_som_io_print_f("EpsiNo?,nak,%lu\r\n",status);

    sprintf(apf_reply_str,"EpsiNo?,nak,%lu\r\n",status);
    reply_str_len = strlen(apf_reply_str);
    // sending the above string to the APF port - Mai - Nov 18, 2021
    bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

    if(bytes_sent==0){
        //TODO handle the error
    }

}
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
mod_som_apf_status_t mod_som_apf_probe_id_f(CPU_INT16U argc,
                                            CPU_CHAR *argv[]){

  mod_som_apf_status_t status=0;
  uint32_t cal;
  char *endptr;
  bool argument_flag=false;
  uint8_t channel_id;
  uint8_t length_argument2;
  uint8_t length_argument3;

  // for send_string to the port
  uint32_t bytes_sent = 0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


  mod_som_efe_settings_ptr_t local_efe_settings_ptr=
      mod_som_efe_get_settings_ptr_f();

  switch (argc){
    case 4:
      cal= strtol(argv[3], &endptr, 10);
      length_argument2=strlen(argv[2]);
      length_argument3=strlen(argv[3]);

      if(strcmp(argv[1],"S") & (length_argument2==3) & (length_argument3==2) & (cal>0)){
          channel_id=1;
          memcpy(&local_efe_settings_ptr->sensors[channel_id].sn,
                  argv[2],3);

          local_efe_settings_ptr->sensors[1].cal=cal;
          argument_flag=true;
      }

      if(strcmp(argv[1],"F") & (length_argument2==3) & (length_argument3==2) & (cal>0)){
          channel_id=0;
          memcpy(&local_efe_settings_ptr->sensors[0].sn,
                  argv[2],3);
           local_efe_settings_ptr->sensors[0].cal=cal;
           argument_flag=true;
      }

      if (argument_flag){
          status|=mod_som_settings_save_settings_f();
          status|= mod_som_io_print_f("ProbeNo,ack,%s,%s,%lu\r\n",argv[1],
                                      local_efe_settings_ptr->sensors[channel_id].sn,
                                      (uint32_t)local_efe_settings_ptr->sensors[channel_id].cal);
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"ProbeNo,ack,%s,%s,%lu\r\n",argv[1],
                  local_efe_settings_ptr->sensors[channel_id].sn,
                  (uint32_t)local_efe_settings_ptr->sensors[channel_id].cal);
     }else{
          status|=MOD_SOM_APF_STATUS_FAIL_WRONG_ARGUMENTS;
          mod_som_io_print_f("ProbeNo,nak,%lu\r\n",status);
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"ProbeNo,nak,%lu\r\n",status);
     }
      reply_str_len = strlen(apf_reply_str);
     // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      if(bytes_sent==0){
          //TODO handle the error
      }


      break;
    default:
      status|=MOD_SOM_APF_STATUS_FAIL_WRONG_ARGUMENTS;
  }
  if(status>0){
      mod_som_io_print_f("ProbeNo,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"ProbeNo,nak,%lu\r\n",status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  }
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

  mod_som_apf_status_t status=0;
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  mod_som_efe_settings_ptr_t local_efe_settings_ptr=
      mod_som_efe_get_settings_ptr_f();

  status|= mod_som_io_print_f("ProbeNo,ack,%s,%s,%lu\r\n","S",
                              local_efe_settings_ptr->sensors[1].sn,
                              (uint32_t)local_efe_settings_ptr->sensors[1].cal);
  // save to the local string for sending out - Mai-Nov 18, 2021
  sprintf(apf_reply_str,"ProbeNo,ack,%s,%s,%lu\r\n","S",
          local_efe_settings_ptr->sensors[1].sn,
          (uint32_t)local_efe_settings_ptr->sensors[1].cal);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  status|= mod_som_io_print_f("ProbeNo,ack,%s,%s,%lu\r\n","S",
                              local_efe_settings_ptr->sensors[0].sn,
                              (uint32_t)local_efe_settings_ptr->sensors[0].cal);
  // save to the local string for sending out - Mai-Nov 18, 2021
  sprintf(apf_reply_str,"ProbeNo,ack,%s,%s,%lu\r\n","S",
          local_efe_settings_ptr->sensors[0].sn,
          (uint32_t)local_efe_settings_ptr->sensors[0].cal);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }


  if(status>0){
      mod_som_io_print_f("ProbeNo,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"ProbeNo,nak,%lu\r\n",status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      if(bytes_sent==0){
          //TODO handle the error
      }

  }

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for sleep command
 *   put SOM to sleep
 *   should return an apf status.
 *   This command puts the Epsilometer back to sleep.
 *   If the sensor happened to be in DAQ mode when it was awakened,
 *   the sleep command has the e ect of resuming the DAQ period
 *   already in progress.
 *   If the sensor was not in DAQ mode, then the sleep command has the
 *   effect of inducing the Epsilometer into its low-power sleep state.
 *   The sensor should respond with sleep,ack\r\n.
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_sleep_f(){

  mod_som_apf_status_t status=0;
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


  //ALB check if in Daq mode
  if(mod_som_apf_ptr->daq){
      //ALB we are in Daq mode. Do nothing.
      // send some ack - mnbui Dec 1, 2021
        status|=mod_som_io_print_f("sleep,ack,%lu\r\n",status);
       // save to the local string for sending out - Mai- Dec1, 2021
       sprintf(apf_reply_str,"sleep,ack,%lu\r\n",status);
       reply_str_len = strlen(apf_reply_str);
       // sending the above string to the APF port - Mai - Dec 1, 2021
       bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
       if(bytes_sent==0){
           //TODO handle the error
       }

  }else{
      //ALB we are not in daq mode make sure
      //ALB efe,sdio,efe obp,sbe-sniffer are asleep
      // stop ADC master clock timer
 /*     status|= mod_som_efe_stop_sampling_f();



      // stop collecting CTD data
      status|= mod_som_sbe41_stop_collect_data_f();
      status|= mod_som_sbe41_disconnect_f();

      // stop turbulence processing task
      status = mod_som_efe_obp_stop_fill_segment_task_f();
      status|= mod_som_efe_obp_stop_cpt_spectra_task_f();
      status|= mod_som_efe_obp_stop_cpt_dissrate_task_f();

      //ALB stop APF producer task
      status |= mod_som_apf_stop_producer_task_f();
      //ALB stop APF consumer task
      status |= mod_som_apf_stop_consumer_task_f();


      //ALB disable SDIO hardware
      mod_som_sdio_disable_hardware_f();
// comment out for testing sleep - mai Nov 30, 2021
*/
      if (status==0){
          status|=mod_som_io_print_f("sleep,ack\r\n");
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"sleep,ak\r\n");
      }else{
          status|=mod_som_io_print_f("sleep,nak,%lu\r\n",status);
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"sleep,nak,%lu\r\n",status);
     }
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      if(bytes_sent==0){
          //TODO handle the error
      }

}

  return mod_som_apf_encode_status_f(status);
}

/*******************************************************************************
 * @brief
 *   command shell for time command
 *   set UnixEpoch time on SOM
 *   should return an apf status.
 *   command: "Time,UnixEpoch\r"
 *   reply: "time,ack,UnixEpoch\r\n"
 *       or "time,nak,error-description\r\n"
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_f(CPU_INT16U argc,
                                        CPU_CHAR *argv[]){
  RTOS_ERR  p_err;
  uint16_t year;
  uint8_t month;
  uint8_t month_day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;

  uint32_t bytes_sent = 0;
  // paramters for send_line_f()
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  sl_sleeptimer_timestamp_t time;
  sl_sleeptimer_time_zone_offset_t time_zone;
  sl_sleeptimer_date_t date;
  mod_som_status_t status = 0;

  time_zone=0;
  time= shellStrtol(argv[1],&p_err);

  sl_sleeptimer_convert_time_to_date((uint32_t) time,time_zone,&date);

  year=date.year;
  month=date.month;
  month_day=date.month_day;
  hour=date.hour;
  min=date.min;
  sec=date.sec;

  status|=mod_som_calendar_set_time_f(year,month,month_day,hour,min,sec,time_zone);
  status|=mod_som_settings_save_settings_f();
  status|=mod_som_io_print_f("time,ack,%s\r\n",argv[1]);


  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"time,ack,%s\r\n",argv[1]);
  reply_str_len = strlen(apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if (status==MOD_SOM_APF_STATUS_OK){
      mod_som_io_print_f("time,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"time,nak,%lu\r\n",status);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

 }
  if(bytes_sent==0){
      //TODO handle the error
  }

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for time? command
 *   get UnixEpoch time on SOM
 *   should return an apf status.
 * Description:
 * - get the port
 * - get time
 * - construct the time string
 * - get the string len of the time string
 * - send the string to the port
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_status_f(){

  mod_som_status_t status = 0;
  sl_sleeptimer_timestamp_t time;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  uint32_t bytes_sent = 0;

  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  time= sl_sleeptimer_get_time();

  // send out "time,ak,time\r\n"
  status|=mod_som_io_print_f("time?,ack,%lu\r\n",time);

  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"time?,ack,%lu\r\n",(unsigned long)time);
  reply_str_len = strlen(apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

   if (status==MOD_SOM_APF_STATUS_OK){
	    mod_som_io_print_f("time?,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"time?,nak,%lu\r\n",status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
	}
   if(bytes_sent==0){
       //TODO handle the error
   }

	return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_comm_packet_format_f
 *   set the format of the data
 *   0 = no format (latter on called F0)
 *   1 = format 1 (F1) time pressure epsilon chi fom
 *   2 = format 2 (F2) time pressure epsilon chi fom + something
 *   3 = format 3 (F3) time pressure epsilon chi + decimated spectra
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
mod_som_apf_status_t mod_som_apf_comm_packet_format_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[])
{

  RTOS_ERR  p_err;
  uint8_t mode;
  mod_som_status_t status=0;

  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mode=shellStrtol(argv[1],&p_err);
      if(mode<3){
          mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format=mode;
      }else{
          mod_som_io_print_f("comm_packet_format,nak,%s\r\n","argument to high");
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"comm_packet_format,nak,%s\r\n","argument to high");
          reply_str_len = strlen(apf_reply_str);
          // sending the above string to the APF port - Mai - Nov 18, 2021
          bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      }
      break;
    default:
      mod_som_io_print_f("comm_packet_format,nak,%s\r\n","wrong arguments");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"comm_packet_format,nak,%s\r\n","wrong arguments");
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      break;
    }
    if(bytes_sent==0){
        //TODO handle the error
    }


  switch (mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format){
    case F0:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
      break;
    case F1:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_FOM_SIZE;
      break;
    case F2:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_SEAWATER_SPEED_SIZE+
         (MOD_SOM_EFE_OBP_CHANNEL_NUMBER*
          mod_som_apf_ptr->producer_ptr->nfft_diag);

      break;
  }

  mod_som_settings_save_settings_f();

  mod_som_io_print_f("comm_packet_format,ack\r\n");
  // save to the local string for sending out - Mai-Nov 18, 2021
  sprintf(apf_reply_str,"comm_packet_format,ack\r\n");
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  return status;
}

/*******************************************************************************
 * @brief
 *   command shell for ProbeNo? command
 *   get the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_comm_packet_format_status_f(){

  mod_som_apf_status_t status=0;
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  status|= mod_som_io_print_f("comm_packet_format,ack\r\n");
  // save to the local string for sending out - Mai-Dec 1, 2021
  sprintf(apf_reply_str,"comm_packet_format,ack\r\n");
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Dec 1, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

   if(status>0){
      mod_som_io_print_f("comm_packet_format,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai- Dec 1, 2021
      sprintf(apf_reply_str,"comm_packet_format,nak,%lu\r\n",status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Dec 1, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  }
   if(bytes_sent==0){
       //TODO handle the error
   }

  return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_sd_format_f
 *   set the format of the data stored in the SD card
 *   0 = no sd storage format (latter on called SD0)
 *   1 = raw store everything on the SD card
 *   2 = time pressure epsilon chi fom dpdt kvis avg_t avg_s decimated spectra
 *   3 = time pressure epsilon chi fom dpdt kvis avg_t avg_s full avg spectra
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
mod_som_apf_status_t mod_som_apf_sd_format_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[])
{

  mod_som_apf_status_t status=0;
  char *endptr;
  uint8_t mode;
  bool good_argument=false;
  CPU_INT16U argc_sbe41 = 2;
  CPU_CHAR   *argv_sbe41_sd[2]={"sbe.mode" ,"1"};
  CPU_CHAR   *argv_sbe41_off[2]={"sbe.mode" ,"2"};
  CPU_INT16U argc_efe = 2;
  CPU_CHAR   *argv_efe_sd[2]={"efe.mode" ,"1"};
  CPU_CHAR   *argv_efe_off[2]={"efe.mode" ,"2"};
  CPU_INT16U argc_efe_obp = 2;
  CPU_CHAR   *argv_efe_obp_sd[2]={"efeobp.mode" ,"1"};
  CPU_CHAR   *argv_efe_obp_off[2]={"efeobp.mode" ,"2"};
  CPU_CHAR   *argv_efe_obp_avgspec_format[2]={"efeobp.format" ,"2"}; //spectra
  CPU_CHAR   *argv_efe_obp_none_format[2]={"efeobp.format" ,"3"}; //only dissrates

  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
//      mode=shellStrtol(argv[1],&p_err);
      mode = strtol(argv[1], &endptr, 10); // strtol

      if((mode<3) & (mode>=0)){
          mod_som_apf_ptr->settings_ptr->sd_packet_format=mode;
          good_argument=true;
      }
      break;
    }

    if(mod_som_apf_ptr->settings_ptr->sd_packet_format>1){
        //ALB start writing data on the sd from the apf producers.
        mod_som_apf_ptr->consumer_ptr->started_flg=true;
        //ALB if sd_packet is "on" I ll always write the disrates on the SD card.
        mod_som_efe_obp_consumer_format_f(argc_efe_obp,argv_efe_obp_none_format);
        mod_som_efe_obp_mode_f(argc_efe_obp,argv_efe_obp_sd);
    }else{
        mod_som_efe_obp_mode_f(argc_efe_obp,argv_efe_obp_off);
    }
    if(mod_som_apf_ptr->settings_ptr->sd_packet_format==1){
        //ALB write raw data in the Profile file
        mod_som_sbe41_consumer_mode_f(argc_sbe41,argv_sbe41_sd);
        mod_som_efe_consumer_mode_f(argc_efe,argv_efe_sd);
    }else{
        //ALB sd_mode will use apf producer data.
        //ALB make sure I turn off raw data sd write.
        mod_som_sbe41_consumer_mode_f(argc_sbe41,argv_sbe41_off);
        mod_som_efe_consumer_mode_f(argc_efe,argv_efe_off);
    }


    if(mod_som_apf_ptr->settings_ptr->sd_packet_format==2){
        //ALB write spectra from efeobp on the SD card
        mod_som_efe_obp_consumer_format_f(argc_efe_obp,argv_efe_obp_avgspec_format);
    }else{
        mod_som_efe_obp_consumer_format_f(argc_efe_obp,argv_efe_obp_none_format);
    }

    if(mod_som_apf_ptr->settings_ptr->sd_packet_format==3){
        //ALB write decimated spectra from apf on the SDcard.


    }

  status|=mod_som_settings_save_settings_f();

  if(good_argument){
      mod_som_io_print_f("sd_format,ak\r\n");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"sd_format,ak\r\n");
  }else{
      mod_som_io_print_f("sd_format,nak,%s\r\n","wrong arguments");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"sd_format,nak,%s\r\n","wrong arguments");
  }
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }

  return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell to check mod_som_apf_cmd_sd_format_f
 *   set the format of the data stored in the SD card
 *   0 = no sd storage format (latter on called SD0)
 *   1 = raw store everything on the SD card
 *   2 = time pressure epsilon chi fom dpdt kvis avg_t avg_s decimated spectra
 *   3 = time pressure epsilon chi fom dpdt kvis avg_t avg_s full avg spectra
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



mod_som_apf_status_t mod_som_apf_sd_format_status_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[])
{
  mod_som_apf_status_t status = 0;
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;



      status = mod_som_io_print_f("sd_format,ack,%i\r\n",mod_som_apf_ptr->settings_ptr->sd_packet_format);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"sd_format,ack,%i\r\n",mod_som_apf_ptr->settings_ptr->sd_packet_format);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if (status){
      mod_som_io_print_f("sd_format,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"sd_format,nak,%lu\r\n",status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
 }
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }

  return status;
}


/*******************************************************************************
 * @brief
 *   command shell for upload command
 *   start uploading data from the SD card to the apf
 *   should return an apf status.
 *
 *   stream the dacq structure through packets of 990 bytes each
 *
 *   Bytes 1-2  : CRC code
 *   Bytes 3-4  : high 6 bits packet counter,
 *                low 10 bits contains the number of data bytes.
 *   Bytes 5-990: Epsi data(i.e., producer_ptr->acq profile )
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_upload_f(){

  mod_som_status_t status=MOD_SOM_APF_STATUS_OK;

  uint32_t delay=MOD_SOM_APF_UPLOAD_DELAY;

  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  int c;

  uint32_t   timeout = 0;
  uint32_t   dacq_bytes_available = 0;
  uint32_t   dacq_bytes_sent      = 0;
  uint32_t   dacq_bytes_to_sent   = 0;
  uint8_t *  current_data_ptr=
      (uint8_t *) &mod_som_apf_ptr->producer_ptr->acq_profile;
  uint8_t    eot_byte = MOD_SOM_APF_UPLOAD_EOT_BYTE;


  //ALB start transmit the packets
  //ALB check if daq is stopped
  if(!mod_som_apf_ptr->daq){

      //ALB upload cmd received
      //ALB send msg back
      mod_som_io_print_f("upload,ack,start\r\n");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"upload,ack,start\r\n");
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      //ALB wait 500 ms
      while (delay>0){
          delay--;
      }


      //ALB initialize send_packet_tries.
      mod_som_apf_ptr->consumer_ptr->send_packet_tries=0;
      //ALB split acq_profile into 990 bytes packets:
      //ALB count how bytes are left to send out.
      dacq_bytes_available=
          mod_som_apf_ptr->producer_ptr->dacq_size-dacq_bytes_sent;

      //ALB while bytes available > than 986 bytes
      //ALB I build a full packet and stream it.
      while ((dacq_bytes_available>=0) &
          (mod_som_apf_ptr->consumer_ptr->send_packet_tries<
              MOD_SOM_APF_UPLOAD_MAX_TRY_PACKET)){

          //ALB define how many byte to send
          //ALB i.e. min(dacq_bytes_available,
          //ALB          MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE)

          dacq_bytes_to_sent=MIN(dacq_bytes_available,
                                 MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE);

          //ALB end of profile sending the EOT bytes
          if(dacq_bytes_to_sent==0){
              current_data_ptr=&eot_byte;
              dacq_bytes_to_sent=1;
          }
          //ALB copy the dacq bytes in the packet payload structure.
          memcpy(&mod_som_apf_ptr->consumer_ptr->packet.payload,
                 current_data_ptr,
                 dacq_bytes_to_sent);
          //ALB compute the packet CRC
          //TODO
          *((uint16_t *) &mod_som_apf_ptr->consumer_ptr->packet.CRC)=0;
          //ALB compute the counters
          //TODO
          *((uint16_t *) &mod_som_apf_ptr->consumer_ptr->packet.counters)=0;

          //ALB packet should be ready. Now send it
          mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
          mod_som_io_stream_data_f(
              (uint8_t *)&mod_som_apf_ptr->consumer_ptr->packet,
              dacq_bytes_to_sent+MOD_SOM_APF_UPLOAD_PACKET_CRC_SIZE+
              MOD_SOM_APF_UPLOAD_PACKET_CNT_SIZE,
              &mod_som_apf_ptr->consumer_ptr->consumed_flag);

          //ALB Wait for consumed_flag to be on before doing the following lines
          //ALB I think the 5s timeout covers pretty much everything
          while(mod_som_apf_ptr->consumer_ptr->consumed_flag){
              timeout++;
              if (timeout>MOD_SOM_APF_UPLOAD_APF11_TIMEOUT){
                  status=MOD_SOM_APF_UPLOAD_APF11_TIMEOUT;
                  mod_som_apf_ptr->consumer_ptr->consumed_flag=true;
              }//end of timeout
          }//while consumed flag

          //ALB Wait for APF11 answer.
          //ALB APF sends ACK if fine.
          //ALB APF send NACK if not fine.
          //ALB There is also a 5sec time out ~ NACK.
          //ALB If NACK or timeout, try 3 times to send the packet.
          //ALB If packet is NACK after 3 tries, go back to menu (i.e., exit the upload function).
          if(timeout<MOD_SOM_APF_UPLOAD_APF11_TIMEOUT){
              timeout=0;
              c=0;
              while (c < 0){ // Wait for valid input
                  //Release for waiting tasks
                  c = RETARGET_ReadChar();

                  //ALB good transmit, go to next packet.
                  if(c==MOD_SOM_APF_UPLOAD_APF11_ACK){
                      //ALB update current_data_ptr to point to the next dacq data
                      current_data_ptr+=dacq_bytes_to_sent;
                      dacq_bytes_sent+=dacq_bytes_to_sent;
                      mod_som_apf_ptr->consumer_ptr->send_packet_tries=0;
                      status=MOD_SOM_APF_UPLOAD_APF11_ACK;
                  }
                  if(c==MOD_SOM_APF_UPLOAD_APF11_NACK){
                      mod_som_apf_ptr->consumer_ptr->send_packet_tries++;
                      status=MOD_SOM_APF_UPLOAD_APF11_NACK;

                  }
                  //ALB timeout
                  timeout++;
                  if (timeout>MOD_SOM_APF_UPLOAD_APF11_TIMEOUT){
                      mod_som_apf_ptr->consumer_ptr->send_packet_tries++;
                      c=1;//ALB set c>0 to get out of the while loop
                      status=MOD_SOM_APF_UPLOAD_APF11_TIMEOUT;
                  }//ALB end of timeout
              }//ALB end of while c

          }//ALB end of packet stream time out
          //ALB update dacq_bytes_available.
          dacq_bytes_available=
              mod_som_apf_ptr->producer_ptr->dacq_size-dacq_bytes_sent;
      }//ALB end of while bytes available


  }else{
      //ALB daq is still running
      status=MOD_SOM_APF_STATUS_DAQ_IS_RUNNING;
  }//ALB end if daq

  //ALB end of upload send the upload status
  if(status!=MOD_SOM_APF_STATUS_OK){
      mod_som_io_print_f("upload,ak,success\r\n");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"upload,ak,success\r\n");
  }else{
      mod_som_io_print_f("upload,nak,%lu\r\n",status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"upload,nak,%lu\r\n", status);
  }
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }


  return mod_som_apf_encode_status_f(status);
}


#endif


