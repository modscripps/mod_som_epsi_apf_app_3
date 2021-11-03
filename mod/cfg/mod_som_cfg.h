/*******************************************************************************
 * @file mod_som_bsp_cfg.h
 * @brief MOD SOM board configuration per application
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * The board configuration should be based on the pins defined in  mod_som_bsp.h
 *
 ******************************************************************************/

#ifndef MOD_BSP_CFG_H_
#define MOD_BSP_CFG_H_

#include <mod_som_bsp.h>
#include <mod_som_common.h>
#include "em_cmu.h"

#define MOD_SOM_BOARD // this definition is to make sure you are using the som board
//#define MOD_SOM_MEZZANINE_BOARD // this definition is to make sure you are using the som board
//#define RETARGET_VCOM

//MHA define a compile flag that enables the vecnav power (USART SPARE) upon efe init to turn the fluorometer on. 6/27/2021
//#define STANDALONE_FLUOROMETER

#define MOD_SOM_IO_TASK_PRIORITY 16u
#define MOD_SOM_CFG_LOOP_TICK_DELAY 1u

#if defined(MOD_SOM_BOARD)

// MOD SOM High frequency oscillator enable
#define MOD_SOM_HFXO_EN_PORT                    MOD_SOM_U8_3_GPIO_PORT
#define MOD_SOM_HFXO_EN_PIN                     MOD_SOM_U8_3_GPIO_PIN

// MOD SOM UART Transceiver Power Enable
#define MOD_SOM_UART_EN_PORT                    MOD_SOM_U11_6_GPIO_PORT
#define MOD_SOM_UART_EN_PIN                     MOD_SOM_U11_6_GPIO_PIN

// MOD SOM Serial communication                  system config
#define MOD_SOM_MAIN_COM_USART                  USART5
#define MOD_SOM_MAIN_COM_CLK                    cmuClock_USART5
#define MOD_SOM_MAIN_COM_IRQn                   USART5_RX_IRQn
#define MOD_SOM_MAIN_COM_IRQ_HANDLER_NAME       USART5_RX_IRQHandler

#define MOD_SOM_MAIN_COM_RX_PORT                MOD_SOM_J1_4_US5_RX_L0_PORT
#define MOD_SOM_MAIN_COM_RX_PIN                 MOD_SOM_J1_4_US5_RX_L0_PIN
#define MOD_SOM_MAIN_COM_RX_LOC                 _USART_ROUTELOC0_RXLOC_LOC0

#define MOD_SOM_MAIN_COM_TX_PORT                MOD_SOM_J1_3_US5_TX_L0_PORT
#define MOD_SOM_MAIN_COM_TX_PIN                 MOD_SOM_J1_3_US5_TX_L0_PIN
#define MOD_SOM_MAIN_COM_TX_LOC                 _USART_ROUTELOC0_TXLOC_LOC0

#define MOD_SOM_MAIN_COM_BAUDRATE               230400

#define MOD_SOM_MAIN_COM_EN_PORT                MOD_SOM_J10_27_GPIO_PORT
#define MOD_SOM_MAIN_COM_EN_PIN                 MOD_SOM_J10_27_GPIO_PIN
#endif


#if defined(MOD_SOM_MEZZANINE_BOARD)
// MOD SOM High frequency oscillator enable
#define MOD_SOM_HFXO_EN_PORT                    MOD_SOM_U8_3_GPIO_PORT
#define MOD_SOM_HFXO_EN_PIN                     MOD_SOM_U8_3_GPIO_PIN

// MOD SOM UART Transceiver Power Enable
#define MOD_SOM_MEZZANINE_UART_EN_PORT          gpioPortC
#define MOD_SOM_MEZZANINE_UART_EN_PIN           3

#define MOD_SOM_MEZZANINE_UART_VCC_EN_PORT      gpioPortB
#define MOD_SOM_MEZZANINE_UART_VCC_EN_PIN       13


// MOD SOM Serial communication                  system config
#define MOD_SOM_MEZZANINE_COM_USART                  UART1
#define MOD_SOM_MEZZANINE_COM_CLK                    cmuClock_UART1
#define MOD_SOM_MEZZANINE_COM_IRQn                   UART1_RX_IRQn
#define MOD_SOM_MEZZANINE_COM_IRQ_HANDLER_NAME       UART1_RX_IRQHandler

#define MOD_SOM_MEZZANINE_COM_RX_PORT                gpioPortE
#define MOD_SOM_MEZZANINE_COM_RX_PIN                 13
#define MOD_SOM_MEZZANINE_COM_RX_LOC                 _USART_ROUTELOC0_RXLOC_LOC4

#define MOD_SOM_MEZZANINE_COM_TX_PORT                gpioPortE
#define MOD_SOM_MEZZANINE_COM_TX_PIN                 12
#define MOD_SOM_MEZZANINE_COM_TX_LOC                 _USART_ROUTELOC0_RXLOC_LOC4


#define MOD_SOM_MEZZANINE_COM_RTS_PORT                gpioPortC
#define MOD_SOM_MEZZANINE_COM_RTS_PIN                 5
#define MOD_SOM_MEZZANINE_COM_RTS_LOC                 _USART_ROUTELOC0_RXLOC_LOC4


#define MOD_SOM_MEZZANINE_COM_CTS_PORT                gpioPortC
#define MOD_SOM_MEZZANINE_COM_CTS_PIN                 4
#define MOD_SOM_MEZZANINE_COM_CTS_LOC                 _USART_ROUTELOC0_RXLOC_LOC4


#define MOD_SOM_MEZZANINE_COM_BAUDRATE               230400

#define MOD_SOM_MEZZANINE_COM_EN_PORT                MOD_SOM_J10_27_GPIO_PORT
#define MOD_SOM_MEZZANINE_COM_EN_PIN                 MOD_SOM_J10_27_GPIO_PIN
#endif


#if defined(RETARGET_VCOM)

// MOD SOM High frequency oscillator enable
#define MOD_SOM_HFXO_EN_PORT                    MOD_SOM_U8_3_PORT
#define MOD_SOM_HFXO_EN_PIN                     MOD_SOM_U8_3_PIN

// MOD SOM UART Transceiver Power Enable
#define MOD_SOM_UART_EN_PORT                    MOD_SOM_U11_6_GPIO_PORT
#define MOD_SOM_UART_EN_PIN                     MOD_SOM_U11_6_GPIO_PIN

// MOD SOM Serial communication                  system config
#define MOD_SOM_MAIN_COM_USART                  USART4
#define MOD_SOM_MAIN_COM_CLK                    cmuClock_USART4
#define MOD_SOM_MAIN_COM_IRQn                   USART4_RX_IRQn
#define MOD_SOM_MAIN_COM_IRQ_HANDLER_NAME       USART4_RX_IRQHandler

#define MOD_SOM_MAIN_COM_RX_PORT                gpioPortH
#define MOD_SOM_MAIN_COM_RX_PIN                 5
#define MOD_SOM_MAIN_COM_RX_LOC                 _USART_ROUTELOC0_TXLOC_LOC4

#define MOD_SOM_MAIN_COM_TX_PORT                gpioPortH
#define MOD_SOM_MAIN_COM_TX_PIN                 4
#define MOD_SOM_MAIN_COM_TX_LOC                 _USART_ROUTELOC0_TXLOC_LOC4

#define MOD_SOM_MAIN_COM_BAUDRATE               115200

#define MOD_SOM_MAIN_COM_EN_PORT                gpioPortE
#define MOD_SOM_MAIN_COM_EN_PIN                 1
#endif


//ALB EFE module
#define MOD_SOM_EFE_EN
#define MOD_SOM_EFE_OBP_EN
#define MOD_SOM_SBE41_EN
//#define MOD_SOM_APF_EN
//#define MOD_SOM_EFE_REV3
#define MOD_SOM_EFE_REV4

//MHA klugy compile flag for FCTD
//#define MOD_SOM_FCTD_EN

#define MOD_SOM_SDIO_EN
#define MOD_SOM_CALENDAR_EN
#define MOD_SOM_SETTINGS_EN
#define MOD_SOM_VOLTAGE_EN
//#define MOD_SOM_ACTUATOR_EN
//#define MOD_SOM_ALTIMETER_EN
//#define MOD_SOM_VEC_NAV_EN
//#define MOD_SOM_AGGREGATOR_EN

#endif /* MOD_MOD_CFG_MOD_BSP_CFG_H_ */
