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

#ifndef MOD_SOM_SDIO_PRIV_H_
#define MOD_SOM_SDIO_PRIV_H_
//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include "mod_som_common.h"
#include "mod_som_sdio.h"

//------------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------------
/*******************************************************************************
 * @define MOD_SOM_SDIO_DEFAULT_SIZE_LARGEST_BLOCK
 *     default size of the size of data block to be expect to be sent
 *     so that we can allocate appropriate
 ******************************************************************************/
#ifndef MOD_SOM_SDIO_DEFAULT_SIZE_LARGEST_BLOCK
#define MOD_SOM_SDIO_DEFAULT_SIZE_LARGEST_BLOCK 128U
#endif
#define MOD_SOM_SDIO_MSG_QUEUE_COUNT 80U
#define MOD_SOM_SDIO_DEFAULT_FILE_DURATION 3600 //ALB in second, Default 1 hour
#define MOD_SOM_SDIO_CANNOT_SAVE_SETUP 1
#define MOD_SOM_SDIO_CANNOT_SAVE_PREFIX 2
#define MOD_SOM_SDIO_DEFAULT_SETUP_HEADER "SDIO"
#define MOD_SOM_SDIO_DEFAULT_FILENAME_HEADER "som"
#define MOD_SOM_SDIO_MAX_FILENAME_LENGTH 100
//------------------------------------------------------------------------------
// typedefs
//------------------------------------------------------------------------------

typedef struct{
    uint32_t size;
    const uint8_t * data_ptr;
    uint32_t data_length;
    volatile bool *done_streaming_flag_ptr;
    uint8_t * printf_str_ptr;
    uint32_t printf_str_length;
    bool is_printf_mode;
    //    const char* format_for_printf;
    //    va_list args_for_printf;
    //    void * next_item_ptr;
}mod_som_sdio_xfer_t,*mod_som_sdio_xfer_ptr_t;


//typedef struct{
//    mod_som_cir_buff_ptr_t cir_buff_ptr;
//    uint64_t cnsmr_count;
//    uint32_t cnsmr_idx;
//    uint32_t skip_cnt;
//    uint8_t * output_buffer;
//    uint32_t output_buffer_size;
//    bool output_buffer_done_streaming_flag;
//    bool output_buffer_done_file_writing_flag;
////    uint32_t output_
//    void * next_ptr;
//    void * prev_ptr;
//}mod_som_sdio_cir_buff_list_item_t, *mod_som_sdio_cir_buff_list_item_ptr_t;



//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------
/*******************************************************************************
 * @function
 *     mod_som_sdio_task_f
 * @abstract
 *     MOD SOM I/O OS task that would run in parallel with the main task
 * @discussion
 *     This task would dequeue the MOD SOM I/O transfer as the data is piped out
 * @param       p_arg
 *     argument passed in by OSTaskCreate (not used)
 ******************************************************************************/
static void mod_som_sdio_print_task_f(void *p_arg);

///*******************************************************************************
// * @function
// *     mod_som_sdio_task_f
// * @abstract
// *     MOD SOM I/O OS task that would run in parallel with the main task
// * @discussion
// *     This task would dequeue the MOD SOM I/O transfer as the data is piped out
// * @param       p_arg
// *     argument passed in by OSTaskCreate (not used)
// ******************************************************************************/
//static void mod_som_sdio_cir_buff_task_f(void *p_arg);

/*******************************************************************************
 * @function
 *     mod_som_sdio_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     high 16 bits are system identifier, the low 16 bits are the status code
 *     according each system
 * @param       mod_som_sdio_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_sdio_encode_status_f(uint8_t mod_som_sdio_status);

/*******************************************************************************
 * @function
 *     mod_som_sdio_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the last 16 bits if the
 *     higher bits show the status code is of MOD SOM I/O
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xffff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_sdio_decode_status_f(mod_som_status_t mod_som_status);

/*******************************************************************************
 * @function
 *     mod_som_sdio_new_xfer_item_f
 * @abstract
 *     Create new I/O transfer item
 * @discussion
 *     Using dynamic memory pool for IO transfer item to create new memory
 *     pointer for a new I/O transfer item
 * @return
 *     Pointer to new I/O transfer item, if pointer is null, this indicates
 *     error in allocating memory
 ******************************************************************************/
mod_som_sdio_xfer_ptr_t mod_som_sdio_new_xfer_item_f(void);

/*******************************************************************************
 * @function
 *     mod_som_sdio_free_xfer_item_f
 * @abstract
 *     Free memory of I/O transfer item
 * @discussion
 *     Since memory of I/O transfer item is in a dynamic memory pool, item freed
 *     allows new items to be created
 * @param       xfer_item_ptr
 *     Item to free
 * @return
 *     Status of success or failure of freeing transfer item
 ******************************************************************************/
mod_som_status_t mod_som_sdio_free_xfer_item_f(mod_som_sdio_xfer_ptr_t xfer_item_ptr);

/*******************************************************************************
 * @function
 *     mod_som_sdio_add_to_queue_f
 * @abstract
 *     Add I/O transfer item to queue
 * @discussion
 *     Add I/O transfer item to queue (FIFO), update current tail next item, and
 *     tail to queue to current item
 * @param       xfer_item_ptr
 *     Item to add
 * @return
 *     Status of adding to queue
 ******************************************************************************/
mod_som_status_t mod_som_sdio_add_to_queue_f(mod_som_sdio_xfer_ptr_t xfer_item_ptr);

///******************************************************************************
// * @function
// *     mod_som_sdio_remove_from_queue_f
// * @abstract
// *     Add I/O transfer item to queue
// * @discussion
// *     Add I/O transfer item to queue (FIFO), update current tail next item, and
// *     tail to queue to current item
// * @param       xfer_item_ptr
// *     Item to add
// * @return
// *     Status of adding to queue
// ******************************************************************************/
//mod_som_status_t mod_som_sdio_remove_from_queue_f(mod_som_sdio_xfer_ptr_t xfer_item_ptr);

//mod_som_sdio_cir_buff_list_item_ptr_t mod_som_sdio_new_cir_buff_list_item_f(mod_som_cir_buff_ptr_t cir_buff_ptr);
//
//mod_som_status_t mod_som_sdio_free_cir_buff_list_item_f(mod_som_sdio_cir_buff_list_item_ptr_t cir_buff_list_item_ptr);
//
//uint32_t mod_som_sdio_generate_output_buffer(mod_som_sdio_cir_buff_list_item_ptr_t mod_som_sdio_cir_buff_list_item_ptr);



#endif /* MOD_SOM_SDIO_PRIV_H_ */
