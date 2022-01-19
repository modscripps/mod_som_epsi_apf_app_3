/*******************************************************************************
 * @file mod_som_shell.c
 * @brief MOD SOM shell API implementation
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

#include "rtos_description.h"
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"
#include "mod_som_io.h"



//------------------------------------------------------------------------------
// LOCAL VARIABLES
//------------------------------------------------------------------------------
static bool mod_som_shell_initialized_flag = false;
static CPU_STK mod_som_shell_task_stack[MOD_SOM_SHELL_TASK_STK_SIZE];
static OS_TCB mod_som_shell_task_tcb;
//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   initialize pre-defined Micrium shell
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_shell_init_f(){
    RTOS_ERR  err;
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    Shell_Init(&err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_shell_encode_status_f(MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_INIT);
#endif
    mod_som_shell_initialized_flag = true;
    return MOD_SOM_STATUS_OK;
}

mod_som_status_t mod_som_shell_start_f(){
    if(!mod_som_shell_initialized_flag)
        return mod_som_shell_encode_status_f(MOD_SOM_SHELL_STATUS_ERR_NOT_INIT);
    RTOS_ERR err;
    OSTaskCreate(&mod_som_shell_task_tcb, // Create the Start Task
                "MOD SOM Shell Task",
                mod_som_shell_task_f,
                DEF_NULL,
                MOD_SOM_SHELL_TASK_PRIORITY,
                &mod_som_shell_task_stack[0],
                (MOD_SOM_SHELL_TASK_STK_SIZE / 10u),
                MOD_SOM_SHELL_TASK_STK_SIZE,
                0u,
                0u,
                DEF_NULL,
                (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_shell_encode_status_f(MOD_SOM_SHELL_STATUS_ERR_FAIL_TO_RUN);
    return MOD_SOM_STATUS_OK;
}

/*****************************************************************************
 * @brief
 *   Printf shell output function. used for micrium shell
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
        void *popt){
    (void)popt; // Unused argument

    mod_som_io_stream_data_f((uint8_t *)pbuf,buf_len,DEF_NULL);
    return buf_len;
}

//------------------------------------------------------------------------------
// FUNCTIONS NOT FOR USER, only for implementation
//------------------------------------------------------------------------------
uint16_t mod_som_shell_decode_status_f(mod_som_status_t status){
    if(status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_SHELL_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

mod_som_status_t mod_som_shell_encode_status_f(uint16_t status){
    if(status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_SHELL_PREFIX, status);
}

/*******************************************************************************
 * @brief
 *   Execute user's input when a carriage return is pressed.
 *
 * @param input
 *   The string entered at prompt.
 * @param input_len
 *   Length of string input
 ******************************************************************************/
mod_som_status_t mod_som_shell_execute_input_f(char* input,uint32_t input_len){
    RTOS_ERR err;
    mod_som_status_t status=MOD_SOM_STATUS_OK;

    //contruct command parameters to pass in
    SHELL_CMD_PARAM  p_cmd_param = {
            .CurWorkingDirPtr = DEF_NULL,
            .OutputOptPtr = DEF_NULL,
            .SessionActiveFlagsPtr = DEF_NULL
    };
    //execute the commands
    Shell_Exec(input, mod_som_shell_output_f, &p_cmd_param, &err);

    switch (RTOS_ERR_CODE_GET(err)) {
    case RTOS_ERR_NULL_PTR:
        shellPrint(mod_som_shell_output_f, "Error, NULL pointer passed.\n");
        status=MOD_SOM_STATUS_NOT_OK;
        break;
    case RTOS_ERR_NOT_FOUND:
        shellPrintf(mod_som_shell_output_f, "Error, command not found: %s\n", input);
        status=MOD_SOM_STATUS_NOT_OK;
        break;
    case RTOS_ERR_NOT_SUPPORTED:
        shellPrint(mod_som_shell_output_f, "Error, command not supported.\n");
        status=MOD_SOM_STATUS_NOT_OK;
        break;
    case RTOS_ERR_INVALID_ARG:
        shellPrint(mod_som_shell_output_f, "Error, invalid arguments\n");
        status=MOD_SOM_STATUS_NOT_OK;
        break;
    case RTOS_ERR_SHELL_CMD_EXEC:
        shellPrint(mod_som_shell_output_f, "Error, command failed to execute.\n");
        status=MOD_SOM_STATUS_NOT_OK;
        break;
    case RTOS_ERR_NONE: /* No errors. */
        break;
    default:
        shellPrint(mod_som_shell_output_f, "Error, unknown error\n");
        status=MOD_SOM_STATUS_NOT_OK;

        break;
    }
    return status;
}

/*******************************************************************************
 * @brief
 *   Get text input from user.
 *
 * @param buf
 *   Buffer to hold the input string.
 * @param buf_length
 *  Length of buffer as the user is typing
 ******************************************************************************/
mod_som_status_t mod_som_shell_get_input_f(char *buf, uint32_t * buf_len){
    int c;
    int32_t i;
    RTOS_ERR err;

    Mem_Set(buf, '\0', MOD_SOM_SHELL_INPUT_BUF_SIZE); // Clear previous input
    for (i = 0; i < MOD_SOM_SHELL_INPUT_BUF_SIZE - 1; i++) {
        c = RETARGET_ReadChar();

        //ALB read the input str.
        while (c < 0){ // Wait for valid input
            //Release for waiting tasks
            OSTimeDly(
                    (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                    (OS_OPT      )OS_OPT_TIME_DLY,
                    &err);
            APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
            c = RETARGET_ReadChar();
        }


        if (c == ASCII_CHAR_DELETE || c == 0x08) { // User inputed backspace
            if (i) {
                mod_som_io_print_f("\b \b");
                buf[--i] = '\0';
            }
            i--;
            continue;
        } else if (c == '\r' || c == '\n') {
            if (i) {
                mod_som_io_print_f("\r\n");
                break;
            } else {
                mod_som_io_print_f("\r\n$ ");
                i--;
                continue;
            }
        }else if(c == 27){
            for(i--;i>=0;i--){
                mod_som_io_print_f("\b \b");
                //Release for waiting tasks
                OSTimeDly(
                        (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                        (OS_OPT      )OS_OPT_TIME_DLY,
                        &err);
            }
            buf[0] = '\0';
            continue;
        }else if (!(c>31 && c<127)){ // check for printable characters
            i--;
            continue;
        }

        mod_som_io_putchar_f(c); // Echo to terminal
        buf[i] = c;
    }

    buf[i] = '\0';
    *(buf_len) = i;
    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   This is the task that will be called by the initializing function in the
 *   main task to start shell task
 *
 * @param p_arg
 *   Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
void mod_som_shell_task_f(void *p_arg){
    (void)p_arg; // Deliberately unused argument
    RTOS_ERR err;
    char     input_buf[MOD_SOM_SHELL_INPUT_BUF_SIZE];
    uint32_t input_buf_len;


    while (DEF_ON) {
        OSTimeDly(
                (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
                (OS_OPT      )OS_OPT_TIME_DLY,
                &err);
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

        shellPrint(mod_som_shell_output_f, "\r$ ");
        mod_som_shell_get_input_f(input_buf,&input_buf_len);

        if (!Str_Cmp(input_buf, "exit")) {
            break;
        }

        mod_som_shell_execute_input_f(input_buf,input_buf_len);
    }
}
#endif
