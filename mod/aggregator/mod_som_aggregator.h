/*******************************************************************************
 * @file mod_som_aggregator.h
 * @brief MOD SOM aggregator API
 * @date Mar 26, 2020
 * @author aleboyer (aleboyer@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * This API implementation file defines functions to be used with FOO BAR
 * module running on the MOD SOM board.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_AGGREGATOR_H_
#define MOD_SOM_AGGREGATOR_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_AGGREGATOR_STATUS_PREFIX        99U
#define MOD_SOM_AGGREGATOR_SETTING_STR_LENGTH   8
//ALB define header
#define MOD_SOM_AGGREGATOR_MAX_HEADER_SIZE      40
#define MOD_SOM_AGGREGATOR_TAG_LENGTH            4
#define MOD_SOM_AGGREGATOR_HEXTIMESTAMP_LENGTH  16
#define MOD_SOM_AGGREGATOR_HEXPAYLOAD_LENGTH     8
#define MOD_SOM_AGGREGATOR_HEXSYNC_LENGTH        1
#define MOD_SOM_AGGREGATOR_HEXCHECKSUM_LENGTH    2
#define MOD_SOM_AGGREGATOR_PAYLOAD_CHKSUM_LGTH   5
#define MOD_SOM_AGGREGATOR_HEADER_CHKSUM_LGTH    3


#define MOD_SOM_AGGREGATOR_PACKET_FORMAT_DEFAULT 0
#define MOD_SOM_AGGREGATOR_EFE_DEFAULT_RECORD_PER_PACKET    1
#define MOD_SOM_AGGREGATOR_SBE_DEFAULT_RECORD_PER_PACKET    3
#define MOD_SOM_AGGREGATOR_VECNAV_DEFAULT_RECORD_PER_PACKET 1
#define MOD_SOM_AGGREGATOR_ALTI_DEFAULT_RECORD_PER_PACKET   1

#define MOD_SOM_AGGREGATOR_NONE_DATA_HEADER_TEXT   "NONE"
#define MOD_SOM_AGGREGATOR_EPSIA_DATA_HEADER_TEXT  "EPSA"
#define MOD_SOM_AGGREGATOR_EPSIB_DATA_HEADER_TEXT  "EPSB"
#define MOD_SOM_AGGREGATOR_FCTDA_DATA_HEADER_TEXT  "FCTA"
#define MOD_SOM_AGGREGATOR_FCTDB_DATA_HEADER_TEXT  "FCTB"


#define MOD_SOM_AGGREGATOR_STATUS_FAIL_INIT_CMD 0x2U
#define MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_ALLOCATE_MEMORY 0x3U

//ALB TASK defines
#define MOD_SOM_AGGREGATOR_CONSUMER_TASK_PRIO             18u
#define MOD_SOM_AGGREGATOR_CONSUMER_TASK_STK_SIZE      16384u
#define MOD_SOM_AGGREGATOR_CONSUMER_DELAY                  10      // delay for consumer 2 (ex: 4000 = 1 time / 4 secs).


//ALB Msg queue
#define MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_INITIALIZED                  0x04U
#define MOD_SOM_AGGREGATOR_STATUS_ERR_NOT_STARTED                      0x05U
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ENQUEUE                  0x06U
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE        0x08U
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_FREE_MEMORY              0x09u
#define MOD_SOM_AGGREGATOR_STATUS_FAIL_TO_START_CONSUMER_TASK          0x10u
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE         0x11u
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_FLUSH_QUEUE              0x12u
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY  0x13u
#define MOD_SOM_AGGREGATOR_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY          0x14U

#define AGGREGATOR_OVF_MSG_LIST_THERSHOLD                              0x128U


//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------


/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM SBE49
 ******************************************************************************/
typedef struct{

  uint32_t efe_record_length; //
  uint32_t sbe_record_length; //
  uint32_t vecnav_record_length; //
  uint32_t alti_record_length; //
  uint32_t actu_record_length; //

  uint32_t efe_element_per_record; //
  uint32_t sbe_element_per_record; //
  uint32_t vecnav_element_per_record; //
  uint32_t alti_element_per_record; //
  uint32_t actu_element_per_record; //

}mod_som_aggregator_config_t, *mod_som_aggregator_config_ptr_t;

/*******************************************************************************
 * @brief
 *     a settings structure SBE49 data
 * @description
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char data_header_text[MOD_SOM_AGGREGATOR_SETTING_STR_LENGTH];
    char sn[MOD_SOM_AGGREGATOR_SETTING_STR_LENGTH];
    uint32_t packet_format;
    uint32_t efe_record_per_packet; //
    uint32_t sbe_record_per_packet; //i.e. stream/store frequency.
    uint32_t vecnav_record_per_packet; //i.e. stream/store frequency.
    uint32_t alti_record_per_packet; //i.e. stream/store frequency.
    uint32_t actu_record_per_packet;
    uint32_t initialize_flag;
}mod_som_aggregator_settings_t, *mod_som_aggregator_settings_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure managing the AGGREGATOR data consumers
 ******************************************************************************/
typedef struct{
  uint64_t  timestamp;
  uint64_t  cnsmr_cnt;

  uint32_t  packet_length;           //ALB length of the streaming data buffer
  uint32_t  nb_efe_record;
  uint32_t  nb_vecnav_record;
  uint32_t  nb_sbe_record;
  uint32_t  nb_alti_record;
  uint32_t  record_skipped;
  uint32_t  payload_length;

  uint8_t * packet_data_ptr;             //ALB pointer to stream data
  uint8_t * base_efe_packet_data_ptr;    //ALB just ptr no need to alloc mem
  uint8_t * base_sbe_packet_data_ptr;    //ALB just ptr no need to alloc mem
  uint8_t * base_alti_packet_data_ptr;   //ALB just ptr no need to alloc mem
  uint8_t * base_vecnav_packet_data_ptr; //ALB just ptr no need to alloc mem

//  uint8_t * efe_packet_data_ptr;
//  uint8_t * sbe_packet_data_ptr;
//  uint8_t * alti_packet_data_ptr;
//  uint8_t * vecnav_packet_data_ptr;

  uint8_t   chksum;
  uint8_t   header[MOD_SOM_AGGREGATOR_MAX_HEADER_SIZE];//TODO move this to consumer_ptr
  uint8_t   length_header;
  uint8_t   header_chksum;
  uint8_t   packet_ready_flg;
  uint8_t   efe_ready_flg;
  uint8_t   alti_ready_flg;
  uint8_t   sbe_ready_flg;
  uint8_t   vecnav_ready_flg;

  bool     done_stream_flg;
  bool     done_sdwrite_flg;

  enum      {collect,consume,packet_sent,clear}phase;
}
mod_som_aggregator_data_consumer_t, *mod_som_aggregator_data_consumer_ptr_t;

typedef struct{
    uint32_t size;
    uint8_t * data_ptr;
    uint32_t data_length;
    volatile bool *done_process_flag_ptr;
}mod_som_aggregator_xfer_t,*mod_som_aggregator_xfer_ptr_t;



/*******************************************************************************
 * @brief
 *     run-time device structure for MOD SOM AGGREGATOR
 * @description
 ******************************************************************************/
typedef struct{
    volatile mod_som_status_t status;
    bool initialized_flag;
    uint8_t  consumer_format;  //0:EPSA, 1:FCTA, 2:EPSB, 3:FCTB
    uint8_t  mode;             //0:stream, 1:SD store, 2: both?
    uint64_t timestamp;
    uint64_t sample_count;
    uint8_t  started_flag;

    mod_som_aggregator_config_ptr_t          config_ptr;
    mod_som_aggregator_settings_ptr_t        settings_ptr;
    mod_som_aggregator_data_consumer_ptr_t   consumer_ptr;

    OS_Q msg_queue;
    uint32_t msg_blk_size;
    MEM_DYN_POOL dyn_mem_pool;
    uint8_t  listoverflow_flag;

}mod_som_aggregator_t,*mod_som_aggregator_ptr_t;



//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize AGGREGATOR ,odule, if shell is available, then the command table is added
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_init_f();


/*******************************************************************************
 * @brief
 * set the format of the aggregator data block.
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_consumer_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]);

mod_som_status_t mod_som_aggregator_construct_msq_queue_f();

mod_som_status_t mod_som_aggregator_process_data_f(uint8_t *data_ptr, uint32_t data_length,
        bool * done_process_flag_ptr);

mod_som_aggregator_xfer_ptr_t mod_som_aggregator_new_xfer_item_f(void);
mod_som_status_t mod_som_aggregator_add_to_queue_f(mod_som_aggregator_xfer_ptr_t xfer_item_ptr);
mod_som_status_t mod_som_aggregator_free_xfer_item_f(mod_som_aggregator_xfer_ptr_t xfer_item_ptr);
mod_som_status_t mod_som_aggregator_stop_consumer_task_f();
mod_som_status_t mod_som_aggregator_start_consumer_task_f();
void mod_som_aggregator_consumer_task_f(void  *p_arg);
void mod_som_aggregator_collect_packet_f(mod_som_aggregator_xfer_ptr_t tmp_mod_som_aggregator_xfer_item_ptr);
void mod_som_aggregator_consume_packet_f();
void mod_som_aggregator_wait_packet_f();
void mod_som_aggregator_clear_packet_f();


/*******************************************************************************
 * @brief
 *   a function to output hello
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_aggregator_say_hello_world_f();


mod_som_status_t mod_som_aggregator_construct_consumer_ptr_f();

void mod_som_aggregator_header_f(mod_som_aggregator_data_consumer_ptr_t consumer_ptr);
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
mod_som_status_t mod_som_aggregator_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_aggregator_decode_status_f(mod_som_status_t mod_som_status);
#endif /* MOD_SOM_AGGREGATOR_H_ */
