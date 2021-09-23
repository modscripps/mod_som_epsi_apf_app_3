/*******************************************************************************
 * @file mod_som_aggregator.h
 * @brief MOD SOM aggregator command shell API Implementation
 * @date Apr 20, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API uses micrium shell to insert shell table and execute shell commands
 * Make sure the SHELL_CMD table is static so that when you call your custom
 * command you don't get memory access error.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <aggregator/mod_som_aggregator_cmd.h>

#ifdef  MOD_SOM_AGGREGATOR_EN

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_aggregator_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_aggregator_cmd_tbl[] =
{
        { "aggreg.format", mod_som_aggregator_cmd_format_f},
        { "aggreg.start", mod_som_aggregator_cmd_start_f},
        { "aggreg.stop", mod_som_aggregator_cmd_stop_f},
        { 0, 0 }
};

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("AGG CMDs", mod_som_aggregator_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   command shell for hello world command
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_aggregator_cmd_format_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_status_t status = mod_som_aggregator_consumer_mode_f(argc,argv);


    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell for hello world command
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_aggregator_cmd_start_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_status_t status = mod_som_aggregator_start_consumer_task_f();


    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell for hello world command
 * @param argc
 *   argument count
 * @param argv
 *   argument values
 * @param out_put_f
 *   out_put_f (print function)
 * @param cmd_param
 *   command parameters (passing along)
 * @return
 *   Micrium Command Shell Status
 ******************************************************************************/
CPU_INT16S mod_som_aggregator_cmd_stop_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

    mod_som_status_t status = mod_som_aggregator_stop_consumer_task_f();


    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

#endif

