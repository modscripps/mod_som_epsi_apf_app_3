/****************************************************************************
 Module
    mod_som_efe_obp_calc
 Description
    description of module
 Notes
    additional notes

 History
 When             Who    What/Why
 --------------   ---    --------
 DD MMMM YYYY     XXX    changes
*****************************************************************************/
#ifndef MOD_SOM_EFE_OBP_CALC_H_
#define MOD_SOM_EFE_OBP_CALC_H_

/*----------------------------- Include Files ------------------------------*/
#include <mod_som_common.h>
#include <efe/mod_som_efe.h>
#include "mod_som_efe_obp.h"
#include "arm_math.h"
#include "em_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "arm_const_structs.h"
#include "arm_common_tables.h"

/*-------------------------------- Defines ---------------------------------*/
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define MOD_SOM_EFE_OBP_CANNOT_ALLOCATE_VALS 0x01u
/*--------------------------------- Types ----------------------------------*/

/*******************************************************************************
 * @brief
 *   Structure
 ******************************************************************************/
typedef struct{
  float *freq;
  float *kvec; // wavenumber vector
  float *hamming_window; // hamming window weights vector
  float normalization; // normalization vector
  float *fp07_noise;
  uint16_t * fp07_cutoff; // index for temperature integration cutoff to calculate chi
  float *epsi_spectrum_buffer; // buffer for all of the shear calculated spectra in a block
  float *chi_spectrum_buffer; //
  float *epsi_averaged_spectrum; // buffer for the averaged shear spectrum
  float *chi_averaged_spectrum; // buffer for the averaged temperature spectrum
}
mod_som_efe_obp_calc_vals_t, *mod_som_efe_obp_calc_vals_ptr_t;

typedef struct{
  float * shear_filter;
  float * fp07_filter;
  float * ca_shear;
  float * oakey_shear;
  float * adc_tf;
  float * magsq;
}
mod_som_efe_obp_calc_filters_t, *mod_som_efe_obp_calc_filters_ptr_t;

typedef struct{
  float * panchev_spec;
  float * batchelor_spec;
  float * kn;
}
mod_som_efe_obp_calc_theospec_t, *mod_som_efe_obp_calc_theospec_ptr_t;

typedef struct{
  float * detrended_data;
  float * windowed_data;
  float * averaged_data;
  float * y;
  float * x;
  float * fft_data;
  float * fp07_smoothed;
  float * epsi_spectrum_buffer;
}
mod_som_efe_obp_calc_fft_t, *mod_som_efe_obp_calc_fft_ptr_t;



// TEMPORARY
typedef struct {
  float w;
  float P; // pressure in dbar
  float T; // temperature in degC
  float S; // salinity in psu
} mod_som_epsiobp_scan_CTD_t, *mod_som_epsiobp_scan_CTD_ptr_t;

/*------------------------------- Variables --------------------------------*/
/*-------------------------- Function Prototypes ---------------------------*/
// OBP CALC INIT
void mod_som_epsiobp_init_f(mod_som_efe_obp_config_ptr_t config_ptr_in,
                            mod_som_efe_obp_settings_ptr_t settings_ptr_in,
                            mod_som_efe_obp_calibration_ptr_t cals_ptr_in);
// OBP CALCS
// OBP CALCULATIONS
void mod_som_efe_obp_shear_spectrum_f(float *seg_buffer,
                                      int spectra_offset,
                                      mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
void mod_som_efe_obp_temp_spectrum_f(float *seg_buffer,
                                     int spectra_offset,
                                     mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
void mod_som_efe_obp_accel_spectrum_f(float *seg_buffer,
                                      int spectra_offset,
                                      mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
void mod_som_efe_obp_calc_epsilon_f(float *local_epsilon, float *nu,
                                    float *fom_ptr,float * kcutoff,
                                    mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
void mod_som_efe_obp_calc_chi_f(float *local_epsilon, float *local_chi,
                                float *kappa_t, float *fcutoff, float *fom,
                                mod_som_efe_obp_ptr_t mod_som_efe_obp_ptr);
void mod_som_efe_obp_correct_convert_avg_spectra_f(float * temp_spectrum,
                                               float * shear_spectrum,
                                               float * accel_spectrum,
                                               float   fall_rate);


/*------------------------------ End of file -------------------------------*/
#endif /* MOD_SOM_EFE_OBP_CALC_H_ */
