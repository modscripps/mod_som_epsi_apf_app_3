/*******************************************************************************
 * @file mod_som_efe_obp.h
 * @brief MOD SOM foo bar API
 * @date Mar 26, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API implementation file defines functions to be used with FOO BAR
 * module running on the MOD SOM board.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_EFE_OBP_H_
#define MOD_SOM_EFE_OBP_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include <efe/mod_som_efe.h>


//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_EFE_OBP_STATUS_PREFIX        99U
#define MOD_SOM_EFE_OBP_STATUS_FAIL_INIT_CMD 0x2U

//settings
#define MOD_SOM_EFE_OBP_DEFAULT_NFFT            4096
#define MOD_SOM_EFE_RECORD_DEFAULT_FORMAT       0
#define MOD_SOM_EFE_TELEMETRY_DEFAULT_FORMAT    0
#define MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH     8
#define MOD_SOM_EFE_OBP_SYNC_LENGTH             1
#define MOD_SOM_EFE_OBP_TAG_LENGTH              4
#define MOD_SOM_EFE_OBP_MAX_HEADER_SIZE         100
#define MOD_SOM_EFE_OBP_HEXTIMESTAMP_LENGTH     16
#define MOD_SOM_EFE_OBP_PAYLOAD_LENGTH          8
#define MOD_SOM_EFE_OBP_HEADER              "OBPE"
#define MOD_SOM_EFE_OBP_CHANNEL_NUMBER          3      // number of channels being processed. Default should be t1,s1,a3.

#define MOD_SOM_EFE_OBP_SYNC_TAG_LENGTH                      1
#define MOD_SOM_EFE_OBP_HEADER_TAG_LENGTH                    4
#define MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH                  8
#define MOD_SOM_EFE_OBP_LENGTH_HEADER_CHECKSUM               3

//ALB data consumer
#define MOD_SOM_EFE_OBP_CONSUMER_TASK_PRIO           18u
#define MOD_SOM_EFE_OBP_CONSUMER_TASK_STK_SIZE       512u
#define MOD_SOM_EFE_OBP_CONSUMER_DELAY               10          // delay for consumer 2 (ex: 4000 = 1 time / 4 secs).
#define MOD_SOM_EFE_OBP_CONSUMER_PADDING             5          // number of elements for padding for data consumer.
#define MOD_SOM_EFE_OBP_CONSUMER_NB_SEGMENT_PER_RECORD       1
#define MOD_SOM_EFE_OBP_CONSUMER_NB_SPECTRA_PER_RECORD       1
#define MOD_SOM_EFE_OBP_CONSUMER_NB_DISSRATE_PER_RECORD      1
#define MOD_SOM_EFE_OBP_CONSUMER_SEGMENT_TAG             "SEGM"
#define MOD_SOM_EFE_OBP_CONSUMER_SPECTRA_TAG             "SPEC"
#define MOD_SOM_EFE_OBP_CONSUMER_RATE_TAG                "RATE"
#define MOD_SOM_EFE_OBP_CONSUMER_PAYLOAD_CHECKSUM_LENGTH     5

//ALB Fill segment
#define MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_PRIO              18u
#define MOD_SOM_EFE_OBP_FILL_SEGMENT_TASK_STK_SIZE          512u
#define MOD_SOM_EFE_OBP_FILL_SEGMENT_DELAY                  10      // delay for fill segment task
#define MOD_SOM_EFE_OBP_FILL_SEGMENT_NB_SEGMENT_PER_RECORD  2       // this number wants to be as low as possible it can creates huge arrrays

//ALB compute spectra
#define MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_PRIO              18u
#define MOD_SOM_EFE_OBP_CPT_SPECTRA_TASK_STK_SIZE          512u
#define MOD_SOM_EFE_OBP_CPT_SPECTRA_DELAY                  10      // delay for compute spectra task
#define MOD_SOM_EFE_OBP_CPT_SPECTRA_NB_SPECTRA_PER_RECORD  1       // this number wants to be as low as possible it can creates huge arrrays
#define MOD_SOM_EFE_OBP_CPT_SPECTRA_DEGREE_OF_FREEDOM      5       //

//ALB compute dissrate
#define MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_PRIO                    18u
#define MOD_SOM_EFE_OBP_CPT_DISSRATE_TASK_STK_SIZE                512u
#define MOD_SOM_EFE_OBP_CPT_DISSRATE_DELAY                        10      // delay for compute dissrate task.
#define MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_AVG_SPECTRA_PER_RECORD    1
#define MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD          10

//error handling
#define MOD_SOM_EFE_OBP_CANNOT_ALLOCATE_SETUP                       0x01u
#define MOD_SOM_EFE_OBP_CANNOT_OPEN_CONFIG                          0x02u
#define MOD_SOM_EFE_OBP_STATUS_FAIL_TO_ALLOCATE_MEMORY              0x03u
#define MOD_SOM_EFE_OBP_STATUS_ERR_NOT_INITIALIZED                  0x04U
#define MOD_SOM_EFE_OBP_STATUS_ERR_NOT_STARTED                      0x05U
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY          0x06U
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_ENQUEUE                  0x07U
#define EFE_OBP_OVF_MSG_LIST_THERSHOLD                              0x08U
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE        0x09U
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_FREE_MEMORY              0x10u
#define MOD_SOM_EFE_OPB_STATUS_FAIL_TO_START_CONSUMER_TASK          0x11u
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE         0x12u
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_FLUSH_QUEUE              0x13u
#define MOD_SOM_EFE_OBP_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY  0x14u
#define MOD_SOM_EFE_OBP_NOT_STARTED                                 0x15u


//count to volt
#define MOD_SOM_EFE_OBP_FULL_RANGE 2.5 //2.5 Volt
#define MOD_SOM_EFE_OBP_GAIN       1
#define MOD_SOM_EFE_OBP_ADC_BIT    24

//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Structure to store set up config parameters for MOD SOM EFE
 *
 * @field is_initialized_flag
 *
 * @field communication_config_usart
 *
 * @field communication_config_leuart
 *
 *
 ******************************************************************************/
typedef struct{
    uint32_t  initialized_flag;
    uint32_t  header_length;
    uint32_t  nb_sample_per_segment;
    uint8_t   num_shear;
    uint8_t   num_fp07;
    float     f_samp;
    float     f_CTD_pump;


}mod_som_efe_obp_config_t, *mod_som_efe_obp_config_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM EFE
 *
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char header[MOD_SOM_EFE_OBP_SETTINGS_STR_lENGTH];
    uint32_t nfft; //ALB 2048 if float speed is 20 cm/s or 4096 if float speed is 10 cm/s
    uint32_t degrees_of_freedom; // suggested number is 4
    uint32_t record_format;
    uint32_t telemetry_format;
    uint32_t channels_id[MOD_SOM_EFE_OBP_CHANNEL_NUMBER];     //
    uint32_t initialize_flag;
}
mod_som_efe_obp_settings_t, *mod_som_efe_obp_settings_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure managing the stream of EFE OBP data
 ******************************************************************************/
typedef struct{
  bool       started_flg;        //ALB ???
  uint32_t   record_length;          //ALB length of the streaming data buffer
  uint32_t   max_record_length;
  uint32_t   length_header;
  uint32_t   max_payload_length;
  uint32_t   payload_length;
  uint8_t    header[MOD_SOM_EFE_OBP_MAX_HEADER_SIZE];
  char       tag[MOD_SOM_EFE_OBP_TAG_LENGTH];
  enum       {segment,spectra,dissrate}mode;
  enum       {temp,shear,accel}channel;

  uint32_t   segments_length;
  uint32_t   spectra_length;
  uint32_t   rates_length;
  uint32_t   diffusitvity_length;

  uint64_t   segment_cnt;
  uint64_t   spectrum_cnt;
  uint64_t   rates_cnt;
  uint64_t   record_timestamp;

  uint8_t * record_ptr;     //ALB pointer to the segments section in the record

  uint8_t    data_ready_flg;           //ALB ???
  uint8_t    header_chksum;
  uint8_t    chksum;
  uint32_t   elmnts_skipped;
  bool       consumed_flag;
}
mod_som_efe_obp_data_consumer_t, *mod_som_efe_obp_data_consumer_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
 ******************************************************************************/
typedef struct{

  uint64_t efe_element_cnt;
  uint64_t efe_element_skipped;
  uint64_t ctd_element_cnt;

  uint64_t segment_cnt;
  uint64_t half_segment_cnt;

  uint64_t timestamp_segment;
  float * seg_temp_volt_ptr;
  float * seg_shear_volt_ptr;
  float * seg_accel_volt_ptr;

  float avg_ctd_pressure;
  float avg_ctd_temperature;
  float avg_ctd_salinity;
  float avg_ctd_fallrate;

  bool  started_flg;        //ALB ???

  uint32_t segment_skipped;
}
mod_som_efe_obp_data_fill_segment_t, *mod_som_efe_obp_data_fill_segment_ptr_t;



/*******************************************************************************
 * @brief
 *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
 ******************************************************************************/
typedef struct{

  uint64_t spectrum_cnt;

  uint32_t volt_read_index;
  uint8_t  dof;
  uint64_t avg_timestamp;

  float * spec_temp_ptr;        //ALB pointer to spectrum
  float * spec_shear_ptr;        //ALB pointer to spectrum
  float * spec_accel_ptr;        //ALB pointer to spectrum

  float avg_ctd_pressure;
  float avg_ctd_temperature;
  float avg_ctd_salinity;
  float avg_ctd_fallrate;

  bool  started_flg;        //ALB ???

  uint32_t spectra_skipped;
}
mod_som_efe_obp_data_cpt_spectra_t, *mod_som_efe_obp_data_cpt_spectra_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
 ******************************************************************************/
typedef struct{

  bool dof_flag;

  uint64_t dissrates_cnt;
  uint64_t spectrum_cnt;
  uint64_t avg_timestamp[MOD_SOM_EFE_OBP_CPT_DISSRATE_NB_RATES_PER_RECORD];

  float * avg_spec_temp_ptr;        //ALB pointer to spectrum
  float * avg_spec_shear_ptr;        //ALB pointer to spectrum
  float * avg_spec_accel_ptr;        //ALB pointer to spectrum

  float avg_ctd_pressure;
  float avg_ctd_temperature;
  float avg_ctd_salinity;
  float avg_ctd_fallrate;

  float * nu;

  float * kappa;
  float * epsilon;
  float * chi;
  float * fom; //figure of merit

  bool  started_flg;        //ALB ???

  uint32_t dissrate_skipped;
}
mod_som_efe_obp_data_cpt_dissrate_t, *mod_som_efe_obp_data_cpt_dissrate_ptr_t;



/*******************************************************************************
 * @brief
 *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
 ******************************************************************************/
typedef struct{
  float    * shear_sv;
  float    * fp07_dTdV;
  uint16_t   Tdiff_size;
  float    * Tdiff_freq;
  float    * Tdiff_coeff;
  uint16_t   cafilter_size;
  float    * cafilter_freq;
  float    * cafilter_coeff;
  float      fp07_noise[4];

}
mod_som_efe_obp_calibration_t, *mod_som_efe_obp_calibration_ptr_t;



/*******************************************************************************
 * @brief
 *   Run-time structure for MOD SOM EFE
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    uint32_t started_flag;
    uint32_t error_flag;
    uint64_t sample_count;
    uint64_t start_computation_timestamp;
    uint64_t stop_computation_timestamp;
    uint8_t  mode;
    uint8_t  format;

    mod_som_efe_obp_settings_ptr_t      settings_ptr;      //
    mod_som_efe_settings_ptr_t          efe_settings_ptr;  //
    mod_som_efe_obp_config_ptr_t        config_ptr;        //
    mod_som_efe_obp_data_consumer_ptr_t consumer_ptr;
    mod_som_efe_obp_data_fill_segment_ptr_t fill_segment_ptr;
    mod_som_efe_obp_data_cpt_spectra_ptr_t  cpt_spectra_ptr;
    mod_som_efe_obp_data_cpt_dissrate_ptr_t cpt_dissrate_ptr;
    uint32_t sampling_flag;
    uint8_t data_ready_flag;
    mod_som_status_t status;

}mod_som_efe_obp_t,*mod_som_efe_obp_ptr_t;



//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize efe obp, if shell is available, then the command table is added
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_init_f();


/*******************************************************************************
 * @brief
 *   once the efe data are in the obp producer.
 *   We need to parse [t1,t2,s1,s2,a1,a2,a3] and convert then into floats [Volts]
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_parse_efe_data_f();


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
                                                       );

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
                                                      float * local_temp_avg_spec_ptr,
                                                      float * local_shear_avg_spec_ptr,
                                                      float * local_accel_avg_spec_ptr,
                                                      float * local_epsilon,
                                                      float * local_chi,
                                                      float * local_nu,
                                                      float * local_kappa,
                                                      float * local_fom
                                                      );


/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_say_hello_world_f();

/*******************************************************************************
 * @brief
 *   construct settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_allocate_settings_ptr_f();

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
mod_som_status_t mod_som_efe_obp_default_settings_f(mod_som_efe_obp_settings_ptr_t settings_ptr);

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_efe_obp_settings_t mod_som_efe_obp_get_settings_f();

/*******************************************************************************
 * @brief
 *   get the efe obp runtime ptr
 *
 * @param
 *
 ******************************************************************************/
mod_som_efe_obp_ptr_t mod_som_efe_obp_get_runtime_ptr_f();

/*******************************************************************************
 * @brief
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_config_ptr_f();

/*******************************************************************************
 * @brief
 *   construct consumer structure
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_consumer_ptr_f();

/*******************************************************************************
 * @brief
 *   construct producers structures
 *   fill segment producer
 *   compute spectra producer
 *   compute dissipation rate producer
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_construct_fill_segment_ptr_f();
mod_som_status_t mod_som_efe_obp_construct_cpt_spectra_ptr_f();
mod_som_status_t mod_som_efe_obp_construct_cpt_dissrate_ptr_f();


/*******************************************************************************
 * @brief
 *   create producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_start_fill_segment_task_f();
mod_som_status_t mod_som_efe_obp_stop_fill_segment_task_f();

mod_som_status_t mod_som_efe_obp_start_cpt_spectra_task_f();
mod_som_status_t mod_som_efe_obp_stop_cpt_spectra_task_f();

mod_som_status_t mod_som_efe_obp_start_cpt_dissrate_task_f();
mod_som_status_t mod_som_efe_obp_stop_cpt_dissrate_task_f();


mod_som_status_t mod_som_efe_obp_start_consumer_task_f();
mod_som_status_t mod_som_efe_obp_stop_consumer_task_f();


/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_efe_obp_header_f(mod_som_efe_obp_data_consumer_ptr_t consumer_ptr);

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

uint32_t mod_som_efe_obp_copy_producer_segment_f();


/*******************************************************************************
 * @brief
 *   copy spectra in the cnsmr buffer
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

uint32_t mod_som_efe_obp_copy_producer_spectra_f();

/*******************************************************************************
 * @brief
 *   copy spectra in the cnsmr buffer
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

uint32_t mod_som_efe_obp_copy_producer_dissrate_f();


/*******************************************************************************
 * @brief
 *   change efeobp.mode  for the 'efeobp.mode' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_consumer_mode_f(
    CPU_INT16U argc,
    CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   change efeobp.mode  for the 'efeobp.mode' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_obp_consumer_channel_f(
    CPU_INT16U argc,
    CPU_CHAR *argv[]);


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
mod_som_status_t mod_som_efe_obp_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_efe_obp_decode_status_f(mod_som_status_t mod_som_status);
#endif /* MOD_SOM_EFE_OBP_H_ */
