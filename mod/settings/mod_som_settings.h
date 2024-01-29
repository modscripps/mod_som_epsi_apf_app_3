/*******************************************************************************
 * @file mod_som_settings.h
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

#ifndef MOD_SOM_SETTINGS_H_
#define MOD_SOM_SETTINGS_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>

#if defined(MOD_SOM_CALENDAR_EN)
#include "mod_som_calendar.h"
#endif
#if defined(MOD_SOM_EFE_EN)
#include "mod_som_efe.h"
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
#include "mod_som_efe_obp.h"
#endif
#if defined(MOD_SOM_APF_EN)
#include "mod_som_apf.h"
#endif
#if defined(MOD_SOM_SBE49_EN)
#include "mod_som_sbe49.h"
#endif
#if defined(MOD_SOM_VOLTAGE_EN)
#include "mod_som_voltage.h"
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
#include "mod_som_actuator.h"
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
#include "mod_som_altimeter.h"
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
#include "mod_som_vec_nav.h"
#endif
#if defined(MOD_SOM_SBE41_EN)
#include "mod_som_sbe41.h"
#endif
#if defined(MOD_SOM_SDIO_EN)
#include "mod_som_sdio.h"
#endif


//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_SETTINGS_STATUS_PREFIX 99U

#define MOD_SOM_SETTINGS_STATUS_FAIL_INIT_CMD 0x2U
#define MOD_SOM_SETTINGS_STATUS_FAIL_WRITING_USERPAGE_CMD 0x3U

#define MOD_SOM_SETTINGS_DEFAULT_HEADER         "SOM3"
#define MOD_SOM_SETTINGS_DEFAULT_MISSION_NAME   "dev"
#define MOD_SOM_SETTINGS_DEFAULT_VEHICLE_NAME   "apex"
#define MOD_SOM_SETTINGS_DEFAULT_CTL_REV_NAME   "rev3"
#define MOD_SOM_SETTINGS_DEFAULT_CTL_SN         "001"
#define MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_NAME  PROJECTNAME
#define MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_GITID "a545880"

#define MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH       8
#define MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH      24
#define MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH  40
#define MOD_SOM_SETTINGS_HEADER_CHECKSUM_LENGTH   3
#define MOD_SOM_SETTINGS_PAYLOAD_CHECKSUM_LENGTH  5
//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------
typedef struct{
	uint32_t size;
  char     header[MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH];
	char     mission_name[MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH];
	char     vehicle_name[MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH];
	char     firmware[MOD_SOM_SETTINGS_DEFAULT_FIRMWARE_LENGTH];
	char     gitid[MOD_SOM_SETTINGS_DEFAULT_NAME_LENGTH];
	char     rev[MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH];
	char     sn[MOD_SOM_SETTINGS_DEFAULT_STR_LENGTH];
	uint32_t   initialized_flag;

#if defined(MOD_SOM_CALENDAR_EN)
mod_som_calendar_settings_t mod_som_calendar_settings;
#endif
#if defined(MOD_SOM_EFE_EN)
mod_som_efe_settings_t mod_som_efe_settings;
#endif
#if defined(MOD_SOM_SBE49_EN)
mod_som_sbe49_settings_t mod_som_sbe49_settings;
#endif
#if defined(MOD_SOM_SBE41_EN)
mod_som_sbe41_settings_t mod_som_sbe41_settings;
#endif
#if defined(MOD_SOM_SDIO_EN)
mod_som_sdio_settings_t mod_som_sdio_settings;
#endif
#if defined(MOD_SOM_APF_EN)
mod_som_apf_settings_t mod_som_apf_settings;
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
mod_som_efe_obp_settings_t mod_som_efe_obp_settings;
#endif

#if defined(MOD_SOM_VOLTAGE_EN)
mod_som_voltage_settings_t mod_som_voltage_settings;
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
mod_som_actuator_settings_t mod_som_actuator_settings;
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
mod_som_altimeter_settings_t mod_som_altimeter_settings;
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
mod_som_vecnav_settings_t mod_som_vec_nav_settings;
#endif

}mod_som_settings_struct_t,*mod_som_settings_struct_ptr_t;



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
mod_som_status_t mod_som_settings_init_f();

/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_settings_say_hello_world_f();

mod_som_status_t mod_som_settings_clear_settings_f();
mod_som_status_t mod_som_settings_save_settings_f();
mod_som_status_t mod_som_settings_sd_settings_f();
mod_som_status_t mod_som_settings_stream_settings_f();

void mod_som_settings_id_f(CPU_INT16U argc,CPU_CHAR *argv[]);
void mod_som_settings_mission_f(CPU_INT16U argc,CPU_CHAR *argv[]);


mod_som_settings_struct_ptr_t mod_som_settings_get_settings_f();
/*******************************************************************************
 * @function
 *     mod_som_settings_encode_status_f
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
mod_som_status_t mod_som_settings_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_settings_decode_status_f(mod_som_status_t mod_som_status);
#endif /* MOD_SOM_SETTINGS_H_ */
