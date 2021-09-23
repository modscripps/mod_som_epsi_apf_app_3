/*******************************************************************************
 * @file mod_som_common.h
 * @brief MOD SOM common header
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This program uses the Micrium shell utility to initializes communication port
 * for SBE 49 and has the capability to stream SBE49 data
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_COMMON_H_
#define MOD_SOM_COMMON_H_

//#define SL_SLEEPTIMER_WALLCLOCK_CONFIG 1

#include <mod_som_bsp.h>
#include <mod_som_cfg.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include <em_assert.h>
#include <em_chip.h>
#include <em_common.h>
#include <em_core.h>
#include <em_cryotimer.h>
#include <em_device.h>
#include <em_emu.h>
#include <em_cmu.h>
#include <em_gpio.h> //system dependent EMF32
#include <em_usart.h>
#include <em_leuart.h>
#include <em_ldma.h>
#include <em_rtc.h>
#include <em_rtcc.h>
#include <em_timer.h>

//ALB add msc to save User data in flash
#include <em_msc.h>



#include <stdlib.h>
#include <string.h>

#include <common/include/lib_mem.h>
#include <common/include/lib_def.h>
#include <common/include/lib_ascii.h>

#include <retargetserial.h>
#include <cpu/include/cpu.h>
#include <kernel/include/os.h>
#include <common/include/rtos_err.h>
#include <common/include/rtos_utils.h>

#include <common/include/common.h>
#include <common/include/auth.h>

#include <rtos_description.h>
#include <rtos_cfg.h>

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
#include  <common/include/clk.h>
#endif


#include <sl_sleeptimer.h>
#include "sl_sleeptimer_config.h"


#define MOD_SOM_ENCODE_STATUS(status_pfix, status) (status_pfix<<24 | ((status & 0xffU)<<16))
#define MOD_SOM_DECODE_STATUS(status_code,status_pfix_ptr, status_ptr) { \
    *(status_pfix_ptr) = status_code>>24; \
    *(status_ptr) = (status_code>>16) & 0xffU; \
}
#define MOD_SOM_STATUS_OK 0
#define MOD_SOM_STATUS_NOT_OK 1

//------------------------------------------------------------------------------
// type defs
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   selection of uart_slct_t (EMF32_specific
 *
 * @option uart
 * UART
 *
 * @option leuart
 * LEUART
 ******************************************************************************/
//typedef enum {
//    mod_som_uart_slct_usart0 = 0,
//    mod_som_uart_slct_usart1 = 1,
//    mod_som_uart_slct_usart2 = 2,
//    mod_som_uart_slct_usart3 = 3,
//    mod_som_uart_slct_usart4 = 4,
//    mod_som_uart_slct_usart5 = 5,
//    mod_som_uart_slct_uart0 = 10,
//    mod_som_uart_slct_uart1 = 11,
//    mod_som_uart_slct_leuart0 = 20,
//    mod_som_uart_slct_leuart1 = 21
//} mod_som_uart_slct_t;

/** Parity selection. */
typedef enum {
    mod_som_uart_parity_none   = 0,    /**< No parity. */
    mod_som_uart_parity_even = 2,    /**< Even parity. */
    mod_som_uart_parity_odd  = 1      /**< Odd parity. */
} mod_som_uart_parity_t;

/** Stop bits selection. */
typedef enum {
    mod_som_uart_stop_bits_1 = 1,           /**< 1 stop bits. */
    mod_som_uart_stop_bits_2 = 2            /**< 2 stop bits. */
} mod_som_uart_stop_bits_t;
typedef enum {
    mod_som_uart_data_bits_8 = 0,           /**< 1 stop bits. */
    mod_som_uart_data_bits_9 = 1            /**< 2 stop bits. */
}mod_som_uart_data_bits_t;


/*******************************************************************************
 * @brief
 *     peripheral type where interrupts can be handled
 * @description
 *
 * @field handle_port
 *     a pointer to register (if used with EFM system)
 * @field irq_handler_1_f
 *     pointer to interrupt callback function
 *     if UART port has two interrupt callback function, this would be the RX
 *     interrupt callback function
 * @field irq_handler_2_f
 *     pointer to additional interrupt callback function
 *     if UART port has two interrupt callback function, this would be the TX
 *     interrupt callback function
 * @field device_ptr
 *     pointer to parent device of this peripheral
 ******************************************************************************/
typedef struct{
    void * handle_port;
    void (* irq_handler_1_f)(void * mod_som_device_ptr);
    void (* irq_handler_2_f)(void * mod_som_device_ptr);
    void * device_ptr;
}mod_som_prf_t,*mod_som_prf_ptr_t;



/*******************************************************************************
 * @brief
 *   GPIO port definition
 *
 * @field Port
 *   Port A,B,C,D....
 * @field Pin
 *   Pin 1 2 3 4 5 6
 ******************************************************************************/
typedef struct{
  GPIO_Port_TypeDef port;
  unsigned int pin;
} mod_som_gpio_port_t, *mod_som_gpio_port_ptr_t;

/*******************************************************************************
 * @brief
 *   TX - RX port definition
 *
 * @field Port
 *   Port A,B,C,D....
 * @field Pin
 *   Pin 1 2 3 4 5 6
 ******************************************************************************/
typedef struct{
  void * com_port;
  GPIO_Port_TypeDef tx_port;
  GPIO_Port_TypeDef rx_port;
  GPIO_Port_TypeDef en_port;
  unsigned int tx_pin;
  unsigned int rx_pin;
  unsigned int route;
  unsigned int en_pin;
  mod_som_uart_parity_t parity;
  mod_som_uart_data_bits_t data_bits;
  mod_som_uart_stop_bits_t stop_bits;

} mod_som_com_port_t, *mod_som_com_port_ptr_t;


/*******************************************************************************
 * @brief
 *   mod SOM timer handle
 *
 * @field timer
 *  timer ID e.g. WTIMER0
 *
 * @field phase_shift
 *    define the phase shift
 *
 * @field timer_clock
 *    Define the timer clock e.g. cmuClock_WTIMER0
 *
 *
 * @field  timer_pin;
 * gpioPortC, 7
 *
 * @field  init;
 * TIMER_Init_TypeDef init
 *
 * @field route_location
 *   TODO I do not know how to factor this in the code.
 *
 ******************************************************************************/
typedef struct{
  TIMER_TypeDef *timer;
  uint32_t top;
  uint32_t compare_value;
  CMU_Clock_TypeDef timer_clock;
  mod_som_gpio_port_t timer_pin;
  TIMER_Init_TypeDef timer_init;
  uint32_t route_location;
} mod_som_timer_handle_t, *mod_som_timer_handle_ptr_t;

//typedef struct{
//  TIMER_TypeDef *timer;
//  uint32_t compare_value;
//  uint32_t phase_shift;
//  CMU_Clock_TypeDef timer_clock;
//  mod_som_gpio_port_t timer_pin;
//  TIMER_Init_TypeDef timer_init;
//  uint32_t route_location;
//} mod_som_timer_handle_t, *mod_som_timer_handle_ptr_t;

/*******************************************************************************
 * @brief
 *   SOM LDMA handle
 *
 * @field init
 *    LDMA_INIT_DEFAULT
 *    LDMA_Descriptors
 *    LDMA channel
 ******************************************************************************/
typedef struct{
  LDMA_Init_t init;
  uint8_t ch;
  void (* irq_f)();
  IRQn_Type irqn;


} mod_som_ldma_handle_t, *mod_som_ldma_handle_ptr_t;



typedef uint32_t mod_som_status_t;


#endif /* MOD_SOM_COMMON_H_ */

