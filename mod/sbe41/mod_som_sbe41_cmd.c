/*
 * mod_som_sbe41_cmd.c
 *
    * @date Jan 27, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 *      Author: aleboyer
 */
#include <rtos_description.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <sbe41/mod_som_sbe41_cmd.h>
#include <sbe41/mod_som_sbe41.h>
#include <sbe41/mod_som_sbe41_priv.h>
//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_foo_bar_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_sbe41_cmd_tbl[] =
{
        { "sbe.start", mod_som_sbe41_cmd_start_collect_data_f},
        { "sbe.mode" , mod_som_sbe41_cmd_consumer_mode_f},
        { "sbe.stop" , mod_som_sbe41_cmd_stop_collect_data_f},
        { "sbe.gate" , mod_som_sbe41_cmd_gate_f},
        { "sbe.id"   , mod_som_sbe41_cmd_id_f},
        { "sbe.data_mode" , mod_som_sbe41_cmd_output_mode_f},
        { 0, 0 }
};
/*******************************************************************************
 * @brief
 *   Initialize SBE41, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sbe41_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("SBE CMDs", mod_som_sbe41_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_sbe41_encode_status_f(MOD_SOM_SBE41_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_sbe41_encode_status_f(MOD_SOM_STATUS_OK);
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe41_start_collect_data' command.
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
CPU_INT16S mod_som_sbe41_cmd_start_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status=0;
    status |= mod_som_sbe41_connect_f();
    status |= mod_som_sbe41_start_collect_data_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe41_stop_collect_data' command.
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
CPU_INT16S mod_som_sbe41_cmd_stop_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status = mod_som_sbe41_stop_collect_data_f();
    //ALB turn off SBE41
    mod_som_sbe41_stop_collect_data_f();
    mod_som_sbe41_disconnect_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe41_cmd_record_mode_f' command.
 *   - it configure the SOM to either STREAM or SD STORE the SBE data
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
CPU_INT16S mod_som_sbe41_cmd_consumer_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe41_consumer_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe41_cmd_output_mode_f' command.
 *   - it configure the output format of the sbe
 *   - it also configure the SOM to receive the newly defined outputformat
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
CPU_INT16S mod_som_sbe41_cmd_output_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe41_output_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe41_cmd_gate_f' command.
 *
 *   it opens a direct access to the SBE user interface
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
CPU_INT16S mod_som_sbe41_cmd_gate_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe41_gate_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/*******************************************************************************
 * @brief
 *   command shell to get or set the SBE id (rev and serial number).
 *   Each parameters REV number and SBE SN is save in an exact location in the Flash memory.
 *
******************************************************************************/
CPU_INT16S mod_som_sbe41_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_sbe41_id_f(argc,argv);


    mod_som_status_t status = mod_som_sbe41_encode_status_f(MOD_SOM_SBE41_STATUS_OK);
    if(status != MOD_SOM_STATUS_OK)
      return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}




#endif
