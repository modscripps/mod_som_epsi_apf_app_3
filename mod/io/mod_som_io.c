/*******************************************************************************
 * @file mod_som_io.c
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

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include "mod_som_io.h"
#include "mod_som_io_priv.h"
#include "mod_som.h"

#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
 * @define MOD_SOM_IO_DEFAULT_SIZE_LARGEST_BLOCK
 *     default size of the size of data block to be expect to be sent
 *     so that we can allocate appropriate
 ******************************************************************************/
#ifndef MOD_SOM_IO_DEFAULT_SIZE_LARGEST_BLOCK
#define MOD_SOM_IO_DEFAULT_SIZE_LARGEST_BLOCK 128U
#endif
#define MOD_SOM_IO_MSG_QUEUE_COUNT 64U

#define MOD_SOM_IO_LDMA_CH   3
#define MOD_SOM_IO_LDMA_IRQ  LDMA_IRQn
#define MOD_SOM_IO_LDMA_CLCK cmuClock_LDMA


//------------------------------------------------------------------------------
// local variable declarations
//------------------------------------------------------------------------------
//static volatile mod_som_io_xfer_ptr_t mod_som_io_xfer_head_ptr;
//static volatile mod_som_io_xfer_ptr_t mod_som_io_xfer_tail_ptr;

static struct{
    bool initialized_flag;
    bool started_flag;
    bool listoverflow_flag;
    MEM_DYN_POOL dyn_mem_pool;
    uint32_t max_size_of_complete_data_block;
    CPU_STK print_task_stack[MOD_SOM_IO_TASK_STK_SIZE];
    OS_TCB print_task_tcb;
    CPU_STK cir_buff_task_stack[MOD_SOM_IO_TASK_STK_SIZE];
    OS_TCB cir_buff_task_tcb;
    //bool allow_append = true;
    OS_Q msg_queue;
    //OS_SEM semaphore;
    mod_som_ldma_handle_t ldma;
    bool * done_flag_ptr;

//    mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_head_ptr;
//    mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_tail_ptr;
//    MEM_DYN_POOL cir_buff_dyn_mem_pool_ptr;
} mod_som_io_struct;

//------------------------------------------------------------------------------mod_som_io_struct.initialized_flag
// global variable declarations
//------------------------------------------------------------------------------

//LDMA variable
LDMA_TransferCfg_t mod_som_io_ldma_signal= LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART5_TXEMPTY);
//ALB Place holder for adc_read from USART4 RX to databuffer. It will be change in config adc.
LDMA_Descriptor_t mod_som_io_ldma_write_tx = LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(0,0,0);
LDMA_Descriptor_t mod_som_io_ldma_descriptor_write;//ALB Descriptor list to send data.


//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------
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
mod_som_status_t mod_som_io_set_max_size_of_complete_data_block_f(uint32_t max_size){
    if(mod_som_io_struct.initialized_flag){
        return mod_som_io_encode_status_f(
                MOD_SOM_IO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT);
    }
    mod_som_io_struct.max_size_of_complete_data_block = max_size;
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}

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
uint32_t mod_som_io_get_max_size_of_complete_data_block_f(void){
    return mod_som_io_struct.max_size_of_complete_data_block;
}

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
mod_som_status_t mod_som_io_init_f(void){
	// set initialize flag to false
	// they will be set up to true once initialization successful
    mod_som_io_struct.initialized_flag = false;
    mod_som_io_struct.started_flag = false;
    // TODO SN What is complete data block for?
    if(mod_som_io_struct.max_size_of_complete_data_block == 0)
        mod_som_io_struct.max_size_of_complete_data_block = MOD_SOM_IO_DEFAULT_SIZE_LARGEST_BLOCK;

    // error for OS functions
    RTOS_ERR err;

    // ALB create msg queue to store the task messages
    // The messages are added to the queue by OSPost when mod_som_printf is invoked
    // This list is part of the streaming consumer.
    OSQCreate(&mod_som_io_struct.msg_queue,           /*   Pointer to user-allocated message queue.          */
            "MOD SOM Message Queue",          /*   Name used for debugging.                          */
            MOD_SOM_IO_MSG_QUEUE_COUNT,                     /*   Queue will have 10 messages maximum.              */
            &err);

    //Stall if OSQCreate fails
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE);

    //Flush all message in the queue.
    // ALB From what I understand:Be carefull using OSQFlush it only de-reference the message.
    // ALB It does not clear the memory.
    OSQFlush(&mod_som_io_struct.msg_queue,&err);
    //Stall if OSQFlush fails
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    //return error if OSQ fails
    // ALB This probably useless because the code would stall if there is an error
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_FLUSH_QUEUE);

    // Create Dynamic memory pool. TODO SN comment on the use of dynamic memory pool.
    Mem_DynPoolCreate(
            "MOD SOM IO Dynamic Memory Pool",
            &mod_som_io_struct.dyn_mem_pool,
            DEF_NULL,
            sizeof(mod_som_io_xfer_t)+mod_som_io_struct.max_size_of_complete_data_block,
            sizeof(CPU_ALIGN),//LIB_MEM_BUF_ALIGN_AUTO,
            MOD_SOM_IO_MSG_QUEUE_COUNT,
            2*MOD_SOM_IO_MSG_QUEUE_COUNT,
            &err);
    //Stall if Mem_DynPoolCreate fails
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    //return an error if Mem_DynPoolCreate fails
    // ALB This probably useless because the code would stall if there is an error
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY);


//    //TODO make all IO through LDMA
//    mod_som_io_init_ldma_f();

    // Create overflow flag is to many messages are in the list
    mod_som_io_struct.listoverflow_flag=false;

    // IO module initialize (i.e  streaming consumer initialize)
    mod_som_io_struct.initialized_flag = true;
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}

/***************************************************************************//**
 * @brief
 *   Initialize LDMA
 ******************************************************************************/
mod_som_status_t mod_som_io_init_ldma_f()
{
  // TODO It will always return MOD_SOM_STATUS_OK.
  // TODO I do not know how to check if everything is fine here.

  mod_som_io_struct.ldma.ch=MOD_SOM_IO_LDMA_CH;

  mod_som_io_struct.ldma.irq_f = mod_som_io_ldma_irq_handler_f;
  mod_som_io_struct.ldma.irqn = MOD_SOM_IO_LDMA_IRQ;


  //ALB initialize the LDMA clock
  CMU_Clock_TypeDef io_ldma_clk = MOD_SOM_IO_LDMA_CLCK;
  CMU_ClockEnable(io_ldma_clk, true);
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  //ALB LDMA IRQ default priority is 3.
  LDMA_Init( &init );

  // set LDMA descriptors for Direct Memory access and transfer -> ADC to memory using CS cascade
  // Define LDMA_TransferCfg_t for ADC config LDMA transfer

  // ALB create read descriptor list
  // ALB this the main descriptor list used during the sampling.
  // ALB The LDMA transfer define by this list is called inside the GPIO interrupt handler
  // ALB after the ADC send their interrupt signal
  mod_som_io_define_write_descriptor_f();

  return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_io_start_f
 *     create and start the IO task (i.e streaming consumer)
 * @abstract
 *     Start the background task for MOD SOM I/O
 * @discussion
 *
 * @return
 *     status would indicate error in initialization
 ******************************************************************************/
mod_som_status_t mod_som_io_start_f(void){

    if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    RTOS_ERR err;

    OSTaskCreate(&mod_som_io_struct.print_task_tcb, // Create the Start Print Task
            "MOD SOM I/O Print Task",
            (OS_TASK_PTR)mod_som_io_print_task_f,
            DEF_NULL,
            MOD_SOM_IO_PRINT_TASK_PRIORITY,
            &mod_som_io_struct.print_task_stack[0],
            (MOD_SOM_IO_TASK_STK_SIZE / 10u),
            MOD_SOM_IO_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            &err);
    //Stall if fails
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    //retrun an error if fails. Probably useless since code stall if fails
    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_CREATE_TASK);

    mod_som_io_struct.started_flag = true;
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   mod_som_io_stop_task_f
 *   stop the io task (i.e. efe stream consumer)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_io_stop_task_f(){

  mod_som_status_t status=MOD_SOM_STATUS_OK;
  RTOS_ERR err;

  OSTaskDel(&mod_som_io_struct.print_task_tcb,
            &err);

  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    status=1;

  return status;
}

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
mod_som_status_t mod_som_io_print_f(const char *format, ...){
    if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);

    if(!mod_som_io_struct.started_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);

    mod_som_io_xfer_ptr_t mod_som_io_xfer_item_ptr;
    mod_som_status_t mod_som_status;
//    RTOS_ERR err;
    va_list vl_args;

    mod_som_io_xfer_item_ptr = mod_som_io_new_xfer_item_f();
    if(mod_som_io_xfer_item_ptr == DEF_NULL)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);
    mod_som_io_xfer_item_ptr->is_printf_mode = true;
    va_start(vl_args, format);
    vsprintf((char*)(mod_som_io_xfer_item_ptr->printf_str_ptr),format,vl_args);
    va_end(vl_args);

    mod_som_io_xfer_item_ptr->printf_str_length = strlen((char *)mod_som_io_xfer_item_ptr->printf_str_ptr);
    mod_som_io_xfer_item_ptr->done_streaming_flag_ptr = DEF_NULL;

    mod_som_status = mod_som_io_add_to_queue_f(mod_som_io_xfer_item_ptr);
    if(mod_som_status != MOD_SOM_STATUS_OK){
        mod_som_io_free_xfer_item_f(mod_som_io_xfer_item_ptr);
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ENQUEUE);
    }
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}

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
mod_som_status_t mod_som_io_stream_data_f(const uint8_t *data_ptr, uint32_t data_length,
        bool * done_streaming_flag_ptr){
    if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_io_struct.started_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);
    mod_som_io_xfer_ptr_t mod_som_io_xfer_item_ptr;
    mod_som_status_t mod_som_status;
//    RTOS_ERR err;

    mod_som_io_xfer_item_ptr = mod_som_io_new_xfer_item_f();
    if(mod_som_io_xfer_item_ptr == DEF_NULL)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);

    mod_som_io_xfer_item_ptr->done_streaming_flag_ptr = done_streaming_flag_ptr;

    if(done_streaming_flag_ptr != DEF_NULL)
        *(mod_som_io_xfer_item_ptr->done_streaming_flag_ptr)  = false;

    mod_som_io_xfer_item_ptr->data_ptr = data_ptr;
    mod_som_io_xfer_item_ptr->data_length = data_length;

    mod_som_status = mod_som_io_add_to_queue_f(mod_som_io_xfer_item_ptr);

    if(mod_som_status != MOD_SOM_STATUS_OK){
        mod_som_io_free_xfer_item_f(mod_som_io_xfer_item_ptr);
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ENQUEUE);
    }

    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}

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
mod_som_status_t mod_som_io_putchar_f(char c){
    if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_io_struct.started_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);

    mod_som_status_t mod_som_status;
    mod_som_io_xfer_ptr_t mod_som_io_xfer_item_ptr;

    mod_som_io_xfer_item_ptr = mod_som_io_new_xfer_item_f();
    if(mod_som_io_xfer_item_ptr == DEF_NULL)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);
    *((char *)mod_som_io_xfer_item_ptr->printf_str_ptr) = c;
    mod_som_io_xfer_item_ptr->printf_str_length = 1;
    mod_som_io_xfer_item_ptr->is_printf_mode = true;
    mod_som_status = mod_som_io_add_to_queue_f(mod_som_io_xfer_item_ptr);
    if(mod_som_status != MOD_SOM_STATUS_OK){
        mod_som_io_free_xfer_item_f(mod_som_io_xfer_item_ptr);
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ENQUEUE);
    }
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}


///*******************************************************************************
// * @function
// *     mod_som_io_print_task_f
// * @abstract
// *     MOD SOM I/O OS task that would run in parallel with the main task
// * @discussion
// *     This task would dequeue the MOD SOM I/O transfer as the data is piped out
// * @param       p_arg
// *     argument passed in by OSTaskCreate (not used)
// ******************************************************************************/
//void mod_som_io_cir_buff_task_f(void *p_arg)
//{
//
//    (void)p_arg; // Deliberately unused argument
//    RTOS_ERR err;
//    uint32_t o_buff_size;
//
//    CORE_DECLARE_IRQ_STATE;
//
//    while(DEF_ON){
//
//
//        if(mod_som_io_struct.cir_buff_list_head_ptr != DEF_NULL){
//            mod_som_io_cir_buff_list_item_ptr_t cur_cb_list_item_ptr = mod_som_io_struct.cir_buff_list_head_ptr;
//            uint32_t buffer_cnt;
//            do{
//                buffer_cnt = cur_cb_list_item_ptr->cir_buff_ptr->prdcr_cnt - cur_cb_list_item_ptr->cnsmr_count;
//                if(buffer_cnt>0){
//                    //check of skipped records
//                    if(buffer_cnt >= cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_total){
//                        cur_cb_list_item_ptr->skip_cnt =
//                                (buffer_cnt/cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_total)*cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_total
//                                + cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_padding_for_skip;
//                        cur_cb_list_item_ptr->cnsmr_count =
//                                cur_cb_list_item_ptr->cir_buff_ptr->prdcr_cnt-cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_total+
//                                cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_padding_for_skip;
//                        buffer_cnt = cur_cb_list_item_ptr->cir_buff_ptr->prdcr_cnt - cur_cb_list_item_ptr->cnsmr_count;
//                    }
//                    if(buffer_cnt>=cur_cb_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_per_rec){
//                        //process header
//                        //TODO fill me in here to data streaming or sd card writing
//                        o_buff_size = mod_som_io_generate_output_buffer(cur_cb_list_item_ptr);
////                        if(o_buff_size == cur_cb_list_item_ptr->output_buffer_size){
////
////                        }
//                        //reset skip count
//                        cur_cb_list_item_ptr->skip_cnt = 0;
//                    }
//                }
//                cur_cb_list_item_ptr = cur_cb_list_item_ptr->next_ptr;
//            }while(cur_cb_list_item_ptr != DEF_NULL);
//        }
//        //necessary for every task
//        OSTimeDly(
//                (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
//                (OS_OPT      )OS_OPT_TIME_DLY,
//                &err);
//        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    }
//}

/*******************************************************************************
 * @function
 *     mod_som_io_print_task_f
 *
 *     ALB TODO convert the putchar into a LDMA transfer.
 * @abstract
 *     MOD SOM I/O OS task that would run in parallel with the main task
 * @discussion
 *     This task would dequeue the MOD SOM I/O transfer as the data is piped out
 * @param       p_arg
 *     argument passed in by OSTaskCreate (not used)
 ******************************************************************************/
void mod_som_io_print_task_f(void *p_arg)
{

    (void)p_arg; // Deliberately unused argument
    RTOS_ERR err;

    mod_som_io_xfer_ptr_t tmp_mod_som_io_xfer_item_ptr = DEF_NULL;
    OS_MSG_SIZE tmp_mod_som_io_xfer_item_size;
    CPU_TS time_passed_msg_pend;
    uint32_t i;

    CORE_DECLARE_IRQ_STATE;

    while(DEF_ON)
        {
//        mod_som_io_struct.done_flag=false;
        //necessary for every task
//        OSTimeDly(
//                (OS_TICK     )MOD_SOM_CFG_LOOP_TICK_DELAY,
//                (OS_OPT      )OS_OPT_TIME_DLY,
//                &err);
//        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
        tmp_mod_som_io_xfer_item_ptr =
                (mod_som_io_xfer_ptr_t)OSQPend(&mod_som_io_struct.msg_queue,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        &tmp_mod_som_io_xfer_item_size,
                        &time_passed_msg_pend,
                        &err);
        //        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
        if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
            continue;
        if(tmp_mod_som_io_xfer_item_ptr == NULL)
            continue;

        if(tmp_mod_som_io_xfer_item_ptr->is_printf_mode){
            //body of data
            for(i=0;i<tmp_mod_som_io_xfer_item_ptr->printf_str_length;i++){
                putchar(tmp_mod_som_io_xfer_item_ptr->printf_str_ptr[i]);
            }
        }else{

            //body of data
            for(i=0;i<tmp_mod_som_io_xfer_item_ptr->data_length;i++){
                putchar(tmp_mod_som_io_xfer_item_ptr->data_ptr[i]);
            }
            //ALB change putchar to an LDMA transfer
//            mod_som_io_struct.done_flag_ptr=
//                tmp_mod_som_io_xfer_item_ptr->done_streaming_flag_ptr;
//
//            mod_som_io_ldma_transfer_f(tmp_mod_som_io_xfer_item_ptr);

            CORE_ENTER_CRITICAL();
//                mod_som_io_struct.done_flag=true;
                *(tmp_mod_som_io_xfer_item_ptr->done_streaming_flag_ptr)  = true;
            CORE_EXIT_CRITICAL();
        }
        //free memory
        mod_som_io_free_xfer_item_f(tmp_mod_som_io_xfer_item_ptr);

    }
}

/*******************************************************************************
 * @function
 *     mod_som_io_new_xfer_item_f
 * @abstract
 *     Create new I/O transfer item
 * @discussion
 *     Using dynamic memory pool for IO transfer item to create new memory
 *     pointer for a new I/O transfer item
 * @return
 *     Pointer to new I/O transfer item, if pointer is null, this indicates
 *     error in allocating memory
 ******************************************************************************/
mod_som_io_xfer_ptr_t mod_som_io_new_xfer_item_f(void){
    if(!mod_som_io_struct.initialized_flag)
        return DEF_NULL;
//        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_io_struct.started_flag)
        return DEF_NULL;
//        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);
    RTOS_ERR err;
    mod_som_io_xfer_ptr_t mod_som_io_xfer_item_ptr =
            (mod_som_io_xfer_ptr_t)Mem_DynPoolBlkGet(
                    &mod_som_io_struct.dyn_mem_pool,
                    &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_io_xfer_item_ptr==DEF_NULL)
        return DEF_NULL;

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return DEF_NULL;

    Mem_Set(mod_som_io_xfer_item_ptr,0,sizeof(mod_som_io_xfer_t));
    //    mod_som_io_xfer_item_ptr->next_item_ptr = DEF_NULL;
    mod_som_io_xfer_item_ptr->printf_str_ptr = ((uint8_t *)mod_som_io_xfer_item_ptr) + sizeof(mod_som_io_xfer_t);
    mod_som_io_xfer_item_ptr->printf_str_length = 0;
    mod_som_io_xfer_item_ptr->data_length = 0;
    mod_som_io_xfer_item_ptr->is_printf_mode = false;
    mod_som_io_xfer_item_ptr->size = sizeof(mod_som_io_xfer_t);
    //this is so we can find the wrapper class in case we need to clear memory
    return mod_som_io_xfer_item_ptr;
}

/*******************************************************************************
 * @function
 *     mod_som_io_free_xfer_item_f
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
mod_som_status_t mod_som_io_free_xfer_item_f(mod_som_io_xfer_ptr_t xfer_item_ptr){
    RTOS_ERR err;
    if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_io_struct.started_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);
    Mem_DynPoolBlkFree(&mod_som_io_struct.dyn_mem_pool,
            (void *)xfer_item_ptr,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_FREE_MEMORY);
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @function
 *     mod_som_io_add_to_queue_f
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
mod_som_status_t mod_som_io_add_to_queue_f(mod_som_io_xfer_ptr_t xfer_item_ptr){

  if(!mod_som_io_struct.initialized_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_io_struct.started_flag)
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_STARTED);

    RTOS_ERR err;
    if(mod_som_io_struct.msg_queue.MsgQ.NbrEntries>OVF_MSG_LIST_THERSHOLD){
    	mod_som_io_struct.listoverflow_flag=true;
    	//ALB TODO say How I want to handle the overflow?
    	//ALB TODO check for ovf.
    	//ALB TODO handle ovf.
    	//ALB TODO flag the ovf.
    	//ALB TODO So the user knows how much time or sample we missed
    }else{
    	mod_som_io_struct.listoverflow_flag=false;
    }

    if (!mod_som_io_struct.listoverflow_flag){
    OSQPost (&mod_som_io_struct.msg_queue,
            xfer_item_ptr,
            xfer_item_ptr->size,
            OS_OPT_POST_FIFO,
            &err);

    if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)){
        mod_som_io_free_xfer_item_f(xfer_item_ptr);
        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE);
      }
    }else{
        mod_som_io_free_xfer_item_f(xfer_item_ptr);
    	printf("\n Not adding to stream queue Nb entries %u \n",mod_som_io_struct.msg_queue.MsgQ.NbrEntries);
    }
    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
}


/***************************************************************************//**
 * @brief
 *   mod_som_io_ldma_transfer
 *
 **************************************************************************  */

void mod_som_io_ldma_transfer_f(mod_som_io_xfer_ptr_t tmp_mod_som_io_xfer_item_ptr)
{

  mod_som_io_ldma_descriptor_write.xfer.srcAddr=(uint32_t) \
      tmp_mod_som_io_xfer_item_ptr->data_ptr;

  mod_som_io_ldma_descriptor_write.xfer.xferCnt= \
              tmp_mod_som_io_xfer_item_ptr->data_length-1;

  LDMA_StartTransfer( mod_som_io_struct.ldma.ch ,\
                      (void*)&mod_som_io_ldma_signal, \
                      (void*)&mod_som_io_ldma_descriptor_write);

}
/***************************************************************************//**
 * @brief
 *   descriptor list to organize the LDMA transfer of 1 SBE sampled
 *
 *   State machine: Fix length algorithm.
 *
 *   I want to read 1 block of SBE data.
 *
 *   I
 *
 *
 *   state 1: read X char (e.g. 24 char) but last char is not \n
 *            find where \n is in the block. Save the char after \n in the element buffer
 *   state 0: read the last
 *
 *   state 2:
 *
 *
 ******************************************************************************/

void mod_som_io_define_write_descriptor_f()
{

  mod_som_io_ldma_descriptor_write = mod_som_io_ldma_write_tx;
  mod_som_io_ldma_descriptor_write.xfer.srcInc=ldmaCtrlSrcIncOne;

#if defined(MOD_SOM_BOARD)
  mod_som_io_ldma_descriptor_write.xfer.dstAddr=(uint32_t) (&MOD_SOM_MAIN_COM_USART->TXDATA);
#endif
#if defined(MOD_SOM_MEZZANINE_BOARD)
  mod_som_io_ldma_descriptor_write.xfer.dstAddr=(uint32_t) (&MOD_SOM_MEZZANINE_COM_USART->TXDATA);
#endif
}


/*******************************************************************************
 * @brief
 *
 * This "local" IRQ handler is called in mod_som.c
 * TODO manage IRQhandler better.
 *
 ******************************************************************************/
void mod_som_io_ldma_irq_handler_f()
{

//update done flag
  *(mod_som_io_struct.done_flag_ptr)  = true;
//  mod_som_io_struct.done_flag=true;
}


/*******************************************************************************
 * @function
 *     mod_som_io_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the last 8 bits if the
 *     higher bits show the status code is of MOD SOM I/O
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_io_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_IO_STATUS_PREFIX){
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
 *     high 16 bits are system identifier, the low 16 bits are the status code
 *     according each system
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_io_encode_status_f(uint16_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_IO_STATUS_PREFIX, mod_som_io_status);
}


//mod_som_io_cir_buff_list_item_ptr_t mod_som_io_new_cir_buff_list_item_f(mod_som_cir_buff_ptr_t cir_buff_ptr){
//    RTOS_ERR err;
//    if(!mod_som_io_struct.initialized_flag)
//            return DEF_NULL;
//    mod_som_io_cir_buff_list_item_ptr_t mod_som_io_cir_buff_list_item_ptr =
//            (mod_som_io_cir_buff_list_item_ptr_t)Mem_DynPoolBlkGet(
//                    &mod_som_io_struct.dyn_mem_pool,
//                    &err);
//    // Check error code
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if(mod_som_io_cir_buff_list_item_ptr==DEF_NULL)
//        return DEF_NULL;
//
//    Mem_Set(mod_som_io_cir_buff_list_item_ptr,0,sizeof(mod_som_io_cir_buff_list_item_t));
//    mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr = cir_buff_ptr;
//    mod_som_io_cir_buff_list_item_ptr->cnsmr_count = 0;
//    mod_som_io_cir_buff_list_item_ptr->cnsmr_idx = 0;
//    mod_som_io_cir_buff_list_item_ptr->skip_cnt = 0;
//
//    //TODO calculate size for output buffer
//    uint32_t size_of_bytes_in_output_for_header = mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_header_output_hex_mode_flag?2:1;
//    uint32_t size_of_bytes_in_output_for_item = mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_output_hex_mode_flag?2:1;
//    mod_som_io_cir_buff_list_item_ptr->output_buffer_size =
//              1 //$
//            + mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.id_length //ID
//            + mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_header_size*size_of_bytes_in_output_for_header //header
//            + 4*size_of_bytes_in_output_for_header // for the output overflow status
//            + mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_size*mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_per_rec*size_of_bytes_in_output_for_item
//            + 3 //checksum
//            + 2 //\r\n
//            + 1; //end of string
//
//
//    //Allocate memory for output buffer
//    mod_som_io_cir_buff_list_item_ptr->output_buffer =
//            (uint8_t *)Mem_SegAlloc(
//                    "Out put buffer",DEF_NULL,
//                    mod_som_io_cir_buff_list_item_ptr->output_buffer_size,
//                    &err);
//    // Check error code
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
//        return DEF_NULL;
////        return (mod_som_sbe49_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY));
//    if(mod_som_io_cir_buff_list_item_ptr->output_buffer==DEF_NULL){
//        mod_som_io_cir_buff_list_item_ptr->output_buffer = DEF_NULL;
//        return DEF_NULL;
////        return (mod_som_sbe49_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY));
//    }
//
//    mod_som_io_cir_buff_list_item_ptr->output_buffer_done_streaming_flag = true;
//    mod_som_io_cir_buff_list_item_ptr->output_buffer_done_file_writing_flag = true;
//    mod_som_io_cir_buff_list_item_ptr->next_ptr = DEF_NULL;
//    mod_som_io_cir_buff_list_item_ptr->prev_ptr = DEF_NULL;
//    //this is so we can find the wrapper class in case we need to clear memory
//    return mod_som_io_cir_buff_list_item_ptr;
//}
//
//mod_som_status_t mod_som_io_free_cir_buff_list_item_f(mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_item_ptr){
//    RTOS_ERR err;
//    if(!mod_som_io_struct.initialized_flag)
//            return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_NOT_INITIALIZED);
//    Mem_DynPoolBlkFree(&mod_som_io_struct.cir_buff_dyn_mem_pool_ptr,
//            (void *)cir_buff_list_item_ptr,
//            &err);
//
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
//        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_FREE_MEMORY);
//
//    return mod_som_io_encode_status_f(MOD_SOM_STATUS_OK);
//}
//
//mod_som_status_t mod_som_io_register_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr){
//
//    mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_item_ptr;
//    bool need_to_add_flag = false;
//
//    //check if circular buffer has been in the list already, if not add it it.
//    if(mod_som_io_struct.cir_buff_list_head_ptr == DEF_NULL){
//        need_to_add_flag = true;
//    }else{
//        cir_buff_list_item_ptr = mod_som_io_struct.cir_buff_list_head_ptr;
//        need_to_add_flag = true;
//        do {
//            if(cir_buff_list_item_ptr->cir_buff_ptr == cir_buff_ptr){
//                need_to_add_flag = false;
//                break;
//            }
//            cir_buff_list_item_ptr = cir_buff_list_item_ptr->next_ptr;
//        }while(cir_buff_list_item_ptr != DEF_NULL);
//
//    }
//    if(need_to_add_flag){
//        cir_buff_list_item_ptr = mod_som_io_new_cir_buff_list_item_f(cir_buff_ptr);
//        if(mod_som_io_struct.cir_buff_list_head_ptr == DEF_NULL){
//            mod_som_io_struct.cir_buff_list_head_ptr=cir_buff_list_item_ptr;
//            mod_som_io_struct.cir_buff_list_tail_ptr=cir_buff_list_item_ptr;
//        }else{
//            cir_buff_list_item_ptr->prev_ptr = mod_som_io_struct.cir_buff_list_tail_ptr;
//            mod_som_io_struct.cir_buff_list_tail_ptr->next_ptr = cir_buff_list_item_ptr;
//            mod_som_io_struct.cir_buff_list_tail_ptr=cir_buff_list_item_ptr;
//        }
//    }
//    return MOD_SOM_STATUS_OK;
//}
//
///*******************************************************************************
// * @brief
// *   remove or deregister a circular buffer
// * @param peripheral_ptr a mod_som peripheral pointer
// ******************************************************************************/
//mod_som_status_t mod_som_io_deregister_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr){
//    mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_item_ptr;
//    mod_som_status_t mod_som_status;
//    bool found_flag = false;
//    if(mod_som_io_struct.cir_buff_list_head_ptr == DEF_NULL){
//        return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_QUEUE_EMPTY);
//    }else{
//        cir_buff_list_item_ptr = mod_som_io_struct.cir_buff_list_head_ptr;
//        do {
//            if(cir_buff_list_item_ptr->cir_buff_ptr == cir_buff_ptr){ //remove item from list
//                ((mod_som_io_cir_buff_list_item_ptr_t)cir_buff_list_item_ptr->prev_ptr)->next_ptr = cir_buff_list_item_ptr->next_ptr;
//                ((mod_som_io_cir_buff_list_item_ptr_t)cir_buff_list_item_ptr->next_ptr)->prev_ptr = cir_buff_list_item_ptr->prev_ptr;
//                //if item is header, find new header for queue
//                if(cir_buff_list_item_ptr == mod_som_io_struct.cir_buff_list_head_ptr){
//                    mod_som_io_struct.cir_buff_list_head_ptr = cir_buff_list_item_ptr->next_ptr;
//                }
//                //if item is tail, find new tail for queue
//                if(cir_buff_list_item_ptr == mod_som_io_struct.cir_buff_list_tail_ptr){
//                    mod_som_io_struct.cir_buff_list_tail_ptr = cir_buff_list_item_ptr->prev_ptr;
//                }
//                //free memory
//                mod_som_status = mod_som_io_free_cir_buff_list_item_f(cir_buff_list_item_ptr);
//                if(mod_som_status != MOD_SOM_STATUS_OK)
//                    return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_FAIL_TO_FREE_MEMORY);
//                found_flag = true;
//                break;
//            }
//            //end of list
//            if(cir_buff_list_item_ptr->next_ptr == DEF_NULL)
//                break;
//            if(!found_flag)
//                return mod_som_io_encode_status_f(MOD_SOM_IO_STATUS_ERR_QUEUE_ITEM_NOT_FOUND);
//            cir_buff_list_item_ptr = cir_buff_list_item_ptr->next_ptr;
//        }while(cir_buff_list_item_ptr != DEF_NULL);
//    }
//    return MOD_SOM_STATUS_OK;
//}
//
//uint32_t mod_som_io_generate_output_buffer(mod_som_io_cir_buff_list_item_ptr_t mod_som_io_cir_buff_list_item_ptr){
//    if((mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->prdcr_cnt - mod_som_io_cir_buff_list_item_ptr->cnsmr_count)<
//        mod_som_io_cir_buff_list_item_ptr->cir_buff_ptr->cfg.itm_cnt_per_rec){
//        mod_som_io_cir_buff_list_item_ptr->output_buffer[0] = '\0';
//        return 0;
//    }
//    mod_som_io_cir_buff_list_item_ptr_t cir_buff_list_item_ptr = mod_som_io_cir_buff_list_item_ptr;
//    mod_som_cir_buff_ptr_t cir_buff_ptr = cir_buff_list_item_ptr->cir_buff_ptr;
//    volatile uint8_t * header_ptr = cir_buff_ptr->itm_header_array_ptr + cir_buff_list_item_ptr->cnsmr_idx*cir_buff_ptr->cfg.itm_header_size;
//    volatile uint8_t * data_ptr = cir_buff_ptr->itm_data_array_ptr + cir_buff_list_item_ptr->cnsmr_idx*cir_buff_ptr->cfg.itm_size;
//    uint32_t ob_size; // size of output string
//    uint8_t * out_buff = cir_buff_list_item_ptr->output_buffer;
//    *(out_buff) = '$';
//    out_buff++;
//    Mem_Copy(out_buff,cir_buff_ptr->cfg.id_str_ptr,cir_buff_ptr->cfg.id_length);
//    out_buff += cir_buff_ptr->cfg.id_length;
//    //header output
//    if(cir_buff_ptr->cfg.itm_header_output_hex_mode_flag){
//        for(int i = 0; i<cir_buff_ptr->cfg.itm_header_size;i++){
//            *((uint16_t *) out_buff) = mod_som_int8_2hex_f(header_ptr[i]);
////            sprintf(out_buff,"%02x",header_ptr[i]);
//            out_buff += 2;
//        }
//        //skip count output
//        *((uint64_t *) out_buff) = mod_som_int32_2hex_f(cir_buff_list_item_ptr->skip_cnt);
////        sprintf(out_buff,"%08x",cir_buff_list_item_ptr->skip_cnt);
//        out_buff += 8;
//    }else{
//        Mem_Copy(out_buff,header_ptr,cir_buff_ptr->cfg.itm_header_size);
//        out_buff += cir_buff_ptr->cfg.itm_header_size;
//        //skip count output
//        Mem_Copy(out_buff,&cir_buff_list_item_ptr->skip_cnt,4);
//        out_buff += 4;
//    }
//    //data output
//    if(cir_buff_ptr->cfg.itm_output_hex_mode_flag){
//        uint32_t total_item_size = cir_buff_ptr->cfg.itm_size*cir_buff_ptr->cfg.itm_cnt_per_rec;
//        for(int i = 0; i<total_item_size;i++){
//            *((uint16_t *) out_buff) = mod_som_int8_2hex_f(data_ptr[i]);
////            sprintf(out_buff,"%02x",data_ptr[i]);
//            out_buff += 2;
//        }
//    }else{
//        Mem_Copy(out_buff,data_ptr,cir_buff_ptr->cfg.itm_size*cir_buff_ptr->cfg.itm_cnt_per_rec);
//        out_buff += cir_buff_ptr->cfg.itm_size*cir_buff_ptr->cfg.itm_cnt_per_rec;
//    }
//    ob_size = out_buff - cir_buff_list_item_ptr->output_buffer;
//
//    // calculate checksum
//    uint8_t chk_sum = 0;
//    for(int i = 0; i<ob_size; i++){
//        chk_sum ^= cir_buff_list_item_ptr->output_buffer[i];
//    }
//
//    //store checksum
//    *(out_buff++) = '*';
//    *((uint16_t *) out_buff) = mod_som_int8_2hex_f(chk_sum);
//    ob_size += 3;
//
//    return ob_size;
//}
