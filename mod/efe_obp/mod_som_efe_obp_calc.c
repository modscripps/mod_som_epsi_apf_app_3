// /*******************************************************************************
//  * @brief
//  *   Structure managing the production of EFE OBP data (epsi, chi, accel, spectra)
//  ******************************************************************************/

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
static mod_som_efe_obp_calibration_ptr_t cals_ptr;
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

static mod_som_efe_obp_calc_filters_ptr_t filters_ptr;

static mod_som_efe_obp_calc_theospec_ptr_t theospec_ptr;

static mod_som_efe_obp_calc_fft_ptr_t fft_ptr;


// spectrum buffer for calculation storage and operation
float *spectrum_buffer;

// UNIVERSAL CONSTANTS
static const float g = 9.807; // m/s^2
// CALIBRATION INPUTS

// USER INPUTS


static uint8_t spectrum_counter; // counter for number of spectra up to dof
static uint16_t master_counter; //counter of how many values we have processed.

/*---------------------------- Module Functions ----------------------------*/
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
static void hamming_window_f(uint16_t size, float sampling_frequency, float *hamming_window, float* normalization);
static float sinc_f(float x);
//static void interp1_f(float *x, float *v, uint32_t size, float *xq, float *vq, uint32_t sizeq);
static void smooth_movingmean_f(float *data, float *smoothed, uint16_t size, uint16_t window);
static void detrend_f(float *data, uint32_t size, float *detrended_data);
static void find_vector_indices_f(float *vector, uint32_t size, float *limits, uint16_t *indices);
// SEAWATER PROPERTIES
static float seawater_specific_heat_f(float salinity, float temperature);
static float seawater_density_f(float salinity, float temperature, float pressure);
static float seawater_thermal_conductivity_f(float salinity, float temperature, float pressure);
static float seawater_thermal_diffusivity_f(float salinity, float temperature, float pressure);
static float seawater_kinematic_viscosity_f(float salinity, float temperature, float pressure);
float fom_panchev_f(float *shear_spec, float epsilon, float kvis,uint16_t kvec_indices, uint16_t kvec_size, float *panchev_spec);
static float fom_batchelor_f(float *temp_spec,float epsilon, float chi, float kvis, float kappa, uint16_t cutoff, float *batchelor_spec);
// end

/*------------------------------ Module Code ----------------------------a---*/
// public functions

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
void mod_som_epsiobp_init_f(mod_som_efe_obp_config_ptr_t config_ptr_in, mod_som_efe_obp_settings_ptr_t settings_ptr_in, mod_som_efe_obp_calibration_ptr_t cals_ptr_in)
{
  config = config_ptr_in;
  settings = settings_ptr_in;
  cals_ptr = cals_ptr_in;

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

  RTOS_ERR  err;

  //ALB alloc memory for setup pointer
  //set up default configuration
  vals =
      (mod_som_efe_obp_calc_vals_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP vals.",DEF_NULL,
          sizeof(mod_som_efe_obp_calc_vals_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  vals->freq =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP vals freq.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  vals->kvec =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP vals kvec.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  vals->fp07_noise =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP vals fp07_noise.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  vals->fp07_cutoff =
      (uint16_t *)Mem_SegAlloc(
          "MOD SOM EFE OBP vals fp07_cutoff.",DEF_NULL,
          sizeof(uint16_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  vals->hamming_window =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP vals hamming_window.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  spectrum_buffer =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP spectrum_buffer.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


  //ALB alloc memory for setup pointer
  //set up default configuration
  filters_ptr =
      (mod_som_efe_obp_calc_filters_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP filters.",DEF_NULL,
          sizeof(mod_som_efe_obp_calc_filters_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  filters_ptr->ca_shear =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt ca.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  filters_ptr->adc_tf =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt adc tf.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  filters_ptr->fp07_filter =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt fpo7.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  filters_ptr->magsq =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt magsq.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  filters_ptr->oakey_shear =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt oakey.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  filters_ptr->shear_filter =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP filt shear.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  theospec_ptr =
      (mod_som_efe_obp_calc_theospec_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP filters.",DEF_NULL,
          sizeof(mod_som_efe_obp_calc_filters_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  theospec_ptr->batchelor_spec =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP theo batch.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  theospec_ptr->panchev_spec =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP theo panch.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  theospec_ptr->kn =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP theo kn.",DEF_NULL,
          sizeof(float)*settings->nfft/2,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  fft_ptr =
      (mod_som_efe_obp_calc_fft_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE OBP fft.",DEF_NULL,
          sizeof(mod_som_efe_obp_calc_fft_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->detrended_data =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft detre.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->fft_data =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft data.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->windowed_data =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft windata.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->averaged_data =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft avgdata.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->fp07_smoothed =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft fpo7sm.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->x =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft x.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->y =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft y.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  fft_ptr->epsi_spectrum_buffer =
      (float *)Mem_SegAlloc(
          "MOD SOM EFE OBP fft spec.",DEF_NULL,
          sizeof(float)*settings->nfft,
          &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);



  //ALB calculate the frequency vector
  //ALB make a fake cafilter_coeff
//  float kvec_start=2.0;
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    vals->freq[i] = (float) (i + 1)/(settings->nfft)*config->f_samp;
    filters_ptr->ca_shear[i]=cals_ptr->cafilter_coeff[i];
  }


  //  for (uint16_t i = 0; i < kvec_interp_size; i++) {
  //  }

  //ALB TODO create cafilter_coeff like the FPO7 noise

  //ALB compute elect shear, it is use in shear_filter_f
  float denom = 2*vals->freq[settings->nfft/2 - 1];
  // interpolate to get proper cafilter given freq
//  interp1_f(cals->cafilter_freq, cals->cafilter_coeff,
//            cals->cafilter_size, vals->freq,
//            filters_ptr->ca_shear, settings->nfft/2);

  // loop to find values for other elements of the filter
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // elect_shear is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
      // elect_fp07 is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
      filters_ptr->adc_tf[i] = pow(sinc_f(vals->freq[i]/denom), 4.0);

  }

  //ALB compute fpo7 noise spectrum
  mod_som_epsiobp_fp07_noise_f();

  // calculate hamming window and normalization
  hamming_window_f(settings->nfft, config->f_samp, vals->hamming_window, &vals->normalization);
  // initialize spectrum_counter
  spectrum_counter = 0;
  //initialize master counter
  master_counter = 0;
}

/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units vs frequency
 *   for a single shear channel and stuff into an output location
 * @param seg_buffer
 *   input - the ptr to shear data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
void mod_som_efe_obp_shear_spectrum_f(float *seg_buffer, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
{
  // SHEAR
  // pull in fall rate
  //ALB no need for w  at that stage.
  //ALB I'll do it with the avg_spectrum

//  float w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt;
  // Get the electronics transfer functions.

  //ALB modify
//  float shear_filter[settings->nfft/2];
  //ALB no filter correction at that stage.
  //ALB I'll do it with the avg_spectrum
//  mod_som_epsiobp_shear_filters_f(filters_ptr->shear_filter, w);
  // removed looping over all shear channels, now just doing 1
  // calculate spectrum from shear data
  power_spectrum_f(seg_buffer, spectrum_buffer, settings->nfft, config->f_samp);
  //CAP convert spectra from V^2/Hz to (m/s)^2/Hz
  //ALB no conversion at that stage.
  //ALB I'll do it with the avg_spectrum
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
//      spectrum_buffer[i] = spectrum_buffer[i]*pow(2*g/(cals->shear_sv*w), 2);
      //CAP run spectrum through filter
      //ALB no filter correction at that stage.
      //ALB I'll do it with the avg_spectrum
//      spectrum_buffer[i] = spectrum_buffer[i]/filters_ptr->shear_filter[i];
      // stuff spectrum into output
      //ALB move the level up to be able to compute chi and epsilon
      if (mod_som_efe_obp_ptr->cpt_spectra_ptr->dof==0){
          //ALB move the level up to be able to compute chi and epsilon
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr+spectra_offset+i) = spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }else{
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr+spectra_offset+i) += spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }
//      *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_shear_ptr+spectra_offset+i) += 100000*spectrum_buffer[i];
  }

}

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
void mod_som_efe_obp_temp_spectrum_f(float *seg_buffer, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
{
  // TEMP
  // pull in fall rate
  //ALB no need for w at that stage.
  //ALB I'll do it with the avg_spectrum

//  float w = mod_som_efe_obp_ptr->cpt_spectra_ptr->avg_ctd_dpdt;
  //  pull in fp07 filter function and calculate using fall speed as input

  //ALB modify
//  float fp07_filter[settings->nfft/2];

//  mod_som_epsiobp_fp07_filters_f(filters_ptr->fp07_filter, w);
  // removed looping over all temp channels, now just doing 1
  //init cutoff to end so no funny business
//  *(vals->fp07_cutoff) = settings->nfft/2;
  // calculate spectrum from temp data
  power_spectrum_f(seg_buffer, spectrum_buffer, settings->nfft, config->f_samp);
  //CAP convert spectra from V^2/Hz to (degC)^2/Hz
  //ALB no need for conversion at that stage.
  //ALB I'll do it with the avg_spectrum
//  for (uint16_t i = 0; i < settings->nfft/2; i++) {
//    spectrum_buffer[i] = spectrum_buffer[i]*pow(cals->fp07_dTdV, 2);
//  }

  //ALB Finding the cutoff should be done with avg_spectrum with the right number of dof.
  //ALB i.e., inside cpt_dissrate.
  //TODO move the line below to the right spot.
  //ALB  *(vals->fp07_cutoff) = mod_som_epsiobp_fp07_cutoff_f(spectrum_buffer, settings->nfft/2);


  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    //CAP correct spectrum using filter, divide by filter to get temp spectrum vs. freq
    //ALB no need for conversion at that stage.
    //ALB I'll do it with the avg_spectrum
//    spectrum_buffer[i] = spectrum_buffer[i]/filters_ptr->fp07_filter[i];
    // stuff spectrum into output
      //ALB move the level up to be able to compute chi and epsilon
      if (mod_som_efe_obp_ptr->cpt_spectra_ptr->dof==0){
          //ALB move the level up to be able to compute chi and epsilon
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr+spectra_offset+i) =spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }else{
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr+spectra_offset+i) +=spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }
//   *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_temp_ptr+spectra_offset+i) += 100000 * spectrum_buffer[i];
  }
}

/*******************************************************************************
 * @brief
 *   function to calculate power density spectra in appropriate units vs frequency
 *   for a single acceleration channel and stuff into an output location
 * @param shear_ptr
 *   input - the ptr to accel data from which to calculate spectra
 * @param spectra_offset
 *   input - offset to determine correct output location
 * @param mod_som_efe_obp_ptr
 *   input - the struct containing the output locations for the spectra and all
 *   the relevant input CTD data
 ******************************************************************************/
void mod_som_efe_obp_accel_spectrum_f(float *seg_buffer, int spectra_offset, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
{
  // ACCEL
  // changed to accommodate only a single acceleration channel
  // calculate spectrum from acceleration data
  power_spectrum_f(seg_buffer, spectrum_buffer, settings->nfft, config->f_samp);
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // convert spectra from V^2/Hz to (g)^2/Hz, recalling g is ~9.81 m/s^2
    // ALB no conversion to g. Do not know if this is necessary yet
    // NOTE: this should be double checked (linear offsets in space should not affect psd)
    // stuff spectrum into output
      //ALB The sumation of FOCO is done here (i.e., +=)
    //ALB BE AWARE: I am summing the FOCO over dof times (i.e. 5 times). That is why I have the +=
      if (mod_som_efe_obp_ptr->cpt_spectra_ptr->dof==0){
          //ALB move the level up to be able to compute chi and epsilon
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr+spectra_offset+i) = spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }else{
          *(mod_som_efe_obp_ptr->cpt_spectra_ptr->spec_accel_ptr+spectra_offset+i) += spectrum_buffer[i]/(float)mod_som_efe_obp_ptr->settings_ptr->degrees_of_freedom;
      }
  }


}

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
void mod_som_efe_obp_calc_epsilon_f(float *local_epsilon, float *nu,
                                    float *fom_ptr,float * kcutoff,
                                    mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
{


  // pull in CTD values
  static float w, P, T, S;
  w = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt;
  P = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure;
  T = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature;
  S = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity;

  // calculate wavenumber vector
//  for (uint16_t i = 0; i < settings->nfft/2; i++) {
//    //Make the k vector from the freq vector with the appropriate fall speed
//    vals->kvec[i] = vals->freq[i]/w;
//  }

  //lookup tables for epsilon from the spectral integrals (shear10) from 2-10 cpm, these come from eps1_mmp/Mike Gregg's work and are (apparently) empirically determined
  static const float eps_fit_shear10[5] = {8.6819e-04, -3.4473e-03, -1.3373e-03, 1.5248, -3.1607};
  static const float shtotal_fit_shear10[5] = {6.9006e-04, -4.2461e-03, -7.0832e-04, 1.5275, 1.8564};

  float   spectrum_integral;
  float * local_avg_spec_shear_ptr;
  float log10_spectrum_integral;
  float threshold = -3.0;

  float kvis = seawater_kinematic_viscosity_f(S, T, P);
  *nu = kvis;
  static const float kvec_min_1 = 2, kvec_max_1 = 10;
  static float eps_1, log10_eps;

  float kvec_limits_1[] = {kvec_min_1, kvec_max_1};
  float k_max = config->f_CTD_pump/w;

  float dk = (float) config->f_samp/(float) settings->nfft/w;
  float k_cutoff;
  float kvec_limits_2[2];
  float kvec_limits_3[2];
  uint16_t kvec_indices_2[2];
  uint16_t kvec_indices_3[2];
  uint16_t kvec_2_size;
  uint16_t kvec_3_size;

  float eps_2;
  float eps_3;

  // start loop here
  // removed multiple channel handling for the time being
//  for (uint8_t j = 0; config->num_shear; j++) {
  //We can now begin the iterative procedure of calculating epsilon, following eps1_mmp.m
  //STAGE 1
  //first find the min/max wavenumbers
  static uint16_t kvec_indices_1[2];
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_1, kvec_indices_1);
  uint16_t kvec_1_size = kvec_indices_1[1] - kvec_indices_1[0] + 1;

  local_avg_spec_shear_ptr=
      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[kvec_indices_1[0]];

  spectrum_integral = 0;
  //Compute the integral of the spectrum from 2-10 cpm
  for (uint16_t i = 0; i < kvec_1_size; i++) {
//    spectrum_integral += (local_avg_spec_shear_ptr[i]*w*pow((2*M_PI*vals->kvec[kvec_indices_1[0]+i]), 2.0))*dk;
    spectrum_integral += local_avg_spec_shear_ptr[i]*dk;
  }
  log10_spectrum_integral = log10(spectrum_integral);



  if (log10_spectrum_integral > threshold) {
    log10_eps = eps_fit_shear10[0]*pow(log10_spectrum_integral, 4.0) +
                eps_fit_shear10[1]*pow(log10_spectrum_integral, 3.0) +
                eps_fit_shear10[2]*pow(log10_spectrum_integral, 2.0) +
                eps_fit_shear10[3]*log10_spectrum_integral +
                eps_fit_shear10[4];
    eps_1 = pow(10.0, log10_eps);
  } else {
    log10_eps = shtotal_fit_shear10[0]*pow(log10_spectrum_integral, 4.0) +
                shtotal_fit_shear10[1]*pow(log10_spectrum_integral, 3.0) +
                shtotal_fit_shear10[2]*pow(log10_spectrum_integral, 2.0) +
                shtotal_fit_shear10[3]*log10_spectrum_integral +
                shtotal_fit_shear10[4];
    eps_1 = 7.5*kvis*pow(10.0, log10_eps);
  };


  //STAGE 2 - Second estimate
  k_cutoff = 0.0816*pow(eps_1, 0.25)/pow(kvis, 0.75);  // k for 90% variance of Panchev spectrum
  if (k_cutoff > k_max) {k_cutoff = k_max;} // limit set by noise spectrum (or in this case the pump freq).  May need to fix that.
  kvec_limits_2[0] = kvec_min_1,
  kvec_limits_2[1] = k_cutoff;

  //find this k range, 2 cpm out to k_cutoff
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_2, kvec_indices_2);
  kvec_2_size = kvec_indices_2[1] - kvec_indices_2[0] + 1;
//  float spectrum_kvec_2[kvec_2_size];
//  float dk = vals->kvec[1] - vals->kvec[0];

  //Compute the integral of the spectrum over this range
  local_avg_spec_shear_ptr=
      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[kvec_indices_2[0]];

  spectrum_integral = 0;
  for (uint16_t i = 0; i < kvec_2_size; i++) {
//    spectrum_kvec_2[i] = (mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[kvec_indices_2[0] + i]);
//    spectrum_integral += (local_avg_spec_shear_ptr[i]*w*pow((2*M_PI*vals->kvec[kvec_indices_2[0]+i]), 2.0))*dk;
    spectrum_integral += local_avg_spec_shear_ptr[i]*dk;
//
//    spectrum_integral += spectrum_kvec_2[i]*dk;
  }
  //Compute eps from this and divide by 0.9 to account for the excess over 90%
  eps_2 = 7.5*kvis*spectrum_integral/0.9;


  // THIRD STAGE
  k_cutoff = 0.0816*pow(eps_2, 0.25)/pow(kvis, 0.75);

  if (k_cutoff > k_max) {k_cutoff = k_max;} // limit set by noise spectrum (or in this case the pump freq).  May need to fix that.
  *kcutoff=k_cutoff;

//  % third estimate
  //find this k range, 2 cpm out to k_cutoff
  kvec_limits_3[0] = kvec_min_1;
  kvec_limits_3[1] = k_cutoff;
  //find this k range, 2 cpm out to k_cutoff
  find_vector_indices_f(vals->kvec, settings->nfft/2, kvec_limits_3, kvec_indices_3);
  kvec_3_size = kvec_indices_3[1] - kvec_indices_3[0] + 1;
//  float spectrum_kvec_3[kvec_3_size];

  //Compute the integral of the spectrum over this range
  local_avg_spec_shear_ptr=
      &mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[kvec_indices_3[0]];
  //Compute the integral of the spectrum
  spectrum_integral = 0;
  for (uint16_t i = 0; i < kvec_3_size; i++) {
//    spectrum_kvec_3[i] = (mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_shear_ptr[kvec_indices_3[0] + i]);
//    spectrum_integral += (local_avg_spec_shear_ptr[i]*w*pow((2*M_PI*vals->kvec[kvec_indices_3[0]+i]), 2.0))*dk;
    spectrum_integral += local_avg_spec_shear_ptr[i]*dk;
//    spectrum_integral += spectrum_kvec_3[i]*dk;
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
//  char kc[10];
//  sprintf(kc,"kc=%3.2f",k_cutoff);
//  printf(kc);

  // calculate figure of merit
  *fom_ptr = fom_panchev_f(local_avg_spec_shear_ptr, eps_3, kvis, kvec_indices_3[0], kvec_3_size, theospec_ptr->panchev_spec);

//    }
}

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
void mod_som_efe_obp_calc_chi_f(float *local_epsilon, float *local_chi,
                                float *kappa_t, float *fcutoff,
                                float *fom, mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr)
{
    // pull in CTD values
    float w, P, T, S;
    w = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_dpdt;
    P = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_pressure;
    T = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_temperature;
    S = mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_ctd_salinity;

    float * local_avg_spec_temp_ptr;
    float chi_sum;
    float kvis;
    float dk;
    float chi;


//    //ALB get cut off
//    *(vals->fp07_cutoff) = mod_som_epsiobp_fp07_cutoff_f(
//        mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr,
//        settings->nfft/2);

    local_avg_spec_temp_ptr=
        mod_som_efe_obp_ptr->cpt_dissrate_ptr->avg_spec_temp_ptr;

    chi_sum = 0;

    for (uint16_t i = 0; i < *(vals->fp07_cutoff); i++) {

        // multiply by fall rate and (2*pi*k)^2 to get spectrum vs wavenumber
//        chi_sum += local_avg_spec_temp_ptr[i]*w*pow((2*M_PI*vals->kvec[i]), 2.0);
        chi_sum += local_avg_spec_temp_ptr[i];
    }

    //  compute chi
    *kappa_t = seawater_thermal_diffusivity_f(S, T, P);

    dk = (float) config->f_samp/ (float) settings->nfft/w;
    chi = 6*(*kappa_t)*dk*chi_sum;
    *local_chi = chi;

    //ALB modify
//    float batchelor_spec[settings->nfft/2];
    kvis = seawater_kinematic_viscosity_f(S, T, P);
    *fcutoff=vals->freq[*(vals->fp07_cutoff)];

    //ALB compute FOM
    *fom = fom_batchelor_f(local_avg_spec_temp_ptr,*local_epsilon, chi, kvis, *kappa_t, *(vals->fp07_cutoff), theospec_ptr->batchelor_spec);

//  }
}

void mod_som_efe_obp_correct_convert_avg_spectra_f(float * temp_spectrum,
                                               float * shear_spectrum,
                                               float * accel_spectrum,
                                               float   fall_rate){

  uint16_t fp07_cutoff=0;
  mod_som_epsiobp_shear_filters_f(filters_ptr->shear_filter, fall_rate);
  mod_som_epsiobp_fp07_filters_f(filters_ptr->fp07_filter, fall_rate);
  //init cutoff to end so no funny business. We need to get the freq cutoff in the frequency domain
//  *(vals->fp07_cutoff) = settings->nfft/2;
  fp07_cutoff = mod_som_epsiobp_fp07_cutoff_f(
      temp_spectrum,
      settings->nfft/2);
  *(vals->fp07_cutoff)=fp07_cutoff;

  for (uint16_t i = 0; i < settings->nfft/2; i++) {
      //Make the k vector from the freq vector with the appropriate fall speed
      vals->kvec[i] = vals->freq[i]/fall_rate;

      shear_spectrum[i] = shear_spectrum[i]*pow(2*g/(cals_ptr->shear_sv*fall_rate), 2)*fall_rate*pow((2*M_PI*vals->kvec[i]), 2.0);
      temp_spectrum[i]  = temp_spectrum[i]*pow(cals_ptr->fp07_dTdV, 2)*fall_rate*pow((2*M_PI*vals->kvec[i]), 2.0);
      shear_spectrum[i] = shear_spectrum[i]/filters_ptr->shear_filter[i];
      temp_spectrum[i]  = temp_spectrum[i]/filters_ptr->fp07_filter[i];

  }
}

/*******************************************************************************
 * @brief
 *   function to calculate the filters (physical, electrical, etc.) necessary to correct the shear probe data spectrum
 * @param *shear_filter
 *   output - vector of length settings->nfft/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
void mod_som_epsiobp_shear_filters_f(float *shear_filter, float fall_rate)
{
  // define arrays and variables
  //ALB modify
//  float elect_shear[settings->nfft/2];
//  float ca_shear[settings->nfft/2];
//  float oakey_shear[settings->nfft/2];

//  uint16_t end = settings->nfft/2;
//  float denom = 2*vals->freq[end - 1];
  // interpolate to get proper cafilter given freq
//  float *ca_freq = cals->cafilter_freq, *ca_coeff = cals->cafilter_coeff, *v_freq = vals->freq;
//  interp1_f(ca_freq, ca_coeff, cals->cafilter_size, v_freq, filters_ptr->ca_shear, settings->nfft/2);
  // loop to find values for other elements of the filter
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // elect_shear is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
//      filters_ptr->elect_shear[i] = pow(sinc_f(vals->freq[i]/denom), 4.0);
    // oakey filter comes from airfoil probe response
      filters_ptr->oakey_shear[i] = oakey_filter_f(vals->freq[i], fall_rate);
    // calculate single filter array
    shear_filter[i] = pow((filters_ptr->ca_shear[i]*
                           filters_ptr->adc_tf[i]), 2.0)*
                           filters_ptr->oakey_shear[i];
  }
}

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
float oakey_filter_f(float f, float fall_rate)
{
  static const float lambda_c = 0.02;
  return 1/(1 + pow((lambda_c*f/fall_rate), 2.0));
}

/*******************************************************************************
 * @brief
 *   function to calculate the filters (physical, electrical, etc.) necessary to correct the fp07 data spectrum
 * @param *fp07_filter
 *   output - vector of length settings->nfft/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
void mod_som_epsiobp_fp07_filters_f(float *fp07_filter, float fall_rate)
{
  // interpolate to get proper Tdiff filter given freq
//  interp1_f(cals->Tdiff_freq, cals->Tdiff_coeff, cals->Tdiff_size, vals->freq, Tdiff, settings->nfft/2);
  // loop to find values for other elements of the filter
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    // magsq filter comes from... where? but it's function is constant and shouldn't ever change
      filters_ptr->magsq[i] = 1/(1 + pow((2*M_PI*0.005*pow(fall_rate, -0.32)*vals->freq[i]), 2.0));
    // calculate single filter array
    fp07_filter[i] = pow(filters_ptr->adc_tf[i], 2)*filters_ptr->magsq[i];
  }
}

/*******************************************************************************
 * @brief
 *   function to calculate noise on the raw fp07 spectrum
 ******************************************************************************/
void mod_som_epsiobp_fp07_noise_f()
{
  // loop over all frequencies using the noise calibration values
  for (uint16_t i = 0; i < settings->nfft/2; i++) {
    vals->fp07_noise[i] = pow(10, (cals_ptr->fp07_noise[0] + cals_ptr->fp07_noise[1]*log10(vals->freq[i]) + cals_ptr->fp07_noise[2]*pow(log10(vals->freq[i]), 2) + cals_ptr->fp07_noise[3]*pow(log10(vals->freq[i]), 3)));
  }
}

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
uint16_t mod_som_epsiobp_fp07_cutoff_f(float *fp07_spectrum, uint16_t size)
{
  // define variables and arrays
  float signal_noise_ratio = 3;
  uint16_t window = 15; //this is arbitrary, determined in processing before this
  float threshold;
  uint16_t cutoff = size;
  float adjust_noise=0;
//  float fp07_smoothed[size];
  // smooth data
  smooth_movingmean_f(fp07_spectrum, fft_ptr->fp07_smoothed, size, window);
  for (uint16_t i = size-10; i < size; i++) {
      adjust_noise+=fft_ptr->fp07_smoothed[i]/vals->fp07_noise[i];
  }
  adjust_noise=adjust_noise/10;

  // loop to find cutoff, starting at the 5th FOCO
  for (uint16_t i = 5; i < size; i++) {
    threshold = signal_noise_ratio*vals->fp07_noise[i]*adjust_noise;
    if (fft_ptr->fp07_smoothed[i] < threshold) {
      cutoff = i;
      break;
    }
  }
  return cutoff;
}

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
void power_spectrum_f(float *data, float *spectrum, uint32_t size, float sampling_frequency)
{
  bool fft_direction = 0;
  uint16_t j, end;
  end = size/2 - 1;
  float nyquist_power = 0;
//  static float df;
  //ALB modify
//  float detrended_data[size], hamming_window[size], windowed_data[size], fft_data[size]; // see note about hamming window near calculation below
//  df = (float) sampling_frequency / (float) size;
  // this initialization should be taken out and probably put in a separate function so we're not reinitializing on every spectrum; it wastes a huge amount of time
  // this being said, sometimes weird errors pop up when this isn't initialized here, so...
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  arm_rfft_fast_instance_f32 rfft_instance_1; //a fast RFFT instance for the real FFT
//  arm_status status_1 =
      arm_rfft_fast_init_f32(&rfft_instance_1, size);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  float epsi_spectrum_buffer[size/2]; // for reasons that I don't understand, this array must be defined down here in order to have a valid pointer
  // detrend the input data
  detrend_f(data, size, fft_ptr->detrended_data);
  /////////////////////////////////////////////////////////////////
  // CAP TODO can now change this to array multiplication using DSP library functions
  // apply a hamming window to the current data block
  for (j = 0; j < size; j++) {
      fft_ptr->windowed_data[j] = fft_ptr->detrended_data[j]*vals->hamming_window[j];
  };
  // fft the windowed data
  arm_rfft_fast_f32(&rfft_instance_1, fft_ptr->windowed_data, fft_ptr->fft_data, fft_direction);
  // store the correct value for power at the nyquist frequency
  nyquist_power = fft_ptr->fft_data[1];
  // compute the square magnitude of the complex fft values
  arm_cmplx_mag_squared_f32(fft_ptr->fft_data, fft_ptr->epsi_spectrum_buffer, size);
  // plug the power at the nyquist frequency into the spectrum vector in the correct location
  spectrum[end] = pow(nyquist_power, 2) / vals->normalization;
  ///////////////////////////////////////////////////////////////////////////////////
  // correct for symmetry and normalization and place values in the correct spectrum vector location
  for (j = 0; j < end; j++) {
    spectrum[j] = 2*fft_ptr->epsi_spectrum_buffer[j + 1] / vals->normalization;
  }
}

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
//static void average_spectra_f(uint16_t size, uint8_t num_blocks, float *spectra_in, float *spectrum_out)//{
//  uint16_t i, j;
//  for(j = 0; j < size; j++) {
//    spectrum_out[j] = 0;
//    for(i = 0; i < num_blocks; i++) {
//      spectrum_out[j] += spectrum_out[j + i*size] / ((float) num_blocks);
//    }
//  }
//}

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
void hamming_window_f(uint16_t size, float sampling_frequency, float *window, float* normal)
{
  for (uint16_t j = 0; j < size; j++) {
    window[j] = 0.54 - 0.46*cos(2*M_PI*j/(size - 1));
    *normal += pow(window[j], 2);
  }
  *normal = *normal*((float) sampling_frequency);
}

/*******************************************************************************
 * @brief
 *   function to calculate sinc for a given x
 * @param x
 *   input - sinc argument
 * @return
 *   value for sinc(x)
 ******************************************************************************/
float sinc_f(float x)
{
  float value;
  if (x == 0) {
    value = 1;
  } else {
    value = sin(M_PI*x)/(M_PI*x);
  }
  return value;
}

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
/*void interp1_f(float *x, float *v, uint32_t size, float *xq, float *vq, uint32_t sizeq)

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
//*/

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
void smooth_movingmean_f(float *data, float *smoothed, uint16_t size, uint16_t window)
{
  // if size of data vector is smaller than window size, leave data vector as is
  if (size > window) {
//    float averaged_data[size];
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
      fft_ptr->averaged_data[i] = average;
      average = 0;
    }
    for (i = 0; i < size; i++) {
      smoothed[i] = fft_ptr->averaged_data[i];
    }
  }
}

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
void detrend_f(float *data, uint32_t size, float *detrended_data)
{
  // define variables and arrays
//  float x[size], y[size];
  double slope, intercept;
  double sum_x = 0, sum_y = 0, sum_x2 = 0, sum_y2 = 0, sum_xy = 0;
  // calculate sums
  for (uint16_t i = 0; i < size; i++) {
    fft_ptr->x[i] = i + 1;
    fft_ptr->y[i] = data[i];
    sum_x += fft_ptr->x[i];
    sum_y += fft_ptr->y[i];
    sum_x2 += pow(fft_ptr->x[i], 2);
    sum_y2 += pow(fft_ptr->y[i], 2);
    sum_xy += fft_ptr->x[i]*fft_ptr->y[i];
  }
  // use least squares to find slope and intercept of best fit line
  slope = (size*sum_xy - sum_x*sum_y)/(size*sum_x2 - pow(sum_x, 2.0));
  intercept = (sum_y - slope*sum_x)/size;
  // remove best fit line from data
  for (uint16_t i = 0; i < size; i++) {
    detrended_data[i] = (float) (data[i] - (slope*(i+1) + intercept));
  }
}

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
void find_vector_indices_f(float *vector, uint32_t size, float *limits, uint16_t *indices)
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
float fom_panchev_f(float *shear_spec, float epsilon, float kvis,uint16_t kvec_indices, uint16_t kvec_size, float *panchev_spec)
{
  // initialize constants and variables
  float eta, conv, z, phi;
  const float a = 1.6, delta = 0.1, c32 = 1.5, c23 = 2.0/3.0, c43 = 4.0/3.0;
  float sc32 = sqrt(c32), ac32 = pow(a, c32);
  float zeta[10] = {0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95};
  float scale = 2*M_PI*pow((epsilon*pow(kvis, 5)), 0.25);
  //ALB modify
//  float kn[settings->nfft/2];
  float sum      = 0;
  float integral = 0;
  float average  = 0;

  //ALB Figure of Merit business
  float sig_c1 =  5.0/4.0;
  float sig_c2 = -7.0/9.0;
  float sig_lnS = sig_c1 * pow((float)settings->degrees_of_freedom,sig_c2);
  float sum1=0;
  float fom=0;


  eta = pow(pow(kvis, 3)/epsilon, 1.0/4.0);
  conv = 1/(eta*2.0*M_PI);

  // compute spectrum
  for (uint16_t i = 0; i < kvec_size; i++) {
      theospec_ptr->kn[i] = vals->kvec[kvec_indices+i]/conv;

      for (uint16_t j = 0; j < (sizeof(zeta)/sizeof(zeta[0])); j++) {
          z = zeta[j];
          sum = sum + delta*(1 + pow(z, 2))*
                      (a*pow(z, c23) +
                      (sc32*ac32)*pow(theospec_ptr->kn[i], c23))*
                       exp(-c32*a*pow((theospec_ptr->kn[i]/z), c43) -
                       (sc32*ac32*pow((theospec_ptr->kn[i]/z), 2)));
      }
      phi = 0.5*pow(theospec_ptr->kn[i], -5.0/3.0)*sum;
      panchev_spec[i] =log(shear_spec[i]/( scale*pow((theospec_ptr->kn[i]/eta), 2.0)*phi));
      integral = integral + panchev_spec[i];
  }

  average = integral / (float) kvec_size;
  /*  Compute  variance*/
  for (uint16_t i = 0; i < kvec_size; i++) {
      {
        sum1 = sum1 + pow((panchev_spec[i] - average), 2);
      }
  }
  fom = sum1 / (float)(kvec_size-1);
  fom = fom /sig_lnS;

  // return integral value
  return fom;
}

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
float fom_batchelor_f(float *temp_spec, float epsilon, float chi, float kvis, float kappa, uint16_t cutoff, float *batchelor_spec)
{
  // initialize constants and variables
//  float eta = pow(pow(kvis, 3)/epsilon, 1.0/4.0);
  static const float q = 3.7;
  float kb = pow((epsilon/kvis/pow(kappa, 2.0)), 1.0/4.0);
  float a, uppera, g_b;
//  float dk = vals->kvec[1] - vals->kvec[0];
  float integral = 0;
  float average  =0;

  //ALB Figure of Merit buisness
  float sig_c1 =  5.0/4.0;
  float sig_c2 = -7.0/9.0;
  float sig_lnS = sig_c1 * pow((float)settings->degrees_of_freedom,sig_c2);

  float sum1;
  float fom;
  uint16_t fom_cutoff=cutoff;

  // calculate spectrum
  for (uint16_t i = 0; i < cutoff; i++) {
      a = sqrt(2*q)*2*M_PI*vals->kvec[i]/kb;
      uppera = erf(a/sqrt(2.0))*sqrt(M_PI/2);
      g_b = 2*M_PI*a*(exp(-pow(a, 2.0)/2) - a*uppera);
//      batchelor_spec[i] = sqrt(q/2)*(chi/kb/kappa)*g_b;
      batchelor_spec[i] =log(temp_spec[i]/(sqrt(q/2)*(chi/kb/kappa)*g_b));
      if (isnan(batchelor_spec[i])){
          //ALB end of the inegration
          fom_cutoff=i;
          break;
      }
//      integral = integral + batchelor_spec[i]*dk;
      integral = integral + batchelor_spec[i];
  }

  average = integral / (float) fom_cutoff;
  /*  Compute  variance*/
  for (uint16_t i = 0; i < fom_cutoff; i++) {
      {
        sum1 = sum1 + pow((batchelor_spec[i] - average), 2);
      }
      fom = sum1 / (float)(fom_cutoff-1);
      fom = fom /sig_lnS;
  }

  // return integral value
  return fom;

}
/*----------------------------- Test Harness -------------------------------*/

/*------------------------------- Footnotes --------------------------------*/
/*------------------------------ End of file -------------------------------*/
