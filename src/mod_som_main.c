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
#include "em_emu.h"

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
bool mod_som_sleep_flag;

//AlB Structure to initialize the watchdog timer.
/* Defining the watchdog initialization data */
//WDOG_Init_TypeDef wdog_init =
//{
//  .enable     = true,                 /* Start watchdog when init done */
//  .debugRun   = false,                /* WDOG not counting during debug halt */
//  .em2Run     = true,                 /* WDOG counting when in EM2 */
//  .em3Run     = true,                 /* WDOG counting when in EM3 */
//  .em4Block   = false,                /* EM4 can be entered */
//  .swoscBlock = false,                /* Do not block disabling LFRCO/LFXO in CMU */
//  .lock       = false,                /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
//  .clkSel     = wdogClkSelULFRCO,     /* Select 1kHZ WDOG oscillator */
//  .perSel     = wdogPeriod_32k,        /* Set the watchdog period to 2049 clock periods (ie ~2 seconds) */
//};


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
//*******************************************************************************
 /* @brief
 *   - We are inside the main shell task.
 *   - This function initialize all the enabled modules.
 *
 ******************************************************************************/
void mod_som_main_start_modules_f()
{

//#if defined(MOD_SOM_SDIO_EN)
//  CPU_CHAR filename[10]="modsom";
//  mod_som_sdio_define_filename_f(filename);
//#endif

#if defined(MOD_SOM_SETTINGS_EN)
  mod_som_settings_stream_settings_f();
#endif

#if defined(MOD_SOM_SBE41_EN)
  mod_som_sbe41_connect_f();
  mod_som_sbe41_start_collect_data_f();
#endif

#if defined(MOD_SOM_EFE_EN)
  mod_som_efe_sampling_f();
#endif

  printf("ok\r\n");

  mod_som_running_flag=true;
  mod_som_sleep_flag=false;

}

/*******************************************************************************
 * @brief
 *   - We are inside the main shell task.
 *   - This function initialize all the enabled modules.
 *
 ******************************************************************************/
void mod_som_main_stop_modules_f()
{

  int delay =1000;
  mod_som_status_t status;
  status=MOD_SOM_STATUS_OK;

  // stop ADC master clock timer
  status|= mod_som_efe_stop_sampling_f();



  // stop collecting CTD data
  status|= mod_som_sbe41_stop_collect_data_f();
  status|= mod_som_sbe41_disconnect_f();

  // stop turbulence processing task
  status = mod_som_efe_obp_stop_fill_segment_task_f();
  status|= mod_som_efe_obp_stop_cpt_spectra_task_f();
  status|= mod_som_efe_obp_stop_cpt_dissrate_task_f();
  status|= mod_som_efe_obp_stop_consumer_task_f();

  //ALB stop APF producer task
  status |= mod_som_apf_stop_producer_task_f();
  //ALB stop APF consumer task
  status |= mod_som_apf_stop_consumer_task_f();

  sl_sleeptimer_delay_millisecond(delay);
  //ALB disable SDIO hardware
  mod_som_sdio_disable_hardware_f();

printf("epsi sleep\r\n");


  mod_som_running_flag=false;
  WDOGn_Lock(DEFAULT_WDOG);

}

/*******************************************************************************
 * @brief
 *   - We are inside the main shell task.
 *   - This function initialize all the enabled modules.
 *
 ******************************************************************************/
mod_som_status_t mod_som_main_sleep_f()
{
  int delay =1000;

  if (mod_som_sleep_flag==false){
      printf("Making all modules are stopped \r\n");
//      mod_som_main_stop_modules_f();

      //Select intern HFRCO
//      CMU_OscillatorEnable(cmuOsc_HFRCO, true,true);
//      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

      //ALB Disable HFXO
      //ALB IF cmuSelect_HFRCO is NOT selected before it will reset the whole board
      /* Power External Oscillator SOM-U8-U4*/
      // HF oscillator disable.

      // turn dowm HFXO
      //
      //ALB      DC/DC burst mode  PF10 low
      RETARGET_SerialFlush(); // Wait for UART TX buffer to be empty
      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
      CMU_HFRCOBandSet(cmuHFRCOFreq_13M0Hz);
      RETARGET_SerialInit(); // Re-enable VCOM
      GPIO_PinModeSet(gpioPortF, 10, gpioModePushPull, 0);
      CMU_OscillatorEnable(cmuOsc_HFXO, false, false);
      GPIO_PinModeSet(MOD_SOM_HFXO_EN_PORT,
                      MOD_SOM_HFXO_EN_PIN,
                      gpioModePushPull, 0);

//WAKE UP CMD
//      GPIO_PinModeSet(gpioPortF, 10, gpioModePushPull, 1);
//      RETARGET_SerialFlush(); // Wait for UART TX buffer to be empty
//      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
//      RETARGET_SerialInit(); // Re-enable VCOM



      //ALB I want to keep LEUART alive  (SBEcom) so I reconnect it
      //ALB It also send some power to the SBE I need to NOT do this

//      CMU_ClockEnable(cmuClock_HFLE, true);
//      CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // Set a reference clock
//      CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable device clock */
//      CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1); // Don't prescale LEUART clock
//      LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT;
//      LEUART_Init(LEUART0, &init);
      EMU_EnterEM2(false);
      mod_som_sleep_flag=true;

  }else{
      EMU_EnterEM2(false);
  }

  return 0;
}

/*******************************************************************************
 * @brief
 *   - We are inside the main shell task.
 *   - This function initialize all the enabled modules.
 *
 ******************************************************************************/
mod_som_status_t mod_som_main_wake_up_f()
{

  int delay =1000;

  if (mod_som_sleep_flag==true){
      printf("Making all modules are stopped \r\n");

      //ALB      DC/DC not burst mode  PF10 high
      GPIO_PinModeSet(gpioPortF, 10, gpioModePushPull, 1);

      // turn dowm HFXO
      GPIO_PinModeSet(MOD_SOM_HFXO_EN_PORT, MOD_SOM_HFXO_EN_PIN, gpioModePushPull, 1);
//      //Select intern HFXO
      CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
      RETARGET_SerialFlush(); // Wait for UART TX buffer to be empty
      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
      RETARGET_SerialInit(); // Re-enable VCOM
      sl_sleeptimer_delay_millisecond(delay);

//      // HFRCO oscillator disable.
//      CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
//
//      CMU_ClockEnable(cmuClock_HFLE, true);
      CMU_ClockEnable(cmuClock_HFPER, true);
//      CMU_ClockEnable(cmuClock_CORELE, true);


      //ALB Software reset of SDIO
//      SDIO->CLOCKCTRL|=(_SDIO_CLOCKCTRL_SFTRSTA_MASK & SDIO_CLOCKCTRL_SFTRSTA);

      sl_sleeptimer_delay_millisecond(delay);


      mod_som_sleep_flag=false;




  }

  return 0;
}



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
    int delay =10;

    //initialize the SOM running flag
    mod_som_running_flag=false;
    /*****************************************
     * Initialize main task (i.e. som shell)
     *****************************************/
    mod_som_main_task_init_f();

    sl_sleeptimer_delay_millisecond(delay);

    printf("\r\n=====START INITIALIZATION======\r\n");

    /*****************************************
     * Initialize Modules
     *****************************************/
    mod_som_modules_init_f();

    printf("\r\n=====STOP INITIALIZATION======\r\n");
    printf("===================================\r\n");
    printf("== YOU MUST SET THE CLOCK NOW WITH 'time.set'!!! ==\r\n");
    printf("== Type 'help' for commands and syntax. ==\r\n\r\n");//MHA
    printf("===================================\r\n");
    printf("===========MODSOM SHELL============\r\n");
    printf("===================================\r\n$");

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
