/*******************************************************************************
 * @app mod_som_epsi_base_app.h
 * @
 * @brief MOD SOM EPSI base app
 * @date May,18 2020
 * @author Arnaud Le Boyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This is only an example mod_som_application
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

//TOTO
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "mod_som.h"

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   This is the standard entry point for C applications.
 *   It is assumed that your code will call main() once you have performed
 *   all necessary initialization.
 *
 *   - initialize the som board before OSCreateTask
 *   - start main shell task (OSCreateTask)
 *\
 ******************************************************************************/
int main(void)
  {
    mod_som_status_t status;

    //ALB initialize the som board before OSCreateTask
    status = mod_som_main_init_f();
    //ALB stall if main_init fail
    APP_RTOS_ASSERT_DBG(status == MOD_SOM_STATUS_OK, 1);

    /*****************************************
     * END Pre OS system start
     * Add your code here
     *****************************************/
    //ALB start main shell task (OSCreateTask)
    status = mod_som_main_task_start_f();
    //ALB stall if mod_som_main_start_f fail
    APP_RTOS_ASSERT_DBG(status == MOD_SOM_STATUS_OK, 1);

    return (1);
}
