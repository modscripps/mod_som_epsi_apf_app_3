/*
 * mod_som_shell_cmd.c
 *
 *  Created on: Jun 4, 2020
 *      Author: aleboyer
 */



//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include "mod_som_io.h"
#include "mod_som_shell_cmd.h"


//------------------------------------------------------------------------------
// local variable declarations
//static uint32_t userdata[1024]={0xFFFFFFFF};
//static char git_branch[32] = {0};
//
////ALB The flash memory is organized in pages. For EFM32 G11 a pages is 4096 bytes
////ALB starting at address 0x0FE00000
//// ALB I save SOM info on page 0
//static uint32_t *addr_som_id = (uint32_t *)0x0FE00000;
//static uint32_t *addr_som_firmware_id = (uint32_t *)0x0FE00008;


//------------------------------------------------------------------------------
/* @var mod_som_efe_cmd_tbl predefined command table of fixed size        */
static SHELL_CMD  mod_som_cmd_tbl[] =
{
    { "som_void"            , mod_som_void_cmd_f },
    { "som_startall" , mod_som_startall_cmd_f},//MHA new startall command
		{ 0, 0 }
};
//{ "som.start"            , mod_som_starteverything_cmd_f },

/*******************************************************************************
 * @brief
 *   Initialize EFE BAR, command shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_init_shellcmd_f(){
    RTOS_ERR err;
    Shell_CmdTblAdd("SOM CMDs", mod_som_cmd_tbl, &err);

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
        return mod_som_shell_encode_status_f(MOD_SOM_STATUS_FAIL_TO_ADD_CMD_TBL);
    }

    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
}

CPU_INT16S mod_som_void_cmd_f (CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam){

	return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);

}

CPU_INT16S mod_som_startall_cmd_f (CPU_INT16U argc,
        CPU_CHAR *argv[],
        SHELL_OUT_FNCT outFunc,
        SHELL_CMD_PARAM *cmdParam){

  return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);

}

///*******************************************************************************
// * @brief
// *   Initialize EFE BAR, command shell
// *
// * @return
// *   MOD_SOM_STATUS_OK if initialization goes well
// *   or otherwise
// ******************************************************************************/
//mod_som_status_t mod_som_starteverything_cmd_f(){
//    RTOS_ERR err;
//    Shell_CmdTblAdd("SOM CMDs", mod_som_cmd_tbl, &err);
//
//    //ALB start efe
//    //ALB start sbe
//    //ALB start vecnav;
//    //ALB
//
//    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
//        return mod_som_shell_encode_status_f(MOD_SOM_STATUS_FAIL_TO_ADD_CMD_TBL);
//    }
//
//    return mod_som_shell_encode_status_f(MOD_SOM_STATUS_OK);
//}

