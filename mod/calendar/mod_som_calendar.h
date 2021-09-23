/*******************************************************************************
 * @file mod_som_foo_bar.h
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

#ifndef MOD_SOM_FOO_BAR_H_
#define MOD_SOM_FOO_BAR_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include <time.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_CALENDAR_STATUS_PREFIX 99U
#define MOD_SOM_CALENDAR_MAXIMUM_LENGTH_HEADER 16U
#define MOD_SOM_CALENDAR_STATUS_FAIL_INIT_CMD 0x2U
//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

typedef struct{
	uint32_t size;
	char header[MOD_SOM_CALENDAR_MAXIMUM_LENGTH_HEADER]; //maximum
    sl_sleeptimer_date_t initial_date; //Watch for the settings offsets. the size here is not a multiple of words.
    uint64_t poweron_offset_ms; //MHA: 1000 times unix time of powerup; computed in time.set
    uint32_t initialize_flag;
}mod_som_calendar_settings_t,*mod_som_calendar_settings_ptr_t;

typedef struct{
bool initialized_flag;
bool started_flag;
mod_som_calendar_settings_ptr_t  mod_som_calendar_settings_ptr;
sl_sleeptimer_date_t current_date;
char buff[20];
}mod_som_calendar_t,*mod_som_calendar_ptr_t;

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
mod_som_status_t mod_som_calendar_init_f();
mod_som_status_t mod_som_calendar_default_settings_f();
mod_som_status_t mod_som_calendar_settings_f();
mod_som_calendar_settings_t mod_som_calendar_get_settings_f();
void mod_som_calendar_set_settings_f();
/*******************************************************************************
 * @brief
 *   get the Timestamp, wall clock time in seconds
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
sl_sleeptimer_timestamp_t mod_som_calendar_get_time_f();

/*******************************************************************************
 * @brief
 *   get the wall clock Timestamp, in char * format is "%Y%m%d_%H%M%S"
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
char * mod_som_calendar_get_datetime_f();

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
sl_status_t mod_som_calendar_set_time_f(uint16_t year, uint8_t month, uint8_t month_day, uint8_t hour, \
		                         uint8_t min, uint8_t sec, uint8_t tz_offset);


/*******************************************************************************
 * @function
 *     mod_som_foo_bar_encode_status_f
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
mod_som_status_t mod_som_calendar_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_calendar_decode_status_f(mod_som_status_t mod_som_status);
#endif /* MOD_SOM_FOO_BAR_H_ */
