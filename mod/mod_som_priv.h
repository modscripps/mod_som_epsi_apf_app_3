/*
 * mod_som_priv.h
 *
 *  Created on: Apr 7, 2020
 *      Author: snguyen
 */

#ifndef MOD_SOM_PRIV_H_
#define MOD_SOM_PRIV_H_
#include "mod_som.h"

//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------
typedef struct{
    mod_som_prf_ptr_t prf_ptr;
    void *next_item_ptr;
    void *prev_item_ptr;
}mod_som_prf_list_item_t, *mod_som_prf_list_item_ptr_t;

typedef struct{
    mod_som_prf_ptr_t  MSC_prf_ptr;
    mod_som_prf_ptr_t  EMU_prf_ptr;
    mod_som_prf_ptr_t  RMU_prf_ptr;
    mod_som_prf_ptr_t  CMU_prf_ptr;
    mod_som_prf_ptr_t  CRYPTO0_prf_ptr;
    mod_som_prf_ptr_t  LESENSE_prf_ptr;
    mod_som_prf_ptr_t  EBI_prf_ptr;
    mod_som_prf_ptr_t  ETH_prf_ptr;
    mod_som_prf_ptr_t  SDIO_prf_ptr;
    mod_som_prf_ptr_t  GPIO_prf_ptr;
    mod_som_prf_ptr_t  PRS_prf_ptr;
    mod_som_prf_ptr_t  LDMA_prf_ptr;
    mod_som_prf_ptr_t  FPUEH_prf_ptr;
    mod_som_prf_ptr_t  GPCRC_prf_ptr;
    mod_som_prf_ptr_t  CAN0_prf_ptr;
    mod_som_prf_ptr_t  CAN1_prf_ptr;
    mod_som_prf_ptr_t  TIMER0_prf_ptr;
    mod_som_prf_ptr_t  TIMER1_prf_ptr;
    mod_som_prf_ptr_t  TIMER2_prf_ptr;
    mod_som_prf_ptr_t  TIMER3_prf_ptr;
    mod_som_prf_ptr_t  TIMER4_prf_ptr;
    mod_som_prf_ptr_t  TIMER5_prf_ptr;
    mod_som_prf_ptr_t  TIMER6_prf_ptr;
    mod_som_prf_ptr_t  WTIMER0_prf_ptr;
    mod_som_prf_ptr_t  WTIMER1_prf_ptr;
    mod_som_prf_ptr_t  WTIMER2_prf_ptr;
    mod_som_prf_ptr_t  WTIMER3_prf_ptr;
    mod_som_prf_ptr_t  USART0_prf_ptr;
    mod_som_prf_ptr_t  USART1_prf_ptr;
    mod_som_prf_ptr_t  USART2_prf_ptr;
    mod_som_prf_ptr_t  USART3_prf_ptr;
    mod_som_prf_ptr_t  USART4_prf_ptr;
    mod_som_prf_ptr_t  USART5_prf_ptr;
    mod_som_prf_ptr_t  UART0_prf_ptr;
    mod_som_prf_ptr_t  UART1_prf_ptr;
    mod_som_prf_ptr_t  QSPI0_prf_ptr;
    mod_som_prf_ptr_t  LEUART0_prf_ptr;
    mod_som_prf_ptr_t  LEUART1_prf_ptr;
    mod_som_prf_ptr_t  LETIMER0_prf_ptr;
    mod_som_prf_ptr_t  LETIMER1_prf_ptr;
    mod_som_prf_ptr_t  CRYOTIMER_prf_ptr;
    mod_som_prf_ptr_t  PCNT0_prf_ptr;
    mod_som_prf_ptr_t  PCNT1_prf_ptr;
    mod_som_prf_ptr_t  PCNT2_prf_ptr;
    mod_som_prf_ptr_t  I2C0_prf_ptr;
    mod_som_prf_ptr_t  I2C1_prf_ptr;
    mod_som_prf_ptr_t  I2C2_prf_ptr;
    mod_som_prf_ptr_t  ADC0_prf_ptr;
    mod_som_prf_ptr_t  ADC1_prf_ptr;
    mod_som_prf_ptr_t  ACMP0_prf_ptr;
    mod_som_prf_ptr_t  ACMP1_prf_ptr;
    mod_som_prf_ptr_t  ACMP2_prf_ptr;
    mod_som_prf_ptr_t  ACMP3_prf_ptr;
    mod_som_prf_ptr_t  VDAC0_prf_ptr;
    mod_som_prf_ptr_t  USB_prf_ptr;
    mod_som_prf_ptr_t  IDAC0_prf_ptr;
    mod_som_prf_ptr_t  CSEN_prf_ptr;
    mod_som_prf_ptr_t  LCD_prf_ptr;
    mod_som_prf_ptr_t  RTC_prf_ptr;
    mod_som_prf_ptr_t  RTCC_prf_ptr;
    mod_som_prf_ptr_t  WDOG0_prf_ptr;
    mod_som_prf_ptr_t  WDOG1_prf_ptr;
    mod_som_prf_ptr_t  ETM_prf_ptr;
    mod_som_prf_ptr_t  SMU_prf_ptr;
    mod_som_prf_ptr_t  TRNG0_prf_ptr;
    mod_som_prf_ptr_t  DEVINFO_prf_ptr;
    mod_som_prf_ptr_t  ROMTABLE_prf_ptr;
}mod_som_sys_prf_list_t, *mod_som_sys_prf_list_ptr_t;


//------------------------------------------------------------------------------
// FUNCTIONS NOT FOR USER, only for implementation
//------------------------------------------------------------------------------
/*******************************************************************************
 * @function
 *     mod_som_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion
 *     The status is system wide, so we only decode the last 16 bits if the
 *     higher bits show the status code is of MOD SOM I/O
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xffff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_decode_status_f(mod_som_status_t mod_som_status);

/*******************************************************************************
 * @function
 *     mod_som_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     high 16 bits are system identifier, the low 16 bits are the status code
 *     according each system
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_encode_status_f(uint8_t mod_som_io_status);

mod_som_prf_list_item_ptr_t mod_som_new_prf_list_item_f(mod_som_prf_ptr_t peripheral_ptr);

mod_som_status_t mod_som_free_prf_list_item_f(mod_som_prf_list_item_ptr_t prf_list_item_ptr);

/*******************************************************************************
 * @function
 *     mod_som_int32_2hex_f
 * @abstract
 *     Converts 32-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 32-bit number
 * @return
 *     A 64-bit ASCII array that represents 8 characters
 ******************************************************************************/
uint64_t mod_som_int32_2hex_f(uint32_t bin);

/*******************************************************************************
 * @function
 *     mod_som_int16_2hex_f
 * @abstract
 *     Converts 16-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 16-bit number
 * @return
 *     A 32-bit ASCII array that represents 4 characters
 ******************************************************************************/
uint32_t mod_som_int16_2hex_f(uint16_t bin);

/*******************************************************************************
 * @function
 *     mod_som_int8_2hex_f
 * @abstract
 *     Converts 8-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 8-bit number
 * @return
 *     A 16-bit ASCII array that represents 2 characters
 ******************************************************************************/
uint16_t mod_som_int8_2hex_f(uint8_t bin);

/*******************************************************************************
 * @function
 *     mod_som_lut_hex_str_f
 * @abstract
 *     Converts 32-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses a lookup table to convert to hexadecimal
 * @param       num
 *     binary 32-bit number
 * @param       s
 *     converted array of characters
 * @param
 *     lower_case indicator of lower or upper case of the alphabet a-f
 * @return
 *     success status
 ******************************************************************************/
uint32_t mod_som_lut_hex_str_f(uint32_t num, char *s, bool lower_case);
#endif /* MOD_MOD_SOM_PRIV_H_ */
