/*******************************************************************************
 * @file mod_som_shell.h
 * @brief MOD SOM shell API
 * @date Mar 26, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API describes shell functions and task to run
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_SHELL_H_
#define MOD_SOM_SHELL_H_
#include "rtos_description.h"
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include  <common/include/shell.h>
#include "shell_util.h"
#endif
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL


//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_PREFIX
 *     16 bit identifying prefix for MOD SOM I/O status
 ******************************************************************************/
#define MOD_SOM_SHELL_PREFIX 03U

#define MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_INIT 01U
#define MOD_SOM_SHELL_STATUS_ERR_NOT_INIT 02U
#define MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_RUN 03U

#define MOD_SOM_SHELL_INPUT_BUF_SIZE 128U

/*******************************************************************************
 * @define MOD_SOM_SHELL_TASK_PRIORITY
 *     priority value of MOD SOM SHELL background task
 ******************************************************************************/
#ifndef MOD_SOM_SHELL_TASK_PRIORITY
#define MOD_SOM_SHELL_TASK_PRIORITY         20u
#endif

/*******************************************************************************
 * @define MOD_SOM_SHELL_TASK_STK_SIZE
 *     size of the tcb stack of the task
 ******************************************************************************/
#ifndef MOD_SOM_SHELL_TASK_STK_SIZE
#define MOD_SOM_SHELL_TASK_STK_SIZE         512u
#endif

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *     Initialize shell before running
 * @return
 *     MOD_SOM_STATUS_OK if initialized sucessfully
 *     MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_INIT
 ******************************************************************************/
mod_som_status_t mod_som_shell_init_f();

/*******************************************************************************
 * @brief
 *   start shell
 *
 * @return
 *     MOD_SOM_STATUS_OK if initialized sucessfully
 *     MOD_SOM_SHELL_STATUS_ERR_NOT_INIT
 *     MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_RUN
 ******************************************************************************/
mod_som_status_t mod_som_shell_start_f();


//------------------------------------------------------------------------------
// FUNCTIONS NOT FOR USER, only for implementation
//------------------------------------------------------------------------------

/*****************************************************************************
 * @brief
 *   Printf shell output function.
 *
 * @param pbuf
 *   String to be printed
 *
 * @param buf_len
 *   Lenght of the string
 *
 * @param popt
 *   Options, these are not used in this example
 *
 * @return
 *   How many characters that was printed.
 ******************************************************************************/
CPU_INT16S mod_som_shell_output_f(CPU_CHAR *pbuf,
        CPU_INT16U buf_len,
        void *popt);

uint16_t mod_som_shell_decode_status_f(mod_som_status_t status);

mod_som_status_t mod_som_shell_encode_status_f(uint16_t status);

/*******************************************************************************
 * @brief
 *   Execute user's input when a carriage return is pressed.
 *
 * @param input
 *   The string entered at prompt.
 * @param input_len
 *   Length of string input
 * @param p_arg
 *   argument passed along
 ******************************************************************************/
mod_som_status_t mod_som_shell_execute_input_f(char* input,uint32_t input_len);

/*******************************************************************************
 * @brief
 *   Get text input from user.
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_shell_get_input_f(char *buf, uint32_t * buf_len);

/*******************************************************************************
 * @brief
 *   This is the task that will be called by the initializing function in the
 *   main task to start shell task
 *
 * @param p_arg
 *   Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
void mod_som_shell_task_f(void *p_arg);
#endif
#endif /* MOD_SOM_SHELL_H_ */
