/*
 * mod_som_apf.h
 *
 *  Created on: Apr 9, 2020
 *      Author: aleboyer
 */

// TODO create a MOD_SOM_APF_STATUS_OK to replace a MOD_SOM_STATUS_OK

#ifndef MOD_APF_MOD_SOM_APF_H_
#define MOD_APF_MOD_SOM_APF_H_


//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_APF_STATUS_PREFIX 99U

#define MOD_SOM_APF_STATUS_FAIL_INIT_CMD 0x2U


#define MOD_SOM_APF_SHELL_STR_LENGTH 64
#define MOD_SOM_APF_VIBRATION_CUT_OFF   50
#define MOD_SOM_APF_HEADER0              "APF0"
#define MOD_SOM_APF_HEADER1              "APF1"
#define MOD_SOM_APF_HEADER2              "APF2"


#define MOD_SOM_APF_ENDACK_STR            "\r\n"
#define MOD_SOM_APF_ACK_STR               "ack"
#define MOD_SOM_APF_NACK_STR              "nak"
#define MOD_SOM_APF_DAQ_STR               "daq"
#define MOD_SOM_APF_DAQSTAT_STR           "daq?"
#define MOD_SOM_APF_TIME_STR              "time"
#define MOD_SOM_APF_TIMESTAT_STR          "time?"
#define MOD_SOM_APF_OKSTAT_STR            "ok?"
#define MOD_SOM_APF_SLEEP_STR             "sleep"
#define MOD_SOM_APF_GATE_STR              "gate"
#define MOD_SOM_APF_FWREV_STAT_STR        "frw_rev?"
#define MOD_SOM_APF_UPLOAD_STR            "upload"
#define MOD_SOM_APF_EPSINO_STAT_STR       "epsi_no?"
#define MOD_SOM_APF_PROBENO_STR           "probe_no"
#define MOD_SOM_APF_PROBENO_STAT_STR      "probe_no?"
#define MOD_SOM_APF_POWEROFF_STR          "poweroff"
#define MOD_SOM_APF_SDFORMAT_STAT_STR     "sd_format?"
#define MOD_SOM_APF_SDFORMAT_STR          "sd_format"
#define MOD_SOM_APF_PACKETFORMAT_STAT_STR "packet_format?"
#define MOD_SOM_APF_PACKETFORMAT_STR      "packet_format"


#define MOD_SOM_APF_SYNC_LENGTH             1
#define MOD_SOM_APF_TAG_LENGTH              4
#define MOD_SOM_APF_MAX_HEADER_SIZE         100
#define MOD_SOM_APF_LENGTH_HEADER_CHECKSUM  3
#define MOD_SOM_APF_HEXTIMESTAMP_LENGTH     16
#define MOD_SOM_APF_PAYLOAD_LENGTH          8
#define MOD_SOM_APF_HEADER_CHECKSUM_LENGTH  3
#define MOD_SOM_APF_PAYLOAD_LENGTH          8
#define MOD_SOM_APF_PAYLOAD_CHECKSUM_LENGTH 5

#define MOD_SOM_APF_SYNC_TAG_LENGTH         1
#define MOD_SOM_APF_HEADER_TAG_LENGTH       4
#define MOD_SOM_APF_SETTINGS_STR_LENGTH     8



#define MOD_SOM_APF_DACQ_STRUCT_SIZE         25000
//ALB test with a smaller payload
//#define MOD_SOM_APF_DACQ_STRUCT_SIZE         2500
#define MOD_SOM_APF_METADATA_SIZE            43
#define MOD_SOM_APF_DACQ_TIMESTAMP_SIZE      2
#define MOD_SOM_APF_DACQ_DISSRATE_SIZE       3
#define MOD_SOM_APF_DACQ_PRESSURE_SIZE       4
#define MOD_SOM_APF_DACQ_TEMPERATURE_SIZE    4
#define MOD_SOM_APF_DACQ_SALINITY_SIZE       4
#define MOD_SOM_APF_DACQ_KCUT_SIZE           4
#define MOD_SOM_APF_DACQ_FCUT_SIZE           4
#define MOD_SOM_APF_DACQ_DPDT_SIZE           4
#define MOD_SOM_APF_DACQ_FOCO_SIZE           4
#define MOD_SOM_APF_DACQ_FLOAT_SIZE          4
#define MOD_SOM_APF_DACQ_SEAWATER_SPEED_SIZE 4
#define MOD_SOM_APF_DACQ_FLAG_SIZE           1
#define MOD_SOM_APF_DACQ_FOM_SIZE            1
#define MOD_SOM_APF_DACQ_F3_NFFT_DECIM_COEF  8  //4096foco/8 -> 320Hz/4 -> 40Hz
#define MOD_SOM_APF_DACQ_MINIMUM_PRESSURE    5
#define MOD_SOM_APF_DACQ_CTD_DELAY           100

#define MOD_SOM_APF_METADATA_STRUCT_SIZE    31
#define MOD_SOM_APF_END_METADATA_STRUCT     0xFFFF


#define MOD_SOM_APF_PRODUCER_TASK_PRIO              18u
#define MOD_SOM_APF_PRODUCER_TASK_STK_SIZE          512u
#define MOD_SOM_APF_PRODUCER_DELAY                  10      // delay for fill segment task
#define MOD_SOM_APF_PRODUCER_MIN_DISSRATE           1e-12   // min epsilon, chi or Fourier coef
#define MOD_SOM_APF_PRODUCER_MAX_DISSRATE           1e-3    // min epsilon, chi or Fourier coef
#define MOD_SOM_APF_PRODUCER_MIN_FOM                0       // mininum figure of merit
#define MOD_SOM_APF_PRODUCER_MAX_FOM                10      // maininum figure of merit
#define MOD_SOM_APF_PRODUCER_DISSRATE_RES           3       // mod dissrate resolution 3 bytes
#define MOD_SOM_APF_PRODUCER_DISSRATE_RANGE         0xFFF   // mod dissrate range 12 bits
#define MOD_SOM_APF_PRODUCER_FOM_RES                1       // mod fom resolution 1 bytes
#define MOD_SOM_APF_PRODUCER_FOM_RANGE              0xF     // mod fomte range 1 bytes
#define MOD_SOM_APF_PRODUCER_MIN_FOCO               1e-13   // min Fourier coef,
#define MOD_SOM_APF_PRODUCER_MAX_FOCO               1e-2       // max Fourier coef
#define MOD_SOM_APF_PRODUCER_FOCO_RANGE             0xFFFF     // mod fourier coef resolution 16 bits
#define MOD_SOM_APF_PRODUCER_FOCO_RES               2       // mod foco resolution 2 bytes


#define MOD_SOM_APF_CONSUMER_TASK_PRIO              18u
#define MOD_SOM_APF_CONSUMER_TASK_STK_SIZE          512u
#define MOD_SOM_APF_CONSUMER_DELAY                  10      // delay for fill segment task
#define MOD_SOM_APF_CONSUMER_TIMESTAMP_SIZE         8
#define MOD_SOM_APF_CONSUMER_CHECKSUM_SIZE          5

#define MOD_SOM_APF_SHELL_TASK_PRIO              20u
#define MOD_SOM_APF_SHELL_TASK_STK_SIZE          1024u
#define MOD_SOM_APF_SHELL_DELAY                  1      // delay for fill segment task
#define MOD_SOM_APF_SHELL_TIMEOUT                30      // delay for fill segment task
//#define MOD_SOM_APF_SHELL_SAMPLE_LASTCHAR        '\r'

#define MOD_SOM_APF_STATUS_OK   0
#define MOD_SOM_APF_STATUS_ERR 0x1u   //ALB cannot -1 because it interfere with default shell ERR
#define MOD_SOM_APF_STATUS_WRONG_ARG                            0x2u
#define MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_SETUP                0x1u
#define MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_CONFIG               0x2u
#define MOD_SOM_APF_STATUS_CANNOT_ALLOCATE_PRODUCER             0x3u
#define MOD_SOM_APF_STATUS_FAIL_TO_START_PRODUCER_TASK          0x4u
#define MOD_SOM_APF_STATUS_FAIL_TO_STOP_PRODUCER_TASK           0x5u
#define MOD_SOM_APF_STATUS_FAIL_TO_START_CONSUMER_TASK          0x6u
#define MOD_SOM_APF_STATUS_FAIL_TO_STOP_CONSUMER_TASK           0x7u
#define MOD_SOM_APF_STATUS_FAIL_WRONG_ARGUMENTS                 0x8u
#define MOD_SOM_APF_STATUS_DAQ_IS_RUNNING                       0x9u
#define MOD_SOM_APF_STATUS_FAIL_TO_ALLOCATE_MEMORY              0x10U
#define MOD_SOM_APF_STATUS_FAIL_SEND_MS                         0x11U
#define MOD_SOM_APF_STATUS_FAIL_SEND_PACKET                     0x12U
#define MOD_SOM_APF_STATUS_DAQ_ALREADY_STARTED                  0x13U
#define MOD_SOM_APF_STATUS_ARG_TOO_HIGH                         0x14U
#define MOD_SOM_APF_STATUS_BUFFER_OVFLW                         0x15U
#define MOD_SOM_APF_STATUS_NO_CTD_DATA                          0x16U
#define MOD_SOM_APF_STATUS_SLEEPING                             0x17U
#define MOD_SOM_APF_STATUS_NO_DATA                              0x18U
#define MOD_SOM_APF_STATUS_CANNOT_OPENFILE                      0x19U


#define MOD_SOM_APF_UPLOAD_DELAY                  500      // 500 ms delay upon reception of the upload cmd
#define MOD_SOM_APF_UPLOAD_PACKET_CRC_SIZE        2
#define MOD_SOM_APF_UPLOAD_PACKET_CNT_SIZE        2
#define MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE       986
#define MOD_SOM_APF_UPLOAD_PACKET_SIZE            990
#define MOD_SOM_APF_UPLOAD_APF11_ACK              0x06
#define MOD_SOM_APF_UPLOAD_APF11_NACK             0x15
#define MOD_SOM_APF_UPLOAD_EOT_BYTE               0x04
#define MOD_SOM_APF_UPLOAD_APF11_TIMEOUT          5           // 5 second timeout
#define MOD_SOM_APF_UPLOAD_MAX_TRY_PACKET         3           // 3 tries to send a packet


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define MOD_SOM_APF_REPLY_MAX_LEN 128

#define MOD_SOM_APF_DAQ_CMMD_LIMIT 65534 // mnbui - April 19, 2022
// testing
//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

typedef uint32_t mod_som_apf_status_t;


/******************************************************************************/
typedef struct{
   uint32_t  initialized_flag;
   uint32_t  header_length;
   uint8_t   vibration_cut_off;
   mod_som_com_port_t port;
   uint32_t baud_rate;

}mod_som_apf_config_t, *mod_som_apf_config_ptr_t;

/*******************************************************************************
* @brief
*   Structure to store set up configuration for MOD SOM EFE
*
******************************************************************************/
typedef struct{
   uint32_t size;
   char header[MOD_SOM_APF_SETTINGS_STR_LENGTH];

   //ALB comm_telemetry_packet_format:
   //ALB 0: time,pr,epsi,chi,nu,kappa,fomepsi,fomchi,
   //ALB 1:  time,pr,epsi,chi,avg_spectra,
   uint32_t comm_telemetry_packet_format;

   //ALB sd_packet_format:
   //0: no SD write
   //1: SD store full time,pr,temp,salinity,epsi,chi,avg_spectra,
   //2: SD store each full dissrate time,pr,temp,salinity,epsi,chi,avg_spectra + stream daq profile
   uint32_t sd_packet_format;

   uint32_t initialize_flag;
}
mod_som_apf_settings_t, *mod_som_apf_settings_ptr_t;


/*******************************************************************************
* @brief
*   APEX probe structure used in the metadata structure .
******************************************************************************/

typedef struct{
  enum {t1,s1}type;
  uint16_t sn;
  uint16_t cal;
}
mod_som_apf_probe_t, *mod_som_apf_probe_ptr_t;


/*******************************************************************************
* @brief
*   MetaData Structure .
*   Meta_Data Bytes Description
0-3    Unix epoch at the start of the Daq session (mostly start profile but not always)
4-5    Profile identifier (i.e., profile number)
6-7    SOM serial number - revision (two bytes for serial and revision)
8-9    EFE serial number - revision   (two bytes for serial and revision)
10-13  Firmware revision. ALB: I would recommend the latest github commit number like 6e92d93  (could be 4 bytes in hex)
14-15  Nfft  (2 bytes) ALB not a param the user can change
16-17  Nfftdiag  (2 bytes) ALB not a param the user can change
18-22  <ProbeType1><ProbeSerNo1><ProbeCalcoef1> (1+2 +2 bytes)
23-27  <ProbeType2><ProbeSerNo2><ProbeCalcoef2>(1+2 +2 bytes)

30      comm_telemetry_packet_format (1 byte)
31      sd_format (1 byte)

32-33   Number of samples.
34-37   Voltage (2bytes)
38-39   0xFFFF
******************************************************************************/

//#pragma pack(1)
typedef struct{

  uint32_t daq_timestamp; //
  uint16_t  profile_id;
  uint16_t modsom_sn;
  uint16_t efe_sn;
  uint32_t firmware_rev;
  uint16_t nfft;
  uint16_t nfftdiag;
  mod_som_apf_probe_t  probe1;
  mod_som_apf_probe_t  probe2;
  uint8_t  comm_telemetry_packet_format;
  uint8_t  sd_format;
  uint16_t sample_cnt;
  uint32_t voltage;
  uint16_t end_metadata; //always 0xFFFF;
}
mod_som_apf_meta_data_t, *mod_som_apf_meta_data_ptr_t;

//#pragma pack(4)

/*******************************************************************************
 * Dacq Stucture.
 *
 */
//typedef struct{
//
//  mod_som_apf_meta_data_t mod_som_apf_meta_data;
//}
//mod_som_apf_dacq_t, *mod_som_apf_dacq_ptr_t;

/*******************************************************************************
 * conversion Structure.
 * define the params need for the MOD dissrate decimation
 */
typedef struct{

  float dissrate_per_bit;            //ALB  (dissrate range) / max(dissrate) - min(dissrate))
  float dissrate_counts_at_origin;   //ALB  nb of counts for dissrate = 1

  float fom_per_bit;                 //ALB  (fom range) / max(fom) - min(fom))
  float fom_counts_at_origin;        //ALB  nb of counts for dissrate = 1

  float foco_per_bit;                 //ALB  (fourier coef range) / max(fourier) - min(fourier))
  float foco_counts_at_origin;        //ALB   nb of counts for dissrate = 1
}
mod_som_apf_decimation_t, *mod_som_apf_decimation_ptr_t;


/*******************************************************************************
* @brief
*   Procuder Structure to  collect the efe obp data
*   Profile Data structure
******************************************************************************/
typedef struct{
  bool  initialized_flag;
  bool  started_flg;        //ALB is the APF producer started
  bool  collect_flg;        //ALB flag to collect the data (Trigger by CTD data?)

  uint32_t nfft_diag;

  uint64_t dissrates_cnt;
  uint64_t stored_dissrates_cnt;

//  uint64_t avg_timestamp;

//  float * avg_spec_temp_ptr;         //ALB pointer to spectrum
//  float * avg_spec_shear_ptr;        //ALB pointer to spectrum
//  float * avg_spec_accel_ptr;        //ALB pointer to spectrum

//  float * avg_ctd_pressure;
//  float * avg_ctd_temperature;
//  float * avg_ctd_salinity;
//  float * avg_ctd_dpdt;

//  float * nu;
//  float * kappa;
//  float * epsilon;
//  float * chi;

//  mod_som_apf_dacq_t acq_profile;
  mod_som_apf_meta_data_t mod_som_apf_meta_data;
  int32_t meta_data_buffer_byte_cnt;

  uint8_t * meta_data_buffer_ptr;

  uint8_t * dacq_ptr;
  bool dacq_full;
  uint32_t dacq_size;
  uint32_t dacq_element_size;
  bool done_sd_flag;
  mod_som_apf_decimation_t decim_coef;

  uint32_t dissrate_skipped;

}
mod_som_apf_producer_t, *mod_som_apf_producer_ptr_t;


/*******************************************************************************
* @brief
*   Upload structure
*   990 Packet with 2 bytes CRC+ 2 bytes counters + 986 payload bytes

*
*   Profile Data structure
******************************************************************************/
typedef struct{
  uint16_t  CRC;
  uint16_t  counters;
  uint8_t  payload[MOD_SOM_APF_UPLOAD_PACKET_LOAD_SIZE];
}
mod_som_apf_upload_packet_t, *mod_som_apf_upload_packet_ptr_t;


/*******************************************************************************
* @brief
*   Consumer structure to store the efe obp data in an SD file.
*
*   Profile Data structure
******************************************************************************/
typedef struct{
  bool  initialized_flag;
  bool  started_flg;        //ALB is the APF consumer started

  uint32_t  dacq_size;
  uint8_t * dacq_ptr;
  uint64_t dissrates_cnt;
  uint64_t record_timestamp;
  uint32_t dissrate_skipped;
//  uint32_t stored_dissrates_cnt;
  uint32_t payload_length;

  bool      consumed_flag;
  uint8_t   send_packet_tries;
  uint32_t  nb_packet_sent;
  int       daq_remaining_bytes;

  uint8_t    header[MOD_SOM_APF_MAX_HEADER_SIZE];
  char       tag[MOD_SOM_APF_TAG_LENGTH];
  uint8_t    header_chksum;
  uint8_t    chksum;
  uint32_t   length_header;



  mod_som_apf_upload_packet_t packet;

}
mod_som_apf_consumer_t, *mod_som_apf_consumer_ptr_t;

/*******************************************************************************
 * @brief
 *     peripheral structure for MOD SOM APF
 * @description
 *     this structure is based on the peripheral structure described in
 *     mod_som_common.h, mod_som_prf_t. The first 3 fields of the structure
 *     has to be the same as that of mod_som_prf_t.
 * @field handle_port
 *     a pointer to UART handle port (same as mod_som_prf_t)
 * @field irq_f
 *     pointer to interrupt callback function
 *     if UART port has two interrupt callback function, this would be the RX
 *     interrupt callback function
 * @field irq_extra_f
 *     pointer to additional interrupt callback function
 *     if UART port has two interrupt callback function, this would be the TX
 *     interrupt callback function
 * @field irqn
 *     interrupt number identification for the interrupt system
 ******************************************************************************/
typedef struct{
    void * handle_port;
    void (* irq_f)();
    void (* irq_extra_f)();
    IRQn_Type irqn;
    enum {retarget,apfleuart} portID;
}mod_som_apf_prf_t,*mod_som_apf_prf_ptr_t;




/*******************************************************************************
* @brief
*   Structure to store set up configuration for MOD SOM EFE
*
*   *   F0 = format 0 no data collection
*   F1 = format 1 data collection time + pr + log10(epsi) + log10(chi) + flag
*        2+4+3+1 bytes  10 bytes.
*        Say we optimistically think we have 1% precision for epsilon ->
*        log10(1.01)~ 0.004 in log10 space.
*        If we then want a range of log10 (epsilon,chi) of {-12,-3},
*        then we require (9/0.004)=2000 numbers or so.
*        This is 11 bits for 1 sample. Let’s put 12 bits for convenience.
*        (12 bits= 4096 counts -> 9./4096 =0.0022).
*        Epsilon+chi -> 24 bits -> 3 bytes.
*
*   F2 = format 2
*        A sample is
*        time + pr + log10(epsi) + log10(chi) + flag + rms a1 + rms a2 + rms a3=
*        2+4+3+1+ 2 +2 +2 = 16 bytes.
*        Noise floor of the accelerometer= 45 x 10^{-12} g^2/Hz.
*        Similarly to epsilon and chi we want to get a range from -12 to -1
*        (in log space) with 2 bytes it gives 11./65535=
*        1.4 x 10^{-4} count -> 0.04% resolution.
*        This is way enough.
*        Thus, rms a1, a2, a3 can also be 2 bytes each.
*
*   F3 = format 3
*        This format sends Fourier coef back to shore at some <dz_check> intervals.
*        We want to be able to adapt the number of Fourier coef to fill up
*        the whole 25kB. The proposed design is the following:
*        The Fourier coef could be 2 bytes (see following numerical application)
*        Epsi should compute the number of samples from Dz_check.
*        This number is bytes 37-38 in the Header.
*        Epsi should compute NFFTdiag the max Fourier coef that can fit
*        inside 25kB-52 bytes header.
*        NFFTdiag could be 12 bits (4095 NFFT) and could be stored
*        as uint16_t in header bytes 39-40.
*
*        A sample is
*        time + pr + (log10(epsi) log10(chi))
*                  + s1 Fourier_Coef
*                  + t1 Fourier coef
*                  + a3 Fourier coef =
*                   2 + 4 + 3 + 2*NFFTdiag + 2*NFFTdiag + 2*NFFTdiag =
*                   9+6*NFFTdiag bytes
*
*                   Small numerical application to check the math/design
*                   of format 3 design.
*                   The ideal wavenumber range to observe is 1-100 cpm.
*                   The float speed is about 10cm/s.
*                   So the ideal frequency range would be 0.1-10Hz.
*                   Let say we want to get a maximum of 10 samples
*                   (time,pr,epsi,chi,spectra).
*                   We want to get the Fourier coef between 0.1 Hz and 10Hz
*                   of the spectra.
*                   10 samples is 10 x 10 bytes~ 100 bytes.
*                   The header is 52 bytes. We have 25kB-152 bytes left for
*                   the Fourrier coef. I round up to 24kB. 24kB/10= 2400 bytes
*                   for the Fourier coef of the 3 spectra (s1, t1 and a3).
*                   2400/3 = 800 bytes for 1 spectrum for 10 samples.
*                   In Volt^2/Hz, in log10 space, all spectra will range between
*                   -14 and -6 (Fig. 1). That is a range of 8 in log10 space.
*                   Let say 2 bytes per Fourier coef 8/65535= 0.03% resolution.
*                   NFFTdiag = 800/2 = 400 Fourier coef
*
*
*
******************************************************************************/
typedef struct{
   uint32_t initialize_flag;
   uint32_t sleep_flag;
   uint32_t upload_flag;
   mod_som_status_t status;


   mod_som_apf_settings_ptr_t settings_ptr;
   mod_som_apf_config_ptr_t   config_ptr;
   mod_som_apf_producer_ptr_t producer_ptr;
   mod_som_apf_consumer_ptr_t consumer_ptr;
   mod_som_apf_prf_ptr_t      com_prf_ptr;

   uint64_t profile_id;
   bool     daq;
   float    dacq_start_pressure;
   float    dacq_pressure;
   float    dacq_dz;

   char apf_reply_str[MOD_SOM_APF_REPLY_MAX_LEN];   // add reply str from APF - Arnaud&Mai Nov 16, 2021

}
mod_som_apf_t, *mod_som_apf_ptr_t;


//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize APF, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely

 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_init_f();

/*******************************************************************************
 * @brief
 *   construct settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_allocate_settings_ptr_f();

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
    mod_som_apf_settings_ptr_t settings_ptr);

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_apf_settings_t mod_som_apf_get_settings_f();
/*******************************************************************************
 * @brief
 *   get the runtime ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_apf_ptr_t mod_som_apf_get_runtime_ptr_f();


bool mod_som_apf_get_daq_f(); // mnbui Nov 29, 2021

/*******************************************************************************
 * @brief
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_config_ptr_f();
/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
void* mod_som_apf_get_port_ptr_f();

/*******************************************************************************
 * @brief
 *   construct producer_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_producer_ptr_f();

/*******************************************************************************
 * @brief
 *   construct consumer_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_consumer_ptr_f();

/*******************************************************************************
 * @brief
 *   Daq function
 *   start Data acquisition.
 *   It should start the EFE adc master clock
 *   start the turbulence processing task
 *   and store the data
 *   it should store and return a Daq Status
 *   (e.g., Daq enable/disable)
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_apf_status_t mod_som_apf_daq_start_f(uint64_t profile_id);

/*******************************************************************************
 * @brief
 *   Daq function
 *   stop Data acquisition.
 *   It should start the EFE adc master clock
 *   start the turbulence processing task
 *   and store the data
 *   it should store and return a Daq Status
 *   (e.g., Daq enable/disable)
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_apf_status_t mod_som_apf_daq_stop_f();


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

//mod_som_apf_status_t mod_som_apf_daq_status_f();


/*******************************************************************************
 * @brief
 *    initialize dacq Meta_Data
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_apf_init_meta_data(mod_som_apf_meta_data_ptr_t mod_som_apf_meta_data);

/*******************************************************************************
 * @brief
 *   command shell for FubarCal command
 *   run FubarCal cmd with arguments arg1, arg2, ..., argn
 *   it should return a Fubar status
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fubar_f();

/*******************************************************************************
 * @brief
 *   command shell for FubarCal? command
 *   display Fubar status
 *   it should return an error if can not access to the information
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fubar_status_f();

/*******************************************************************************
 * @brief
 *    get meta data ptr
 *
 * @return
 *   mod_som_apf_meta_data_ptr_t
 ******************************************************************************/
mod_som_apf_meta_data_ptr_t mod_som_apf_get_meta_data_ptr();

/*******************************************************************************
 * @brief
 *   command shell for FwRev? command
 *   display Firmware Revision ID
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_fwrev_status_f();

/*******************************************************************************
 * @brief
 *   command shell for ok? command
 *   wake up SOM and display apf status
 *   if nothing happens after 30 sec go back to sleep
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_ok_status_f();

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
mod_som_apf_status_t mod_som_apf_poweroff_f();

///*******************************************************************************
// * @brief
// *   command shell for EpsiNo command
// *   set the SOM and EFE SN
// *   should return an apf status.
// * @return
// *   MOD_SOM_APF_STATUS_OK if function execute nicely
// ******************************************************************************/
//mod_som_apf_status_t mod_som_apf_epsi_id_f();

/*******************************************************************************
 * @brief
 *   command shell for EpsiNo? command
 *   get the SOM and EFE SN
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_epsi_id_status_f();

/*******************************************************************************
 * @brief
 *   command shell for ProbeNo command
 *   set the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_probe_id_f(CPU_INT16U argc,
                                            CPU_CHAR *argv[]);
/*******************************************************************************
 * @brief
 *   command shell for ProbeNo? command
 *   get the Shear and FPO7 probe SN and its  calibration coefficient Sv or dT/dV
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_probe_id_status_f();

/*******************************************************************************
 * @brief
 *   command shell for sleep command
 *   put SOM to sleep
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_sleep_f();

/*******************************************************************************
 * @brief
 *   command shell for gate command
 *   gate,on gate,off turn on and off the 232 drive and MOD shell
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_gate_f(CPU_INT16U argc,
                                         CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   command shell for time command
 *   set UnixEpoch time on SOM
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_f(CPU_INT16U argc,
                                        CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   command shell for time? command
 *   get UnixEpoch time on SOM
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_status_f();

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
mod_som_apf_status_t mod_som_apf_packet_format_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[]);
mod_som_apf_status_t mod_som_apf_packet_format_status_f();
/*******************************************************************************
 * @brief
 *   command shell for mod_som_apf_cmd_sd_format_f
 *   set the format of the data stored in the SD card
 *   0 = no format (latter on called SD0)
 *   1 = format 1 (SD1) time pressure epsilon chi fom dpdt kvis avg_t avg_s decimated avg spectra
 *   2 = format 2 (SD2) time pressure epsilon chi fom dpdt kvis avg_t avg_s full avg spectra
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
                                                      CPU_CHAR *argv[]);
mod_som_apf_status_t mod_som_apf_sd_format_status_f(CPU_INT16U argc,
                                                      CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   command shell for upload command
 *   start uploading data from the SD card to the apf
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_upload_f();

/*******************************************************************************
 * @brief
 *   construct apf com_prf
 *   ALB We should be able to choose between RETARGET_SERIAL and any other port
 *   (e.g., LEUART)
 *
 * @param mod_som_apf_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_com_prf_f();

/*******************************************************************************
 * @brief
 *   start apf producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_start_producer_task_f();

/*******************************************************************************
 * @brief
 *   stop apf producer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_stop_producer_task_f();

mod_som_status_t mod_som_apf_start_consumer_task_f();
mod_som_status_t mod_som_apf_stop_consumer_task_f();

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
void mod_som_apf_producer_task_f(void  *p_arg);

/*******************************************************************************
 * @brief
 *   apf consumer task
 *
 *   1- store the dacq profile in a sd file (as the dacq_profile is getting filled up)
 *   2- when asked stream out the data to the apf with the afp format
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_consumer_task_f(void  *p_arg);

/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_apf_header_f(mod_som_apf_consumer_ptr_t consumer_ptr, uint8_t tag_id);


/*******************************************************************************
 * @brief
 *   apf shell task
 *
 *   shell task
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_apf_shell_task_f(void  *p_arg);

/*******************************************************************************
 * @brief
 *   Get text input APEX.
 *
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_get_input_f(char *buf, uint32_t * buf_len);

/*******************************************************************************
 * @brief
 *   Get text input from user.
 *   TODO
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_get_line_f(char *buf, uint32_t * buf_len);

/*******************************************************************************
 * @brief
 *   Execute user's input when a carriage return is pressed.
 *
 * @param input
 *   The string entered at prompt.
 * @param input_len
 *   Length of string input
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_execute_input_f(char* input,uint32_t input_len);
/*******************************************************************************
 * @brief
 *   Get text input from user.
 *   TODO
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_apf_shell_get_line_f(char *buf, uint32_t * buf_len);

/*******************************************************************************
 * @brief
 *   Send a line from a PORT.
 *
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
uint32_t mod_som_apf_send_line_f(LEUART_TypeDef *leuart, char * buf, uint32_t nb_of_char_to_send);
//uint32_t mod_som_apf_make_nack_apf_reply_f(uint8_t * apf_reply_str,char * cmd, uint32_t status);
/*******************************************************************************
 * @brief
 * convert the dissrates into MOD format
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
uint32_t mod_som_apf_copy_F1_element_f( uint64_t * curr_avg_timestamp_ptr,
                                    float * curr_pressure_ptr,
                                    float * curr_epsilon_ptr,
                                    float * curr_chi_ptr,
                                    float * curr_fom_epsi_ptr,
                                    float * curr_fom_chi_ptr);


/*******************************************************************************
 * @brief
 * downgrade avg spectra into MOD format
 *
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
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
                                float * curr_accel_avg_spectra_ptr);

void mod_som_apf_copy_sd_element_f(  uint64_t * curr_avg_timestamp_ptr,
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
                                     float * curr_accel_avg_spectra_ptr);
/*******************************************************************************
 * @function
 *     mod_som_apf_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
 *     lower 16 bits are reserved for flags
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely

 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_apf_encode_status_f(uint8_t mod_som_io_status);

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
 *   MOD_SOM_APF_STATUS_OK if function execute nicely

 *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_apf_decode_status_f(mod_som_status_t mod_som_status);

uint32_t mod_som_apf_convert_string_f(char* input_str, uint32_t * bytes_read, char* output_str);
mod_som_status_t mod_som_apf_get_char_f(LEUART_TypeDef *leuart, int* read_char);

#endif /* MOD_APF_MOD_SOM_APF_H_ */
