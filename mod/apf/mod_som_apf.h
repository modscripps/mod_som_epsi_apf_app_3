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

#define MOD_SOM_APF_STATUS_OK   0
#define MOD_SOM_APF_STATUS_ERR -2   //ALB cannot -1 because it interfere with default shell ERR
#define MOD_SOM_APF_OBP_CANNOT_ALLOCATE_SETUP 1
#define MOD_SOM_APF_CANNOT_OPEN_CONFIG 2

#define MOD_SOM_APF_SETTINGS_STR_lENGTH 8
#define MOD_SOM_APF_HEADER              "APF0"

#define MOD_SOM_APF_SYNC_LENGTH             1
#define MOD_SOM_APF_TAG_LENGTH              4
#define MOD_SOM_APF_HEXTIMESTAMP_LENGTH     16
#define MOD_SOM_APF_PAYLOAD_LENGTH          8
#define MOD_SOM_APF_HEADER_CHECKSUM_LENGTH  3
#define MOD_SOM_APF_PAYLOAD_LENGTH          8
#define MOD_SOM_APF_PAYLOAD_CHECKSUM_LENGTH 5

#define MOD_SOM_APF_DACQ_STRUCT_SIZE        25000



//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

typedef uint32_t mod_som_apf_status_t;


/******************************************************************************/
typedef struct{
   uint32_t  initialized_flag;
   uint32_t  header_length;

}mod_som_apf_config_t, *mod_som_apf_config_ptr_t;

/*******************************************************************************
* @brief
*   Structure to store set up configuration for MOD SOM EFE
*
******************************************************************************/
typedef struct{
   uint32_t size;
   char header[MOD_SOM_APF_SETTINGS_STR_lENGTH];
   uint32_t initialize_flag;
}
mod_som_apf_settings_t, *mod_som_apf_settings_ptr_t;


/*******************************************************************************
* @brief
*   APEX probe structure used in the metadata structure .
******************************************************************************/

typedef struct{
  enum {t1,s1}type;
  uint16_t sn[2];
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
10-14  Firmware revision. ALB: I would recommend the latest github commit number like 6e92d93  (could be 4 bytes in hex)
15-16  Nfft  (2 bytes) ALB not a param the user can change
17-21  <ProbeType1><ProbeSerNo1><ProbeCalcoef1> (1+2 +2 bytes)
22-26  <ProbeType2><ProbeSerNo2><ProbeCalcoef2>(1+2 +2 bytes)

27      comm_telemetry_packet_format (1 byte)
28      Voltage 1 byte
29      4 bits Epsilon - 4 bits chi algorithm version
30      Vibration cut off frequency (1 byte)
31-32   Number of samples.
33-34   NFFTdiag (should be 0 when comm_telemetry_packet_format is *not* 3 )
35-36   0xFFFF
******************************************************************************/

typedef struct{

  uint32_t daq_timestamp; //
  uint8_t  profile_id;
  uint16_t modsom_sn;
  uint16_t efe_sn;
  uint32_t firmware_rev;
  uint16_t nfft;
  mod_som_apf_probe_t  probe1;
  mod_som_apf_probe_t  probe2;
  uint8_t  comm_telemetry_packet_format;
  uint8_t  voltage;
  uint8_t  algorithm_version;
  uint8_t  vibration_cutoff;
  uint16_t sample_cnt;
  uint16_t NFFTdiag;
  uint16_t end_metadata; //always 0xFFFF;
}
mod_som_apf_meta_data_t, *mod_som_apf_meta_data_ptr_t;


/*******************************************************************************
 * Dacq Stucture.
 *
 */
typedef struct{

  mod_som_apf_meta_data_t mod_som_apf_meta_data;
  uint8_t data_acq[MOD_SOM_APF_DACQ_STRUCT_SIZE];
}
mod_som_apf_dacq_t, *mod_som_apf_dacq_ptr_t;

/*******************************************************************************
* @brief
*   Procuder Structure to store collect the efe obp data
*   and store them in an SD file.
*
*   Profile Data structure


*
*
*
*
******************************************************************************/
typedef struct{
  bool  started_flg;        //ALB is the APF producer started
  bool  collect_flg;        //ALB flag to collect the data (Trigger by CTD data?)

  uint64_t dissrates_cnt;
  uint64_t avg_timestamp;

  float * avg_spec_temp_ptr;         //ALB pointer to spectrum
  float * avg_spec_shear_ptr;        //ALB pointer to spectrum
  float * avg_spec_accel_ptr;        //ALB pointer to spectrum

  float * avg_ctd_pressure;
  float * avg_ctd_temperature;
  float * avg_ctd_salinity;
  float * avg_ctd_fallrate;

  float * nu;
  float * kappa;
  float * epsilon;
  float * chi;

  mod_som_apf_dacq_t acq_profile;

   uint32_t dissrate_skipped;

}
mod_som_apf_producer_t, *mod_som_apf_producer_ptr_t;


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
   mod_som_apf_settings_ptr_t settings_ptr;
   mod_som_apf_config_ptr_t   config_ptr;
   mod_som_apf_producer_ptr_t producer_ptr;

   uint64_t profile_id;
   bool     daq;

   enum {F0,F1,F2,F3}comm_telemetry_packet_format; //L0 p,t

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
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_apf_construct_config_ptr_f();

/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely

 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_say_hello_world_f();

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

mod_som_apf_status_t mod_som_apf_daq_status_f();

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

/*******************************************************************************
 * @brief
 *   command shell for EpsiNo command
 *   set the SOM and EFE SN
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_epsi_id_f();

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
mod_som_apf_status_t mod_som_apf_probe_id_f();

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
 *   command shell for time command
 *   set UnixEpoch time on SOM
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_time_f();

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
 *   command shell for upload command
 *   start uploading data from the SD card to the apf
 *   should return an apf status.
 * @return
 *   MOD_SOM_APF_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_apf_status_t mod_som_apf_upload_f();

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


#endif /* MOD_APF_MOD_SOM_APF_H_ */
