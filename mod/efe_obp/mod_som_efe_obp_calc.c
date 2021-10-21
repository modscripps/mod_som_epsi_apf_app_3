// /*******************************************************************************
//  * @brief
//  *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
//  ******************************************************************************/
// typedef struct{
//   float    * shear_sv;
//   float    * fp07_dTdV;
//   uint16_t   Tdiff_size;
//   float    * Tdiff_freq;
//   float    * Tdiff_coeff;
//   uint16_t   cafilter_size;
//   float    * cafilter_freq;
//   float    * cafilter_coeff;
//   float      fp07_noise[4];
//
// }
// mod_som_efe_obp_calibration_t, *mod_som_efe_obp_calibration_ptr_t;

/****************************************************************************
 Module
    module_name.c
 Description
    description of module
 Notes
    additional notes

 History
 When             Who    What/Why
 --------------   ---    --------
 DD MMMM YYYY     XXX    changes
*****************************************************************************/

/*----------------------------- Include Files ------------------------------*/
#include "mod_som_efe_obp_calc.h"
/*--------------------------- External Variables ---------------------------*/
/*----------------------------- Module Defines -----------------------------*/
/*------------------------------ Module Types ------------------------------*/
/*---------------------------- Module Variables ----------------------------*/

// Config struct
static mod_som_efe_obp_config_ptr_t config;
// will contain following variables
// static float f_CTD_pump;
// static float f_samp;
// static uint8_t num_shear; // number of shear probes on instrument
// static uint8_t num_fp07; // number of fp07 sensors on instrument

// settings struct
static mod_som_efe_obp_settings_ptr_t settings;
// will contain following variables
// static uint32_t fft_size;
// static uint32_t dof;

// calibration struct
static mod_som_efe_obp_calibration_ptr_t cals;
// will contain following variables
// static float *sv; // Sensitivity; this should be put into a structure with other needed constants and params.
// static float *cafilter_freq; // empirical charge filter frequency vector
// static float *cafilter_coeff; // empirical charge filter coefficient vector
// static uint16_t cafilter_size; // length of charge filter vector
// static float n0, n1, n2, n3;
// static float *dTdV; // fp07 calibration value
// static float *Tdiff_coeff; // empirical Tdiff filter frequency vector
// static float *Tdiff_freq; // empirical Tdiff filter coefficient vector
// static uint16_t Tdiff_size; // length of Tdiff filter vector

// producer struct
// will be passed in to appropriate functions
// will contain following variables
// outputs

// CTD struct
//static mod_som_epsiobp_scan_CTD_ptr_t ctd;
// will contain salinity, temp, pressure, fall rate

// calculated values struct
static mod_som_efe_obp_calc_vals_ptr_t vals;
// will contain freq, kvec, hamming_window, normalization, fp07_noise, epsi_spectrum_buffer, chi_spectrum_buffer, epsi_averaged_spectrum, chi_averaged_spectrum

// spectrum buffer for calculation storage and operation
float *spectrum_buffer;

// UNIVERSAL CONSTANTS
static const float g = 9.807; // m/s^2
// CALIBRATION INPUTS

// USER INPUTS

// INPUT DATA
//static float *shear_data; // This is a placeholder for the buffer where the shear data are stored.
//static float *fp07_data;

//static uint16_t num_samples; // number of data points from a single sensor being passed in
//static uint8_t num_blocks; // number of non-overlapping windowed blocks averaged in spectra-calculating functions
//static uint16_t scan_length; // number of data points being used to calculate spectra/values

static uint8_t spectrum_counter; // counter for number of spectra up to dof
static uint16_t master_counter; //counter of how many values we have processed.

/*---------------------------- Module Functions ----------------------------*/
// OBP CALCULATIONS
static void mod_som_efe_obp_shear_spectra_f(float *shear_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
static void mod_som_efe_obp_temp_spectra_f(float *temp_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
static void mod_som_efe_obp_accel_spectra_f(float *accel_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
// FILTERS
static void mod_som_epsiobp_shear_filters_f(float *shear_filter, float fall_rate);
static float oakey_filter_f(float f, float w);
static void mod_som_epsiobp_fp07_filters_f(float *fp07_filter, float fall_rate);
static void mod_som_epsiobp_fp07_noise_f();
uint16_t mod_som_epsiobp_fp07_cutoff_f(float *fp07_spectrum, uint16_t size);
// MATH AND VECTOR FUNCTIONS
//static void mod_som_epsiobp_pwelch_f(float *data, float *spectrum, uint16_t size, uint16_t sampling_frequency);
static void power_spectrum_f(float *data, float *spectrum, uint32_t size, float sampling_frequency);
//static void average_spectra_f(uint16_t size, uint8_t num_blocks, float *spectra_in, float *spectrum_out);
static void hamming_window_f(uint16_t size, float sampling_frequency, float *hamming_window, float normalization);
static float sinc_f(float x);
static void interp1_f(float *x, float *v, uint16_t size, float *xq, float *vq, uint16_t sizeq);
static void smooth_movingmean_f(float *data, float *smoothed, uint16_t size, uint16_t window);
static void detrend_f(float *data, uint16_t size, float *detrended_data);
static void find_vector_indices_f(float *vector, uint16_t size, float *limits, uint16_t *indices);
// SEAWATER PROPERTIES
static float seawater_specific_heat_f(float salinity, float temperature);
static float seawater_density_f(float salinity, float temperature, float pressure);
static float seawater_thermal_conductivity_f(float salinity, float temperature, float pressure);
static float seawater_thermal_diffusivity_f(float salinity, float temperature, float pressure);
static float seawater_kinematic_viscosity_f(float salinity, float temperature, float pressure);
static float panchev_f(float epsilon, float kvis, uint16_t kvec_size, float *panchev_spec);
static float batchelor_f(float epsilon, float chi, float kvis, float kappa, uint16_t cutoff, float *batchelor_spec);
// end

/*------------------------------ Module Code ----------------------------a---*/
// public functions

void mod_som_epsiobp_init_f(mod_som_efe_obp_config_ptr_t config_ptr_in, mod_som_efe_obp_settings_ptr_t settings_ptr_in, mod_som_efe_obp_calibration_ptr_t cals_ptr_in)
/*******************************************************************************
 * @brief
 *   function to initialize variables with correct user inputs, calibration values, etc.
 * @param config
 *   input - see header file
 * @param settings
 *   input - see header file
 * @param calibrations
 *   input - see header file
 ******************************************************************************/
{
  config = config_ptr_in;
  settings = settings_ptr_in;
  cals = cals_ptr_in;
//  // read in calibration inputs
//  num_shear = config.num_shear;
//  num_fp07 = config.num_fp07;
//  f_samp = config.f_samp;
//  f_CTD_pump = config.f_CTD_pump;
//  n0 = calibrations.fp07_noise[0];
//  n1 = calibrations.fp07_noise[1];
//  n2 = calibrations.fp07_noise[2];
//  n3 = calibrations.fp07_noise[3];
//  Tdiff_size = calibrations.Tdiff_size;
//  cafilter_size = calibrations.cafilter_size;
//  // read in user inputs and calculate corresponding values
//  fft_size = settings.nfft;
//  num_samples = (uint16_t) (scan_duration*f_samp);
//  for (num_blocks = user_inputs.max_num_blocks; num_blocks > 0; num_blocks--) {
//    if ((uint16_t) (num_blocks*fft_size) < num_samples) {break;}
//  }
//  scan_length = num_blocks*fft_size;
//  dof = settings.degrees_of_freedom;
  // allocate memory based on inputs
//  cals->shear_sv = (float) malloc(config->num_shear*sizeof(float));
//  cals->dTdV = (float*) malloc(num_fp07*sizeof(float));
//  shear_data = (float*) malloc(fft_size*sizeof(float)); // shear probe data vector
//  fp07_data = (float*) malloc(fft_size*sizeof(float)); // fp07 data vector
  vals->freq = (float*) calloc(settings->nfft/2, sizeof(float)); // freq vector, zero freq NOT included
  vals->kvec = (float*) calloc(settings->nfft/2, sizeof(float)); // wavenumber vector
  vals->fp07_noise = (float*) calloc(settings->nfft/2, sizeof(float)); // fp07 noise vector
  vals->hamming_window = (float*) calloc(settings->nfft, sizeof(float)); // hamming window weights vector
  vals->epsi_spectrum_buffer = (float*) calloc(settings->nfft/2, sizeof(float)); // buffer for spectra
  vals->chi_spectrum_buffer = (float*) calloc(settings->nfft/2, sizeof(float));
  vals->epsi_averaged_spectrum = (float*) calloc(settings->nfft/2, sizeof(float)); // buffer for averaged spectrum
  vals->chi_averaged_spectrum = (float*) calloc(settings->nfft, sizeof(float));
  spectrum_buffer = (float*) calloc(settings->nfft/2, sizeof(float));
  // calculate the frequency vector
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    vals->freq[i] = (float) (i + 1)/(settings->nfft)*config->f_samp;
  }
  // calculate hamming window and normalization
  hamming_window_f(settings->nfft, config->f_samp, vals->hamming_window, vals->normalization);
  // initialize spectrum_counter
  spectrum_counter = 0;
  //initialize master counter
  master_counter = 0;
}

void mod_som_efe_obp_all_spectra_f(float *temp_ptr, float *shear_ptr, float *accel_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units across all
 *   shear, temperature, and acceleration channels,
 *   and output in given structure
 * @param temp_ptr
 *   input - the ptr to temperature data from which to calculate spectra
 * @param shear_ptr
 *   input - the ptr to shear data from which to calculate spectra
 * @param accel_ptr
 *   input - the ptr to acceleration data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
{
  // call appropriate functions
  mod_som_efe_obp_shear_spectra_f(shear_ptr, spectra_offset, mod_som_efe_obp_ptr);
  mod_som_efe_obp_temp_spectra_f(temp_ptr, spectra_offset, mod_som_efe_obp_ptr);
  mod_som_efe_obp_accel_spectra_f(accel_ptr, spectra_offset, mod_som_efe_obp_ptr);
}

void mod_som_efe_obp_shear_spectra_f(float *shear_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units vs frequency
 *   for a single shear channel and stuff into an output location
 * @param shear_ptr
 *   input - the ptr to shear data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
{
  // SHEAR
  // pull in fall rate
  float w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure;
  // Get the electronics transfer functions.
  float shear_filter[settings->nfft/2];
  mod_som_epsiobp_shear_filters_f(shear_filter, w);

  // removed looping over all shear channels, now just doing 1
  // calculate spectrum from shear data
  power_spectrum_f(shear_ptr, spectrum_buffer, settings->nfft, config->f_samp);
  // convert spectra from V^2/Hz to (m/s)^2/Hz
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
      spectrum_buffer[i] = spectrum_buffer[i]*pow(2*g/(cals->shear_sv*w), 2);
      // run spectrum through filter
      spectrum_buffer[i] = spectrum_buffer[i]/shear_filter[i];
      // stuff spectrum into output
      *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr+spectra_offset) = spectrum_buffer[i];
  }

// DEPRECATED CODE
//  // loop over all shear channels
//  for (uint8_t j = 0; j < config->num_shear; j++) {
//      // calculate spectrum from shear data for channel j
//      power_spectrum_f(*(&(shear_ptr) + j*settings->nfft/2), spectrum_buffer, settings->nfft, config->f_samp);
//      // convert spectra from V^2/Hz to (m/s)^2/Hz
//      for (uint16_t i = 0; i < settings->nfft/2; i++) {
//        spectrum_buffer[i] = spectrum_buffer[i]*pow(2*g/(cals->shear_sv[j]*w), 2);
////        // Make the k vector from the freq vector with the appropriate fall speed
////        vals->kvec[i] = vals->freq[i]/w;
//        // run spectrum through filter and convert to wavenumber space
//        // run through filter
//        spectrum_buffer[i] = spectrum_buffer[i]/shear_filter[i];
////        // change to wavenumber space
////        spectrum_buffer[i] = spectrum_buffer[i]*w*pow((2*M_PI*vals->kvec[i]), 2.0);
///
//      }
//      // stuff spectrum into output
//
//      // NOTE: this addressing mechanism should work, but there are some questions around it
//      for (uint16_t i = 0; i < settings->nfft/2; i++) {
//          if (producer->dof == 0) {
//              *(&(producer->s1_spectrum_ptr[i]) + j*settings->nfft/2) = spectrum_buffer[i];
//          } else {
//              *(&(producer->s1_spectrum_ptr[i]) + j*settings->nfft/2) = *(&(producer->s1_spectrum_ptr[i]) + j*settings->nfft/2)*producer->dof/(producer->dof + 1) + spectrum_buffer[i]/producer->dof;
//          }
//      // spec is now the shear spectrum, corrected for the antialias and probe response filters,
//      //in units of (s^-2)/Hz.
//   }
// }
}

void mod_som_efe_obp_temp_spectra_f(float *temp_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units vs frequency
 *   for a single temperature channel and stuff into an output location
 * @param shear_ptr
 *   input - the ptr to temp data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
{
  // SHEAR
  // pull in fall rate
  float w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure;
  //  pull in fp07 filter function and calculate using fall speed as input
  float fp07_filter[settings->nfft/2];
  mod_som_epsiobp_fp07_filters_f(fp07_filter, w);
  // removed looping over all temp channels, now just doing 1
  //init cutoff to end so no funny business
  *(vals->fp07_cutoff) = settings->nfft/2;
  // calculate spectrum from temp data
  power_spectrum_f(temp_ptr, spectrum_buffer, settings->nfft, config->f_samp);
  // convert spectra from V^2/Hz to (degC)^2/Hz
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    spectrum_buffer[i] = spectrum_buffer[i]*pow(cals->fp07_dTdV, 2);
  }
  // find cutoff before filtering
  *(vals->fp07_cutoff) = mod_som_epsiobp_fp07_cutoff_f(spectrum_buffer, settings->nfft/2);
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // correct spectrum using filter, divide by filter to get temp spectrum vs. freq
    spectrum_buffer[i] = spectrum_buffer[i]/fp07_filter[i];
    // stuff spectrum into output
   *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr+spectra_offset) = spectrum_buffer[i];
  }

// DEPRECATED CODE
//  //  for (uint16_t i = 0; i < settings->nfft/2; i++) {
//  //    //Make the k vector from the freq vector with the appropriate fall speed
//  //    vals->kvec[i] = vals->freq[i]/w;
//  //  }
//    //  pull in fp07 filter function and calculate using fall speed as input
//    float fp07_filter[settings->nfft/2];
//    mod_som_epsiobp_fp07_filters_f(fp07_filter, w);
//    // loop over all temp channels
//    for (uint8_t j = 0; j < config->num_fp07; j++) {
//        // init cutoff to end so no funny business
//        vals->fp07_cutoff[j] = settings->nfft/2;
//        // calculate spectrum from temperature data for channel j
//        power_spectrum_f(*(&(producer->t1_volt_ptr) + j*settings->nfft/2), spectrum_buffer, settings->nfft, config->f_samp);
//        // convert spectra from V^2/Hz to (degC)^2/Hz
//        for (uint16_t i = 0; i < settings->nfft/2; i++) {
//          spectrum_buffer[i] = spectrum_buffer[i]*pow(cals->fp07_dTdV[j], 2);
//        }
//        // find cutoff before filtering
//        vals->fp07_cutoff[j] = mod_som_epsiobp_fp07_cutoff_f(spectrum_buffer, settings->nfft/2);
//        // correct spectrum using filter and convert to wavenumber space
//        for (uint16_t i = 0; i < settings->nfft/2; i++) {
//          //  divide by filter to get temp spectrum vs freq
//          spectrum_buffer[i] = spectrum_buffer[i]/fp07_filter[i];
//  //        //  multiply by fall rate and (2*pi*k)^2 to get temp gradient spectrum vs wavenumber
//  //        spectrum_buffer[i] = spectrum_buffer[i]*w*pow((2*M_PI*vals->kvec[i]), 2);
//        }
//        // stuff spectrum into output with correction for current dof
//        for (uint16_t i = 0; i < settings->nfft/2; i++) {
//            if (producer->dof == 1) {
//                *(&(producer->t1_volt_ptr[i]) + j*settings->nfft/2) = spectrum_buffer[i];
//            } else {
//                *(&(producer->t1_volt_ptr[i]) + j*settings->nfft/2) = *(&(producer->t1_volt_ptr[i]) + j*settings->nfft/2)*(producer->dof)/(producer->dof + 1) + spectrum_buffer[i]/producer->dof;
//            }
//        }
//    }
}

void mod_som_efe_obp_accel_spectra_f(float *accel_ptr, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units vs frequency
 *   for a single acceleration channel and stuff into an output location
 * @param shear_ptr
 *   input - the ptr to temp data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
{
  // ACCEL
  // changed to accommodate only a single acceleration channel
  // calculate spectrum from acceleration data
  power_spectrum_f(accel_ptr, spectrum_buffer, settings->nfft, config->f_samp);
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // convert spectra from V^2/Hz to (g)^2/Hz, recalling g is ~9.81 m/s^2
    // NOTE: this should be double checked (linear offsets in space should not affect psd)
    spectrum_buffer[i] = 4*spectrum_buffer[i];
    // stuff spectrum into output
    *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr+spectra_offset) = spectrum_buffer[i];
  }



// DEPRECATED CODE
//
//  // loop over all acceleration channels
//  uint8_t axes = 3;
//  for (uint8_t j = 0; j < axes; j++) {
//      // calculate spectrum from acceleration data for channel j
//      power_spectrum_f(*(&(producer->a1_volt_ptr) + j*settings->nfft/2), spectrum_buffer, settings->nfft, config->f_samp);
//      // convert spectra from V^2/Hz to (g)^2/Hz, recall g is ~9.81 m/s^2
//      // NOTE: this should be double checked (linear offsets in space should not affect psd)
//      for (uint16_t i = 0; i < settings->nfft/2; i++) {
//        spectrum_buffer[i] = 4*spectrum_buffer[i];
//      }
//      // stuff spectrum into output with correction for current dof
//      for (uint16_t i = 0; i < settings->nfft/2; i++) {
//          if (producer->dof == 1) {
//              *(&(producer->a1_volt_ptr[i]) + j*settings->nfft/2) = spectrum_buffer[i];
//          } else {
//              *(&(producer->a1_volt_ptr[i]) + j*settings->nfft/2) = *(&(producer->a1_volt_ptr[i]) + j*settings->nfft/2)*(producer->dof)/(producer->dof + 1) + spectrum_buffer[i]/producer->dof;
//          }
//      }
//  }
}


void mod_som_efe_obp_calc_epsilon_f(float *local_epsilon, float *nu, float *fom, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate a value for dissipation (i.e. epsilon) in W/kg for
 *   each available shear channel and stuff them into the appropriate area in
 *   the producer struct
 * @param producer
 *   input - the producer struct containing all data and spectra
 * @param scan_CTD
 *   input - the struct containing the appropriate CTD data
 ******************************************************************************/
{


  // pull in CTD values
  static float w, P, T, S;
  w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt;
  P = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure;
  T = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature;
  S = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity;

  // calculate wavenumber vector
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    //Make the k vector from the freq vector with the appropriate fall speed
    vals->kvec[i] = vals->freq[i]/w;
  }

  //lookup tables for epsilon from the spectral integrals (shear10) from 2-10 cpm, these come from eps1_mmp/Mike Gregg's work and are (apparently) empirically determined
  static const float eps_fit_shear10[5] = {8.6819e-04, -3.4473e-03, -1.3373e-03, 1.5248, -3.1607};
  static const float shtotal_fit_shear10[5] = {6.9006e-04, -4.2461e-03, -7.0832e-04, 1.5275, 1.8564};


  float kvis = seawater_kinematic_viscosity_f(S, T, P);
  *nu = kvis;
  static const float kvec_min_1 = 2, kvec_max_1 = 10;
  float k_max = config->f_CTD_pump/w;
  float kvec_limits_1[] = {kvec_min_1, kvec_max_1};


  // start loop here
  // removed multiple channel handling for the time being
//  for (uint8_t j = 0; config->num_shear; j++) {
  //We can now begin the iterative procedure of calculating epsilon, following eps1_mmp.m
  //STAGE 1
  //first find the min/max wavenumbers
  static uint16_t kvec_indices_1[2];
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_1, kvec_indices_1);
  uint16_t kvec_1_size = kvec_indices_1[1] - kvec_indices_1[0] + 1;
  float kvec_1[kvec_1_size], spectrum_kvec_1[kvec_1_size];
  // calculate wavenumber vector and convert spectrum from freq to wavenumber space
  for (uint16_t i = 0; i < kvec_1_size; i++) {
    kvec_1[i] = vals->kvec[kvec_indices_1[0] + i];
    spectrum_kvec_1[i] = (mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr[kvec_indices_1[0] + i]);
    // multiply by fall rate and (2*pi*k)^2 to get spectrum vs wavenumber
    spectrum_kvec_1[i] = spectrum_kvec_1[i]*w*pow((2*M_PI*vals->kvec[i]), 2.0);
  }
  static const float dk_interp = 0.2;
  uint16_t kvec_interp_size = (uint16_t) ((kvec_1[kvec_1_size - 1] - kvec_1[0])/dk_interp);
  float spectrum_interp[kvec_interp_size];
  float kvec_interp[kvec_interp_size];
  float kvec_interp_start;
  for (uint16_t i = 0; i < 41; i++) { // i know this is a magic number... might handle it at some point
    kvec_interp_start = 2 + i*dk_interp;
    if (kvec_interp_start > kvec_1[0]) {break;}
  }
  for (uint16_t i = 0; i < kvec_interp_size; i++) {
    kvec_interp[i] = kvec_interp_start + i*dk_interp;
  }
  interp1_f(kvec_1, spectrum_kvec_1, kvec_1_size, kvec_interp, spectrum_interp, kvec_interp_size);
  static float spectrum_interp_integral = 0;
  //Compute the integral of the spectrum from 2-10 cpm
  for (uint16_t i = 0; i < kvec_interp_size; i++) {
    spectrum_interp_integral += spectrum_interp[i]*dk_interp;
  }
  float log10_spectrum_integral = log10(spectrum_interp_integral), threshold = -3.0;
  static float eps_1, log10_eps;
  if (log10_spectrum_integral > threshold) {
    log10_eps = eps_fit_shear10[0]*pow(log10_spectrum_integral, 4.0) + eps_fit_shear10[1]*pow(log10_spectrum_integral, 3.0) + eps_fit_shear10[2]*pow(log10_spectrum_integral, 2.0) + eps_fit_shear10[3]*log10_spectrum_integral + eps_fit_shear10[4];
    eps_1 = pow(10.0, log10_eps);
  } else {
    log10_eps = shtotal_fit_shear10[0]*pow(log10_spectrum_integral, 4.0) + shtotal_fit_shear10[1]*pow(log10_spectrum_integral, 3.0) + shtotal_fit_shear10[2]*pow(log10_spectrum_integral, 2.0) + shtotal_fit_shear10[3]*log10_spectrum_integral + shtotal_fit_shear10[4];
    eps_1 = 7.5*kvis*pow(10.0, log10_eps);
  };
  //STAGE 2 - Second estimate
  float k_cutoff = 0.0816*pow(eps_1, 0.25)/pow(kvis, 0.75);  // k for 90% variance of Panchev spectrum
  if (k_cutoff > k_max) {k_cutoff = k_max;} // limit set by noise spectrum (or in this case the pump freq).  May need to fix that.
  float kvec_limits_2[] = {kvec_min_1, k_cutoff};
  static uint16_t kvec_indices_2[2];
  //find this k range, 2 cpm out to k_cutoff
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_2, kvec_indices_2);
  uint16_t kvec_2_size = kvec_indices_2[1] - kvec_indices_2[0] + 1;
  float spectrum_kvec_2[kvec_2_size];
  float eps_2;
  float dk = vals->kvec[1] - vals->kvec[0];
  //Compute the integral of the spectrum over this range
  static float spectrum_integral = 0;
  for (uint16_t i = 0; i < kvec_2_size; i++) {
    spectrum_kvec_2[i] = (mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr[kvec_indices_2[0] + i]);
    spectrum_integral += spectrum_kvec_2[i]*dk;
  }
  //Compute eps from this and divide by 0.9 to account for the excess over 90%
  eps_2 = 7.5*kvis*spectrum_integral/0.9;
  // THIRD STAGE
  k_cutoff = 0.0816*pow(eps_2, 0.25)/pow(kvis, 0.75);
  if (k_cutoff > k_max) {k_cutoff = k_max;} // limit set by noise spectrum (or in this case the pump freq).  May need to fix that.
//  % third estimate
  //find this k range, 2 cpm out to k_cutoff
  float kvec_limits_3[] = {kvec_min_1, k_cutoff};
  static uint16_t kvec_indices_3[2];
  //find this k range, 2 cpm out to k_cutoff
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_3, kvec_indices_3);
  uint16_t kvec_3_size = kvec_indices_3[1] - kvec_indices_3[0] + 1;
  float spectrum_kvec_3[kvec_3_size];
  static float eps_3;
  //Compute the integral of the spectrum
  spectrum_integral = 0;
  for (uint16_t i = 0; i < kvec_3_size; i++) {
    spectrum_kvec_3[i] = (mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr[kvec_indices_3[0] + i]);
    spectrum_integral += spectrum_kvec_3[i]*dk;
  } //Compute eps from this and divide by 0.9 to account for the excess over 90%
  eps_3 = 7.5*kvis*spectrum_integral;
  if (eps_3 < 1e-10 || kvec_3_size < 4) {
    eps_3 = 1e-10;
  } else {
    static float correction;
    static const float c[] = {-3.12067617e-05, -1.31492870e-03, -2.29864661e-02, -2.18323426e-01, -1.23597906, -4.29137352, -8.91987933, -9.58856889, -2.41486526};
    float log10_eps_3 = log10(eps_3);
    if (log10_eps_3 <= -6) {
      correction = 0;
    } else if (log10_eps_3 <= -1) {
      correction = c[0]*pow(log10_eps_3, 8.0) + c[1]*pow(log10_eps_3, 7.0) + c[2]*pow(log10_eps_3, 6.0) + c[3]*pow(log10_eps_3, 5.0) + c[4]*pow(log10_eps_3, 4.0) + c[5]*pow(log10_eps_3, 3.0) + c[6]*pow(log10_eps_3, 2.0) + c[7]*log10_eps_3 + c[8];
      if (log10_eps_3 < -2) {correction = correction + 0.05*(correction + 6.0)*(1.3e-6 - kvis)/(0.3e-6);}
    } else {
      correction = 1.5058;
    }
    eps_3 = eps_3*pow(10, correction);
  }
  *local_epsilon = eps_3;
  // calculate figure of merit
  float panchev_spec[settings->nfft/2];
  float panchev_integral;
  panchev_integral = panchev_f(eps_3, kvis, kvec_3_size, panchev_spec);
  *fom = spectrum_integral/panchev_integral;
//    }
}

void mod_som_efe_obp_calc_chi_f(float *local_epsilon, float *local_chi, float *kappa, float *fom, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
/*******************************************************************************
 * @brief
 *   function to calculate a value for dissipation of thermal variance in K^2/s (chi)
 * @param fp07_channel
 *   input - the number of the fp07 sensor from which the input data was recorded (0-indexed), used for pulling correct calibration
 * @param *fp07_data
 *   inut - pointer to vector of sequentially stored fp07 spectra
 * @param scan_CTD
 *   input - structure with CTD values from the scan with which the input data is associated (i.e. average pressure, temperature, salinity, and fall rate)
 * @return
 *   thermal variance dissipation (i.e. chi) in K^2/s
 ******************************************************************************/
{
    // pull in CTD values
    static float w, P, T, S;
    w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt;
    P = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_pressure;
    T = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_temperature;
    S = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_salinity;

    // calculate wavenumber vector
    for (uint16_t i = 0; i < settings->nfft/2; i++) {
      //Make the k vector from the freq vector with the appropriate fall speed
      vals->kvec[i] = vals->freq[i]/w;
    }
    // start loop here
    // changed for one channel only (for right now)
//    for (uint8_t j = 0; config->num_shear; j++) {
    //  redefine spectrum and wavenumbers only up to cutoff
    float k_fp07[*(vals->fp07_cutoff)];
    float chi_spectrum[*(vals->fp07_cutoff)];
    float chi_sum = 0;
    for (uint16_t i = 0; i < *(vals->fp07_cutoff); i++) {
      k_fp07[i] = vals->kvec[i];
      chi_spectrum[i] = (mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr[i]);
      // multiply by fall rate and (2*pi*k)^2 to get spectrum vs wavenumber
      chi_spectrum[i] = chi_spectrum[i]*w*pow((2*M_PI*vals->kvec[i]), 2.0);
      chi_sum += chi_spectrum[i];
    }
    //  compute chi
    float kappa_t = seawater_thermal_diffusivity_f(S, T, P);
    float dk = k_fp07[1] - k_fp07[0];
    float chi = 6*kappa_t*dk*chi_sum;
    *local_chi = chi;
    float batchelor_integral;
    float batchelor_spec[settings->nfft/2];
    float kvis = seawater_kinematic_viscosity_f(S, T, P);
    batchelor_integral = batchelor_f(*local_epsilon, chi, kvis, kappa_t, *(vals->fp07_cutoff), batchelor_spec);
    *fom = chi_sum/batchelor_integral;
//  }
}

void mod_som_epsiobp_shear_filters_f(float *shear_filter, float fall_rate)
/*******************************************************************************
 * @brief
 *   function to calculate the filters (physical, electrical, etc.) necessary to correct the shear probe data spectrum
 * @param *shear_filter
 *   output - vector of length settings->nfft/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
{
  // define arrays and variables
  float elect_shear[settings->nfft/2], ca_shear[settings->nfft/2], oakey_shear[settings->nfft/2];
  uint16_t end = settings->nfft/2;
  float denom = 2*vals->freq[end - 1];
  // interpolate to get proper cafilter given freq
  interp1_f(cals->cafilter_freq, cals->cafilter_coeff, cals->cafilter_size, vals->freq, ca_shear, settings->nfft/2);
  // loop to find values for other elements of the filter
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // elect_shear is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
    elect_shear[i] = pow(sinc_f(vals->freq[i]/denom), 4.0);
    // oakey filter comes from airfoil probe response
    oakey_shear[i] = oakey_filter_f(vals->freq[i], fall_rate);
    // calculate single filter array
    shear_filter[i] = pow((ca_shear[i]*elect_shear[i]), 2.0)*oakey_shear[i];
  }
}

float oakey_filter_f(float f, float fall_rate)
/*******************************************************************************
 * @brief
 *   function to calculate Oakey airfoil probe response
 * @param f
 *   input - frequency at which to calculate the value
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 * @return
 *   oakey filter value for f and fall_rate
 ******************************************************************************/
{
  static const float lambda_c = 0.02;
  return 1/(1 + pow((lambda_c*f/fall_rate), 2.0));
}

void mod_som_epsiobp_fp07_filters_f(float *fp07_filter, float fall_rate)
/*******************************************************************************
 * @brief
 *   function to calculate the filters (physical, electrical, etc.) necessary to correct the fp07 data spectrum
 * @param *fp07_filter
 *   output - vector of length settings->nfft/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
{
  // define arrays and variables
  float elect_fp07[settings->nfft/2], magsq[settings->nfft/2], Tdiff[settings->nfft/2];
  uint16_t end = settings->nfft/2;
  float denom = 2*vals->freq[end - 1];
  // interpolate to get proper Tdiff filter given freq
//  interp1_f(cals->Tdiff_freq, cals->Tdiff_coeff, cals->Tdiff_size, vals->freq, Tdiff, settings->nfft/2);
  // loop to find values for other elements of the filter
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // elect_fp07 is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
    elect_fp07[i] = pow(sinc_f(vals->freq[i]/denom), 4.0);
    // magsq filter comes from... where? but it's function is constant and shouldn't ever change
    magsq[i] = 1/(1 + pow((2*M_PI*0.005*pow(fall_rate, -0.32)*vals->freq[i]), 2.0));
    // calculate single filter array
    fp07_filter[i] = pow(elect_fp07[i], 2)*magsq[i]*pow(Tdiff[i], 2);
  }
}

void mod_som_epsiobp_fp07_noise_f()
/*******************************************************************************
 * @brief
 *   function to calculate noise on the raw fp07 spectrum
 ******************************************************************************/
{
  // loop over all frequencies using the noise calibration values
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    vals->fp07_noise[i] = (cals->fp07_noise[0] + cals->fp07_noise[1]*log10(vals->freq[i]) + cals->fp07_noise[2]*pow(log10(vals->freq[i]), 2) + cals->fp07_noise[3]*pow(log10(vals->freq[i]), 3));
  }
}

uint16_t mod_som_epsiobp_fp07_cutoff_f(float *fp07_spectrum, uint16_t size)
/*******************************************************************************
 * @brief
 *   function to calculate the cutoff above which the fp07 spectrum is assumed to be noise
 * @param *fp07_spectrum
 *   input - pointer to raw fp07 spectrum in V^2/Hz
 * @param size
 *   input - should always be settings->nfft/2, this is unnecessary at this point
 * @return
 *   index of the highest frequency at which the fp07 signal is not noise
 ******************************************************************************/
{
  // define variables and arrays
  float signal_noise_ratio = 3;
  uint16_t window = 15; //this is arbitrary, determined in processing before this
  float threshold;
  uint16_t cutoff = size;
  float fp07_smoothed[size];
  mod_som_epsiobp_fp07_noise_f();
  // smooth data
  smooth_movingmean_f(fp07_spectrum, fp07_smoothed, size, window);
  // loop to find cutoff
  for (uint16_t i = 0; i < size; i++) {
    threshold = signal_noise_ratio*pow(10, vals->fp07_noise[i]);
    if (fp07_smoothed[i] < threshold) {
      cutoff = i;
      break;
    }
  }
  return cutoff;
}

void power_spectrum_f(float *data, float *spectrum, uint32_t size, float sampling_frequency)
/*******************************************************************************
 * @brief
 *   function to calculate power spectral density from a single block of data
 * @param *data
 *   input - pointer to input data to be analyzed, must be between 'size' and 65535 points (the input data will not be modified)
 * @param *spectrum
 *   output - pointer to vector of length 'size/2' where spectrum data will be spit out
 * @param size
 *   input - number of points to use in fft (alternatively referred to as nfft)
 * @param sampling_frequecy
 *   input - frequency at which the data under study was sampled
 * @notes
 *   -uses hamming windowing
 *   -all power offset calculated in the fft are tossed as they correspond to zero frequency/wavenumber
 *    (i.e. output spectrum does not contain zero frequency data)
 *   -there is non-trivial loss of precision due to using 32bit types (floats) instead of 64bit types (doubles)
 ******************************************************************************/
{
  bool fft_direction = 0;
  uint16_t j, end;
  end = size/2 - 1;
  float nyquist_power = 0;
//  static float df;
  float detrended_data[size], hamming_window[size], windowed_data[size], fft_data[size]; // see note about hamming window near calculation below
//  df = (float) sampling_frequency / (float) size;
  // this initialization should be taken out and probably put in a separate function so we're not reinitializing on every spectrum; it wastes a huge amount of time
  // this being said, sometimes weird errors pop up when this isn't initialized here, so...
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  arm_rfft_fast_instance_f32 rfft_instance_1; //a fast RFFT instance for the real FFT
  arm_status status_1 = arm_rfft_fast_init_f32(&rfft_instance_1, size);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  float epsi_spectrum_buffer[size/2]; // for reasons that I don't understand, this array must be defined down here in order to have a valid pointer
  // detrend the input data
  detrend_f(data, size, detrended_data);
  /////////////////////////////////////////////////////////////////
  // CAP TODO can now change this to array multiplication using DSP library functions
  // apply a hamming window to the current data block
  for (j = 0; j < size; j++) {
    windowed_data[j] = detrended_data[j]*hamming_window[j];
  };
  // fft the windowed data
  arm_rfft_fast_f32(&rfft_instance_1, windowed_data, fft_data, fft_direction);
  // store the correct value for power at the nyquist frequency
  nyquist_power = fft_data[1];
  // compute the square magnitude of the complex fft values
  arm_cmplx_mag_squared_f32(fft_data, epsi_spectrum_buffer, size);
  // plug the power at the nyquist frequency into the spectrum vector in the correct location
  spectrum[end] = pow(nyquist_power, 2) / vals->normalization;
  ///////////////////////////////////////////////////////////////////////////////////
  // correct for symmetry and normalization and place values in the correct spectrum vector location
  for (j = 0; j < end; j++) {
    spectrum[j] = 2*epsi_spectrum_buffer[j + 1] / vals->normalization;
  }
}
//
//static void average_spectra_f(uint16_t size, uint8_t num_blocks, float *spectra_in, float *spectrum_out)
///*******************************************************************************
// * @brief
// *   function to average a number of spectra
// * @param size
// *   input - length of spectra, in this case settings->nfft/2
// * @param num_blocks
// *   input - number of discrete spectra over which to average
// * @param *spectra_in
// *   input - pointer to array containing spectra to be averaged, concatenated sequentially
// *   (i.e. length of the array is size*num_blocks)
// * @param *spectrum_out
// *   input - pointer to array where averaged spectrum will be output (length size)
// ******************************************************************************/
//{
//  uint16_t i, j;
//  for(j = 0; j < size; j++) {
//    spectrum_out[j] = 0;
//    for(i = 0; i < num_blocks; i++) {
//      spectrum_out[j] += spectrum_out[j + i*size] / ((float) num_blocks);
//    }
//  }
//}

void hamming_window_f(uint16_t size, float sampling_frequency, float *window, float normal)
/*******************************************************************************
 * @brief
 *   function to calculate hamming window weights and normalization for a given
 *   vector length and sampling frequency
 * @param size
 *   input - length of window
 * @param sampling frequency
 *   input - sampling frequency in Hz
 * @param *hamming_window
 *   input - pointer to array where hamming window weights will be stored
 * @param *normalization
 *   input - pointer to array where normalization values will be stored
 ******************************************************************************/
{
  for (uint16_t j = 0; j < size; j++) {
    window[j] = 0.54 - 0.46*cos(2*M_PI*j/(size - 1));
    normal += pow(window[j], 2);
  }
  normal = normal*((float) sampling_frequency);
}

float sinc_f(float x)
/*******************************************************************************
 * @brief
 *   function to calculate sinc for a given x
 * @param x
 *   input - sinc argument
 * @return
 *   value for sinc(x)
 ******************************************************************************/
{
  float value;
  if (x == 0) {
    value = 1;
  } else {
    value = sin(M_PI*x)/(M_PI*x);
  }
  return value;
}

void interp1_f(float *x, float *v, uint16_t size, float *xq, float *vq, uint16_t sizeq)
/*******************************************************************************
 * @brief
 *   function to interpolate a given set of data onto another array of independent variable values
 * @param *x
 *   input - pointer to array of independent variable
 * @param *v
 *   input - pointer to array of dependent variable associated with x
 * @param size
 *   input - length of x and v arrays
 * @param *xq
 *   input - pointer to array of independent variables values at which to calculate indepenent variable values
 * @param *vq
 *   output - pointer to array of dependent variables values corresponding to values of xq
 * @param sizeq
 *   input - length of xq array
 * @notes
 *   -x and xq must BOTH be sorted in increasing order
 *   -all xq values must be within the limits of x values
 ******************************************************************************/
{
  // define variables
  float x1, x2, delta_x, v1, v2, delta_v, slope, diff;
  uint16_t i, j;
  uint16_t last = 0;
  // loop over xq
  for (i = 0; i < sizeq; i++) {
    // loop over x
    for (j = last; j < size; j++) {
      diff = xq[i] - x[j];
      // if current xq value between appropriate x values, interpolate and set proper vq value
      if (diff < 0) {
        if (j == 0) {
          x1 = x[j]; x2 = x[j + 1];
          v1 = v[j]; v2 = v[j + 1];
          delta_x = x2 - x1; delta_v = v2 - v1;
          slope = delta_v/delta_x;
          vq[i] = v1 + slope*(-xq[i] + x[j]);
          last = 0;
          break;
        } else {
          x1 = x[j - 1]; x2 = x[j];
          v1 = v[j - 1]; v2 = v[j];
          delta_x = x2 - x1; delta_v = v2 - v1;
          slope = delta_v/delta_x;
          vq[i] = v1 + slope*(xq[i] - x[j - 1]);
          last = j - 1;
          break;
        }
      }
    }
  }
}

void smooth_movingmean_f(float *data, float *smoothed, uint16_t size, uint16_t window)
/*******************************************************************************
 * @brief
 *   function to smooth an array using the moving mean technique
 * @param *data
 *   input - pointer to array of input data to smooth
 * @param *smoothed
 *   output - pointer to array where smoothed data will be placed
 * @param size
 *   input - length of data/smoothed arrays
 * @param window
 *   input - number of values over which to perform the mean for smoothing
 * @notes
 *   -there are a number of inefficiencies in this function I haven't gone back to clean up
 ******************************************************************************/
{
  // if size of data vector is smaller than window size, leave data vector as is
  if (size > window) {
    float averaged_data[size];
    // import data vector, initialize constants
    uint16_t left, right, i, k, numpoints;
    float average = 0;
    for (i = 0; i < size; i++) {
      data[i] = data[i];
    }
    // if window is even, put extra data point earlier in the vector (set left)
    if (window % 2 == 0) {
      left = window/2;
      right = window/2;
    } else {
      left = (window - 1)/2;
      right = left + 1;
    }
    // loop over data
    for (i = 0; i < size; i++) {
      // if there are not at least left data points left of the current point, only use available points
      if (i < left) {
        numpoints = i + right;
        // loop and calculate average
        for (k = 0; k < numpoints; k++) {
          average += data[k]/((float) numpoints);
        }
      // if there are not at least right data point right of the current point, only use available points
      } else if (i > ((size - 1) - (right + 1))){
        numpoints = left + (size - i);
        // loop and calculate average
        for (k = i - left; k < size; k++) {
          average += data[k]/((float) numpoints);
        }
      } else {
        numpoints = left + right;
        // loop and calculate average
        for(k = i - left; k < i + right; k++) {
          average += data[k]/((float) numpoints);
        }
      }
      // set data to average and reset average
      averaged_data[i] = average;
      average = 0;
    }
    for (i = 0; i < size; i++) {
      smoothed[i] = averaged_data[i];
    }
  }
}

void detrend_f(float *data, uint16_t size, float *detrended_data)
/*******************************************************************************
 * @brief
 *   function to detrend an array using least squares (i.e. remove line of best fit)
 * @param *data
 *   input - pointer to array of input data to detrend
 * @param size
 *   input - length of data/detrended_data arrays
 * @param *detrended_data
 *   output - pointer to array where detrended data will be places
 * @notes
 *   -had to use 64 bit variables here (doubles) to maintain precision; still some small concern over truncation but I don't think it should be an issue
 ******************************************************************************/
{
  // define variables and arrays
  float x[size], y[size];
  double slope, intercept;
  double sum_x = 0, sum_y = 0, sum_x2 = 0, sum_y2 = 0, sum_xy = 0;
  // calculate sums
  for (uint16_t i = 0; i < size; i++) {
    x[i] = i + 1;
    y[i] = data[i];
    sum_x += x[i];
    sum_y += y[i];
    sum_x2 += pow(x[i], 2);
    sum_y2 += pow(y[i], 2);
    sum_xy += x[i]*y[i];
  }
  // use least squares to find slope and intercept of best fit line
  slope = (size*sum_xy - sum_x*sum_y)/(size*sum_x2 - pow(sum_x, 2.0));
  intercept = (sum_y - slope*sum_x)/size;
  // remove best fit line from data
  for (uint16_t i = 0; i < size; i++) {
    detrended_data[i] = (float) (data[i] - (slope*(i+1) + intercept));
  }
}

void find_vector_indices_f(float *vector, uint16_t size, float *limits, uint16_t *indices)
/*******************************************************************************
 * @brief
 *   function to find the indices of an array at the given limits
 * @param *vector
 *   input - pointer to array for which the function will find the indices
 * @param size
 *   input - length of the vector array
 * @param *limits
 *   input - pointer to a 2 element array with the lower limit as element 0 and the upper limit as element 1
 * @param *indices
 *   output - pointer to a 2 element array where the element 0 is the index of the minimum and element 1 is the index of the maximum
 * @notes
 *   -vector must be sorted in ascending order
 *   -the function finds the first index WITHIN the given limits
 ******************************************************************************/
{
  // define variables
  float min = limits[0], max = limits[1];
  static uint16_t i, j, low, high;
  // find lower index
  for (i = 0; i < size; i++) {
    if(vector[i] > min) {low = i; break;} else {low = i;}
  }
  // find higher index
  for (j = i; j < size; j++) {
    if(vector[j] > max) {high = j - 1; break;} else {high = j;}
  }
  // report values
  indices[0] = low; indices[1] = high;
}

float seawater_specific_heat_f(float salinity, float temperature)
//Inputs:   salinity in ppt
//      temperature in degC
//Outputs:  specific heat in J/(kg*K)
{
  // convert salinity to ppt and pull in t
  static float s;
  s = salinity;
  static float t;
  t = temperature;
  // find specific heat of fresh water
  static float c_p;
  static const float c0 = 4.2174, c1 = -3.720283e-3, c2 = 1.412855e-4, c3 = -2.654387e-6, c4 = 2.093236e-8;
  c_p = c0 + t*(c1 + t*(c2 + t*(c3 + t*c4)));
  // correct for salinity
  static const float a0 = -7.644e-3, a1 = 1.0727e-4, a2 = -1.38e-6;
  static const float b0 = 1.77e-4, b1 = -4.08e-6, b2 = 5.35e-8;
  static float acp;
  acp = a0 + t*(a1 + t*a2);
  static float bcp;
  bcp = b0 + t*(b1 + t*b2);
  c_p = c_p + acp*s + bcp*pow(s, 1.5);
  // convert to J/kgdegC
  c_p = 1000*c_p;
  // return specific heat
  return c_p;
}

float seawater_density_f(float salinity, float temperature, float pressure)
//Inputs:   salinity in ppt
//      temperature in degC
//      pressure in dbar
//Outputs:  density in kg/m^3
{
  // pull in data
  float s, pr, t, rho;
  t = temperature;
  s = salinity;
  pr = pressure;
  // calculate density based on temperature only
  static const float t0 = 999.842594, t1 = 6.793952e-2, t2 = -9.095290e-3, t3 = 1.001685e-4, t4 = -1.120083e-6, t5 = 6.536332e-9;
  rho = t0 + t1*t + t2*pow(t, 2) + t3*pow(t, 3) + t4*pow(t, 4) + t5*pow(t, 5);
  // correct density for salinity
  static const float a0 = 8.24493e-1, a1 = -4.0899e-3, a2 = 7.6438e-5, a3 = -8.2467e-7, a4 = 5.3875e-9;
  static const float b0 = -5.72466e-3, b1 = 1.0227e-4, b2 = -1.6546e-6;
  static float s0, s1;
  s0 = a0 + a1*t + a2*pow(t, 2) + a3*pow(t, 3) + a4*pow(t, 4);
  s1 = b0 + b1*t + b2*pow(t, 2);
  static const float s2= 4.8314e-4;
  rho = rho + s0*s + s1*pow(s, 1.5) + s2*pow(s, 2);
  // correct density for pressure
  static const float c0 = 1.965221e5, c1 = 1484.206, c2 = -23.27105, c3 = 1.360477e-1, c4 = -5.155288e-4;
  static const float d0 = 3.239908, d1 = 1.43713e-3, d2 = 1.16092e-4, d3 = -5.77905e-7;
  static const float e0 = 8.50935e-6, e1 = -6.12293e-7, e2 = 5.2787e-9;
  static const float f0 = 546.746, f1 = -6.03459, f2 = 1.09987e-1, f3 = -6.1670e-4, f4 = 7.944e-1, f5 = 1.6483e-1, f6 = -5.3009e-3;
  static const float g0 = 2.2838e-3, g1 = -1.0981e-5, g2 = -1.6078e-6, g3 = 1.91075e-4;
  static const float h0 = -9.9348e-8, h1 = 2.0816e-9, h2 = 9.1697e-11;
  static float c, d, e;
  c = c0 + c1*t + c2*pow(t, 2) + c3*pow(t, 3) + c4*pow(t, 4);
  d = d0 + d1*t + d2*pow(t, 2) + d3*pow(t, 3);
  e = e0 + e1*t + e2*pow(t, 2);
  static float p, p0, p1, p2;
  p0 = c + (f0 + f1*t + f2*pow(t, 2) + f3*pow(t, 3))*s + (f4 + f5*t + f6*pow(t, 2))*pow(s, 1.5);
  p1 = d + (g0 + g1*t + g2*pow(t, 2))*s + g3*pow(s, 1.5);
  p2 = e + (h0 + h1*t + h2*pow(t, 2))*s;
  p = p0 + p1*pr + p2*pow(pr, 2);
  rho = rho/(1.0 - pr/p);
  return rho;
}

float seawater_thermal_conductivity_f(float salinity, float temperature, float pressure)
//Inputs: salinity in ppt
//      temperature in degC
//      pressure in dbar
//Outputs:  thermal conductivity in W/(m*K)
{
  float S = salinity/1000, P = pressure/100, T = temperature;
  static float k;
  static const float k0 = 0.001365, k1 = 418.55, k2 = 0.003, k3 = -1.025e-5, k4 = 6.53e-4, k5 = -0.29;
  k = k0*k1*(1.0 + k2*T + k3*pow(T, 2) + k4*P + k5*S);
  return k;
}

float seawater_thermal_diffusivity_f(float salinity, float temperature, float pressure)
// Inputs:  salinity in ppt
//      temperature in degC
//      pressure in dbar
// Outputs: thermal diffusivity in m^2/s
{
  float kappa_t, k, rho, c_p;
  k = seawater_thermal_conductivity_f(salinity, temperature, pressure);
  rho = seawater_density_f(salinity, temperature, pressure);
  c_p = seawater_specific_heat_f(salinity, temperature);
  kappa_t = k/(rho*c_p);
  return kappa_t;
}

float seawater_kinematic_viscosity_f(float salinity, float temperature, float pressure)
// Inputs:  salinity in ppt
//      temperature in degC
//      pressure in dbar
// Output:  kinematic viscosity in m^2/s
{
  float S = salinity/1000, T = temperature;
  static float nu, mu, rho;
  static const float mu0 = 1.779e-3, mu1 = 5.9319e-5, mu2 = 1.2917e-6, mu3 = 1.3402e-8, mu4 = 2.8782e-3, mu5 = 3.0553e-6, mu6 = 1.1835e-6;
  mu = mu0 - T*(mu1 - T*(mu2 - T*mu3)) + S*(mu4 - T*(mu5 + T*mu6));
  rho = seawater_density_f(salinity, temperature, pressure);
  nu = mu/rho;
  return nu;
}

float panchev_f(float epsilon, float kvis, uint16_t kvec_size, float *panchev_spec)
/*******************************************************************************
 * @brief
 *   function to calculate the panchev spectrum for the current wavenumber vector
 * @param epsilon
 *   input - dissipation rate in W/kg
 * @param kvis
 *   input - kinemetic viscosity in m^2/s
 * @param kvec_size
 *   number of elements over which to integrate the spectrum til the epsilon cutoff
 * @param *panchev_spec
 *   output - pointer to an array of size fft/2 where the output panchev
 *   spectrum will be stored
 * @return
 *   returns the integral of the panchev spectrum up to the epsilon cutoff
 ******************************************************************************/
{
  // initialize constants and variables
  float eta, conv, z, phi;
  static const float a = 1.6, delta = 0.1, c32 = 1.5, c23 = 2.0/3.0, c43 = 4.0/3.0;
  float sc32 = sqrt(c32), ac32 = pow(a, c32);
  eta = pow(pow(kvis, 3)/epsilon, 1.0/4.0);
  conv = 1/(eta*2.0*M_PI);
  float zeta[10] = {0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95};
  float scale = 2*M_PI*pow((epsilon*pow(kvis, 5)), 0.25);
  float kn[settings->nfft/2];
  float sum = 0;
  float dk = vals->kvec[1] - vals->kvec[0];
  float integral = 0;

  // compute spectrum
  for (uint16_t i = 0; i < kvec_size; i++) {
      kn[i] = vals->kvec[i]/conv;
      for (uint16_t j = 0; j < (sizeof(zeta)/sizeof(zeta[0])); j++) {
          z = zeta[j];
          sum = sum + delta*(1 + pow(z, 2))*(a*pow(z, c23) + (sc32*ac32)*pow(kn[i], c23))*exp(-c32*a*pow((kn[i]/z), c43) - (sc32*ac32*pow((kn[i]/z), 2)));
      }
      phi = 0.5*pow(kn[i], -5.0/3.0)*sum;
      panchev_spec[i] = scale*pow((kn[i]/eta), 2)*phi;
      integral = integral + panchev_spec[i]*dk;
  }

  // return integral value
  return integral;
}

float batchelor_f(float epsilon, float chi, float kvis, float kappa, uint16_t cutoff, float *batchelor_spec)
/*******************************************************************************
 * @brief
 *   function to calculate the batchelor spectrum for the current wavenumber vector
 * @param epsilon
 *   input - dissipation rate in W/kg
 * @param chi
 *   input - temperature dissipation rate in K^2/s
 * @param kvis
 *   input - kinemetic viscosity in m^2/s
 * @param kappa
 *   input - thermal diffusivity in m^2/s
 * @param cutoff
 *   number of elements over which to integrate the spectrum til the chi cutoff
 * @param *batchelor_spec
 *   output - pointer to an array of size fft/2 where the output batchelor
 *   spectrum will be stored
 * @return
 *   returns the integral of the batchelor spectrum up to the chi cutoff
 ******************************************************************************/
{
  // initialize constants and variables
  float eta = pow(pow(kvis, 3)/epsilon, 1.0/4.0);
  static const float q = 3.7;
  float kb = pow((epsilon/kvis/pow(kappa, 2.0)), 1.0/4.0);
  float a, uppera, g_b;
  float dk = vals->kvec[1] - vals->kvec[0];
  float integral = 0;

  // calculate spectrum
  for (uint16_t i = 0; i < cutoff; i++) {
      a = sqrt(2*q)*2*M_PI*vals->kvec[i]/kb;
      uppera = erf(a/sqrt(2.0))*sqrt(M_PI/2);
      g_b = 2*M_PI*a*(exp(-pow(a, 2.0)/2) - a*uppera);
      batchelor_spec[i] = sqrt(q/2)*(chi/kb/kappa)*g_b;
      integral = integral + batchelor_spec[i]*dk;
  }

  // return integral value
  return integral;

}
/*----------------------------- Test Harness -------------------------------*/

/*------------------------------- Footnotes --------------------------------*/
/*------------------------------ End of file -------------------------------*/
