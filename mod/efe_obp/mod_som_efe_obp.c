/*
 * mod_som_food_bar.c
 *
 *  Created on: Apr 3, 2020
 *      Author: snguyen
 */
#include <efe_obp/mod_som_efe_obp.h>

#ifdef  MOD_SOM_EFE_OBP_EN


#include <efe_obp/mod_som_efe_obp_calc.h>
#include "mod_som_io.h"
#include "mod_som_priv.h"

#include <math.h>

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <efe_obp/mod_som_efe_obp_cmd.h>
#endif
#ifdef MOD_SOM_SETTINGS_EN
  #include <mod_som_settings.h>
#endif

#ifdef MOD_SOM_SBE41_EN
  #include <mod_som_sbe41.h>
#endif


//MHA
#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;


//PRIVATE DEFINE
#define MOD_SOM_EFE_OBP_DEFAULT_SIZE_LARGEST_BLOCK 128U
#define MOD_SOM_EFE_OBP_MSG_QUEUE_COUNT 64U

//PRIVATE FUNCTIONS
/*******************************************************************************
 * @brief
 *   producer task function
 *
 *   convert cnt to volt
 *   compute the frequency spectra
 *   compute nu and kappa
 *   convert frequency to wavenumber
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_cnt2volt_f(uint8_t * efe_sample,
                                            float   * local_efeobp_efe_temp_ptr,
                                            float   * local_efeobp_efe_shear_ptr,
                                            float   * local_efeobp_efe_accel_ptr);

//PRIVATE VARIABLES

//sleeptimer status
sl_status_t mystatus;

// Fill segment
static CPU_STK efe_obp_fill_segment_task_stk[MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_STK_SIZE];
static OS_TCB  efe_obp_fill_segment_task_tcb;

// compute spectra
static CPU_STK efe_obp_cpt_spectra_task_stk[MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_STK_SIZE];
static OS_TCB  efe_obp_cpt_spectra_task_tcb;

// compute dissrate
static CPU_STK efe_obp_cpt_dissrate_task_stk[MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_STK_SIZE];
static OS_TCB  efe_obp_cpt_dissrate_task_tcb;


// Data consumer
static CPU_STK efe_obp_consumer_task_stk[MOD_SOM_EFE_OBP_CONSUMER_TASK_STK_SIZE];
static OS_TCB  efe_obp_consumer_task_tcb;


mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr;

mod_som_sbe41_ptr_t local_sbe41_ptr;


/*******************************************************************************
 * @brief
 *   Initialize efeobp, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_init_f(){
    mod_som_status_t status;
    RTOS_ERR  err;

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_efe_obp_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_efe_obp_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_INIT_CMD);
#endif


    //ALB allocate memory for the module_ptr.
    //ALB The module_ptr is also the "scope" of the runtime_ptr
    //ALB but the module_ptr also contains the settings_ptr and the config_ptr
    //ALB The settings_ptr an config_ptr should allocated and defined
    //ALB during the module initialization
    mod_som_efe_obp_ptr = (mod_som_efe_obp_ptr_t)Mem_SegAlloc(
        "MOD SOM EFE OBP RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_efe_obp_t),
        &err);

    //SN Check error code
    //ALB WARNING: The following line hangs in a while loop -
    //ALB WARNING: - if the previous allocation fails.
    //ALB TODO change return -1 to return the an appropriate error code.
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_efe_obp_ptr==DEF_NULL){
      printf("EFE OBP not initialized\n\r");
      return -1;
    }

    //ALB Initialize the runtime flag module_ptr->initialized_flag to false.
    //ALB It will be set to true once the module is initialized at the
    //ALB end of mod_som_efe_init_f().
    mod_som_efe_obp_ptr->initialized_flag = false;

    //2025 06 14 adding this for monitoring the task
    mod_som_efe_obp_ptr->efe_obp_fill_segment_task_stk_ptr = efe_obp_fill_segment_task_stk;
    mod_som_efe_obp_ptr->efe_obp_fill_segment_task_tcb_ptr = &efe_obp_fill_segment_task_tcb;

    // compute spectra
    mod_som_efe_obp_ptr->efe_obp_cpt_spectra_task_stk_ptr = efe_obp_cpt_spectra_task_stk;
    mod_som_efe_obp_ptr->efe_obp_cpt_spectra_task_tcb_ptr = &efe_obp_cpt_spectra_task_tcb;

    // compute dissrate
    mod_som_efe_obp_ptr->efe_obp_cpt_dissrate_task_stk_ptr = efe_obp_cpt_dissrate_task_stk;
    mod_som_efe_obp_ptr->efe_obp_cpt_dissrate_task_tcb_ptr = &efe_obp_cpt_dissrate_task_tcb;


    // Data consumer
    mod_som_efe_obp_ptr->efe_obp_consumer_task_stk_ptr = efe_obp_consumer_task_stk;
    mod_som_efe_obp_ptr->efe_obp_consumer_task_tcb_ptr = &efe_obp_consumer_task_tcb;

    // ALB allocate memory for the settings_ptr.
    // ALB WARNING: The setup pointer CAN NOT have pointers inside.
    status |= mod_som_efe_obp_allocate_settings_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("EFE not initialized\n\r");
      return status;
    }



    // ALB checking if a previous EFE OBP setup exist, from the setup module
    //ALB (i.e. setup file or UserData setup)
  #ifdef MOD_SOM_SETTINGS_EN
      mod_som_settings_struct_ptr_t local_settings_ptr=
                                              mod_som_settings_get_settings_f();
      mod_som_efe_obp_ptr->settings_ptr=
                                  &local_settings_ptr->mod_som_efe_obp_settings;
  #else
      mod_som_efe_obp_ptr->settings_ptr->initialize_flag=false;
  #endif

    // ALB If no pre-existing settings, use the default settings
      if (!mod_som_efe_obp_ptr->settings_ptr->initialize_flag){
        // initialize the setup structure.
        status |= mod_som_efe_obp_default_settings_f(
                                             mod_som_efe_obp_ptr->settings_ptr);
        if (status!=MOD_SOM_STATUS_OK){
          printf("EFE OBP not initialized\n\r");
          return status;
        }
      }

           //ALB get efe_settings: Nb of channels, record length, ...
              mod_som_efe_obp_ptr->efe_settings_ptr=mod_som_efe_get_settings_ptr_f();


      //ALB Allocate memory for the config pointer,
      //ALB using the settings_ptr variable
      status |= mod_som_efe_obp_construct_config_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("EFE OBP not initialized\n\r");
      return status;
    }

    //ALB Allocate memory for the calib pointer,
    //ALB using the settings_ptr variable
    status |= mod_som_efe_obp_construct_calibration_ptr_f();
  if (status!=MOD_SOM_STATUS_OK){
    printf("EFE OBP not initialized\n\r");
    return status;
  }

    // ALB Allocate memory for the consumer_ptr,
    // ALB contains the consumer stream data_ptr and stream_data length.
    // ALB This pointer is also used to store the data on the SD card
      status |= mod_som_efe_obp_construct_consumer_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("EFE OBP not initialized\n\r");
      return status;
    }

    mod_som_epsiobp_init_f(mod_som_efe_obp_ptr->config_ptr,
                           mod_som_efe_obp_ptr->settings_ptr,
                           mod_som_efe_obp_ptr->calibration_ptr);

    // ALB Allocate memory for the consumer_ptr,
    // ALB contains the consumer stream data_ptr and stream_data length.
    // ALB This pointer is also used to store the data on the SD card
      status |= mod_som_efe_obp_construct_fill_segment_ptr_f();
      status |= mod_som_efe_obp_construct_cpt_spectra_ptr_f();
      status |= mod_som_efe_obp_construct_cpt_dissrate_ptr_f();
    if (status!=MOD_SOM_STATUS_OK){
      printf("EFE OBP not initialized\n\r");
      return status;
    }



    //ALB temporary for debug so I do not type the command
//    mod_som_efe_obp_start_fill_segment_task_f();
//    mod_som_efe_obp_start_cpt_spectra_task_f();
//    mod_som_efe_obp_start_cpt_dissrate_task_f();
//    mod_som_efe_obp_start_consumer_task_f();

    mod_som_efe_obp_ptr->sample_count                = 0;
    mod_som_efe_obp_ptr->sampling_flag               = 0;
    mod_som_efe_obp_ptr->data_ready_flag             = 0;
    mod_som_efe_obp_ptr->start_computation_timestamp = 0;
    mod_som_efe_obp_ptr->stop_computation_timestamp  = 0;

    mod_som_efe_obp_ptr->initialized_flag            = true;

    printf("EFE OBP initialized\n\r");



    return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   construct settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_allocate_settings_ptr_f(){

  RTOS_ERR  err;

  //ALB alloc memory for setup pointer
  //set up default configuration
  mod_som_efe_obp_ptr->settings_ptr =
      (mod_som_efe_obp_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP setup.",DEF_NULL,
          sizeof(mod_som_efe_obp_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_efe_obp_ptr->settings_ptr==NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return MOD_SOM_EFE_OBP_CANNOT_ALLOCATE_SETUP;
  }

  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Initialize setup struct ptr
 *   uint32_t size;
 *   char header[MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH];
 *   uint32_t nfft;
 *   uint32_t record_format;
 *   uint32_t telemetry_format;
 *   uint32_t initialize_flag;
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_default_settings_f(
                                    mod_som_efe_obp_settings_ptr_t settings_ptr)
{
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  settings_ptr->nfft               = MOD_SOM_EFE_OBP_DEFAULT_NFFT;
  settings_ptr->record_format      = MOD_SOM_EFE_RECORD_DEFAULT_FORMAT;
  settings_ptr->telemetry_format   = MOD_SOM_EFE_TELEMETRY_DEFAULT_FORMAT;
  settings_ptr->degrees_of_freedom = MOD_SOM_EFE_OBP_CPT_SPECTRA_DEGREE_OF_FREEDOM;
  settings_ptr->channels_id[0]     = 0 ; //select channel t1
  settings_ptr->channels_id[1]     = 1 ; //select channel s1
  settings_ptr->channels_id[2]     = 2 ; //select channel a3

  settings_ptr->mode=1;
  settings_ptr->format=1;
  settings_ptr->channel=0;

  strncpy(settings_ptr->header,
          MOD_SOM_EFE_OBP_HEADER,MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH);
  settings_ptr->initialize_flag=true;
  settings_ptr->size=sizeof(*settings_ptr);


  // get efe_settings: Nb of channels, record length, ...
  mod_som_efe_obp_ptr->efe_settings_ptr=mod_som_efe_get_settings_ptr_f();

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_efe_obp_settings_t mod_som_efe_obp_get_settings_f(){
  return *mod_som_efe_obp_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get the efe obp runtime ptr
 *
 * @param
 *
 ******************************************************************************/
mod_som_efe_obp_ptr_t mod_som_efe_obp_get_runtime_ptr_f(){
  return mod_som_efe_obp_ptr;
}


/*******************************************************************************
 * @brief
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_config_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for config pointer
  mod_som_efe_obp_ptr->config_ptr =
      (mod_som_efe_obp_config_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP config.",DEF_NULL,
          sizeof(mod_som_efe_obp_config_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_efe_obp_ptr->config_ptr==NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return MOD_SOM_EFE_OBP_CANNOT_OPEN_CONFIG;
  }

  mod_som_efe_obp_config_ptr_t config_ptr = mod_som_efe_obp_ptr->config_ptr;
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  config_ptr->initialized_flag = false;
  config_ptr->header_length    = MOD_SOM_EFE_OBP_SYNC_TAG_LENGTH      +
                                 MOD_SOM_EFE_OBP_HEADER_TAG_LENGTH    +
                                 MOD_SOM_EFE_OBP_HEXTIMESTAMP_LENGTH    +
                                 MOD_SOM_EFE_OBP_PAYLOAD_LENGTH         +
                                 MOD_SOM_EFE_OBP_LENGTH_HEADER_CHECKSUM;

  config_ptr->f_samp     = MOD_SOM_EFE_OBP_SAMPLING_FREQ;
  config_ptr->f_CTD_pump = MOD_SOM_EFE_OBP_PUMP_FREQ;


  config_ptr->initialized_flag = true;
  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct calibration_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_calibration_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for config pointer
  mod_som_efe_obp_ptr->calibration_ptr =
      (mod_som_efe_obp_calibration_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP calib.",DEF_NULL,
          sizeof(mod_som_efe_obp_calibration_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_efe_obp_ptr->calibration_ptr==NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return MOD_SOM_EFE_OBP_CANNOT_OPEN_CALIBRATION;
  }

  mod_som_efe_obp_calibration_ptr_t calib_ptr = mod_som_efe_obp_ptr->calibration_ptr;
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  // ALB Allocate memory for the spectra
  calib_ptr->cafilter_coeff = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP CA coef ptr",DEF_NULL,
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  // ALB Allocate memory for the spectra
  calib_ptr->cafilter_freq = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP FREQ PTR",DEF_NULL,
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  //ALB so far I use a fake cafilter
  //ALB change to use a polynomial.
  calib_ptr->cafilter_size=mod_som_efe_obp_ptr->settings_ptr->nfft/2;
  //ALB hardcoded poly coef of the fpo7 noise
  //ALB TODO update this coef after the stand alone test
  calib_ptr->fp07_noise[0]=-12.2172;
  calib_ptr->fp07_noise[1]=-0.9104;
  calib_ptr->fp07_noise[2]= 1.3882;
  calib_ptr->fp07_noise[3]=-0.5674;

  calib_ptr->shear_sv=mod_som_efe_obp_ptr->efe_settings_ptr->sensors[1].cal;;
  calib_ptr->fp07_dTdV=mod_som_efe_obp_ptr->efe_settings_ptr->sensors[0].cal;

  calib_ptr->acc_offset=MOD_SOM_EFE_OBP_ACCELL_OFFSET;
  calib_ptr->acc_factor=MOD_SOM_EFE_OBP_ACCELL_FACTOR;


  //ALB adding MHA code here.
  //TODO Make it not hard coded.
  for (int c=0;c<mod_som_efe_obp_ptr->settings_ptr->nfft/2;c++) {
//    *(calib_ptr->cafilter_coeff+c)=0.837; // The value from ~10 - 1000 Hz of the H2_elec function
    *(calib_ptr->cafilter_coeff+c)=0.9151; // The value from ~10 - 1000 Hz of the H_elec function
    //Now add in the low-freq rolloffs.
  };

  switch (mod_som_efe_obp_ptr->settings_ptr->nfft/2) {
  case 256: // not enough resolution for a low-freq correction.
    break;
  case 512:
    *(calib_ptr->cafilter_coeff+1)=0.7995;
    break;
  case 1024:
//    *(calib_ptr->cafilter_coeff)  = 0;
//    *(calib_ptr->cafilter_coeff+1)= 0.6917;
//    *(calib_ptr->cafilter_coeff+2)= 0.7995;
//    *(calib_ptr->cafilter_coeff+3)= 0.8205;
    *(calib_ptr->cafilter_coeff)  = 0;
    *(calib_ptr->cafilter_coeff+1)= 0.8291;
    *(calib_ptr->cafilter_coeff+2)= 0.8935;
    *(calib_ptr->cafilter_coeff+3)= 0.9055;
    *(calib_ptr->cafilter_coeff+4)= 0.9097;
    *(calib_ptr->cafilter_coeff+5)= 0.9117;
    *(calib_ptr->cafilter_coeff+6)= 0.9127;
    *(calib_ptr->cafilter_coeff+7)= 0.9134;
    break;
  case 2048: //0.3487    0.6917    0.7706    0.7995    0.8130    0.8205    0.8250    0.8279    0.8299
    //fill in the values above. NOT FINISHED
    break;
  case 4096:
    //0.0672    0.3487    0.5896    0.6917    0.7423    0.7706    0.7880    0.7995    0.8074    0.8130    0.8173    0.8205    0.8230    0.8250    0.8266    0.8279
    //          0.8290    0.8299    0.8307    0.8313    0.8319    0.8324    0.8328    0.8332    0.8336    0.8339    0.8341    0.8344    0.8346
    break;
    //default:
  }


  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct consumer structure
 *
 *   The consumer will creates 3 type of records: segment, spectra, rates.
 *   Each segments will follow the rules of mod som record:
 *   header + header checksum + payload + payload checksum
 *   header:  1  char sync ($)               +
 *            4  char tag (SEGM, SPEC, RATE) +
 *            16 char timestamps             +
 *            8  char payload size.
 *   header checksum: 1 char sync (*) +
 *                    2 char checksum (Hex of xor header)
 *   payload: data
 *   payload checksum : 1 char sync (*)                      +
 *                      2 char checksum (Hex of xor payload) +
 *                      2 char \r\n.
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_consumer_ptr_f(){

  RTOS_ERR  err;


  // allocate memory for streaming_ptr
  mod_som_efe_obp_ptr->consumer_ptr =
      (mod_som_efe_obp_data_consumer_ptr_t)Mem_SegAlloc(
      "MOD SOM EFE OBP consumer ptr",DEF_NULL,
      sizeof(mod_som_efe_obp_data_consumer_t),
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status =
        mod_som_efe_encode_status_f(
            MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));

  if(mod_som_efe_obp_ptr->consumer_ptr==DEF_NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return (mod_som_efe_obp_ptr->status =
        mod_som_efe_obp_encode_status_f(
            MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

  //ALB allocating memory for the biggest record use in this consumer
  //ALB That would be for printing segment (timestamps + segment)
  //ALB I am going to print only one channel.
  mod_som_efe_obp_ptr->consumer_ptr->max_payload_length =
     sizeof(uint64_t)+
     3*(mod_som_efe_obp_ptr->settings_ptr->nfft*sizeof(float))*
      MOD_SOM_EFE_OBP_CONSUMER_NB_SEGMENT_PER_RECORD;

  mod_som_efe_obp_ptr->consumer_ptr->max_record_length=
      mod_som_efe_obp_ptr->config_ptr->header_length +
      mod_som_efe_obp_ptr->consumer_ptr->max_payload_length+
      MOD_SOM_EFE_OBP_CONSUMER_PAYLOAD_CHECKSUM_LENGTH;

  mod_som_efe_obp_ptr->consumer_ptr->length_header=
                                           MOD_SOM_EFE_OBP_SYNC_TAG_LENGTH+
                                           MOD_SOM_EFE_OBP_HEADER_TAG_LENGTH+
                                           MOD_SOM_EFE_OBP_HEXTIMESTAMP_LENGTH +
                                           MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH+
                                           MOD_SOM_EFE_OBP_LENGTH_HEADER_CHECKSUM;


  //ALB initialize counters.
  mod_som_efe_obp_ptr->consumer_ptr->segment_cnt  = 0;
  mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt = 0;
  mod_som_efe_obp_ptr->consumer_ptr->rates_cnt    = 0;
  mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt  = 0;

  mod_som_efe_obp_ptr->consumer_ptr->started_flg  = false;

  //place holder allocation. It should actually point to the data file.

  mod_som_efe_obp_ptr->consumer_ptr->record_ptr =
      (uint8_t *)Mem_SegAlloc(
          "MOD SOM EFE OBP rec.",DEF_NULL,
          mod_som_efe_obp_ptr->consumer_ptr->max_record_length,
          &err);

  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct fill segment producer structure
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_fill_segment_ptr_f(){

  RTOS_ERR  err;


  // allocate memory for fill_segment_ptr
  mod_som_efe_obp_ptr->fill_segment_ptr = (mod_som_efe_obp_data_fill_segment_ptr_t)Mem_SegAlloc(
      "MOD SOM EFE OBP fill_segment ptr",DEF_NULL,
      sizeof(mod_som_efe_obp_data_fill_segment_t),
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_efe_obp_ptr->fill_segment_ptr==DEF_NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return (mod_som_efe_obp_ptr->status = mod_som_efe_obp_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

  // ALB Allocate memory for the spectra

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr = (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP segment_buffer.",DEF_NULL,
          sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft,
          &err);

  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->fill_segment_ptr->timestamp_segment_ptr = (uint64_t *)Mem_SegAlloc(
        "MOD SOM EFE OBP timestamp seg ptr",DEF_NULL,
        (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)*
        sizeof(uint64_t),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP temp volt ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP shear volt ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP accel volt ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  //ALB initialize field ctd data
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_pressure=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_temperature=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_salinity=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_dpdt=0;

  //ALB initialize all parameters. They should be reset right before
  //ALB fill_segment task is starts running.
//  mod_som_efe_obp_ptr->fill_segment_ptr->timestamp_segment=0;
//  mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_element_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_skipped=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->started_flg=false;
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped=0;


  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   construct fill spectra producer structure
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_cpt_spectra_ptr_f(){

  RTOS_ERR  err;


  // allocate memory for cpt_spectra_ptr
  mod_som_efe_obp_ptr->cpt_spectra_ptr = (mod_som_efe_obp_data_cpt_spectra_ptr_t)Mem_SegAlloc(
      "MOD SOM EFE OBP cpt_spectra ptr",DEF_NULL,
      sizeof(mod_som_efe_obp_data_cpt_spectra_t),
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_efe_obp_ptr->fill_segment_ptr==DEF_NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return (mod_som_efe_obp_ptr->status = mod_som_efe_obp_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  // ALB Allocate memory for the spectra
  mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP temp spectrum ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP shear spectrum ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP accel spectrum ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD*
        sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  //ALB initialize field ctd data
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure    =0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature =0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity    =0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt    =0;

  //ALB initialize all parameters. They should be reset right before
  //ALB cpt_spectra task is starts running.

  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp=0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->dof=0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->started_flg=false;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->spectra_skipped=0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt=0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt=0;
  mod_som_efe_obp_ptr->cpt_spectra_ptr->volt_read_index=0;

  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct fill segment producer structure
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_cpt_dissrate_ptr_f(){

  RTOS_ERR  err;


  // allocate memory for streaming_ptr
  mod_som_efe_obp_ptr->cpt_dissrate_ptr = (mod_som_efe_obp_data_cpt_dissrate_ptr_t)Mem_SegAlloc(
      "MOD SOM EFE OBP cpt_dissrate ptr",DEF_NULL,
      sizeof(mod_som_efe_obp_data_cpt_dissrate_t),
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_efe_obp_ptr->fill_segment_ptr==DEF_NULL)
  {
    mod_som_efe_obp_ptr = DEF_NULL;
    return (mod_som_efe_obp_ptr->status = mod_som_efe_obp_encode_status_f(MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  // ALB Allocate memory for the dissrate avg temp spec
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr= (float *)Mem_SegAlloc(
      "MOD SOM EFE OBP dissrate avg temp spec ptr",DEF_NULL,
      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD*
      sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
      &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  // ALB Allocate memory for the dissrate avg shear spec
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr= (float *)Mem_SegAlloc(
      "MOD SOM EFE OBP dissrate avg shear spec ptr",DEF_NULL,
      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD*
      sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
      &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  // ALB Allocate memory for the dissrate avg accel spec
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr= (float *)Mem_SegAlloc(
      "MOD SOM EFE OBP dissrate avg accel spec ptr",DEF_NULL,
      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD*
      sizeof(float)*mod_som_efe_obp_ptr->settings_ptr->nfft/2,
      &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);



 mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate chi ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate epsilon ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->nu = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate nu ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->kappa = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate kappa ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->kcutoff_shear = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate kcutoff_shear ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->fcutoff_temp = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate fcutoff_temp ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate epsi fom ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom = (float *)Mem_SegAlloc(
        "MOD SOM EFE OBP dissrate chi fom ptr",DEF_NULL,
        MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD*sizeof(float),
        &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  //ALB initialize field ctd data
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure    = 0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature = 0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity    = 0;

  //ALB initialize all parameters. They should be reset right before
  //ALB cpt_spectra task is starts running.
//  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dof_flag=false;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->consumed_flag=false;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt=0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrate_skipped=0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->started_flg=false;


  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   stop fill segment task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_stop_fill_segment_task_f(){
  if(!mod_som_efe_obp_ptr->fill_segment_ptr->started_flg)
    return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

  if(efe_obp_fill_segment_task_tcb.TaskState != OS_TASK_STATE_DEL){
      RTOS_ERR err;
      OSTaskDel(&efe_obp_fill_segment_task_tcb,
                &err);

      //  mod_som_efe_obp_ptr->started_flag=false;


      if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  }
  mod_som_efe_obp_ptr->fill_segment_ptr->started_flg = false;
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   stop compute spectra task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_stop_cpt_spectra_task_f(){
  if(!mod_som_efe_obp_ptr->cpt_spectra_ptr->started_flg){
      return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
  }

  if(efe_obp_cpt_spectra_task_tcb.TaskState != OS_TASK_STATE_DEL){
      RTOS_ERR err;
      OSTaskDel(&efe_obp_cpt_spectra_task_tcb,
                &err);

      //  mod_som_efe_obp_ptr->started_flag=false;


      if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  }
  mod_som_efe_obp_ptr->cpt_spectra_ptr->started_flg =false;
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   stop cpt dissrate task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_stop_cpt_dissrate_task_f(){
  if(!mod_som_efe_obp_ptr->cpt_dissrate_ptr->started_flg){
      return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
  }

  if(efe_obp_cpt_dissrate_task_tcb.TaskState != OS_TASK_STATE_DEL){
      RTOS_ERR err;
      OSTaskDel(&efe_obp_cpt_dissrate_task_tcb,
                &err);

      //  mod_som_efe_obp_ptr->started_flag=false;

      if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  }
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->started_flg = false;
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   start fill segment task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_start_fill_segment_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
  //ALB initialize all parameters. They should be reset right before
  //ALB fill_segment task is starts running.
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_element_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_skipped=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->started_flg=true;

  //ALB initialize field ctd data
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_pressure=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_temperature=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_salinity=0;
  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_dpdt=0;

   OSTaskCreate(&efe_obp_fill_segment_task_tcb,
                        "efe obp fill segment task",
                        mod_som_efe_obp_fill_segment_task_f,
                        DEF_NULL,
                        MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_PRIO,
            &efe_obp_fill_segment_task_stk[0],
            (MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_STK_SIZE / 10u),
             MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   create producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_start_cpt_spectra_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->dof=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->spectra_skipped=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->volt_read_index=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->started_flg=true;

   //ALB initialize field ctd data
   mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure     = 0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature  = 0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity     = 0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt         = 0;

   mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_pressure=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_temperature=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_salinity=0;
   mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_dpdt=0;


   OSTaskCreate(&efe_obp_cpt_spectra_task_tcb,
                        "efe obp spectra task",
                        mod_som_efe_obp_cpt_spectra_task_f,
                        DEF_NULL,
                        MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_PRIO,
            &efe_obp_cpt_spectra_task_stk[0],
            (MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_STK_SIZE / 10u),
            MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * @brief
 *   create producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_start_cpt_dissrate_task_f(){


  RTOS_ERR err;
  // Consumer Task 2
//  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dof_flag=false;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->consumed_flag=false;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt    = 0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrate_skipped = 0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->started_flg      = true;


  //ALB initialize field ctd data
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure=0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature=0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity=0;
  mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt=0;

  //ALB re-initialize sample count
  mod_som_efe_obp_ptr->sample_count=0;

  OSTaskCreate(&efe_obp_cpt_dissrate_task_tcb,
               "efe obp cpt_dissrate task",
               mod_som_efe_obp_cpt_dissrate_task_f,
               DEF_NULL,
               MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_PRIO,
               efe_obp_cpt_dissrate_task_stk,
               (MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_STK_SIZE / 10u),
               MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   fill_segment task function
 *
 *   gather, convert cnt to volt and fill the segment float array
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_efe_obp_fill_segment_task_f(void  *p_arg){
  RTOS_ERR  err;
  int error_cnt = 0;

  (void)p_arg; // Deliberately unused argument

  float    * local_efeobp_efe_temp_ptr;
  float    * local_efeobp_efe_shear_ptr;
  float    * local_efeobp_efe_accel_ptr;

  uint8_t efe_element_length=MOD_SOM_EFE_TIMESTAMP_LENGTH+                     \
                    mod_som_efe_obp_ptr->efe_settings_ptr->number_of_channels* \
                            AD7124_SAMPLE_LENGTH;
  uint8_t local_efe_sample[mod_som_efe_obp_ptr->efe_settings_ptr->number_of_channels* \
                           AD7124_SAMPLE_LENGTH];

  int elmnts_avail=0, reset_segment_cnt=0;
  int data_elmnts_offset=0;
  int fill_segment_offset=0;

//  uint32_t cnsmr_indx=0;

  int padding = 5; // the padding should be big enough to include the time variance.

  //ALB GET the efe run time structure
  mod_som_efe_ptr_t local_mod_som_efe_ptr = mod_som_efe_get_runtime_ptr_f();
  uint8_t * curr_data_ptr   = local_mod_som_efe_ptr->rec_buff->efe_elements_buffer;

  float local_fill_segment_ctd_temperature=0;
  float local_fill_segment_ctd_pressure=0;
  float local_fill_segment_ctd_salinity=0;
  float local_fill_segment_ctd_dpdt=0;
  float local_fill_segment_ctd_direction=none;

  //local_sbe41_ptr
  //ALB get runtime sbe41 for futur use.
  local_sbe41_ptr=mod_som_sbe41_get_runtime_ptr_f();

  local_fill_segment_ctd_pressure =
      local_sbe41_ptr->consumer_ptr->record_pressure[1];

  local_fill_segment_ctd_temperature =
      local_sbe41_ptr->consumer_ptr->record_temp[1];

  local_fill_segment_ctd_salinity =
      local_sbe41_ptr->consumer_ptr->record_salinity[1];

  local_fill_segment_ctd_dpdt=
      local_sbe41_ptr->consumer_ptr->dPdt;

  local_fill_segment_ctd_direction=local_sbe41_ptr->consumer_ptr->direction;

  while (DEF_ON) {



      /************************************************************************/
      //ALB phase 1
      //ALB append the new block of data to the volt segment
      //ALB parse timestamp and efe data
      //ALB convert EFE samples into volts
      //ALB fill the volt-producer buffers

      if (mod_som_efe_obp_ptr->fill_segment_ptr->started_flg){
          elmnts_avail = local_mod_som_efe_ptr->sample_count -
              mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt;  //calculate number of elements available have been produced

          //ALB User stopped efe. I need to reset the obp producers count
          if(elmnts_avail<0){
              mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt = 0;
              mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt     = 0;
              mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt     = 0;
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt   = 0;
          }
          // LOOP without delay until caught up to latest produced element
          local_fill_segment_ctd_direction=local_sbe41_ptr->consumer_ptr->direction;
          while ((elmnts_avail > 0) & (local_fill_segment_ctd_direction==up))
            {
              // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
              // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
              // calculate the number of data we skipped, report number of elements skipped.
              // Reset the consumers cnt equal with producer data plus padding
              if (elmnts_avail>local_mod_som_efe_ptr->config_ptr->element_per_buffer){ // checking over flow. TODO check adding padding is correct.
                  // reset the consumer count less one buffer than producer count plus padding
                  //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                  reset_segment_cnt = local_mod_som_efe_ptr->sample_count -
                      local_mod_som_efe_ptr->config_ptr->element_per_buffer +
                      padding;
                  // calculate the number of skipped elements
                  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_skipped = reset_segment_cnt -
                      mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt;

                  mod_som_io_print_f("\nefe obp fill seg task: CB overflow: sample count = %lu,"
                      "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                      (uint32_t)local_mod_som_efe_ptr->sample_count, \
                      (uint32_t)mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt, \
                      (uint32_t)mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_skipped);

                  mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt = reset_segment_cnt;
              }

              // calculate the offset for current pointer
              data_elmnts_offset = mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt %
                                   local_mod_som_efe_ptr->config_ptr->element_per_buffer;

              fill_segment_offset     = mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt %
                  (MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD*
                  mod_som_efe_obp_ptr->settings_ptr->nfft);


              // update the current element pointer using the element map
              curr_data_ptr   =
                 (uint8_t*)
                 local_mod_som_efe_ptr->config_ptr->element_map[data_elmnts_offset];

              local_efeobp_efe_temp_ptr =
                  mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr +
                  fill_segment_offset;
              local_efeobp_efe_shear_ptr =
                  mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr +
                  fill_segment_offset;
              local_efeobp_efe_accel_ptr =
                  mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr +
                  fill_segment_offset;


              //ALB save each efe_sample inside the efe_block in a local_efe_sample;
              memcpy(local_efe_sample,
                     curr_data_ptr+MOD_SOM_EFE_TIMESTAMP_LENGTH,
                     efe_element_length);
              //ALB
              //local efe sample contains only the ADC samples.
              //convert the local efe sample from counts to volts
              //store the results directly in mod_som_efe_obp_ptr->producer_ptr->volt_ptr
              mod_som_efe_obp_cnt2volt_f(local_efe_sample,
                                         local_efeobp_efe_temp_ptr,
                                         local_efeobp_efe_shear_ptr,
                                         local_efeobp_efe_accel_ptr);


              mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt++;  // increment cnsmr count
              elmnts_avail = local_mod_som_efe_ptr->sample_count -
                      mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt; //elements available have been produced

              //ALB a segment is full. Store last PTS value
              if(((mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt %
                  (mod_som_efe_obp_ptr->settings_ptr->nfft)) ==0) &
                  (mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt>0)){

                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_pressure=
                      local_fill_segment_ctd_pressure;
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_temperature=
                      local_fill_segment_ctd_temperature;
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_salinity=
                      local_fill_segment_ctd_salinity;
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_dpdt=
                      local_fill_segment_ctd_dpdt;

//ALB do this if we want to average PTS dPdt
//                  local_fill_segment_ctd_pressure=0;
//                  local_fill_segment_ctd_temperature=0;
//                  local_fill_segment_ctd_salinity=0;
//                  local_fill_segment_ctd_dpdt=0;
//                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_element_cnt=0;

                  mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt++;
              }


              //ALB get the timestamp, PTS at half of the segment
              if(((mod_som_efe_obp_ptr->fill_segment_ptr->efe_element_cnt %
                  (mod_som_efe_obp_ptr->settings_ptr->nfft/2)) ==0)){

                  memcpy(mod_som_efe_obp_ptr->fill_segment_ptr->timestamp_segment_ptr+
                         (mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt%
                             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)),
                             (uint64_t*) curr_data_ptr,
                             sizeof(uint64_t));
                  mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt++;

                  if(local_sbe41_ptr->collect_data_flag){
                      //ALB sbe41_ptr->consumer_ptr has 2 slots for PTS.
                      //ALB 0 is the former sample. 1 is the most recent.
                      local_fill_segment_ctd_pressure =
                          local_sbe41_ptr->consumer_ptr->record_pressure[1];

                      local_fill_segment_ctd_temperature =
                          local_sbe41_ptr->consumer_ptr->record_temp[1];

                      local_fill_segment_ctd_salinity =
                          local_sbe41_ptr->consumer_ptr->record_salinity[1];

                      local_fill_segment_ctd_dpdt=
                          local_sbe41_ptr->consumer_ptr->dPdt;

                      local_fill_segment_ctd_direction=
                          local_sbe41_ptr->consumer_ptr->direction;


                      mod_som_efe_obp_ptr->fill_segment_ptr->ctd_element_cnt++;
                  }else{
                      //ALB no SBE41 sample. Make fake ones
                      local_fill_segment_ctd_dpdt=0.5;
                      local_fill_segment_ctd_temperature=10;
                      local_fill_segment_ctd_pressure=100;
                      local_fill_segment_ctd_salinity=33;
                  }
              }
            }  // end of while (elemts_avail > 0)

          // ALB done with segment storing.
          mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped = 0;


      } //end while DEF_ON

      // Delay Start Task execution for
      OSTimeDly( MOD_SOM_EFE_OBP_FILL_SEGMENT_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
          error_cnt = 0;
      }
      else{
          error_cnt++;
      }
      if(error_cnt>MOD_SOM_MAX_ERROR_CNT){
          mod_som_io_print_f("%s error accumulation maxed\r\n",__func__);
          return;
      }
//      //   Check error code.
//      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}


/*******************************************************************************
 * @brief
 *   producer cpt spectra function
 *
 *   this will be a state machine:
 *   compute freq spectra and mean time of the segment(i.e., segment unique timestamp)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_efe_obp_cpt_spectra_task_f(void  *p_arg){

  RTOS_ERR  err;
  int error_cnt = 0;
  uint64_t tick;

  (void)p_arg; // Deliberately unused argument


  int segment_avail=0,reset_spectra_cnt=0;
  int spectra_offset=0;
  int padding = 0; // the padding should be big enough to include the time variance.


  while (DEF_ON) {



      /************************************************************************/
      //ALB phase 1
      //ALB check for available spectra
      //ALB compute the FFT.
      //ALB
      //ALB
      if (mod_som_efe_obp_ptr->cpt_spectra_ptr->started_flg &
          (mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt>0)){
          //ALB half_segment_cnt-1 so I get the number of segment available with 50 % overlap;
          segment_avail = mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt-1 -
              mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt;  //calculate number of elements available have been produced
          // LOOP without delay until caught up to latest produced element
          while (segment_avail > 0)
            {
              // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
              // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
              // calculate the number of data we skipped, report number of elements skipped.
              // Reset the consumers cnt equal with producer data plus padding

              if (segment_avail>(2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)-1){ // checking over flow. TODO check adding padding is correct.
                  // reset the consumer count less one buffer than producer count plus padding
                  //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                  reset_spectra_cnt = mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt -
                                    (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD) +
                                    padding;
                  // calculate the number of skipped elements
                  mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped = reset_spectra_cnt -
                      mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt;

                  mod_som_io_print_f("\nefe obp cpt spec: CB overflow: sample count = %lu,"
                      "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                      (uint32_t)mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt, \
                      (uint32_t)mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt, \
                      (uint32_t)mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped);

                  mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt = reset_spectra_cnt;
              }


              //ALB Get the CTD data and timestamps at the half segment (see fill_segment_task)
              mod_som_efe_obp_ptr->cpt_spectra_ptr->timestamp=
                  (uint64_t) *(mod_som_efe_obp_ptr->fill_segment_ptr->timestamp_segment_ptr+
                      ((mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt-1)%
                  (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)));

              mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_pressure=
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_pressure;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_temperature=
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_temperature;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_salinity=
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_salinity;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_dpdt  =
                  mod_som_efe_obp_ptr->fill_segment_ptr->ctd_dpdt;



          tick=sl_sleeptimer_get_tick_count64();
          mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                                &mod_som_efe_obp_ptr->start_computation_timestamp);

          //ALB I need to fill up the segment buffer with half segment for each channel.
          //ALB I need that buffer because
          //ALB I organized the segment circular buffer in half-segment.
          //ALB I need a way to create a segment with the last half-segment and the first half segment.
          //ALB My solution is a segment buffer in which I ll copy the segments one after the other.
          uint32_t indx = 0;

          //ALB start copying the segment in the record buffer.
          //ALB I do it it in 2 times to write 2 half segments in order
          //ALB to handle the weird case of the end of the timeseries
          //ALB halfseg4 and halfseg1.

          // start with shear
          memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                 &mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr[
                  (mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt%
                   (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                 * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                   mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

            indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2;

            memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                   &mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr[
                  ((mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt+1)%
                   (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                 * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                   mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
            // call shear spectrum function
            mod_som_efe_obp_shear_spectrum_f(mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr, spectra_offset, mod_som_efe_obp_ptr);
            // move on to temperature


            indx = 0;
            memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                   &mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr[
                  (mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt%
                   (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                 * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                   mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

            indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2;

            memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                   &mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr[
                  ((mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt+1)%
                   (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                 * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                   mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
            // call temperature spectrum function
            mod_som_efe_obp_temp_spectrum_f(mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr, spectra_offset, mod_som_efe_obp_ptr);
            // last do acceleration

            indx = 0;
            memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                     &mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr[
                     (mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt%
                     (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                    * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                      mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

            indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2;

            memcpy(&mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr[indx],
                   &mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr[
                   ((mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt+1)%
                   (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
                  * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
                    mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

            // call accel spectrum function
            mod_som_efe_obp_accel_spectrum_f(mod_som_efe_obp_ptr->fill_segment_ptr->segment_buffer_ptr, spectra_offset, mod_som_efe_obp_ptr);
//
//
//            //ALB update the current spectra timestamp (middle segment timestamp )
//            mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp+=
//                mod_som_efe_obp_ptr->cpt_spectra_ptr->timestamp/
//                (uint64_t)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
//
//            //ALB sum ctd temp,pressure/salinty
//            mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure+=
//                mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_pressure/
//                (float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
//
//            mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity+=
//                mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_salinity/
//                (float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
//
//            mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature+=
//                mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_temperature/
//                (float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
//
//            mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt+=
//                mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_dpdt/
//                (float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;

          //ALB update mod_som_efe_obp_ptr->producer_ptr->volt_read_index
          //ALB to get a new segment with a 50% overlap.
          mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt++;
          mod_som_efe_obp_ptr->cpt_spectra_ptr->dof ++;
          mod_som_efe_obp_ptr->cpt_spectra_ptr->dof %=
          mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;

          segment_avail = mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt -
              mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt; //elements available have been produced

          spectra_offset= (mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt %
                          MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)*
                           mod_som_efe_obp_ptr->settings_ptr->nfft/2;

          mod_som_efe_obp_ptr->fill_segment_ptr->segment_skipped=0;

          //ALB handle the avg spectra when we reach dof
          if ((mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt %
                mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom)==0)
              {

              //ALB for debbugpurpose get the last timestamp last PTS
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->timestamp;

              //ALB sum ctd temp,pressure/salinty
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_pressure;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_salinity;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_temperature;

              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->ctd_dpdt;

              memcpy(mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr,
                     mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr,
                      mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
              memcpy(mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr,
                     mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr,
                      mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
              memcpy(mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr,
                     mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr,
                      mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

              mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt;
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure;
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature;
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity;
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp=
                  mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp;


              //ALB convert avg specs
             mod_som_efe_obp_correct_convert_avg_spectra_f(mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr,
                                                           mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr,
                                                           mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr,
                                                           mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt);


              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt++;
//              mod_som_efe_obp_ptr->settings_ptr->format=3;

              //ALB reset avg ctd data
              //ALB NB: I do not reset the FOCO since the reset is embedded
              //ALB NB: in the cpt-spectra functions.
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt        = 0;
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure    = 0;
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature = 0;
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity    = 0;
              mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_timestamp       = 0;

//              mod_som_efe_obp_ptr->cpt_dissrate_ptr->dof_flag           = true;

              } //ALB done with dof
            }//ALB done with segment available
      }//ALB done computing spectra

      // Delay Start Task execution for

      OSTimeDly( MOD_SOM_EFE_OBP_CPT_SPECTRA_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
          error_cnt = 0;
      }
      else{
          error_cnt++;
      }
      if(error_cnt>MOD_SOM_MAX_ERROR_CNT){
          mod_som_io_print_f("%s error accumulation maxed\r\n",__func__);
          return;
      }
      //   Check error code.
//      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)
  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}



/*******************************************************************************
 * @brief
 *   producer task function
 *
 *   this will be a state machine:
 *    3 - compute epsilon and chi
 *
 *   compute nu and kappa
 *   convert frequency to wavenumber
 *   sum Fourier coef
 *   get frequency cut-off
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_efe_obp_cpt_dissrate_task_f(void  *p_arg){
  RTOS_ERR  err;
  int error_cnt = 0;
  (void)p_arg; // Deliberately unused argument


//  int i;
  int spectra_avail=0;

  float * local_epsilon_ptr;
  float * local_chi_ptr;
  float * local_nu_ptr;
  float * local_kappa_ptr;
  float * local_kcutoff_shear_ptr;
  float * local_fcutoff_temp_ptr;
  float * local_epsi_fom_ptr;
  float * local_chi_fom_ptr;




  while (DEF_ON) {
      /************************************************************************/
      //ALB phase 3
      //ALB start computing epsilon chi.
      //ALB check first if spectra_ready_flg true: i.e.,do we have enough dof
      if (mod_som_efe_obp_ptr->cpt_dissrate_ptr->started_flg){

          spectra_avail = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt -
          mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt;  //calculate number of elements available have been produced

          while (spectra_avail > 0)
//          if (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dof_flag)
            {

              //ALB update pointers
              local_epsilon_ptr = mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_chi_ptr     = mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_nu_ptr      = mod_som_efe_obp_ptr->cpt_dissrate_ptr->nu +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_kappa_ptr   = mod_som_efe_obp_ptr->cpt_dissrate_ptr->kappa +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_kcutoff_shear_ptr   = mod_som_efe_obp_ptr->cpt_dissrate_ptr->kcutoff_shear +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_fcutoff_temp_ptr   = mod_som_efe_obp_ptr->cpt_dissrate_ptr->fcutoff_temp +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_epsi_fom_ptr     = mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);
              local_chi_fom_ptr     = mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom +
                             (mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt%
                              MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD);


//              //ALB reset  dof_flag so we know when the avg spec is converted;
//               mod_som_efe_obp_ptr->cpt_dissrate_ptr->dof_flag=false;


              //CAP Add compute epsilon chi
              //CAP Add compute spectra functions

               mod_som_efe_obp_compute_dissrate_data_f(mod_som_efe_obp_ptr     ,
                                                      local_epsilon_ptr        ,
                                                      local_chi_ptr            ,
                                                      local_nu_ptr             ,
                                                      local_kappa_ptr          ,
                                                      local_kcutoff_shear_ptr  ,
                                                      local_fcutoff_temp_ptr   ,
                                                      local_epsi_fom_ptr       ,
                                                      local_chi_fom_ptr);


              //ALB increment the counters
              mod_som_efe_obp_ptr->sample_count++;

              //get spectral offset
//              avg_spectra_offset=(mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt*
//                  mod_som_efe_obp_ptr->settings_ptr->nfft/2) %
//                      (MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD*
//                          mod_som_efe_obp_ptr->settings_ptr->nfft/2);

              //ALB increment dissrate_cnt
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt++;
              spectra_avail = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt -
              mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt;  //calculate number of elements available have been produced

          } //ALB end of dof if loop.
     } //end started flag

      // Delay Start Task execution for
      OSTimeDly( MOD_SOM_EFE_OBP_CPT_DISSRATE_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
          error_cnt = 0;
      }
      else{
          error_cnt++;
      }
      if(error_cnt>MOD_SOM_MAX_ERROR_CNT){
          mod_som_io_print_f("%s error accumulation maxed\r\n",__func__);
          return;
      }
      //   Check error code.
//      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

 } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}







/*******************************************************************************
 * @brief
 *   mod_som_efe_obp_cnt2volt_f
 *
 *   - Get a efe sample split in  nb_channels * ADC sample bytes
 *     default: 7 channels * 3 bytes
 *   - convert each ADC counts into volts. The conversion depends on the ADC config
 *     Unipolar = @(FR,data) (FR/gain*double(data)/2.^(bit_counts));
 *     Bipolar  = @(FR,data) (FR/gain*(double(data)/2.^(bit_counts-1)-1));
 *     FR = Full Range 2.5V
 *     bit counts: 24 bit
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t mod_som_efe_obp_cnt2volt_f(uint8_t * efe_sample,
                                            float   * local_efeobp_efe_temp_ptr,
                                            float   * local_efeobp_efe_shear_ptr,
                                            float   * local_efeobp_efe_accel_ptr)
{
uint32_t adc_sample=0;
float    adc_sample_volt=0;
uint32_t index=0;

  //ALB loop ing over each ADC sample and storing them in the seg_volt array.
  //ALB Since I am only storing 3 channels (1 temp, 1 shear,1 accell) I am checking
  //ALB which channels the user selected.
  for(int i=0;i<MOD_SOM_EFE_OBP_CHANNEL_NUMBER;i++)
    {
      index=mod_som_efe_obp_ptr->settings_ptr->channels_id[i]*AD7124_SAMPLE_LENGTH;
      adc_sample=0;
      adc_sample=     (uint32_t) (efe_sample[index]<<16 |
                                 efe_sample[index+1]<<8 |
                                 efe_sample[index+2]);

      switch(mod_som_efe_obp_ptr->efe_settings_ptr->sensors[i].registers.CONFIG_0){
        case 0x1e0: //unipolar
          adc_sample_volt  = MOD_SOM_EFE_OBP_FULL_RANGE/
                             MOD_SOM_EFE_OBP_GAIN*
                             adc_sample/
                             pow(2,MOD_SOM_EFE_OBP_ADC_BIT);
          //SN pow 2 could be improve timewise.
          //SN We could have pow(2,MOD_SOM_EFE_OBP_ADC_BIT) as a pre-defined number.
          //SN We could move the factors
          //SN MOD_SOM_EFE_OBP_FULL_RANGE, MOD_SOM_EFE_OBP_GAIN etc to later.
          //SN to reduce the conversion time.
          break;
        case 0x9e0: //bipolar
          adc_sample_volt  = MOD_SOM_EFE_OBP_FULL_RANGE/
                             MOD_SOM_EFE_OBP_GAIN*
                             (adc_sample/
                             pow(2,MOD_SOM_EFE_OBP_ADC_BIT-1)-1);
          //SN pow 2 could be improve timewise.
          //SN We could have pow(2,MOD_SOM_EFE_OBP_ADC_BIT) as a pre-defined number.
          //SN We could move the factors
          //SN MOD_SOM_EFE_OBP_FULL_RANGE, MOD_SOM_EFE_OBP_GAIN etc to later.
          //SN to reduce the conversion time.
          break;
      }

      //ALB t1 and t2 are idx 0 and 1 in the efe_channel_id
      //ALB Check efe_settings_ptr to convince your self.
      if ((mod_som_efe_obp_ptr->settings_ptr->channels_id[i]==0))
      {
          memcpy( local_efeobp_efe_temp_ptr,
                 &adc_sample_volt,
                  sizeof(float));
      }

      //ALB s1 and s2 are idx 2 and 3 in the efe_channel_id
      //ALB Check efe_settings_ptr to convince your self.
      if ((mod_som_efe_obp_ptr->settings_ptr->channels_id[i]==1))
      {
          memcpy( local_efeobp_efe_shear_ptr,
                 &adc_sample_volt,
                  sizeof(float));
      }

      //ALB a1, a2, a3 are idx 4, 5 and 6.
      //ALB Check efe_settings_ptr to convince your self.
      if ((mod_som_efe_obp_ptr->settings_ptr->channels_id[i]==2))
      {
          memcpy( local_efeobp_efe_accel_ptr,
                 &adc_sample_volt,
                  sizeof(float));
          //ALB conve
          *local_efeobp_efe_accel_ptr=((*local_efeobp_efe_accel_ptr)-
                                      MOD_SOM_EFE_OBP_ACCELL_OFFSET)/
                                      MOD_SOM_EFE_OBP_ACCELL_FACTOR;
      }
  }//end for loop nb_channel

  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Computes FFT from segments
 *   once the efe obp starts it fill segment array.
 *   We need to compute the FFT over the record (i.e. data received)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_compute_spectra_data_f(float * local_temp_seg_ptr,
                                                        float * local_shear_seg_ptr,
                                                        float * local_accel_seg_ptr,
                                                        float * local_temp_spec_ptr,
                                                        float * local_shear_spec_ptr,
                                                        float * local_accel_spec_ptr
                                                        )
{
  //CAP
  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * @brief
 *   Computes dissrate from averaged segment
 *   once the efe obp starts it fill segment array.
 *   We need to compute the FFT over the record (i.e. data received)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_compute_dissrate_data_f(
                                                      mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr,
                                                      float * local_epsilon,
                                                      float * local_chi,
                                                      float * local_nu,
                                                      float * local_kappa,
                                                      float * local_kcutoff_shear,
                                                      float * local_fcutoff_temp,
                                                      float * local_epsi_fom,
                                                      float * local_chi_fom
                                                      )
{

  //CAP
  mod_som_efe_obp_calc_epsilon_f(local_epsilon, local_nu, local_epsi_fom, local_kcutoff_shear, mod_som_efe_obp_ptr);
  mod_som_efe_obp_calc_chi_f(local_epsilon, local_chi, local_kappa,local_fcutoff_temp, local_chi_fom, mod_som_efe_obp_ptr);
  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   create consumer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_start_consumer_task_f(){

  RTOS_ERR err;
  // Consumer Task 2

  if(!mod_som_efe_obp_ptr->fill_segment_ptr->started_flg){
      printf("EFE OBP not started\r\n");
      return (mod_som_efe_obp_ptr->status = mod_som_efe_obp_encode_status_f(MOD_SOM_EFE_OBP_NOT_STARTED));
  }


  mod_som_efe_obp_ptr->consumer_ptr->segment_cnt  = 0;
  mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt = 0;
  mod_som_efe_obp_ptr->consumer_ptr->rates_cnt    = 0;
  mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt    = 0;
  mod_som_efe_obp_ptr->consumer_ptr->started_flg  = true;

  OSTaskCreate(&efe_obp_consumer_task_tcb,
                       "efe obp consumer task",
                       mod_som_efe_obp_consumer_task_f,
                       DEF_NULL,
                       MOD_SOM_EFE_OBP_CONSUMER_TASK_PRIO,
           &efe_obp_consumer_task_stk[0],
           (MOD_SOM_EFE_OBP_CONSUMER_TASK_STK_SIZE / 10u),
           MOD_SOM_EFE_OBP_CONSUMER_TASK_STK_SIZE,
           0u,
           0u,
           DEF_NULL,
           (OS_OPT_TASK_STK_CLR),
           &err);


 // Check error code
 APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
 if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
   return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));

  return mod_som_efe_obp_encode_status_f(MOD_SOM_STATUS_OK);

}
/*******************************************************************************
 * @brief
 *   stop consumer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_stop_consumer_task_f(){

  if(!mod_som_efe_obp_ptr->started_flag){
      return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
  }

  if(efe_obp_consumer_task_tcb.TaskState != OS_TASK_STATE_DEL){
      RTOS_ERR err;
      OSTaskDel(&efe_obp_consumer_task_tcb,
                &err);

      if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return (mod_som_efe_obp_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK));
  }
  mod_som_efe_obp_ptr->started_flag=false;
   mod_som_efe_obp_ptr->settings_ptr->format=1;

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   consumer task function
 *
 *   the user can choose between different mode of consumption
 *   0: none
 *   1: stream data (volt timeseries,volt spectra, epsi, chi)
 *   2: SDstore data (volt timeseries,volt spectra epsi, chi)
 *   3: stream and store (volt timeseries,volt spectra, epsi, chi)
 *   4: apf - The efe obp communicates with the apf module
 *
 *
 * @return
 *
 ******************************************************************************/
void mod_som_efe_obp_consumer_task_f(void  *p_arg){

  RTOS_ERR err;
  int error_cnt = 0;
  uint32_t segment_avail;
  uint32_t spectrum_avail;
  uint32_t rates_avail;
  uint32_t avgspec_avail;

  uint32_t payload_length=0;
  int reset_segment_cnt=0;
  int reset_spectrum_cnt=0;
  int reset_rates_cnt=0;
  int reset_avgspec_cnt=0;

  uint64_t tick;

  uint8_t * curr_consumer_record_ptr;

  mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
       mod_som_sdio_get_runtime_ptr_f();

   mod_som_sdio_file_ptr_t rawfile_ptr =
       local_mod_som_sdio_ptr_t->rawdata_file_ptr;

  //        printf("In Consumer Task 2\n");
  while (DEF_ON) {

      if (mod_som_efe_obp_ptr->consumer_ptr->started_flg &
          (mod_som_efe_obp_ptr->fill_segment_ptr->segment_cnt>=1)){

          switch(mod_som_efe_obp_ptr->settings_ptr->format){
            case 1:
              //spit out segment
              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
                      MOD_SOM_EFE_OBP_CONSUMER_SEGMENT_TAG,
                      MOD_SOM_EFE_OBP_TAG_LENGTH);
              break;
            case 2:
              //spit out freq spectrum
              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
                      MOD_SOM_EFE_OBP_CONSUMER_SPECTRA_TAG,
                      MOD_SOM_EFE_OBP_TAG_LENGTH);
              break;
            case 3:
              //spit out avg spectrum
              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
                      MOD_SOM_EFE_OBP_CONSUMER_AVGSPEC_TAG,
                      MOD_SOM_EFE_OBP_TAG_LENGTH);
              break;
            case 0:
              //spit out dissrate
              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
                      MOD_SOM_EFE_OBP_CONSUMER_RATE_TAG,
                      MOD_SOM_EFE_OBP_TAG_LENGTH);
              break;
          }

          switch(mod_som_efe_obp_ptr->settings_ptr->format){
            case 1:
//              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
//                     MOD_SOM_EFE_OBP_CONSUMER_SEGMENT_TAG,
//                     MOD_SOM_EFE_OBP_TAG_LENGTH);

              //ALB phase 1: check elements available and copy them in the cnsmr buffer
              segment_avail=mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt-1-
                                mod_som_efe_obp_ptr->consumer_ptr->segment_cnt;

              if(segment_avail>0){
                  if (segment_avail>(2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)){
                      reset_segment_cnt =
                          mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt -
                          (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD) ;
                      // calculate the number of skipped elements
                      mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped =
                          reset_segment_cnt -
                          mod_som_efe_obp_ptr->consumer_ptr->segment_cnt;

                      mod_som_io_print_f("\nEFE OBP consumer task: "
                                         "segment overflow sample count = "
                                         "%lu,cnsmr_cnt = %lu,"
                                         "skipped %lu elements\r\n",
                           (uint32_t)mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->segment_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped);

                      mod_som_efe_obp_ptr->consumer_ptr->segment_cnt =
                                                                  reset_segment_cnt;
                  }//ALB end deal with overflow


                  //ALB cpy the segments in the cnsmr buffer.
                  payload_length=mod_som_efe_obp_copy_producer_segment_f();
                  //ALB increase counter.
                  mod_som_efe_obp_ptr->consumer_ptr->segment_cnt++;

                  segment_avail=mod_som_efe_obp_ptr->fill_segment_ptr->half_segment_cnt-1-
                                    mod_som_efe_obp_ptr->consumer_ptr->segment_cnt;
            }
              break;

            case 2:
              //ALB phase 1: check elements available and copy them in the cnsmr buffer
              spectrum_avail=mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt-
                                mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt;
              if(spectrum_avail>0){
                  if (spectrum_avail>MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD){
                      reset_spectrum_cnt =
                          mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt -
                          MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD ;
                      // calculate the number of skipped elements
                      mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped =
                          reset_spectrum_cnt -
                          mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt;

                      mod_som_io_print_f("\nEFE OBP consumer task: "
                                         "spectra overflow sample count = "
                                         "%lu,cnsmr_cnt = %lu,"
                                         "skipped %lu elements\r\n",
                           (uint32_t)mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped);

                      mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt =
                                                                  reset_spectrum_cnt;
                  }//ALB end deal with overflow

                  //ALB cpy the segments in the cnsmr buffer.
                  payload_length=mod_som_efe_obp_copy_producer_spectra_f();
                  //ALB increase counter.
                  mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt++;
                  spectrum_avail=mod_som_efe_obp_ptr->cpt_spectra_ptr->spectrum_cnt-
                      mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt;
              }
              break;

            case 3:
              //ALB phase 1: check elements available and copy them in the cnsmr buffer
              avgspec_avail=mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt-
                                mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt;
              if(avgspec_avail>0){
                  if (avgspec_avail>MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD){
                      reset_avgspec_cnt =
                          mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt -
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD ;
                      // calculate the number of skipped elements
                      mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped =
                          reset_avgspec_cnt -
                          mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt;

                      mod_som_io_print_f("\nEFE OBP consumer task: "
                                         "rates overflow sample count = "
                                         "%lu,cnsmr_cnt = %lu,"
                                         "skipped %lu elements\r\n",
                           (uint32_t)mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt,
                           (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped);

                      mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt =
                          reset_avgspec_cnt;
                  }//ALB end deal with overflow


                  //ALB cpy the segments in the cnsmr buffer.
                  payload_length=mod_som_efe_obp_copy_producer_avgspectra_f();

                  mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt++;
                  avgspec_avail=mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_spectrum_cnt-
                                    mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt;

              }
              break;
            case 0:
              break;

          }

          //ALB consume the dissrates
          rates_avail=mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt-
                            mod_som_efe_obp_ptr->consumer_ptr->rates_cnt;
          if((rates_avail>0) & (payload_length==0)){
              memcpy(mod_som_efe_obp_ptr->consumer_ptr->tag,
                     MOD_SOM_EFE_OBP_CONSUMER_RATE_TAG,
                     MOD_SOM_EFE_OBP_TAG_LENGTH);

              if (rates_avail>MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD){
                  reset_rates_cnt =
                      mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt -
                      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD ;
                  // calculate the number of skipped elements
                  mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped =
                      reset_rates_cnt -
                      mod_som_efe_obp_ptr->consumer_ptr->rates_cnt;

                  mod_som_io_print_f("EFE OBP consumer task: "
                                     "rates overflow sample count = "
                                     "%lu,cnsmr_cnt = %lu,"
                                     "skipped %lu elements\r\n",
                       (uint32_t)mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt,
                       (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->rates_cnt,
                       (uint32_t)mod_som_efe_obp_ptr->consumer_ptr->elmnts_skipped);

                  mod_som_efe_obp_ptr->consumer_ptr->rates_cnt =
                                                              reset_rates_cnt;
              }//ALB end deal with overflow


              //ALB cpy the segments in the cnsmr buffer.
              payload_length=mod_som_efe_obp_copy_producer_dissrate_f();

              mod_som_efe_obp_ptr->consumer_ptr->rates_cnt++;
              rates_avail=mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt-
                                mod_som_efe_obp_ptr->consumer_ptr->rates_cnt;

          }


          if(payload_length>0){

          //get the timestamp for the record header
          tick=sl_sleeptimer_get_tick_count64();
          mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                           &mod_som_efe_obp_ptr->consumer_ptr->record_timestamp);

          //MHA: Now augment timestamp by poweron_offset_ms
          mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
          mod_som_efe_obp_ptr->consumer_ptr->record_timestamp +=
                                    mod_som_calendar_settings.poweron_offset_ms;


          mod_som_efe_obp_ptr->consumer_ptr->payload_length=payload_length;

          //ALB create header
          mod_som_efe_obp_header_f(mod_som_efe_obp_ptr->consumer_ptr);
          //add header to the beginning of the stream block
          memcpy(mod_som_efe_obp_ptr->consumer_ptr->record_ptr, \
                 mod_som_efe_obp_ptr->consumer_ptr->header,
                 mod_som_efe_obp_ptr->consumer_ptr->length_header);


          //ALB compute checksum
          curr_consumer_record_ptr=mod_som_efe_obp_ptr->consumer_ptr->record_ptr+
              mod_som_efe_obp_ptr->consumer_ptr->length_header;
          mod_som_efe_obp_ptr->consumer_ptr->chksum=0;
          for(int i=0;i<mod_som_efe_obp_ptr->consumer_ptr->payload_length;i++)
            {
              mod_som_efe_obp_ptr->consumer_ptr->chksum ^=\
                  curr_consumer_record_ptr[i];
            }


          //ALB the curr_consumer_element_ptr should be at the right place to
          //ALB write checksum at the end of the record.
          curr_consumer_record_ptr+=
                              mod_som_efe_obp_ptr->consumer_ptr->payload_length;
          *(curr_consumer_record_ptr++) = '*';
          *((uint16_t*)curr_consumer_record_ptr) = \
              mod_som_int8_2hex_f(mod_som_efe_obp_ptr->consumer_ptr->chksum);
          curr_consumer_record_ptr += 2;
          *(curr_consumer_record_ptr++) = '\r';
          *(curr_consumer_record_ptr++) = '\n';

          //ALB get the length of the record with the checksum
          mod_som_efe_obp_ptr->consumer_ptr->record_length= \
              (int) &curr_consumer_record_ptr[0]- \
              (int) &mod_som_efe_obp_ptr->consumer_ptr->record_ptr[0];




          switch(mod_som_efe_obp_ptr->settings_ptr->mode){
            case 0:
              //ALB stream

              mod_som_efe_obp_ptr->consumer_ptr->consumed_flag=false;
              mod_som_io_stream_data_f(
                  mod_som_efe_obp_ptr->consumer_ptr->record_ptr,
                  mod_som_efe_obp_ptr->consumer_ptr->record_length,
                  &mod_som_efe_obp_ptr->consumer_ptr->consumed_flag);

              break;
            case 1:
              //ALB store
              mod_som_efe_obp_ptr->consumer_ptr->consumed_flag=false;
              mod_som_sdio_write_data_f(rawfile_ptr,
                  mod_som_efe_obp_ptr->consumer_ptr->record_ptr,
                  mod_som_efe_obp_ptr->consumer_ptr->record_length,
                  &mod_som_efe_obp_ptr->consumer_ptr->consumed_flag);


              break;
            case 2:
              //ALB stream and store
              break;

          }//end switch efe_obp mode
          while(!mod_som_efe_obp_ptr->consumer_ptr->consumed_flag){};
          payload_length=0;
          //ALB small code to switch between spectrum and shear output
          switch(mod_som_efe_obp_ptr->settings_ptr->format){
            case 1:
              mod_som_efe_obp_ptr->settings_ptr->format=2;
              break;
            case 2:
              if(mod_som_efe_obp_ptr->cpt_dissrate_ptr->dissrates_cnt-
              mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt){
                  mod_som_efe_obp_ptr->settings_ptr->format=3;
              }else{
                  mod_som_efe_obp_ptr->settings_ptr->format=1;
              }
              break;
            case 3:
              mod_som_efe_obp_ptr->settings_ptr->format=1;
              break;
            default:
              mod_som_efe_obp_ptr->settings_ptr->format=1;
              break;

          }
          }// end if payload length =0
  }//end if mod_som_efe_obp_ptr->consumer_ptr->started_flg
  // Delay Start Task execution for
  OSTimeDly( MOD_SOM_EFE_OBP_CONSUMER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
             OS_OPT_TIME_DLY,          //   from now.
             &err);
  if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
      error_cnt = 0;
  }
  else{
      error_cnt++;
  }
  if(error_cnt>MOD_SOM_MAX_ERROR_CNT){
      mod_som_io_print_f("%s error accumulation maxed\r\n",__func__);
      return;
  }
  //   Check error code.
//  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
} // end of while (DEF_ON)

PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.


}//end consumer task

/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_efe_obp_header_f(mod_som_efe_obp_data_consumer_ptr_t consumer_ptr)
{

  //time stamp
  uint32_t t_hex[2];
  uint8_t * local_header;


  t_hex[0] = (uint32_t) (mod_som_efe_obp_ptr->consumer_ptr->record_timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_efe_obp_ptr->consumer_ptr->record_timestamp;

  //header  contains $EFE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
  sprintf((char*) mod_som_efe_obp_ptr->consumer_ptr->header,  \
      "$%.4s%08x%08x%08x*FF", \
      mod_som_efe_obp_ptr->consumer_ptr->tag, \
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
                                       MOD_SOM_EFE_OBP_LENGTH_HEADER_CHECKSUM+1];
  *((uint16_t*)local_header) = \
      mod_som_int8_2hex_f(consumer_ptr->header_chksum);

}


  /*******************************************************************************
   * @brief
   * ALB  copy segments in the cnsmr buffer segment_ptr
   * ALB  The plan so far is to store only 1 segment and consume it right after.
   * ALB  We start to fill the segment at the end of the header.
   * ALB  Once the memcopy is done I compute the checksum.
   *
   * @return
   *   MOD_SOM_STATUS_OK if initialization goes well
   *   or otherwise
   ******************************************************************************/

uint32_t mod_som_efe_obp_copy_producer_segment_f()
  {
  uint32_t indx = mod_som_efe_obp_ptr->config_ptr->header_length;
    //copy the prdcr segment inside cnsmr segment.

      memcpy( &mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
              mod_som_efe_obp_ptr->fill_segment_ptr->timestamp_segment_ptr+
              ((mod_som_efe_obp_ptr->consumer_ptr->segment_cnt)%
                  (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD)),
                  sizeof(uint64_t));

      indx+=sizeof(uint64_t);

      //ALB start copying the segment in the record buffer.
      //ALB I do it it in 2 times to write 2 half segments in order
      //ALB to handle the weird case of the end of the timeseries
      //ALB halfseg4 and halfseg1.
      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr[
            (mod_som_efe_obp_ptr->consumer_ptr->segment_cnt%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
           * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_temp_volt_ptr[
            ((mod_som_efe_obp_ptr->consumer_ptr->segment_cnt+1)%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
           * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);


      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr[
            (mod_som_efe_obp_ptr->consumer_ptr->segment_cnt%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
           * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_shear_volt_ptr[
            ((mod_som_efe_obp_ptr->consumer_ptr->segment_cnt+1)%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
           * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr[
             (mod_som_efe_obp_ptr->consumer_ptr->segment_cnt%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
            * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
              mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->fill_segment_ptr->seg_accel_volt_ptr[
             ((mod_som_efe_obp_ptr->consumer_ptr->segment_cnt+1)%
             (2*MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD))
            * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
              mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));


      //ALB return the length of the payload
      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);
      indx-=mod_som_efe_obp_ptr->config_ptr->header_length;
     return indx;
  }

  /*******************************************************************************
   * @brief
   *   copy spectra in the cnsmr buffer
   *
   *
   * @return
   *   MOD_SOM_STATUS_OK if initialization goes well
   *   or otherwise
   ******************************************************************************/

  uint32_t mod_som_efe_obp_copy_producer_spectra_f()
  {
    uint32_t indx=mod_som_efe_obp_ptr->config_ptr->header_length;
    //copy the prdcr spectra inside cnsmr spectra buffer.


    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_spectra_ptr->timestamp,
           sizeof(uint64_t));

    indx+=sizeof(uint64_t);

//      switch(mod_som_efe_obp_ptr->consumer_ptr->channel){
//    case temp:

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr[
                      (mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt%
                       MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                       * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
           mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
//    break;
//    case shear:
    indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr[
                        (mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt%
                         MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                         * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
//      break;
//    case accel:
      indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);
      memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
             &mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr[
                        (mod_som_efe_obp_ptr->consumer_ptr->spectrum_cnt%
                         MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                         * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
             mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));
//      break;
//      }

    //ALB return the length of the payload
    indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);
    indx-=mod_som_efe_obp_ptr->config_ptr->header_length;
     return indx;
  }


  /*******************************************************************************
   * @brief
   *   copy spectra in the cnsmr buffer
   *
   *
   * @return
   *   MOD_SOM_STATUS_OK if initialization goes well
   *   or otherwise
   ******************************************************************************/

  uint32_t mod_som_efe_obp_copy_producer_avgspectra_f()
  {
    uint32_t indx=mod_som_efe_obp_ptr->config_ptr->header_length;


    //copy the prdcr spectra inside cnsmr spectra buffer.
    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp,
           sizeof(uint64_t));

    indx+=sizeof(uint64_t);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr[
                      (mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt%
                       MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                       * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
           mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

    indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[
                      (mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt%
                       MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                       * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
           mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

    indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_accel_ptr[
                      (mod_som_efe_obp_ptr->consumer_ptr->avgspec_cnt%
                       MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD)
                       * mod_som_efe_obp_ptr->settings_ptr->nfft/2],
           mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float));

    indx+=mod_som_efe_obp_ptr->settings_ptr->nfft/2*sizeof(float);
    indx-=mod_som_efe_obp_ptr->config_ptr->header_length;
    //ALB return the length of the payload
     return indx;
  }

  /*******************************************************************************
   * @brief
   *   copy spectra in the cnsmr buffer
   *
   *
   * @return
   *   MOD_SOM_STATUS_OK if initialization goes well
   *   or otherwise
   ******************************************************************************/

  uint32_t mod_som_efe_obp_copy_producer_dissrate_f()
  {
    uint32_t indx=mod_som_efe_obp_ptr->config_ptr->header_length;


    //copy the prdcr spectra inside cnsmr spectra buffer.
//    memcpy((void *) &mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
//           (void *) &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp[
//                     (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
//                      MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
//                     ],sizeof(uint64_t));
    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp,
           sizeof(uint64_t));

    indx+=sizeof(uint64_t);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure,
                    sizeof(float));
    indx+=sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature,
                    sizeof(float));
    indx+=sizeof(float);
    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity,
                    sizeof(float));
    indx+=sizeof(float);
    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt,
                    sizeof(float));
    indx+=sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));
    indx+=sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->chi_fom[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));
    indx+=sizeof(float);


    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsilon[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));

    indx+=sizeof(float);

    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->epsi_fom[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));
    indx+=sizeof(float);


    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->nu[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));

    indx+=sizeof(float);
    memcpy(&mod_som_efe_obp_ptr->consumer_ptr->record_ptr[indx],
           &mod_som_efe_obp_ptr->cpt_dissrate_ptr->kappa[
                      (mod_som_efe_obp_ptr->consumer_ptr->rates_cnt%
                          MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD)
                       ],sizeof(float));

    indx+=sizeof(float);
    indx-=mod_som_efe_obp_ptr->config_ptr->header_length;
    //ALB return the length of the payload
     return indx;
  }


// ALB command functions //SN edited to be functions called in shell //SN need to edit comments
/*******************************************************************************
 * @brief
 *   change efeobp.format  for the 'efeobp.format' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_consumer_format_f(CPU_INT16U argc,CPU_CHAR *argv[])
{

  RTOS_ERR  p_err;
  uint8_t format;

  if (argc==1){
      printf("efeobp.format %lu\r\n.", mod_som_efe_obp_ptr->settings_ptr->format);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      format=shellStrtol(argv[1],&p_err);
      if(format<4){
          mod_som_efe_obp_ptr->settings_ptr->format=format;
      }else{
          printf("format: efeobp.format format (0:segment, 1:spectra, 2:avgspec 3: only dissrates)\r\n");
      }
      break;
    default:
      printf("format: efeobp.format format (0:segment, 1:spectra, 2:avgspec 3: only dissrates)\r\n");
      break;
    }
  }

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   change efeobp.mode  for the 'efeobp.mode' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_mode_f(CPU_INT16U argc,CPU_CHAR *argv[])
{

  RTOS_ERR  p_err;
  uint8_t mode;

  if (argc==1){
      printf("efeobp.mode %lu\r\n.", mod_som_efe_obp_ptr->settings_ptr->mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mode=shellStrtol(argv[1],&p_err);
      if(mode<3){
          mod_som_efe_obp_ptr->settings_ptr->mode=mode;
      }else{
          printf("mode: efeobp.mode mode (0:stream, 1:store, 2: other)\r\n");
      }
      break;
    default:
      printf("mode: efeobp.mode mode (0:stream, 1:store, 2: other)\r\n");
      break;
    }
  }

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


// ALB command functions //SN edited to be functions called in shell //SN need to edit comments
/*******************************************************************************
 * @brief
 *   change efeobp.mode  for the 'efeobp.mode' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_consumer_channel_f(CPU_INT16U argc,CPU_CHAR *argv[])
{

  RTOS_ERR  p_err;
  uint8_t channel;

  if (argc==1){
      printf("efeobp.channel %lu\r\n.", mod_som_efe_obp_ptr->settings_ptr->channel);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      channel=shellStrtol(argv[1],&p_err);
      if(channel<3){
          mod_som_efe_obp_ptr->settings_ptr->channel=channel;
      }else{
          printf("format: efeobp.channel (0:temperature, 1:shear, 2: acceleration)\r\n");
      }
      break;
    default:
      printf("format: efeobp.channel (0:temperature, 1:shear, 2: acceleration)\r\n");
      break;
    }
  }

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
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
uint8_t mod_som_efe_obp_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_EFE_OBP_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_efe_obp_encode_status_f
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
mod_som_status_t mod_som_efe_obp_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_EFE_OBP_STATUS_PREFIX, mod_som_io_status);
}

#endif

