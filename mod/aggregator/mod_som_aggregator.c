/*
 * mod_som_aggregator.c
 *
 *  Created on: Apr 20, 2021
 *      Author: aleboyer
 */
#include <aggregator/mod_som_aggregator.h>

#if defined(MOD_SOM_AGGREGATOR_EN)

#include "mod_som_io.h"
#include "mod_som_priv.h"

#ifdef  MOD_SOM_SDIO_EN
#include "mod_som_sdio.h"
#endif



#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include <aggregator/mod_som_aggregator_cmd.h>
#endif

#if defined(MOD_SOM_EFE_EN)
#include "mod_som_efe.h"
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
#include "mod_som_actuator.h"
#endif
#if defined(MOD_SOM_SBE49_EN)
#include "mod_som_sbe49.h"
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
#include "mod_som_altimeter.h"
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
#include "mod_som_vec_nav.h"
#endif



//PRIVATE DEFINE
#define MOD_SOM_AGGREGATOR_DEFAULT_SIZE_LARGEST_BLOCK 256U
#define MOD_SOM_AGGREGATOR_MSG_QUEUE_COUNT 64U

// Data consumer
static CPU_STK aggregator_consumer_task_stk[
                                     MOD_SOM_AGGREGATOR_CONSUMER_TASK_STK_SIZE];
static OS_TCB  aggregator_consumer_task_tcb;
uint32_t full_aggregator_packet_length;


sl_status_t mystatus;
mod_som_aggregator_ptr_t mod_som_aggregator_ptr;


//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------
/*******************************************************************************
 * @brief
 *
 *
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_construct_config_f(){
  RTOS_ERR  err;

  //Configuration
  mod_som_aggregator_ptr->config_ptr =
      (mod_som_aggregator_config_ptr_t)Mem_SegAlloc(
          "MOD SOM AGGREGATOR config.",DEF_NULL,
          sizeof(mod_som_aggregator_config_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_aggregator_ptr->status = mod_som_aggregator_encode_status_f(
                            MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_aggregator_ptr->config_ptr==DEF_NULL)
    {
      mod_som_aggregator_ptr = DEF_NULL;
      return (mod_som_aggregator_ptr->status =
                            mod_som_aggregator_encode_status_f(
                            MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  mod_som_aggregator_config_ptr_t config_ptr =
                                             mod_som_aggregator_ptr->config_ptr;

#if defined(MOD_SOM_EFE_EN)
  mod_som_efe_config_ptr_t local_efe_config= mod_som_efe_get_config_f();
  config_ptr->efe_record_length=local_efe_config->record_length;
#endif
#if defined(MOD_SOM_SBE49_EN)
  mod_som_sbe49_config_ptr_t local_sbe_config= mod_som_sbe49_get_config_f();
  config_ptr->sbe_record_length=local_sbe_config->record_length;
#endif
#if defined(MOD_SOM_SBE41_EN)
  mod_som_sbe41_config_ptr_t local_sbe41_config= mod_som_sbe41_get_config_f();
  config_ptr->sbe_elements_length=local_sbe41_config->record_length;
#endif
#if defined(MOD_SOM_ACTUATOR_EN)
  mod_som_actuator_config_ptr_t local_actu_config=
                                                mod_som_actuator_get_config_f();
  config_ptr->actu_record_length=local_actu_config->record_length;
#endif
#if defined(MOD_SOM_ALTIMETER_EN)
  mod_som_altimeter_config_ptr_t local_alti_config=
                                               mod_som_altimeter_get_config_f();
  config_ptr->alti_record_length=local_alti_config->record_length;
#endif
#if defined(MOD_SOM_VEC_NAV_EN)
  mod_som_vecnav_config_ptr_t local_vecnav_config=
                                                  mod_som_vecnav_get_config_f();
  config_ptr->vecnav_record_length=local_vecnav_config->record_length;
#endif


  return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Initialize settings pointer with default data
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_construct_settings_f(){
  RTOS_ERR  err;

  //settings
  mod_som_aggregator_ptr->settings_ptr =
      (mod_som_aggregator_settings_ptr_t)Mem_SegAlloc(
          "MOD SOM AGGREGATOR SETUP.",DEF_NULL,
          sizeof(mod_som_aggregator_settings_t),
          &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_aggregator_ptr->status =
                                             mod_som_aggregator_encode_status_f(
                             MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_aggregator_ptr->settings_ptr==DEF_NULL)
    {
      mod_som_aggregator_ptr = DEF_NULL;
      return (mod_som_aggregator_ptr->status =
                                             mod_som_aggregator_encode_status_f(
                             MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
    }


  mod_som_aggregator_settings_ptr_t settings_ptr =
                                           mod_som_aggregator_ptr->settings_ptr;

  strncpy(settings_ptr->data_header_text,
          MOD_SOM_AGGREGATOR_EPSIA_DATA_HEADER_TEXT,8);

  settings_ptr->packet_format             =
                                       MOD_SOM_AGGREGATOR_PACKET_FORMAT_DEFAULT;
  settings_ptr->efe_record_per_packet     =
                               MOD_SOM_AGGREGATOR_EFE_DEFAULT_RECORD_PER_PACKET;
  settings_ptr->sbe_record_per_packet     =
                               MOD_SOM_AGGREGATOR_SBE_DEFAULT_RECORD_PER_PACKET;
  settings_ptr->vecnav_record_per_packet  =
                            MOD_SOM_AGGREGATOR_VECNAV_DEFAULT_RECORD_PER_PACKET;
  settings_ptr->alti_record_per_packet    =
                              MOD_SOM_AGGREGATOR_ALTI_DEFAULT_RECORD_PER_PACKET;

  settings_ptr->initialize_flag      = true;
  settings_ptr->size=sizeof(*settings_ptr);

  return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get a the settings pointer to be used in other modules
 ******************************************************************************/

mod_som_aggregator_settings_t mod_som_aggregator_get_settings_f(){
  return *mod_som_aggregator_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_init_f(){
  RTOS_ERR  err;
  mod_som_status_t status;

    //Allocate memory for the main module pointer
    mod_som_aggregator_ptr = (mod_som_aggregator_ptr_t)Mem_SegAlloc(
        "MOD SOM AGGREG RUNTIME Memory",DEF_NULL,
        sizeof(mod_som_aggregator_t),
        &err);
    // Check error code
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_aggregator_ptr==DEF_NULL)
      return (mod_som_aggregator_encode_status_f(
                            MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));


#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    status = mod_som_aggregator_init_cmd_f();
    if(status != MOD_SOM_STATUS_OK)
        return mod_som_aggregator_encode_status_f(
                                       MOD_SOM_AGGREGATOR_STATUS_FAIL_INIT_CMD);
#endif

    mod_som_aggregator_ptr->initialized_flag = false;

    //ALB set up configuration
    mod_som_aggregator_construct_settings_f();
    mod_som_aggregator_construct_config_f();
    mod_som_aggregator_construct_consumer_ptr_f();


    mod_som_aggregator_ptr->mode = 5;
    mod_som_aggregator_ptr->initialized_flag = true;
    return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_say_hello_world_f(){
    mod_som_io_print_f("[foo bar]: hello world\r\n");
    return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct consumer_buff
 *
 *  uint64_t  timestamp;
 *  uint32_t  packet_length;           //ALB length of the streaming data buffer
 *  uint32_t  max_sample_per_record;   //ALB maximum element per stream block
 *  uint8_t * packet_data_ptr;         //ALB pointer to stream data
 *  uint8_t   data_ready_flg;          //ALB ???
 *  uint8_t   chksum;
 *  uint32_t  record_skipped;
 *            //TODO move this to consumer_ptr
 *  uint8_t   header[MOD_SOM_AGGREGATOR_MAX_HEADER_SIZE];
 *  uint8_t   length_header;//TODO move this to consumer_ptr
 *
 *
 * @param mod_som_aggregator_ptr
 *   runtime device pointer where data can be stored and communication is done
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_construct_consumer_ptr_f(){

  RTOS_ERR  err;

  // allocate memory for streaming_data_ptr
  mod_som_aggregator_ptr->consumer_ptr =
          (mod_som_aggregator_data_consumer_ptr_t)Mem_SegAlloc(
          "MOD SOM AGGREGATOR consumer ptr",DEF_NULL,
          sizeof(mod_som_aggregator_data_consumer_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
      return (mod_som_aggregator_ptr->status =
                                            mod_som_aggregator_encode_status_f(
                            MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_aggregator_ptr->consumer_ptr==DEF_NULL)
  {
      mod_som_aggregator_ptr = DEF_NULL;
      return (mod_som_aggregator_ptr->status =
          mod_som_aggregator_encode_status_f(
              MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }


  mod_som_aggregator_ptr->consumer_ptr->nb_efe_record    = 0;
  mod_som_aggregator_ptr->consumer_ptr->nb_sbe_record    = 0;
  mod_som_aggregator_ptr->consumer_ptr->nb_alti_record   = 0;
  mod_som_aggregator_ptr->consumer_ptr->nb_vecnav_record = 0;


  //ALB define header length.
  //ALB 1 char for sync + 4char + 16 char for timestamp + 8 char for payload +
  //ALB 1 char for chksum sync + 2 char for checksumm= 32 char
  //ALB 29 without the *checksum
  mod_som_aggregator_ptr->consumer_ptr->length_header=
      MOD_SOM_AGGREGATOR_HEXSYNC_LENGTH+\
      MOD_SOM_AGGREGATOR_TAG_LENGTH+\
      MOD_SOM_AGGREGATOR_HEXTIMESTAMP_LENGTH+\
      MOD_SOM_AGGREGATOR_HEXPAYLOAD_LENGTH;

  //ALB length of consumer packet buffer without checksum
  //ALB mod_som_aggregator_ptr->consumer_ptr->length_header+
  mod_som_aggregator_ptr->consumer_ptr->payload_length =
      (mod_som_aggregator_ptr->settings_ptr->alti_record_per_packet * \
       mod_som_aggregator_ptr->config_ptr->alti_record_length)  + \
      (mod_som_aggregator_ptr->settings_ptr->sbe_record_per_packet * \
       mod_som_aggregator_ptr->config_ptr->sbe_record_length)  + \
      (mod_som_aggregator_ptr->settings_ptr->efe_record_per_packet * \
       mod_som_aggregator_ptr->config_ptr->efe_record_length)  + \
      (mod_som_aggregator_ptr->settings_ptr->vecnav_record_per_packet * \
       mod_som_aggregator_ptr->config_ptr->vecnav_record_length);

  //ALB payload length + payload checksum.
  mod_som_aggregator_ptr->consumer_ptr->packet_length =
                          mod_som_aggregator_ptr->consumer_ptr->length_header  +
                          MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH                +
                          mod_som_aggregator_ptr->consumer_ptr->payload_length +
                          MOD_SOM_AGGREGATOR_PAYLOAD_CHKSUM_LGTH+10; //+10 fpr safety



  // allocate memory for the record data ptr consumer_data_ptr
  mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr =
      (uint8_t *)Mem_SegAlloc(
          "MOD SOM AGGREGATOR consumer packet ptr",
          DEF_NULL,
          mod_som_aggregator_ptr->consumer_ptr->packet_length,
          &err);

  //initialize packet buffer.
  for(int i;i<mod_som_aggregator_ptr->consumer_ptr->packet_length;i++){
      mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr=0;
  }

  //ALB define the base ptr for the altimeter sub_packet
  mod_som_aggregator_ptr->consumer_ptr->base_alti_packet_data_ptr=
      &mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr[
                           mod_som_aggregator_ptr->consumer_ptr->length_header+
                           MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH];

  //ALB define the base ptr for the sea bird sub_packet
  mod_som_aggregator_ptr->consumer_ptr->base_sbe_packet_data_ptr=
      &mod_som_aggregator_ptr->consumer_ptr->base_alti_packet_data_ptr[
             (mod_som_aggregator_ptr->settings_ptr->alti_record_per_packet * \
                      mod_som_aggregator_ptr->config_ptr->alti_record_length)];

  //ALB define the base ptr for the efe sub_packet
  mod_som_aggregator_ptr->consumer_ptr->base_efe_packet_data_ptr=
      &mod_som_aggregator_ptr->consumer_ptr->base_sbe_packet_data_ptr[
              (mod_som_aggregator_ptr->settings_ptr->sbe_record_per_packet * \
                       mod_som_aggregator_ptr->config_ptr->sbe_record_length)];

  //ALB define the base ptr for the vector nav sub_packet
  mod_som_aggregator_ptr->consumer_ptr->base_vecnav_packet_data_ptr=
      &mod_som_aggregator_ptr->consumer_ptr->base_efe_packet_data_ptr[
           (mod_som_aggregator_ptr->settings_ptr->efe_record_per_packet * \
                    mod_som_aggregator_ptr->config_ptr->efe_record_length)];


  mod_som_aggregator_construct_msq_queue_f();


  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
      return (mod_som_aggregator_ptr->status =
          mod_som_aggregator_encode_status_f(
              MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_aggregator_ptr->consumer_ptr==DEF_NULL)
  {
      mod_som_aggregator_ptr = DEF_NULL;
      return (mod_som_aggregator_ptr->status =
          mod_som_aggregator_encode_status_f(
              MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

  // initialize data_ready_flag
  mod_som_aggregator_ptr->consumer_ptr->packet_ready_flg = 0;
  // initialize checksum
  mod_som_aggregator_ptr->consumer_ptr->chksum=0;
  // initialize element skipped
  mod_som_aggregator_ptr->consumer_ptr->record_skipped=0;
  // initialize timestamp
  mod_som_aggregator_ptr->consumer_ptr->timestamp=0;


  return (mod_som_aggregator_ptr->status =
      mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK));

}



/*******************************************************************************
 * @brief
 *   a define the data format of the data aggregator.
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/


mod_som_status_t mod_som_aggregator_consumer_mode_f(CPU_INT16U argc,
                                                    CPU_CHAR *argv[])
{
  RTOS_ERR  p_err;

  if (argc==1){
      printf("aggreg.format %u\r\n.", mod_som_aggregator_ptr->consumer_format);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_aggregator_ptr->consumer_format=shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: som.mode mode "
             "(0:none, 1:EPSA, 2:FCTA, 3:EPSB, 4:FCTB, ...)\r\n");
      break;
    }
  }
  return MOD_SOM_STATUS_OK;
}


/*******************************************************************************
 * @brief
 *   construct msq queue
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_construct_msq_queue_f(){

  RTOS_ERR  err;


// ALB create msg queue to store the task messages
// The messages are added to the queue by OSPost when mod_som_printf is invoked
// This list is part of the streaming consumer.
OSQCreate(&mod_som_aggregator_ptr->msg_queue,           /*   Pointer to user-allocated message queue.          */
        "MOD AGGREG Message Queue",          /*   Name used for debugging.                          */
        MOD_SOM_AGGREGATOR_MSG_QUEUE_COUNT,                     /*   Queue will have 10 messages maximum.              */
        &err);

//Stall if OSQCreate fails
APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
    return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE);

//Flush all message in the queue.
// ALB From what I understand:Be carefull using OSQFlush it only de-reference the message.
// ALB It does not clear the memory.
OSQFlush(&mod_som_aggregator_ptr->msg_queue,&err);
//Stall if OSQFlush fails
APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//return error if OSQ fails
// ALB This probably useless because the code would stall if there is an error
if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
    return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_FLUSH_QUEUE);

mod_som_aggregator_ptr->msg_blk_size = MOD_SOM_AGGREGATOR_DEFAULT_SIZE_LARGEST_BLOCK;

// Create Dynamic memory pool. TODO SN comment on the use of dynamic memory pool.
Mem_DynPoolCreate(
        "MOD SOM AGGREG Dynamic Memory Pool",
        &mod_som_aggregator_ptr->dyn_mem_pool,
        DEF_NULL,
        sizeof(mod_som_aggregator_xfer_t)+mod_som_aggregator_ptr->msg_blk_size,
        sizeof(CPU_ALIGN),//LIB_MEM_BUF_ALIGN_AUTO,
        MOD_SOM_AGGREGATOR_MSG_QUEUE_COUNT,
        2*MOD_SOM_AGGREGATOR_MSG_QUEUE_COUNT,
        &err);
//Stall if Mem_DynPoolCreate fails
APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//return an error if Mem_DynPoolCreate fails
// ALB This probably useless because the code would stall if there is an error
if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE))
    return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY);


// Create overflow flag is to many messages are in the list
mod_som_aggregator_ptr->listoverflow_flag=false;

return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_aggregator_data_f
 * @abstract
 *     queue up data to process in the consumer task
 * @discussion
 * @param data_ptr
 *     pointer to data to process
 * @param data_length
 *     length of data to process
 * @param done_process_flag_ptr
 *     pointer to a boolean flag to indicate when process is done.
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_process_data_f(uint8_t *data_ptr, uint32_t data_length,
        bool * done_process_flag_ptr){
    // check if OBP is initialized
    if(!mod_som_aggregator_ptr->initialized_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_INITIALIZED);

    // check if OBP is started
    if(!mod_som_aggregator_ptr->started_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_STARTED);

    // declare xfer structure and function status
    // xfer: receives the data from efe consumer task.
    mod_som_aggregator_xfer_ptr_t mod_som_aggregator_xfer_item_ptr;
    mod_som_status_t mod_som_status;

    // set function status to "all good"
    mod_som_status = MOD_SOM_STATUS_OK;

    // set the transfer structure: we received a block from efe.c
    // we are now queuing this block to the obp_consumer task.
    // Once the block processed the consumer task should return a "done_process_flag_ptr" flag
    //
    mod_som_aggregator_xfer_item_ptr = mod_som_aggregator_new_xfer_item_f();

    //Check if transfer structure initialization is successful
    if(mod_som_aggregator_xfer_item_ptr == DEF_NULL){
//        mod_som_aggregator_free_xfer_item_f(mod_som_aggregator_xfer_item_ptr);
        printf("can not add a record to the aggregator %s.\r\n",data_ptr);
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY);
    }
    // deal with the done process flag.
    // ALB TODO come back when ready to design the synchronization between efe and aggregator.
    mod_som_aggregator_xfer_item_ptr->done_process_flag_ptr = done_process_flag_ptr;
    if(done_process_flag_ptr != DEF_NULL){
        *(mod_som_aggregator_xfer_item_ptr->done_process_flag_ptr)  = false;
    }

    mod_som_aggregator_xfer_item_ptr->data_ptr    = data_ptr;
    mod_som_aggregator_xfer_item_ptr->data_length = data_length;

    // add to queue the  new xfer item
    //ALB even if do not submit to the queue I still have an issue when reading SBE41 data
    mod_som_status = mod_som_aggregator_add_to_queue_f(mod_som_aggregator_xfer_item_ptr);

    // Free the xfer item memory if add to queue fails
    if(mod_som_status != MOD_SOM_STATUS_OK){
        mod_som_aggregator_free_xfer_item_f(mod_som_aggregator_xfer_item_ptr);
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ENQUEUE);
    }

    return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_aggregator_new_xfer_item_f
 * @abstract
 *     Create new I/O transfer item
 * @discussion
 *     Using dynamic memory pool for IO transfer item to create new memory
 *     pointer for a new I/O transfer item
 * @return
 *     Pointer to new I/O transfer item, if pointer is null, this indicates
 *     error in allocating memory
 ******************************************************************************/
mod_som_aggregator_xfer_ptr_t mod_som_aggregator_new_xfer_item_f(void){
  if(!mod_som_aggregator_ptr->initialized_flag)
        return DEF_NULL;
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED);
  if(!mod_som_aggregator_ptr->started_flag)
        return DEF_NULL;
//        return mod_som_sdio_encode_status_f(MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED);
    RTOS_ERR err;
    // declare xfer structure and function status
    // xfer: receives the data from efe consumer task.
    mod_som_aggregator_xfer_ptr_t mod_som_aggregator_xfer_item_ptr= \
            (mod_som_aggregator_xfer_ptr_t)Mem_DynPoolBlkGet(
                    &mod_som_aggregator_ptr->dyn_mem_pool,
                    &err);
    // Check error code
//    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    if(mod_som_aggregator_xfer_item_ptr==DEF_NULL)
        return DEF_NULL;

    if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return DEF_NULL;

    Mem_Set(mod_som_aggregator_xfer_item_ptr,0,sizeof(mod_som_aggregator_xfer_ptr_t));

    mod_som_aggregator_xfer_item_ptr->data_length = 0;
    mod_som_aggregator_xfer_item_ptr->size = sizeof(mod_som_aggregator_xfer_ptr_t);
    //this is so we can find the wrapper class in case we need to clear memory
    return mod_som_aggregator_xfer_item_ptr;
}

/*******************************************************************************
 * @function
 *     mod_som_aggregator_add_to_queue_f
 * @abstract
 * @discussion
 * @param       xfer_item_ptr
 *     Item to add
 * @return
 *     Status of adding to queue
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_add_to_queue_f(mod_som_aggregator_xfer_ptr_t xfer_item_ptr){
  if(!mod_som_aggregator_ptr->initialized_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_INITIALIZED);
  if(!mod_som_aggregator_ptr->started_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_STARTED);

  //Initialize OS error.
  RTOS_ERR err;

  //ALB check the Nb of entries and pass if it the list is full.
  if(mod_som_aggregator_ptr->msg_queue.MsgQ.NbrEntries>AGGREGATOR_OVF_MSG_LIST_THERSHOLD){
      mod_som_aggregator_ptr->listoverflow_flag=true;
      //ALB TODO say How I want to handle the overflow?
      //ALB TODO check for ovf.
      //ALB TODO handle ovf.
      //ALB TODO flag the ovf.
      //ALB TODO So the user knows how much time or sample we missed
  }else{
      mod_som_aggregator_ptr->listoverflow_flag=false;
  }

  if (!mod_som_aggregator_ptr->listoverflow_flag){
      OSQPost (&mod_som_aggregator_ptr->msg_queue,
               xfer_item_ptr,
               xfer_item_ptr->size,
               OS_OPT_POST_FIFO,
               &err);

      if((RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)){
          mod_som_aggregator_free_xfer_item_f(xfer_item_ptr);
          return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE);
      }
      //      printf("\n add to queue Nb entries %u \n",mod_som_sdio_struct.msg_queue.MsgQ.NbrEntries);

  }else{
      mod_som_aggregator_free_xfer_item_f(xfer_item_ptr);
//      printf("\n Not adding to SD queue Nb entries %u \n",mod_som_sdio_struct.msg_queue.MsgQ.NbrEntries);
  }
  return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @function
 *     mod_som_aggregator_free_xfer_item_f
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
mod_som_status_t mod_som_aggregator_free_xfer_item_f(mod_som_aggregator_xfer_ptr_t xfer_item_ptr){
    RTOS_ERR err;
    if(!mod_som_aggregator_ptr->initialized_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_INITIALIZED);
    if(!mod_som_aggregator_ptr->started_flag)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_STARTED);

    Mem_DynPoolBlkFree(&mod_som_aggregator_ptr->dyn_mem_pool,
            (void *)xfer_item_ptr,
            &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        return mod_som_aggregator_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_FREE_MEMORY);
    return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   create consumer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_stop_consumer_task_f(){


  RTOS_ERR err;
  OSTaskDel(&aggregator_consumer_task_tcb,
             &err);

  mod_som_aggregator_ptr->started_flag=false;


  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_aggregator_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_START_CONSUMER_TASK));

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   create consumer task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_start_consumer_task_f(){


  RTOS_ERR err;
  // aggregator consumer Task: this tak will packetize the different SOM data
   mod_som_aggregator_ptr->started_flag=true;
   mod_som_aggregator_ptr->consumer_ptr->phase = collect;
   mod_som_aggregator_ptr->consumer_ptr->packet_ready_flg = false;
   mod_som_aggregator_ptr->consumer_ptr->efe_ready_flg    = false;
   mod_som_aggregator_ptr->consumer_ptr->sbe_ready_flg    = false;
   mod_som_aggregator_ptr->consumer_ptr->alti_ready_flg   = false;
   mod_som_aggregator_ptr->consumer_ptr->vecnav_ready_flg = false;

//   //ALB define the base ptr for the altimeter sub_packet
//   mod_som_aggregator_ptr->consumer_ptr->alti_packet_data_ptr=
//                mod_som_aggregator_ptr->consumer_ptr->base_alti_packet_data_ptr;
//
//   //ALB define the base ptr for the sea bird sub_packet
//   mod_som_aggregator_ptr->consumer_ptr->sbe_packet_data_ptr=
//                 mod_som_aggregator_ptr->consumer_ptr->base_sbe_packet_data_ptr;
//
//   //ALB define the base ptr for the efe bird sub_packet
//   mod_som_aggregator_ptr->consumer_ptr->efe_packet_data_ptr=
//       mod_som_aggregator_ptr->consumer_ptr->base_efe_packet_data_ptr;
//
//   //ALB define the base ptr for the vector nav sub_packet
//   mod_som_aggregator_ptr->consumer_ptr->vecnav_packet_data_ptr=
//       mod_som_aggregator_ptr->consumer_ptr->base_vecnav_packet_data_ptr;

   //ALB initialize packet ready flg
   mod_som_aggregator_ptr->consumer_ptr->packet_ready_flg = 0;
   //ALB initialize checksum
   mod_som_aggregator_ptr->consumer_ptr->chksum=0;
   //ALB initialize element skipped
   mod_som_aggregator_ptr->consumer_ptr->record_skipped=0;
   //ALB initialize timestamp
   mod_som_aggregator_ptr->consumer_ptr->timestamp=0;




   OSTaskCreate(&aggregator_consumer_task_tcb,
                        "aggregator consumer task",
                        mod_som_aggregator_consumer_task_f,
                        DEF_NULL,
                        MOD_SOM_AGGREGATOR_CONSUMER_TASK_PRIO,
            &aggregator_consumer_task_stk[0],
            (MOD_SOM_AGGREGATOR_CONSUMER_TASK_STK_SIZE / 10u),
            MOD_SOM_AGGREGATOR_CONSUMER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_aggregator_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_aggregator_encode_status_f(MOD_SOM_STATUS_OK);
}



/*******************************************************************************
 * @brief
 *   consumer task function
 *
 *   this will be a state machine:
 *    1 - gather peripheral data
 *    2 - packetize them in a pre-defined fixed length format.
 *    3 - send or SD store the packet.
 *
 *   ALB TODO the packet payload checksum is done in phase 2.
 *   ALB TODO I could be smarter and use the record payload checksum
 *   ALB TODO to compute the packet payload checksum.
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
void mod_som_aggregator_consumer_task_f(void  *p_arg){
  RTOS_ERR  err;



  mod_som_aggregator_xfer_ptr_t tmp_mod_som_aggregator_xfer_item_ptr = DEF_NULL;
  (void)p_arg; // Deliberately unused argument

  OS_MSG_SIZE tmp_mod_som_aggregator_xfer_item_size;
  CPU_TS time_passed_msg_pend;


  while (DEF_ON) {

//      printf("Pre Pend - Msg  Entries %i\r\n", (int) mod_som_aggregator_ptr->msg_queue.MsgQ.NbrEntries);
      tmp_mod_som_aggregator_xfer_item_ptr =
              (mod_som_aggregator_xfer_ptr_t)OSQPend(&mod_som_aggregator_ptr->msg_queue,
                      0,
                      OS_OPT_PEND_NON_BLOCKING,
                      &tmp_mod_som_aggregator_xfer_item_size,
                      &time_passed_msg_pend,
                      &err);

      //ALB free memory
      if (tmp_mod_som_aggregator_xfer_item_ptr!=DEF_NULL){
          mod_som_aggregator_free_xfer_item_f(tmp_mod_som_aggregator_xfer_item_ptr);
      }

//      printf("Post Pend - Msg  Entries %i\r\n", (int) mod_som_aggregator_ptr->msg_queue.MsgQ.NbrEntries);
            /************************************************************************/
      //ALB the aggregator can be a state machine since I think the
      //ALB different phases have to be sequencial.
      switch (mod_som_aggregator_ptr->consumer_ptr->phase){
        case collect:
          //ALB phase 1
//          mod_som_aggregator_collect_packet_f(tmp_mod_som_aggregator_xfer_item_ptr);
          break;
        case consume:
          //ALB Aggreg phase 2 sent/store packets
          //ALB TODO the packet payload checksum is done in phase 2.
          //ALB TODO I could be smarter and use the record payload checksum
          //ALB TODO in phase 1 to compute the packet payload checksum.
//          mod_som_aggregator_consume_packet_f();
          break;
        case packet_sent:
          mod_som_aggregator_wait_packet_f();
          break;
        case clear:
          //ALB Aggreg phase 3 clear packet bytes
          mod_som_aggregator_clear_packet_f();
          break;
      }
      /************************************************************************/


      // Delay Start Task execution for
      OSTimeDly( MOD_SOM_AGGREGATOR_CONSUMER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                 OS_OPT_TIME_DLY,          //   from now.
                 &err);
      //   Check error code.
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // end of while (DEF_ON)

  PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.

}

/*******************************************************************************
 * @brief
 *   mod_som_aggregator_collect_f
 *   Collect phase of the consumer state machine
 *   - this will fill the payload of the packet with the record payload from peripheral data.
 *   - change phase to consume
 *   - re-initialize the ready flags
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/


void mod_som_aggregator_collect_packet_f(mod_som_aggregator_xfer_ptr_t tmp_mod_som_aggregator_xfer_item_ptr)
{
  uint8_t * local_record_ptr;
  char      record_tag[5]={0};
  uint8_t * current_subpacket_data_ptr;
  char      str_payload_size[9]={0};
  uint32_t  payload_size;
  uint8_t * payload_ptr;
  uint32_t * local_nb_record;

  if (tmp_mod_som_aggregator_xfer_item_ptr!=DEF_NULL){
      //ALB get local record
      local_record_ptr     = tmp_mod_som_aggregator_xfer_item_ptr->data_ptr;
      //ALB get record tag.
      memcpy(record_tag,&local_record_ptr[1],4);
      //ALB get payload size str.
      memcpy(str_payload_size,&local_record_ptr[21],8);
      //ALB convert get payload size str into uint32_t.
      payload_size     = (int32_t) strtol(str_payload_size, NULL, 16);
//      payload_ptr          = local_record_ptr+
//                         mod_som_aggregator_ptr->consumer_ptr->length_header  +
//                         MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH;


      //ALB Check what block this is (EFE,SBE,VECNAV,ALTI).
      //ALB Read the tag of the record (bytes 1-4)
      //ALB Switch tag- store the elements. What happens when too many elements?
      //ALB I decided to allow for go to phase 2 (stream/SD store) if there is at least
      //ALB 1 record of each peripherals; i.e., the "slowest" peripheral to send and
      //ALB msg is the trigger.
      //ALB TODO change the hard-coded module tag.
      if (!strcmp(record_tag,"EFE3")){
          mod_som_aggregator_ptr->consumer_ptr->efe_ready_flg    = true;
          local_nb_record= &mod_som_aggregator_ptr->consumer_ptr->nb_efe_record;
          current_subpacket_data_ptr=mod_som_aggregator_ptr->consumer_ptr->
                                                      base_efe_packet_data_ptr +
                                                     *local_nb_record          *
                         mod_som_aggregator_ptr->config_ptr->efe_record_length;
          payload_size=mod_som_aggregator_ptr->config_ptr->efe_record_length;
          //ALB increment nb_efe_record, modulo maximum record per packet
              (*local_nb_record)++;
              (*local_nb_record) %=
                  mod_som_aggregator_ptr->settings_ptr->efe_record_per_packet;
              printf("EFE %lu\r\n",*local_nb_record);
      }
      if (!strcmp(record_tag,"SB49")){
          mod_som_aggregator_ptr->consumer_ptr->sbe_ready_flg    =  true;
          local_nb_record= &mod_som_aggregator_ptr->consumer_ptr->nb_sbe_record;
          current_subpacket_data_ptr=mod_som_aggregator_ptr->consumer_ptr->
                                                      base_sbe_packet_data_ptr +
                                                     *local_nb_record          *
                         mod_som_aggregator_ptr->config_ptr->sbe_record_length;
          //ALB increment nb_sbe_record, modulo maximum record per packet
          payload_size=mod_som_aggregator_ptr->config_ptr->sbe_record_length;
          (*local_nb_record)++;
          (*local_nb_record) %=
                  mod_som_aggregator_ptr->settings_ptr->sbe_record_per_packet;
          printf("SBE %lu\r\n",*local_nb_record);
      }
      if (!strcmp(record_tag,"VNAV")){
          mod_som_aggregator_ptr->consumer_ptr->vecnav_ready_flg =  true;
          local_nb_record= &mod_som_aggregator_ptr->consumer_ptr->nb_vecnav_record;
          current_subpacket_data_ptr=mod_som_aggregator_ptr->consumer_ptr->
                                                   base_vecnav_packet_data_ptr +
                                                   *local_nb_record *
                         mod_som_aggregator_ptr->config_ptr->vecnav_record_length;
          //ALB increment nb_vecnav_record, modulo maximum record per packet
          payload_size=mod_som_aggregator_ptr->config_ptr->vecnav_record_length;
          (*local_nb_record)++;
          (*local_nb_record) %=
                  mod_som_aggregator_ptr->settings_ptr->vecnav_record_per_packet;
          printf("VNAV %lu\r\n",*local_nb_record);
      }
      if (!strcmp(record_tag,"ALTI")){
          mod_som_aggregator_ptr->consumer_ptr->alti_ready_flg   = true;
          local_nb_record= &mod_som_aggregator_ptr->consumer_ptr->nb_alti_record;
          current_subpacket_data_ptr=mod_som_aggregator_ptr->consumer_ptr->
                                                     base_alti_packet_data_ptr +
                                                    *local_nb_record           *
                         mod_som_aggregator_ptr->config_ptr->alti_record_length;
          //ALB increment nb_alti_record, modulo maximum record per packet
          payload_size=mod_som_aggregator_ptr->config_ptr->alti_record_length;
          (*local_nb_record)++;
          (*local_nb_record) %=
                  mod_som_aggregator_ptr->settings_ptr->alti_record_per_packet;
          printf("ALTI %lu\r\n",*local_nb_record);

      }

      //ALB Copy the record in the packet
      //ALB I offset the ptr with length_header.
      //ALB THe length header is/should be the same header for all the records.
      memcpy(current_subpacket_data_ptr,
             local_record_ptr,
             payload_size);

      //ALB update the done rdy flag. To be used be the record producer
      //ALB (e.g., efe, sbe, alti)
      *tmp_mod_som_aggregator_xfer_item_ptr->done_process_flag_ptr=true;
      //ALB free memory
      mod_som_aggregator_free_xfer_item_f(tmp_mod_som_aggregator_xfer_item_ptr);

      //ALB Check if we are ready for phase 2.
      if(mod_som_aggregator_ptr->consumer_ptr->alti_ready_flg   &
         mod_som_aggregator_ptr->consumer_ptr->vecnav_ready_flg &
         mod_som_aggregator_ptr->consumer_ptr->sbe_ready_flg    &
         mod_som_aggregator_ptr->consumer_ptr->efe_ready_flg )
        {
          mod_som_aggregator_ptr->consumer_ptr->phase              = consume;
          mod_som_aggregator_ptr->consumer_ptr->alti_ready_flg     = false;
          mod_som_aggregator_ptr->consumer_ptr->sbe_ready_flg      = false;
          mod_som_aggregator_ptr->consumer_ptr->vecnav_ready_flg   = false;
          mod_som_aggregator_ptr->consumer_ptr->efe_ready_flg      = false;
          mod_som_aggregator_ptr->consumer_ptr->nb_efe_record    = 0;
          mod_som_aggregator_ptr->consumer_ptr->nb_sbe_record    = 0;
          mod_som_aggregator_ptr->consumer_ptr->nb_vecnav_record = 0;
          mod_som_aggregator_ptr->consumer_ptr->nb_alti_record   = 0;
          printf("AGGREG: end phase 1.\r\n");
        }
    }//ALB end of if DEF_NULL
}


/*******************************************************************************
 * @brief
 *   mod_som_aggregator_consume_f
 *   consume phase of the consumer state machine
 *   - compute the header and payload
 *   - change phase to consume
 *   - re-initialize the ready flags
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/


void mod_som_aggregator_consume_packet_f()
{
  uint64_t tick;
  uint8_t * curr_packet_ptr;

  //ALB ptr to the packet checksum.
  curr_packet_ptr =
      &mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr[
                          mod_som_aggregator_ptr->consumer_ptr->length_header  +
                          MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH               ];

  //get the timestamp for the record header
  tick=sl_sleeptimer_get_tick_count64();
  mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                    &mod_som_aggregator_ptr->timestamp);

  //create header
  mod_som_aggregator_header_f(mod_som_aggregator_ptr->consumer_ptr);
  //add header to the beginning of the stream block
  memcpy(mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr, \
         mod_som_aggregator_ptr->consumer_ptr->header,
         mod_som_aggregator_ptr->consumer_ptr->length_header+
         MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH);

  mod_som_aggregator_ptr->consumer_ptr->chksum=0;
  for (int i=0;i<mod_som_aggregator_ptr->consumer_ptr->payload_length;i++)
    {
      mod_som_aggregator_ptr->consumer_ptr->chksum ^=\
          curr_packet_ptr[i];
    }

  curr_packet_ptr =
      &mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr[
                          mod_som_aggregator_ptr->consumer_ptr->length_header  +
                          MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH                +
                          mod_som_aggregator_ptr->consumer_ptr->payload_length];

  // the curr_consumer_element_ptr should be at the right place to
  // write the checksum already
  //write checksum at the end of the packet.
  *(curr_packet_ptr++) = '*';
  *((uint16_t*)curr_packet_ptr) = \
      mod_som_int8_2hex_f(mod_som_aggregator_ptr->consumer_ptr->chksum);
  curr_packet_ptr += 2;
  *(curr_packet_ptr++) = '\r';
  *(curr_packet_ptr++) = '\n';

  //ALB Here we send the record to other task:
  //ALB streaming, SD store, aggregate, obp.
  switch(mod_som_aggregator_ptr->mode){
    case 0:
      mod_som_aggregator_ptr->consumer_ptr->done_stream_flg=false;
      mod_som_io_stream_data_f(
          mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr,
          mod_som_aggregator_ptr->consumer_ptr->packet_length,
          &mod_som_aggregator_ptr->consumer_ptr->done_stream_flg);
      break;
    case 1:
      mod_som_aggregator_ptr->consumer_ptr->done_sdwrite_flg=false;
      mod_som_sdio_write_data_f(
          mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr,
          mod_som_aggregator_ptr->consumer_ptr->packet_length,
          &mod_som_aggregator_ptr->consumer_ptr->done_sdwrite_flg);
      break;
    case 2:
      mod_som_aggregator_ptr->consumer_ptr->done_stream_flg=false;
      mod_som_aggregator_ptr->consumer_ptr->done_sdwrite_flg=false;
      mod_som_io_stream_data_f(
          mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr,
          mod_som_aggregator_ptr->consumer_ptr->packet_length,
          &mod_som_aggregator_ptr->consumer_ptr->done_stream_flg);
      mod_som_sdio_write_data_f(
          mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr,
          mod_som_aggregator_ptr->consumer_ptr->packet_length,
          &mod_som_aggregator_ptr->consumer_ptr->done_sdwrite_flg);
      break;
    default:
      break;
  }
  mod_som_aggregator_ptr->consumer_ptr->phase  = packet_sent;

}

/*******************************************************************************
 * @brief
 *   mod_som_aggregator_clear_packet_f
 *   clear phase of the consumer state machine
 *   - simply put all the bytes in the packet to 0
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/


void mod_som_aggregator_wait_packet_f()
{
  if(mod_som_aggregator_ptr->consumer_ptr->done_stream_flg |
     mod_som_aggregator_ptr->consumer_ptr->done_sdwrite_flg)
    {
      mod_som_aggregator_ptr->consumer_ptr->phase  = collect;
   }

}

/*******************************************************************************
 * @brief
 *   mod_som_aggregator_clear_packet_f
 *   clear phase of the consumer state machine
 *   - simply put all the bytes in the packet to 0
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/


void mod_som_aggregator_clear_packet_f()
{

  for (int i=0;i<mod_som_aggregator_ptr->consumer_ptr->packet_length;i++)
    {
          mod_som_aggregator_ptr->consumer_ptr->packet_data_ptr[i]=0;
    }
    mod_som_aggregator_ptr->consumer_ptr->phase=collect;
}


/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_aggregator_header_f(mod_som_aggregator_data_consumer_ptr_t consumer_ptr)
{

  //time stamp
  uint32_t t_hex[2];
  uint8_t * local_header;

  t_hex[0] = (uint32_t) (mod_som_aggregator_ptr->timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_aggregator_ptr->timestamp;

  //header  contains SBE,flags. Currently flags are hard coded to 0x1e
  //time stamp will come at the end of header
      sprintf((char*) consumer_ptr->header,  \
          "$%s%08x%08x%08x*FF", \
          mod_som_aggregator_ptr->settings_ptr->data_header_text, \
          (int) t_hex[0],\
          (int) t_hex[1],\
          (int) consumer_ptr->payload_length);

      consumer_ptr->header_chksum=0;
      for(int i=0;i<mod_som_aggregator_ptr->consumer_ptr->length_header;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
        {
          consumer_ptr->header_chksum ^=\
              consumer_ptr->header[i];
        }


      // the curr_consumer_element_ptr should be at the right place to
      // write the checksum already
      //write checksum at the end of the steam block (record).
      local_header = &consumer_ptr->header[
                         mod_som_aggregator_ptr->consumer_ptr->length_header+1];
      *((uint16_t*)local_header) = \
          mod_som_int8_2hex_f(consumer_ptr->header_chksum);

}






/*******************************************************************************
 * @function
 *     mod_som_io_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the bit 16-23 if the
 *     higher bits show the status code is of MOD SOM FOO BAR
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_aggregator_decode_status_f(mod_som_status_t mod_som_status){
    if(mod_som_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    uint8_t status_prefix;
    uint8_t decoded_status;
    MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
    if(status_prefix != MOD_SOM_AGGREGATOR_STATUS_PREFIX){
        return 0xffU;
    }
    return decoded_status;
}

/*******************************************************************************
 * @function
 *     mod_som_aggregator_encode_status_f
 * @abstract
 *     Encode status of MOD SOM I/O error code to MOD SOM general status
 * @discussion TODO SN
 *     Encode status of MOD SOM IO status code to MOD SOM general status, where
 *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
 *     lower 16 bits are reserved for flags
 * @param       mod_som_io_status
 *     MOD SOM I/O error code
 * @return
 *     MOD SOM status code
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_encode_status_f(uint8_t mod_som_io_status){
    if(mod_som_io_status==MOD_SOM_STATUS_OK)
        return MOD_SOM_STATUS_OK;
    return MOD_SOM_ENCODE_STATUS(MOD_SOM_AGGREGATOR_STATUS_PREFIX, mod_som_io_status);
}

#endif


