/*******************************************************************************
 * @file mod_som.h
 * @brief MOD SOM board API
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This defines the API for the MOD SOM board
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/


#ifndef MOD_SOM_H_
#define MOD_SOM_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_cfg.h>
//#include "mod_som_bsp.h"
#include "mod_som_common.h"



//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#ifndef MOD_SOM_MAIN_TASK_PRIORITY
#define  MOD_SOM_MAIN_TASK_PRIORITY         21u
#endif
#ifndef MOD_SOM_MAIN_TASK_STK_SIZE
#define  MOD_SOM_MAIN_TASK_STK_SIZE         512u
#endif
/*******************************************************************************
 * @define MOD_SOM_STATUS_PREFIX
 *     16 bit identifying prefix for MOD SOM I/O status
 ******************************************************************************/
#define MOD_SOM_STATUS_PREFIX 0x01U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY
 *     error status value: fail to allocate dynamic memory pool for MOD SOM
 *     I/O usage
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY 0x01U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY
 *     error status value: fail to allocate memory from dynamic memory pool
 *     for MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY 0x02U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_FREE_MEMORY
 *     error status value: fail to free memory from dynamic memory pool
 *     for a MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_FREE_MEMORY 0x03U

///*******************************************************************************
// * @define MOD_SOM_STATUS_ERR_FAIL_TO_CREATE_TASK
// *     error status value: fail use OSCreateTask to initiate MOD SOM I/O
// *     background task to pipe data
// ******************************************************************************/
//#define MOD_SOM_STATUS_ERR_FAIL_TO_CREATE_TASK 0x04U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT
 *     error status value: value cannot be set after initialization
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT 0x05U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_ENQUE
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_ENQUEUE 0x06U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_INIT_MAIN
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_INIT_MAIN 0x07U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_INIT_MAIN
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_NOT_INITIALIZED_MAIN 0x07U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_START_MAIN_TASK
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_START_MAIN_TASK 0x08U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_INIT_RTOS_MODULE 0x09U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_FIND_BASE_PRF
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_FIND_BASE_PRF 0x010U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_FAIL_TO_FIND_ITEM_IN_QUEUE
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_FAIL_TO_FIND_ITEM_IN_QUEUE 0x11U

/*******************************************************************************
 * @define MOD_SOM_STATUS_ERR_QUEUE_EMPTY
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_STATUS_ERR_QUEUE_EMPTY 0x0AU

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *   initialize the som board in main()
 ******************************************************************************/
mod_som_status_t mod_som_main_init_f(void);

/*******************************************************************************
 * @brief
 *   turn off main com
 ******************************************************************************/
mod_som_status_t mod_som_main_com_off_f(void);
/*******************************************************************************
 * @brief
 *   turn on main com
 ******************************************************************************/
mod_som_status_t mod_som_main_com_on_f(void);
/*******************************************************************************
 * @brief
 *   prep the SOM to sleep
 *   HFXO off
 *   CRYOstuff on
 ******************************************************************************/
mod_som_status_t mod_som_prep_sleep_f(void);
/*******************************************************************************
 * @brief
 *   start the som board in main()
 ******************************************************************************/
mod_som_status_t mod_som_main_start_f(void);
/*******************************************************************************
 * @brief
 *   initialize additional steps of the som board in the main_task_f()
 ******************************************************************************/
mod_som_status_t mod_som_main_task_init_f(void);
mod_som_status_t mod_som_add_peripheral_f(mod_som_prf_ptr_t peripheral_ptr);
mod_som_status_t mod_som_rmv_peripheral_f(mod_som_prf_ptr_t peripheral_ptr);
uint32_t mod_som_calculate_data_block_size_f(uint32_t header_length, uint32_t data_length);
uint32_t mod_som_generate_data_block_f(
        uint8_t * data_block_ptr,
        uint8_t *data_header_ptr, uint32_t data_header_length,
        uint8_t *data_ptr, uint32_t data_length,
        uint64_t data_timestamp);


/*******************************************************************************
 * @brief
 *   This is the task that will be called by the Startup when all services
 *   are initializes successfully.
 *
 * @param p_arg
 *   Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
void mod_som_modules_init_f(void);
mod_som_status_t mod_som_main_sleep_f();
mod_som_status_t mod_som_main_wake_up_f();

void mod_som_main_task_f(void *p_arg);// __attribute__ ((weak));
#endif /* MOD_MOD_SOM_H_ */
