/*******************************************************************************
 * @file mod_som.c
 * @brief MOD SOM board API implementation
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This implements the API for the MOD SOM board
 *
 ******************************************************************************/

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include "mod_som.h"
#include "mod_som_priv.h"
#include "mod_som_io.h"
#include "mod_som_io_priv.h"
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_shell.h"
#include "mod_som_shell_cmd.h"
#endif
#ifdef  MOD_SOM_SDIO_EN
#include "mod_som_sdio.h"
#endif

#include "retargetserial.h"

#if defined(HAL_CONFIG)
#include "retargetserialhalconfig.h"
#else
#include "retargetserialconfig.h"
#endif


//TODO remove once we have multiple LDMA ISR
#ifdef  MOD_SOM_EFE_EN
#include "mod_som_efe.h"
#endif
#ifdef  MOD_SOM_SBE49_EN
#include "mod_som_sbe49_priv.h"
#endif
#ifdef  MOD_SOM_SBE41_EN
#include "mod_som_sbe41_priv.h"
#endif
#ifdef  MOD_SOM_VEC_NAV_EN
#include "mod_som_vec_nav_priv.h"
#endif


#include <string.h>
#include <stdarg.h>

//------------------------------------------------------------------------------
// local variables
//------------------------------------------------------------------------------
static mod_som_sys_prf_list_ptr_t mod_som_sys_peripherals_list_ptr;

static mod_som_prf_list_item_ptr_t mod_som_prf_list_head_ptr;
static mod_som_prf_list_item_ptr_t mod_som_prf_list_tail_ptr;

static MEM_DYN_POOL mod_som_prf_dyn_mem_pool;
static bool mod_som_initialized_flag = false;

static CPU_STK mod_som_main_task_stack[MOD_SOM_MAIN_TASK_STK_SIZE];
static OS_TCB mod_som_main_task_tcb;


////AlB Structure to initialize the watchdog timer.
///* Defining the watchdog initialization data */
WDOG_Init_TypeDef wdog_init =
{
  .enable     = true,                 /* Start watchdog when init done */
  .debugRun   = false,                /* WDOG not counting during debug halt */
  .em2Run     = true,                 /* WDOG counting when in EM2 */
  .em3Run     = true,                 /* WDOG counting when in EM3 */
  .em4Block   = false,                /* EM4 can be entered */
  .swoscBlock = false,                /* Do not block disabling LFRCO/LFXO in CMU */
  .lock       = false,                /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
  .clkSel     = wdogClkSelULFRCO,     /* Select 1kHZ WDOG oscillator */
  .perSel     = wdogPeriod_32k,        /* Set the watchdog period to 2049 clock periods (ie ~2 seconds) */
};


//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   initialize the som board before OSCreateTask in main()
 *   - create a memory pool for the peripherals. TODO SN ask San for more details
 *   - Chip initialization routine for revision errata workarounds
 *   - pin state is released by calling this function (release any unforced latches)
 *   - Initialize CPU
 *   - selecting and setting clocks
 *   - enable HFRCO(19MHz) and HFXO(50MHz)
 *   - initialize the sleeptimer. The calendar date will be set up in the CALENDAR module
 *   - retarget to main serial port to match the SOM hardware (i.e. USART4)
 *   - initialize the kernel and check for kernel initialization errors
 ******************************************************************************/
mod_som_status_t mod_som_main_init_f(void){
    RTOS_ERR  err;

    //SN creating a memory pool to register all peripherals being used by MOD_SOM
    mod_som_sys_peripherals_list_ptr =
            (mod_som_sys_prf_list_ptr_t)Mem_SegAlloc(
                    "MOD SOM System peripheral list",DEF_NULL,
                    sizeof(mod_som_sys_prf_list_t),
                    &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);


    //    Mem_Set(&mod_som_sys_peripherals_list_ptr,0,sizeof(mod_som_sys_prf_list_t));
    // Creates a dynamic memory pool for peripherals
    Mem_DynPoolCreate(
            "MOD SOM peripheral list Dynamic Memory Pool",
            &mod_som_prf_dyn_mem_pool,
            DEF_NULL,
            sizeof(mod_som_prf_list_item_t),
            LIB_MEM_BUF_ALIGN_AUTO,
            16,
            128,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY);
    mod_som_prf_list_head_ptr = DEF_NULL;
    mod_som_prf_list_tail_ptr = DEF_NULL;

    //  Chip initialization routine for revision errata workarounds
    CHIP_Init();

    // pin state is released by calling this function (release any unforced latches)
    EMU_UnlatchPinRetention();
    // Initialize CPU
    CPU_Init();

    // selecting and setting clocks

    /* Use 19 MHZ HFRCO as core clock frequency*/
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
    CMU_ClockEnable(cmuClock_GPIO, true);

#if defined(MOD_SOM_BOARD)||defined(MOD_SOM_MEZZANINE_BOARD)
    /* Power External Oscillator SOM-U8-U4*/
    // HF oscillator enable high
    GPIO_PinModeSet(MOD_SOM_HFXO_EN_PORT, MOD_SOM_HFXO_EN_PIN, gpioModePushPull, 1);

    //initialize external crystal oscillator
    CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
    /*ALB change default HFXO mode to External*/
    hfxoInit.mode=cmuOscMode_External;
    CMU_HFXOInit(&hfxoInit); // Initialize the HFXO to ensure valid start state
    /* Starting HFXO and waiting until it is stable */
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);

    //50MHz
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Set new reference
#endif

    /* Starting LFRCO and waiting until it is stable */
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

    /* Enabling clock to the interface of the low energy modules and sleeptimer */
    // RTCC needs HFLE to be turned on before init for it to work
    CMU_ClockEnable(cmuClock_HFLE, true);

    // needed for sleep timer
    //ADD THIS LINE TO ENABLE 32768 LFXO ON SOM BY ASSERTING THE EM_BUCTRL (and not use the internal RC oscillator) MAG July 2021
    EMU_BUVoutResSet(EMU_BUCTRL_VOUTRES_WEAK);
//    CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFRCO); MHA removed - see below
    CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO); // MHA new line - see below

    CMU_ClockEnable(cmuClock_RTCC, true);

    //MHA NEW CLOCK ENABLE CODE FROM MAG July 2021
    ////////////////////////////////////////////////////
    //TODO ADD THIS LINE TO ENABLE 32768 LFXO ON SOM BY ASSERTING THE EM_BUCTRL (and not use the internal RC oscillator) MAG July 2021
   //   EMU_BUVoutResSet(EMU_BUCTRL_VOUTRES_WEAK);
    //TODO CHANGE THIS LINE TO:
    //   CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO); // Select the LFXO (32768 external crystal) powered by the battery backup domai
    //FROM OLD LINE:
    // CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFRCO);  // Set new reference

    //////////////////////////////////////////////////////////

    //init sleep timer (by silicon labs), we use it for delays,etc.
    sl_sleeptimer_init();

    //ALB init watchdog
    CMU_ClockEnable(cmuClock_CORELE, true);
    WDOG_Init(&wdog_init);
    /* Locking watchdog register (reset needed to unlock) */
//    WDOG_Lock();


//    /* Enable watchdog warning interrupt */
//    WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_WARN);
//    NVIC_EnableIRQ(WDOG0_IRQn);



#if defined(MOD_SOM_BOARD)
    /* enable Uart ports */
    GPIO_PinModeSet(MOD_SOM_UART_EN_PORT,\
                    MOD_SOM_UART_EN_PIN,\
                    gpioModePushPull, 1);    // URT_EN high
#endif
#if defined(MOD_SOM_MEZZANINE_BOARD)
    /* enable Uart ports */
    GPIO_PinModeSet(MOD_SOM_MEZZANINE_UART_EN_PORT, \
                    MOD_SOM_MEZZANINE_UART_EN_PIN, \
                    gpioModePushPull, 1);    // URT_EN high
    GPIO_PinModeSet(MOD_SOM_MEZZANINE_UART_VCC_EN_PORT,\
                    MOD_SOM_MEZZANINE_UART_VCC_EN_PIN,\
                    gpioModePushPull, 1);    // URT_EN high


#endif

    RETARGET_SerialInit();
    RETARGET_SerialCrLf(0);
    RETARGET_SerialFlush();

    OSInit(&err); // Initialize the Kernel

// mai bui - try to turn on the main shell - May 4, 2022
//    mod_som_main_com_on_f();

    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_MAIN);
    mod_som_initialized_flag = true;
    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   turn off main com
 ******************************************************************************/
mod_som_status_t mod_som_main_com_off_f(void){
  mod_som_io_stop_task_f();
  mod_som_shell_stop_f();

  RETARGET_SerialdeInit();
  return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   turn on main com
 ******************************************************************************/
mod_som_status_t mod_som_main_com_on_f(void){
  RETARGET_SerialInit();
   mod_som_io_start_f();
   mod_som_shell_start_f();

  return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   prep the SOM to sleep
 *   HFXO off
 *   CRYOstuff on
 ******************************************************************************/
mod_som_status_t mod_som_prep_sleep_f(void){
  return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   start main task
 *   - print date and time through main serial port
 *   - enable round robin scheme for handling task with same priority level.
 *   TODO MNB give more detail on the choice of round robin parameters
 *   - stall if round robin errors
 *   - Create main shell task: While loop waiting for user inputs.
 *   TODO SN comment of the choices of task parameter
 *   - Start the task.
 *   - stall if  OS start error
 ******************************************************************************/
mod_som_status_t mod_som_main_start_f(void){
    RTOS_ERR  err;
    if(!mod_som_initialized_flag)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_NOT_INITIALIZED_MAIN);


    printf("\r\n==============================\r\n");
    printf(PROJECTNAME);
    printf("\r\n");
    printf("Version: " __DATE__);
    printf(" ");
    printf(__TIME__);
    printf("\r\n");
    printf("==============================\r\n");

    // MNB round robin enbable from cbtest project
    // using Schedule Round Robin to handle 2 or more process has the same priority
    OSSchedRoundRobinCfg( DEF_TRUE,  /* DEF_TRUE to enable, DEF_FALSE to disable        */
                                     /* Round-Robin scheduling.                         */
                             10u,    /* Default time amount per task, in OS Ticks.      */
                            &err);
       if (err.Code != RTOS_ERR_NONE) {
           /*ALB  Handle error on Round-Robin Scheduler configuration. */
    	   //ALB stall if round robin errors
    	   APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    }
    OSTaskCreate(&mod_som_main_task_tcb, // Create the Start Task
            "Main Task",
            mod_som_main_task_f,
            DEF_NULL,
            MOD_SOM_MAIN_TASK_PRIORITY,
            &mod_som_main_task_stack[0],
            (MOD_SOM_MAIN_TASK_STK_SIZE / 10u),
            MOD_SOM_MAIN_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            &err);
    // Check error code
    //SN stall if Task create fails
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    OSStart(&err); // Start the kernel
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   initialize additional steps of the som board in the main_task_f()
 *   -CommonInit TODO SN comment on what Common_Init does.
 *   ALB I found out that Common Initneeds to be done after task create... I think
 *   -
 ******************************************************************************/
mod_som_status_t mod_som_main_task_init_f(void){
    RTOS_ERR  err;
    mod_som_status_t mod_som_status;
    //Initializes all the Common modules (Lib Mem, Lib Math, Logging, KAL, platform manager) in
    //the correct order with the specified configurations.
    Common_Init(&err);
    //enters CPU_SW_EXCEPTION if fails
    APP_RTOS_ASSERT_CRITICAL(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE,; );

    //Return error if common Init fails
    //ALB Not clear how it works with the previous line
    // TODO SN explain this
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE);

    // ALB initialize IO module
    // IO is a consumer. It will print to the main serial port the data gathered by the SOM.
    mod_som_status = mod_som_io_init_f();
    //enters CPU_SW_EXCEPTION if fails
    APP_RTOS_ASSERT_CRITICAL(mod_som_status == MOD_SOM_STATUS_OK,; );
    //Return error if mod_som_io_init_f fails
    //ALB Not clear how it works with the previous line
    if(mod_som_status != MOD_SOM_STATUS_OK)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE);


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    //ALB initialize pre-defined Micrium shell
    mod_som_status = mod_som_shell_init_f();
    //enters CPU_SW_EXCEPTION if fails
    APP_RTOS_ASSERT_CRITICAL(mod_som_status == MOD_SOM_STATUS_OK,; );
    //Return error if mod_som_shell_init_f fails
    if(mod_som_status != MOD_SOM_STATUS_OK)
    	return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE);
#endif


#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
    Clk_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE,; );
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE);
#endif

    // Initializes authentication module.
    // It has to do with User credential
    // TODO SN comment on the use of Auth_Init
    Auth_Init(&err);
    //enters CPU_SW_EXCEPTION if fails
    APP_RTOS_ASSERT_CRITICAL(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE,; );
    //Return error if fails
   if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE);

#if defined(RTOS_MODULE_IO_SD_AVAIL)
    IO_SD_CARD_CTRLR_REG("sd0", &BSP_SD_SDHC_BSP_DrvInfo);
#endif

    //start IO and shell here
    // create and start the streaming consumer (i.e IO task)
    mod_som_io_start_f();
#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    //ALB I am not startig the main shell task
    mod_som_shell_start_f();
    //ALB add MOD default shell commands.
    //mod_som_init_shellcmd_f(); //MHA uncomment this line to enable default shell commandsso
#endif

    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * @brief
 *   TODO SN Comment on the use of mod_som_add_peripheral_f

 *   - ALB I do not understand the need of this.
 ******************************************************************************/

mod_som_status_t mod_som_add_peripheral_f(mod_som_prf_ptr_t peripheral_ptr){

    mod_som_prf_list_item_ptr_t prf_list_item_ptr;
    bool need_to_add_flag = false;
    if(mod_som_prf_list_head_ptr == DEF_NULL){
        prf_list_item_ptr = mod_som_new_prf_list_item_f(peripheral_ptr);
        need_to_add_flag = true;
    }else{
        prf_list_item_ptr = mod_som_prf_list_head_ptr;
        need_to_add_flag = true;
        do {
            if(prf_list_item_ptr->prf_ptr == peripheral_ptr){
                need_to_add_flag = false;
                break;
            }
            if(prf_list_item_ptr->next_item_ptr == DEF_NULL)
                break;
            prf_list_item_ptr = prf_list_item_ptr->next_item_ptr;
        }while(prf_list_item_ptr->next_item_ptr != DEF_NULL);

    }
    if(need_to_add_flag){
        if(mod_som_prf_list_head_ptr == DEF_NULL){
            mod_som_prf_list_head_ptr=prf_list_item_ptr;
            mod_som_prf_list_tail_ptr=prf_list_item_ptr;
        }else{
            prf_list_item_ptr->prev_item_ptr = mod_som_prf_list_tail_ptr;
            mod_som_prf_list_tail_ptr->next_item_ptr = prf_list_item_ptr;
            mod_som_prf_list_tail_ptr=prf_list_item_ptr;
        }
    }

    switch ((uint32_t)peripheral_ptr->handle_port){
    case MSC_BASE:
        mod_som_sys_peripherals_list_ptr->MSC_prf_ptr = peripheral_ptr;
        break;
    case EMU_BASE:
        mod_som_sys_peripherals_list_ptr->EMU_prf_ptr = peripheral_ptr;
        break;
    case RMU_BASE:
        mod_som_sys_peripherals_list_ptr->RMU_prf_ptr = peripheral_ptr;
        break;
    case CMU_BASE:
        mod_som_sys_peripherals_list_ptr->CMU_prf_ptr = peripheral_ptr;
        break;
    case CRYPTO0_BASE:
        mod_som_sys_peripherals_list_ptr->CRYPTO0_prf_ptr = peripheral_ptr;
        break;
    case LESENSE_BASE:
        mod_som_sys_peripherals_list_ptr->LESENSE_prf_ptr = peripheral_ptr;
        break;
    case EBI_BASE:
        mod_som_sys_peripherals_list_ptr->EBI_prf_ptr = peripheral_ptr;
        break;
    case ETH_BASE:
        mod_som_sys_peripherals_list_ptr->ETH_prf_ptr = peripheral_ptr;
        break;
    case SDIO_BASE:
        mod_som_sys_peripherals_list_ptr->SDIO_prf_ptr = peripheral_ptr;
        break;
    case GPIO_BASE:
        mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr = peripheral_ptr;
        break;
    case PRS_BASE:
        mod_som_sys_peripherals_list_ptr->PRS_prf_ptr = peripheral_ptr;
        break;
    case LDMA_BASE:
        mod_som_sys_peripherals_list_ptr->LDMA_prf_ptr = peripheral_ptr;
        break;
    case FPUEH_BASE:
        mod_som_sys_peripherals_list_ptr->FPUEH_prf_ptr = peripheral_ptr;
        break;
    case GPCRC_BASE:
        mod_som_sys_peripherals_list_ptr->GPCRC_prf_ptr = peripheral_ptr;
        break;
    case CAN0_BASE:
        mod_som_sys_peripherals_list_ptr->CAN0_prf_ptr = peripheral_ptr;
        break;
    case CAN1_BASE:
        mod_som_sys_peripherals_list_ptr->CAN1_prf_ptr = peripheral_ptr;
        break;
    case TIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER0_prf_ptr = peripheral_ptr;
        break;
    case TIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER1_prf_ptr = peripheral_ptr;
        break;
    case TIMER2_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER2_prf_ptr = peripheral_ptr;
        break;
    case TIMER3_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER3_prf_ptr = peripheral_ptr;
        break;
    case TIMER4_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER4_prf_ptr = peripheral_ptr;
        break;
    case TIMER5_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr = peripheral_ptr;
        break;
    case TIMER6_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER6_prf_ptr = peripheral_ptr;
        break;
    case WTIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER0_prf_ptr = peripheral_ptr;
        break;
    case WTIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER1_prf_ptr = peripheral_ptr;
        break;
    case WTIMER2_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER2_prf_ptr = peripheral_ptr;
        break;
    case WTIMER3_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER3_prf_ptr = peripheral_ptr;
        break;
    case USART0_BASE:
        mod_som_sys_peripherals_list_ptr->USART0_prf_ptr = peripheral_ptr;
        break;
    case USART1_BASE:
        mod_som_sys_peripherals_list_ptr->USART1_prf_ptr = peripheral_ptr;
        break;
    case USART2_BASE:
        mod_som_sys_peripherals_list_ptr->USART2_prf_ptr = peripheral_ptr;
        break;
    case USART3_BASE:
        mod_som_sys_peripherals_list_ptr->USART3_prf_ptr = peripheral_ptr;
        break;
    case USART4_BASE:
        mod_som_sys_peripherals_list_ptr->USART4_prf_ptr = peripheral_ptr;
        break;
    case USART5_BASE:
        mod_som_sys_peripherals_list_ptr->USART5_prf_ptr = peripheral_ptr;
        break;
    case UART0_BASE:
        mod_som_sys_peripherals_list_ptr->UART0_prf_ptr = peripheral_ptr;
        break;
    case UART1_BASE:
        mod_som_sys_peripherals_list_ptr->UART1_prf_ptr = peripheral_ptr;
        break;
    case QSPI0_BASE:
        mod_som_sys_peripherals_list_ptr->QSPI0_prf_ptr = peripheral_ptr;
        break;
    case LEUART0_BASE:
        mod_som_sys_peripherals_list_ptr->LEUART0_prf_ptr = peripheral_ptr;
        break;
    case LEUART1_BASE:
        mod_som_sys_peripherals_list_ptr->LEUART1_prf_ptr = peripheral_ptr;
        break;
    case LETIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->LETIMER0_prf_ptr = peripheral_ptr;
        break;
    case LETIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->LETIMER1_prf_ptr = peripheral_ptr;
        break;
    case CRYOTIMER_BASE:
        mod_som_sys_peripherals_list_ptr->CRYOTIMER_prf_ptr = peripheral_ptr;
        break;
    case PCNT0_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT0_prf_ptr = peripheral_ptr;
        break;
    case PCNT1_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT1_prf_ptr = peripheral_ptr;
        break;
    case PCNT2_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT2_prf_ptr = peripheral_ptr;
        break;
    case I2C0_BASE:
        mod_som_sys_peripherals_list_ptr->I2C0_prf_ptr = peripheral_ptr;
        break;
    case I2C1_BASE:
        mod_som_sys_peripherals_list_ptr->I2C1_prf_ptr = peripheral_ptr;
        break;
    case I2C2_BASE:
        mod_som_sys_peripherals_list_ptr->I2C2_prf_ptr = peripheral_ptr;
        break;
    case ADC0_BASE:
        mod_som_sys_peripherals_list_ptr->ADC0_prf_ptr = peripheral_ptr;
        break;
    case ADC1_BASE:
        mod_som_sys_peripherals_list_ptr->ADC1_prf_ptr = peripheral_ptr;
        break;
    case ACMP0_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP0_prf_ptr = peripheral_ptr;
        break;
    case ACMP1_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP1_prf_ptr = peripheral_ptr;
        break;
    case ACMP2_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP2_prf_ptr = peripheral_ptr;
        break;
    case ACMP3_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP3_prf_ptr = peripheral_ptr;
        break;
    case VDAC0_BASE:
        mod_som_sys_peripherals_list_ptr->VDAC0_prf_ptr = peripheral_ptr;
        break;
    case USB_BASE:
        mod_som_sys_peripherals_list_ptr->USB_prf_ptr = peripheral_ptr;
        break;
    case IDAC0_BASE:
        mod_som_sys_peripherals_list_ptr->IDAC0_prf_ptr = peripheral_ptr;
        break;
    case CSEN_BASE:
        mod_som_sys_peripherals_list_ptr->CSEN_prf_ptr = peripheral_ptr;
        break;
    case LCD_BASE:
        mod_som_sys_peripherals_list_ptr->LCD_prf_ptr = peripheral_ptr;
        break;
    case RTC_BASE:
        mod_som_sys_peripherals_list_ptr->RTC_prf_ptr = peripheral_ptr;
        break;
    case RTCC_BASE:
        mod_som_sys_peripherals_list_ptr->RTCC_prf_ptr = peripheral_ptr;
        break;
    case WDOG0_BASE:
        mod_som_sys_peripherals_list_ptr->WDOG0_prf_ptr = peripheral_ptr;
        break;
    case WDOG1_BASE:
        mod_som_sys_peripherals_list_ptr->WDOG1_prf_ptr = peripheral_ptr;
        break;
    case ETM_BASE:
        mod_som_sys_peripherals_list_ptr->ETM_prf_ptr = peripheral_ptr;
        break;
    case SMU_BASE:
        mod_som_sys_peripherals_list_ptr->SMU_prf_ptr = peripheral_ptr;
        break;
    case TRNG0_BASE:
        mod_som_sys_peripherals_list_ptr->TRNG0_prf_ptr = peripheral_ptr;
        break;
    case DEVINFO_BASE:
        mod_som_sys_peripherals_list_ptr->DEVINFO_prf_ptr = peripheral_ptr;
        break;
    case ROMTABLE_BASE:
        mod_som_sys_peripherals_list_ptr->ROMTABLE_prf_ptr = peripheral_ptr;
        break;
    default:
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_FIND_BASE_PRF);
        break;
    }
    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   TODO SN Comment on the use of mod_som_rmv_peripheral_f

 *   - ALB I do not understand the need of this.
 ******************************************************************************/

mod_som_status_t mod_som_rmv_peripheral_f(mod_som_prf_ptr_t peripheral_ptr){
    mod_som_prf_list_item_ptr_t prf_list_item_ptr;
    mod_som_status_t mod_som_status;
    bool found_flag = false;
    if(mod_som_prf_list_head_ptr == DEF_NULL){
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_QUEUE_EMPTY);
    }else{
        prf_list_item_ptr = mod_som_prf_list_head_ptr;
        do {
            if(prf_list_item_ptr->prf_ptr == peripheral_ptr){ //remove item from list
                ((mod_som_prf_list_item_ptr_t)(prf_list_item_ptr->prev_item_ptr))->next_item_ptr = prf_list_item_ptr->next_item_ptr;
                ((mod_som_prf_list_item_ptr_t)(prf_list_item_ptr->next_item_ptr))->prev_item_ptr = prf_list_item_ptr->prev_item_ptr;
                if(prf_list_item_ptr == mod_som_prf_list_head_ptr){
                    mod_som_prf_list_head_ptr = prf_list_item_ptr->next_item_ptr;
                }
                if(prf_list_item_ptr == mod_som_prf_list_tail_ptr){
                    mod_som_prf_list_tail_ptr = prf_list_item_ptr->prev_item_ptr;
                }
                mod_som_status = mod_som_free_prf_list_item_f(prf_list_item_ptr);
                if(mod_som_status != MOD_SOM_STATUS_OK)
                    return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_FREE_MEMORY);
                found_flag = true;
                break;
            }
            //end of list
            if(prf_list_item_ptr->next_item_ptr == DEF_NULL)
                break;
            if(!found_flag)
                return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_FIND_ITEM_IN_QUEUE);
            prf_list_item_ptr = prf_list_item_ptr->next_item_ptr;
        }while(prf_list_item_ptr->next_item_ptr != DEF_NULL);
    }

    switch ((uint32_t)peripheral_ptr->handle_port){
    case MSC_BASE:
        mod_som_sys_peripherals_list_ptr->MSC_prf_ptr = DEF_NULL;
        break;
    case EMU_BASE:
        mod_som_sys_peripherals_list_ptr->EMU_prf_ptr = DEF_NULL;
        break;
    case RMU_BASE:
        mod_som_sys_peripherals_list_ptr->RMU_prf_ptr = DEF_NULL;
        break;
    case CMU_BASE:
        mod_som_sys_peripherals_list_ptr->CMU_prf_ptr = DEF_NULL;
        break;
    case CRYPTO0_BASE:
        mod_som_sys_peripherals_list_ptr->CRYPTO0_prf_ptr = DEF_NULL;
        break;
    case LESENSE_BASE:
        mod_som_sys_peripherals_list_ptr->LESENSE_prf_ptr = DEF_NULL;
        break;
    case EBI_BASE:
        mod_som_sys_peripherals_list_ptr->EBI_prf_ptr = DEF_NULL;
        break;
    case ETH_BASE:
        mod_som_sys_peripherals_list_ptr->ETH_prf_ptr = DEF_NULL;
        break;
    case SDIO_BASE:
        mod_som_sys_peripherals_list_ptr->SDIO_prf_ptr = DEF_NULL;
        break;
    case GPIO_BASE:
        mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr = DEF_NULL;
        break;
    case PRS_BASE:
        mod_som_sys_peripherals_list_ptr->PRS_prf_ptr = DEF_NULL;
        break;
    case LDMA_BASE:
        mod_som_sys_peripherals_list_ptr->LDMA_prf_ptr = DEF_NULL;
        break;
    case FPUEH_BASE:
        mod_som_sys_peripherals_list_ptr->FPUEH_prf_ptr = DEF_NULL;
        break;
    case GPCRC_BASE:
        mod_som_sys_peripherals_list_ptr->GPCRC_prf_ptr = DEF_NULL;
        break;
    case CAN0_BASE:
        mod_som_sys_peripherals_list_ptr->CAN0_prf_ptr = DEF_NULL;
        break;
    case CAN1_BASE:
        mod_som_sys_peripherals_list_ptr->CAN1_prf_ptr = DEF_NULL;
        break;
    case TIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER0_prf_ptr = DEF_NULL;
        break;
    case TIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER1_prf_ptr = DEF_NULL;
        break;
    case TIMER2_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER2_prf_ptr = DEF_NULL;
        break;
    case TIMER3_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER3_prf_ptr = DEF_NULL;
        break;
    case TIMER4_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER4_prf_ptr = DEF_NULL;
        break;
    case TIMER5_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr = DEF_NULL;
        break;
    case TIMER6_BASE:
        mod_som_sys_peripherals_list_ptr->TIMER6_prf_ptr = DEF_NULL;
        break;
    case WTIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER0_prf_ptr = DEF_NULL;
        break;
    case WTIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER1_prf_ptr = DEF_NULL;
        break;
    case WTIMER2_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER2_prf_ptr = DEF_NULL;
        break;
    case WTIMER3_BASE:
        mod_som_sys_peripherals_list_ptr->WTIMER3_prf_ptr = DEF_NULL;
        break;
    case USART0_BASE:
        mod_som_sys_peripherals_list_ptr->USART0_prf_ptr = DEF_NULL;
        break;
    case USART1_BASE:
        mod_som_sys_peripherals_list_ptr->USART1_prf_ptr = DEF_NULL;
        break;
    case USART2_BASE:
        mod_som_sys_peripherals_list_ptr->USART2_prf_ptr = DEF_NULL;
        break;
    case USART3_BASE:
        mod_som_sys_peripherals_list_ptr->USART3_prf_ptr = DEF_NULL;
        break;
    case USART4_BASE:
        mod_som_sys_peripherals_list_ptr->USART4_prf_ptr = DEF_NULL;
        break;
    case USART5_BASE:
        mod_som_sys_peripherals_list_ptr->USART5_prf_ptr = DEF_NULL;
        break;
    case UART0_BASE:
        mod_som_sys_peripherals_list_ptr->UART0_prf_ptr = DEF_NULL;
        break;
    case UART1_BASE:
        mod_som_sys_peripherals_list_ptr->UART1_prf_ptr = DEF_NULL;
        break;
    case QSPI0_BASE:
        mod_som_sys_peripherals_list_ptr->QSPI0_prf_ptr = DEF_NULL;
        break;
    case LEUART0_BASE:
        mod_som_sys_peripherals_list_ptr->LEUART0_prf_ptr = DEF_NULL;
        break;
    case LEUART1_BASE:
        mod_som_sys_peripherals_list_ptr->LEUART1_prf_ptr = DEF_NULL;
        break;
    case LETIMER0_BASE:
        mod_som_sys_peripherals_list_ptr->LETIMER0_prf_ptr = DEF_NULL;
        break;
    case LETIMER1_BASE:
        mod_som_sys_peripherals_list_ptr->LETIMER1_prf_ptr = DEF_NULL;
        break;
    case CRYOTIMER_BASE:
        mod_som_sys_peripherals_list_ptr->CRYOTIMER_prf_ptr = DEF_NULL;
        break;
    case PCNT0_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT0_prf_ptr = DEF_NULL;
        break;
    case PCNT1_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT1_prf_ptr = DEF_NULL;
        break;
    case PCNT2_BASE:
        mod_som_sys_peripherals_list_ptr->PCNT2_prf_ptr = DEF_NULL;
        break;
    case I2C0_BASE:
        mod_som_sys_peripherals_list_ptr->I2C0_prf_ptr = DEF_NULL;
        break;
    case I2C1_BASE:
        mod_som_sys_peripherals_list_ptr->I2C1_prf_ptr = DEF_NULL;
        break;
    case I2C2_BASE:
        mod_som_sys_peripherals_list_ptr->I2C2_prf_ptr = DEF_NULL;
        break;
    case ADC0_BASE:
        mod_som_sys_peripherals_list_ptr->ADC0_prf_ptr = DEF_NULL;
        break;
    case ADC1_BASE:
        mod_som_sys_peripherals_list_ptr->ADC1_prf_ptr = DEF_NULL;
        break;
    case ACMP0_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP0_prf_ptr = DEF_NULL;
        break;
    case ACMP1_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP1_prf_ptr = DEF_NULL;
        break;
    case ACMP2_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP2_prf_ptr = DEF_NULL;
        break;
    case ACMP3_BASE:
        mod_som_sys_peripherals_list_ptr->ACMP3_prf_ptr = DEF_NULL;
        break;
    case VDAC0_BASE:
        mod_som_sys_peripherals_list_ptr->VDAC0_prf_ptr = DEF_NULL;
        break;
    case USB_BASE:
        mod_som_sys_peripherals_list_ptr->USB_prf_ptr = DEF_NULL;
        break;
    case IDAC0_BASE:
        mod_som_sys_peripherals_list_ptr->IDAC0_prf_ptr = DEF_NULL;
        break;
    case CSEN_BASE:
        mod_som_sys_peripherals_list_ptr->CSEN_prf_ptr = DEF_NULL;
        break;
    case LCD_BASE:
        mod_som_sys_peripherals_list_ptr->LCD_prf_ptr = DEF_NULL;
        break;
    case RTC_BASE:
        mod_som_sys_peripherals_list_ptr->RTC_prf_ptr = DEF_NULL;
        break;
    case RTCC_BASE:
        mod_som_sys_peripherals_list_ptr->RTCC_prf_ptr = DEF_NULL;
        break;
    case WDOG0_BASE:
        mod_som_sys_peripherals_list_ptr->WDOG0_prf_ptr = DEF_NULL;
        break;
    case WDOG1_BASE:
        mod_som_sys_peripherals_list_ptr->WDOG1_prf_ptr = DEF_NULL;
        break;
    case ETM_BASE:
        mod_som_sys_peripherals_list_ptr->ETM_prf_ptr = DEF_NULL;
        break;
    case SMU_BASE:
        mod_som_sys_peripherals_list_ptr->SMU_prf_ptr = DEF_NULL;
        break;
    case TRNG0_BASE:
        mod_som_sys_peripherals_list_ptr->TRNG0_prf_ptr = DEF_NULL;
        break;
    case DEVINFO_BASE:
        mod_som_sys_peripherals_list_ptr->DEVINFO_prf_ptr = DEF_NULL;
        break;
    case ROMTABLE_BASE:
        mod_som_sys_peripherals_list_ptr->ROMTABLE_prf_ptr = DEF_NULL;
        break;
    default:
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_FIND_BASE_PRF);
        break;

    }
    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   TODO SN Comment on the use of mod_som_calculate_data_block_size_f

 *   - ALB This is part of the STREAM and STORE consumer as of 10/16/2020.
 *   - It gets the length of a full data block generated by the "consummer" (mod_som_generate_data_block_f)
 ******************************************************************************/

uint32_t mod_som_calculate_data_block_size_f(uint32_t header_length, uint32_t data_length){
    return 3+ // \r\n$
            header_length +
            16 +        //timestamp
            data_length+
            5;          //checksum size (*FF) + \r\n
}


uint32_t mod_som_generate_data_block_f(
        uint8_t * data_block_ptr,
        uint8_t *data_header_ptr, uint32_t data_header_length,
        uint8_t *data_ptr, uint32_t data_length,
        uint64_t data_timestamp){

    uint32_t data_block_length = 0;
    uint32_t chksum = 0,i;
    data_block_length = 0;

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    *(data_block_ptr++) = '\r';
    *(data_block_ptr++) = '\n';
    data_block_length +=2;
    chksum ^= (*(data_block_ptr++) = '$');

    data_block_length++;

    //data header
    for(i=0;i<data_header_length;i++){
        chksum ^= (uint8_t)data_header_ptr[i];
        *(data_block_ptr++) = data_header_ptr[i];
    }

    data_block_length += data_header_length;

    //time stamp
    uint64_t t_hex;

    t_hex = mod_som_int32_2hex_f((uint32_t) (data_timestamp>>32));
    // had to copy twice, because complier doesn't allow to copy 64bit, weird //TODO
    ((uint32_t*) data_block_ptr)[0] = ((uint32_t*)&t_hex)[0];
    ((uint32_t*) data_block_ptr)[1] = ((uint32_t*)&t_hex)[1];

    t_hex = mod_som_int32_2hex_f((uint32_t) data_timestamp);
    ((uint32_t*) (data_block_ptr+8))[0] = ((uint32_t*)&t_hex)[0];
    ((uint32_t*) (data_block_ptr+8))[1] = ((uint32_t*)&t_hex)[1];

    for(i=0;i<16;i++){
        chksum ^= *((uint8_t*)data_block_ptr++);
    }
    data_block_length += 16;

    //data
    for(i=0;i<data_length;i++){
        chksum ^= data_ptr[i];
        *(data_block_ptr++) = data_ptr[i];
    }

    data_block_length += data_length;

    //checksum
    *(data_block_ptr++) = '*';
    *((uint16_t*)data_block_ptr) =
            mod_som_int8_2hex_f((uint8_t)chksum);
    data_block_length += 3;
    data_block_ptr += 2;
    *(data_block_ptr++) = '\r';
    *(data_block_ptr++) = '\n';
    data_block_length += 2;
    CORE_EXIT_ATOMIC();
    return data_block_length;

}
/*******************************************************************************
 * @function
 *     mod_som_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM error codes
 * @discussion
 *     The status is system wide, so we only decode the last 16 bits if the
 *     higher bits show the status code is of MOD SOM I/O
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xffff if non-MOD SOM status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_io_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     high 8 bits are system identifier, the low 8 bits are the status code
 *     according each system
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_STATUS_PREFIX, mod_som_io_status);
}


/*******************************************************************************
 * @function
 *      mod_som_new_prf_list_item_f
 * @abstract
 * TODO SN Comment on the use of  mod_som_new_prf_list_item_f.
 ******************************************************************************/

mod_som_prf_list_item_ptr_t mod_som_new_prf_list_item_f(mod_som_prf_ptr_t peripheral_ptr){
    RTOS_ERR err;
    if(!mod_som_initialized_flag)
        mod_som_main_init_f();
    mod_som_prf_list_item_ptr_t mod_som_prf_list_item_ptr =
            (mod_som_prf_list_item_ptr_t)Mem_DynPoolBlkGet(
                    &mod_som_prf_dyn_mem_pool,
                    &err);
    // Check error code
    //ALB Stall if error in Mem_DynPoolBlkGet
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    //ALB return def_null if error in Stall if error in Mem_DynPoolBlkGet
    if(mod_som_prf_list_item_ptr==DEF_NULL)
        return DEF_NULL;

    Mem_Set(mod_som_prf_list_item_ptr,0,sizeof(mod_som_prf_list_item_t));
    mod_som_prf_list_item_ptr->prf_ptr = peripheral_ptr;
    mod_som_prf_list_item_ptr->next_item_ptr = DEF_NULL;
    mod_som_prf_list_item_ptr->prev_item_ptr = DEF_NULL;
    //this is so we can find the wrapper class in case we need to clear memory
    return mod_som_prf_list_item_ptr;
}

mod_som_status_t mod_som_free_prf_list_item_f(mod_som_prf_list_item_ptr_t prf_list_item_ptr){
    RTOS_ERR err;
    if(!mod_som_initialized_flag)
        mod_som_main_init_f();
    Mem_DynPoolBlkFree(&mod_som_prf_dyn_mem_pool,
            (void *)prf_list_item_ptr,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_encode_status_f(MOD_SOM_STATUS_ERR_FAIL_TO_FREE_MEMORY);

    return mod_som_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @function
 *     mod_som_int32_2hex_f
 * @abstract
 *     Converts 32-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 32-bit number
 * @return
 *     A 64-bit ASCII array that represents 8 characters
 ******************************************************************************/
uint64_t mod_som_int32_2hex_f(uint32_t bin){
    uint64_t x, m;
    uint32_t b = bin;
    //bit flip
    //  b = (((b & 0xaaaaaaaa)>>1) |((b & 0x55555555)<<1));
    //  b = (((b & 0xcccccccc)>>2) |((b & 0x33333333)<<2));
    //byte flip
    b = (((b & 0xf0f0f0f0)>>4) |((b & 0x0f0f0f0f)<<4));
    b = (((b & 0xff00ff00)>>8) |((b & 0x00ff00ff)<<8));//
    b = ((b>>16)|(b<<16));
    x = b;
    x = ((x & 0x00000000ffff0000LL) << 16) | (x & 0x000000000000ffffLL);
    x = ((x & 0x0000ff000000ff00LL) << 8)  | (x & 0x000000ff000000ffLL);
    x = ((x & 0x00f000f000f000f0LL) << 4)  | (x & 0x000f000f000f000fLL);
    x += 0x0606060606060606LL;
    m = ((x & 0x1010101010101010LL) >> 4) + 0x7f7f7f7f7f7f7f7fLL;
    x += (m & 0x2a2a2a2a2a2a2a2aLL) | (~m & 0x3131313131313131LL);
    return x;
}

/*******************************************************************************
 * @function
 *     mod_som_int16_2hex_f
 * @abstract
 *     Converts 16-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 16-bit number
 * @return
 *     A 32-bit ASCII array that represents 4 characters
 ******************************************************************************/
uint32_t mod_som_int16_2hex_f(uint16_t bin){
    uint32_t x, m;
    uint16_t b = bin;
    //bit flip
    //  b = (((b & 0xaaaa)>>1) |((b & 0x5555)<<1));
    //  b = (((b & 0xcccc)>>2) |((b & 0x3333)<<2));
    //byte flip
    b = (((b & 0xf0f0)>>4) |((b & 0x0f0f)<<4));
    b = ((b>>8)|(b<<8));
    x = b;
    //  x = ((x & 0x00000000ffff0000LL) << 16) | (x & 0x000000000000ffffLL);
    x = ((x & 0x0000ff00L) << 8)  | (x & 0x000000ffL);
    x = ((x & 0x00f000f0L) << 4)  | (x & 0x000f000fL);
    x += 0x06060606L;
    m = ((x & 0x10101010L) >> 4) + 0x7f7f7f7fL;
    x += (m & 0x2a2a2a2aL) | (~m & 0x31313131L);
    return x;
}

/*******************************************************************************
 * @function
 *     mod_som_int8_2hex_f
 * @abstract
 *     Converts 8-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses bit shifting to convert to hexadecimal
 * @param       bin
 *     binary 8-bit number
 * @return
 *     A 16-bit ASCII array that represents 2 characters
 ******************************************************************************/
uint16_t mod_som_int8_2hex_f(uint8_t bin){
    uint16_t x, m;

    x = bin;
    x = ((x & 0x00f0L) << 4)  | (x & 0x000fL);
    x += 0x0606L;
    m = ((x & 0x1010L) >> 4) + 0x7f7fL;
    x += (m & 0x2a2aL) | (~m & 0x3131L);
    x=(x>>8) | (x<<8);

    return x;
}

/*******************************************************************************
 * @function
 *     mod_som_lut_hex_str_f
 * @abstract
 *     Converts 32-bit integer to hexadecimal ASCII representation
 * @discussion
 *     This function uses a lookup table to convert to hexadecimal
 * @param       num
 *     binary 32-bit number
 * @param       s
 *     converted array of characters
 * @param
 *     lower_case indicator of lower or upper case of the alphabet a-f
 * @return
 *     success status
 ******************************************************************************/
uint32_t mod_som_lut_hex_str_f(uint32_t num, char *s, bool lower_case)
{
    static const char digits[513] =
            "000102030405060708090A0B0C0D0E0F"
            "101112131415161718191A1B1C1D1E1F"
            "202122232425262728292A2B2C2D2E2F"
            "303132333435363738393A3B3C3D3E3F"
            "404142434445464748494A4B4C4D4E4F"
            "505152535455565758595A5B5C5D5E5F"
            "606162636465666768696A6B6C6D6E6F"
            "707172737475767778797A7B7C7D7E7F"
            "808182838485868788898A8B8C8D8E8F"
            "909192939495969798999A9B9C9D9E9F"
            "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
            "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
            "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
            "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
            "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
            "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    static const char digits_lowercase[513] =
            "000102030405060708090a0b0c0d0e0f"
            "101112131415161718191a1b1c1d1e1f"
            "202122232425262728292a2b2c2d2e2f"
            "303132333435363738393a3b3c3d3e3f"
            "404142434445464748494a4b4c4d4e4f"
            "505152535455565758595a5b5c5d5e5f"
            "606162636465666768696a6b6c6d6e6f"
            "707172737475767778797a7b7c7d7e7f"
            "808182838485868788898a8b8c8d8e8f"
            "909192939495969798999a9b9c9d9e9f"
            "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
            "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
            "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
            "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
            "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
            "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
    uint32_t x = num;
    int i = 3;
    char *lut = (char *)((lower_case) ? digits_lowercase : digits);
    while (i >= 0)
    {
        int pos = (x & 0xFF) * 2;
        char ch = lut[pos];
        s[i * 2] = ch;

        ch = lut[pos + 1];
        s[i * 2 + 1] = ch;

        x >>= 8;
        i -= 1;
    }

    return 0;
}


void EMU_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->EMU_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->EMU_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->EMU_prf_ptr->device_ptr);
    }
    return;
}
//void WDOG0_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->WDOG0_prf_ptr != DEF_NULL){
//        mod_som_sys_peripherals_list_ptr->WDOG0_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->WDOG0_prf_ptr->device_ptr);
//    }
//    return;
//}
//ALB comment to make EFE and SBE module work
// TODO create different IRQ handler with Mike
void LDMA_IRQHandler(void){

  //ALB so far I am hard coding the ch bit position.
  //ALB lets check for automatic channel selection.
  uint8_t ch0_bit_position=1;
  uint8_t ch1_bit_position=2;
  uint8_t ch2_bit_position=4;
  uint8_t ch3_bit_position=8;
  uint32_t pending;

  /* Read interrupt source */
  pending = LDMA->IF;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  /* clear interrupt source */
  LDMA->IFC;
  if (pending & ch0_bit_position){
      LDMA_IntClear(ch0_bit_position);
      mod_som_efe_ldma_irq_handler_f();

  }
  if(pending & ch1_bit_position){
      LDMA_IntClear(ch1_bit_position);
      mod_som_sbe41_ldma_irq_handler_f();
  }
  if(pending & ch2_bit_position){
      LDMA_IntClear(ch2_bit_position);
#ifdef MOD_SOM_VECNAV_EN
      mod_som_vecnav_ldma_irq_handler_f();
#endif
  }
  if(pending & ch3_bit_position){
      LDMA_IntClear(ch3_bit_position);
      mod_som_io_ldma_irq_handler_f();
  }

  CORE_EXIT_ATOMIC();


    return;
}
//ALB end comment

//void GPIO_EVEN_IRQHandler(void){
//if(mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr != DEF_NULL){
//    mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr->irq_handler_1_f(
//            (void *)mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr->device_ptr);
//}
//    return;
//}
void SMU_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->SMU_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->SMU_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->SMU_prf_ptr->device_ptr);
    }
    return;
}
void TIMER0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER0_prf_ptr->device_ptr);
    }
    return;
}
void USART0_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART0_prf_ptr->device_ptr);
    }
    return;
}
void USART0_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART0_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART0_prf_ptr->device_ptr);
    }
    return;
}
void ACMP0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->ACMP0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->ACMP0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->ACMP0_prf_ptr->device_ptr);
    }
    return;
}
//void ADC0_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->ADC0_prf_ptr != DEF_NULL){
//        mod_som_sys_peripherals_list_ptr->ADC0_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->ADC0_prf_ptr->device_ptr);
//    }
//    return;
//}
void IDAC0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->IDAC0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->IDAC0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->IDAC0_prf_ptr->device_ptr);
    }
    return;
}
void I2C0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->I2C0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->I2C0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->I2C0_prf_ptr->device_ptr);
    }
    return;
}
void I2C1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->I2C1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->I2C1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->I2C1_prf_ptr->device_ptr);
    }
    return;
}
//void GPIO_ODD_IRQHandler(void){
//if(mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr != DEF_NULL){
//    mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr->irq_handler_2_f(
//            (void *)mod_som_sys_peripherals_list_ptr->GPIO_prf_ptr->device_ptr);
//    return;
//}
//}
void TIMER1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER1_prf_ptr->device_ptr);
    }
    return;
}
void TIMER2_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER2_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER2_prf_ptr->device_ptr);
    }
    return;
}
void TIMER3_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER3_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER3_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER3_prf_ptr->device_ptr);
    }
    return;
}
void USART1_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART1_prf_ptr->device_ptr);
    }
    return;
}
void USART1_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART1_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART1_prf_ptr->device_ptr);
    }
    return;
}
void USART2_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART2_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART2_prf_ptr->device_ptr);
    }
    return;
}
void USART2_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART2_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART2_prf_ptr->device_ptr);
    }
    return;
}
void UART0_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->UART0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->UART0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->UART0_prf_ptr->device_ptr);
    }
    return;
}
void UART0_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->UART0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->UART0_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->UART0_prf_ptr->device_ptr);
    }
    return;
}
//ALB comment to make the retarget serial port works on the SOM MEZZANINE REV2
//void UART1_RX_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->UART1_prf_ptr != DEF_NULL){
//        mod_som_sys_peripherals_list_ptr->UART1_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->UART1_prf_ptr->device_ptr);
//    }
//    return;
//}
void UART1_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->UART1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->UART1_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->UART1_prf_ptr->device_ptr);
    }
    return;
}
//void LEUART0_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->LEUART0_prf_ptr != DEF_NULL){
//        //        printf("LEUART0_IRQHandler();\r\n");
//        mod_som_sys_peripherals_list_ptr->LEUART0_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->LEUART0_prf_ptr->device_ptr);
//        //    printf("LEUART0_IRQHandler();\r\n");
//    }
//    return;
//}
void LEUART1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->LEUART1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->LEUART1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->LEUART1_prf_ptr->device_ptr);
    }
    return;
}
void LETIMER0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->LETIMER0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->LETIMER0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->LETIMER0_prf_ptr->device_ptr);
    }
    return;
}
void PCNT0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->PCNT0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->PCNT0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->PCNT0_prf_ptr->device_ptr);
    }
    return;
}
void PCNT1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->PCNT1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->PCNT1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->PCNT1_prf_ptr->device_ptr);
    }
    return;
}
void PCNT2_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->PCNT2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->PCNT2_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->PCNT2_prf_ptr->device_ptr);
    }
    return;
}
//void RTCC_IRQHandler(void){
//if(mod_som_sys_peripherals_list_ptr->RTCC_prf_ptr != DEF_NULL){
//    mod_som_sys_peripherals_list_ptr->RTCC_prf_ptr->irq_handler_1_f(
//            (void *)mod_som_sys_peripherals_list_ptr->RTCC_prf_ptr->device_ptr);
//}
//    return;
//}
void CMU_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CMU_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CMU_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CMU_prf_ptr->device_ptr);
    }
    return;
}
void MSC_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->MSC_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->MSC_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->MSC_prf_ptr->device_ptr);
    }
    return;
}
void CRYPTO0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CRYPTO0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CRYPTO0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CRYPTO0_prf_ptr->device_ptr);
    }
    return;
}
void CRYOTIMER_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CRYOTIMER_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CRYOTIMER_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CRYOTIMER_prf_ptr->device_ptr);
    }
    return;
}
void FPUEH_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->FPUEH_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->FPUEH_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->FPUEH_prf_ptr->device_ptr);
    }
    return;
}
void USART3_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART3_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART3_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART3_prf_ptr->device_ptr);
    }
    return;
}
void USART3_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART3_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART3_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART3_prf_ptr->device_ptr);
    }
    return;
}
void USART4_RX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART4_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART4_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART4_prf_ptr->device_ptr);
    }
    return;
}
void USART4_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART4_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART4_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART4_prf_ptr->device_ptr);
    }
    return;
}
void WTIMER0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->WTIMER0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->WTIMER0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->WTIMER0_prf_ptr->device_ptr);
    }
    return;
}
void WTIMER1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->WTIMER1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->WTIMER1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->WTIMER1_prf_ptr->device_ptr);
    }
    return;
}
//void WTIMER2_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->WTIMER2_prf_ptr != DEF_NULL){
//        mod_som_sys_peripherals_list_ptr->WTIMER2_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->WTIMER2_prf_ptr->device_ptr);
//    }
//    return;
//}
//void WTIMER3_IRQHandler(void){
//    if(mod_som_sys_peripherals_list_ptr->WTIMER3_prf_ptr != DEF_NULL){
//        mod_som_sys_peripherals_list_ptr->WTIMER3_prf_ptr->irq_handler_1_f(
//                (void *)mod_som_sys_peripherals_list_ptr->WTIMER3_prf_ptr->device_ptr);
//    }
//    return;
//}
void I2C2_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->I2C2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->I2C2_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->I2C2_prf_ptr->device_ptr);
    }
    return;
}
void VDAC0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->VDAC0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->VDAC0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->VDAC0_prf_ptr->device_ptr);
    }
    return;
}
void TIMER4_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER4_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER4_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER4_prf_ptr->device_ptr);
    }
    return;
}
void TIMER5_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr->device_ptr);
    }
    return;
}
void TIMER6_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TIMER5_prf_ptr->device_ptr);
    }
    return;
}
//void USART5_RX_IRQHandler(void){
//if(mod_som_sys_peripherals_list_ptr->USART5_prf_ptr != DEF_NULL){
//    mod_som_sys_peripherals_list_ptr->USART5_prf_ptr->irq_handler_1_f(
//            (void *)mod_som_sys_peripherals_list_ptr->USART5_prf_ptr->device_ptr);
//}
//    return;
//}
void USART5_TX_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USART5_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USART5_prf_ptr->irq_handler_2_f(
                (void *)mod_som_sys_peripherals_list_ptr->USART5_prf_ptr->device_ptr);
    }
    return;
}
void CSEN_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CSEN_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CSEN_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CSEN_prf_ptr->device_ptr);
    }
    return;
}
void LESENSE_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->LESENSE_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->LESENSE_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->LESENSE_prf_ptr->device_ptr);
    }
    return;
}
void EBI_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->EBI_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->EBI_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->EBI_prf_ptr->device_ptr);
    }
    return;
}
void ACMP2_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->ACMP2_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->ACMP2_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->ACMP2_prf_ptr->device_ptr);
    }
    return;
}
void ADC1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->ADC1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->ADC1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->ADC1_prf_ptr->device_ptr);
    }
    return;
}
void LCD_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->LCD_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->LCD_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->LCD_prf_ptr->device_ptr);
    }
    return;
}
void SDIO_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->SDIO_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->SDIO_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->SDIO_prf_ptr->device_ptr);
    }
    return;
}
void ETH_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->ETH_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->ETH_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->ETH_prf_ptr->device_ptr);
    }
    return;
}
void CAN0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CAN0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CAN0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CAN0_prf_ptr->device_ptr);
    }
    return;
}
void CAN1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->CAN1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->CAN1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->CAN1_prf_ptr->device_ptr);
    }
    return;
}
void USB_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->USB_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->USB_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->USB_prf_ptr->device_ptr);
    }
    return;
}
void RTC_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->RTC_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->RTC_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->RTC_prf_ptr->device_ptr);
    }
    return;
}
void WDOG1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->WDOG1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->WDOG1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->WDOG1_prf_ptr->device_ptr);
    }
    return;
}
void LETIMER1_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->LETIMER1_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->LETIMER1_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->LETIMER1_prf_ptr->device_ptr);
    }
    return;
}
void TRNG0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->TRNG0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->TRNG0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->TRNG0_prf_ptr->device_ptr);
    }
    return;
}
void QSPI0_IRQHandler(void){
    if(mod_som_sys_peripherals_list_ptr->QSPI0_prf_ptr != DEF_NULL){
        mod_som_sys_peripherals_list_ptr->QSPI0_prf_ptr->irq_handler_1_f(
                (void *)mod_som_sys_peripherals_list_ptr->QSPI0_prf_ptr->device_ptr);
    }
    return;
}

/******************************************************************************
 * @brief WDOG Interrupt Handler. Clears interrupt flag.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *
 *****************************************************************************/
void WDOG0_IRQHandler(void)
{
  //ALB do something?
  /* Clear flag for interrupt */
//  uint8_t toto = 0;
//        WDOGn_IntClear(DEFAULT_WDOG, WDOG_IEN_WARN);
//      WDOG_Feed();
}


