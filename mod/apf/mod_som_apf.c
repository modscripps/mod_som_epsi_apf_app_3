/*
 * mod_som_apf.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


#include <apf/mod_som_apf.h>
#include <apf/mod_som_apf_bsp.h>
#include <limits.h>

#ifdef MOD_SOM_APF_EN

#include "mod_som_io.h"
#include "math.h"
#include "mod_som_priv.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <apf/mod_som_apf_cmd.h>

#endif

#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif

#define SOM_APF_NOT_DEBUG_MODE 1  // use this flag to turn on main command for debugging: 0: turn off,1: debug -- mai bui 5 May, 2022

#include <efe_obp/mod_som_efe_obp.h>

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;


//ALB add crc16bit functions to compute the crc checksum
#include <crc16bit.h>

#include <ctype.h> // for isalpha
mod_som_apf_ptr_t mod_som_apf_ptr;

// producer task
static CPU_STK mod_som_apf_producer_task_stk[MOD_SOM_APF_PRODUCER_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_producer_task_tcb;

// consumer task
static CPU_STK mod_som_apf_consumer_task_stk[MOD_SOM_APF_CONSUMER_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_consumer_task_tcb;

// apf shell task
static CPU_STK mod_som_apf_shell_task_stk[MOD_SOM_APF_SHELL_TASK_STK_SIZE];
static OS_TCB  mod_som_apf_shell_task_tcb;


static volatile int     apf_rxReadIndex  = 0;       /**< Index in buffer to be read */
static volatile int     apf_rxWriteIndex = 0;       /**< Index in buffer to be written to */
static volatile int     apf_rxCount      = 0;       /**< Keeps track of how much data which are stored in the buffer */
static volatile uint8_t apf_rxBuffer[MOD_SOM_APF_SHELL_STR_LENGTH];    /**< Buffer to store data */

sl_status_t mystatus;

#define MOD_SOM_APF_SD_FORMAT_CMMD_LIMIT 255
#define TIME_MIN 1575205199

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


    LEUART_TypeDef* apf_leuart_ptr;
    char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
    size_t reply_str_len = 0;
//    CPU_CHAR filename[100];


    //ALB gittest#3

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
    //ALB Allocate memory for the producer pointer,
    //ALB using the settings_ptr variable
    //ALB REMEMBER, you need consumer struct even if you do not use the consumer task
    status |= mod_som_apf_construct_consumer_ptr_f();
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
    else{
        uint32_t bytes_sent;

        printf("APF initialized\n");

        apf_leuart_ptr =(LEUART_TypeDef *) mod_som_apf_get_port_ptr_f();
        sprintf(apf_reply_str,"APEX-EPSI initialized\r\n");
        reply_str_len = strlen(apf_reply_str);
        //ALB sending the above string to the APF port -
        sl_sleeptimer_delay_millisecond(10);
        bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
        sl_sleeptimer_delay_millisecond(10);
        if (bytes_sent==0){
            status= MOD_SOM_STATUS_NOT_OK;
        }
    }


    //ALB turn off min com
    //MB add "Debugg" flag
#if SOM_APF_NOT_DEBUG_MODE
    mod_som_main_com_off_f();
#endif

    //ALB initialize mod_som_apf_ptr params
    mod_som_apf_ptr->sleep_flag=0;
    mod_som_apf_ptr->upload_flag=0;
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
mod_som_apf_status_t mod_som_apf_allocate_settings_ptr_f(){

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
mod_som_apf_status_t mod_som_apf_default_settings_f(
                                    mod_som_apf_settings_ptr_t settings_ptr)
{
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  strncpy(settings_ptr->header,
          MOD_SOM_APF_HEADER0,MOD_SOM_APF_SETTINGS_STR_LENGTH);

  settings_ptr->comm_telemetry_packet_format=2;
  settings_ptr->sd_packet_format=2;


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
 *   get the runtime ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_apf_ptr_t mod_som_apf_get_runtime_ptr_f(){
  return mod_som_apf_ptr;
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
mod_som_apf_status_t mod_som_apf_construct_config_ptr_f(){

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

  config_ptr->vibration_cut_off= MOD_SOM_APF_VIBRATION_CUT_OFF;

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
mod_som_apf_status_t mod_som_apf_construct_producer_ptr_f(){

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

//  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
//                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dacq_size    = 0;


  mod_som_apf_ptr->producer_ptr->dacq_ptr =
      (uint8_t*)Mem_SegAlloc(
          "MOD SOM APF producer dacq.",DEF_NULL,
          5*sizeof(float) * mod_som_apf_ptr->producer_ptr->nfft_diag,
          &err);

  mod_som_apf_ptr->producer_ptr->meta_data_buffer_ptr =
      (uint8_t*)Mem_SegAlloc(
          "MOD SOM APF producer meta buffer.",DEF_NULL,
          MOD_SOM_APF_METADATA_SIZE,
          &err);


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
mod_som_apf_status_t mod_som_apf_construct_consumer_ptr_f(){

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
  mod_som_apf_ptr->consumer_ptr->dissrates_cnt = 0;

  mod_som_apf_ptr->consumer_ptr->length_header=
                                             MOD_SOM_APF_SYNC_TAG_LENGTH+
                                             MOD_SOM_APF_HEADER_TAG_LENGTH+
                                             MOD_SOM_APF_HEXTIMESTAMP_LENGTH +
                                             MOD_SOM_APF_SETTINGS_STR_LENGTH+
                                             MOD_SOM_APF_LENGTH_HEADER_CHECKSUM;


  //ALB a smallish array to store data in the consumer strucutre before writting them on the SD card or spitting them out
  mod_som_apf_ptr->consumer_ptr->dacq_ptr =
      (uint8_t*)Mem_SegAlloc(
          "MOD SOM APF consumer dacq.",DEF_NULL,
          3*sizeof(float) * MOD_SOM_EFE_OBP_DEFAULT_NFFT,
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
mod_som_apf_status_t mod_som_apf_construct_com_prf_f(){

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
//        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // Set a reference clock

        //ALB cmuClkDiv_1 works with baudrate 115200. Only for testing
//        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_1);
        //ALB cmuClkDiv_4 works with baudrate 9600. apex mode
        CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1); // Don't prescale LEUART clock
//        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_4);
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
        LEUART_IntClear(leuart_ptr, ~0x0);
        LEUART_IntEnable(leuart_ptr, LEUART_IF_RXDATAV);
        NVIC_EnableIRQ(LEUART0_IRQn);



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
mod_som_apf_status_t mod_som_apf_start_producer_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
  //ALB initialize all parameters. They should be reset right before
  //ALB fill_segment task is starts running.
//  mod_som_apf_ptr->producer_ptr->avg_timestamp        = 0;
  mod_som_apf_ptr->producer_ptr->dissrate_skipped     = 0;
  mod_som_apf_ptr->producer_ptr->dissrates_cnt        = 0;
  mod_som_apf_ptr->producer_ptr->dacq_size            = 0;
  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt = 0;

  //ALB watch out: I do not initialize stored_dissrates_cnt to 0
  //ALB because we could be restarting a profile
  //ALB (case where daq stop was issued but we want to continue the profile i.e.,
  //ALB same profile id).TODO make sure stored_dissrates_cnt when starting a new profile
//  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt =
//      mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.sample_cnt;

  mod_som_apf_ptr->producer_ptr->initialized_flag     = false;
  mod_som_apf_ptr->producer_ptr->collect_flg          = false;
  mod_som_apf_ptr->producer_ptr->dacq_full            = false;

//  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
//      &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

  mod_som_apf_ptr->producer_ptr->started_flg          = true;

  switch (mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format){
    case 0:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
      break;
    case 1:
      //ALB element size = 1549 ~ 16 sample inside the 25kB
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_FOM_SIZE;
      break;
    case 2:
      //ALB element size = 1549 ~ 16 sample inside the 25kB
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_SEAWATER_SPEED_SIZE+
         (MOD_SOM_EFE_OBP_CHANNEL_NUMBER*
          mod_som_apf_ptr->producer_ptr->nfft_diag*
          MOD_SOM_APF_PRODUCER_FOCO_RES);

      break;
  }


  //ALB get a P reading and define the dz to get 25kB in the producer->dacq_profile
  //ALB the dz will depends on the comm_packet_format.
  //TODO
  //ALB I am subtracting sample_cnt because it can be non 0
  //ALB f we re-initialize with OBPdata file.
  mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.sample_cnt =
      floor(MOD_SOM_APF_DACQ_STRUCT_SIZE/
            (MOD_SOM_APF_METADATA_STRUCT_SIZE+
      mod_som_apf_ptr->producer_ptr->dacq_element_size))-
      mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.sample_cnt;

  mod_som_apf_ptr->dacq_start_pressure = mod_som_sbe41_get_pressure_f();
  //ALB the evolving pressure is initialized with the start pressure.
  mod_som_apf_ptr->dacq_pressure=mod_som_apf_ptr->dacq_start_pressure;
  //ALB define dz
  mod_som_apf_ptr->dacq_dz = (mod_som_apf_ptr->dacq_start_pressure-
                              MOD_SOM_APF_DACQ_MINIMUM_PRESSURE)/
                              mod_som_apf_ptr->producer_ptr->
                              mod_som_apf_meta_data.sample_cnt;


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
mod_som_apf_status_t mod_som_apf_stop_producer_task_f(){


  RTOS_ERR err;
  OSTaskDel(&mod_som_apf_producer_task_tcb,
             &err);


  //ALB update the Meta Data sample cnt.
  mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.sample_cnt=
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
mod_som_apf_status_t mod_som_apf_start_consumer_task_f(){


  RTOS_ERR err;
  // Consumer Task

  mod_som_apf_ptr->consumer_ptr->dacq_size=0;
  mod_som_apf_ptr->consumer_ptr->dissrates_cnt = 0;

  mod_som_apf_ptr->consumer_ptr->initialized_flag = true;
  mod_som_apf_ptr->consumer_ptr->started_flg=true;

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
 *   stop shell segment task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_stop_consumer_task_f(){


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
  float    * curr_avg_temperature_ptr;
  float    * curr_avg_salinity_ptr;
  float    * curr_kcut_shear_ptr;
  float    * curr_fcut_temp_ptr;
  float    * curr_epsilon_ptr;
  float    * curr_chi_ptr;
  float    * curr_epsi_fom_ptr;
  float    * curr_chi_fom_ptr;

  // parameters for send commands out to APF - mai - Nov 22, 2021
  uint32_t bytes_sent = 0;
//  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
//  uint32_t reply_str_len = 0;
//  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
//  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


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

          //ALB dissrate_avail<0
          //ALB User stopped efe (daq stop)
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
                  &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure;

//              //ALB check if the updated avg pressure is lower than
//              //ALB the previous pressure + dz.
                //ALB if the com is broken between CTD and epsi. I still want to store the data, flag them and
                //TODO set up a time out
              if (*curr_avg_pressure_ptr<=
                   mod_som_apf_ptr->dacq_pressure-mod_som_apf_ptr->dacq_dz)
                {
              //ALB fake if loop to test the dacq
//              if (dissrate_avail>0)
//                {

                  mod_som_apf_ptr->dacq_pressure=*curr_avg_pressure_ptr;

                  avg_spectra_offset     = (mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD)*
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
                      (uint64_t *) &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp;

                  //ALB update avg pressure
                  curr_avg_dpdt_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt;

                  curr_avg_temperature_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature;

                  curr_avg_salinity_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity;

                  //ALB update the dissrate pointers
                  curr_kcut_shear_ptr =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->kcutoff_shear+
                      dissrate_elmnts_offset;

                  curr_fcut_temp_ptr =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->fcutoff_temp+
                      dissrate_elmnts_offset;

                  curr_epsilon_ptr =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon+
                                    dissrate_elmnts_offset;
                  curr_chi_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi+
                                    dissrate_elmnts_offset;
                  curr_epsi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom+
                                    dissrate_elmnts_offset;
                  curr_chi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom+
                                    dissrate_elmnts_offset;

                  switch (mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format){
                    case 0:
                      mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
                      break;
                    case 1:
                      //ALB convert and store the current dissrate into the MOD format
                      // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      mod_som_apf_ptr->producer_ptr->dacq_size +=
                          mod_som_apf_copy_F1_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_epsi_fom_ptr,
                                                     curr_chi_fom_ptr);
                      break;
                    case 2:
                      //ALB convert and store the current dissrate and FFT into the MOD format
                      // time stamp, pressure, dpdt, t,s, kcutoff, log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      //  log10(FFT_shear),log10(FFT_temp),log10(FFT_accel)
                      mod_som_apf_ptr->producer_ptr->dacq_size +=
                      mod_som_apf_copy_F2_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_avg_temperature_ptr,
                                                     curr_avg_salinity_ptr,
                                                     curr_avg_dpdt_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_kcut_shear_ptr,
                                                     curr_fcut_temp_ptr,
                                                     curr_epsi_fom_ptr,
                                                     curr_chi_fom_ptr,
                                                     curr_temp_avg_spectra_ptr,
                                                     curr_shear_avg_spectra_ptr,
                                                     curr_accel_avg_spectra_ptr);
                      break;
                  }//end switch format

                  //ALB update dacq_size
                  //ALB dacq_size is not used anymore. 09/08/2022
//                  mod_som_apf_ptr->producer_ptr->dacq_ptr=
//                      &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0]+
//                       mod_som_apf_ptr->producer_ptr->dacq_size;

                  mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt++;
                  mod_som_io_print_f("\n apf stored: %lu\r\n ", \
                      (uint32_t)mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt);
                  mod_som_io_print_f("\n epsi stored: %e\r\n ", \
                                     *curr_epsilon_ptr);
                  mod_som_io_print_f("\n chi stored: %e\r\n ", \
                                     *curr_chi_ptr);
                  mod_som_io_print_f("\n kcut stored: %f\r\n ", \
                                     *curr_kcut_shear_ptr);
                  mod_som_io_print_f("\n fcut stored: %f\r\n ", \
                                     *curr_fcut_temp_ptr);
                  mod_som_io_print_f("\n epsi_fom stored: %f\r\n ", \
                                     *curr_epsi_fom_ptr);
                  mod_som_io_print_f("\n chi_fom stored: %f\r\n ", \
                                     *curr_chi_fom_ptr);
              }//end if current P<previous P +dz
              //ALB increment cnsmr count
              mod_som_apf_ptr->producer_ptr->dissrates_cnt++;
              mod_som_io_print_f("\n apf cnt: %lu\r\n ", \
                  (uint32_t)mod_som_apf_ptr->producer_ptr->dissrates_cnt);

              //ALB update dissrate available
              dissrate_avail = mod_som_efe_obp_ptr->sample_count -
                  mod_som_apf_ptr->producer_ptr->dissrates_cnt; //elements available have been produced




              //ALB raise flag and stop producer if profile is full (size(dacq)>=25kB)
              //ALB update the number of sample
              //ALB I should also update mod_som_apf_meta_data.sample_cnt in stop producer task i.e. after
              if ((mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt+1)*
                  mod_som_apf_ptr->producer_ptr->dacq_element_size+
                  sizeof(mod_som_apf_meta_data_t)>=
                  MOD_SOM_APF_DACQ_STRUCT_SIZE)
                {
                  mod_som_apf_ptr->producer_ptr->dacq_full=true;
                  mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.sample_cnt=
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

//  uint8_t * curr_dacq_ptr;

  int dissrate_avail=0, reset_dissrate_cnt=0;
  int dissrate_elmnts_offset=0;
  int avg_spectra_offset=0;

  uint64_t * curr_avg_timestamp_ptr;
  float * curr_temp_avg_spectra_ptr;
  float * curr_shear_avg_spectra_ptr;
  float * curr_accel_avg_spectra_ptr;
  float * curr_avg_pressure_ptr;
  float * curr_avg_temperature_ptr;
  float * curr_avg_salinity_ptr;
  float * curr_avg_dpdt_ptr;
  float * curr_epsilon_ptr;
  float * curr_chi_ptr;
  float * curr_kcut_shear_ptr;
  float * curr_fcut_temp_ptr;
  float * curr_epsi_fom_ptr;
  float * curr_chi_fom_ptr;

//  float * curr_epsi_fom_ptr;
//  float * curr_chi_fom_ptr;
  uint64_t tick;
//  uint8_t * curr_consumer_record_ptr;


  // parameters for send commands out to APF - mai - Nov 22, 2021
  uint32_t bytes_send;
//  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
//  uint32_t reply_str_len = 0;
//  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
//  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  int padding = 1; // the padding should be big enough to include the time variance.

//  char chksum_str[5];

  mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr=
                                          mod_som_efe_obp_get_runtime_ptr_f();

  //MHA: Now augment timestamp by poweron_offset_ms
  mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer


  while (DEF_ON) {

      if (mod_som_apf_ptr->consumer_ptr->started_flg & (mod_som_apf_ptr->settings_ptr->sd_packet_format>=1)){
          /************************************************************************/
          //ALB APF producer phase 1
          //ALB check if producer is started, and if the dacp_profile is NOT full
          dissrate_avail =  mod_som_apf_ptr->producer_ptr->dissrates_cnt -
                            mod_som_apf_ptr->consumer_ptr->dissrates_cnt;  //calculate number of elements available have been produced

              //ALB User stopped efe. I need to reset the obp producers count
              if(dissrate_avail<0){
                  mod_som_apf_ptr->consumer_ptr->dissrates_cnt = 0;
              }
              // LOOP without delay until caught up to latest produced element
              while (dissrate_avail > 0)
                {
//                  curr_dacq_ptr=mod_som_apf_ptr->consumer_ptr->dacq_ptr;
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
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure;
                  //ALB update avg temperature
                  curr_avg_temperature_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature;
                  //ALB update avg salinity
                  curr_avg_salinity_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity;
                  //ALB update avg dpdt
                  curr_avg_dpdt_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt;

                  curr_avg_temperature_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature;

                  curr_avg_salinity_ptr =
                      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity;

                  //ALB update the dissrate pointers
                  curr_kcut_shear_ptr =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->kcutoff_shear+
                      dissrate_elmnts_offset;

                  curr_fcut_temp_ptr =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->fcutoff_temp+
                      dissrate_elmnts_offset;


                  avg_spectra_offset     = (mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD)*
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
                          (uint64_t *) &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp;

                      mod_som_io_print_f("apf consumer timestamp=%lu\r\n",
                                         (uint32_t) mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp);

                      //ALB update the dissrate pointers
                      curr_epsilon_ptr =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon+
                                        dissrate_elmnts_offset;
                      curr_chi_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi+
                                        dissrate_elmnts_offset;
                      curr_epsi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom+
                                        dissrate_elmnts_offset;
                      curr_chi_fom_ptr     =mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom+
                                        dissrate_elmnts_offset;


                      tick=sl_sleeptimer_get_tick_count64();
                      mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                       &mod_som_apf_ptr->consumer_ptr->record_timestamp);

                      mod_som_apf_ptr->consumer_ptr->record_timestamp +=
                                                mod_som_calendar_settings.poweron_offset_ms;


                      mod_som_apf_ptr->consumer_ptr->payload_length=
                          MOD_SOM_APF_CONSUMER_TIMESTAMP_SIZE+
                          (10*MOD_SOM_APF_DACQ_FLOAT_SIZE)+
                          (3*sizeof(float)*mod_som_apf_ptr->producer_ptr->
                              mod_som_apf_meta_data.nfft/2);

                      //ALB create header APF2 (only for SD format)
                      mod_som_apf_header_f(mod_som_apf_ptr->consumer_ptr,2);

//                      uint8_t * dacq_ptr=mod_som_apf_ptr->consumer_ptr->dacq_ptr+
//                          mod_som_apf_ptr->consumer_ptr->length_header;

//                      mod_som_sdio_write_data_f(
//                          mod_som_apf_ptr->consumer_ptr->header,
//                          mod_som_apf_ptr->consumer_ptr->length_header,
//                          &mod_som_apf_ptr->consumer_ptr->consumed_flag);


                      //ALB convert and store the current dissrate and FFT into the MOD format
                      // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
                      //  log10(FFT_shear),log10(FFT_temp),log10(FFT_accel)
                      mod_som_apf_copy_sd_element_f( curr_avg_timestamp_ptr,
                                                     curr_avg_pressure_ptr,
                                                     curr_avg_temperature_ptr,
                                                     curr_avg_salinity_ptr,
                                                     curr_avg_dpdt_ptr,
                                                     curr_epsilon_ptr,
                                                     curr_chi_ptr,
                                                     curr_kcut_shear_ptr,
                                                     curr_fcut_temp_ptr,
                                                     curr_epsi_fom_ptr,
                                                     curr_chi_fom_ptr,
                                                     curr_temp_avg_spectra_ptr,
                                                     curr_shear_avg_spectra_ptr,
                                                     curr_accel_avg_spectra_ptr);

//                      mod_som_apf_ptr->consumer_ptr->stored_dissrates_cnt++;

                      //ALB increment cnsmr count
                      mod_som_apf_ptr->consumer_ptr->dissrates_cnt++;

                      //ALB update dissrate available
                      dissrate_avail =  mod_som_apf_ptr->producer_ptr->dissrates_cnt -
                                        mod_som_apf_ptr->consumer_ptr->dissrates_cnt;  //calculate number of elements available have been produced
                  }//end  while dissrate available



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

/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_apf_header_f(mod_som_apf_consumer_ptr_t consumer_ptr, uint8_t tag_id)
{

  //time stamp
  uint32_t t_hex[2];
  uint8_t * local_header;


  t_hex[0] = (uint32_t) (consumer_ptr->record_timestamp>>32);
  t_hex[1] = (uint32_t) consumer_ptr->record_timestamp;

  //header  contains $EFE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
  switch (tag_id){
    case 0:
      memcpy(consumer_ptr->tag,
              MOD_SOM_APF_HEADER0,
              MOD_SOM_APF_TAG_LENGTH);
      break;
    case 1:
      memcpy(consumer_ptr->tag,
              MOD_SOM_APF_HEADER1,
              MOD_SOM_APF_TAG_LENGTH);
      break;
    case 2:
      memcpy(consumer_ptr->tag,
              MOD_SOM_APF_HEADER2,
              MOD_SOM_APF_TAG_LENGTH);
      break;
  }

  sprintf((char*) consumer_ptr->header,  \
      "$%.4s%08x%08x%08x*FF", \
      consumer_ptr->tag, \
      (int) t_hex[0],\
      (int) t_hex[1],\
      (int) consumer_ptr->payload_length);

  consumer_ptr->header_chksum=0;
  for(int i=0;i<consumer_ptr->length_header-
               MOD_SOM_EFE_OBP_LENGTH_HEADER_CHECKSUM;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
    {
      consumer_ptr->header_chksum ^=\
          consumer_ptr->header[i];
    }

  // the curr_consumer_element_ptr should be at the right place to
  // write the checksum already
  //write checksum at the end of the steam block (record).
  local_header = &consumer_ptr->header[consumer_ptr->length_header-
                                       MOD_SOM_APF_LENGTH_HEADER_CHECKSUM+1];
  *((uint16_t*)local_header) = \
      mod_som_int8_2hex_f(consumer_ptr->header_chksum);

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
  char     input_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE+1];
  char     output_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE];
  char     apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE];
  uint32_t bytes_sent;
  uint32_t reply_str_len;
  uint32_t input_buf_len;


  mod_som_apf_status_t status=0;
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

      // SN ensure string termination
      input_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE]='\0';

      // send "NAK,<expression>\r\n"
      if (status > 0) // if no bytes:
      {
          sprintf(apf_reply_str,"nak,%s\r\n",output_buf);
          reply_str_len = strlen(apf_reply_str);
          // sending the above string to the APF port - Mai - Nov 18, 2021
          bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      }else{
          //ALB we got a cmd comming in
          if (!Str_Cmp(input_buf, "exit"))
            {
              break;
            }
          // we have the whole string, we would convert the string to the right format string we want
          //ALB changing convert string so it can read everything no only char
          //ALB I need input_buffer_length
          status = mod_som_apf_convert_string_f(input_buf,&input_buf_len, output_buf);   // convert the input string to the right format: cap -> uncap, coma -> space
          status = mod_som_shell_execute_input_f(output_buf,input_buf_len);   // execute the appropriate routine base on the command. Return the mod_som_status


          if ((status>0) & !mod_som_apf_ptr->sleep_flag){
              sprintf(apf_reply_str,"nak,%s\r\n",output_buf);
              reply_str_len = strlen(apf_reply_str);
              // sending the above string to the APF port - Mai - Nov 18, 2021
              bytes_sent = mod_som_apf_send_line_f (apf_leuart_ptr,apf_reply_str, reply_str_len);
              if(bytes_sent==0){
                  //TODO handle the error
              }
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
uint32_t mod_som_apf_convert_string_f(char* data_str_input, uint32_t * bytes_received, char* data_str_output)
{
    uint32_t retval = 0;
    char* local_str_input_ptr  = data_str_input;
    char* local_str_output_ptr = data_str_output;

    while(*local_str_input_ptr!='\0')
    {
        // if the character is uppercase -> change to lowercase
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
        else if(*local_str_input_ptr==' ')
          {
            //CAP adding hyphen instead of space when reading the input

            printf("outputstr: %c\n", *local_str_output_ptr);
            *local_str_output_ptr = '-';
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
mod_som_apf_status_t mod_som_apf_shell_execute_input_f(char* input,uint32_t input_len){

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
mod_som_apf_status_t mod_som_apf_shell_get_line_f(char *buf, uint32_t * bytes_read){
  mod_som_apf_status_t status=0;
  RTOS_ERR err;

  static sl_sleeptimer_timestamp_t time0=0;
  sl_sleeptimer_timestamp_t        time1=0;

    int32_t i=0;
    int read_char=-1;
    LEUART_TypeDef  *apf_leuart_ptr;

    Mem_Set(buf, '\0', MOD_SOM_SHELL_INPUT_BUF_SIZE); // Clear previous input

    for (i=0;i<MOD_SOM_SHELL_INPUT_BUF_SIZE;i++)
      buf[i] = '\0';
    // get the fd of LEUART port
    apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

    //ALB Clear RX buffer
    //TODO this hardware dependent.
    //TODO make it not hardware dependent
    apf_leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;


    // use for to read input from APF until getting '\r' character
    // it would be ended either reach the max characters need to read or '\r'


    for (i=0;i<MOD_SOM_SHELL_INPUT_BUF_SIZE;i++)
    {
//        if(i==0){
//            //TODO add a checker for that cmd
//            time0= mod_som_calendar_get_time_f();
//        }

        status = mod_som_apf_get_char_f(apf_leuart_ptr, &read_char); // call for getting char from LEUART

        //ALB while status > 0  we do not haver a bytes
        time0= mod_som_calendar_get_time_f();
        while(status>0){
            OSTimeDly(
                    (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                    (OS_OPT      )OS_OPT_TIME_DLY,
                    &err);
            APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
            time1= mod_som_calendar_get_time_f();
            status = mod_som_apf_get_char_f(apf_leuart_ptr, &read_char); // call for getting char from LEUART
            if((time1-time0)>MOD_SOM_APF_SHELL_TIMEOUT){
                i=5;
                memcpy(buf,"sleep",i);
                read_char = '\r';
                status=0;
                break;
            }

        }

        if (read_char == '\r')
        {
//            buf[i] = '\0';
            break;
        }
        //ALB check for printable characters (excluding space character ' '=32)
        else if (!(read_char>32 && read_char<127)){ // check for printable characters
                    i--;
                    continue;
                }
    buf[i] = read_char;// save the read character into the buffer
    }
    //ALB add check on the buffer length
     if (i==MOD_SOM_SHELL_INPUT_BUF_SIZE){
         status= MOD_SOM_APF_STATUS_BUFFER_OVFLW;
     }else{
         buf[i] = '\0';
         *bytes_read = i;
     }
//    buf[i] = '\0';
//    *bytes_read = i;
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
mod_som_status_t mod_som_apf_get_char_f(LEUART_TypeDef *leuart_ptr, int* read_char_ptr)
{
  //Get one bytes from the select port
//  *read_char = LEUART_Rx(leuart_ptr);
  mod_som_status_t status=1;
  *read_char_ptr=-1;
//  int c = -1;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (apf_rxCount > 0) {
    *read_char_ptr = (int) apf_rxBuffer[apf_rxReadIndex];
    apf_rxReadIndex++;
    if (apf_rxReadIndex == MOD_SOM_APF_SHELL_STR_LENGTH) {
      apf_rxReadIndex = 0;
    }
    apf_rxCount--;
    /* Unconditionally enable the RX interrupt. RX interrupts are disabled when
     * a buffer full condition is entered. This way flow control can be handled
     * automatically by the hardware. */
    LEUART_IntEnable(leuart_ptr, LEUART_IF_RXDATAV);
  }

  CORE_EXIT_ATOMIC();

//ALB if read char is a byte chnage status to OK (i.e., 0);
if (*read_char_ptr>0){
  status=0;
}

  return status;
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

///*******************************************************************************
// * @brief
// *   Send a line from a PORT.
// *
// *
// * @param buf
// *   Buffer to hold the input string.
// * @param buf_length
// *  Length of buffer as the user is typing
// *  edit: Nov 19, 2021
// ******************************************************************************/
//uint32_t mod_som_apf_make_nack_apf_reply_f(uint8_t * apf_reply_str,char * cmd1, uint32_t status)
//{
//  size_t reply_str_len = 0;
//
//  sprintf((char *) apf_reply_str,"%s,nack",cmd1);
//  reply_str_len = strlen((char*)apf_reply_str);
//
//  memcpy(&apf_reply_str[reply_str_len],",",sizeof(char));
//  reply_str_len++;
//  memcpy(&apf_reply_str[reply_str_len],&status,sizeof(uint32_t));
//  reply_str_len+=sizeof(uint32_t);
//  memcpy(&apf_reply_str[reply_str_len],"\r",sizeof(char));
//  reply_str_len++;
//  memcpy(&apf_reply_str[reply_str_len],"\n",sizeof(char));
//  reply_str_len++;
//
//  return reply_str_len;
//
//}



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
//uint32_t mod_som_apf_copy_F1_element_f(  uint64_t * curr_avg_timestamp_ptr,
//                                    float * curr_avg_pressure_ptr,
//                                    float * curr_epsilon_ptr,
//                                    float * curr_chi_ptr,
//                                    float * curr_fom_epsi_ptr,
//                                    float * curr_fom_chi_ptr,
//                                    uint8_t * dacq_ptr)
//{
  uint32_t mod_som_apf_copy_F1_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                      float * curr_avg_pressure_ptr,
                                      float * curr_epsilon_ptr,
                                      float * curr_chi_ptr,
                                      float * curr_fom_epsi_ptr,
                                      float * curr_fom_chi_ptr)
  {


  uint32_t dacq_size=0;
  uint32_t mod_epsilon, mod_chi;
  uint8_t mod_epsi_fom,mod_chi_fom;
  uint16_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq
  FRESULT res=0;


  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
      mod_som_sdio_get_runtime_ptr_f();

  mod_som_sdio_file_ptr_t processfile_ptr =
      local_mod_som_sdio_ptr_t->processdata_file_ptr;

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
  local_avg_dissrate_timestamp =(uint16_t)((*curr_avg_timestamp_ptr -
      (uint64_t)(mod_som_apf_ptr->producer_ptr->
                                   mod_som_apf_meta_data.daq_timestamp)*1000)/1000);


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


//  memcpy(dacq_ptr,
//         &local_avg_dissrate_timestamp,
//         MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
//  dacq_ptr+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;

//  memcpy(dacq_ptr,
//         curr_avg_pressure_ptr,
//         MOD_SOM_APF_DACQ_PRESSURE_SIZE);
//  dacq_ptr+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
//  memcpy(dacq_ptr,
//         &mod_bit_dissrates,
//         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
//  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
//  memcpy(dacq_ptr,
//         &mod_bit_fom,
//         MOD_SOM_APF_PRODUCER_FOM_RES);
//
//  dacq_size=dacq_ptr-&mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  mod_som_sdio_write_data_f(processfile_ptr,
                                 (uint8_t*) &local_avg_dissrate_timestamp,
                                 MOD_SOM_APF_DACQ_TIMESTAMP_SIZE,
                                 &mod_som_apf_ptr->producer_ptr->done_sd_flag);
  dacq_size+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
  while(!mod_som_apf_ptr->producer_ptr->done_sd_flag){};
  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
mod_som_sdio_write_data_f(processfile_ptr,
                                 (uint8_t*) curr_avg_pressure_ptr,
                                 MOD_SOM_APF_DACQ_PRESSURE_SIZE,
                                 &mod_som_apf_ptr->producer_ptr->done_sd_flag);
  dacq_size+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
  while(!mod_som_apf_ptr->producer_ptr->done_sd_flag){};
  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  mod_som_sdio_write_data_f(processfile_ptr,
                                 (uint8_t*) &mod_bit_dissrates,
                                 MOD_SOM_APF_PRODUCER_DISSRATE_RES,
                                 &mod_som_apf_ptr->producer_ptr->done_sd_flag);
  dacq_size+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  while(!mod_som_apf_ptr->producer_ptr->done_sd_flag){};
  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  mod_som_sdio_write_data_f(processfile_ptr,
                                 (uint8_t*) &mod_bit_fom,
                                 MOD_SOM_APF_PRODUCER_FOM_RES,
                                 &mod_som_apf_ptr->producer_ptr->done_sd_flag);
  while(!mod_som_apf_ptr->producer_ptr->done_sd_flag){};
  dacq_size+=MOD_SOM_APF_PRODUCER_FOM_RES;

  if(res!=FR_OK){
      return -1;
  }
  return dacq_size;
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
//uint32_t mod_som_apf_copy_F2_element_f(  uint64_t * curr_avg_timestamp_ptr,
//                                float * curr_avg_pressure_ptr,
//                                float * curr_temperature_ptr,
//                                float * curr_salinity_ptr,
//                                float * curr_avg_dpdt_ptr,
//                                float * curr_epsilon_ptr,
//                                float * curr_chi_ptr,
//                                float * curr_kcut_shear_ptr,
//                                float * curr_fcut_temp_ptr,
//                                float * curr_fom_epsi_ptr,
//                                float * curr_fom_chi_ptr,
//                                float * curr_temp_avg_spectra_ptr,
//                                float * curr_shear_avg_spectra_ptr,
//                                float * curr_accel_avg_spectra_ptr,
//                                uint8_t * dacq_ptr)
//{
  uint32_t mod_som_apf_copy_F2_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                  float * curr_avg_pressure_ptr,
                                  float * curr_temperature_ptr,
                                  float * curr_salinity_ptr,
                                  float * curr_avg_dpdt_ptr,
                                  float * curr_epsilon_ptr,
                                  float * curr_chi_ptr,
                                  float * curr_kcut_shear_ptr,
                                  float * curr_fcut_temp_ptr,
                                  float * curr_fom_epsi_ptr,
                                  float * curr_fom_chi_ptr,
                                  float * curr_temp_avg_spectra_ptr,
                                  float * curr_shear_avg_spectra_ptr,
                                  float * curr_accel_avg_spectra_ptr)
  {

  //ALB declare the local parameters
  uint32_t mod_epsilon, mod_chi;
  uint16_t mod_shear_foco, mod_temp_foco, mod_accel_foco;
  uint8_t mod_epsi_fom,mod_chi_fom;
  uint8_t  mod_bit_fom;

  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
      mod_som_sdio_get_runtime_ptr_f();

  mod_som_sdio_file_ptr_t processfile_ptr =
      local_mod_som_sdio_ptr_t->processdata_file_ptr;


  uint16_t  local_avg_dissrate_timestamp; //ALB nb of sec since dacq
  uint8_t   mod_bit_dissrates[MOD_SOM_APF_PRODUCER_DISSRATE_RES] = {0};
  uint32_t  dacq_size=0;
  uint32_t  dacq_size1=0;
  uint32_t  dacq_size2=0;
  uint8_t * dacq_ptr=mod_som_apf_ptr->producer_ptr->dacq_ptr;


  //ALB store the dissrate and foco values in local params.
  float local_epsilon        = log10(*curr_epsilon_ptr);
  float local_chi            = log10(*curr_chi_ptr);
  float local_shear_avg_fft  = log10(*curr_shear_avg_spectra_ptr);
  float local_temp_avg_fft   = log10(*curr_temp_avg_spectra_ptr);
  float local_accel_avg_fft  = log10(*curr_accel_avg_spectra_ptr);
  float local_epsi_fom       = *curr_fom_epsi_ptr;
  float local_chi_fom        = *curr_fom_chi_ptr;




  //ALB store the max min dissrate and foco values in local params.
  //ALB TODO this could be done with macro and not waste CPU time
  float min_dissrate = log10(MOD_SOM_APF_PRODUCER_MIN_DISSRATE);
  float max_dissrate = log10(MOD_SOM_APF_PRODUCER_MAX_DISSRATE);
  float min_foco     = log10(MOD_SOM_APF_PRODUCER_MIN_FOCO);
  float max_foco     = log10(MOD_SOM_APF_PRODUCER_MAX_FOCO);
  float min_fom      = MOD_SOM_APF_PRODUCER_MIN_FOM;
  float max_fom      = MOD_SOM_APF_PRODUCER_MAX_FOM;


  //ALB decimate timestamps and store it
  //ALB dissrate timestamp - dacq start timestamp -> 2bytes (65000 sec)
  local_avg_dissrate_timestamp =(uint16_t)((*curr_avg_timestamp_ptr -
      (uint64_t)(mod_som_apf_ptr->producer_ptr->
          mod_som_apf_meta_data.daq_timestamp)*1000)/1000);

  //ALB decimate dissrates,
  //ALB first make sure it is above the min values
  local_epsilon = MAX(local_epsilon,min_dissrate);
  local_chi     = MAX(local_chi,min_dissrate);

  //ALB then make sure it is below the max values
  local_epsilon = MIN(local_epsilon,max_dissrate);
  local_chi     = MIN(local_chi,max_dissrate);

  //ALB min out the local fom to 0
  local_epsi_fom     = MAX(local_epsi_fom,min_fom);
  local_chi_fom      = MAX(local_chi_fom,min_fom);
  //ALB max out the local fom to 10
  local_epsi_fom     = MIN(local_epsi_fom,max_fom);
  local_chi_fom      = MIN(local_chi_fom,max_fom);


  //ALB then digitize on a 12bits number
  mod_epsilon  = (uint32_t) ceil(local_epsilon*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  mod_chi      = (uint32_t) ceil(local_chi*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);


  //ALB then store epsi and chi in a 3 bytes array
  mod_bit_dissrates[0]= (uint8_t) (mod_epsilon>>4);
  mod_bit_dissrates[1]= (uint8_t) (mod_epsilon << 4);
  mod_bit_dissrates[1]= (uint8_t) (mod_bit_dissrates[1] | (mod_chi>>8));
  mod_bit_dissrates[2]= (uint8_t) mod_chi;

  //ALB decimate fom with 8 bits (1 bytes)
  mod_epsi_fom  = (uint8_t) ceil(local_epsi_fom*
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;
  mod_chi_fom   = (uint8_t) ceil(local_chi_fom*
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;

  //ALB bit shifting to build mod_bit_fom.
  mod_bit_fom = (mod_epsi_fom<<4) + mod_chi_fom;


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
         curr_temperature_ptr,
         MOD_SOM_APF_DACQ_TEMPERATURE_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_TEMPERATURE_SIZE;
  memcpy(dacq_ptr,
         curr_salinity_ptr,
         MOD_SOM_APF_DACQ_SALINITY_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_SALINITY_SIZE;
  memcpy(dacq_ptr,
         curr_avg_dpdt_ptr,
         MOD_SOM_APF_DACQ_DPDT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_DPDT_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_dissrates,
         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  memcpy(dacq_ptr,
         curr_kcut_shear_ptr,
         MOD_SOM_APF_DACQ_KCUT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_KCUT_SIZE;
  memcpy(dacq_ptr,
         curr_fcut_temp_ptr,
         MOD_SOM_APF_DACQ_FCUT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FCUT_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_fom,
         MOD_SOM_APF_PRODUCER_FOM_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_FOM_RES;

  dacq_size1=dacq_ptr-mod_som_apf_ptr->producer_ptr->dacq_ptr;

  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  mod_som_sdio_write_data_f(processfile_ptr,
                            mod_som_apf_ptr->producer_ptr->dacq_ptr,
                            dacq_size1,
                            &mod_som_apf_ptr->producer_ptr->done_sd_flag);

//  res|=mod_som_sdio_write_processdata_f((uint8_t*) &local_avg_dissrate_timestamp,
//                                   MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) curr_avg_pressure_ptr,
//                                   MOD_SOM_APF_DACQ_PRESSURE_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_PRESSURE_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) curr_temperature_ptr,
//                                        MOD_SOM_APF_DACQ_TEMPERATURE_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_TEMPERATURE_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) curr_salinity_ptr,
//                                        MOD_SOM_APF_DACQ_SALINITY_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_SALINITY_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) curr_avg_dpdt_ptr,
//                                        MOD_SOM_APF_DACQ_DPDT_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_DPDT_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) curr_avg_dpdt_ptr,
//                                        MOD_SOM_APF_DACQ_DPDT_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_DPDT_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) &mod_bit_dissrates,
//                                   MOD_SOM_APF_PRODUCER_DISSRATE_RES);
//  dacq_size+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) &curr_kcut_shear_ptr,
//                                        MOD_SOM_APF_DACQ_KCUT_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_KCUT_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) &curr_fcut_temp_ptr,
//                                        MOD_SOM_APF_DACQ_FCUT_SIZE);
//  dacq_size+=MOD_SOM_APF_DACQ_FCUT_SIZE;
//  res|=mod_som_sdio_write_processdata_f((uint8_t*) &mod_bit_fom,
//                                        MOD_SOM_APF_PRODUCER_FOM_RES);
//  dacq_size+=MOD_SOM_APF_PRODUCER_FOM_RES;
//


  dacq_ptr=mod_som_apf_ptr->producer_ptr->dacq_ptr;

  //ALB decimate fourier coef (foco),
  for (int i=0;i<mod_som_apf_ptr->producer_ptr->nfft_diag;i++)
    {
      //ALB first make sure it is above the min/max values
      local_shear_avg_fft = MAX(local_shear_avg_fft,min_foco);
      local_shear_avg_fft = MIN(local_shear_avg_fft,max_foco);

//      local_shear_avg_fft = -5.1;
      //ALB then digitize on a 16bits number
      mod_shear_foco  = (uint16_t) ceil(local_shear_avg_fft*
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_per_bit+
               mod_som_apf_ptr->producer_ptr->decim_coef.foco_counts_at_origin);

      //ALB first make sure it is above the min/max values
      local_temp_avg_fft = MAX(local_temp_avg_fft,min_foco);
      local_temp_avg_fft = MIN(local_temp_avg_fft,max_foco);
//      local_temp_avg_fft = -6.2;
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
  dacq_size2=dacq_ptr-mod_som_apf_ptr->producer_ptr->dacq_ptr;
  while(!mod_som_apf_ptr->producer_ptr->done_sd_flag){};
  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  mod_som_sdio_write_data_f(processfile_ptr,
                            mod_som_apf_ptr->producer_ptr->dacq_ptr,
                            dacq_size2,
                            &mod_som_apf_ptr->producer_ptr->done_sd_flag);
  dacq_size=dacq_size1+dacq_size2;

  return dacq_size;

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
                                     float * curr_avg_temperature_ptr,
                                     float * curr_avg_salinity_ptr,
                                     float * curr_avg_dpdt_ptr,
                                     float * curr_epsilon_ptr,
                                     float * curr_chi_ptr,
                                     float * curr_kcut_shear_ptr,
                                     float * curr_fcut_temp_ptr,
                                     float * curr_fom_epsi_ptr,
                                     float * curr_fom_chi_ptr,
                                     float * curr_temp_avg_spectra_ptr,
                                     float * curr_shear_avg_spectra_ptr,
                                     float * curr_accel_avg_spectra_ptr)
{

  uint8_t * dacq_ptr=mod_som_apf_ptr->consumer_ptr->dacq_ptr;
  uint32_t payload_length=0;
  uint32_t spectra_length=0;
//  char chksum_str[5];
  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
      mod_som_sdio_get_runtime_ptr_f();

  mod_som_sdio_file_ptr_t rawfile_ptr =
      local_mod_som_sdio_ptr_t->rawdata_file_ptr;


  //ALB declare the local parameters
//  uint64_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq

  //ALB copy the data in the acq profile structure
  //ALB TODO check the dacq_ptr update and its value when out of that function
  memcpy(dacq_ptr,
         mod_som_apf_ptr->consumer_ptr->header,
         mod_som_apf_ptr->consumer_ptr->length_header);
  dacq_ptr+=mod_som_apf_ptr->consumer_ptr->length_header;
  memcpy(dacq_ptr,
         curr_avg_timestamp_ptr,
         MOD_SOM_APF_CONSUMER_TIMESTAMP_SIZE);
  dacq_ptr+=MOD_SOM_APF_CONSUMER_TIMESTAMP_SIZE;
  memcpy(dacq_ptr,
         curr_avg_pressure_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_avg_temperature_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_avg_salinity_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_avg_dpdt_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_epsilon_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_chi_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_kcut_shear_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_fcut_temp_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_fom_epsi_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;
  memcpy(dacq_ptr,
         curr_fom_chi_ptr,
         MOD_SOM_APF_DACQ_FLOAT_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_FLOAT_SIZE;


  //ALB Now I am using payload_length just to get the size a 1 spectrum in uint8_t
  spectra_length=sizeof(float)*mod_som_apf_ptr->producer_ptr->
      mod_som_apf_meta_data.nfft/2;

  memcpy(dacq_ptr,
         curr_shear_avg_spectra_ptr,
         spectra_length);
  dacq_ptr+=spectra_length;
//  mod_som_io_stream_data_f(
//      dacq_ptr-payload_length,
//      20,
//      &mod_som_apf_ptr->consumer_ptr->consumed_flag);
  memcpy(dacq_ptr,
         curr_temp_avg_spectra_ptr,
         spectra_length);
  dacq_ptr+=spectra_length;
//  mod_som_io_stream_data_f(
//      dacq_ptr-payload_length,
//      payload_length,
//      &mod_som_apf_ptr->consumer_ptr->consumed_flag);
  memcpy(dacq_ptr,
         curr_accel_avg_spectra_ptr,
         spectra_length);
dacq_ptr+=spectra_length;
//  mod_som_io_stream_data_f(
//      dacq_ptr-payload_length,
//      payload_length,
//      &mod_som_apf_ptr->consumer_ptr->consumed_flag);

  //ALB get the size of the data we just filled up.
  payload_length= dacq_ptr -
                  mod_som_apf_ptr->consumer_ptr->dacq_ptr-
                  mod_som_apf_ptr->consumer_ptr->length_header;

  for(int i=0;i<payload_length;i++)
    {
      mod_som_apf_ptr->consumer_ptr->chksum ^=\
          mod_som_apf_ptr->consumer_ptr->dacq_ptr[i+mod_som_apf_ptr->consumer_ptr->length_header];
    }

  //ALB add checksum
  sprintf((char *)dacq_ptr,"*%x\r\n",mod_som_apf_ptr->consumer_ptr->chksum);
  dacq_ptr+=MOD_SOM_APF_CONSUMER_CHECKSUM_SIZE;
  //ALB update the size of the data we just filled up.
  payload_length= dacq_ptr - mod_som_apf_ptr->consumer_ptr->dacq_ptr;


  mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
  mod_som_sdio_write_data_f(rawfile_ptr,
                            mod_som_apf_ptr->consumer_ptr->dacq_ptr,
                            payload_length,
                            &mod_som_apf_ptr->consumer_ptr->consumed_flag);
  while(!mod_som_apf_ptr->consumer_ptr->consumed_flag){};



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
  mod_som_apf_status_t status=0;
  uint32_t delay1s=1000;
  uint32_t delay2s=2000;
    uint32_t delay10ms=10;
  uint32_t get_ctd_count=0;
  CPU_CHAR filename[100];
  uint32_t file_status;

  if(mod_som_apf_ptr->daq){
      status= MOD_SOM_APF_STATUS_DAQ_ALREADY_STARTED;
      mod_som_apf_daq_stop_f();
  }

  status=MOD_SOM_APF_STATUS_OK;

  mod_som_voltage_ptr_t local_voltage_runtime_ptr =
      mod_som_voltage_get_runtime_ptr_f();

  mod_som_sbe41_ptr_t local_sbe41_runtime_ptr =
      mod_som_sbe41_get_runtime_ptr_f();

  //ALB start collecting CTD.
  status |= mod_som_sbe41_connect_f();
  status |= mod_som_sbe41_start_collect_data_f();

  ////  //ALB enable SDIO hardware
  mod_som_sdio_enable_hardware_f();

  sl_sleeptimer_delay_millisecond(delay2s);


  //ALB I am getting a pressure sample
  while(local_sbe41_runtime_ptr->consumer_ptr->record_pressure[1]==0){
      sl_sleeptimer_delay_millisecond(delay10ms);
      get_ctd_count++;
      //ALB SBE41 is 1Hz 300 counts x 10 ms is 3 seconds
      if (get_ctd_count>300){
          //ALB no CTD sample. Status no CTD data
          status|=MOD_SOM_APF_STATUS_NO_CTD_DATA;
          get_ctd_count=0;
          break;
      }
  }


  if (status==MOD_SOM_APF_STATUS_OK){
      uint32_t obpfile_size;

      //ALB initialize Meta_Data Structure, TODO
      mod_som_apf_ptr->profile_id=profile_id;



      //ALB Always open raw SD file, in append write mode
      sprintf(filename, "Profile%lu",(uint32_t) mod_som_apf_ptr->profile_id);
      mod_som_sdio_define_filename_f(filename);
      mod_som_settings_save_settings_f();

      //ALB Open on board processed data file:
      //ALB If this is a new profile a new file is created because
      //ALB the previous upload would have removed (~sdio.rm) it.
      //ALB If we continue to a profile but the SOM had to restart for some reasons
      //ALB If need to append the new data.
      file_status=mod_som_sdio_open_processfilename_f("OBPdata");
      mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
          mod_som_sdio_get_runtime_ptr_f();

      mod_som_sdio_file_ptr_t processfile_ptr =
          local_mod_som_sdio_ptr_t->processdata_file_ptr;

      obpfile_size=f_size(processfile_ptr->fp);
      //ALB obpfile_size>0 means there is already an OBP file.
      if(obpfile_size>0){
        //ALB read the metadata from the OBP file
        status=mod_som_sdio_read_OBPfile_metadata(processfile_ptr);
      }else{
          mod_som_apf_ptr->producer_ptr->
                    mod_som_apf_meta_data.profile_id=0;
      }
      //ALB As of now a new daq erase the previous data (consistent with the specs)
      //ALB Be careful if we want to keep the the previous data we need to keep
      //ALB sample count, profile id and so on consistent.

      //ALB Metadata is filled up if OBPdata file already exist on the SD card.
      //ALB during mod_som_sdio_open_processfilename_f.
      //ALB now I compare mod_som_apf_meta_data.profile_id!=profile_id
      //ALB to know if this a new new profile or if we continue profile.
      if (mod_som_apf_ptr->producer_ptr->
          mod_som_apf_meta_data.profile_id!=profile_id){
//          FRESULT res;

          mod_som_apf_ptr->producer_ptr->stored_dissrates_cnt=0;
          mod_som_apf_init_meta_data(&mod_som_apf_ptr->producer_ptr->
                                     mod_som_apf_meta_data);
          f_lseek (processfile_ptr->fp, 0);

          mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
          mod_som_sdio_write_data_f(processfile_ptr,
                                    (uint8_t*) &mod_som_apf_ptr->producer_ptr->
                                    mod_som_apf_meta_data,
                                    sizeof(mod_som_apf_meta_data_t),
                                    &mod_som_apf_ptr->producer_ptr->done_sd_flag);


      }


      if (file_status>0){
          status|=file_status;
      }
      //ALB start ADC master clock timer
      mod_som_apf_ptr->daq=true;



    //ALB start turbulence processing task
     status|= mod_som_efe_obp_start_fill_segment_task_f();
     status|= mod_som_efe_obp_start_cpt_spectra_task_f();
     status|= mod_som_efe_obp_start_cpt_dissrate_task_f();
     status|= mod_som_efe_obp_start_consumer_task_f();




    //  //ALB start APF producer task
      status |= mod_som_apf_start_producer_task_f();
    ////  //ALB start APF consumer task
      status |= mod_som_apf_start_consumer_task_f();

    //ALB get the voltage at the beginning of the profile
      mod_som_voltage_scan_f();
      while(local_voltage_runtime_ptr->voltage==0){};
      mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data.voltage=
          local_voltage_runtime_ptr->voltage;
      //ALB reset voltage
      local_voltage_runtime_ptr->voltage=0;

      //ALB      DC/DC not burst mode  PF10 high
//      GPIO_PinModeSet(MOD_SOM_MAIN_COM_EN_PORT, MOD_SOM_MAIN_COM_EN_PIN,
//                      gpioModePushPull, 0);


      status|=mod_som_efe_sampling_f();
  }

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
  int delay =100; //0.1 sec
//  FRESULT res=0;
  mod_som_apf_status_t status;
  status=MOD_SOM_APF_STATUS_OK;

  // from the spec, it
	// stop ADC master clock timer
  status|= mod_som_efe_stop_sampling_f();

  // stop collecting CTD data
  status|= mod_som_sbe41_stop_collect_data_f();
  status|= mod_som_sbe41_disconnect_f();

	// stop turbulence processing task
  status = mod_som_efe_obp_stop_fill_segment_task_f();
  status|= mod_som_efe_obp_stop_cpt_spectra_task_f();
  status|= mod_som_efe_obp_stop_cpt_dissrate_task_f();
  status|= mod_som_efe_obp_stop_consumer_task_f();

  //ALB stop APF producer task
  status |= mod_som_apf_stop_producer_task_f();
  //ALB stop APF consumer task
  status |= mod_som_apf_stop_consumer_task_f();

//ALB update metadata in the OBPdata.modraw file
  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
      mod_som_sdio_get_runtime_ptr_f();

  mod_som_sdio_file_ptr_t processfile_ptr =
      local_mod_som_sdio_ptr_t->processdata_file_ptr;

  mod_som_apf_ptr->producer_ptr->done_sd_flag=false;
  //ALB place the idx at the beginning of the file
  f_lseek (processfile_ptr->fp, 0);

  //ALB write new updated metadata on the SD
  //This is the only place where I update metadata_sample_cnt.

  //if sample_cnt is wrong we still can get it with simple math afterwork.
  mod_som_sdio_write_data_f(processfile_ptr,
                            (uint8_t*) &mod_som_apf_ptr->producer_ptr->
                            mod_som_apf_meta_data,
                            sizeof(mod_som_apf_meta_data_t),
                            &mod_som_apf_ptr->producer_ptr->done_sd_flag);


  //ALB disable SDIO hardware
  status |=mod_som_sdio_stop_store_f();
  sl_sleeptimer_delay_millisecond(delay);
  mod_som_sdio_disable_hardware_f();
  mod_som_sdio_stop_f();



  //reset Daq flags
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
void mod_som_apf_init_meta_data(mod_som_apf_meta_data_ptr_t mod_som_apf_meta_data_ptr)
{

  mod_som_efe_obp_ptr_t local_efe_obp=mod_som_efe_obp_get_runtime_ptr_f();
  mod_som_settings_struct_ptr_t local_settings_ptr=
                                              mod_som_settings_get_settings_f();

  mod_som_apf_meta_data_ptr->nfft=local_efe_obp->settings_ptr->nfft;
  mod_som_apf_meta_data_ptr->nfftdiag=local_efe_obp->settings_ptr->nfft/MOD_SOM_APF_DACQ_F3_NFFT_DECIM_COEF;
  mod_som_apf_meta_data_ptr->comm_telemetry_packet_format=
      mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format;
  mod_som_apf_meta_data_ptr->sd_format=
      mod_som_apf_ptr->settings_ptr->sd_packet_format;

  mod_som_apf_meta_data_ptr->efe_sn=
                      strtol(local_efe_obp->efe_settings_ptr->sn,NULL,10);
  mod_som_apf_meta_data_ptr->firmware_rev=
                      strtol(local_settings_ptr->gitid, NULL, 16);
  mod_som_apf_meta_data_ptr->modsom_sn=
                      strtol(local_settings_ptr->sn,NULL,10);

  //ALB TODO local_efe_obp->efe_settings_ptr->sensors[0].name;
  mod_som_apf_meta_data_ptr->probe1.type=0;
  mod_som_apf_meta_data_ptr->probe1.sn=
   (uint16_t)  strtol(local_efe_obp->efe_settings_ptr->sensors[0].sn, NULL, 10);
  mod_som_apf_meta_data_ptr->probe1.cal=
   (uint16_t) ceil(local_efe_obp->efe_settings_ptr->sensors[0].cal);

  mod_som_apf_meta_data_ptr->probe2.type=1;
  mod_som_apf_meta_data_ptr->probe2.sn=
  (uint16_t)  strtol(local_efe_obp->efe_settings_ptr->sensors[1].sn, NULL, 10);
  mod_som_apf_meta_data_ptr->probe2.cal=
      (uint16_t) ceil(local_efe_obp->efe_settings_ptr->sensors[1].cal);

  mod_som_apf_meta_data_ptr->profile_id=mod_som_apf_ptr->profile_id;

  //ALB get some values
  mod_som_apf_meta_data_ptr->daq_timestamp=sl_sleeptimer_get_time();

  mod_som_apf_meta_data_ptr->sample_cnt=0;
  mod_som_apf_meta_data_ptr->voltage=0;
  mod_som_apf_meta_data_ptr->end_metadata=0xFFFF;


//  if(mod_som_apf_ptr->config_ptr->storing_mode>0){
//      mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
//      mod_som_sdio_write_data_f(
//          mod_som_apf_meta_data_ptr,
//          sizeof(mod_som_apf_meta_data_ptr),
//          &mod_som_apf_ptr->consumer_ptr->consumed_flag);
//  }

}

/*******************************************************************************
 * @brief
 *    get meta data ptr
 *
 * @return
 *   mod_som_apf_meta_data_ptr_t
 ******************************************************************************/
mod_som_apf_meta_data_ptr_t mod_som_apf_get_meta_data_ptr()
{
  return &mod_som_apf_ptr->producer_ptr->mod_som_apf_meta_data;
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

  mod_som_apf_status_t status = MOD_SOM_APF_STATUS_OK;
  uint32_t bytes_send;

  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  uint32_t reply_str_len = 0;

  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  mod_som_settings_struct_ptr_t local_settings_ptr=
                                          mod_som_settings_get_settings_f();


  //ALB NOTE: We are keeping the ack/nack at this scope because we need to grab
  //ALB       the firmware rev at this level also. SO we can not just return the ack/nack
  //ALB In a normal config the return status should be MOD_SOM_IO_STATUS_ERR_NOT_STARTED
  status|= mod_som_io_print_f("%s,%s,%s\r\n",MOD_SOM_APF_FWREV_STAT_STR,MOD_SOM_APF_ACK_STR,
                              local_settings_ptr->gitid);

  // save time string into the temporary local string - Mai - Nov 18, 2021
  //ALB there is no error status here
  sprintf(apf_reply_str,"%s,%s,%s\r\n",
          MOD_SOM_APF_FWREV_STAT_STR,MOD_SOM_APF_ACK_STR,
          local_settings_ptr->gitid);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_send = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if (bytes_send!=reply_str_len){
      status=MOD_SOM_APF_STATUS_FAIL_SEND_MS;
  }
  if (status!=0)
  {
      mod_som_io_print_f("%s,%s,%lu\r\n",
                         MOD_SOM_APF_FWREV_STAT_STR,MOD_SOM_APF_NACK_STR,status);
      for (int i=0;i<MOD_SOM_SHELL_INPUT_BUF_SIZE;i++){
          apf_reply_str[i]=0;
      }

      status = MOD_SOM_APF_STATUS_OK; //ALB setting the status t

//      sprintf(apf_reply_str,"%s,%s,%lu\r\n",
//              MOD_SOM_APF_FWREV_STAT_STR,MOD_SOM_APF_NACK_STR,status);
//      reply_str_len = strlen(apf_reply_str);
//      // sending the above string to the APF port - Mai - Nov 18, 2021
//      bytes_send = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
//
//      if(bytes_send==0){
//          status=MOD_SOM_APF_STATUS_FAIL_SEND_MS;
//      }
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

//  status|= mod_som_io_print_f("ok?,ack\r\n");

	// can it see this string: mod_som_io_xfer_item_ptr->printf_str_ptr
  if(mod_som_apf_ptr->daq){
      status=0;
  }
  if(mod_som_apf_ptr->sleep_flag){
      //ALB epsi is sleeping
      //ALB Wake Up
      status|=mod_som_main_wake_up_f();
      status=0;

  }

  if (status!=0)
  {  //PB somewhere
      mod_som_io_print_f("%s,%s,%lu\r\n",
                         MOD_SOM_APF_OKSTAT_STR,MOD_SOM_APF_NACK_STR,status);

      sprintf(apf_reply_str,"%s,%s,improper wake up\r\n",
              MOD_SOM_APF_OKSTAT_STR,MOD_SOM_APF_NACK_STR);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      if(bytes_sent==0){
          status=MOD_SOM_APF_STATUS_FAIL_SEND_MS;
      }
  }else{
      sprintf(apf_reply_str,"%s,%s\r\n",
              MOD_SOM_APF_OKSTAT_STR,MOD_SOM_APF_ACK_STR);
      reply_str_len = strlen(apf_reply_str);
      mod_som_apf_ptr->sleep_flag=0;
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
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
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_poweroff_f(){
  mod_som_apf_status_t status=0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;
  uint32_t bytes_sent = 0;
//  CPU_CHAR filename[100];

  //make sure we are not in daq mode
  mod_som_apf_daq_stop_f();
  //ALB save settings in the UserData page
	mod_som_io_print_f("%s,%s\r\n",MOD_SOM_APF_POWEROFF_STR,MOD_SOM_APF_ACK_STR);

  // save time string into the temporary local string - Mai - Nov 18, 2021
  sprintf(apf_reply_str,"%s,%s\r\n",MOD_SOM_APF_POWEROFF_STR,MOD_SOM_APF_ACK_STR);
  reply_str_len = strlen(apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  if(bytes_sent==0){

      sprintf(apf_reply_str,"%s,%s,%lu\r\n",
              MOD_SOM_APF_POWEROFF_STR,MOD_SOM_APF_NACK_STR,status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      status=MOD_SOM_APF_STATUS_FAIL_SEND_MS;
  }


	return status;
}

/*******************************************************************************
 * @brief
 *   command shell for EpsiNo? command
 *   get the SOM and EFE SN
 *   should return an apf status.
 *
 *   The sensor should respond with: epsi no?,ack,ctlserno,feserno\r\n
 *   where ctlserno and feserno occupy 16-bits each. The high-order nibble represents a revision
 *   number and the low-order 3-nibbles represent a 12-bit serial number.
 *
 *
 *   For example, for ctlserno, if the revision number is 5 (hex: 0x5)
 *   and the serial number is 17 (hex: 0x11) then that would be encoded
 *   as the 16-bit hex value of 0x5011 which is decimal 20497.
 *   Similarly, for the feserno, if the revision is 11 (hex: 0xb) and
 *   the serial number is 3201 (hex: 0xc81) then that would be encoded
 *   as 0xbc81 which is decimal 48257.  So the command-response would
 *   be: epsi_no?,ack,20497,48257
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_epsi_id_status_f(){
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;

  uint16_t ctlserno=0;
  uint16_t feserno=0;

  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;
//  uint32_t bytes_sent = 0;

  mod_som_apf_status_t status = 0;
  mod_som_settings_struct_ptr_t local_settings_ptr=
                                          mod_som_settings_get_settings_f();

  //ALB encode ctlserno. 0 < rev < 9 and 000 < sn < 999
  uint32_t som_rev = strtoul(&local_settings_ptr->rev[3],NULL,10);
  uint32_t som_sn  = strtoul(local_settings_ptr->sn,NULL,10);

  ctlserno =  som_rev << 12; // High order nibble is rev no
  ctlserno =  ctlserno | (som_sn & 0xFFF); // first 3 nibbles are sn no

  //ALB encode feserno. 0 < rev < 9 and 000 < sn < 999
  uint32_t efe_rev = strtoul(&local_settings_ptr->mod_som_efe_settings.rev[3],NULL,10);
  uint32_t efe_sn  = strtoul(local_settings_ptr->mod_som_efe_settings.sn,NULL,10);

  feserno =  efe_rev << 12; // High order nibble is rev no
  feserno =  feserno | (efe_sn & 0xFFF); // first 3 nibbles are sn no



  mod_som_io_print_f("%s,%s,%hu,%hu\r\n",
                             MOD_SOM_APF_EPSINO_STAT_STR,
                             MOD_SOM_APF_ACK_STR,
                             ctlserno,
                             feserno);
  // save the string into the temporary local string - Mai - Nov 18, 2021

//ALB-CAP There is not error status here

  if ( (strtol(&local_settings_ptr->rev[3],NULL,10)>99) |
       (strtol(local_settings_ptr->sn,NULL,10)>999)    |
       (strtol(&local_settings_ptr->mod_som_efe_settings.rev[3],NULL,10)>99) |
       (strtol(local_settings_ptr->sn,NULL,10)>999) )
      {
      status=MOD_SOM_APF_STATUS_ERR;
  }

  if(status==MOD_SOM_APF_STATUS_OK){

      sprintf(apf_reply_str,"%s,%s,%hu,%hu\r\n",
              MOD_SOM_APF_EPSINO_STAT_STR,
              MOD_SOM_APF_ACK_STR,
              ctlserno,
              feserno);
      reply_str_len = strlen(apf_reply_str);

      // sending the above string to the APF port - Mai - Nov 18, 2021
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  }else{

      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_EPSINO_STAT_STR,
              MOD_SOM_APF_NACK_STR,
              "Issue with saved rev and SN");

      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  }

	return mod_som_apf_encode_status_f(status);
}


int isNumber(char *input_str);

int isNumber(char *input_str)
{
    int i = 0;
    i = strlen(input_str);
    while (i--)
      {
        if(input_str[i]>47 && input_str[i]<58)
          continue;
        return 0;
      }
    return 1;
}
/*******************************************************************************
 * @brief
 *   command shell for ProbeNo command
 *   set the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 *
 *   "Probe No,Type1,SerNo1,Coef1,Type2,SerNo2,Coef2\r":  This command is used to set
 *   1. the type (eg., shear, FP07). "Type#" is a character string (eg., “Shear”, “FP07”)
 *   2. serial number. "SerNo" is an unsigned long integer
 *   3. calibration coefficient for probes 1 & 2. "Coef" is a float.
 *   These parameters should be stored in the sensor’s nonvolatile storage.
 *   The sensor should respond with: "probe no,ack,type1,serno1,coef1,type2,serno2,coef2\r\n"
 *   If the sensor detects an exception that prevents it from storing these parameters,
 *   the sensor should respond with: "probe no,nak,error-description\r\n".
 *   probe_no command must:
      // Type1 is 's'
      // the SerNo1 & SerNo2 have 3 digits and
      // the Coef1 & Coef2 have 2 digits and
      // Coef1 & Coef2 not 0
      // "probe_id,Type1,SerNo1,Coef1,Type2,SerNo2,Coef2\r"
      // example command: "probe_id,s,123,12,f,123,12"
      // sensor 1: "s", length(123) = 3, length(12) = 2
      // if Type1 is 's', length of SerNo1 is 3, length of Coef1 is 2, Coef1 is positive => good command => save it
 *
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_probe_id_f(CPU_INT16U argc,
                                            CPU_CHAR *argv[]){

  mod_som_apf_status_t status=0;
  uint32_t cal1 = 0;
  uint32_t cal2 = 0;
  char *endptr1;
  char *endptr2;
//  bool argument_flag1 = false;
//  bool argument_flag2 = false;
  uint8_t channel_id  = 0;
  uint8_t length_argument2 = 0;
  uint8_t length_argument3 = 0;
  uint8_t length_argument5 = 0;
  uint8_t length_argument6 = 0;

  char arg2[16] = "\0";
  char arg3[16] = "\0";
  char arg5[16] = "\0";
  char arg6[16] = "\0";

  // probe_no command guide
  char probe_no_invalid_input[] = "probe_no,nak,invalid input type";
  int invalid_command = 0;

  // for send_string to the port
  uint32_t bytes_sent = 0;
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE] = "\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;

  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


  mod_som_efe_settings_ptr_t local_efe_settings_ptr =
      mod_som_efe_get_settings_ptr_f();

  switch (argc)
  {
     case 7: // the right number args of the command "probe_id,Type1,SerNo1,Coef1,Type2,SerNo2,Coef2\r"

       // bad type1
       if(strcmp(argv[1],"s"))
       {
           invalid_command = 1;
       }
       // bad Serial1 number - arg2
       if (arg2[0]=='-') // negative
       {
           invalid_command = 1;
       }
        // get the length of the rest of arguments - mai bui May 3rd, 2022
       length_argument2 = strlen(argv[2]); //SerNo1
       length_argument3 = strlen(argv[3]); //Coef1
       length_argument5 = strlen(argv[5]); //SerNo2
       length_argument6 = strlen(argv[6]); //Coef2

       // save into the temporary variables
       strcpy(arg2,argv[2]);
       strcpy(arg3,argv[3]);
       strcpy(arg5,argv[5]);
       strcpy(arg6,argv[6]);

       if (length_argument2!=3) // serial_no1 is NOT 3 digits
       {
           invalid_command = 1;
       }
       if (isNumber(arg2)==0)  // serial_no1 is NOT a number
       {
           invalid_command = 1;
       }
       if (arg3[0]=='-')  // Coef1 is NEGATIVE
       {
           invalid_command = 1;
       }
       if (length_argument3!=2) // Coef1 is NOT 2 digits
       {
           invalid_command = 1;
       }
       if (isNumber(arg3)==0)  // Coef is NOT a number
       {
           invalid_command = 1;
       }
       // *** Type 2:
       if(strcmp(argv[4],"f")) // Type2 is not 'f'
       {
           invalid_command = 1;
       }
       if (arg5[0]=='-') // Serial_no2 is negative
       {
           invalid_command = 1;
       }
       if (length_argument5!=3) // Serial_no2 does NOT have 3 digits
       {
           invalid_command = 1;
       }
       if (isNumber(arg5)==0) // Serial_no2 is NOT a number
       {
           invalid_command = 1;
       }
       if (arg6[0]=='-')// Coef2 number is NEGATIVE
       {
           invalid_command = 1;
       }
       if (length_argument6!=2) // Coef2 number is NOT 2 digits
       {
           invalid_command = 1;
       }
       if (isNumber(arg6)==0)  //Coef2 number is NOT a number
       {
           invalid_command = 1;
       }
       if (invalid_command)
       {
           // send out a short error message - maibui 16Aug2022
           sprintf(apf_reply_str,"%s\r\n",probe_no_invalid_input);
           status |= MOD_SOM_APF_STATUS_WRONG_ARG;
           break;
       }
       // get value of all parameter of the probe
       cal1 = strtol(argv[3], &endptr1, 10);  // Coef1
       cal2 = strtol(argv[6], &endptr2, 10);  // Coef2

       // this point, the input command is valid command
       channel_id = 1;  // save 'f' set in channel 1

       memcpy(&local_efe_settings_ptr->sensors[channel_id].sn,argv[2],3);
       local_efe_settings_ptr->sensors[channel_id].cal = cal1;

       channel_id = 0; // save 's' set in channel 0 - Maibui 5 May, 2022

       memcpy(&local_efe_settings_ptr->sensors[channel_id].sn,argv[5],3);
       local_efe_settings_ptr->sensors[channel_id].cal = cal2;
       // save all parameters' value
       status|= mod_som_settings_save_settings_f();

       // send 'ack' to screen
       status|= mod_som_io_print_f("%s,%s,s,%s,%lu,f,%s,%lu\r\n",
                                      MOD_SOM_APF_PROBENO_STR,MOD_SOM_APF_ACK_STR,
                                      local_efe_settings_ptr->sensors[1].sn,
                                      (uint32_t)local_efe_settings_ptr->sensors[1].cal,
                                      local_efe_settings_ptr->sensors[0].sn,
                                      (uint32_t)local_efe_settings_ptr->sensors[0].cal);

       sprintf(apf_reply_str,"%s,%s,s,%s,%lu,f,%s,%lu\r\n",
                  MOD_SOM_APF_PROBENO_STR,MOD_SOM_APF_ACK_STR,
                  local_efe_settings_ptr->sensors[1].sn,
                  (uint32_t)local_efe_settings_ptr->sensors[1].cal,
                  local_efe_settings_ptr->sensors[0].sn,
                  (uint32_t)local_efe_settings_ptr->sensors[0].cal);
      break;  // argc == 7
    default:  // argc != 7
      // save to the local string for sending out - Mai- May 3, 2022
      sprintf(apf_reply_str,"%s,%s,not enough arguments\r\n",
              MOD_SOM_APF_PROBENO_STR,MOD_SOM_APF_NACK_STR);
      status|= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
  } // end of switch (args)

  // sending to the screen - Mai- May 3, 2022
  mod_som_io_print_f("%s",apf_reply_str);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  if(bytes_sent==0){
      sprintf(apf_reply_str,"%s,%s,%lu, failed to send to the APF port\r\n",
              MOD_SOM_APF_PROBENO_STR,MOD_SOM_APF_NACK_STR,
              status);
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
   }
//  return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
  return status;
}

/*******************************************************************************
 * @brief
 *   command shell for ProbeNo? command
 *   get the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 *   according to in pdf file:
 *   "probeNo?\r"
 *   "proNo?,ack,type1,serno1,coef1,type2,serno2,coef2\r\n"
 *   "probe_no?,nak,error-description\r\n"
 *
 *   'S' = shear (sensor 2)
 *   'F' = FP07 (sensor 1)
 *
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

  char *endptr1;

  mod_som_efe_settings_ptr_t local_efe_settings_ptr=
      mod_som_efe_get_settings_ptr_f();

  uint16_t sn_temp   = (uint16_t) strtoul(local_efe_settings_ptr->sensors[0].sn,&endptr1,10) ;
  uint16_t sn_shear  = (uint16_t) strtoul(local_efe_settings_ptr->sensors[1].sn,&endptr1,10) ;
  uint16_t cal_temp  = (uint16_t) local_efe_settings_ptr->sensors[0].cal;
  uint16_t cal_shear = (uint16_t) local_efe_settings_ptr->sensors[1].cal;

  //ALB check errors on the sn and cal
  if ( (sn_temp>999) |
       (sn_shear>999)    |
       (cal_temp>99) |
       (cal_shear>99) )
      {
      status=MOD_SOM_APF_STATUS_ERR;
  }


if (status==MOD_SOM_APF_STATUS_OK){
    //ALB good case

    mod_som_io_print_f("%s,%s,%s,%s,%lu,%s,%s,%lu\r\n",
                       MOD_SOM_APF_PROBENO_STAT_STR,MOD_SOM_APF_ACK_STR,
                       "S",
                       local_efe_settings_ptr->sensors[1].sn,
                       (uint32_t)local_efe_settings_ptr->sensors[1].cal,
                       "F",
                       local_efe_settings_ptr->sensors[0].sn,
                       (uint32_t)local_efe_settings_ptr->sensors[0].cal);

  // save to the local string for sending out - Mai-Nov 18, 2021
   sprintf(apf_reply_str,"%s,%s,%s,%s,%lu,%s,%s,%lu\r\n",
           MOD_SOM_APF_PROBENO_STAT_STR,MOD_SOM_APF_ACK_STR,
            "S",
            local_efe_settings_ptr->sensors[1].sn,
            (uint32_t)local_efe_settings_ptr->sensors[1].cal,
            "F",
            local_efe_settings_ptr->sensors[0].sn,
            (uint32_t)local_efe_settings_ptr->sensors[0].cal);

    reply_str_len = strlen(apf_reply_str);

    // sending the above string to the APF port - Mai - Nov 18, 2021
    bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

    if(bytes_sent==0){
        status=MOD_SOM_APF_STATUS_ERR;
    }

}else{
    //ALB case if there is an error
    mod_som_io_print_f("%s,%s,%lu\r\n",
                       MOD_SOM_APF_PROBENO_STAT_STR,MOD_SOM_APF_NACK_STR,
                       status);
    // save to the local string for sending out - Mai-Nov 18, 2021
    sprintf(apf_reply_str,"%s,%s,%lu\r\n",
            MOD_SOM_APF_PROBENO_STAT_STR,MOD_SOM_APF_NACK_STR,
            status);
    reply_str_len = strlen(apf_reply_str);
    // sending the above string to the APF port - Mai - Nov 18, 2021
    bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
    if(bytes_sent==0){
        status=MOD_SOM_APF_STATUS_ERR;
    }

}

	return mod_som_apf_encode_status_f(status);
}

/*******************************************************************************
 * @brief
 *   command shell for sleep command
 *   put SOM to sleep
 *   should return an apf status.
 *   This command puts the Epsilometer back to sleep.
 *
 *   // 25 Aug, 2022: Arnaud, Mike: able to start acquisition, the whole system is awake.
 *   // With a "sleep" command
 *   // we stop acquistion and turn off everything off
 *   // put all data files in pending
 *
 *   // there are many ways to do: after checking the daq's mode:
 *   // Option 1: if in the daq mode, have to wait for finishing collect the last load of epsi data
 *   // and save that data into the file and close all datafile. (?)
 *
 *   // Option 2: using the daq modes to define (idle, sleep, shutdown)
 *   //       1. if NOT in daq mode -> shutdown, sleep (current version - send to Dana)
 *   //       2. if in daq mode -> (idle, shutdown, sleep):
 *   //         a. sleep & CTD, sleep efe + sniffing CTD (everthing need to sleep except CTD)
 *   //         b. sleep and timer
 *   //             (checking the postion of the equipment, reach to the surface, and save data)
 *
 *   // Conclusion: in acquistion mode or not:
 *   // 1. turn off everything to save power
 *   // 2. have a delay to save data into the file before close the data file
 *
 *   If the sensor happened to be in DAQ mode when it was awakened,
 *   the sleep command has the e ect of resuming the DAQ period
 *   already in progress. ???
 *   Mike's note: this one is not clear
 *
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

  //ALB TODO move "sleep,ack\r\n" in the upper level (####_cmd.c)
  char reply_str[] = "sleep,ack\r\n";

//ALB IDLE SLEEP
 if ((!mod_som_apf_ptr->daq) & (mod_som_apf_ptr->sleep_flag==0)){
      //ALB we are not in daq mode make sure
      //ALB efe,sdio,efe obp,sbe-sniffer are asleep
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

      sl_sleeptimer_delay_millisecond(100);
      //ALB disable SDIO hardware
      mod_som_sdio_disable_hardware_f();

      mod_som_main_sleep_f();
 }
 else{
     //ALB place holder for the other modes
     //TODO mode DAQ pressure - SLEEP
     //TODO mode DAQ time - SLEEP
 }
 if (mod_som_apf_ptr->sleep_flag==0){
     if ((status==0)){
         mod_som_apf_ptr->sleep_flag=1;
         //          status|=mod_som_io_print_f("sleep,ack\r\n");
         // save to the local string for sending out - Mai-Nov 18, 2021
         //          sprintf(apf_reply_str,"%s,%s\r\n"
         //                  MOD_SOM_APF_SLEEP_STR,MOD_SOM_APF_ACK_STR);
         sprintf(apf_reply_str,"%s",reply_str); // maibui 25Aug2022
     }else{
         //          status|=mod_som_io_print_f("sleep,nak,%lu\r\n",status);
         // save to the local string for sending out - Mai-Nov 18, 2021
         sprintf(apf_reply_str,"%s,%s,%lu\r\n",
                 MOD_SOM_APF_SLEEP_STR,MOD_SOM_APF_NACK_STR,status);
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
mod_som_apf_status_t mod_som_apf_gate_f(CPU_INT16U argc,
                                        CPU_CHAR *argv[]){

  mod_som_apf_status_t status = MOD_SOM_APF_STATUS_OK;

  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;



  if (argc == 2) // valid command: "gate on gate off\r"
  {

      if (strcmp(argv[1],"on")==0){
      //ALB turn on RS232 driver
          mod_som_main_com_on_f();
          sprintf(apf_reply_str,"%s,on,%s\r\n",
                  MOD_SOM_APF_GATE_STR,MOD_SOM_APF_ACK_STR);
      }else if (strcmp(argv[1],"off")==0){
      //ALB turn off RS232 driver
          mod_som_main_com_off_f();
          sprintf(apf_reply_str,"%s,off,%s\r\n",
                  MOD_SOM_APF_GATE_STR,MOD_SOM_APF_ACK_STR);
      }else{
          //ALB ERROR
          status=MOD_SOM_APF_STATUS_WRONG_ARG;
      }

      }else{
          //ALB ERROR
          status=MOD_SOM_APF_STATUS_WRONG_ARG;
      }


      if (status==MOD_SOM_APF_STATUS_OK){
          status|=mod_som_io_print_f("%s,%s\r\n",
                                     MOD_SOM_APF_GATE_STR,MOD_SOM_APF_ACK_STR);
          // save to the local string for sending out - Mai-Nov 18, 2021
      }else{
          status|=mod_som_io_print_f("%s,%s,%lu\r\n",
                                     MOD_SOM_APF_GATE_STR,MOD_SOM_APF_ACK_STR,
                                     status);
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"%s,%s,%lu\r\n",
                  MOD_SOM_APF_GATE_STR,MOD_SOM_APF_ACK_STR,
                  status);
     }
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
      if(bytes_sent==0){
          //TODO handle the error
      }


  return mod_som_apf_encode_status_f(status);
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
//  RTOS_ERR  p_err;
//  uint16_t year;
//  uint8_t month;
//  uint8_t month_day;
//  uint8_t hour;
//  uint8_t min;
//  uint8_t sec;

//  uint32_t bytes_sent = 0;
  // paramters for send_line_f()
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  unsigned long apex_time=0;
//  sl_sleeptimer_timestamp_t time;
  mod_som_apf_status_t status = 0;
  char second_arg[25] = "\0";
 // uint64_t time_unix = 0;
//  char time_valid_cmmd[] = "time,posixtime(>01-01-2020)"; // unixEpoch time range [1575205200  12345678901] (from Jan,1 2020)
  char invalid_time_cmmd[] = "time, nak, wrong input time";
  int invalid_command = 0;

  switch(argc)
  {
    case 2: // have 2 arguments
      // copy to the temp string
      strcpy(second_arg,argv[1]);

      // bad Coef1 number - arg3
      if (second_arg[0]=='-')  // NEGATIVE
      {
          invalid_command = 1;
      }
      // big number, more than 20 digits
      else if (strlen(second_arg) > 20)
      {
          invalid_command = 1;
      }
      // the input time is not the number
      else if (isNumber(second_arg)== 0)  // NOT a number
      {
          invalid_command = 1;
      }
      if (invalid_command)
      {
          sprintf(apf_reply_str,"%s\r\n", invalid_time_cmmd);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
          break;
      }
      // get the apex_time
      apex_time = strtoul(argv[1],NULL,10);
//      mod_som_io_print_f("apex_time %ld\r\n",apex_time);
//      apex_time = shellStrtol(argv[1],&p_err);
//      time = (int32_t) apex_time;
      if(apex_time>=ULONG_MAX)
      {
          invalid_command = 1;
      }
//      if(apex_time<TIME_MIN)  // before Jan 1, 2020 = 155205199
//      {
//          invalid_command = 1;
//      }
      if (invalid_command)
      {
          sprintf(apf_reply_str,"%s\r\n", invalid_time_cmmd);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
          break;
      }

      // valid command: "time,1234567\r"
      //ALB for APEX time is UNIX time the number of sec since Jan 1 1970
      //ALB so we need to add time_orig to time
      //ALB The epoch of timestamps in the sleeptimer is 1970

      // calculate the seconds from

      status|=mod_som_calendar_set_time_f(apex_time);
      status|=mod_som_settings_save_settings_f();

      // save time string into the temporary local string - Mai - Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_TIME_STR,MOD_SOM_APF_ACK_STR,
              argv[1]);
      status |= MOD_SOM_APF_STATUS_OK;

      break;
    default:  // not 2 agurments
      sprintf(apf_reply_str,"%s\r\n", invalid_time_cmmd);
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
  }

  mod_som_io_print_f("%s\r\n",apf_reply_str);
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  return status;
  //  return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
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

  mod_som_apf_status_t status = 0;
  sl_sleeptimer_timestamp_t time;
//  sl_sleeptimer_timestamp_t time1;

  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  uint32_t bytes_sent = 0;
//  uint64_t tick=0;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  //TODO add a checker for that cmd
  time= mod_som_calendar_get_time_f();

  //TODO add a checker for that cmd
  sl_sleeptimer_get_tick_count64();

//  mod_som_calendar_settings_ptr_t local_cal_setting = mod_som_calendar_get_settings_ptr_f();
//  time1=tick-local_cal_setting->poweron_offset_ms;
//
//  time1 = local_cal_setting->poweron_offset_ms;

  // save time string into the temporary local string - Mai - Nov 18, 2021
/*  sprintf(apf_reply_str,"time?,ack,%lu\r\n",
          MOD_SOM_APF_TIMESTAT_STR,MOD_SOM_APF_ACK_STR,
          (unsigned long)time);
 */ sprintf(apf_reply_str,"time?,ack,%lu\r\n",time);
  reply_str_len = strlen(apf_reply_str);


  //ALB  send out "time,ak,time\r\n"
//  status|=mod_som_io_print_f("time?,ack,%lu\r\n",
//                             MOD_SOM_APF_TIMESTAT_STR,MOD_SOM_APF_ACK_STR,
//                             time1);
  status|=mod_som_io_print_f("%s\r\n",apf_reply_str);

  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

//   if (status!=MOD_SOM_APF_STATUS_OK){
//	    mod_som_io_print_f("time?,nak,%lu\r\n",status);
//      // save to the local string for sending out - Mai-Nov 18, 2021
//      sprintf(apf_reply_str,"time?,nak,%lu\r\n",status);
//      reply_str_len = strlen(apf_reply_str);
//      // sending the above string to the APF port - Mai - Nov 18, 2021
//      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
//	}
   if(bytes_sent==0){
       //TODO handle the error
       status=MOD_SOM_APF_STATUS_ERR;

   }

	return mod_som_apf_encode_status_f(status);
}


/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_packet_format_f
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
mod_som_apf_status_t mod_som_apf_packet_format_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[])
{
//  RTOS_ERR  p_err;
//  uint8_t mode;
  mod_som_apf_status_t status=0;

//  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  char second_arg[25] = "\0";
  char invalid_packet_format[] = "packet_format,nak,invalid format_number";
  uint8_t mode_val = 0;


  if(mod_som_apf_ptr->daq){
            sprintf(apf_reply_str,"%s\r\n",
             "daq is running");
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;

  }else{

    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      // copy to the temp string
      strcpy(second_arg,argv[1]);
      // detect for not integer, only need check the first element of the third argument
      if(isalpha(second_arg[0]))  // format is not integer
      {
          // use the short invalid command error message - maibui 16Aug2022
          sprintf(apf_reply_str,"%s\r\n",invalid_packet_format);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
       }

      mode_val = atoi(argv[1]);
      if (((mode_val==1) || (mode_val==2)))// mode is either 1 or 2 -> valid - maibui 23Aug2022
      {
         mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format = mode_val;
         mod_som_settings_save_settings_f();

         // save to the local string for sending out - Mai-Nov 18, 2021
         sprintf(apf_reply_str,"%s,%s,%u\r\n",
                 MOD_SOM_APF_PACKETFORMAT_STR,MOD_SOM_APF_ACK_STR,
                 mode_val);
         status |= MOD_SOM_APF_STATUS_OK;
         break;
      }
     else// invalid_command
      {
          // use the short invalid command error message - maibui 16Aug2022
          sprintf(apf_reply_str,"%s\r\n",invalid_packet_format);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      }
      break;  // end off args = 2
  default:  // not 2 arguments
      // use the short invalid command error message - maibui 16Aug2022
      sprintf(apf_reply_str,"%s\r\n",
             invalid_packet_format);
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
 }  // end of  switch (argc)
  }
    mod_som_io_print_f("%s\r\n",apf_reply_str);
    // save to the local string for sending out - Mai-Nov 18, 2021
    reply_str_len = strlen(apf_reply_str);
    // sending the above string to the APF port - Mai - Nov 18, 2021
    mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  switch (mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format)
  {
    case 0:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
      break;
    case 1:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_FOM_SIZE;
      break;
    case 2:
      mod_som_apf_ptr->producer_ptr->dacq_element_size=
          MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
          MOD_SOM_APF_DACQ_PRESSURE_SIZE+
          MOD_SOM_APF_DACQ_DISSRATE_SIZE+
          MOD_SOM_APF_DACQ_SEAWATER_SPEED_SIZE+
         (MOD_SOM_EFE_OBP_CHANNEL_NUMBER*
          mod_som_apf_ptr->producer_ptr->nfft_diag*
          MOD_SOM_APF_PRODUCER_FOCO_RES);
      break;
    default:
      break;
  }
  if(status != MOD_SOM_APF_STATUS_OK)
      return MOD_SOM_APF_STATUS_ERR;

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
mod_som_apf_status_t mod_som_apf_packet_format_status_f(){

  mod_som_apf_status_t status=0;
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;


  uint32_t mode=mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format;

  if ( (mode<0) | (mode>2)){
      status=MOD_SOM_APF_STATUS_ERR;
  }


  if (status==MOD_SOM_APF_STATUS_OK){
      //ALB good case
      mod_som_io_print_f("%s,%s,%lu\r\n",
                         MOD_SOM_APF_PACKETFORMAT_STAT_STR,MOD_SOM_APF_ACK_STR,
                         mode);

      // save to the local string for sending out - Mai-Dec 1, 2021
      sprintf(apf_reply_str,"%s,%s,%lu\r\n",
              MOD_SOM_APF_PACKETFORMAT_STAT_STR,MOD_SOM_APF_ACK_STR,
              mode);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Dec 1, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);


  }else{
      //ALB bad case
      mod_som_io_print_f("%s,%s,%lu\r\n",
                         MOD_SOM_APF_PACKETFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR,
                         status);
      // save to the local string for sending out - Mai- Dec 1, 2021
      sprintf(apf_reply_str,"%s,%s,error while reading the mode\r\n",
              MOD_SOM_APF_PACKETFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Dec 1, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);
  }

  if(bytes_sent==0){
      //TODO handle the error
      status=MOD_SOM_APF_STATUS_ERR;
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
  uint8_t mode = 0;
//  bool good_argument=false;
//  CPU_INT16U argc_sbe41 = 2;
//  CPU_CHAR   *argv_sbe41_sd[2]={"sbe.mode" ,"1"};
//  CPU_CHAR   *argv_sbe41_off[2]={"sbe.mode" ,"2"};
//  CPU_INT16U argc_efe = 2;
//  CPU_CHAR   *argv_efe_sd[2]={"efe.mode" ,"1"};
//  CPU_CHAR   *argv_efe_off[2]={"efe.mode" ,"2"};
//  CPU_INT16U argc_efe_obp = 2;
//  CPU_CHAR   *argv_efe_obp_sd[2]={"efeobp.mode" ,"1"};
//  CPU_CHAR   *argv_efe_obp_off[2]={"efeobp.mode" ,"2"};
//  CPU_CHAR   *argv_efe_obp_avgspec_format[2]={"efeobp.format" ,"2"}; //spectra
//  CPU_CHAR   *argv_efe_obp_none_format[2]={"efeobp.format" ,"3"}; //only dissrates

  char second_arg[16] = "\0";
  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

  //char sd_format_valid_cmmd[] = "sd_format format_id (format_id is 1 or 2)\r\n";

    //ALB switch statement easy to handle all user input cases.
    // command must only 2 arguments, otherwise send 'nak' to SOM and message to APF's port - maibui April 27, 2022
  switch (argc)
  {
    case 2: // 2 args:      "sd_format,arg2"
      // if sd's format is not interger => break
      strcpy(second_arg,argv[1]);
      if (isalpha(second_arg[0]))
      {
          sprintf(apf_reply_str,"%s,%s,invalid input\r\n",
                  MOD_SOM_APF_SDFORMAT_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
          break;
     }
//      mode=shellStrtol(argv[1],&p_err);
      // get the mode value of sd_format
      mode = strtol(argv[1], &endptr, 10); // strtol

      // if mode is in range: 0< mode < 3 ==> good command
      if((mode<3) && (mode>0)){
          // save mode into the sd_format structure
          mod_som_apf_ptr->settings_ptr->sd_packet_format = mode;
          sprintf(apf_reply_str,"%s,%s,%i\r\n",
                  MOD_SOM_APF_SDFORMAT_STR,MOD_SOM_APF_ACK_STR,
                  mode);
          status |= mod_som_settings_save_settings_f();
          status |= MOD_SOM_APF_STATUS_OK;
      }
      else  // not 1 or 2
      {
          // save to the local string for sending out - Mai-Nov 18, 2021
          sprintf(apf_reply_str,"%s,%s,invalid input\r\n",
                  MOD_SOM_APF_SDFORMAT_STR,MOD_SOM_APF_NACK_STR);
          status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      }
      break;
    // not 2 arguments
    default:  // command does NOT have 2 arguments: set the status, send 'nak' message to som
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,invalid input\r\n",
              MOD_SOM_APF_SDFORMAT_STR,MOD_SOM_APF_NACK_STR);
      status |= MOD_SOM_APF_STATUS_WRONG_ARG;
      break;
    } // end of switch case of number of arguments


  reply_str_len = strlen(apf_reply_str);
  mod_som_io_print_f("%s\r\n",apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }

  return status;
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
//  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;



  uint32_t mode=mod_som_apf_ptr->settings_ptr->comm_telemetry_packet_format;

  if ( (mode<0) | (mode>2)){
      status=MOD_SOM_APF_STATUS_ERR;
  }


  if (status==MOD_SOM_APF_STATUS_OK){
      //ALB good case
      mod_som_io_print_f("%s,%s,%lu\r\n",
                                  MOD_SOM_APF_SDFORMAT_STAT_STR,MOD_SOM_APF_ACK_STR,
                                  mod_som_apf_ptr->settings_ptr->sd_packet_format);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%lu\r\n",
              MOD_SOM_APF_SDFORMAT_STAT_STR,MOD_SOM_APF_ACK_STR,
              mod_som_apf_ptr->settings_ptr->sd_packet_format);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  }else{
      //ALB bad case
      mod_som_io_print_f("%s,%s,%lu\r\n",
                         MOD_SOM_APF_SDFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR,
                         status);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%lu\r\n",
              MOD_SOM_APF_SDFORMAT_STAT_STR,MOD_SOM_APF_NACK_STR,
              status);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  }

  return status;
}


/*******************************************************************************
 * @brief
 *   command shell for q command
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

  mod_som_apf_status_t status = MOD_SOM_APF_STATUS_OK;
//  FRESULT res=0;
  uint32_t delay=MOD_SOM_APF_UPLOAD_DELAY;

  uint32_t bytes_sent = 0;
  // for send_string to the port
  char apf_reply_str[MOD_SOM_SHELL_INPUT_BUF_SIZE]="\0";
  size_t reply_str_len = 0;
  LEUART_TypeDef* apf_leuart_ptr;
  // get the port's fd
  apf_leuart_ptr = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;

//  int c;
  int read_char=-1;

  unsigned int crc=0;
//  uint32_t   timeout = 0;
  uint32_t   timeout_flag = 0;
  uint32_t   t1 = 0;
  uint32_t   t0 = 0;
//  uint32_t   dacq_bytes_available = 0;
  uint32_t   dacq_bytes_sent      = 0;
  uint32_t   dacq_bytes_to_sent   = 0;
//  uint8_t *  current_data_ptr=
//      mod_som_apf_ptr->producer_ptr->dacq_ptr;
  uint8_t    eot_byte = MOD_SOM_APF_UPLOAD_EOT_BYTE;
  uint32_t cnt=0;


  //ALB start transmit the packet
  //ALB check if daq is stopped
  if(!mod_som_apf_ptr->daq & !mod_som_apf_ptr->sleep_flag){

      ////  //ALB enable SDIO hardware
      mod_som_sdio_enable_hardware_f();

      /*********/
//      //ALB if this a reboot dacq_size=0
//      if(mod_som_apf_ptr->producer_ptr->dacq_size==0){

//          sl_sleeptimer_delay_millisecond(500);
          //ALB file not open
          status = mod_som_sdio_opentoread_processfilename_f("OBPdata");
//          if (status==0x2u){
//              status=MOD_SOM_APF_STATUS_CANNOT_OPENFILE;
//          }
//      }

      mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
          mod_som_sdio_get_runtime_ptr_f();
      mod_som_sdio_file_ptr_t processfile_ptr =
          local_mod_som_sdio_ptr_t->processdata_file_ptr;

      f_lseek (processfile_ptr->fp, 0);
      cnt=f_size(processfile_ptr->fp);
      mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes   = cnt;

      if(cnt==0){ //ALB no data
          status = MOD_SOM_APF_STATUS_NO_DATA;
      }

      if (status == MOD_SOM_APF_STATUS_OK){
      //ALB upload cmd received
      //ALB send msg back
      mod_som_io_print_f("%s,%s,start\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_ACK_STR);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,start\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_ACK_STR);
      reply_str_len = strlen(apf_reply_str);
      // sending the above string to the APF port - Mai - Nov 18, 2021
      bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

      //ALB wait 500 ms
      sl_sleeptimer_delay_millisecond(delay);

      //ALB initialize send_packet_tries.
      mod_som_apf_ptr->consumer_ptr->send_packet_tries   = 0;
      mod_som_apf_ptr->consumer_ptr->nb_packet_sent      = 0;
      mod_som_apf_ptr->consumer_ptr->packet.CRC          = 0;
      mod_som_apf_ptr->consumer_ptr->packet.counters     = 0;
//      mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes =
//          mod_som_apf_ptr->producer_ptr->dacq_size+
//          sizeof(mod_som_apf_meta_data_t);

      //ALB split acq_profile into 990 bytes packets:
      //ALB count how bytes are left to send out.
//      dacq_bytes_available=
//          mod_som_apf_ptr->producer_ptr->dacq_size-dacq_bytes_sent;

      //ALB while bytes available > than 986 bytes
      //ALB I build a full packet and stream it.
      while ((mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes>=0) &&
          (mod_som_apf_ptr->consumer_ptr->send_packet_tries<
              MOD_SOM_APF_UPLOAD_MAX_TRY_PACKET)){

          status=MOD_SOM_APF_STATUS_OK;
          //ALB define how many byte to send
          //ALB i.e. min(dacq_bytes_available,
          //ALB          MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE)

          dacq_bytes_to_sent=MIN(mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes,
                                 MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE);

          //ALB end of profile sending the EOT bytes
          if(mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes==0){
//              current_data_ptr=&eot_byte;
              dacq_bytes_to_sent=1;
              mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes=-1;
              memcpy(&mod_som_apf_ptr->consumer_ptr->packet.payload,
                     &eot_byte,
                     dacq_bytes_to_sent);

          }else{
              printf("Bytes to send %lu\r\n",dacq_bytes_to_sent);
              printf("Bytes sent %lu\r\n",dacq_bytes_sent);

              //ALB copy the dacq bytes in the packet payload structure.
              mod_som_sdio_read_processfile_f(
                  (uint8_t*) &mod_som_apf_ptr->consumer_ptr->packet.payload,
                  dacq_bytes_to_sent,
                  dacq_bytes_sent);
          }

          //ALB compute the packet CRC
          //TODO
          crc=Crc16Bit((unsigned char *) mod_som_apf_ptr->consumer_ptr->packet.payload,
                       dacq_bytes_to_sent);

//          printf("crc %u\r\n",crc);

          mod_som_apf_ptr->consumer_ptr->packet.CRC=crc;
          //ALB compute the counters
          //TODO finalize the bitshifting to add the remaining number of bytes to send
          mod_som_apf_ptr->consumer_ptr->packet.counters=
              (uint16_t) mod_som_apf_ptr->consumer_ptr->nb_packet_sent;

//          printf("packet.counters %u\r\n",mod_som_apf_ptr->consumer_ptr->packet.counters);

          //ALB bitshift packet.counters
          mod_som_apf_ptr->consumer_ptr->packet.counters=
              mod_som_apf_ptr->consumer_ptr->packet.counters<<10;
          //ALB update daq_remaining_bytes. !!This the while loop param!!
          mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes-=dacq_bytes_to_sent;
          //ALB update packet.counters
          mod_som_apf_ptr->consumer_ptr->packet.counters|=
                             dacq_bytes_to_sent;

          //ALB packet should be ready. Now send it
          mod_som_apf_ptr->consumer_ptr->consumed_flag=false;

          //TODO MNB send the packet to APEX-shell
          mod_som_io_stream_data_f(
              (uint8_t *)&mod_som_apf_ptr->consumer_ptr->packet,
              dacq_bytes_to_sent+MOD_SOM_APF_UPLOAD_PACKET_CRC_SIZE+
              MOD_SOM_APF_UPLOAD_PACKET_CNT_SIZE,
              &mod_som_apf_ptr->consumer_ptr->consumed_flag);

          //ALB We need to send the packet to the APEX shell
//          sl_sleeptimer_delay_millisecond(10);
          bytes_sent = mod_som_apf_send_line_f(
              apf_leuart_ptr,
              (char *) &mod_som_apf_ptr->consumer_ptr->packet,
              dacq_bytes_to_sent+MOD_SOM_APF_UPLOAD_PACKET_CRC_SIZE+
                                 MOD_SOM_APF_UPLOAD_PACKET_CNT_SIZE);



          //ALB Wait for APF11 answer.
          //ALB APF sends ACK if fine.
          //ALB APF send NACK if not fine.
          //ALB There is also a 5sec time out ~ NACK.
          //ALB If NACK or timeout, try 3 times to send the packet.
          //ALB If packet is NACK after 3 tries, go back to menu (i.e., exit the upload function).

          //ALB get a t0=timestamp right before the while
          mod_som_apf_ptr->consumer_ptr->nb_packet_sent++;
          mod_som_apf_ptr->consumer_ptr->nb_packet_sent%=64;
          t0= mod_som_calendar_get_time_f();
          read_char=0;
          while (read_char <= 0){ // Wait for valid input
              timeout_flag=0;
              //Release for waiting tasks
//              c = RETARGET_ReadChar();
              status = mod_som_apf_get_char_f(apf_leuart_ptr, &read_char); // call for getting char from LEUART

              //TODO MNB read the bytes from APEX-shell
//              c=MOD_SOM_APF_UPLOAD_APF11_ACK;
              //ALB good transmit, go to next packet.
              if(read_char==MOD_SOM_APF_UPLOAD_APF11_ACK){
                  //ALB update current_data_ptr to point to the next dacq data
//                  current_data_ptr+=dacq_bytes_to_sent;
                  dacq_bytes_sent+=dacq_bytes_to_sent;
                  mod_som_apf_ptr->consumer_ptr->send_packet_tries=0;
                  status=MOD_SOM_APF_STATUS_OK;
                  break;
              }
              //ALB get current timestamp t1
              t1= mod_som_calendar_get_time_f();
              //ALB calculate time out if t1-t0>5sec
              if ((t1-t0)>=MOD_SOM_APF_UPLOAD_APF11_TIMEOUT){
                  //e
                  timeout_flag=1;
              }
              //ALB error: either NACK of timeout.
              //ALB Break the while loop and retry to send the packet
              if((read_char==MOD_SOM_APF_UPLOAD_APF11_NACK) || (timeout_flag==1)){
                  mod_som_apf_ptr->consumer_ptr->send_packet_tries++;
                  mod_som_apf_ptr->consumer_ptr->nb_packet_sent--;
                  mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes+=
                                                             dacq_bytes_to_sent;
                  status=MOD_SOM_APF_STATUS_FAIL_SEND_PACKET;
//                  dacq_bytes_sent=0;
                  break; //Break the while loop to try again
              }//ALB end of if
              read_char=0;
          }//ALB end of while c

          //ALB update dacq_bytes_available.
//          dacq_bytes_available=
//              mod_som_apf_ptr->producer_ptr->dacq_size-dacq_bytes_sent;
//          mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes

          //ALB Feed the watch dog
          WDOG_Feed();
          mod_som_io_print_f("daq_remaining_bytes:%i\r\n",mod_som_apf_ptr->consumer_ptr->daq_remaining_bytes);

      }//ALB end of while bytes available

      }//ALB end if MOD_SOM_OK
  }//ALB end if(!mod_som_apf_ptr->daq)
  else{
      if(mod_som_apf_ptr->daq){
          //ALB daq is still running
          status=MOD_SOM_APF_STATUS_DAQ_IS_RUNNING;
      }
      if(mod_som_apf_ptr->sleep_flag){
          status=MOD_SOM_APF_STATUS_SLEEPING;
      }
  }//ALB end if daq

  //ALB end of upload send the upload status

  switch (status){
    case MOD_SOM_APF_STATUS_CANNOT_OPENFILE:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "can not open file");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "can not open file");
      status = MOD_SOM_APF_STATUS_OK;
      break;

    case MOD_SOM_APF_STATUS_NO_DATA:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "no data");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "no data");
      status = MOD_SOM_APF_STATUS_OK;

      break;
    case MOD_SOM_APF_STATUS_DAQ_IS_RUNNING:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "daq is still running");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "daq is still running");
      status = MOD_SOM_APF_STATUS_OK;
      break;
    case MOD_SOM_APF_STATUS_SLEEPING:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "epsi is sleeping");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "epsi is sleeping");

      status = MOD_SOM_APF_STATUS_OK;
      break;
    case MOD_SOM_APF_STATUS_FAIL_SEND_PACKET:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "fail sending packets");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "fail sending packets");
      status = MOD_SOM_APF_STATUS_OK;
      break;
    case MOD_SOM_APF_STATUS_OK:
      mod_som_io_print_f("%s,%s,success\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_ACK_STR);
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,success\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_ACK_STR);
      //ALB remove OBPdata file
      //ALB actually I do not think I need to remove the OBPdata file
//      mod_som_sdio_rm_sd_f("OBPdata");
      status = MOD_SOM_APF_STATUS_OK;
      break;
    default:
      mod_som_io_print_f("%s,%s,%s\r\n",
                         MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
                         "unknown error");
      // save to the local string for sending out - Mai-Nov 18, 2021
      sprintf(apf_reply_str,"%s,%s,%s\r\n",
              MOD_SOM_APF_UPLOAD_STR,MOD_SOM_APF_NACK_STR,
              "unknown error");

      status = MOD_SOM_APF_STATUS_OK;
      break;

      mod_som_sdio_disable_hardware_f();

  }
  reply_str_len = strlen(apf_reply_str);
  // sending the above string to the APF port - Mai - Nov 18, 2021
  bytes_sent = mod_som_apf_send_line_f(apf_leuart_ptr,apf_reply_str, reply_str_len);

  if(bytes_sent==0){
      //TODO handle the error
  }

  return mod_som_apf_encode_status_f(status);
}


/*******************************************************************************
 * @brief
 *
 * @param handle
 *   handle to communication port
 *
 * @param xfer_status
 *  status of transfer
 *
 * @param data
 *  pointer to data
 *
 * @param xfer_count
 *  length of data receive
 ******************************************************************************/
void LEUART0_IRQHandler(){

  LEUART_TypeDef  *leuart_ptr;
//  uint8_t indx    =0;

  uint32_t interrupt_sig;
  leuart_ptr  = (LEUART_TypeDef *)mod_som_apf_ptr->com_prf_ptr->handle_port;;
  // Disabling the LEUART IRQ to "run" on the LDMA interrupt.

  interrupt_sig = leuart_ptr->IF;
  LEUART_IntClear(leuart_ptr,interrupt_sig);


  if(interrupt_sig & LEUART_IF_RXDATAV){

      if (apf_rxCount < (MOD_SOM_APF_SHELL_STR_LENGTH)) {
        /* There is room for data in the RX buffer so we store the data. */
        apf_rxBuffer[apf_rxWriteIndex] = LEUART_Rx(leuart_ptr);
        apf_rxWriteIndex++;
        apf_rxCount++;

        if (apf_rxWriteIndex == (MOD_SOM_APF_SHELL_STR_LENGTH)) {
            apf_rxWriteIndex = 0;
        }
      } else {
        /* The RX buffer is full so we must wait for the RETARGET_ReadChar()
         * function to make some more room in the buffer. RX interrupts are
         * disabled to let the ISR exit. The RX interrupt will be enabled in
         * RETARGET_ReadChar(). */
          LEUART_IntDisable(leuart_ptr, LEUART_IF_RXDATAV);
      }
  }
}




#endif


