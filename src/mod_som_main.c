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
//ALB SBE41 module
#if defined(MOD_SOM_SBE41_EN)
// ALB I had to make a copy of  system_efm32gg11b.h in external_copied folder
// ALB and comment void GPIO_ODD_IRQHandler
#include <sbe41/mod_som_sbe41.h>
#include <sbe41/mod_som_sbe41_bsp.h>
#endif

#if defined(MOD_SOM_SDIO_EN)
#include "mod_som_sdio.h"
#endif
#if defined(MOD_SOM_APF_EN)
#include "mod_som_apf.h"
#endif
#if defined(MOD_SOM_VOLTAGE_EN)
#include "mod_som_voltage.h"
#endif


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
#if defined(MOD_SOM_VOLTAGE_EN)
    mod_som_voltage_init_f();
#endif
#if defined(MOD_SOM_SDIO_EN)
    mod_som_sdio_init_f();
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

//#if defined(MOD_SOM_SETTINGS_EN)
//  mod_som_settings_stream_settings_f();
//#endif
//
//#if defined(MOD_SOM_SBE41_EN)
//  mod_som_sbe41_connect_f();
//  mod_som_sbe41_start_collect_data_f();
//#endif
//
//#if defined(MOD_SOM_EFE_EN)
//  mod_som_efe_sampling_f();
//#endif

//  printf("ok\r\n");

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

//printf("epsi sleep\r\n");


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
//  int delay =1000;

  if (mod_som_sleep_flag==false){
      mod_som_io_print_f("Making all modules are stopped \r\n");
//      mod_som_main_stop_modules_f();

      //Select intern HFRCO
//      CMU_OscillatorEnable(cmuOsc_HFRCO, true,true);
//      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

      //ALB Disable HFXO
      //ALB IF cmuSelect_HFRCO is NOT selected before it will reset the whole board
      /* Power External Oscillator SOM-U8-U4*/
      // HF oscillator disable.

      /* Freeze registers to avoid stalling for LF synchronization. */
      LEUART_FreezeEnable(LEUART0, true);

      LEUART0->CMD = LEUART_CMD_RXDIS | LEUART_CMD_TXDIS | LEUART_CMD_RXBLOCKEN
          | LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;
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

      //2025 08 22 trying to mitigate clock errors for LEUART0
      CMU_ClockEnable(cmuClock_GPIO, true);

      /* Enable CORE LE clock in order to access LE modules */
      CMU_ClockEnable(cmuClock_HFLE, true);
      //        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
      CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // Set a reference clock

      //ALB cmuClkDiv_1 works with baudrate 115200. Only for testing
      //        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_1);
      //ALB cmuClkDiv_4 works with baudrate 9600. apex mode
      CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1); // Don't prescale LEUART clock
      //        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_4);
      CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable device clock */

      // 2d) Clear any error/overflow that might have occurred during the gate
      LEUART0->IFC = LEUART_IF_FERR | LEUART_IF_PERR | LEUART_IF_RXOF;

      LEUART0->CMD = LEUART_CMD_RXEN | LEUART_CMD_TXEN | LEUART_CMD_RXBLOCKDIS
                        | LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;
      /* Freeze registers to avoid stalling for LF synchronization. */
      LEUART_FreezeEnable(LEUART0, false);


//      GPIO_PinModeSet(MOD_SOM_SBE41_EN_PORT, MOD_SOM_SBE41_EN_PIN,gpioModePushPull, 0);

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
 *   - trying to do some sort of power off
 *
 ******************************************************************************/
mod_som_status_t mod_som_main_wake_up_f()
{

  int delay =1000;

  if (mod_som_sleep_flag==true){
      mod_som_io_print_f("Waking up modules\r\n");

      /* Freeze registers to avoid stalling for LF synchronization. */
      LEUART_FreezeEnable(LEUART0, true);
      NVIC_DisableIRQ(LEUART0_IRQn);
      LEUART_IntDisable(LEUART0,~0x0);
      LEUART_IntClear(LEUART0, ~0x0);

      LEUART0->CMD = LEUART_CMD_RXDIS | LEUART_CMD_TXDIS | LEUART_CMD_RXBLOCKEN
          | LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;

      //ALB      DC/DC not burst mode  PF10 high
      GPIO_PinModeSet(gpioPortF, 10, gpioModePushPull, 1);

//      GPIO_PinModeSet(MOD_SOM_SBE41_EN_PORT, MOD_SOM_SBE41_EN_PIN, gpioModePushPull, 1);

      // turn dowm HFXO
      GPIO_PinModeSet(MOD_SOM_HFXO_EN_PORT, MOD_SOM_HFXO_EN_PIN, gpioModePushPull, 1);
//      //Select intern HFXO
      CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
      RETARGET_SerialFlush(); // Wait for UART TX buffer to be empty
      CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
      RETARGET_SerialInit(); // Re-enable VCOM
      sl_sleeptimer_delay_millisecond(delay/4);

//      // HFRCO oscillator disable.
//      CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
//
//      CMU_ClockEnable(cmuClock_HFLE, true);
      CMU_ClockEnable(cmuClock_HFPER, true);
//      CMU_ClockEnable(cmuClock_CORELE, true);


      //ALB Software reset of SDIO
      SDIO->CLOCKCTRL|=(_SDIO_CLOCKCTRL_SFTRSTA_MASK & SDIO_CLOCKCTRL_SFTRSTA);

      sl_sleeptimer_delay_millisecond(delay/4);

      //2025 08 22 SAN testing a fix for LEUART Timing when waking up

#if defined(_CMU_HFPERCLKEN0_MASK)
       CMU_ClockEnable(cmuClock_HFPER, true);
#endif
       CMU_ClockEnable(cmuClock_GPIO, true);

       /* Enable CORE LE clock in order to access LE modules */
       CMU_ClockEnable(cmuClock_HFLE, true);
//        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
       CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // Set a reference clock

       //ALB cmuClkDiv_1 works with baudrate 115200. Only for testing
//        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_1);
       //ALB cmuClkDiv_4 works with baudrate 9600. apex mode
       CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1); // Don't prescale LEUART clock
//        CMU_ClockDivSet(MOD_SOM_APF_USART_CLK,cmuClkDiv_4);
       CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable device clock */



       LEUART0->IFC = LEUART_IF_FERR | LEUART_IF_PERR | LEUART_IF_RXOF;
       LEUART0->CMD = LEUART_CMD_RXEN | LEUART_CMD_TXEN | LEUART_CMD_RXBLOCKDIS
                  | LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;

       //2025 08 28 SAN add a loopback to tickle the receive section with some signals
       LEUART0->CTRL |= LEUART_CTRL_LOOPBK;
       /* Freeze registers to avoid stalling for LF synchronization. */
       LEUART_FreezeEnable(LEUART0, false);


       sl_sleeptimer_delay_millisecond(delay/2);

       int i;

       char send_char = 0x07;//0x1b;//'A';
       char rx_char[21];
       for(i=0; i<10;i++){
           while (!(LEUART0->STATUS & LEUART_STATUS_TXBL));
           LEUART_Tx(LEUART0,  send_char);
//           send_char++;

           sl_sleeptimer_delay_millisecond(10);
           if((LEUART0->STATUS & _LEUART_STATUS_RXDATAV_MASK)){
               rx_char[i] = LEUART_Rx(LEUART0);
           }

       }

       while((LEUART0->STATUS & _LEUART_STATUS_RXDATAV_MASK)){
           rx_char[0] = LEUART_Rx(LEUART0);
       }
#ifdef MOD_SOM_DEBUG
       rx_char[20] = '\0';
       mod_som_io_print_f("rx_chars: %s\r\n",rx_char);
#else
       (void)rx_char;
#endif

       while (!(LEUART0->STATUS & LEUART_STATUS_TXC));
//       LEUART_FreezeEnable(LEUART0, true);
       LEUART0->CTRL &= ~LEUART_CTRL_LOOPBK;
//       LEUART_FreezeEnable(LEUART0, false);
       while(LEUART0->SYNCBUSY){

       }

      LEUART_IntClear(LEUART0, ~0x0);
      LEUART_IntEnable(LEUART0, LEUART_IF_RXDATAV);
      NVIC_EnableIRQ(LEUART0_IRQn);


      mod_som_sleep_flag=false;

  }

  return 0;
}


mod_som_status_t mod_som_main_power_off_f(){
  mod_som_status_t status;
  mod_som_apf_ptr_t mod_som_apf_ptr = mod_som_apf_get_runtime_ptr_f();
  if(mod_som_apf_ptr->daq){
       status= MOD_SOM_APF_STATUS_DAQ_ALREADY_STARTED;
       mod_som_apf_daq_stop_f();
   }
  status = mod_som_sdio_disable_hardware_f();
  status = mod_som_voltage_stop_scan_task_f();
  status = mod_som_voltage_stop_adc1_scan_task_f();
  status = mod_som_shell_stop_f();
  status = mod_som_apf_stop_shell_task_f();
  status = mod_som_io_stop_task_f();
  status = mod_som_main_task_stop_f();
  return MOD_SOM_APF_STATUS_OK;
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
    status = mod_som_main_task_start_f();
    //ALB stall if mod_som_main_start_f fail
    APP_RTOS_ASSERT_DBG(status == MOD_SOM_STATUS_OK, 1);

    return (1);
}
