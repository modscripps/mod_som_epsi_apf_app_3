/*
 * mod_som_efe_bsp.h
 *
 * Defines all the GG11 pins and hardware features (timers and LDMA)
 * used by the EFE.
 *
 *  Created on: Dec 8, 2020
 *      Author: aleboyer
 */

#ifndef MOD_EFE_MOD_SOM_EFE_BSP_H_
#define MOD_EFE_MOD_SOM_EFE_BSP_H_


// EFE connections

#define MOD_SOM_EFE_SPI_COM                 USART0
#define MOD_SOM_EFE_SPI_CLK                 cmuClock_USART0

//TODO Ask Sean to fill up the mod_som_bsp.h with the correct values
#define MOD_SOM_EFE_MOSI_PORT               gpioPortC
#define MOD_SOM_EFE_MOSI_PIN                11
#define MOD_SOM_EFE_MISO_PORT               gpioPortC
#define MOD_SOM_EFE_MISO_PIN                10
#define MOD_SOM_EFE_CLCK_PORT               gpioPortC
#define MOD_SOM_EFE_CLCK_PIN                9

#ifdef MOD_SOM_EFE_REV3
//MAG 7AUG2020 add address select table
#define slct_A0 0x4000;   //PA14 could use pins as defined in bsp table and shift into place here
#define slct_A1 0x2000;   //PA13
#define slct_A2 0x1000;   //PA12
#define slct_A3 0x80;    //PA7
#define CH1_ADDR  = 0x0
#define CH2_ADDR  = (slct_A0)
#define CH3_ADDR  = (slct_A1)
#define CH4_ADDR  = (slct_A1 | slct_A0)
#define CH5_ADDR  = slct_A2)
#define CH6_ADDR  = (slct_A2 | slct_A0)
#define CH7_ADDR  = (slct_A2 | slct_A1)
#define CH8_ADDR  = (slct_A2 | slct_A1 | slct_A0)
#define CH9_ADDR  = (slct_A3)
#define CH10_ADDR = (slct_A3 | slct_A0)
#define CH11_ADDR = (slct_A3 | slct_A1)
#define CH12_ADDR = (slct_A3 | slct_A1 | slct_A0)
#define CH13_ADDR = (slct_A3 | slct_A2)
#define CH14_ADDR = (slct_A3 | slct_A2 | slct_A0)
#define CH15_ADDR = (slct_A3 | slct_A2 | slct_A1)
#define CH16_ADDR = (slct_A3 | slct_A2 | slct_A1 | slct_A0)

#else
#define MOD_SOM_EFE_CS1_PORT                gpioPortA
#define MOD_SOM_EFE_CS1_PIN                 4
#define MOD_SOM_EFE_CS2_PORT                gpioPortA
#define MOD_SOM_EFE_CS2_PIN                 5
#define MOD_SOM_EFE_CS3_PORT                gpioPortA
#define MOD_SOM_EFE_CS3_PIN                 7
#define MOD_SOM_EFE_CS4_PORT                gpioPortA
#define MOD_SOM_EFE_CS4_PIN                 12
#define MOD_SOM_EFE_CS5_PORT                gpioPortA
#define MOD_SOM_EFE_CS5_PIN                 13
#define MOD_SOM_EFE_CS6_PORT                gpioPortA
#define MOD_SOM_EFE_CS6_PIN                 14
#define MOD_SOM_EFE_CS7_PORT                gpioPortA
#define MOD_SOM_EFE_CS7_PIN                 15
#endif


#define MOD_SOM_EFE_SPI_BAUDRATE            2000000

#define MOD_SOM_EFE_EN_PORT               MOD_SOM_SBE_EN_U20_16_PORT
#define MOD_SOM_EFE_EN_PIN                MOD_SOM_SBE_EN_U20_16_PIN

//ALB EFE-ADC defines
#define MOD_SOM_EFE_GPIO_IRQn               GPIO_ODD_IRQn
#define MOD_SOM_EFE_GPIO_IRQ_HANDLER_NAME   GPIO_ODD_IRQHandler
#define MOD_SOM_EFE_MCLOCK_PORT             gpioPortB
#define MOD_SOM_EFE_MCLOCK_PIN              2
#define MOD_SOM_EFE_MCLOCK_TIMER            WTIMER0
#define MOD_SOM_EFE_MCLOCK_CC_VALUE         39
#define MOD_SOM_EFE_MCLOCK_PHASE_SHIFT      20
#define MOD_SOM_EFE_MCLOCK_CLK              cmuClock_WTIMER0

#define MOD_SOM_EFE_SYNC_CLK                cmuClock_WTIMER1
#define MOD_SOM_EFE_SYNC_PORT               gpioPortC
#define MOD_SOM_EFE_SYNC_PIN                7
#define MOD_SOM_EFE_SYNC_TIMER              WTIMER1
#define MOD_SOM_EFE_SYNC_PHASE_SHIFT        20

#define MOD_SOM_EFE_ADC_INTERUPT_PORT       gpioPortE
#define MOD_SOM_EFE_ADC_INTERUPT_PIN        11
#define MOD_SOM_EFE_ADC_INTERUPT_ADDRESS    0x800

//ALB LDMA defines
#define MOD_SOM_EFE_LDMA_CH                0
#define MOD_SOM_EFE_LDMA_CLCK              cmuClock_LDMA
#define MOD_SOM_EFE_LDMA_IRQn              LDMA_IRQn
#define MOD_SOM_EFE_LDMA_IRQ_HANDLER_NAME  LDMA_IRQHandler

//MHA: FCTD defines to enable power on EFE-MEZZ
#define MOD_SOM_EFE_MEZZ_EN_PORT                gpioPortB
#define MOD_SOM_EFE_MEZZ_EN_PIN                 11



#endif /* MOD_EFE_MOD_SOM_EFE_BSP_H_ */
