/*
 * mod_som_vecnav_cmd.c
 *
    * @date Mar 12, 2021
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 *      Author: aleboyer
 */
#include <mod_som_vec_nav_cmd.h>

#ifdef MOD_SOM_VECNAV_EN

#include <rtos_description.h>
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <mod_som_vec_nav.h>
#include <mod_som_vec_nav_priv.h>

//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
/* @var mod_som_foo_bar_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_vecnav_cmd_tbl[] =
{
        { "vec.start", mod_som_vecnav_cmd_start_collect_data_f},
        { "vec.mode" , mod_som_vecnav_cmd_consumer_mode_f},
        { "vec.stop" , mod_som_vecnav_cmd_stop_collect_data_f},
        { 0, 0 }
};
/*******************************************************************************
 * @brief
 *   Initialize VECNAV, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_init_cmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("VEC CMDs", mod_som_vecnav_cmd_tbl, &err);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_vecnav_encode_status_f(MOD_SOM_VECNAV_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_vecnav_encode_status_f(MOD_SOM_STATUS_OK);
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'vecnav_start_collect_data' command.
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
CPU_INT16S mod_som_vecnav_cmd_start_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status=0;
    status |= mod_som_vecnav_connect_f();
    status |= mod_som_vecnav_start_collect_data_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'vecnav_stop_collect_data' command.
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
CPU_INT16S mod_som_vecnav_cmd_stop_collect_data_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){
    mod_som_status_t status = mod_som_vecnav_stop_collect_data_f();
    //ALB turn off VECNAV
    mod_som_vecnav_disconnect_f();

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}



/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_record_mode_f' command.
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
CPU_INT16S mod_som_vecnav_cmd_consumer_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_vecnav_consumer_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_output_mode_f' command.
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
CPU_INT16S mod_som_vecnav_cmd_output_mode_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_vecnav_output_mode_f(argc,argv);

    if(status != MOD_SOM_STATUS_OK)
        return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}


/***************************************************************************//**
 * @brief
 *   Shell command function for the 'mod_som_vecnav_cmd_gate_f' command.
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
CPU_INT16S mod_som_vecnav_cmd_gate_f(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT outFunc,
                   SHELL_CMD_PARAM *cmdParam){

    mod_som_status_t status = mod_som_vecnav_gate_f(argc,argv);

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
CPU_INT16S mod_som_vecnav_cmd_id_f(CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT out_put_f,
        SHELL_CMD_PARAM *cmd_param){

  mod_som_vecnav_id_f(argc,argv);


    mod_som_status_t status = mod_som_vecnav_encode_status_f(MOD_SOM_VECNAV_STATUS_OK);
    if(status != MOD_SOM_STATUS_OK)
      return SHELL_EXEC_ERR;
    return SHELL_EXEC_ERR_NONE;
}




#endif
#endif


