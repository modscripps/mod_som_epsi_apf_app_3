/*******************************************************************************
 * @file mod_som_sbe49.h
 * @brief MOD SOM SBE41 API/header
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 * * @date Jan 27, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API/header file defines configuration, run time data handles, and
 * function placeholders to be used with an SBE 49 device connecting to the
 * MOD SOM board.
 * The ports definition are established in mod_bsp_cfg.h file.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_SBE41_CFG_H_
#define MOD_SOM_SBE41_CFG_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_bsp.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
// TODO example of the output and file format
// TODO Pictogram
#define MOD_SOM_SBE41_BAUDRATE                      MOD_SOM_SBE41_RX_DEFAULT_BAUD_RATE
#define MOD_SOM_SBE41_DATA_SAMPLE_DATA_LENGTH       MOD_SOM_SBE41_DATA_DEFAULT_SAMPLE_DATA_LENGTH
#define MOD_SOM_SBE41_DATA_SAMPLES_PER_RECORD       MOD_SOM_SBE41_DATA_DEFAULT_SAMPLES_PER_RECORD
#define MOD_SOM_SBE41_DATA_SAMPLES_PER_BUFFER       MOD_SOM_SBE41_DATA_DEFAULT_SAMPLES_PER_BUFFER
#define MOD_SOM_SBE41_DATA_HEADER_TEXT              MOD_SOM_SBE41_DEFAULT_DATA_HEADER_TEXT
#define MOD_SOM_SBE41_DATA_HEADER_TEXT_LENGTH       MOD_SOM_SBE41_DEFAULT_DATA_HEADER_TEXT_LENGTH
#define MOD_SOM_SBE41_STAT_HEADER_TEXT              MOD_SOM_SBE41_DEFAULT_STAT_HEADER_TEXT
#define MOD_SOM_SBE41_STAT_HEADER_TEXT_LENGTH       MOD_SOM_SBE41_DEFAULT_STAT_HEADER_TEXT_LENGTH
#define MOD_SOM_SBE41_CMD_HEADER_TEXT               MOD_SOM_SBE41_DEFAULT_CMD_HEADER_TEXT
#define MOD_SOM_SBE41_CMD_HEADER_TEXT_LENGTH        MOD_SOM_SBE41_DEFAULT_CMD_HEADER_TEXT_LENGTH
#define MOD_SOM_SBE41_DIRECT_EXIT_CMD               MOD_SOM_SBE41_DEFAULT_DIRECT_EXIT_CMD
#define MOD_SOM_SBE41_DIRECT_EXIT_CMD_LENGTH        MOD_SOM_SBE41_DEFAULT_DIRECT_EXIT_CMD_LENGTH

#define MOD_SOM_SBE41_SYNC_HEADER_LENGTH     1
#define MOD_SOM_SBE41_TAG_LENGTH             4
#define MOD_SOM_SBE41_HEXTIMESTAMP_LENGTH    16
#define MOD_SOM_SBE41_HEXPAYLOAD_LENGTH      8
#define MOD_SOM_SBE41_HEADER_CHECKSUM_LENGTH 3
#define MOD_SOM_SBE41_SAMPLE_LASTCHAR '\n'

#endif /* MOD_SOM_SBE41_CFG_H_ */
