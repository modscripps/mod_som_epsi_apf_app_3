/****************************************************************************
 Module
    mod_som_epsiobp.c
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
 07	24 2020       CAP    v0.1.0 functional module
 08 31 2020		  CAP	 v0.2.0 included junk data packet function
 03 31 2021     CAP  changed types and includes to accommodate SS5 development
                     environment
*****************************************************************************/

/*----------------------------- Include Files ------------------------------*/
#include <mod_som_efe_obp_calculus.h>

/*--------------------------- External Variables ---------------------------*/
/*----------------------------- Module Defines -----------------------------*/
/*------------------------------ Module Types ------------------------------*/
/*---------------------------- Module Variables ----------------------------*/
// UNIVERSAL CONSTANTS
static const float g = 9.807; // m/s^2
// CALIBRATION INPUTS
static float f_CTD_pump;
static float f_samp;
static uint16_t num_shear; // number of shear probes on instrument
static uint16_t num_fp07; // number of fp07 sensors on instrument
static float *sv; // Sensitivity; this should be put into a structure with other needed constants and params.
static float *cafilter_freq; // empirical charge filter frequency vector
static float *cafilter_coeff; // empirical charge filter coefficient vector
static uint16_t cafilter_size; // length of charge filter vector
static float n0, n1, n2, n3;
static float *dTdV; // fp07 calibration value
static float *Tdiff_coeff; // empirical Tdiff filter frequency vector
static float *Tdiff_freq; // empirical Tdiff filter coefficient vector
static uint16_t Tdiff_size; // length of Tdiff filter vector
// USER INPUTS
static uint16_t fft_size;
// INPUT DATA
static float *shear_data; // This is a placeholder for the buffer where the shear data are stored.
static float *fp07_data;
// CALCULATED VALUES/VECTORS
static float *freq;
static float *kvec; // wavenumber vector
static uint16_t num_samples; // number of data points from a single sensor being passed in
static uint8_t num_blocks; // number of non-overlapping windowed blocks averaged in spectra-calculating functions
static uint16_t scan_length; // number of data points being used to calculate spectra/values
static float *fp07_noise;
//static arm_rfft_fast_instance_f32 rfft_instance_1; //a fast RFFT instance for the real FFT
static float *spectrum_1; // buffer for the spectrum
static float *spectrum_2; // second spectrum buffer
static uint16_t master_counter; //counter of how many values we have processed.

// for telemetry packet function which spits out junk data; temporary only
//static uint16_t telemetry_counter = 0;
//static uint16_t accel_counter = 0;

//Placeholder output structures.  Define a struct with the needed output data.
//static float epsi_out[MAXNUM];
//static float chi_out[MAXNUM];


/*---------------------- Private Function Prototypes -----------------------*/
// FILTERS
static void mod_som_epsiobp_shear_filters_f(float *shear_filter, float fall_rate);
static float oakey_filter_f(float f, float w);
static void mod_som_epsiobp_fp07_filters_f(float *fp07_filter, float fall_rate);
static void mod_som_epsiobp_fp07_noise_f();
uint16_t mod_som_epsiobp_fp07_cutoff_f(float *fp07_spectrum, uint16_t size);
// MATH AND VECTOR FUNCTIONS
static void mod_som_epsiobp_pwelch_f(float *data, float *spectrum, uint16_t size, uint16_t sampling_frequency);
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


/*---------------- Module Code - Public Functions --------------------------*/
void mod_som_epsiobp_telemetry_profile_f(mod_som_epsi_comm_telemetry_t comm_telemetry_struct)
/*******************************************************************************
 * @brief
 *   function to populate junk data into memory for telemetry, packets output are packet_size length in bytes
 *   and profile_length bytes are stuffed into memory starting at the *packets pointer
 * @param comm_telemetry_struct
 *   input - structure with telemetry input and output information; user must populate packet_format and either
 *   num_scans (if packet_format < 3) or nfft (if packet_format == 3) before calling function
 ******************************************************************************/
{
	mod_som_epsi_comm_telemetry_ptr_t comm_telemetry_struct_ptr = &comm_telemetry_struct;
	// first handle packet formats 1 and 2
	uint8_t packet_format = comm_telemetry_struct.comm_telemetry_packet_format;
	// allocate memory based on packet format and number of scans
	if (packet_format == 1) {
		comm_telemetry_struct.comm_telemetry_packet_size = 10;
	} else if (packet_format == 2) {
		comm_telemetry_struct.comm_telemetry_packet_size = 16;
	} else if (packet_format == 3) {
		float num_fourier_bytes = comm_telemetry_struct.nfft;
		float max_size = 25000 - 52;
		comm_telemetry_struct.comm_telemetry_packet_size = 9 + 3*num_fourier_bytes;
		comm_telemetry_struct.num_scans = (uint16_t) (max_size/comm_telemetry_struct.comm_telemetry_packet_size);
	}
	comm_telemetry_struct.profile_length = comm_telemetry_struct.num_scans*comm_telemetry_struct.comm_telemetry_packet_size;

	/////////////////////////////////////// these malloc's will need to change to Mem_SegAlloc for micriumOS purposes ////////////////////////////////////
	comm_telemetry_struct.packets = (uint8_t*) malloc(comm_telemetry_struct.profile_length);

	// create the junk data; time will tick up from 1000 (increase by 10 with every scan), pressure will start at 2000 and decrease by 5 with every scan,
	// chi will go from -3 to -12, epsi will go from -12 to -3, acceleration will be constant (0xBBBB, 0xCCCC, and 0xDDDD on ascending channels), and qc flag will be constant at 0xAA
	for (uint16_t i = 0; i < comm_telemetry_struct.num_scans; i++) {
		// first calculate the three bytes of espi and chi
		static float const lower_limit = -12, upper_limit= -3;
		static float const num_bits_per = 12;
		// this is to create linearly increasing/decreasing junk data values for epsi and chi
		float epsi, chi;
		uint8_t epsi_chi_byte_1, epsi_chi_byte_2, epsi_chi_byte_3;
		float delta = (upper_limit - lower_limit)/(pow(2, num_bits_per) - 1);
		float eight_bit_cutoff = lower_limit + delta*255, four_bit_cutoff = lower_limit + delta*15;
		float step = (upper_limit - lower_limit)/comm_telemetry_struct.num_scans;
		// epsi starts at -12, goes up by step for every function call til -3
		epsi = lower_limit + ((float) i)*(step);
		// chi starts at -3, goes down by step for every function call til -12
		chi = upper_limit - ((float) i)*(step);
		// format epsi and chi into 12 bit representations - all 0's is -12, for every integer value, add delta (in this case, 9/4095)
		// epsi is first, chi is second, so we'll start with chi and format the 3rd byte first
		uint8_t chi_temp;
		if (chi > eight_bit_cutoff) {
			uint16_t chi_mask = 0x0F00;
			uint16_t chi_diff = (uint16_t) ((chi - lower_limit)/delta);
			epsi_chi_byte_3 = (uint8_t) chi_diff;
			chi_temp = (chi_diff & chi_mask) >> 8;
		} else {
			epsi_chi_byte_3 = (uint8_t) ((chi - lower_limit)/delta);
			chi_temp = 0x00;
		}
		if (epsi > four_bit_cutoff) {
			uint16_t epsi_mask = 0x000F;
			uint16_t epsi_diff = (uint16_t) ((epsi - lower_limit)/delta);
			uint8_t epsi_temp = (epsi_diff & epsi_mask) << 4;
			epsi_chi_byte_2 = epsi_temp + chi_temp;
			epsi_chi_byte_1 = (epsi_diff >> 4);
		} else {
			uint8_t epsi_diff = (uint8_t) ((epsi - lower_limit)/delta);
			epsi_chi_byte_2 = (epsi_diff << 4) + chi_temp;
			epsi_chi_byte_1 = 0x00;
		}
		uint16_t time = 1000 + i*10;
		float pressure = 2000 - i*5;
		uint8_t *pressure_ptr = &pressure;
		// swap pressure endianness for packet
		float swapped_pressure;
		uint8_t *swapped_pressure_ptr = (uint8_t*) &swapped_pressure;
		swapped_pressure_ptr[0] = pressure_ptr[3];
		swapped_pressure_ptr[1] = pressure_ptr[2];
		swapped_pressure_ptr[2] = pressure_ptr[1];
		swapped_pressure_ptr[3] = pressure_ptr[0];
		float *new_pressure_ptr = &swapped_pressure;
		uint8_t *packet_pressure_ptr = comm_telemetry_struct_ptr->packets;
		packet_pressure_ptr = packet_pressure_ptr + 2 + i*comm_telemetry_struct.comm_telemetry_packet_size;
		// stuff first 8 bytes of packet
		*(comm_telemetry_struct.packets + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) (time >> 8); // time
		*(comm_telemetry_struct.packets + 1 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) (time); // time
		memcpy(packet_pressure_ptr, new_pressure_ptr, sizeof(float)); // pressure
		*(comm_telemetry_struct.packets + 6 + i*comm_telemetry_struct.comm_telemetry_packet_size) = epsi_chi_byte_1; // epsilon and chi bytes
		*(comm_telemetry_struct.packets + 7 + i*comm_telemetry_struct.comm_telemetry_packet_size) = epsi_chi_byte_2; // epsilon and chi bytes
		*(comm_telemetry_struct.packets + 8 + i*comm_telemetry_struct.comm_telemetry_packet_size) = epsi_chi_byte_3; // epsilon and chi bytes
		// stuff qc flag for formats 1 and 2
		if (packet_format < 3) {
			*(comm_telemetry_struct.packets + 9 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xAA; // qc flag
		}
		if (packet_format == 2) {
			*(comm_telemetry_struct.packets + 10 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xBB; // rms acceleration channel 1
			*(comm_telemetry_struct.packets + 11 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xBB; // rms acceleration channel 1
			*(comm_telemetry_struct.packets + 12 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xCC; // rms acceleration channel 2
			*(comm_telemetry_struct.packets + 13 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xCC; // rms acceleration channel 2
			*(comm_telemetry_struct.packets + 14 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xDD; // rms acceleration channel 3
			*(comm_telemetry_struct.packets + 15 + i*comm_telemetry_struct.comm_telemetry_packet_size) = (uint8_t) 0xDD; // rms acceleration channel 3
		// stuff fourier coefficients for format 3
		} else if (packet_format == 3) {
			for (uint16_t j = 0; j < 3*comm_telemetry_struct.nfft; j++) {
				if (j < comm_telemetry_struct.nfft) {
					*(comm_telemetry_struct.packets + 9 + i*comm_telemetry_struct.comm_telemetry_packet_size + j) = (uint8_t) 0xAA;
				} else if (j < 2*comm_telemetry_struct.nfft) {
					*(comm_telemetry_struct.packets + 9 + i*comm_telemetry_struct.comm_telemetry_packet_size + j) = (uint8_t) 0xBB;
				} else {
					*(comm_telemetry_struct.packets + 9 + i*comm_telemetry_struct.comm_telemetry_packet_size + j) = (uint8_t) 0xCC;
				}
			}
		}
	}
}


void mod_som_epsiobp_init_f(mod_som_epsiobp_calibration_t epsi_cal_values, mod_som_epsiobp_user_inputs_t user_inputs)
/*******************************************************************************
 * @brief
 *   function to initialize mod_som_epsiobp module with correct user inputs, calibration values, etc.
 * @param epsi_cal_values
 *   input - structure with calibration for sensors and necessary instrumentation information
 * @param user_inputs
 *   input - structure with user-defined inputs for epsi onboard processing
 ******************************************************************************/
{
	// read in calibration inputs
	num_shear = epsi_cal_values.num_shear;
	num_fp07 = epsi_cal_values.num_fp07;
	f_samp = epsi_cal_values.f_samp;
	f_CTD_pump = epsi_cal_values.f_CTD_pump;
	n0 = epsi_cal_values.fp07_noise[0];
	n1 = epsi_cal_values.fp07_noise[1];
	n2 = epsi_cal_values.fp07_noise[2];
	n3 = epsi_cal_values.fp07_noise[3];
	Tdiff_size = epsi_cal_values.Tdiff_size;
	cafilter_size = epsi_cal_values.cafilter_size;
	// read in user inputs and calculate corresponding values
	fft_size = user_inputs.fft_size_input;
	float scan_duration = user_inputs.scan_duration_input;
	num_samples = (uint16_t) (scan_duration*f_samp);
	for (num_blocks = user_inputs.max_num_blocks; num_blocks > 0; num_blocks--) {
		if ((uint16_t) (num_blocks*fft_size) < num_samples) {break;}
	}
	scan_length = num_blocks*fft_size;
	// allocate memory based on inputs
	sv = (float*) malloc(num_shear*sizeof(float));
	dTdV = (float*) malloc(num_fp07*sizeof(float));
	shear_data = (float*) malloc(num_samples*sizeof(float)); // shear probe data vector
	fp07_data = (float*) malloc(num_samples*sizeof(float)); // fp07 data vector
	freq = (float*) malloc(fft_size/2*sizeof(float)); // freq vector, zero freq NOT included
	spectrum_1 = (float*) malloc(fft_size/2*sizeof(float)); // first spectrum buffer
	spectrum_2 = (float*) malloc(fft_size/2*sizeof(float)); // second spectrum buffer
	kvec = (float*) malloc(fft_size/2*sizeof(float)); // wavenumber vector
	Tdiff_freq = (float*) malloc(Tdiff_size*sizeof(float)); // empirical Tdiff filter frequency vector
	Tdiff_coeff = (float*) malloc(Tdiff_size*sizeof(float)); // empirical Tdiff filter coefficient vector
	cafilter_freq = (float*) malloc(cafilter_size*sizeof(float));
	cafilter_coeff = (float*) malloc(cafilter_size*sizeof(float));
	fp07_noise = (float*) malloc(fft_size/2*sizeof(float)); // fp07 noise vector
	// read in arrays
	for (uint8_t i = 0; i < num_shear; i++) {
		sv[i] = epsi_cal_values.shear_sv[i];
	}
	for (uint8_t i = 0; i < num_fp07; i++) {
		dTdV[i] = epsi_cal_values.fp07_dTdV[i];
	}
	for (uint16_t i = 0; i < cafilter_size; i++) {
		cafilter_freq[i] = epsi_cal_values.cafilter_freq[i];
		cafilter_coeff[i] = epsi_cal_values.cafilter_coeff[i];
	}
	for (uint16_t i = 0; i < Tdiff_size; i++) {
		Tdiff_freq[i] = epsi_cal_values.Tdiff_freq[i];
		Tdiff_coeff[i] = epsi_cal_values.Tdiff_coeff[i];
	}
	// calculate the frequency vector
	for (uint16_t i = 0; i < fft_size/2; i++) {
		freq[i] = (float) (i + 1)/(fft_size)*f_samp;
	}
	//initialize master counter
	master_counter = 0;
}

void mod_som_epsiobp_scan_f(mod_som_epsiobp_scan_input_t epsiobp_scan_inputs, mod_som_epsiobp_scan_output_t epsiobp_scan_outputs)
/*******************************************************************************
 * @brief
 *   function to calculate relevant values based on desired verbosity from a single scan
 *   IN DEVELOPMENT
 ******************************************************************************/
{
	// will first need to handle verbosity I think
	// if only one sensor each
	if (num_shear == 1 && num_fp07 == 1) {
		float epsilon, chi;
		epsilon = mod_som_epsiobp_scan_epsilon_f(0, epsiobp_scan_inputs.shearprobe_data, epsiobp_scan_inputs.scan_CTD);
		chi = mod_som_epsiobp_scan_chi_f(0, epsiobp_scan_inputs.fp07_data, epsiobp_scan_inputs.scan_CTD);
		epsiobp_scan_outputs.epsilon = &epsilon;
		epsiobp_scan_outputs.num_epsilon = 1;
		epsiobp_scan_outputs.chi = &chi;
		epsiobp_scan_outputs.num_chi = 1;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// below in in development
	} else {
		//Compute epsilon and chi for a scan
		float epsilon[num_shear], chi[num_fp07];
		// development of these loops in progress; we'll need to decide how to handle the pointer to the correct sensor's data or if we want to stuff arrays before passing in parameters
	//	for (uint16_t i = 0; i < num_shear; i++) {
	//		epsilon[i] = mod_som_epsiobp_scan_epsilon_f(i, shearprobe_data, scan_CTD);
	//	}
	//	for (uint16_t i = 0; i < num_fp07; i++) {
	//		chi[i] = mod_som_epsiobp_scan_chi_f(i, fp07_data, scan_CTD);
	//	}
	}

	master_counter++;
}

float mod_som_epsiobp_scan_epsilon_f(uint8_t shear_channel, float *shear_data, mod_som_epsiobp_scan_CTD_t scan_CTD)
/*******************************************************************************
 * @brief
 *   function to calculate a value for dissipation (i.e. epsilon) in W/kg over a single scan (vertical bin)
 * @param shear_channel
 *   input - the number of the probe from which the input data was recorded (0-indexed), used for pulling correct calibration
 * @param *shear_data
 *   input - pointer to vector of raw shear probe data of at least length num_samples (see module init function for calculation)
 * @param scan_CTD
 *   input - structure with CTD values from the scan with which the input data is associated (i.e. average pressure, temperature, salinity, and fall rate)
 * @return
 *   turbulent dissipation (i.e. epsilon) in W/kg
 * @notes
 * 	 -data will be truncated to length num_samples for calculations
 ******************************************************************************/
{
	// pull in CTD values
	static float w, P, T, S;
	w = scan_CTD.w; P = scan_CTD.P; T = scan_CTD.T; S = scan_CTD.S;
	// set up a local array for analysis and convert data to m/s
	float shear_analysis[scan_length];
	for (uint16_t i = 0; i < scan_length; i++) {
		shear_analysis[i] = shear_data[i]*2*g/(sv[shear_channel]*w);
	}
	float kvis = seawater_kinematic_viscosity_f(S, T, P);
	float shear_filter[fft_size/2];
	static const float kvec_min_1 = 2, kvec_max_1 = 10;
	float k_max = f_CTD_pump/w;
	float kvec_limits_1[] = {kvec_min_1, kvec_max_1};
	//lookup tables for epsilon from the spectral integrals (shear10) from 2-10 cpm, these come from eps1_mmp/Mike Gregg's work and are (apparently) empirically determined
	static const float eps_fit_shear10[5] = {8.6819e-04, -3.4473e-03, -1.3373e-03, 1.5248, -3.1607};
	static const float shtotal_fit_shear10[5] = {6.9006e-04, -4.2461e-03, -7.0832e-04, 1.5275, 1.8564};
	//We start with the pointers to the data. These will need to be updated to point to the real data.
	// The length of the data to be used should be fft_size, and the number of half-overlapping blocks that we will average over will be N_BLOCKS.
	//This can only be 3, 5 or 7 to keep total data a sensible and understandable number. In that case N_TOT would be 2, 3 and 4 times fft_size, respectively.
	//In all cases, the spectrum will wind up being the same size, fft_size/2.
	//The first step is to get the wavenumber spectrum of shear in (s^2)/cpm.
	//First compute the spectrum of the raw data in V^2/Hz and put it in spec:
	// this has changed, as the shear_analysis is now in different units
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// difference in detrending (in window vs. in full data) results in up to a ~5% difference in epsilon
	mod_som_epsiobp_pwelch_f(shear_analysis, spectrum_1, fft_size, f_samp);

	// Get the electronics transfer functions.
	mod_som_epsiobp_shear_filters_f(shear_filter, w);
	// run spectrum through filter and convert to wavenumber space
	for (uint16_t i = 0; i < fft_size/2; i++) {
		// Make the k vector from the freq vector with the appropriate fall speed
		kvec[i] = freq[i]/w;
		// run through filter
		spectrum_1[i] = spectrum_1[i]/shear_filter[i];
		// change to wavenumber space
		spectrum_1[i] = spectrum_1[i]*w*pow((2*M_PI*kvec[i]), 2.0);
	}
	// spec is now the shear spectrum, corrected for the antialias and probe response filters,
	//in units of (s^-2)/Hz.
	//We can now begin the iterative procedure of calculating epsilon, following eps1_mmp.m.
	//STAGE 1
	//first find the min/max wavenumbers
	static uint16_t kvec_indices_1[2];
	find_vector_indices_f(kvec, fft_size/2, kvec_limits_1, kvec_indices_1);
	uint16_t kvec_1_size = kvec_indices_1[1] - kvec_indices_1[0] + 1;
	float kvec_1[kvec_1_size], spectrum_kvec_1[kvec_1_size];
	for (uint16_t i = 0; i < kvec_1_size; i++) {
		kvec_1[i] = kvec[kvec_indices_1[0] + i];
		spectrum_kvec_1[i] = spectrum_1[kvec_indices_1[0] + i];
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
	find_vector_indices_f(kvec, fft_size/2, kvec_limits_2, kvec_indices_2);
	uint16_t kvec_2_size = kvec_indices_2[1] - kvec_indices_2[0] + 1;
	float kvec_2[kvec_2_size], spectrum_kvec_2[kvec_2_size];
	float eps_2;
	float dk = kvec[1] - kvec[0];
	//Compute the integral of the spectrum over this range
	static float spectrum_integral = 0;
	for (uint16_t i = 0; i < kvec_2_size; i++) {
		kvec_2[i] = kvec[kvec_indices_2[0] + i];
		spectrum_kvec_2[i] = spectrum_1[kvec_indices_2[0] + i];
		spectrum_integral += spectrum_kvec_2[i]*dk;
	}
	//Compute eps from this and divide by 0.9 to account for the excess over 90%
	eps_2 = 7.5*kvis*spectrum_integral/0.9;
	// THIRD STAGE
	k_cutoff = 0.0816*pow(eps_2, 0.25)/pow(kvis, 0.75);
	if (k_cutoff > k_max) {k_cutoff = k_max;} // limit set by noise spectrum (or in this case the pump freq).  May need to fix that.
//	% third estimate
	//find this k range, 2 cpm out to k_cutoff
	float kvec_limits_3[] = {kvec_min_1, k_cutoff};
	static uint16_t kvec_indices_3[2];
	//find this k range, 2 cpm out to k_cutoff
	find_vector_indices_f(kvec, fft_size/2, kvec_limits_3, kvec_indices_3);
	uint16_t kvec_3_size = kvec_indices_3[1] - kvec_indices_3[0] + 1;
	float kvec_3[kvec_3_size], spectrum_kvec_3[kvec_3_size];
	static float eps_3;
	//Compute the integral of the spectrum
	spectrum_integral = 0;
	for (uint16_t i = 0; i < kvec_3_size; i++) {
		kvec_3[i] = kvec[kvec_indices_3[0] + i];
		spectrum_kvec_3[i] = spectrum_1[kvec_indices_3[0] + i];
		spectrum_integral += spectrum_kvec_3[i]*dk;
	}	//Compute eps from this and divide by 0.9 to account for the excess over 90%
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
	return eps_3;
}

float mod_som_epsiobp_scan_chi_f(uint8_t fp07_channel, float *fp07_data, mod_som_epsiobp_scan_CTD_t scan_CTD)
/*******************************************************************************
 * @brief
 *   function to calculate a value for dissipation of thermal variance in K^2/s (chi)
 * @param fp07_channel
 *   input - the number of the fp07 sensor from which the input data was recorded (0-indexed), used for pulling correct calibration
 * @param *fp07_data
 *   inut - pointer to vector of raw fp07 data of at least length num_samples (see module init function for calculation)
 * @param scan_CTD
 *   input - structure with CTD values from the scan with which the input data is associated (i.e. average pressure, temperature, salinity, and fall rate)
 * @return
 *   thermal variance dissipation (i.e. chi) in K^2/s
 * @notes
 * 	 -data will be truncated to length num_samples for calculations
 ******************************************************************************/
{
	// set up an array for local calculations
	float fp07_analysis[scan_length];
	// pull in CTD values
	static float w, P, T, S;
	w = scan_CTD.w; P = scan_CTD.P; T = scan_CTD.T; S = scan_CTD.S;
	//	compute raw data spectrum in V^2/Hz
	mod_som_epsiobp_pwelch_f(fp07_data, spectrum_1, fft_size, f_samp);
	//	convert data to temperature using calibration value dTdV
	for (uint16_t i = 0; i < scan_length; i++) {
		fp07_analysis[i] = fp07_data[i]*dTdV[fp07_channel];
	}
	//	compute temperature spectrum in degC^2/Hz
	mod_som_epsiobp_pwelch_f(fp07_analysis, spectrum_2, fft_size, f_samp);
	//	pull in fp07 filter function and calculate using fall speed as input
	float fp07_filter[fft_size/2];
	mod_som_epsiobp_fp07_filters_f(fp07_filter, w);
	// correct spectrum using filter and convert to wavenumber space
	for (uint16_t i = 0; i < fft_size/2; i++) {
		//Make the k vector from the freq vector with the appropriate fall speed
		kvec[i] = freq[i]/w;
		//	divide by filter to get temp spectrum vs freq
		spectrum_2[i] = spectrum_2[i]/fp07_filter[i];
		//	multiply by fall rate and (2*pi*k)^2 to get temp gradient spectrum vs wavenumber
		spectrum_2[i] = spectrum_2[i]*w*pow((2*M_PI*kvec[i]), 2);
	}
	//	find fp07 cutoff
	uint16_t fp07_cutoff = fft_size/2;
	fp07_cutoff = mod_som_epsiobp_fp07_cutoff_f(spectrum_1, fft_size/2);
	//	redefine spectrum and wavenumbers only up to cutoff
	float k_fp07[fp07_cutoff];
	float chi_spectrum[fp07_cutoff];
	static float chi_sum = 0;
	for (uint16_t i = 0; i < fp07_cutoff; i++) {
		k_fp07[i] = kvec[i];
		chi_spectrum[i] = spectrum_2[i];
		chi_sum += chi_spectrum[i];
	}
	//	compute chi
	float kappa_t = seawater_thermal_diffusivity_f(S, T, P);
	float dk = k_fp07[1] - k_fp07[0];
	float chi = 6*kappa_t*dk*chi_sum;
	return chi;
}


/*---------------- Module Code - Private Functions ------------------------*/
void mod_som_epsiobp_shear_filters_f(float *shear_filter, float fall_rate)
/*******************************************************************************
 * @brief
 *   function to calculate the filters (physical, electrical, etc.) necessary to correct the shear probe data spectrum
 * @param *shear_filter
 *   output - vector of length fft_size/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
{
	// define arrays and variables
	float elect_shear[fft_size/2], ca_shear[fft_size/2], oakey_shear[fft_size/2];
	uint16_t end = fft_size/2;
	float denom = 2*freq[end - 1];
	// interpolate to get proper cafilter given freq
	interp1_f(cafilter_freq, cafilter_coeff, cafilter_size, freq, ca_shear, fft_size/2);
	// loop to find values for other elements of the filter
	for (uint16_t i = 0; i < fft_size/2; i++) {
		// elect_shear is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
		elect_shear[i] = pow(sinc_f(freq[i]/denom), 4.0);
		// oakey filter comes from airfoil probe response
		oakey_shear[i] = oakey_filter_f(freq[i], fall_rate);
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
 *   output - vector of length fft_size/2 to which the shear filter values will be output
 * @param fall_rate
 *   input - average fall rate of the instrument over the scan under study in m/s
 ******************************************************************************/
{
	// define arrays and variables
	float elect_fp07[fft_size/2], magsq[fft_size/2], Tdiff[fft_size/2];
	uint16_t end = fft_size/2;
	float denom = 2*freq[end - 1];
	// interpolate to get proper Tdiff filter given freq
	interp1_f(Tdiff_freq, Tdiff_coeff, Tdiff_size, freq, Tdiff, fft_size/2);
	// loop to find values for other elements of the filter
	for (uint16_t i = 0; i < fft_size/2; i++) {
		// elect_fp07 is the ADC filter, in this case a sinc4 function, will eventually need to change to allow for user input
		elect_fp07[i] = pow(sinc_f(freq[i]/denom), 4.0);
		// magsq filter comes from... where? but it's function is constant and shouldn't ever change
		magsq[i] = 1/(1 + pow((2*M_PI*0.005*pow(fall_rate, -0.32)*freq[i]), 2.0));
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
	for (uint16_t i = 0; i < fft_size/2; i++) {
		fp07_noise[i] = n0 + n1*log10(freq[i]) + n2*pow(log10(freq[i]), 2) + n3*pow(log10(freq[i]), 3);
	}
}

uint16_t mod_som_epsiobp_fp07_cutoff_f(float *fp07_spectrum, uint16_t size)
/*******************************************************************************
 * @brief
 *   function to calculate the cutoff above which the fp07 spectrum is assumed to be noise
 * @param *fp07_spectrum
 *   input - pointer to raw fp07 spectrum in V^2/Hz
 * @param size
 *   input - should always be fft_size/2, this is unnecessary at this point
 * @return
 *   index of the highest frequency at which the fp07 signal is not noise
 ******************************************************************************/{
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
		threshold = signal_noise_ratio*pow(10, fp07_noise[i]);
		if (fp07_smoothed[i] < threshold) {
			cutoff = i;
			break;
		}
	}
	return cutoff;
}

void mod_som_epsiobp_pwelch_f(float *data, float *spectrum, uint16_t size, uint16_t sampling_frequency)
/*******************************************************************************
 * @brief
 *   function to calculate power spectral density from a given set of data using welch's method
 *   	(for a good breakdown, see Solomon Jr.'s 'PSD Computations Using Welch's Method' published by Sandia National Lab)
 * @param *data
 *   input - pointer to input data to be analyzed, must be between 'size' and 65535 points (the input data will not be modified)
 * @param *spectrum
 *   output - pointer to vector of length 'size/2' where spectrum data will be spit out
 * @param size
 *   input - number of points to use in fft (alternatively referred to as nfft)
 * @param sampling_frequecy
 *   input - frequency at which the data under study was sampled
 * @notes
 * 	 -uses hamming windowing
 * 	 -num_blocks in the epsiobp initialization routine, not here
 * 	 -all power offset calculated in the fft are tossed as they correspond to zero frequency/wavenumber
 * 	 	(i.e. output spectrum does not contain zero frequency data)
 * 	 -there is non-trivial loss of precision due to using 32bit types (floats) instead of 64bit types (doubles)
 * 	 -currently no error reporting for a bad parseval's theorem check
 * 	 -arm_rfft instances are initialized in a separate function
 ******************************************************************************/
{
	// initialize variables, constants, and arrays
	bool fft_direction = 0;
	uint16_t i, j, end, num_windows;
	num_windows = 2*num_blocks - 1;
	end = size/2 - 1;
	float normalization = 0, nyquist_power = 0;
	static float df;
	float parseval_check_detrended_data[size*num_blocks], chopped_data[num_windows][size], detrended_data[size], hamming_window[size], windowed_data[size], fft_data[size];
	df = (float) sampling_frequency / (float) size;
	// this initialization should be taken out and probably put in a separate function so we're not reinitializing on every spectrum; it wastes a huge amount of time
	// this being said, sometimes weird errors pop up when this isn't initialized here, so...
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	arm_rfft_fast_instance_f32 rfft_instance_1; //a fast RFFT instance for the real FFT
	arm_status status_1 = arm_rfft_fast_init_f32(&rfft_instance_1, size);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// create the hamming window and set normalization value
	for (j = 0; j < size; j++) {
		hamming_window[j] = 0.54 - 0.46*cos(2*M_PI*j/(size - 1));
		normalization += pow(hamming_window[j], 2);
	}
	normalization = normalization*((float) sampling_frequency);
	float block_spectrum[size/2]; // for reasons that I don't understand, this array must be defined down here in order to have a valid pointer
	// loop across each window of length 'size'
	for (i = 0; i < num_windows; i++) {
		// chop the data into the correct block length with 50% overlap
		for (j = 0; j < size; j++) {
			chopped_data[i][j] = data[(size/2)*i + j];
		}
		// detrend the current data block
		detrend_f(&chopped_data[i][0], size, detrended_data);
		// apply a hamming window to the current data block
		for (j = 0; j < size; j++) {
			windowed_data[j] = detrended_data[j]*hamming_window[j];
		};
		// fft the windowed data
		arm_rfft_fast_f32(&rfft_instance_1, windowed_data, fft_data, fft_direction);
		// store the correct value for power at the nyquist frequency
		nyquist_power = fft_data[1];
		// compute the square magnitude of the complex fft values
		arm_cmplx_mag_squared_f32(fft_data, block_spectrum, size);
		// plug the power at the nyquist frequency into the spectrum vector in the correct location
		if (i == 0) {
			spectrum[end] = pow(nyquist_power, 2)/normalization/num_windows;
		} else {
			spectrum[end] += pow(nyquist_power, 2)/normalization/num_windows;
		}
		// correct for symmetry, normalization, and averaging across multiple data blocks and place values in the correct spectrum vector location
		for (j = 0; j < end; j++) {
			if (i == 0) {
				spectrum[j] = 2*block_spectrum[j + 1]/normalization/num_windows;
			} else {
				spectrum[j] += 2*block_spectrum[j + 1]/normalization/num_windows;
			}
		}
	}
	// check parseval's theorem
	#ifdef DEBUG_EPSIOBP
		float spectrum_integral = 0, variance, parseval_check;
		// detrend the input data
		detrend_f(data, size*num_blocks, parseval_check_detrended_data);
		// compute data variance
		arm_power_f32(parseval_check_detrended_data, size*num_blocks, &variance);
		variance = variance/(size*num_blocks);
		// compute the integral of the spectrum
		for (j = 0; j < size/2; j++) {
			spectrum_integral += spectrum[j]*df;
		}
		// compare the values (ratio should be one)
		parseval_check = spectrum_integral/variance;
	#endif
}

float sinc_f(float x)
/*******************************************************************************
 * @brief
 *   function to calculate sinc for a given x
 * @param x
 *   input - sinc argument
 * @return
 * 	 value for sinc(x)
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
//Inputs: 	salinity in ppt
//			temperature in degC
//Outputs: 	specific heat in J/(kg*K)
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
//Inputs: 	salinity in ppt
//			temperature in degC
//			pressure in dbar
//Outputs: 	density in kg/m^3
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
//Inputs:	salinity in ppt
//			temperature in degC
//			pressure in dbar
//Outputs: 	thermal conductivity in W/(m*K)
{
	float S = salinity/1000, P = pressure/100, T = temperature;
	static float k;
	static const float k0 = 0.001365, k1 = 418.55, k2 = 0.003, k3 = -1.025e-5, k4 = 6.53e-4, k5 = -0.29;
	k = k0*k1*(1.0 + k2*T + k3*pow(T, 2) + k4*P + k5*S);
	return k;
}

float seawater_thermal_diffusivity_f(float salinity, float temperature, float pressure)
// Inputs: 	salinity in ppt
//			temperature in degC
//			pressure in dbar
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
// Inputs:	salinity in ppt
//			temperature in degC
//			pressure in dbar
// Output:	kinematic viscosity in m^2/s
{
	float S = salinity/1000, T = temperature;
	static float nu, mu, rho;
	static const float mu0 = 1.779e-3, mu1 = 5.9319e-5, mu2 = 1.2917e-6, mu3 = 1.3402e-8, mu4 = 2.8782e-3, mu5 = 3.0553e-6, mu6 = 1.1835e-6;
	mu = mu0 - T*(mu1 - T*(mu2 - T*mu3)) + S*(mu4 - T*(mu5 + T*mu6));
	rho = seawater_density_f(salinity, temperature, pressure);
	nu = mu/rho;
	return nu;
}

/*----------------------------- Test Harness -------------------------------*/
/*------------------------------- Footnotes --------------------------------*/
/*------------------------------ End of file -------------------------------*/

