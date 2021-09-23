/*
 * mod_som_actuator_bsp.h
 *
 *  Created on: Jan 4, 2021
 *      Author: aleboyer
 */

#ifndef MOD_SOM_ALTIMETER_BSP_H_
#define MOD_SOM_ALTIMETER_BSP_H_

/* Write here the GG11 hardware connection required to enable/run the module */

/* e.g MOD SOM bsp convention Table 2

#define MOD_SOM_SBE49_USART     LEUART0

#define MOD_SOM_SBE49_TX_LOC    _USART_ROUTELOC0_TXLOC_LOC0
#define MOD_SOM_SBE49_TX_PORT   MOD_SOM_J1_10_LEU0_TX_L0_PORT
#define MOD_SOM_SBE49_TX_PIN    MOD_SOM_J1_10_LEU0_TX_L0_PIN

#define MOD_SOM_SBE49_RX_LOC    _USART_ROUTELOC0_RXLOC_LOC0
#define MOD_SOM_SBE49_RX_PORT   MOD_SOM_J1_9_LEU0_RX_L0_PORT
#define MOD_SOM_SBE49_RX_PIN    MOD_SOM_J1_9_LEU0_RX_L0_PIN

#define MOD_SOM_SBE49_EN_PORT   MOD_SOM_U20_16_GPIO_PORT
#define MOD_SOM_SBE49_EN_PIN    MOD_SOM_U20_16_GPIO_PIN
*/


//ALB Altimeter tx timer is WTIMER 2 route #0 using CC0
//ALB Altimeter echo     is WTIMER 2 route #0 using CC1

#define MOD_SOM_ALTIMETER_TX_CLK                cmuClock_WTIMER2
#define MOD_SOM_ALTIMETER_TX_PORT               gpioPortA
#define MOD_SOM_ALTIMETER_TX_PIN                9
#define MOD_SOM_ALTIMETER_TX_TIMER              WTIMER2
#define MOD_SOM_ALTIMETER_TX_CC                 20
#define MOD_SOM_ALTIMETER_TX_TOP                10
#define MOD_SOM_ALTIMETER_TX_ROUTE              0
#define MOD_SOM_ALTIMETER_TX_CC_CHANNEL         0

#define MOD_SOM_ALTIMETER_ECHO_CLK                cmuClock_WTIMER2
#define MOD_SOM_ALTIMETER_ECHO_PORT               gpioPortA
#define MOD_SOM_ALTIMETER_ECHO_PIN                10
#define MOD_SOM_ALTIMETER_ECHO_TIMER              WTIMER0
#define MOD_SOM_ALTIMETER_ECHO_CC                 20
#define MOD_SOM_ALTIMETER_ECHO_TOP                10
#define MOD_SOM_ALTIMETER_ECHO_ROUTE              0
#define MOD_SOM_ALTIMETER_ECHO_CC_CHANNEL         1


#endif /* MOD_SOM_ACTUATOR_BSP_H_ */
