/*******************************************************************************
 * @file mod_som_vec_nav.h
 * @brief MOD SOM VECNAV API/header
 *  * @date Mar 12, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
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

#ifndef MOD_SOM_VECNAV_PRIV_H_
#define MOD_SOM_VECNAV_PRIV_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
void mod_som_vecnav_irq_rx_handler_f();
void mod_som_vecnav_irq_tx_handler_f();
void mod_som_vecnav_ldma_irq_handler_f();
/*******************************************************************************
 * @brief
 *   Stream data to the main com port
 ******************************************************************************/
void mod_som_vecnav_stream_data_f();
uint16_t mod_som_vecnav_decode_status_f(mod_som_status_t status);
mod_som_status_t mod_som_vecnav_encode_status_f(uint16_t status);
#endif /* MOD_SOM_VECNAV_PRIV_H_ */
