/*
 * mod_som_voltage.c
 *
 *  Created on: Jan 26, 2021
 *      Author: aleboyer
 *
 *      The module sample the voltage coming to the SOM.
 *      After initializing the ADC, the use can use
 *        - volt.scan to sample one time
 *        - volt.sample to periodically perfomr the ADC sampling.
 *
 *      The result of the ADC sampling is updated in the module_prt
 *      mod_som_voltage_ptr->
 */
#include <voltage/mod_som_voltage.h>
#include "mod_som_sdio.h"
#include "mod_som_io.h"
#include "mod_som_priv.h"

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <voltage/mod_som_voltage_cmd.h>
#endif
#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif


#define ADCFREQ   32000

//MHA
#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;

// Data producer
static CPU_STK voltage_scan_task_stk[MOD_SOM_VOLTAGE_SCAN_STK_SIZE];
static OS_TCB  voltage_scan_task_tcb;

static CPU_STK voltage_adc1_scan_task_stk[MOD_SOM_VOLTAGE_SCAN_STK_SIZE];
static OS_TCB  voltage_adc1_scan_task_tcb;

//ALB EFE STATUS
sl_status_t mystatus;

mod_som_voltage_ptr_t mod_som_voltage_ptr;

/*******************************************************************************
 * @brief
 *   Initialize voltage BAR, if shell is available, then the command table is added
 *
 *   after initializing the internal ADC, T
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_voltage_init_f(){
    mod_som_status_t status;
    RTOS_ERR  err;

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_voltage_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_voltage_encode_status_f(MOD_SOM_VOLTAGE_STATUS_FAIL_INIT_CMD);
#endif


    // ALB allocate memory for the module_ptr.
    // ALB The module_ptr is also the "scope" of the runtime_ptr
    // ALB but the module_ptr also contains the settings_ptr and the config_ptr
    // ALB The settings_ptr an config_ptr should allocated and defined during the module initialization
    mod_som_voltage_ptr = (mod_som_voltage_ptr_t)Mem_SegAlloc(
        "MOD SOM VOLTAGE RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_voltage_t),
        &err);

    //SN Check error code
    //ALB WARNING: The following line hangs in a while loop -
    //ALB WARNING: - if the previous allocation fails.
    //ALB TODO change return -1 to return the an appropriate error code.
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_voltage_ptr==DEF_NULL){
      printf("VOLT not initialized\n");
      return -1;
    }

    mod_som_voltage_ptr->settings_ptr = (mod_som_voltage_settings_ptr_t)Mem_SegAlloc(
        "MOD SOM VOLTAGE SETTINGS Memory",DEF_NULL,
        sizeof(mod_som_voltage_settings_t),
        &err);

    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_voltage_ptr==DEF_NULL){
      printf("VOLT not initialized\n");
      return -1;
    }

    // ALB checking if a previous EFE setup exist, from the setup module (i.e. setup file or UserData setup)
  #ifdef MOD_SOM_SETTINGS_EN
      mod_som_settings_struct_ptr_t local_settings_ptr=mod_som_settings_get_settings_f();
      mod_som_voltage_ptr->settings_ptr=&local_settings_ptr->mod_som_voltage_settings;
  #else
      mod_som_voltage_ptr->settings_ptr->initialize_flag=false;
  #endif

      // ALB If no pre-existing settings, use the default settings
      if (!mod_som_voltage_ptr->settings_ptr->initialize_flag){
          // initialize the setup structure.
          status |= mod_som_voltage_default_settings_f(mod_som_voltage_ptr->settings_ptr);
          if (status!=MOD_SOM_STATUS_OK){
              printf("VOLT not initialized");
              return status;
          }
      }


    mod_som_voltage_ptr->initialized_flag=false;

    // initialize data mode 0:streaming, 1:SD store
    mod_som_voltage_ptr->mode=0;

    // Enable ADC0 clock
    CMU_ClockEnable(cmuClock_ADC0, true);

    // Enable ADC1 clock
    CMU_ClockEnable(cmuClock_ADC1, true);

    // Declare init structs
    //ALB I do not know why I can not directly
    //    define init
    ADC_Init_TypeDef init= ADC_INIT_DEFAULT;
    ADC_InitScan_TypeDef initScan=ADC_INITSCAN_DEFAULT;
    ADC_InitSingle_TypeDef initSingle1=ADC_INITSINGLE_DEFAULT;

//    init     = init;
//    //ADC0 stuff
//
//    initScan  = initScan;

    // Modify init structs
    init.prescale   = ADC_PrescaleCalc(ADCFREQ, 0);
    init.timebase   = ADC_TimebaseCalc(0); //USe the HF per clock frequency

    initScan.diff       = 0;            // single ended
    initScan.reference  = adcRef2V5;    // internal 2.5V reference
    initScan.resolution = adcRes12Bit;  // 12-bit resolution
    initScan.acqTime    = adcAcqTime256;  // set acquisition time to meet minimum requirement
    initScan.fifoOverwrite = true;      // FIFO overflow overwrites old data


    // Select ADC inputs. See README for corresponding EXP header pin.
    // *Note that internal channels are unavailable in ADC scan mode
    ADC_ScanSingleEndedInputAdd(&(initScan), \
                                adcScanInputGroup0, \
                                adcPosSelAPORT0XCH0);

    // Set scan data valid level (DVL) to 2
    ADC0->SCANCTRLX |= (NUM_INPUTS - 1) << _ADC_SCANCTRLX_DVL_SHIFT;

    // Clear ADC Scan fifo
    ADC0->SCANFIFOCLEAR = ADC_SCANFIFOCLEAR_SCANFIFOCLEAR;

    // Initialize ADC and Scan
    ADC_Init(ADC0, &init);
    ADC_InitScan(ADC0, &initScan);

    // Enable Scan interrupts
    ADC_IntEnable(ADC0, ADC_IEN_SCAN);

    // Enable ADC interrupts
    NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_EnableIRQ(ADC0_IRQn);

    //ADC1 stuff
    //initSingle1 = initSingle1;
    // Modify init structs

    initSingle1.diff       = 0;            // single ended
    initSingle1.reference  = adcRefVDD;    // internal 3.3 reference
    initSingle1.resolution = adcRes12Bit;  // 12-bit resolution
    initSingle1.acqTime    = adcAcqTime256;  // set acquisition time to meet minimum requirement
    initSingle1.fifoOverwrite = true;      // FIFO overflow overwrites old data
    initSingle1.posSel     = adcPosSelAPORT1XCH8;

    // Select ADC inputs. See README for corresponding EXP header pin.
    // *Note that internal channels are unavailable in ADC scan mode
//    ADC_ScanSingleEndedInputAdd(&(initSingle1),
//                                adcScanInputGroup0,
//                                adcPosSelAPORT1XCH8);


    // Set scan data valid level (DVL) to 2
//    ADC1->SINGLECTRLX |= (NUM_INPUTS - 1) << _ADC_SINGLECTRLX_DVL_SHIFT;

    // Clear ADC Scan fifo
    ADC1->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;

    // Initialize ADC and Scan
    ADC_Init(ADC1, &init);
//    ADC_InitScan(ADC1, &initScan1);
    ADC_InitSingle(ADC1, &initSingle1);

    // Enable Scan interrupts
    ADC_IntEnable(ADC1, ADC_IEN_SINGLE);

    // Enable ADC interrupts
    NVIC_ClearPendingIRQ(ADC1_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);

    mod_som_voltage_ptr->initialized_flag=true;
    printf("%s initialized\r\n",MOD_SOM_VOLTAGE_HEADER);



    return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   Initialize settings struct ptr
 *     uint16_t size;
 *     char header[4];
 *     uint32_t sampling_frequency;
 *     bool initialize_flag;
 *
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_voltage_default_settings_f(mod_som_voltage_settings_ptr_t settings_ptr){

  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  settings_ptr->initialize_flag=false;
  strncpy(settings_ptr->header,MOD_SOM_VOLTAGE_HEADER,MOD_SOM_VOLTAGE_HEADER_LENGTH);
  settings_ptr->sampling_frequency=MOD_SOM_VOLTAGE_TASK_DELAY; // in kernel ticks TODO change to ms?
  settings_ptr->size=sizeof(*settings_ptr);
  settings_ptr->initialize_flag=true;

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_voltage_settings_t mod_som_voltage_get_settings_f(){
  return *mod_som_voltage_ptr->settings_ptr;
}
/*******************************************************************************
 * @brief
 *   get the voltage runtime ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_voltage_ptr_t mod_som_voltage_get_runtime_ptr_f(){
  return mod_som_voltage_ptr;
}



/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_voltage_say_hello_world_f(){
    mod_som_io_print_f("[voltage]: hello world\r\n");
    return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
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
uint8_t mod_som_voltage_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_VOLTAGE_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_voltage_encode_status_f
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
mod_som_status_t mod_som_voltage_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_VOLTAGE_STATUS_PREFIX, mod_som_io_status);
}


/*******************************************************************************
 * @function
 *     mod_som_voltage_start_task_f
 *     author: aleboyer@ucsd.edu
 * @abstract
 *    Start the task to periodically scan the ADC to get the SOM voltage.
 *    The task priority is 19u.
 *    FYI: shell priority is 20 and stream-store is 18.
 * @discussion
 *
 *
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/

mod_som_status_t mod_som_voltage_start_adc1_scan_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
   OSTaskCreate(&voltage_adc1_scan_task_tcb,
                        "voltage adc1 scan task",
                        mod_som_voltage_adc1_scan_task_f,
                        DEF_NULL,
                        MOD_SOM_VOLTAGE_SCAN_TASK_PRIO,
            &voltage_adc1_scan_task_stk[0],
            (MOD_SOM_VOLTAGE_SCAN_TASK_STK_SIZE / 10u),
            MOD_SOM_VOLTAGE_SCAN_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_voltage_ptr->status = mod_som_voltage_encode_status_f(MOD_SOM_VOLTAGE_STATUS_FAIL_TO_START_STORE_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);


  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}

mod_som_status_t mod_som_voltage_start_scan_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
   OSTaskCreate(&voltage_scan_task_tcb,
                        "voltage scan task",
                        mod_som_voltage_scan_task_f,
                        DEF_NULL,
                        MOD_SOM_VOLTAGE_SCAN_TASK_PRIO,
            &voltage_scan_task_stk[0],
            (MOD_SOM_VOLTAGE_SCAN_TASK_STK_SIZE / 10u),
            MOD_SOM_VOLTAGE_SCAN_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_voltage_ptr->status = mod_som_voltage_encode_status_f(MOD_SOM_VOLTAGE_STATUS_FAIL_TO_START_STORE_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);


  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_voltage_stop_task_f
 *   stop the voltage_scan_task
 *   TODO check if multiple start-stop cycles creates memory leaks
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_voltage_stop_scan_task_f(){

  RTOS_ERR err;

    OSTaskDel(&voltage_scan_task_tcb,
              &err);

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   mod_som_voltage_stop_task_f
 *   stop the voltage_scan_task
 *   TODO check if multiple start-stop cycles creates memory leaks
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_voltage_stop_adc1_scan_task_f(){

  RTOS_ERR err;

    OSTaskDel(&voltage_adc1_scan_task_tcb,
              &err);

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   mod_som_voltage_mode_f
 *   select the data mode 0:streaming, 1:SD storing
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_voltage_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]){


  RTOS_ERR  p_err;

  if (argc==1){
      printf("volt.mode %u\r\n",mod_som_voltage_ptr->mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_voltage_ptr->mode=shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: volt.mode mode (0:stream, 1:SD store)\r\n");
      break;
    }
  }

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   mod_som_voltage_scan_f
 *   scan the GG11 ADC0 on GPIO PD0
 *   I grab the time stamp here to reduce the time in the IRQ Handler but the stream
 *   So be aware the time stamp is not super precise here.
 *   There is a lag between the voltage measurement and the timestamp.
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_voltage_scan_f(void){

  uint64_t tick;

  //ALB get timestamp here right before gathering ADC samples
  tick=sl_sleeptimer_get_tick_count64();
  mystatus = sl_sleeptimer_tick64_to_ms(tick,\
         &mod_som_voltage_ptr->timestamp);

  //MHA: Now augment timestamp by poweron_offset_ms
  mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
  mod_som_voltage_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

  mod_som_voltage_ptr->voltage=0;

  ADC_Start(ADC0, adcStartScan);

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   mod_som_voltage_scan_f
 *   scan the GG11 ADC0 on GPIO PD0
 *   I grab the time stamp here to reduce the time in the IRQ Handler but the stream
 *   So be aware the time stamp is not super precise here.
 *   There is a lag between the voltage measurement and the timestamp.
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_voltage_adc1_scan_f(void){

  uint64_t tick;

  //ALB get timestamp here right before gathering ADC samples
  tick=sl_sleeptimer_get_tick_count64();
  mystatus = sl_sleeptimer_tick64_to_ms(tick,\
         &mod_som_voltage_ptr->timestamp_adc1);

  //MHA: Now augment timestamp by poweron_offset_ms
  mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
  mod_som_voltage_ptr->timestamp_adc1 += mod_som_calendar_settings.poweron_offset_ms;

  ADC_Start(ADC1, adcStartSingle);

  return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_voltage_scan_task_f
 *   scan_task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

void mod_som_voltage_scan_task_f(void  *p_arg){

  RTOS_ERR err;


  //        printf("In Consumer Task 2\n");
  while (DEF_ON) {
      WDOG_Feed();
      mod_som_voltage_scan_f();

      // Delay Start Task execution for
      OSTimeDly( mod_som_voltage_ptr->settings_ptr->sampling_frequency,         //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      //   Check error code.
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}

/*******************************************************************************
 * @brief
 *   mod_som_voltage_scan_task_f
 *   scan_task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

void mod_som_voltage_adc1_scan_task_f(void  *p_arg){

  RTOS_ERR err;


  //        printf("In Consumer Task 2\n");
  while (DEF_ON) {
      WDOG_Feed();
      mod_som_voltage_adc1_scan_f();

      // Delay Start Task execution for
      OSTimeDly( mod_som_voltage_ptr->settings_ptr->sampling_frequency,         //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      //   Check error code.
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}



/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
  uint32_t data, id;
  uint32_t t_hex[2];

  char * curr_record_ptr;

  curr_record_ptr=(char*) mod_som_voltage_ptr->str_record;

  // Get ADC results
  // Read data from ADC
  data = ADC_DataIdScanGet(ADC0, &id);

  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
      mod_som_sdio_get_runtime_ptr_f();

  mod_som_sdio_file_ptr_t rawfile_ptr =
      local_mod_som_sdio_ptr_t->rawdata_file_ptr;


  // Convert data to mV and store into array
  // 4096 is 2^12 the bit resolution
  // 2500 is 2.5 volts the voltage reference.
  // voltage is in v.
  mod_som_voltage_ptr->voltage = MOD_SOM_VOLTAGE_DIVIDER * data *
                                                 2500 / 4096;

  t_hex[0] = (uint32_t) (mod_som_voltage_ptr->timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_voltage_ptr->timestamp;

  //VOLT header + hex timestamp. It will be used in the IRQ handler to send
  // a message to the IO stream or store task
  sprintf((char *) mod_som_voltage_ptr->str_record,  \
      "$%s%08x%08x%08lu*FF\r\n", \
      mod_som_voltage_ptr->settings_ptr->header, \
      (int) t_hex[0],\
      (int) t_hex[1],
      mod_som_voltage_ptr->voltage);

  mod_som_voltage_ptr->chksum=0;
  for(int i=0;i<strlen(mod_som_voltage_ptr->str_record)-MOD_SOM_VOLTAGE_CHCKSUM_LENGTH;i++)
    {
    mod_som_voltage_ptr->chksum ^=\
        *(curr_record_ptr++);
    }

  //ALB move the pointer after the *
  curr_record_ptr++;
  //ALB write the hex chcksum at the right place.    // SN TODO mod_som_int8_2hex_f inverse the HEX caracter.
  uint16_t local_chksum = \
      mod_som_int8_2hex_f(mod_som_voltage_ptr->chksum);

  memcpy(curr_record_ptr,&local_chksum,sizeof(uint16_t));

  switch(mod_som_voltage_ptr->mode){
    case 0:
      //ALB direct stream
      mod_som_io_stream_data_f((uint8_t *) &mod_som_voltage_ptr->str_record, \
                                            MOD_SOM_VOLTAGE_RECORD_LENGTH,   \
                                            DEF_NULL);
      break;
    case 1:
      //ALB direct SD store
      mod_som_sdio_write_data_f(rawfile_ptr,
                               (uint8_t*) mod_som_voltage_ptr->str_record, \
                                MOD_SOM_VOLTAGE_RECORD_LENGTH,   \
                                DEF_NULL);
      break;
    case 2:
      //ALB Aggregator

      break;

  }

}

/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
void ADC1_IRQHandler(void)
{
  uint32_t data;
//  char str_adc1[100];

    ADC1->IFC=1;
// Get ADC results
// Read data from ADC
  data = ADC1->SINGLEDATA;

  // Convert data to mV and store into array
  // 4096 is 2^12 the bit resolution
  // 3300 is 3.3 volts the voltage reference.
  // voltage is in v.
  mod_som_voltage_ptr->voltage_adc1 = data * 3300 / 4096;


//ALB keep this for debbug. It prints the voltage on the supercap.
//  sprintf((char*) str_adc1,
//        "SD voltage $%4lumV\r\n",mod_som_voltage_ptr->voltage_adc1);
//  printf(str_adc1);


}





