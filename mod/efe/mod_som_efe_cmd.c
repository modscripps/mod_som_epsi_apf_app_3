/*******************************************************************************
 * @file mod_som_efe.h
 * @brief MOD SOM foo bar command shell API Implementation
 * @date Apr 01, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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
#include "mod_som_efe_cmd.h"
#include "mod_som_efe.h"
//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_efe_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_efe_cmd_tbl[] =
{
		{ "efe.start"    , mod_som_efe_cmd_sampling_f },
		{ "efe.mode"     , mod_som_efe_cmd_consumer_mode_f },
		{ "efe.stop"     , mod_som_efe_cmd_stop_sampling_f },
		{ "efe.id"       , mod_som_efe_cmd_efe_id_f },
		{ "efe.probe"    , mod_som_efe_cmd_probe_id_f },
		{ 0, 0 }
};
//{ "efe.sleep"    , mod_som_efe_cmd_sleep_f },
//{ "efe.sigramp"  , mod_som_efe_cmd_rampsig_f },
//{ "efe.off"      , mod_som_efe_cmd_off_f },

/*******************************************************************************
 * @brief
 *   Initialize EFE BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_shellcmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("EFE CMDs", mod_som_efe_cmd_tbl, &err);

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   command shell to get or set the EFE id (rev and serial number).
 *   Each parameters REV number and EFE SN is save in an exact location in the Flash memory.
 *   ALB arbitrarily save rev_id 0x0FE00000 and and efe_sn 0x0FE00004.
 *
******************************************************************************/
CPU_INT16S mod_som_efe_cmd_efe_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

	mod_som_efe_id_f(argc,argv);


    mod_som_status_t status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_OK);
    if(status != MOD_SOM_STATUS_OK)
    	return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to get or set the probes id: type (shear or fpo7), serial number (3 digits) and calibration coefficient).
 *   The address of the probe?_id are hard coded and mapped on the Flash memory. They follow the efe_id address
 *   The address of probe1_id=0x0FE00008
 *   The address of probe2_id=0x0FE00018
 *   The address of probe3_id=0x0FE00028
 *   The address of probe4_id=0x0FE00038
 *
 *   if no argument it It will printf the probes_id in the SOM-shell
 *   otherwise it needs 4 arguments: The probe position (1,2,3,4), the probe type (sh,fp), the probe serail number and the probe calibration coef
 *   The arguments CAN ONLY BE 4 BYTES and are pass as STRINGS.
 *   ALB TODO think about Sv resolution and Sv conversion from string to float.
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
CPU_INT16S mod_som_efe_cmd_probe_id_f(CPU_INT16U argc,
		CPU_CHAR *argv[],
		SHELL_OUT_FNCT out_put_f,
		SHELL_CMD_PARAM *cmd_param){

	mod_som_efe_probe_id_f(argc,argv);

	return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to put EFE to sleep. stop sampling and energy mode 2
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
CPU_INT16S mod_som_efe_cmd_sleep_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = MOD_SOM_STATUS_OK;
    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to turn off EFE. stop sampling, deallocate data buffer, disable GPIO interrupt and LDMA transfer, and EFE EN low.
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
CPU_INT16S mod_som_efe_cmd_off_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){
    mod_som_status_t status = MOD_SOM_STATUS_OK;
    mod_som_status_t mod_som_efe_disable_hardware_f();
    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

// ALB command functions
/*******************************************************************************
 * @brief
 *   Shell command function for the 'efe_stream' command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_efe_cmd_consumer_mode_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam)
{

    mod_som_efe_consumer_mode_f(argc,argv);

    return SHELL_EXEC_ERR_NONE;
}


/*******************************************************************************
 * @brief
 *   Shell command function for the 'efe_sample' command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_efe_cmd_sampling_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam)
{
    mod_som_efe_sampling_f();
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   Shell command function for the efe_stop command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_efe_cmd_stop_sampling_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam)
{
    mod_som_efe_stop_sampling_f();
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   Shell command function for the efe_sigramp command.
 *
 *   This cmd will generate a ramp signal on the first ADC.
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S mod_som_efe_cmd_rampsig_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam)
{
    mod_som_efe_sigramp_f();
    return SHELL_EXEC_ERR_NONE;
}


