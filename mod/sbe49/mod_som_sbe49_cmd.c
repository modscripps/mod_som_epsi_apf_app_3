/*
 * mod_som_sbe49_cmd.c
 *
 *  Created on: Feb 6, 2020
 *      Author: snguyen
 */
#include <rtos_description.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_sbe49_cmd.h"
#include "mod_som_sbe49.h"
#include "mod_som_sbe49_priv.h"
//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_foo_bar_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_sbe49_cmd_tbl[] =
{
        { "sbe.start", mod_som_sbe49_cmd_start_collect_data_f},
        { "sbe.mode" , mod_som_sbe49_cmd_consumer_mode_f},
        { "sbe.stop" , mod_som_sbe49_cmd_stop_collect_data_f},
        { "sbe.gate" , mod_som_sbe49_cmd_gate_f},
        { "sbe.id"   , mod_som_sbe49_cmd_id_f},
        { "sbe.dcal" , mod_som_sbe49_cmd_dcal_f},
        { "sbe.actu" , mod_som_sbe49_cmd_actu_f},

        { 0, 0 }
};
//{ "sbe.data_mode" , mod_som_sbe49_cmd_output_mode_f},
/*******************************************************************************
 * @brief
 *   Initialize SBE49, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sbe49_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("SBE CMDs", mod_som_sbe49_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_sbe49_encode_status_f(MOD_SOM_STATUS_OK);
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe49_start_collect_data' command.
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
CPU_INT16S mod_som_sbe49_cmd_start_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status=0;
    status |= mod_som_sbe49_connect_f();
    status |= mod_som_sbe49_start_collect_data_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'sbe49_stop_collect_data' command.
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
CPU_INT16S mod_som_sbe49_cmd_stop_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status = mod_som_sbe49_stop_collect_data_f();
    //ALB turn off SBE49
    mod_som_sbe49_disconnect_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_record_mode_f' command.
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
CPU_INT16S mod_som_sbe49_cmd_consumer_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe49_consumer_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_output_mode_f' command.
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
CPU_INT16S mod_som_sbe49_cmd_output_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe49_output_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_sbe49_cmd_gate_f' command.
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
CPU_INT16S mod_som_sbe49_cmd_gate_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_sbe49_gate_f(argc,argv);

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
CPU_INT16S mod_som_sbe49_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_sbe49_id_f(argc,argv);


    mod_som_status_t status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_OK);
    if(status != MOD_SOM_STATUS_OK)
      return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

//MHA new dcal command
/*******************************************************************************
 * @brief
 *   command shell to get or set the SBE id (rev and serial number).
 *   Each parameters REV number and SBE SN is save in an exact location in the Flash memory.
 *
******************************************************************************/
CPU_INT16S mod_som_sbe49_cmd_dcal_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  //MHA need to implement the mod_som_sbe49_get_dcal_f command and get the arguments right
  mod_som_sbe49_call_get_dcal_f();


    mod_som_status_t status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_OK);
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
CPU_INT16S mod_som_sbe49_cmd_actu_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  //MHA need to implement the mod_som_sbe49_get_dcal_f command and get the arguments right
  mod_som_sbe49_set_actu_depth_f(argc,argv);


    mod_som_status_t status = mod_som_sbe49_encode_status_f(MOD_SOM_SBE49_STATUS_OK);
    if(status != MOD_SOM_STATUS_OK)
      return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



#endif
