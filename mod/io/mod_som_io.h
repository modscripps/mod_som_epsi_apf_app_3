/*******************************************************************************
 * @file mod_som_io.h
 * @brief
 * @date Mar 5, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 *
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_IO_H_
#define MOD_SOM_IO_H_
//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include "mod_som_common.h"

//------------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------------

/*******************************************************************************
 * @define MOD_SOM_IO_PRINT_TASK_PRIORITY
 *     priority value of MOD SOM I/O background task
 ******************************************************************************/
#ifndef MOD_SOM_IO_PRINT_TASK_PRIORITY
#define MOD_SOM_IO_PRINT_TASK_PRIORITY 16u
#endif

/*******************************************************************************
 * @define MOD_SOM_IO_TASK_STK_SIZE
 *     size of the tcb stack of the task
 ******************************************************************************/
#ifndef MOD_SOM_IO_TASK_STK_SIZE
#define MOD_SOM_IO_TASK_STK_SIZE 128u
#endif

#define OVF_MSG_LIST_THERSHOLD 64
/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_PREFIX
 *     16 bit identifying prefix for MOD SOM I/O status
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_PREFIX 0x02U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY
 *     error status value: fail to allocate dynamic memory pool for MOD SOM
 *     I/O usage
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY 0x01U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY
 *     error status value: fail to allocate memory from dynamic memory pool
 *     for MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY 0x02U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_FREE_MEMORY
 *     error status value: fail to free memory from dynamic memory pool
 *     for a MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_FREE_MEMORY 0x03U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE 0x010U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_FLUSH_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_FLUSH_QUEUE 0x011U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE 0x012U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ACCESS_MSG_FROM_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ACCESS_MSG_FROM_QUEUE 0x013U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_SEMAPHORE 0x020U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_PEND_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_PEND_SEMAPHORE 0x021U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_POST_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_POST_SEMAPHORE 0x022U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_TASK
 *     error status value: fail use OSCreateTask to initiate MOD SOM I/O
 *     background task to pipe data
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_TASK 0x04U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT
 *     error status value: value cannot be set after initialization
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT 0x05U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ENQUE
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_FAIL_TO_ENQUEUE 0x06U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED 0x07U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_NOT_STARTED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_NOT_STARTED 0x08U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_ALREADY_STARTED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_ALREADY_STARTED 0x09U

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_QUEUE_EMPTY
 *     empty queue error
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_QUEUE_EMPTY 0x0AU

/*******************************************************************************
 * @define MOD_SOM_IO_STATUS_ERR_QUEUE_ITEM_NOT_FOUND
 *     error: can't find item in queue
 ******************************************************************************/
#define MOD_SOM_IO_STATUS_ERR_QUEUE_ITEM_NOT_FOUND 0x0BU

//#define MOD_SOM_IO_STATUS_
//#define
/*******************************************************************************
 * @function
 *     mod_som_io_set_max_size_of_complete_data_block_f
 * @abstract
 *     Set maximum size in bytes of the largest data block
 * @discussion
 *     Set maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O, this must be set before initialization
 * @return
 *     status would indicate error if set after initialization
 ******************************************************************************/
mod_som_status_t mod_som_io_set_max_size_of_complete_data_block_f(uint32_t max_size);

/*******************************************************************************
 * @function
 *     mod_som_io_get_max_size_of_complete_data_block_f
 * @abstract
 *     Get maximum size in bytes of the largest data block
 * @discussion
 *     Get maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O
 * @return
 *     size as a unsigned 32 bit number
 ******************************************************************************/
uint32_t mod_som_io_get_max_size_of_complete_data_block_f(void);

/*******************************************************************************
 * @function
 *     mod_som_io_init_f
 * @abstract
 *     Initializes MOD SOM I/O and start MOD SOM I/O Task for data piping
 * @discussion
 *     The initialization function creates a dynamic memory pool of memory block
 *     of the size of the largest data block to transfer/pipe. It also
 *     initialized the queue for transfer. And it create and OS task via
 *     OSTaskCreate to run in parallels with the main task
 *     TODO we need to work out the MUTEX part of things
 * @return
 *     status would indicate error in initialization
 ******************************************************************************/
mod_som_status_t mod_som_io_init_f(void);
mod_som_status_t mod_som_io_init_ldma_f();

/*******************************************************************************
 * @function
 *     mod_som_io_start_f
 * @abstract
 *     Start the background task for MOD SOM I/O
 * @discussion
 *
 * @return
 *     status would indicate error in initialization
 ******************************************************************************/
mod_som_status_t mod_som_io_start_f(void);
/*******************************************************************************
 * @brief
 *   mod_som_io_stop_task_f
 *   stop the io task (i.e. efe stream consumer)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_io_stop_task_f();
/*******************************************************************************
 * @function
 *     mod_som_io_print_f
 * @abstract
 *     similar to printf(), but it has an atomic block-queue functionality
 * @discussion
 *     This function takes the same parameters as printf(). It queue up the
 *     output until the system piping is ready to printout
 * @param format
 *     string of printf() format
 * @param other parameters that are in accordance to the rules of printf()
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_io_print_f(const char *format, ...) _ATTRIBUTE ((__format__ (__printf__, 1, 2)));

/*******************************************************************************
 * @function
 *     mod_som_io_stream_data_f
 * @abstract
 *     queue up data to stream
 * @discussion
 *     this function copy data over, queue it up, and stream it out
 *     once it done, it would indicate in the parameter done_streaming_flag_ptr
 * @param data_ptr
 *     pointer to data to stream
 * @param data_length
 *     length of data to stream
 * @param done_streaming_flag_ptr
 *     pointer to a boolean flag to indicate when streaming is done
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_io_stream_data_f(
        const uint8_t *data_ptr, uint32_t data_length,
        bool * done_streaming_flag_ptr);

/*******************************************************************************
 * @function
 *     mod_som_io_putchar_f
 * @abstract
 *     print a character to the streaming pipe
 * @discussion
 *     this function take only a single character
 * @param c
 *     character to output
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_io_putchar_f(char c);


///*******************************************************************************
// * @function
// *     mod_som_io_stream_data_with_header_separate_f
// * @abstract
// *     queue up data to stream with separate header and timestamp
// * @discussion
// *     this function will assemble the whole data block: header, timestamp,
// *     data, checksum
// * @param data_header
// *     data block header
// * @param data_header_length
// *     length (bytes) of data block header
// * @param data_timestamp
// *     timestamp for data block
// * @param data_ptr
// *     pointer to data to stream
// * @param data_length
// *     length of data to stream
// * @param done_streaming_flag_ptr
// *     pointer to a boolean flag to indicate when streaming is done
// * @return
// *     status would indicate error in execution
// ******************************************************************************/
//mod_som_status_t mod_som_io_stream_data_with_header_separate_f(
//        const char *data_header, uint32_t data_header_length,
//        uint64_t data_timestamp,
//        const uint8_t *data_ptr, uint32_t data_length,
//        volatile bool * done_streaming_flag_ptr);
//
//mod_som_status_t mod_som_io_register_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr);
//mod_som_status_t mod_som_io_deregister_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr);

#endif /* MOD_IO_MOD_SOM_IO_H_ */
