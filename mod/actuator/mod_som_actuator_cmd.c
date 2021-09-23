/*******************************************************************************
 * @file mod_som_actuator.h
 * @brief MOD SOM actuator command shell API Implementation
 @date January 4th 2021
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
#include <actuator/mod_som_actuator_cmd.h>

#ifdef  MOD_SOM_ACTUATOR_EN

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_actuator_cmd_tbl predefined command table of fixed size        */

/**********************************
 * Naming convention for the cmd:
 *   modulename.functionname
 *   e.g. actuator.hw
 *
 **********************************/
static SHELL_CMD  mod_som_actuator_cmd_tbl[] =
{
        { "actu.toggle", mod_som_actuator_cmd_toggle_f},
        { "actu.ptc"   , mod_som_actuator_cmd_ptc_f},
        { "actu.ctc"   , mod_som_actuator_cmd_ctc_f},
        { "actu.set"   , mod_som_actuator_cmd_set_f},
        { "actu.mode"  , mod_som_actuator_cmd_mode_f},
        { 0, 0 }
};
//        { "actu.set "  , mod_som_actuator_cmd_set_f},

/*******************************************************************************
 * @brief
 *   Initialize ACTUATOR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_actuator_init_cmd_f(){
    RTOS_ERR err;

    //ALB The name of the memory space "ACT CMDs" NEEDS to be 8 char.
    Shell_CmdTblAdd("ACT CMDs", mod_som_actuator_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_actuator_encode_status_f(MOD_SOM_ACTUATOR_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_actuator_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   command shell for toggling the actuator command
 *
 *   when toggling the actuator  can only have 2 positions
 *    - 100% (full extent)
 *    - 0%   (fully retracted)
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
CPU_INT16S mod_som_actuator_cmd_toggle_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    mod_som_actuator_toggle_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell set the actuator
 *
 *   this command takes one argument a uint8_t percentage between 0-100%
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
CPU_INT16S mod_som_actuator_cmd_set_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    mod_som_actuator_set_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



/*******************************************************************************
 * @brief
 *   command shell set the actuator
 *
 *   this command takes one argument a uint8_t percentage between 0-100%
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
CPU_INT16S mod_som_actuator_cmd_ptc_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    mod_som_actuator_ptc_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   command shell set the actuator
 *
 *   this command takes one argument a uint8_t percentage between 0-100%
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
CPU_INT16S mod_som_actuator_cmd_ctc_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    mod_som_actuator_ctc_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell set the actuator
 *
 *   this command takes one argument a uint8_t percentage between 0-100%
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
CPU_INT16S mod_som_actuator_cmd_mode_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = 0;

    mod_som_actuator_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

#endif
