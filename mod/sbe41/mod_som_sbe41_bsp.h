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

#ifndef MOD_SOM_SBE41_BSP_H_
#define MOD_SOM_SBE41_BSP_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_bsp.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_SBE41_USART      USART4
#define MOD_SOM_SBE41_USART_IRQ  USART4_RX_IRQn
#define MOD_SOM_SBE41_USART_CLK  cmuClock_USART4

#define MOD_SOM_SBE41_TX_LOC    _USART_ROUTELOC0_TXLOC_LOC1
#define MOD_SOM_SBE41_TX_PORT   MOD_SOM_J2_4_US4_TX_L1_PORT
#define MOD_SOM_SBE41_TX_PIN    MOD_SOM_J2_4_US4_TX_L1_PIN

#define MOD_SOM_SBE41_RX_LOC    _USART_ROUTELOC0_RXLOC_LOC1
#define MOD_SOM_SBE41_RX_PORT   MOD_SOM_J2_3_US4_RX_L1_PORT
#define MOD_SOM_SBE41_RX_PIN    MOD_SOM_J2_3_US4_RX_L1_PIN

#define MOD_SOM_SBE41_EN_PORT   MOD_SOM_U16_3_GPIO_PORT
#define MOD_SOM_SBE41_EN_PIN    MOD_SOM_U16_3_GPIO_PIN

#define MOD_SOM_SBE41_RX_DEFAULT_BAUD_RATE 9600
#define MOD_SOM_SBE41_DEFAULT_SN "2128"

#define MOD_SOM_SBE41_LDMA_IRQ  LDMA_IRQn

#endif /* MOD_SOM_SBE41_BSP_H_ */
