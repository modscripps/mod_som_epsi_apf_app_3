/****************************************************************************
 Module
    mod_som_epsiobp.h
 Description
    onboard processing module for MOD epsilometer
    for integration with mod_som
    calculates epsilon and chi for data from epsilometer
 Notes
    additional notes

 History
 When             Who    What/Why
 --------------   ---    --------
 04 16 2020       MHA    creation of module
 07 24 2020		  CAP	 v0.1.0 functional module
 08 31 2020		  CAP	 v0.2.0 structs for junk packet data
 03 31 2021     CAP  changed types and includes to accommodate SS5 development
                     environment
*****************************************************************************/
#ifndef MOD_SOM_EPSIOBP_H_
#define MOD_SOM_EPSIOBP_H_

/*----------------------------- Include Files ------------------------------*/
#include "arm_math.h"
#include "em_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "arm_const_structs.h"
#include "arm_common_tables.h"

/*-------------------------------- Defines ---------------------------------*/
#define MAXNUM 1000 // Greatest number of output values.  This is reasonable for a 2000-m cast at 2-dbar resolution.
// N samples for N point FFT
//#define FFT_SIZE 512 // The desired FFT length for epsi processing
//#define N_BLOCKS 1 // This should be 1, 3, or 5.
//#define NUM_FFTLENGTHS 1/2*(N_BLOCKS - 1) + 1 // This must be 1, 2, or 3 for N_BLOCKS = 1, 3, or 5
//#define DATA_LEN FFT_SIZE*NUM_FFTLENGTHS // This should be FFT_SIZE times NUM_FFTLENGTHS
//#define F_SAMP 325
#define DEBUG_EPSIOBP // debug flag
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------- Types ----------------------------------*/


typedef struct {
	uint8_t		num_shear;
	uint8_t		num_fp07;
	float	*shear_sv;
	float	*fp07_dTdV;
	float	f_samp;
	uint16_t	Tdiff_size;
	float	*Tdiff_freq;
	float	*Tdiff_coeff;
	uint16_t	cafilter_size;
	float	*cafilter_freq;
	float	*cafilter_coeff;
	float	fp07_noise[4];
	float	f_CTD_pump;
} mod_som_epsiobp_calibration_t;

typedef struct {
	uint16_t fft_size_input;
	float scan_duration_input; // in seconds
	uint8_t max_num_blocks; // suggested number is 4
} mod_som_epsiobp_user_inputs_t;

typedef struct {
	float w;
	float P; // pressure in dbar
	float T; // temperature in degC
	float S; // salinity in psu
} mod_som_epsiobp_scan_CTD_t;

typedef struct {
	uint16_t					verbosity;
	mod_som_epsiobp_scan_CTD_t	scan_CTD;
	float					*shearprobe_data;
	float					*fp07_data;
} mod_som_epsiobp_scan_input_t;

typedef struct {
	float					*epsilon;
	uint16_t					num_epsilon;
	float					*chi;
	uint16_t					num_chi;
	// this struct will continue to as we define more verbosity level
} mod_som_epsiobp_scan_output_t;

// assigned single byte packing for structures to avoid padding
#pragma pack(1)
typedef struct {
	uint8_t						comm_telemetry_packet_format;
	uint16_t					comm_telemetry_packet_size;
	uint16_t					num_scans; // i.e. number of vertical bins in profile
	uint16_t					nfft;
	uint16_t					profile_length; // number of total bytes in profile
	uint8_t						*packets;
} mod_som_epsi_comm_telemetry_t, *mod_som_epsi_comm_telemetry_ptr_t;


/*------------------------------- Variables --------------------------------*/

/*---------------------- Public Function Prototypes ------------------------*/
void mod_som_epsiobp_telemetry_profile_f(mod_som_epsi_comm_telemetry_t comm_telemetry_struct);
void mod_som_epsiobp_init_f(mod_som_epsiobp_calibration_t epsi_cal_values, mod_som_epsiobp_user_inputs_t user_inputs);
void mod_som_epsiobp_scan_f(mod_som_epsiobp_scan_input_t epsiobp_scan_inputs, mod_som_epsiobp_scan_output_t epsiobp_scan_outputs);
float mod_som_epsiobp_scan_epsilon_f(uint8_t shear_channel, float *shear_data, mod_som_epsiobp_scan_CTD_t scan_CTD);
float mod_som_epsiobp_scan_chi_f(uint8_t fp07_channel, float *fp07_data, mod_som_epsiobp_scan_CTD_t scan_CTD);


/*------------------------------ End of file -------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* MOD_SOM_EPSIOBP_H_ */





