/*
 * mod_som_apf.c
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */


#include <apf/mod_som_apf.h>

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

    //ALB Allocate memory for the producer pointer,
    //ALB using the settings_ptr variable
    status |= mod_som_apf_construct_producer_ptr_f();
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

  mod_som_apf_ptr->producer_ptr->initialized_flag = false;
  mod_som_apf_ptr->producer_ptr->started_flg  = false;
  mod_som_apf_ptr->producer_ptr->collect_flg  = false;
  mod_som_apf_ptr->producer_ptr->dacq_full    = false;
  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dacq_size    =
                           mod_som_apf_ptr->producer_ptr->dacq_ptr-
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dissrates_cnt_offset=0;


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
  mod_som_apf_ptr->consumer_ptr->data_ptr=
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

  mod_som_apf_ptr->consumer_ptr->initialized_flag = true;
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
  mod_som_apf_ptr->producer_ptr->avg_timestamp    = 0;
  mod_som_apf_ptr->producer_ptr->collect_flg      = false;
  mod_som_apf_ptr->producer_ptr->dissrate_skipped = 0;
  mod_som_apf_ptr->producer_ptr->dissrates_cnt    = 0;
  mod_som_apf_ptr->producer_ptr->initialized_flag = false;

  mod_som_apf_ptr->producer_ptr->dacq_full    = false;
  mod_som_apf_ptr->producer_ptr->dacq_ptr     =
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dacq_size    =
                           mod_som_apf_ptr->producer_ptr->dacq_ptr-
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];
  mod_som_apf_ptr->producer_ptr->dissrates_cnt_offset=0;

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
  mod_som_apf_ptr->consumer_ptr->data_ptr=
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

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

  int64_t * curr_avg_timestamp_ptr;
  float * curr_temp_avg_spectra_ptr;
  float * curr_shear_avg_spectra_ptr;
  float * curr_accel_avg_spectra_ptr;
  float * curr_avg_pressure_ptr;
  float * curr_epsilon_ptr;
  float * curr_chi_ptr;
  float * curr_epsi_fom_ptr;
  float * curr_chi_fom_ptr;


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

                  mod_som_apf_ptr->producer_ptr->dissrates_cnt = reset_dissrate_cnt;
              }

              //ALB calculate the offset for current pointer
              dissrate_elmnts_offset = mod_som_apf_ptr->producer_ptr->dissrates_cnt %
                                       MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD;

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
                  &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_timestamp[0]+
                  dissrate_elmnts_offset;

              //ALB update avg timestamp
              curr_avg_pressure_ptr =
                  &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure+
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



              //ALB convert and store the current dissrate into the MOD format
              // log10(epsi) log10(chi):  3bytes (12 bits for each epsi and chi sample)
              mod_som_apf_copy_F0_element_f( curr_avg_timestamp_ptr,
                                             curr_avg_pressure_ptr,
                                             curr_epsilon_ptr,
                                             curr_chi_ptr,
                                             curr_epsi_fom_ptr,
                                             curr_chi_fom_ptr,
                                             mod_som_apf_ptr->producer_ptr->dacq_ptr
                                         );

              switch (mod_som_apf_ptr->comm_telemetry_packet_format){
                case F0:
                  mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
                  break;
                case F1:
                  break;
                case F2:
                  mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
                  break;
                case F3:
                  mod_som_apf_ptr->producer_ptr->dacq_element_size=0;
                  //ALB downgrade and store the avg_spectra ()
                  mod_som_apf_downgrade_spectra_f(curr_temp_avg_spectra_ptr,
                                                  curr_shear_avg_spectra_ptr,
                                                  curr_accel_avg_spectra_ptr,
                                                 mod_som_apf_ptr->producer_ptr->dacq_ptr
                                             );
                  break;
              }

              //ALB increment cnsmr count
              mod_som_apf_ptr->producer_ptr->dissrates_cnt++;

              //ALB update dissrate available
              dissrate_avail = mod_som_efe_obp_ptr->sample_count -
                  mod_som_apf_ptr->producer_ptr->dissrates_cnt; //elements available have been produced

              //ALB update dacq_size
              mod_som_apf_ptr->producer_ptr->dacq_size=
                           mod_som_apf_ptr->producer_ptr->dacq_ptr-
                          &mod_som_apf_ptr->producer_ptr->acq_profile.data_acq[0];

              //ALB raise flag stop producer if profile is full (size(dacq)>=25kB)
              //ALB update the number of sample
              if (mod_som_apf_ptr->producer_ptr->dacq_size+
                  mod_som_apf_ptr->producer_ptr->dacq_element_size>=
                  MOD_SOM_APF_DACQ_STRUCT_SIZE)
                {
                  mod_som_apf_ptr->producer_ptr->dacq_full=true;
                  mod_som_apf_ptr->producer_ptr->
                  acq_profile.mod_som_apf_meta_data.sample_cnt=
                      mod_som_apf_ptr->producer_ptr->dissrates_cnt-
                      mod_som_apf_ptr->producer_ptr->dissrates_cnt_offset;

                  mod_som_apf_ptr->producer_ptr->dissrates_cnt_offset=
                      mod_som_apf_ptr->producer_ptr->
                                   acq_profile.mod_som_apf_meta_data.sample_cnt;
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
 *   1 - store the dacq profile in a sd file (as the dacq_profile is getting filled up)
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

  int bytes_avail=0;

  uint8_t * curr_dacq_ptr;


  while (DEF_ON) {

      if (mod_som_apf_ptr->consumer_ptr->started_flg){
          bytes_avail = mod_som_apf_ptr->producer_ptr->dacq_size -
              mod_som_apf_ptr->consumer_ptr->dacq_size;  //calculate number of elements available have been produced

          //ALB  phase 1 continuously write the new data on the SD card until
          //ALB  we reach the limit size MOD_SOM_APF_DACQ_STRUCT_SIZE (25kB)
          //ALB  LOOP without delay until caught up to latest produced element
          //ALB
          while (bytes_avail > 0)
            {
              //ALB I am not using a similar desgin as the other consumer because
              //ALB I am not dealing with a circular buffer


              //ALB calculate the offset for current pointer
              curr_dacq_ptr =&mod_som_apf_ptr->consumer_ptr->data_ptr[0]+
                              mod_som_apf_ptr->consumer_ptr->dacq_size;


              // ALB send this block to the SD card
              mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
              mod_som_sdio_write_data_f(
                                 curr_dacq_ptr,
                                 bytes_avail,
                                &mod_som_apf_ptr->consumer_ptr->consumed_flag);



              //ALB update dacq size.
              mod_som_apf_ptr->consumer_ptr->dacq_size+= bytes_avail;

              //ALB update bytes available. Should always be 0 at that line.
              bytes_avail = mod_som_apf_ptr->producer_ptr->dacq_size -
                  mod_som_apf_ptr->consumer_ptr->dacq_size;

            }  // end of while (bytes_avail > 0)

          //ALB No bytes to write
          //ALB Check if profile is over
          //ALB Profile is over or reached his 25kB limit
          //ALB write the Meta_Data on the SD card
          if(mod_som_apf_ptr->producer_ptr->dacq_full){

              mod_som_apf_ptr->consumer_ptr->consumed_flag=false;
              mod_som_sdio_write_data_f(
  (uint8_t *) &mod_som_apf_ptr->producer_ptr->acq_profile.mod_som_apf_meta_data,
       sizeof(mod_som_apf_ptr->producer_ptr->acq_profile.mod_som_apf_meta_data),
                                 &mod_som_apf_ptr->consumer_ptr->consumed_flag);
          }


          //ALB
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
 * convert the dissrates into MOD format
 *    MOD epsilon is 3 bytes ranging from log10(1e-12) and log10(1e-3) V^2/Hz
 *    MOD chi is 3 bytes ranging from log10(1e-12) and log10(1e-3) V^2/Hz
 *    MOD fom is 1 byte
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_copy_F0_element_f(  uint64_t * curr_avg_timestamp_ptr,
                                    float * curr_pressure_ptr,
                                    float * curr_epsilon_ptr,
                                    float * curr_chi_ptr,
                                    float * curr_fom_epsi_ptr,
                                    float * curr_fom_temp_ptr,
                                    uint8_t * dacq_ptr)
{


  uint32_t mod_epsilon, mod_chi;
  uint8_t mod_epsi_fom,mod_chi_fom;
  uint16_t local_avg_dissrate_timestamp; //ALB nb of sec since dacq

  mod_som_apf_ptr->producer_ptr->dacq_element_size=
      MOD_SOM_APF_DACQ_TIMESTAMP_SIZE+
      MOD_SOM_APF_DACQ_PRESSURE_SIZE+
      MOD_SOM_APF_DACQ_DISSRATE_SIZE+
      MOD_SOM_APF_DACQ_FOM_SIZE;


  float local_epsilon   = log10(*curr_epsilon_ptr);
  float local_chi       = log10(*curr_chi_ptr);
  float local_epsi_fom  = *curr_epsilon_ptr;
  float local_chi_fom   = *curr_chi_ptr;


  uint8_t  mod_bit_epsilon[MOD_SOM_APF_PRODUCER_DISSRATE_RES] = {0};
  uint8_t  mod_bit_chi[MOD_SOM_APF_PRODUCER_DISSRATE_RES]     = {0};
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


  local_epsilon = MAX(local_epsilon,min_dissrate);
  local_chi     = MAX(local_chi,min_dissrate);

  local_epsilon = MIN(local_epsilon,max_dissrate);
  local_chi     = MIN(local_chi,max_dissrate);



  local_epsi_fom     = MAX(local_epsi_fom,min_fom);
  local_chi_fom      = MAX(local_chi_fom,min_fom);

  local_epsi_fom     = MIN(local_epsi_fom,max_fom);
  local_chi_fom      = MIN(local_chi_fom,max_fom);


  mod_epsilon  = (uint32_t) ceil(local_epsilon*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  mod_chi      = (uint32_t) ceil(local_chi*
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_per_bit+
           mod_som_apf_ptr->producer_ptr->decim_coef.dissrate_counts_at_origin);

  mod_epsi_fom  = (uint8_t) ceil(local_epsi_fom*
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                     mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;
  mod_chi_fom   = (uint8_t) ceil(local_chi_fom*
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_per_bit+
                       mod_som_apf_ptr->producer_ptr->decim_coef.fom_counts_at_origin) ;

  mod_bit_fom = (mod_epsi_fom<<4) + mod_chi_fom;

  for (int i=0;i<MOD_SOM_APF_PRODUCER_DISSRATE_RES;i++){
      mod_bit_epsilon[i]= mod_epsilon>>(8*i);
      mod_bit_chi[i]    = mod_chi>>(8*i);
  }



  memcpy(dacq_ptr,
         &local_avg_dissrate_timestamp,
         MOD_SOM_APF_DACQ_TIMESTAMP_SIZE);
  dacq_ptr+=MOD_SOM_APF_DACQ_TIMESTAMP_SIZE;
  memcpy(dacq_ptr,
         &mod_bit_chi,
         MOD_SOM_APF_PRODUCER_DISSRATE_RES);
  dacq_ptr+=MOD_SOM_APF_PRODUCER_DISSRATE_RES;
  memcpy(dacq_ptr,
         &mod_bit_epsilon,
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
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_downgrade_spectra_f(float   * curr_temp_avg_spectra_ptr,
                                     float   * curr_shear_avg_spectra_ptr,
                                     float   * curr_accel_avg_spectra_ptr,
                                     uint8_t * dacq_ptr)
{

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
  CPU_CHAR filename[100];

  mod_som_apf_ptr->profile_id=profile_id;
  //ALB Open SD file,
  sprintf(filename, "Profile%lu",(uint32_t) mod_som_apf_ptr->profile_id);
  mod_som_sdio_define_filename_f(filename);
  //ALB write MODSOM settings on the SD file
  mod_som_settings_sd_settings_f();
  //ALB initialize Meta_Data Structure, TODO
  mod_som_apf_init_meta_data(mod_som_apf_ptr->producer_ptr->
                             acq_profile.mod_som_apf_meta_data);


	//ALB start ADC master clock timer
  mod_som_apf_ptr->profile_id=profile_id;
  mod_som_apf_ptr->daq=true;
	//ALB start turbulence processing task
  status=mod_som_efe_obp_start_fill_segment_task_f();
  status|=mod_som_efe_obp_start_cpt_spectra_task_f();
  status|=mod_som_efe_obp_start_cpt_dissrate_task_f();

  //ALB start collecting CTD.
  status |= mod_som_sbe41_connect_f();
  status |= mod_som_sbe41_start_collect_data_f();

  //ALB get a P reading and define the dz to get 25kB in the producer->dacq_profile
  //TODO

  //ALB start APF producer task
  status |= mod_som_apf_start_producer_task_f();
  //ALB start APF consumer task
  status |= mod_som_apf_start_consumer_task_f();

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
      mod_som_io_print_f("daq,stop,ack\r\n");
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
  mod_som_apf_meta_data.daq_timestamp=sl_sleeptimer_get_time();

  mod_som_apf_meta_data.sample_cnt=0;
  mod_som_apf_meta_data.end_metadata=0xFFFF;
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

  if (argc==1){
      printf("comm_packet_format %u\r\n.", mod_som_apf_ptr->comm_telemetry_packet_format);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mode=shellStrtol(argv[1],&p_err);
      if(mode<3){
          mod_som_apf_ptr->comm_telemetry_packet_format=mode;
      }else{
          printf("format: comm_packet_format mode (0:F0, 1:F1, 2: F2, 3:F3)\r\n");
      }
      break;
    default:
      printf("format: comm_packet_format mode (0:F0, 1:F1, 2: F2, 3:F3)\r\n");
      break;
    }
  }


  mod_som_io_print_f("comm_packet_format,ak\r\n");
  mod_som_io_print_f("comm_packet_format,nak,%s\r\n","error message");
  return mod_som_apf_encode_status_f(MOD_SOM_APF_STATUS_OK);
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

  }else{
      mod_som_io_print_f("upload,nak,%lu\r\n",status);
  }
  return mod_som_apf_encode_status_f(status);
}
#endif


