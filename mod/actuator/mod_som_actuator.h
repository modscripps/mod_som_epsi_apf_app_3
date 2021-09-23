/*******************************************************************************
 * @file mod_som_actuator.h
 * @brief MOD SOM actuator API
 * @date January 4th 2021
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

#ifndef MOD_SOM_ACTUATOR_H_
#define MOD_SOM_ACTUATOR_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_ACTUATOR_STATUS_PREFIX 99U
#define MOD_SOM_ACTUATOR_STATUS_FAIL_INIT_CMD 0x2U

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_ACTUATOR_STATUS_PREFIX 99U

#define MOD_SOM_ACTUATOR_STATUS_FAIL_INIT_CMD   0x2U
#define MOD_SOM_ACTUATOR_CANNOT_ALLOCATE_SETUP  0x3u
#define MOD_SOM_ACTUATOR_CANNOT_ALLOCATE_CONFIG 0x4u

#define MOD_SOM_ACTUATOR_MAX_HEADER_SIZE           100
#define MOD_SOM_ACTUATOR_HEADER                    "$ACT"
#define MOD_SOM_ACTUATOR_SETTINGS_STR_LENGTH       8
#define MOD_SOM_ACTUATOR_DEFAULT_REV               "000"
#define MOD_SOM_ACTUATOR_DEFAULT_SN                "000"
#define MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE       50000    //1sec=50e3 cycle
#define MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE      100000   //2sec=100e3 cycle
#define MOD_SOM_ACTUATOR_PTC_EXTENT                100      //
#define MOD_SOM_ACTUATOR_CTC_EXTENT                0        //
//#define MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE       50000  //for debug (shorter pulse)
//#define MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE      100000 //for debug(shorter pulse)
#define MOD_SOM_ACTUATOR_CALC_EXTENT(x)  \
                                       (MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE + \
                                     x*(MOD_SOM_ACTUATOR_DEFAULT_ALLOUT_PULSE-\
                                        MOD_SOM_ACTUATOR_DEFAULT_ALLIN_PULSE)/100)

#define MOD_SOM_ACTUATOR_MSG_LENGTH 26
#define MOD_SOM_ACTUATOR_CHCKSUM_LENGTH 5
#define MOD_SOM_ACTUATOR_MAX_NB_PULSE 81

//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

//structure runtime
//structure compile
//structure settings

/*******************************************************************************
 * @brief
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    mod_som_timer_handle_ptr_t timer_ptr;


}mod_som_actuator_config_t, *mod_som_actuator_config_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM EFE
 *   settings structure should ONLY be words or multiple of words
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char header[MOD_SOM_ACTUATOR_SETTINGS_STR_LENGTH];
    char rev[MOD_SOM_ACTUATOR_SETTINGS_STR_LENGTH];
    char sn[MOD_SOM_ACTUATOR_SETTINGS_STR_LENGTH];
    uint32_t initialize_flag;
}
mod_som_actuator_settings_t, *mod_som_actuator_settings_ptr_t;

/*******************************************************************************
 * @brief
 *   Run-time structure for MOD SOM ACTUATOR
 *
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    uint32_t error_flag;
    uint64_t timestamp;
    uint32_t extent;
    uint32_t percent_extent;
    uint32_t ctc_extent;
    uint32_t ptc_extent;
    uint32_t nb_pulse;
    char     msg[100];
    uint8_t  chksum;
    uint8_t  mode;
    enum {stop,cycle,end_cycle} actuator_state;
    enum {allin,allout,interm} actuator_position;
    mod_som_actuator_settings_ptr_t settings_ptr;      //
    mod_som_actuator_config_ptr_t config_ptr;    //

}mod_som_actuator_t,*mod_som_actuator_ptr_t;


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
mod_som_status_t mod_som_actuator_init_f();

mod_som_status_t mod_som_actuator_allocate_config_f();
mod_som_status_t mod_som_actuator_config_f(mod_som_actuator_config_ptr_t config_ptr);
mod_som_status_t mod_som_actuator_allocate_settings_f();
mod_som_status_t mod_som_actuator_default_settings_f(mod_som_actuator_settings_ptr_t settings_ptr);
mod_som_actuator_settings_t mod_som_actuator_get_settings_f();
mod_som_status_t mod_som_actuator_init_timer_f();
/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_actuator_hello_world_f();

mod_som_status_t mod_som_actuator_toggle_f();
mod_som_status_t mod_som_actuator_ptc_f();
mod_som_status_t mod_som_actuator_ctc_f();
mod_som_status_t mod_som_actuator_set_f(CPU_INT16U argc,CPU_CHAR *argv[]);

void mod_som_actuator_message_f();

mod_som_status_t mod_som_actuator_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]);
/*******************************************************************************
 * @function
 *     mod_som_actuator_encode_status_f
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
mod_som_status_t mod_som_actuator_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_actuator_decode_status_f(mod_som_status_t mod_som_status);
#endif /* MOD_SOM_ACTUATOR_H_ */
