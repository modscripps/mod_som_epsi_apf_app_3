/*******************************************************************************
 * @file mod_som_apf_bsp.h
 * @brief MOD SOM APF API/header
 * @date Nov 2, 2021
 * @author  (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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


#ifndef MOD_APF_MOD_SOM_APF_BSP_H_
#define MOD_APF_MOD_SOM_APF_BSP_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_bsp.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_APF_USART      LEUART0
#define MOD_SOM_APF_USART_IRQ  LEUART0_IRQn
#define MOD_SOM_APF_USART_CLK  cmuClock_LEUART0

#define MOD_SOM_APF_TX_LOC    _USART_ROUTELOC0_TXLOC_LOC0
#define MOD_SOM_APF_TX_PORT   MOD_SOM_J1_10_LEU0_TX_L0_PORT
#define MOD_SOM_APF_TX_PIN    MOD_SOM_J1_10_LEU0_TX_L0_PIN

#define MOD_SOM_APF_RX_LOC    _USART_ROUTELOC0_RXLOC_LOC0
#define MOD_SOM_APF_RX_PORT   MOD_SOM_J1_9_LEU0_RX_L0_PORT
#define MOD_SOM_APF_RX_PIN    MOD_SOM_J1_9_LEU0_RX_L0_PIN

#define MOD_SOM_APF_EN_PORT   MOD_SOM_U20_16_GPIO_PORT
#define MOD_SOM_APF_EN_PIN    MOD_SOM_U20_16_GPIO_PIN

#define MOD_SOM_APF_RX_DEFAULT_BAUD_RATE 9600
//#define MOD_SOM_APF_RX_DEFAULT_BAUD_RATE 115200 // Arnaud: change to 115200 - Nov 10, 2021
#define MOD_SOM_APF_DEFAULT_SN "0000"

//#define MOD_SOM_APF_LDMA_IRQ  LDMA_IRQn

#endif /* MOD_APF_MOD_SOM_APF_BSP_H_ */
