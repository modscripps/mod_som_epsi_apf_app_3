/*******************************************************************************
 * @file mod_som_sbe49.h
 * @brief MOD SOM SBE49 API implementation
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 * @date Jan 27, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API implementation file defines functions to be used with an SBE 49
 * device connecting to the MOD SOM board.
 * The ports definition are established in mod_bsp_cfg.h file.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "mod_som_sbe49.h"
#include "mod_som_sbe49_priv.h"
#include "mod_som_sbe49_bsp.h"
#include "mod_som_sbe49_cfg.h"
#include "mod_som_io.h"
#include "mod_som.h"
#include "mod_som_priv.h"

//#include "arm_math.h"


//#include "mod_som_sdio.h"
//#include "mod_som_priv.h"


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_sbe49_cmd.h"
#endif
#ifdef  MOD_SOM_SDIO_EN
#include "mod_som_sdio.h"
#endif
#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif
#ifdef MOD_SOM_AGGREGATOR_EN
  #include <mod_som_aggregator.h>
#endif

//MHA
#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;

// TODO need to create a task for this  so that mutex can work
// each device should have it own task

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
//#define MOD_SOM_SBE49_DATA_RX_LENGTH 64
//#define MOD_SOM_SBE49_DATA_LENGTH_TRUNCATION 2U

// Data consumer
static CPU_STK sbe49_consumer_task_stk[MOD_SOM_SBE49_CONSUMER_TASK_STK_SIZE];
static OS_TCB  sbe49_consumer_task_tcb;


//------------------------------------------------------------------------------
// global variables
//------------------------------------------------------------------------------
// TODO We want to change the functions so they are more portable
// TODO An option is to have the module_ptr as an argument for the functions.

static mod_som_sbe49_ptr_t mod_som_sbe49_ptr;
sl_status_t mystatus;

//LDMA variable
LDMA_TransferCfg_t sbe_ldma_signal= LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_LEUART0_RXDATAV);
//ALB Place holder for adc_read from LEUART0 RX to databuffer. It will be change in config adc.
LDMA_Descriptor_t sbe_ldma_read_rx = LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(0,0,0);
LDMA_Descriptor_t descriptor_read_sbe;//ALB Descriptor list to gather SBE sample.


//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 *   mod_som_com_port_t rx;
 *   mod_som_com_port_t tx;
 *   mod_som_gpio_port_t en;
 *   uint32_t baud_rate;
 *   mod_som_uart_parity_t parity;
 *   mod_som_uart_data_bits_t data_bits;
 *   mod_som_uart_stop_bits_t stop_bits;
 *   uint32_t sample_data_length;
 *   uint32_t buffer_length;
 *   uint32_t sample_per_buffer;
 *
 *
 *
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_config_f(){
  RTOS_ERR  err;

  //Configuration
  mod_som_sbe49_ptr->config_ptr =
      (mod_som_sbe49_config_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 config.",DEF_NULL,
          sizeof(mod_som_sbe49_config_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->config_ptr==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  mod_som_sbe49_config_ptr_t config_ptr = mod_som_sbe49_ptr->config_ptr;
  config_ptr->port.com_port   = (void *)MOD_SOM_SBE49_USART;
  config_ptr->port.route      = MOD_SOM_SBE49_TX_LOC;
  config_ptr->port.tx_port    = MOD_SOM_SBE49_TX_PORT;
  config_ptr->port.tx_pin     = MOD_SOM_SBE49_TX_PIN;

  config_ptr->port.rx_port    = MOD_SOM_SBE49_RX_PORT;
  config_ptr->port.rx_pin     = MOD_SOM_SBE49_RX_PIN;

  config_ptr->port.en_port    = MOD_SOM_SBE49_EN_PORT;
  config_ptr->port.en_pin     = MOD_SOM_SBE49_EN_PIN;

  config_ptr->port.parity    = mod_som_uart_parity_none;
  config_ptr->port.data_bits = mod_som_uart_data_bits_8;
  config_ptr->port.stop_bits = mod_som_uart_stop_bits_1;

  switch (mod_som_sbe49_ptr->settings_ptr->data_format){
    case 1:
      config_ptr->sample_data_length  = MOD_SOM_SBE49_OUTPUT1_SAMPLE_DATA_LENGTH;
      break;
    case 3:
      config_ptr->sample_data_length  = MOD_SOM_SBE49_OUTPUT3_SAMPLE_DATA_LENGTH;
      break;
    default:
      config_ptr->sample_data_length  = MOD_SOM_SBE49_OUTPUT0_SAMPLE_DATA_LENGTH;
      break;
  }
  config_ptr->elements_per_buffer  = MOD_SOM_SBE49_DATA_SAMPLES_PER_BUFFER;
  config_ptr->baud_rate           = MOD_SOM_SBE49_BAUDRATE;
  config_ptr->consumer_delay      = MOD_SOM_SBE49_CONSUMER_DELAY;
  config_ptr->element_length      = MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH+config_ptr->sample_data_length;
  config_ptr->buffer_length       = config_ptr->element_length*\
                                          config_ptr->elements_per_buffer;

  config_ptr->max_element_per_record=\
                          mod_som_sbe49_ptr->settings_ptr->elements_per_record;

  config_ptr->header_length=MOD_SOM_SBE49_SYNC_HEADER_LENGTH     +
                            MOD_SOM_SBE49_TAG_LENGTH             +
                            MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH    +
                            MOD_SOM_SBE49_HEXPAYLOAD_LENGTH      +
                            MOD_SOM_SBE49_HEADER_CHECKSUM_LENGTH;


 //ALB length of consumer record buffer. Updated while running.
 config_ptr->record_length = config_ptr->header_length +
                             config_ptr->max_element_per_record *
                             config_ptr->element_length+
                             MOD_SOM_SBE49_CHECKSUM_SIZE;


  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   MHA
 *   Initialize OBP pointer with default data
 *
 *
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_obp_f(){
  RTOS_ERR  err;

  //OBP
  mod_som_sbe49_ptr->obp_ptr =
      (mod_som_sbe49_obp_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 obp.",DEF_NULL,
          sizeof(mod_som_sbe49_obp_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->obp_ptr==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }

//MHA get memory for the cal coef struct.
  mod_som_sbe49_ptr->obp_ptr->cal_coef =
      (mod_som_sbe49_obp_cal_coef_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 obp->calcoef",DEF_NULL,
          sizeof(mod_som_sbe49_obp_cal_coef_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->obp_ptr->cal_coef==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  // MHA DO ALL OBP INIT HERE - timestamp, p,t,c,s
  mod_som_sbe49_obp_ptr_t obp_ptr = mod_som_sbe49_ptr->obp_ptr;
  obp_ptr->timestamp = (uint64_t) 0;
  obp_ptr->pressure=0;
  obp_ptr->conductivity=0;
  obp_ptr->temperature=0;
  obp_ptr->salinity=0;

  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Initialize settings pointer with default data
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_settings_f(){
  RTOS_ERR  err;

  //settings
  mod_som_sbe49_ptr->settings_ptr =
      (mod_som_sbe49_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 SETUP.",DEF_NULL,
          sizeof(mod_som_sbe49_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->settings_ptr==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  mod_som_sbe49_settings_ptr_t settings_ptr = mod_som_sbe49_ptr->settings_ptr;

  strcpy(settings_ptr->sn, MOD_SOM_SBE49_DEFAULT_SN);
  strncpy(settings_ptr->data_header_text,MOD_SOM_SBE49_DEFAULT_TAG,MOD_SOM_SBE49_SETTING_STR_LENGTH);

  settings_ptr->data_format          = MOD_SOM_SBE49_DATA_FORMAT_DEFAULT;
  settings_ptr->elements_per_record  = MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;
  settings_ptr->initialize_flag      = true;

  settings_ptr->size=sizeof(*settings_ptr);

  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the settings.
 *   to be used by other modules
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_sbe49_settings_t mod_som_sbe49_get_settings_f(){
	return *mod_som_sbe49_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get the config pointer.
 *   to be used by other modules
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_sbe49_config_ptr_t mod_som_sbe49_get_config_f(){
return  mod_som_sbe49_ptr->config_ptr;
}


//TODO create a function to check memory allocation when using memseg
//mod_som_status_t mod_som_sbe49_check_memory_allocation_f(RTOS_ERR  err, void* struct_ptr){
//
//  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
//      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
//  if(struct_ptr==DEF_NULL)
//  {
//      mod_som_sbe49_ptr = DEF_NULL;
//      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
//  }else{
//      return MOD_SOM_SBE49_STATUS_OK;
//  }
//
//}

/*******************************************************************************
 * @brief
 *   Initialize the device
 * @description
 *  The steps are
 *      1) allocate memories to store device configuration and data
 *      2) configure the communication for data acquisition
 *          a) register the correct setting according to the right peripheral
 *              type including the interrupts
 *          b) add the interrupt handling function to peripheral
 *          c) register peripheral to MOD_SOM peripheral list so that
 *             the correct interrupt handler is passed along
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_init_f(){
    RTOS_ERR  err;


    //Memory allocation
    //Main device pointer
    mod_som_sbe49_ptr = (mod_som_sbe49_ptr_t)Mem_SegAlloc(
        "MOD SOM SBE49 RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_sbe49_t),
        &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_sbe49_ptr==DEF_NULL)
      return (mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    mod_som_sbe49_ptr->status = mod_som_sbe49_init_cmd_f();
    if(mod_som_sbe49_ptr->status != MOD_SOM_STATUS_OK)
      return mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_INIT_CMD);
#endif


    mod_som_sbe49_ptr->initialized_flag = false;




    //set up configuration
    mod_som_sbe49_construct_settings_f();
    mod_som_sbe49_construct_config_f();

    mod_som_sbe49_construct_obp_f(); //MHA initialize OBP

    mod_som_sbe49_construct_com_prf_f();
    mod_som_sbe49_construct_rec_buff_f();
    mod_som_sbe49_construct_consumer_ptr_f();

    mod_som_sbe49_init_ldma_f(mod_som_sbe49_ptr);

    //ALB obp: auto actuator depths
    mod_som_sbe49_ptr->obp_ptr->ptc_depth = MOD_SOM_SBE49_OBP_PTC_DEPTH;
    mod_som_sbe49_ptr->obp_ptr->ctc_depth = MOD_SOM_SBE49_OBP_CTC_DEPTH;

    //register the peripheral
    mod_som_add_peripheral_f((mod_som_prf_ptr_t)mod_som_sbe49_ptr->com_prf_ptr);

    mod_som_sbe49_ptr->status = MOD_SOM_STATUS_OK;
    mod_som_sbe49_ptr->connected_flag = false;
    mod_som_sbe49_ptr->initialized_flag = true;
    mod_som_sbe49_ptr->sample_count= 0;
    mod_som_sbe49_ptr->consumer_mode=0;


    printf("S49 initialized\r\n"); //MHA
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
}


/*******************************************************************************
 * @brief
 *   construct com_prf
 *
 * @param mod_som_sbe49_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_com_prf_f(){

  RTOS_ERR  err;

  LEUART_TypeDef          *leuart_ptr;
  LEUART_Init_TypeDef     leuart_init = LEUART_INIT_DEFAULT;


#if defined(_CMU_HFPERCLKEN0_MASK)
  CMU_ClockEnable(cmuClock_HFPER, true);
#endif
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Enable CORE LE clock in order to access LE modules */
  CMU_ClockEnable(cmuClock_HFLE, true);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
  CMU_ClockDivSet(MOD_SOM_SBE49_USART_CLK,cmuClkDiv_1);
  CMU_ClockEnable(MOD_SOM_SBE49_USART_CLK, true);    /* Enable device clock */


  //device communication peripheral pointer
  mod_som_sbe49_ptr->com_prf_ptr =
      (mod_som_sbe49_prf_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 communication prf",DEF_NULL,
          sizeof(mod_som_sbe49_prf_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->com_prf_ptr==DEF_NULL){
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  // define com port in the com struct.
  mod_som_sbe49_ptr->com_prf_ptr->handle_port = \
                              mod_som_sbe49_ptr->config_ptr->port.com_port;

  // making sure we have UART types defined
  // ALB I remove all the UART cases a kept only LEUART0 to match the SOM
  // ALB if needed it one can add another com port here
  mod_som_sbe49_ptr->com_prf_ptr->irqn = MOD_SOM_SBE49_USART_IRQ;


  leuart_ptr  = \
      (LEUART_TypeDef *) mod_som_sbe49_ptr->com_prf_ptr->handle_port;
  leuart_init.baudrate   = mod_som_sbe49_ptr->config_ptr->baud_rate;

  //parity set
  //ALB the switch statements are a legacy from the previous SBE49 module
  switch(mod_som_sbe49_ptr->config_ptr->port.parity){
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
  switch(mod_som_sbe49_ptr->config_ptr->port.data_bits){
    case mod_som_uart_data_bits_8:
      leuart_init.databits = leuartDatabits8;
      break;
    case mod_som_uart_data_bits_9:
      leuart_init.databits = leuartDatabits9;
      break;
  }
  //stop bits
  switch(mod_som_sbe49_ptr->config_ptr->port.stop_bits){
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
                           | (mod_som_sbe49_ptr->config_ptr->port.route
                           << _LEUART_ROUTELOC0_TXLOC_SHIFT)
                           | (mod_som_sbe49_ptr->config_ptr->port.route
                           << _LEUART_ROUTELOC0_RXLOC_SHIFT);

  //ALB enable the LEUART ROUTE.
  leuart_ptr->ROUTEPEN = LEUART_ROUTEPEN_TXPEN
                         | LEUART_ROUTEPEN_RXPEN;

  //ALB This is important!!
  //ALB It defines the last char of a SBE sample.
  //ALB LEUART will scan it and trigger a SIGFRAME interrupt.
  //ALB I use this interrupt to store the SBE data.
  leuart_ptr->SIGFRAME = MOD_SOM_SBE49_SAMPLE_LASTCHAR;

  //flush data from RX and TX.
  leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;

  /* Clear previous RX interrupts. */
  LEUART_IntClear(leuart_ptr, ~0x0);
  NVIC_ClearPendingIRQ(mod_som_sbe49_ptr->com_prf_ptr->irqn);

  //ALB Trick to use IRQ handlers without being system dependent.
  //ALB we define our own irq_f which points to the system irq.
  mod_som_sbe49_ptr->com_prf_ptr->irq_f = mod_som_sbe49_irq_rx_handler_f;

  mod_som_sbe49_ptr->com_prf_ptr->irq_extra_f=mod_som_sbe49_ldma_irq_handler_f;

  return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
}

/***************************************************************************//**
 * @brief
 *   Initialize LDMA
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_init_ldma_f(mod_som_sbe49_ptr_t module_ptr)
{
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  mod_som_sbe49_ptr->ldma.ch=MOD_SOM_SBE49_LDMA_CH;

  mod_som_sbe49_ptr->ldma.irq_f = mod_som_sbe49_ldma_irq_handler_f;
  mod_som_sbe49_ptr->ldma.irqn = MOD_SOM_SBE49_LDMA_IRQ;


  //ALB initialize the LDMA clock
  CMU_Clock_TypeDef sbe_ldma_clk = MOD_SOM_SBE49_LDMA_CLCK;
  CMU_ClockEnable(sbe_ldma_clk, true);
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  //ALB LDMA IRQ default priority is 3.
  LDMA_Init( &init );

  // set LDMA descriptors for Direct Memory access and transfer -> ADC to memory using CS cascade
  // Define LDMA_TransferCfg_t for ADC config LDMA transfer

  // ALB create read descriptor list
  // ALB this the main descriptor list used during the sampling.
  // ALB The LDMA transfer define by this list is called inside the GPIO interrupt handler
  // ALB after the ADC send their interrupt signal
  mod_som_sbe49_define_read_descriptor_f(module_ptr);

  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   construct rec_buff
 *
 *   uint8_t*  elements_buffer;
 *   uint32_t * elements_map; //TODO figure out a way to move it to rec_buff.
 *   uint32_t  producer_indx; // mod sample count % max sample per buffer
 *   uint8_t   rx_write_idx;
 *
 *  element= uint64_t timestamp * Nbytes of sbe_sample;
 *  N bytes depends on the SBE output format
 *
 * @param mod_som_sbe49_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_rec_buff_f(){

  RTOS_ERR  err;

  //producer record buffer
  mod_som_sbe49_ptr->rec_buff_ptr =
      (mod_som_sbe49_rec_buff_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 data rec. buff",DEF_NULL,
          sizeof(mod_som_sbe49_rec_buff_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->rec_buff_ptr==DEF_NULL){
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  // calculate and allocate record circular buffer
  mod_som_sbe49_ptr->rec_buff_ptr->elements_buffer = (uint8_t*)Mem_SegAlloc(
      "MOD SOM SBE49 data elements ",DEF_NULL,
      sizeof(uint8_t)*mod_som_sbe49_ptr->config_ptr->buffer_length,
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->rec_buff_ptr->elements_buffer==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  // calculate and allocate record circular buffer
  mod_som_sbe49_ptr->rec_buff_ptr->elements_map = (uint32_t*)Mem_SegAlloc(
      "MOD SOM SBE49 elements map ",DEF_NULL,
      sizeof(uint32_t)*mod_som_sbe49_ptr->config_ptr->elements_per_buffer,
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->rec_buff_ptr->elements_map==DEF_NULL)
    {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  //ALB initialize element buffer to 0
  for (int i=0; i< mod_som_sbe49_ptr->config_ptr->buffer_length; i++){
      mod_som_sbe49_ptr->rec_buff_ptr->elements_buffer[i]=0;
  }


  // Now that the memory for the circular buffer is allocated and initialized
  // fill the element map with the addresses of each elements.
  for (int i=0; i< mod_som_sbe49_ptr->config_ptr->elements_per_buffer; i++){
      mod_som_sbe49_ptr->rec_buff_ptr->elements_map[i] =                       \
          (uint32_t) &mod_som_sbe49_ptr->rec_buff_ptr->elements_buffer[        \
                                i*mod_som_sbe49_ptr->config_ptr->element_length];
  }


  // ALB intialize producer index.
  // ALB It will defined as sample_count % max number of sample in the element buffer
  mod_som_sbe49_ptr->rec_buff_ptr->producer_indx=0;

  return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
}


/*******************************************************************************
 * @brief
 *   construct consumer_buff
 *
 *  uint64_t  timestamp;
 *  uint32_t  record_length;           //ALB length of the streaming data buffer
 *  uint32_t  max_sample_per_record;   //ALB maximum element per stream block
 *  uint8_t * record_data_ptr;         //ALB pointer to stream data
 *  uint8_t   data_ready_flg;          //ALB ???
 *  uint8_t   chksum;
 *  uint32_t  elmnts_skipped;
 *  uint8_t   header[MOD_SOM_SBE49_MAX_HEADER_SIZE];//TODO move this to consumer_ptr
 *  uint8_t   length_header;//TODO move this to consumer_ptr
 *
 *
 * @param mod_som_sbe49_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_construct_consumer_ptr_f(){

  RTOS_ERR  err;

  // allocate memory for streaming_data_ptr
  mod_som_sbe49_ptr->consumer_ptr = (mod_som_sbe49_data_consumer_ptr_t)Mem_SegAlloc(
          "MOD SOM SBE49 consumer ptr",DEF_NULL,
          sizeof(mod_som_sbe49_data_consumer_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->consumer_ptr==DEF_NULL)
  {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  //ALB set sbe49_header length:
  //ALB 1 sync char          +
  //ALB 4 TAB char           +
  //ALB 16 char hextimestamp +
  //ALB 8 char hex payload   +
  //ALB 1 char sync chksum   +
  //ALB 2 char chksum        = 32 bytes

  mod_som_sbe49_ptr->consumer_ptr->length_header=
      MOD_SOM_SBE49_SYNC_HEADER_LENGTH     +
      MOD_SOM_SBE49_TAG_LENGTH             +
      MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH    +
      MOD_SOM_SBE49_SETTING_STR_LENGTH     +
      MOD_SOM_SBE49_HEADER_CHECKSUM_LENGTH;



  // allocate memory for the record data ptr consumer_data_ptr
  mod_som_sbe49_ptr->consumer_ptr->record_data_ptr = (uint8_t *)Mem_SegAlloc(
          "MOD SOM SBE49 consumer record ptr",DEF_NULL,
          mod_som_sbe49_ptr->consumer_ptr->length_header+\
          (mod_som_sbe49_ptr->config_ptr->max_element_per_record * \
           mod_som_sbe49_ptr->config_ptr->element_length)  + \
          MOD_SOM_SBE49_CHECKSUM_SIZE,
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_sbe49_ptr->consumer_ptr==DEF_NULL)
  {
      mod_som_sbe49_ptr = DEF_NULL;
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

  // initialize data_ready_flag
  mod_som_sbe49_ptr->consumer_ptr->data_ready_flg = 0;
  // initialize checksum
  mod_som_sbe49_ptr->consumer_ptr->chksum=0;
  // initialize element skipped
  mod_som_sbe49_ptr->consumer_ptr->elmnts_skipped=0;
  // initialize timestamp
  mod_som_sbe49_ptr->consumer_ptr->timestamp=0;

  //ALB initialize the CTD record
  for (int i=0;i<MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;i++)
    {
      mod_som_sbe49_ptr->consumer_ptr->record_pressure[i]=0;
      mod_som_sbe49_ptr->consumer_ptr->record_temp[i]=0;
      mod_som_sbe49_ptr->consumer_ptr->record_salinity[i]=0;

    }

  mod_som_sbe49_ptr->consumer_ptr->dPdt=0;
  mod_som_sbe49_ptr->consumer_ptr->dTdt=0;
  mod_som_sbe49_ptr->consumer_ptr->dSdt=0;

  mod_som_sbe49_ptr->consumer_ptr->Pmean=0;
  mod_som_sbe49_ptr->consumer_ptr->Tmean=0;
  mod_som_sbe49_ptr->consumer_ptr->Smean=0;

  //MHA initialize PTC state - 0 = drag screens retracted; 1 = extended
  mod_som_sbe49_ptr->consumer_ptr->ptc_state=0;

  return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));

}



/*******************************************************************************
 * @brief
 *   Initializes a connection to communication port indicated
 *   config LEAURT and SBE EN gpios
 *   enable the LEUART.
 *
 *   At the end of the function, if the SBE is connected it should spitting out data.
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_connect_f(){

  //Check if module is initialized before "connecting" (power on the SBE)
  if(!mod_som_sbe49_ptr->initialized_flag){
      mod_som_io_print_f("$%s: Device is not yet initialized!\r\n",mod_som_sbe49_ptr->settings_ptr->data_header_text);
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_INITIALIZED));
  }

  //    /* SBE Enable: configure the LEUART pins and SBE EN (send power to the SBE)*/
  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.en_port, mod_som_sbe49_ptr->config_ptr->port.en_pin,
                  gpioModePushPull, 1);

  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.rx_port, mod_som_sbe49_ptr->config_ptr->port.rx_pin,
                  gpioModeInput, 0);

  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.tx_port, mod_som_sbe49_ptr->config_ptr->port.tx_pin,
                  gpioModePushPull, 1);

  GPIO_PinModeSet(gpioPortC,0,gpioModePushPull, 1);


  /* enable LEUART */
  // ALB I voluntarily left this LEAURT_enable system dependent (i.e. not like SN did it)
  // ALB We are using the SOM main com port which is connected to the LEUART
  // ALB so it does not make sense for me to keep ALL the options (i.e UART0 UART1 ...) here.
  // ALB Maybe we should have a mod_som_main_comport_connect or something similar
  // TODO discuss it with MG and SN
  LEUART_Enable((LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port, leuartEnable);
  mod_som_sbe49_ptr->connected_flag = true;

  return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));

}

/*******************************************************************************
 * @brief
 *   diconnect the SBE to communication port indicated
 *   turn off (GPIO down)
 *
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_disconnect_f(){

  //Check if module is initialized before "connecting" (power on the SBE)
  if(!mod_som_sbe49_ptr->initialized_flag){
      mod_som_io_print_f("$%s: Device is not yet initialized!\r\n",mod_som_sbe49_ptr->settings_ptr->data_header_text);
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_INITIALIZED));
  }

  //    /* SBE Disable: configure the LEUART pins and SBE EN (send power to the SBE)*/
  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.en_port, mod_som_sbe49_ptr->config_ptr->port.en_pin,
                  gpioModePushPull, 0);

  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.rx_port, mod_som_sbe49_ptr->config_ptr->port.rx_pin,
                  gpioModePushPull, 0);

  GPIO_PinModeSet(mod_som_sbe49_ptr->config_ptr->port.tx_port, mod_som_sbe49_ptr->config_ptr->port.tx_pin,
                  gpioModePushPull, 0);

  GPIO_PinModeSet(gpioPortC,0,gpioModePushPull, 0);


  /* Finally enable it */
  LEUART_Enable((LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port, leuartDisable);
  mod_som_sbe49_ptr->connected_flag = false;

  return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));

}



/*******************************************************************************
 * @brief
 *   Enable data collection/streaming
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_start_collect_data_f(){

  //Check if module is initialized before collecting data through LEUART
  if(!mod_som_sbe49_ptr->initialized_flag){
      mod_som_io_print_f("$%s: Device is not yet initialized!\r\n",mod_som_sbe49_ptr->settings_ptr->data_header_text);
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_INITIALIZED));
  }
  if(mod_som_sbe49_ptr->collect_data_flag){
      mod_som_io_print_f("$%s: SBE task already running.\r\n Stop the current one first!\r\n",mod_som_sbe49_ptr->settings_ptr->data_header_text);
      return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_INITIALIZED));
  }


    LEUART_TypeDef  *leuart_ptr;

    leuart_ptr      = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
    leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;


    mod_som_sbe49_ptr->sample_count= 0;
    mod_som_sbe49_ptr->collect_data_flag = true;

    mod_som_sbe49_start_consumer_task_f();

    LEUART_FreezeEnable(leuart_ptr,true);
    LEUART_IntEnable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);//_RXDATAV);//LEUART_IEN_SIGF);
    NVIC_EnableIRQ(mod_som_sbe49_ptr->com_prf_ptr->irqn);
    LEUART_FreezeEnable(leuart_ptr,false);


    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
}


/*******************************************************************************
 * @brief
 *   Stop data collection
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_stop_collect_data_f(){


  if(!mod_som_sbe49_ptr->connected_flag){
        mod_som_sbe49_ptr->collect_data_flag = false;
        mod_som_io_print_f("$%s: Device is not connected yet!\r\n",mod_som_sbe49_ptr->settings_ptr->data_header_text);
        return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_CONNECTED));
    }

    LEUART_TypeDef  *leuart_ptr;

    //ALB stop consumer task
    mod_som_sbe49_stop_consumer_task_f();

    leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
    NVIC_DisableIRQ(mod_som_sbe49_ptr->com_prf_ptr->irqn);
    NVIC_ClearPendingIRQ(mod_som_sbe49_ptr->com_prf_ptr->irqn);
    LEUART_FreezeEnable(leuart_ptr,true);
    LEUART_IntDisable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);
    LEUART_IntClear(leuart_ptr,~0x0);
    leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
    LEUART_FreezeEnable(leuart_ptr,false);

    //reset counters
    mod_som_sbe49_ptr->sample_count= 0;
    mod_som_sbe49_ptr->rec_buff_ptr->producer_indx= 0;
    //TODO add the consumer index to the consumer_ptr duhhh!!!!
//    mod_som_sbe49_ptr->consumer_ptr->consumer_indx= 0;

    mod_som_sbe49_ptr->collect_data_flag = false;
    mod_som_sbe49_ptr->consumer_ptr->data_ready_flg= false;

    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
}

/*******************************************************************************
 * @brief
 *   a function to get the sbe id
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_sbe49_id_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  if (argc==1){
    printf("SBE49, %s.\r\n",(char*) mod_som_sbe49_ptr->settings_ptr->sn);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      strcpy(mod_som_sbe49_ptr->settings_ptr->sn,argv[1]);
      break;
    default:
      printf("format: sbe49.id S/N\r\n");
      break;
    }
  }
}




/*******************************************************************************
 * @brief  mod_som_sbe49_consumer_start_task_f
 *   start the consumer task (i.e. sbe49 stream consumer or SD store)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_sbe49_start_consumer_task_f(){

  RTOS_ERR err;
  // Consumer Task 2
  mod_som_sbe49_ptr->sample_count=0;
  mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt=0;
  mod_som_sbe49_ptr->consumer_ptr->consumed_flag=true;

   OSTaskCreate(&sbe49_consumer_task_tcb,
                        "sbe49 consumer task",
                        mod_som_sbe49_consumer_task_f,
                        DEF_NULL,
                        MOD_SOM_SBE49_CONSUMER_TASK_PRIO,
            &sbe49_consumer_task_stk[0],
            (MOD_SOM_SBE49_CONSUMER_TASK_STK_SIZE / 10u),
            MOD_SOM_SBE49_CONSUMER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_sbe49_stream_stop_task_f
 *   stop the stream task (i.e. sbe49 stream consumer)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_sbe49_stop_consumer_task_f(){

  RTOS_ERR err;

  OSTaskDel(&sbe49_consumer_task_tcb,
            &err);

  return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * @brief
 *   mod_som_sbe49_consumer_task_f
 *   conusmer task (i.e. sbe49 stream consumer or SD store consumer)
 *
 *   This task is a while loop, thus, it will run continuously once the task started
 *   cnsmr_cnt increases indefinitely until the task is stopped
 *
 *   - Every MOD_SOM_EFE_CONSUMER_DELAY the task will gather the EFE data stored
 *     in the circular buffer one element at a time.
 *   - Once there no more data available, compute the checksum and append it to the block (*FF\r\n)
 *   - The task gather the current timestamp,
 *   - Build a block header (EFEtimestamp,recordsize,elementskipped,voltage,errorflag)
 *   - Prefix the header to the block
 *   - Add the header for this block and send a message
 *     (header+block) to the IO stream task or SDIO stream task.
 *
 *   1 element    = 1 EFE sample = 3bytes x nb channels
 *   sample_count = producer count
 *   cnsmr_cnt    =
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

static  void  mod_som_sbe49_consumer_task_f(void  *p_arg){
    RTOS_ERR  err;

    //cb_elmnt_ptr curr_read_elmnt_ptr = test_cb_param_block.base_ptr;
    // get local sbe49 element ptr and local sbe49 streamer ptr.
    uint8_t * curr_data_ptr   = mod_som_sbe49_ptr->rec_buff_ptr->elements_buffer;
    uint8_t * curr_consumer_element_ptr = mod_som_sbe49_ptr->consumer_ptr->record_data_ptr+\
                                mod_som_sbe49_ptr->consumer_ptr->length_header;
    uint8_t * base_consumer_element_ptr = mod_som_sbe49_ptr->consumer_ptr->record_data_ptr+\
                                mod_som_sbe49_ptr->consumer_ptr->length_header;

//    int64_t cnsmr_cnt  = 0;
    int32_t cnsmr_indx = 0;
    uint64_t tick;

    mod_som_sbe49_sample_t curr_sbe_sample;


    int elmnts_avail=0, reset_cnsmr_cnt=0;
    int data_elmnts_offset=0;


    int padding = MOD_SOM_SBE49_CONSUMER_PADDING; // the padding should include the variance.


    //        printf("In Consumer Task 2\n");
    while (DEF_ON) {

        if (mod_som_sbe49_ptr->collect_data_flag){
            elmnts_avail = mod_som_sbe49_ptr->sample_count - mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt;  //calculate number of elements available have been produced
            // LOOP without delay until caught up to latest produced element
            while (elmnts_avail > 0)
              {
                // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
                // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
                // calculate the number of data we skipped, report number of elements skipped.
                // Reset the consumers cnt equal with producer data plus padding
                if (elmnts_avail>(mod_som_sbe49_ptr->config_ptr->elements_per_buffer)){ // checking over flow. TODO check adding padding is correct.
                    // reset the consumer count less one buffer than producer count plus padding
                    //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                    reset_cnsmr_cnt = mod_som_sbe49_ptr->sample_count - mod_som_sbe49_ptr->config_ptr->elements_per_buffer + padding;
                    // calculate the number of skipped elements
                    mod_som_sbe49_ptr->consumer_ptr->elmnts_skipped = reset_cnsmr_cnt - mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt;

                    mod_som_io_print_f("\n sbe49 stream task: CB overflow: sample count = %lu,cnsmr_cnt = %lu,skipped %lu elements, ", \
                           (uint32_t)mod_som_sbe49_ptr->sample_count, \
                           (uint32_t) mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt, \
                           mod_som_sbe49_ptr->consumer_ptr->elmnts_skipped);

                    mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt = reset_cnsmr_cnt;
//                    printf("new cns_cnt: %lu\n",(uint32_t) cnsmr_cnt);
                }

                // calculate the offset for current pointer
                data_elmnts_offset     = mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt % mod_som_sbe49_ptr->config_ptr->elements_per_buffer;

                // update the current element pointer using the element map
                curr_data_ptr   =(uint8_t*)mod_som_sbe49_ptr->rec_buff_ptr->elements_map[data_elmnts_offset];

                //ALB move the stream ptr to the next element
                curr_consumer_element_ptr = base_consumer_element_ptr + \
                    (mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt-reset_cnsmr_cnt)*\
                    mod_som_sbe49_ptr->config_ptr->element_length;

                //ALB copy the the local element in the streamer
                memcpy(curr_consumer_element_ptr,curr_data_ptr,mod_som_sbe49_ptr->config_ptr->element_length);

                switch (mod_som_sbe49_ptr->settings_ptr->data_format){
                  case 3:
                    curr_sbe_sample=mod_som_sbe49_parse_sample_f(curr_consumer_element_ptr);

                    mod_som_sbe49_ptr->consumer_ptr->record_pressure[cnsmr_indx]= \
                        curr_sbe_sample.pressure;
                    mod_som_sbe49_ptr->consumer_ptr->record_salinity[cnsmr_indx]= \
                        curr_sbe_sample.salinity;
                    mod_som_sbe49_ptr->consumer_ptr->record_temp[cnsmr_indx]= \
                        curr_sbe_sample.temperature;
                    mod_som_sbe49_ptr->consumer_ptr->record_timestamp[cnsmr_indx]= \
                        curr_sbe_sample.timestamp;
                    break;
                  case 0: //MHA this was 1.
                    //ALB Parse and convert raw engineering data.
                    //ALB Make sure you get the cal-coef first

                    //MHA This appears to be past the beginning of the record.  Experiment with simply adding 16.
                    mod_som_sbe49_obp_parsing_sample((char *) (curr_consumer_element_ptr+16),
                                           mod_som_sbe49_ptr->obp_ptr->cal_coef,
                  &mod_som_sbe49_ptr->consumer_ptr->record_pressure[cnsmr_indx],
                  &mod_som_sbe49_ptr->consumer_ptr->record_temp[cnsmr_indx],
                  &mod_som_sbe49_ptr->consumer_ptr->record_cond[cnsmr_indx],
                  &mod_som_sbe49_ptr->consumer_ptr->record_salinity[cnsmr_indx]);
                    break;
                }

                //MHA Now some simple logic for the PTC state machine.
                //Working temporarily with a new bool field in the conumer structure called
                //ptc_state (0 for retracted, 1 for extended).
                //For the time being the depth pcut is hard coded.  We will issue PTC when
                //pressure is greater than pcut and the state is 0 (retracted).

                //Now set this to 1 to enable the pressure-dependent PTC code
                if (mod_som_sbe49_ptr->obp_ptr->ptc_depth>0) {
                if (mod_som_sbe49_ptr->consumer_ptr->record_pressure[cnsmr_indx] >
                    mod_som_sbe49_ptr->obp_ptr->ptc_depth &&
                  ! mod_som_sbe49_ptr->consumer_ptr->ptc_state )
                  {
                    // ALB we want to extend the actuator at its maximum
                    // ALB I am issuing 3 cmds: 100 %, 100% and 100%
                    mod_som_actuator_ptc_f();

                    //set PTC state to 1 - this should be done later by communicating with the actuator structure
                    mod_som_sbe49_ptr->consumer_ptr->ptc_state=1;
                  }

                //Can insert more logic here to issue the CTC command on the way up.
                //If we are less than pcut_up and the screens are extended, then retract.
                if (mod_som_sbe49_ptr->consumer_ptr->record_pressure[cnsmr_indx] <
                    mod_som_sbe49_ptr->obp_ptr->ctc_depth &&
                    mod_som_sbe49_ptr->consumer_ptr->ptc_state )
                  {
                    //
                    // ALB we want to extend the actuator at its MINIMUM
                    // ALB I am issuing 3 cmds: 0 %, 0% and 0%
                    mod_som_actuator_ctc_f();


                    //set PTC state to 0 - this should be done later by communicating with the actuator structure
                    mod_som_sbe49_ptr->consumer_ptr->ptc_state=0;
                  }
                }

                mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt++;  // increment cnsmr count
                cnsmr_indx=mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt % mod_som_sbe49_ptr->config_ptr->max_element_per_record;  // increment cnsmr count
                elmnts_avail = mod_som_sbe49_ptr->sample_count - mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt; //elements available have been produced
                //ALB This IF check if we have enough element in the consumer record
                //ALB to be streamed/SD stored.
                if(cnsmr_indx ==0){
                    mod_som_sbe49_ptr->consumer_ptr->data_ready_flg=1;
                    break;
                }
              }  // end of while (elemts_avail > 0)
            // No more data available. All data are stored in the stream buffer.


            if (mod_som_sbe49_ptr->consumer_ptr->data_ready_flg &
                mod_som_sbe49_ptr->consumer_ptr->consumed_flag){

                // We are almost ready to send. Just need to get the header, compute the chcksum, append it
                // to the stream buffer and send to the stream task

                //ALB move the stream ptr to the position of the checksum
                curr_consumer_element_ptr = base_consumer_element_ptr + \
                    (mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt-reset_cnsmr_cnt)*\
                    mod_som_sbe49_ptr->config_ptr->element_length;

                //ALB get the length of the stream block
                mod_som_sbe49_ptr->consumer_ptr->payload_length= \
                    (int) &curr_consumer_element_ptr[0]- \
                    (int) base_consumer_element_ptr;

                //get the timestamp for the record header
                tick=sl_sleeptimer_get_tick_count64();
                mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                                      &mod_som_sbe49_ptr->timestamp);

                //MHA: Now augment timestamp by poweron_offset_ms
                mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
                mod_som_sbe49_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;


                if (mod_som_sbe49_ptr->settings_ptr->data_format==3){
                    // compute time derivative of temp, pressure and salinity
                    // over the consumer record
                    mod_som_sbe49_ctd_variation_f();
                    mod_som_sbe49_ctd_mean_f();
                }

                //create header
                mod_som_sbe49_header_f(mod_som_sbe49_ptr->consumer_ptr);
                //add header to the beginning of the stream block
                memcpy(mod_som_sbe49_ptr->consumer_ptr->record_data_ptr, \
                       mod_som_sbe49_ptr->consumer_ptr->header,
                       mod_som_sbe49_ptr->consumer_ptr->length_header);


                mod_som_sbe49_ptr->consumer_ptr->chksum=0;
                for (int i=0;i<mod_som_sbe49_ptr->consumer_ptr->payload_length;i++)
                  {
                    mod_som_sbe49_ptr->consumer_ptr->chksum ^=\
                        base_consumer_element_ptr[i];
                  }


                // the curr_consumer_element_ptr should be at the right place to
                // write the checksum already
                //write checksum at the end of the steam block (record).
                *(curr_consumer_element_ptr++) = '*';
                *((uint16_t*)curr_consumer_element_ptr) = \
                    mod_som_int8_2hex_f(mod_som_sbe49_ptr->consumer_ptr->chksum);
                curr_consumer_element_ptr += 2;
                *(curr_consumer_element_ptr++) = '\r';
                *(curr_consumer_element_ptr++) = '\n';


                //ALB get the length of the stream block
                mod_som_sbe49_ptr->consumer_ptr->record_length= \
                    (int) &curr_consumer_element_ptr[0]- \
                    (int) &mod_som_sbe49_ptr->consumer_ptr->record_data_ptr[0];

                //ALB Here we send the record to other task:
                //ALB streaming,SD store, aggregate, obp.
                switch(mod_som_sbe49_ptr->consumer_mode){
                  case 0:
                    mod_som_sbe49_ptr->consumer_ptr->consumed_flag=false;

                    mod_som_io_stream_data_f(
                        mod_som_sbe49_ptr->consumer_ptr->record_data_ptr,
                        mod_som_sbe49_ptr->consumer_ptr->record_length,
                        &mod_som_sbe49_ptr->consumer_ptr->consumed_flag);
                    break;
                  case 1:
                    mod_som_sbe49_ptr->consumer_ptr->consumed_flag=false;
                    mod_som_sdio_write_data_f(
                        mod_som_sbe49_ptr->consumer_ptr->record_data_ptr,
                        mod_som_sbe49_ptr->consumer_ptr->record_length,
                        &mod_som_sbe49_ptr->consumer_ptr->consumed_flag);
                    break;
                  case 2:
                    printf("SBE aggreg \r\n");
//                    mod_som_sbe49_ptr->consumer_ptr->consumed_flag=false;
//
//                    mod_som_aggregator_process_data_f(
//                        mod_som_sbe49_ptr->consumer_ptr->record_data_ptr,
//                        mod_som_sbe49_ptr->consumer_ptr->record_length,
//                        &mod_som_sbe49_ptr->consumer_ptr->consumed_flag);
                    break;
                  default:
                    break;
                }

                mod_som_sbe49_ptr->consumer_ptr->elmnts_skipped = 0;
                // reset the stream ptr.
                curr_consumer_element_ptr = base_consumer_element_ptr;

                //ALB update reset_cnsmr_cnt so we can fill the stream block from 0 again
                reset_cnsmr_cnt=mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt;
                mod_som_sbe49_ptr->consumer_ptr->data_ready_flg=0;

            }//end if (mod_som_sbe49_ptr->collect_data_flag)
        } // data_ready_flg

        // Delay Start Task execution for
        OSTimeDly( MOD_SOM_SBE49_CONSUMER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                   OS_OPT_TIME_DLY,          //   from now.
                   &err);
        //   Check error code.
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
    } // end of while (DEF_ON)

    PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.
}

//static  void  mod_som_sbe49_store_start_task_f (){
//
//}


void mod_som_sbe49_set_actu_depth_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  RTOS_ERR  p_err;

  if (argc==1){
      printf("sbe.actu ptc:%lu m ctc:%lu m\r\n.",
             mod_som_sbe49_ptr->obp_ptr->ptc_depth,
             mod_som_sbe49_ptr->obp_ptr->ctc_depth);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 3:
      mod_som_sbe49_ptr->obp_ptr->ptc_depth=shellStrtol(argv[1],&p_err);
      mod_som_sbe49_ptr->obp_ptr->ctc_depth=shellStrtol(argv[2],&p_err);
      break;
    default:
      printf("format: sbe.actu ptc_depth ctc_depth (Depths have to be integer\r\n");
      printf("format: if ptc_depth==0 auto_ptc is disabled.\r\n");
      break;
    }
  }
}


/***************************************************************************//**
 * @brief
 *   parse sbe sample str to get temp, cond,pressure, salinity.
 *   TODO handle all the output format available
 ******************************************************************************/
mod_som_sbe49_sample_t mod_som_sbe49_parse_sample_f(uint8_t * element)
{
  mod_som_sbe49_sample_t sbe_sample;
  char  str_temperature[MOD_SOM_SBE49_SBE_CHANNEL_LENGTH];
  char  str_conductivity[MOD_SOM_SBE49_SBE_CHANNEL_LENGTH];
  char  str_pressure[MOD_SOM_SBE49_SBE_CHANNEL_LENGTH];
  char  str_salinity[MOD_SOM_SBE49_SBE_CHANNEL_LENGTH];

  sbe_sample.timestamp=0;
  memcpy(&sbe_sample.timestamp,element,sizeof(uint64_t));

  uint8_t * sbe_str=&element[MOD_SOM_SBE49_TIMESTAMP_LENGTH];
  // parse sample
  for (int i=0;i<MOD_SOM_SBE49_SBE_CHANNEL_LENGTH;i++){
      str_temperature[i]  = sbe_str[i];
      str_conductivity[i] = sbe_str[1*MOD_SOM_SBE49_SBE_CHANNEL_LENGTH+i+1];
      str_pressure[i]     = sbe_str[2*MOD_SOM_SBE49_SBE_CHANNEL_LENGTH+i+3];
      str_salinity[i]     = sbe_str[3*MOD_SOM_SBE49_SBE_CHANNEL_LENGTH+i+5];
  }

  sbe_sample.temperature=strtof(str_temperature,NULL);
  sbe_sample.conductivity=strtof(str_conductivity,NULL);
  sbe_sample.pressure=strtof(str_pressure,NULL);
  sbe_sample.salinity=strtof(str_salinity,NULL);

  return sbe_sample;
}

/***************************************************************************//**
 * @brief
 *   compute time derivative of temp, salinity, pressure within the consumer record.
 ******************************************************************************/
void mod_som_sbe49_ctd_variation_f()
{
  float * pressure    = mod_som_sbe49_ptr->consumer_ptr->record_pressure;
  float * temperature = mod_som_sbe49_ptr->consumer_ptr->record_temp;
  float * salinity    = mod_som_sbe49_ptr->consumer_ptr->record_salinity;

  uint64_t * timestamp = &mod_som_sbe49_ptr->consumer_ptr->timestamp;
  uint32_t dt=(timestamp[MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD] -
               timestamp[0]);
  mod_som_sbe49_ptr->consumer_ptr->dPdt=\
                     (pressure[MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD]- \
                      pressure[0])/dt;

  mod_som_sbe49_ptr->consumer_ptr->dTdt= \
      (temperature[MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD]- \
                           temperature[0])/dt;

  mod_som_sbe49_ptr->consumer_ptr->dSdt=(salinity[MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD]- \
                           salinity[0])/dt;
}


/***************************************************************************//**
 * @brief
 *   parse sbe sample str to get temp, cond,pressure, salinity.
 *   TODO handle all the output format available
 ******************************************************************************/
void mod_som_sbe49_ctd_mean_f()
{
  float * pressure    = mod_som_sbe49_ptr->consumer_ptr->record_pressure;
  float * temperature = mod_som_sbe49_ptr->consumer_ptr->record_temp;
  float * salinity    = mod_som_sbe49_ptr->consumer_ptr->record_salinity;

  mod_som_sbe49_ptr->consumer_ptr->Pmean=0;
  mod_som_sbe49_ptr->consumer_ptr->Tmean=0;
  mod_som_sbe49_ptr->consumer_ptr->Smean=0;

  for (int i=0;i<MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;i++){
      mod_som_sbe49_ptr->consumer_ptr->Pmean+=pressure[i];
      mod_som_sbe49_ptr->consumer_ptr->Tmean+=temperature[i];
      mod_som_sbe49_ptr->consumer_ptr->Smean+=salinity[i];
  }
  mod_som_sbe49_ptr->consumer_ptr->Pmean/=   \
                                  MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;
  mod_som_sbe49_ptr->consumer_ptr->Tmean/=    \
                                  MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;
  mod_som_sbe49_ptr->consumer_ptr->Smean/=    \
                                  MOD_SOM_SBE49_DATA_DEFAULT_SAMPLES_PER_RECORD;

}


/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_sbe49_header_f(mod_som_sbe49_data_consumer_ptr_t consumer_ptr)
{

  //time stamp
  uint32_t t_hex[2];
  uint8_t * local_header;

  t_hex[0] = (uint32_t) (mod_som_sbe49_ptr->timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_sbe49_ptr->timestamp;

  //header  contains SBE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
      sprintf((char*) consumer_ptr->header,  \
          "$%s%08x%08x%08x*FF", \
          mod_som_sbe49_ptr->settings_ptr->data_header_text, \
          (int) t_hex[0],\
          (int) t_hex[1],\
          (int) consumer_ptr->payload_length);

      consumer_ptr->header_chksum=0;
      for(int i=0;i<mod_som_sbe49_ptr->consumer_ptr->length_header-
                    MOD_SOM_SBE49_HEADER_CHECKSUM_LENGTH;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
        {
          consumer_ptr->header_chksum ^=\
              consumer_ptr->header[i];
        }


      // the curr_consumer_element_ptr should be at the right place to
      // write the checksum already
      //write checksum at the end of the steam block (record).
      local_header = &consumer_ptr->header[
                                 mod_som_sbe49_ptr->consumer_ptr->length_header-
                                 MOD_SOM_SBE49_HEADER_CHECKSUM_LENGTH+1];
      *((uint16_t*)local_header) = \
          mod_som_int8_2hex_f(consumer_ptr->header_chksum);


}



/***************************************************************************//**
 * @brief
 *   descriptor list to organize the LDMA transfer of 1 SBE sampled
 *
 *   State machine: Fix length algorithm.
 *
 *   I want to read 1 block of SBE data.
 *
 *   I
 *
 *
 *   state 1: read X char (e.g. 24 char) but last char is not \n
 *            find where \n is in the block. Save the char after \n in the element buffer
 *   state 0: read the last
 *
 *   state 2:
 *
 *
 ******************************************************************************/

void mod_som_sbe49_define_read_descriptor_f(mod_som_sbe49_ptr_t module_ptr)
{

    descriptor_read_sbe = sbe_ldma_read_rx;
    descriptor_read_sbe.xfer.dstAddr=(uint32_t) \
                            (mod_som_sbe49_ptr->rec_buff_ptr->elements_map[0]+ \
                            MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH);
    descriptor_read_sbe.xfer.dstInc=ldmaCtrlDstIncOne;
    descriptor_read_sbe.xfer.srcAddr=(uint32_t) (&LEUART0->RXDATA);
    descriptor_read_sbe.xfer.xferCnt= \
                            mod_som_sbe49_ptr->config_ptr->sample_data_length-1;
}



///*******************************************************************************
// * @brief
// *   Send stuff out
// *
// * @param mod_som_sbe49_ptr
// *   runtime device pointer where data can be stored and communication is done
// ******************************************************************************/
//mod_som_status_t mod_som_sbe49_send_to_device_f(const uint8_t* data_to_send, uint32_t data_length){
//    if(!mod_som_sbe49_ptr->connected_flag){
//        mod_som_io_print_f("$%s: Device is not connected yet!\r\n",mod_som_sbe49_ptr->config_ptr->stat_header_text);
//        return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_NOT_CONNECTED));
//    }
//
//    LEUART_TypeDef  *leuart_ptr;
//    USART_TypeDef   *usart_ptr;
//
//    uint32_t i;
//
//    switch((uint32_t)mod_som_sbe49_ptr->com_prf_ptr->handle_port){
//    case USART0_BASE:
//    case USART1_BASE:
//    case USART2_BASE:
//    case USART3_BASE:
//    case USART4_BASE:
//    case USART5_BASE:
//    case UART0_BASE:
//    case UART1_BASE:
//        usart_ptr  = (USART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
//        usart_ptr->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
//        for(i=0; i<data_length; i++){
//            while(!(usart_ptr->STATUS & USART_STATUS_TXIDLE));
//            USART_Tx(usart_ptr,data_to_send[i]);
//            while(!(usart_ptr->STATUS & USART_STATUS_TXC));
//        }
//        break;
//    case LEUART0_BASE:
//    case LEUART1_BASE:
//        leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
//        leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
//        for(i=0; i<data_length; i++){
//            while(!(leuart_ptr->STATUS & LEUART_STATUS_TXIDLE));
//            LEUART_Tx(leuart_ptr,data_to_send[i]);
//            while(!(leuart_ptr->STATUS & LEUART_STATUS_TXC));
//        }
//        break;
//    }
//    return (mod_som_sbe49_ptr->status = mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK));
//
//
//    //    mod_som_io_print_f("\r\nDone send to device.\r\n");
//}

//------------------------------------------------------------------------------
// local functions
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   call back function when the LEUART driver receive call is non blocking
 *   if we enter in this IRQ:
 *      - mod_som_sbe49_ptr->collect_data_flag has to be true, because the IRG would not be enable otherwise
 *      - TODO use the SBE sample fixed length to create a good state machine.
 *      -
 *
 *   save IF flag
 *   clear IF register
 *   if SIGFRAME: grab the timestamp
 *   if RXDATAV: store the SBE sample in rec_buff
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
void mod_som_sbe49_irq_rx_handler_f(){

  LEUART_TypeDef  *leuart_ptr;
//  uint8_t indx    =0;

  uint32_t interrupt_sig;
  leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
  // Disabling the LEUART IRQ to "run" on the LDMA interrupt.
  LEUART_IntDisable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);

  interrupt_sig = leuart_ptr->IF;
  LEUART_IntClear(leuart_ptr,interrupt_sig);


  if(interrupt_sig & LEUART_IF_SIGF){

      //ALB This happens once at the beginning right after sbe.start cmd.
      //ALB ensure we are at the beginng of a sbe sample when we sbe.start
      //ALB if collect_data_flag is false I clear LEUART RX in the following IF.
      if(!mod_som_sbe49_ptr->collect_data_flag){
          mod_som_sbe49_ptr->sample_count=0;
          mod_som_sbe49_ptr->consumer_ptr->cnsmr_cnt=0;
      }
          //Clear SIGFRAME to start the transfer(data storing) with the begining of a SBE sample
          // i.e. not a \n.
          leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
          LDMA_StartTransfer( mod_som_sbe49_ptr->ldma.ch ,\
                              (void*)&sbe_ldma_signal, \
                              (void*)&descriptor_read_sbe);
          // Disabling the LEUART IRQ to "run" on the LDMA interrupt.
          LEUART_IntDisable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);
  }
  // enter this IF only at the  begining to catch SIGFRAME
  else if(interrupt_sig & LEUART_IF_RXDATAV){
      //ALB I am not storing data yet
      //ALB I need to clear the FIFO until I get SIGFRAME
      leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;

      // Disabling the LEUART IRQ to "run" on the LDMA interrupt.
      LEUART_IntEnable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);
  }
}



/*******************************************************************************
 * @brief
 *
 * This "local" IRQ handler is called in mod_som.c
 * TODO manage IRQhandler better.
 *
 ******************************************************************************/
void mod_som_sbe49_ldma_irq_handler_f(){

  LEUART_TypeDef  *leuart_ptr;
  uint64_t        tick;
  char *   local_elements_map;
  char        hextimestamp[MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH+2];


  uint32_t t_hex[2];

  leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;

  //ALB normal case
  if((*(uint8_t*)(mod_som_sbe49_ptr->rec_buff_ptr->elements_map \
             [mod_som_sbe49_ptr->rec_buff_ptr->producer_indx]+ \
                            MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH+ \
    mod_som_sbe49_ptr->config_ptr->sample_data_length-1)==10) & \
                           (mod_som_sbe49_ptr->collect_data_flag))
    {
      tick=sl_sleeptimer_get_tick_count64();
      mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                            &mod_som_sbe49_ptr->timestamp);
      //MHA: Now augment timestamp by poweron_offset_ms
      mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
      mod_som_sbe49_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

      if(mystatus != SL_STATUS_OK)
        {
          //ALB TODO handle get timestamps errors
        }
      //need to cast local_elements_map as uint64_t.
      local_elements_map=(char *) mod_som_sbe49_ptr->rec_buff_ptr->elements_map \
          [mod_som_sbe49_ptr->rec_buff_ptr->producer_indx];

      //ALB store the timestamp at at the BEGINNING of the SBE element.

      t_hex[0] = (uint32_t) (mod_som_sbe49_ptr->timestamp>>32);
      t_hex[1] = (uint32_t) mod_som_sbe49_ptr->timestamp;

      sprintf(hextimestamp,  \
          "%08x%08x", \
          (int) t_hex[0],\
          (int) t_hex[1]);

      memcpy(local_elements_map, \
             hextimestamp,\
             MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH);

      //ALB increment sample count
      mod_som_sbe49_ptr->sample_count++;
      //ALB adjust producer_indx
      mod_som_sbe49_ptr->rec_buff_ptr->producer_indx= \
          mod_som_sbe49_ptr->sample_count % \
          MOD_SOM_SBE49_DATA_SAMPLES_PER_BUFFER;


      //update the adresse in the ldma descriptor list
      descriptor_read_sbe.xfer.dstAddr=(uint32_t) ( \
                                 mod_som_sbe49_ptr->rec_buff_ptr->elements_map \
                             [mod_som_sbe49_ptr->rec_buff_ptr->producer_indx]+ \
                                                MOD_SOM_SBE49_HEXTIMESTAMP_LENGTH);


      LDMA_StartTransfer( mod_som_sbe49_ptr->ldma.ch ,\
                          (void*)&sbe_ldma_signal, \
                          (void*)&descriptor_read_sbe);

    }else{
        //reset the LEUART IRQ and look for the next SBE sample.
        LEUART_IntClear(leuart_ptr,LEUART_IF_SIGF|LEUART_IF_RXDATAV);
        LEUART_IntEnable(leuart_ptr, LEUART_IF_SIGF|LEUART_IF_RXDATAV);
        //ALB I am not storing data yet
        //ALB I need to clear the FIFO until I get SIGFRAME
        leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;

        //TODO update the error flag in the header.
//        mod_som_sbe49_ptr->collect_data_flag = false;
    }
}


/*******************************************************************************
 * @brief
 *
 *  TODO
 *
 ******************************************************************************/

void mod_som_sbe49_irq_tx_handler_f(){
//    mod_som_sbe49_ptr_t this_device_ptr = (mod_som_sbe49_ptr_t) mod_som_sbe49_ptr;
    //    printf("\r\ndone_tx_announcement_f\r\n");
}


mod_som_status_t mod_som_sbe49_consumer_mode_f(CPU_INT16U argc,CPU_CHAR *argv[])
{
  RTOS_ERR  p_err;

  if (argc==1){
      printf("sbe.mode %u\r\n.", mod_som_sbe49_ptr->consumer_mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_sbe49_ptr->consumer_mode=shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: sbe.mode mode (0:stream, 1:SD store, 2: on board processing)\r\n");
      break;
    }
  }
  return MOD_SOM_STATUS_OK;
}

mod_som_status_t mod_som_sbe49_output_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  return MOD_SOM_STATUS_OK;
}

mod_som_status_t mod_som_sbe49_gate_f(CPU_INT16U argc,CPU_CHAR *argv[]){

  LEUART_TypeDef  *leuart_ptr;
  leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
  char CR= '\r';
  char NL= '\n';
  char SBEchar = 0;
  char* SBEstop = "stop";
  char* SBEdcal = "dcal";
  char* SBEds = "ds";
  uint32_t delay=0xF00000;

  if(!mod_som_sbe49_ptr->connected_flag){
      mod_som_sbe49_connect_f();
      //ALB Wait until the SBE start spitting out data.
      while (delay>0){
          delay--;
      }
      //ALB sure the SBE is stopped
      for (int i=0;i<strlen(SBEstop);i++){
          LEUART_Tx(leuart_ptr, (uint8_t) SBEstop[i]);
      }
      //ALB carriage return, new line to "execute" the cmd on the SBE.
      LEUART_Tx(leuart_ptr, CR);
      LEUART_Tx(leuart_ptr, NL);
      while (delay>0){
          delay--;
      }
    }


  if (argc==1){
      printf("format: sbe.gate sbe command \r\n.");
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      //ALB clear TX RX before sending the cmd
      leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
      //ALB send the cmd
      for (int i=0;i<strlen(argv[1]);i++){
          LEUART_Tx(leuart_ptr, (uint8_t) argv[1][i]);
      }
      //ALB carriage return, new line to "execute" the cmd on the SBE.
      LEUART_Tx(leuart_ptr, CR);
      LEUART_Tx(leuart_ptr, NL);

      // Specific case of dcal. So far only returning the SBE char
      // TODO store the dcal values
      if (strcmp(argv[1],SBEdcal)==0){
          while (SBEchar!=62){
              SBEchar=LEUART_Rx(leuart_ptr);
              printf("%c",SBEchar);
          }
      }
      // Specific case of ds. So far only returning the SBE char
      // TODO store the ds values
      if (strcmp(argv[1],SBEds)==0){
          while (SBEchar!=62){
              SBEchar=LEUART_Rx(leuart_ptr);
              printf("%c",SBEchar);
          }
      }


      break;
    default:
      printf("format: sbe.gate sbe command \r\n.");
      break;
    }
  }



  return MOD_SOM_STATUS_OK;
}

//MHA don't forget to define in .h
void mod_som_sbe49_call_get_dcal_f()
{
  //MHA call the get cal function with argument
  mod_som_status_t foo=mod_som_sbe49_get_dcal_f(mod_som_sbe49_ptr->obp_ptr->cal_coef);
}


mod_som_status_t mod_som_sbe49_get_dcal_f(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef){

  LEUART_TypeDef  *leuart_ptr;
  leuart_ptr  = (LEUART_TypeDef *)mod_som_sbe49_ptr->com_prf_ptr->handle_port;
  char CR= '\r';
  char NL= '\n';
  char SBEchar = 0;
  char* SBEdcal = "dcal";
  char* SBEstop = "stop";
  char* SBE_str_dcal;
  uint32_t delay=0xF00000;
  char *tok_str_ptr;
  int j=0;

  //MHA hard code a string with the SBE49 calibration for testing
char* sbecal_test="  SBE 49 FastCAT V 1.1a  SERIAL NO. 0058"
"  temperature:  10-dec-17"
"      TA0 = 8.646787e-04"
"      TA1 = 2.862445e-04"
"      TA2 = -2.466554e-06"
"      TA3 = 2.121614e-07"
"      TOFFSET = 0.000000e+00"
"  conductivity:  10-dec-17"
"      G = -9.727914e-01"
"      H = 1.412282e-01"
"      I = -2.100128e-04"
"      J = 3.888261e-05"
"      CPCOR = -9.570000e-08"
"      CTCOR = 3.250000e-06"
"      CSLOPE = 1.000000e+00"
"  pressure S/N = 3027, range = 1450 psia:  05-dec-17"
"      PA0 = -9.985992e-02"
"      PA1 = 4.516155e-03"
"      PA2 = -1.856201e-11"
"      PTCA0 = 5.212094e+05"
"      PTCA1 = -2.710947e+00"
"      PTCA2 = 6.387558e-02"
"      PTCB0 = 2.444488e+01"
"      PTCB1 = -6.250000e-04"
"      PTCB2 = 0.000000e+00"
"      PTEMPA0 = -5.789793e+01"
"      PTEMPA1 = 5.622564e+01"
"      PTEMPA2 = -9.682869e-01"
"      POFFSET = 0.000000e+00";

  SBE_str_dcal=calloc(MOD_SOM_SBE49_STR_DCAL_LENGTH,sizeof(char));

  //MHA for now, we have no SBE so don't try to get this from the SBE49 - we will instead copy it from the string above
  //MHA 0 for fake SBE and 1 for real sbe
if (1) {
  if(!mod_som_sbe49_ptr->connected_flag){
      mod_som_sbe49_connect_f();
      //ALB Wait until the SBE start spitting out data.
      //ALB good practice should be sbe.gate stop as the first cmd.
      while (delay>0){
          delay--;
      }
      //MHA kluge: increase delay 3x as a stop gap to avoid stop command getting sent before SBE starting.  San suggests here, wait and check for SBE output data before stopping.
      while (delay>0){
          delay--;
      }
      while (delay>0){
          delay--;
      }
    }

  //Stop SBE
  //ALB clear TX RX before sending the cmd
  leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
  //ALB send the stop cmd
  for (int i=0;i<strlen(SBEstop);i++){
      LEUART_Tx(leuart_ptr, (uint8_t) SBEstop[i]);
  }
  //ALB carriage return, new line to "execute" the cmd on the SBE.
  LEUART_Tx(leuart_ptr, CR);
  LEUART_Tx(leuart_ptr, NL);

  while (delay>0){
      delay--;
  }


  //Send Dcal cmd
  //ALB clear TX RX before sending the cmd
  leuart_ptr->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;
  //ALB send the cmd
  for (int i=0;i<strlen(SBEdcal);i++){
      LEUART_Tx(leuart_ptr, (uint8_t) SBEdcal[i]);
  }
  //ALB carriage return, new line to "execute" the cmd on the SBE.
  LEUART_Tx(leuart_ptr, CR);
  LEUART_Tx(leuart_ptr, NL);

  //Read and store ASCII dcal
  // Specific case of dcal. So far only returning the SBE char
  // TODO store the dcal values
  j=0;
  while (SBEchar!=62){
      SBEchar=LEUART_Rx(leuart_ptr);
      printf("%c",SBEchar);
      *(SBE_str_dcal+j)=SBEchar;
      //strncpy(SBE_str_dcal+j,SBEchar,1);
//      SBE_str_dcal++;
      j++;
  }
} else {
  //MHA copy the hard coded string to the location - need to replace this with the code
  //above when we are talking to a real SBE49.
//  for (int i=0;i<MOD_SOM_SBE49_STR_DCAL_LENGTH;i++){
//      *(SBE_str_dcal++)=*(sbecal_test+i);
//  }
    strncpy(SBE_str_dcal,sbecal_test,MOD_SOM_SBE49_STR_DCAL_LENGTH);
}
  //MHA From here we will parse the info and dispatch it to the structure.

//MHA The sbecal_test below all need to be replaced by SBE_str_dcal

  //Now go in one by one and read in and store each value into the calibration structure.
  //SN
  tok_str_ptr=strstr(SBE_str_dcal,"SERIAL NO.");
  strncpy(cal_coef->serialnum,tok_str_ptr+11,5);//Then copy that string beginning after the delimiter (11 in this case), copying 4 bytes in this case.
  //Cal dates
  tok_str_ptr=strstr(SBE_str_dcal,"temperature:");
  strncpy(cal_coef->tcalDate,tok_str_ptr+12,12);
  tok_str_ptr=strstr(SBE_str_dcal,"conductivity:");
  strncpy(cal_coef->ccalDate,tok_str_ptr+13,12);
  tok_str_ptr=strstr(SBE_str_dcal,"psia:");
  strncpy(cal_coef->pcalDate,tok_str_ptr+5,12);

  tok_str_ptr=strstr(SBE_str_dcal,"TA0 = ");
  cal_coef->ta0=strtof(tok_str_ptr+6,NULL); // convert to a number and then point to the next character afterwards
  tok_str_ptr=strstr(SBE_str_dcal,"TA1 = ");
  cal_coef->ta1=strtof(tok_str_ptr+6,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"TA2 = ");
  cal_coef->ta2=strtof(tok_str_ptr+6,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"TA3 = ");
  cal_coef->ta3=strtof(tok_str_ptr+6,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"G = ");
  cal_coef->cg=strtof(tok_str_ptr+4,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"H = ");
  cal_coef->ch=strtof(tok_str_ptr+4,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"I = ");
  cal_coef->ci=strtof(tok_str_ptr+4,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"J = ");
  cal_coef->cj=strtof(tok_str_ptr+4,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"CPCOR = ");
  cal_coef->cpcor=strtof(tok_str_ptr+8,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"CTCOR = ");
  cal_coef->ctcor=strtof(tok_str_ptr+8,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"PA0 = ");
  cal_coef->pa0=strtof(tok_str_ptr+6,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PA1 = ");
  cal_coef->pa1=strtof(tok_str_ptr+6,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PA2 = ");
  cal_coef->pa2=strtof(tok_str_ptr+6,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"PTCA0 = ");
  cal_coef->ptca0=strtof(tok_str_ptr+8,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTCA1 = ");
  cal_coef->ptca1=strtof(tok_str_ptr+8,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTCA2 = ");
  cal_coef->ptca2=strtof(tok_str_ptr+8,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"PTCB0 = ");
  cal_coef->ptcb0=strtof(tok_str_ptr+8,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTCB1 = ");
  cal_coef->ptcb1=strtof(tok_str_ptr+8,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTCB2 = ");
  cal_coef->ptcb2=strtof(tok_str_ptr+8,NULL);

  tok_str_ptr=strstr(SBE_str_dcal,"PTEMPA0 = ");
  cal_coef->ptempa0=strtof(tok_str_ptr+10,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTEMPA1 = ");
  cal_coef->ptempa1=strtof(tok_str_ptr+10,NULL);
  tok_str_ptr=strstr(SBE_str_dcal,"PTEMPA2 = ");
  cal_coef->ptempa2=strtof(tok_str_ptr+10,NULL);

  free(SBE_str_dcal);

  return MOD_SOM_STATUS_OK;
}




uint16_t mod_som_sbe49_decode_status_f(mod_som_status_t status){
    if(status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint16_t status_prefix = status>>8;
    uint16_t decoded_status = status & 0xffffU;
    if(status_prefix != MOD_SOM_SBE49_STATUS_PREFIX){
        return 0xffffU;
    }
    return decoded_status;
}

mod_som_status_t mod_som_sbe49_encode_status_f(uint16_t status){
    if(status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_SBE49_STATUS_PREFIX, status);
}

mod_som_status_t mod_som_sbe49_direct_communication_f(){
    //TODO SN still working on this!!!
    bool tmp_collect_data_flag = mod_som_sbe49_ptr->collect_data_flag;
    mod_som_status_t status;
    if(tmp_collect_data_flag){
        status = mod_som_sbe49_stop_collect_data_f(mod_som_sbe49_ptr);
        APP_RTOS_ASSERT_DBG((status == MOD_SOM_STATUS_OK), 1);
        if(status != MOD_SOM_STATUS_OK)
            return status;
    }

    char input_buf[128]="\0";
    int c;
    int32_t i;
    while(DEF_ON){
        for (i = 0; i < 127; i++) {
            while ((c = RETARGET_ReadChar())<0);  //read until valid input
            if (c == ASCII_CHAR_DELETE || c == 0x08) { // User inputed backspace
                if (i) {
                    mod_som_io_print_f("\b \b");
                    input_buf[--i] = '\0';
                }
                i--;
                continue;
            } else if (c == '\r' || c == '\n') {
                if (i) {
                    mod_som_io_print_f("\r\n");
                    break;
                } else {
                    mod_som_io_print_f("\r\n$ ");
                    i--;
                    continue;
                }
            }else if(c == 27){
                for(i--;i>=0;i--){
                    mod_som_io_print_f("\b \b");
                }
                input_buf[0] = '\0';
            }else if (!(c>31 && c<127)){ // check for printable characters
                i--;
                continue;
            }

            mod_som_io_putchar_f(c); // Echo to terminal
            input_buf[i] = c;
        }
        input_buf[i] = '\0';
    }

    if(tmp_collect_data_flag){
        status = mod_som_sbe49_start_collect_data_f(mod_som_sbe49_ptr);
        APP_RTOS_ASSERT_DBG((status == MOD_SOM_STATUS_OK), 1);
        if(status != MOD_SOM_STATUS_OK)
            return status;
    }
    return MOD_SOM_STATUS_OK;
}
