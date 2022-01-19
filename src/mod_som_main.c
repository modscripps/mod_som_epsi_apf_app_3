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

/*MAG Please identify and tag with a comment the position in the code for the "producers" and "consumers"
 *MAG for all the sources i.e. EFE LDMA channel done isr,
 *MAG SBE49 USART receive isr, SDIO write,  USART stream  etc.*/


//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "mod_som_common.h"
#include "mod_som.h"
#include "mod_som_io.h"
#include "mod_som_shell.h"

#include <stddef.h>

#ifdef MOD_SOM_SDIO_EN
#include "ff.h"
#include "sdio.h"
#include "diskio.h"
#endif


/*****************************************
 * START Include module header files here
 *****************************************/
//#include "mod_som_foo_bar.h"

#if defined(MOD_SOM_CALENDAR_EN)
#include "mod_som_calendar.h"
#endif

#if defined(MOD_SOM_SETTINGS_EN)
#include "mod_som_settings.h"
#endif

//ALB EFE module
#if defined(MOD_SOM_EFE_EN)
#include "mod_som_efe.h"
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
#include "mod_som_efe_obp.h"
#endif
//ALB SBE49 module
#if defined(MOD_SOM_SBE49_EN)
// ALB I had to make a copy of  system_efm32gg11b.h in external_copied folder
// ALB and comment void GPIO_ODD_IRQHandler
#include "mod_som_sbe49.h"
#endif
#if defined(MOD_SOM_SBE41_EN)
// ALB I had to make a copy of  system_efm32gg11b.h in external_copied folder
// ALB and comment void GPIO_ODD_IRQHandler
#include <sbe41/mod_som_sbe41.h>
#endif


#if defined(MOD_SOM_SDIO_EN)
#include "mod_som_sdio.h"
#endif

#if defined(MOD_SOM_ACTUATOR_EN)
#include "mod_som_actuator.h"
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
#include "mod_som_altimeter.h"
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
#include "mod_som_vec_nav.h"
#endif
#if defined(MOD_SOM_AGGREGATOR_EN)
#include "mod_som_aggregator.h"
#endif
#if defined(MOD_SOM_VOLTAGE_EN)
#include "mod_som_voltage.h"
#endif


bool mod_som_running_flag;

/*****************************************
 * END Include module header files here
 *****************************************/
/*******************************************************************************
 * @brief
 *   - We are inside the main shell task.
 *   - This function initialize all the enabled modules.
 *
 ******************************************************************************/
void mod_som_modules_init_f()
{
#if defined(MOD_SOM_SETTINGS_EN)
    mod_som_settings_init_f();
#endif
#if defined(MOD_SOM_CALENDAR_EN)
    mod_som_calendar_init_f();
#endif
#if defined(MOD_SOM_SDIO_EN)
    mod_som_sdio_init_f();
#endif
#if defined(MOD_SOM_SBE49_EN)
    mod_som_sbe49_init_f();
#endif
#if defined(MOD_SOM_SBE41_EN)
    mod_som_sbe41_init_f();
#endif
#if defined(MOD_SOM_EFE_EN)
    mod_som_efe_init_f();
#endif
#if defined(MOD_SOM_EFE_OBP_EN)
    mod_som_efe_obp_init_f();
#endif
#if defined(MOD_SOM_APF_EN)
    mod_som_apf_init_f();
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
    mod_som_actuator_init_f();
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
    mod_som_altimeter_init_f();
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
    mod_som_vecnav_init_f();
#endif
#if defined(MOD_SOM_VOLTAGE_EN)
    mod_som_voltage_init_f();
#endif
#if defined(MOD_SOM_AGGREGATOR_EN)
    mod_som_aggregator_init_f();
#endif
#if defined(MOD_SOM_SETTINGS_EN)
    mod_som_settings_save_settings_f();
#endif

}



//------------------------------------------------------------------------------
// MAIN TASK
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   - We are inside the main shell task.
 *   - initialize the main shell task
 *   This is the task that will be called by the Startup when all services
 *   are initializes successfully.
 *
 * @param p_arg
 *   Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
void mod_som_main_task_f(void *p_arg)
{
    (void)p_arg; // Deliberately unused argument
    uint64_t tick;
    RTOS_ERR err;

    //initialize the SOM running flag
    mod_som_running_flag=false;
    /*****************************************
     * Initialize main task (i.e. som shell)
     *****************************************/
    mod_som_main_task_init_f();

    /*****************************************
     * Initialize Modules
     *****************************************/
    mod_som_modules_init_f();

    /*****************************************
     * END Post OS start Add your code here
     *****************************************/

    while (DEF_ON) {
        OSTimeDly (
                (OS_TICK     )1000,
                (OS_OPT      )OS_OPT_TIME_DLY,
                &err);
        tick=sl_sleeptimer_get_tick_count64();

        //ALB   feed (reset) the watchdog timer.
        //ALB   Be aware that the priorities of the tasks
        //ALB   can mess up the logic of the watch dog.
        //ALB   I.e. if the shell hangs up the WDOG still get fed but the
        //ALB   user loose the control on the shell and the SOM.

        //ALB   Solution: Create a Feeder task with the right priority
        //ALB   so we are not concerned by such faulty logic.
        WDOG_Feed();


        if((tick >1000) & !mod_som_running_flag){
            //MHA do not start modules automatically
//            mod_som_altimeter_start_task_f();
//            mod_som_efe_sampling_f();
//            mod_som_sbe49_connect_f();
//            mod_som_sbe49_start_collect_data_f();

//            mod_som_vecnav_connect_f();
//            mod_som_vecnav_start_collect_data_f();
//            mod_som_aggregator_start_consumer_task_f();
//            mod_som_efe_obp_start_fill_segment_task_f();
//            mod_som_efe_obp_start_cpt_spectra_task_f();
//            mod_som_efe_obp_start_cpt_dissrate_task_f();
//            mod_som_efe_obp_start_consumer_task_f();

            mod_som_running_flag=true;
        }

        if(((tick % 1000)==0) & !mod_som_running_flag){

        }


        //ALB toggle led to tell us it alive
        GPIO_PinOutToggle(gpioPortC, 6); // LED

        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    }
}

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
    status = mod_som_main_start_f();
    //ALB stall if mod_som_main_start_f fail
    APP_RTOS_ASSERT_DBG(status == MOD_SOM_STATUS_OK, 1);

    return (1);
}
