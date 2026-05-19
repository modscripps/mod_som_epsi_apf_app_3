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

//#define MOD_SOM_DEBUG
#define MOD_SOM_BOARD // this definition is to make sure you are using the som board

#define MOD_SOM_IO_TASK_PRIORITY 16u
#define MOD_SOM_CFG_LOOP_TICK_DELAY 1u

//2025 06 14 added error accumulation for the purpose of restarting tasks
#define MOD_SOM_MAX_ERROR_CNT 5

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

// MOD SOM High frequency oscillator enable
#define MOD_SOM_HFXO_EN_PORT                    MOD_SOM_U8_3_GPIO_PORT
#define MOD_SOM_HFXO_EN_PIN                     MOD_SOM_U8_3_GPIO_PIN

//// MOD SOM UART Transceiver Power Enable
//#define MOD_SOM_MEZZANINE_UART_EN_PORT          MOD_SOM_J9_28_PORT
//#define MOD_SOM_MEZZANINE_UART_EN_PIN           MOD_SOM_J9_28_PIN
//
//#define MOD_SOM_MEZZANINE_UART_VCC_EN_PORT      MOD_SOM_J10_12_PORT
//#define MOD_SOM_MEZZANINE_UART_VCC_EN_PIN       MOD_SOM_J10_12_PIN


//ALB EFE module
#define MOD_SOM_EFE_EN
#define MOD_SOM_EFE_OBP_EN
#define MOD_SOM_SBE41_EN
#define MOD_SOM_APF_EN

//#define MOD_SOM_EFE_REV3
#define MOD_SOM_EFE_REV4

//MHA klugy compile flag for FCTD
//#define MOD_SOM_FCTD_EN

#define MOD_SOM_SDIO_EN
#define MOD_SOM_CALENDAR_EN
#define MOD_SOM_SETTINGS_EN
#define MOD_SOM_VOLTAGE_EN



#endif /* MOD_MOD_CFG_MOD_BSP_CFG_H_ */
