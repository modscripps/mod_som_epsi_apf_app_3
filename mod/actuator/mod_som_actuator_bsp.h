/*
 * mod_som_actuator_bsp.h
 *
 *  Created on: Jan 4, 2021
 */

#ifndef MOD_SOM_ACTUATOR_BSP_H_
#define MOD_SOM_ACTUATOR_BSP_H_

/* Write here the GG11 hardware connection required to enable/run the module */

/* e.g MOD SOM bsp convention Table 1 and Table 2

#define MOD_SOM_SBE49_USART     LEUART0

Actuator #1 timer WTIM3_CC1 #3, GPIO PF10
Actuator #1 Power Enable ACT1_EN, GPIO PF5

Actuator #2 Timer WTIM3_CC2 #3, GPIO PF11
Actuator #2 Power Enable ACT2_EN, GPIO PF12

*/

//G11 Features

//pin definition
#define MOD_SOM_ACTUATOR2_TIMER    WTIMER3
#define MOD_SOM_ACTUATOR2_CLK      cmuClock_WTIMER3
#define MOD_SOM_ACTUATOR2_CC_PORT  gpioPortF
#define MOD_SOM_ACTUATOR2_CC_PIN   10
#define MOD_SOM_ACTUATOR2_ROUTE              3
#define MOD_SOM_ACTUATOR2_CC_CHANNEL         1
#define MOD_SOM_ACTUATOR2_EN_PORT  gpioportF
#define MOD_SOM_ACTUATOR2_EN_PIN   12 //5

#define MOD_SOM_ACTUATOR1_TIMER    WTIMER3
#define MOD_SOM_ACTUATOR1_CLK      cmuClock_WTIMER3
#define MOD_SOM_ACTUATOR1_CC_PORT  gpioPortF
#define MOD_SOM_ACTUATOR1_CC_PIN   11
#define MOD_SOM_ACTUATOR1_ROUTE              3
#define MOD_SOM_ACTUATOR1_CC_CHANNEL         2
#define MOD_SOM_ACTUATOR1_EN_PORT  gpioPortF
#define MOD_SOM_ACTUATOR1_EN_PIN   5 //12





#endif /* MOD_SOM_ACTUATOR_BSP_H_ */
