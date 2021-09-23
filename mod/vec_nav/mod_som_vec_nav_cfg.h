/*******************************************************************************
 * @file mod_som_vecnav.h
 * @brief MOD SOM VECNAV API/header
 * * @date Mar 11, 2021
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

#ifndef MOD_SOM_VECNAV_CFG_H_
#define MOD_SOM_VECNAV_CFG_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_bsp.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
// TODO example of the output and file format
// TODO Pictogram
#define MOD_SOM_VECNAV_BAUDRATE                      MOD_SOM_VECNAV_RX_DEFAULT_BAUD_RATE
#define MOD_SOM_VECNAV_DATA_SAMPLE_DATA_LENGTH       MOD_SOM_VECNAV_DATA_DEFAULT_SAMPLE_DATA_LENGTH
#define MOD_SOM_VECNAV_DATA_SAMPLES_PER_RECORD       MOD_SOM_VECNAV_DATA_DEFAULT_SAMPLES_PER_RECORD
#define MOD_SOM_VECNAV_DATA_SAMPLES_PER_BUFFER       MOD_SOM_VECNAV_DATA_DEFAULT_SAMPLES_PER_BUFFER
#define MOD_SOM_VECNAV_DATA_HEADER_TEXT              MOD_SOM_VECNAV_DEFAULT_DATA_HEADER_TEXT
#define MOD_SOM_VECNAV_DATA_HEADER_TEXT_LENGTH       MOD_SOM_VECNAV_DEFAULT_DATA_HEADER_TEXT_LENGTH
#define MOD_SOM_VECNAV_STAT_HEADER_TEXT              MOD_SOM_VECNAV_DEFAULT_STAT_HEADER_TEXT
#define MOD_SOM_VECNAV_STAT_HEADER_TEXT_LENGTH       MOD_SOM_VECNAV_DEFAULT_STAT_HEADER_TEXT_LENGTH
#define MOD_SOM_VECNAV_CMD_HEADER_TEXT               MOD_SOM_VECNAV_DEFAULT_CMD_HEADER_TEXT
#define MOD_SOM_VECNAV_CMD_HEADER_TEXT_LENGTH        MOD_SOM_VECNAV_DEFAULT_CMD_HEADER_TEXT_LENGTH
#define MOD_SOM_VECNAV_DIRECT_EXIT_CMD               MOD_SOM_VECNAV_DEFAULT_DIRECT_EXIT_CMD
#define MOD_SOM_VECNAV_DIRECT_EXIT_CMD_LENGTH        MOD_SOM_VECNAV_DEFAULT_DIRECT_EXIT_CMD_LENGTH


#define MOD_SOM_VECNAV_SAMPLE_LASTCHAR '\n'
#define MOD_SOM_VECNAV_SAMPLE_FIRSTCHAR '$'

#endif /* MOD_SOM_VECNAV_CFG_H_ */
