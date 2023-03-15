/*******************************************************************************
 * @file mod_som_voltage.h
 * @brief MOD SOM foo bar command shell API Implementation
 * @date Jan 23, 2021
 * @author (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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
#include <voltage/mod_som_voltage_cmd.h>
//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_voltage_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_voltage_cmd_tbl[] =
{
        { "volt.stop", mod_som_voltage_cmd_stop_f},
        { "volt.start",  mod_som_voltage_cmd_start_f},
        { "volt.start_adc1",  mod_som_voltage_cmd_start_adc1_f},
        { "volt.stop_adc1", mod_som_voltage_cmd_stop_adc1_f},
        { "volt.mode",  mod_som_voltage_cmd_mode_f},
        { "volt.scan",  mod_som_voltage_cmd_scan_f},
        { "volt.scan_adc1",  mod_som_voltage_cmd_adc1_scan_f},
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
mod_som_status_t mod_som_voltage_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("VOLT CMDs", mod_som_voltage_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_voltage_encode_status_f(MOD_SOM_VOLTAGE_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_voltage_encode_status_f(MOD_SOM_STATUS_OK);
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
CPU_INT16S mod_som_voltage_cmd_hello_world_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;
    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to enable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_start_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_start_scan_task_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to enable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_start_adc1_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_start_adc1_scan_task_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell to disable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_stop_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_stop_scan_task_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to disable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_stop_adc1_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_stop_adc1_scan_task_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell to disable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_scan_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_scan_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to disable GG11 internal ADC sampling of the voltage
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
CPU_INT16S mod_som_voltage_cmd_adc1_scan_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_adc1_scan_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell to select the data mode 0:streaming, 1:SD storing
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
CPU_INT16S mod_som_voltage_cmd_mode_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    status=mod_som_voltage_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



