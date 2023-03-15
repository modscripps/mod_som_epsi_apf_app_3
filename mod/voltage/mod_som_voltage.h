/*******************************************************************************
 * @file mod_som_voltage.h
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

#ifndef MOD_SOM_VOLTAGE_H_
#define MOD_SOM_VOLTAGE_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include "em_adc.h"

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_VOLTAGE_STATUS_PREFIX          99U
#define MOD_SOM_VOLTAGE_STATUS_FAIL_INIT_CMD   0x2U
#define MOD_SOM_VOLTAGE_CANNOT_ALLOCATE_SETUP  0x3u
#define MOD_SOM_VOLTAGE_CANNOT_ALLOCATE_CONFIG 0x4u
#define MOD_SOM_VOLTAGE_HEADER                 "VOLT"
#define MOD_SOM_VOLTAGE_HEADER_LENGTH          8
#define MOD_SOM_VOLTAGE_RECORD_LENGTH          34   //$VOLT+16+8+5=34
#define MOD_SOM_VOLTAGE_CHCKSUM_LENGTH         5    // *FF\r\n
#define MOD_SOM_VOLTAGE_DIVIDER                20


#define MOD_SOM_VOLTAGE_SCAN_STK_SIZE          512u
#define MOD_SOM_VOLTAGE_SCAN_TASK_PRIO         19u
#define MOD_SOM_VOLTAGE_SCAN_TASK_STK_SIZE     512u
#define MOD_SOM_VOLTAGE_TASK_DELAY             500

#define MOD_SOM_VOLTAGE_STATUS_FAIL_TO_START_STORE_TASK 0x03u

//ALB from
// Init to max ADC clock for Series 1

#define NUM_INPUTS  1

uint32_t inputs[NUM_INPUTS];


//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   voltage module setting
 *
 *
 ******************************************************************************/
typedef struct{
  uint32_t size;
  char header[MOD_SOM_VOLTAGE_HEADER_LENGTH];
  uint32_t sampling_frequency;
  uint32_t initialize_flag;

}mod_som_voltage_settings_t,*mod_som_voltage_settings_ptr_t;

/*******************************************************************************
 * @brief
 *   voltage module config
 *
 *
 ******************************************************************************/
typedef struct{
  ADC_Init_TypeDef init;
  ADC_InitScan_TypeDef initScan;

}mod_som_voltage_config_t,*mod_som_voltage_config_ptr_t;

/*******************************************************************************
 * @brief
 *   Run-time structure for MOD SOM ALTIMETER
 *   the altimeter runs a timer in a task.
 *   the repetition time
 *
 *
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    uint32_t error_flag;
    uint64_t timestamp;
    uint64_t timestamp_adc1;
    char     str_record[MOD_SOM_VOLTAGE_RECORD_LENGTH]; //char VOLThex_timestmaps,voltage*checksum\r\n
    uint32_t voltage;
    uint32_t voltage_adc1;
    uint8_t  mode;   //0 streaming, 1 store;
    uint8_t  status;
    uint8_t  chksum;
    mod_som_voltage_settings_ptr_t settings_ptr;

}mod_som_voltage_t,*mod_som_voltage_ptr_t;

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, if shell is available, then the command table is added
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_voltage_init_f();

/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_voltage_say_hello_world_f();

/*******************************************************************************
 * @function
 *     mod_som_voltage_encode_status_f
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
mod_som_status_t mod_som_voltage_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_voltage_decode_status_f(mod_som_status_t mod_som_status);


mod_som_status_t mod_som_voltage_default_settings_f(mod_som_voltage_settings_ptr_t settings_ptr);
mod_som_voltage_settings_t mod_som_voltage_get_settings_f();
/*******************************************************************************
 * @brief
 *   get the voltage runtime ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_voltage_ptr_t mod_som_voltage_get_runtime_ptr_f();

mod_som_status_t mod_som_voltage_start_scan_task_f(void);
mod_som_status_t mod_som_voltage_start_adc1_scan_task_f();
mod_som_status_t mod_som_voltage_stop_scan_task_f(void);
mod_som_status_t mod_som_voltage_stop_adc1_scan_task_f();
void mod_som_voltage_scan_task_f(void  *p_arg);
void mod_som_voltage_adc1_scan_task_f(void  *p_arg);
mod_som_status_t mod_som_voltage_scan_f(void);
mod_som_status_t mod_som_voltage_adc1_scan_f(void);
mod_som_status_t mod_som_voltage_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]);

#endif /* MOD_SOM_VOLTAGE_H_ */
