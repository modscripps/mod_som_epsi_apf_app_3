/*******************************************************************************
 * @file mod_som_vecnav.h
 * @brief MOD SOM VECNAV API/header
 * @date Feb 6, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
 * @update aleboyer@ucsd.edu
 * @date   Jan 27 2021
 *
 * @description
 * This API/header file defines configuration, run time data handles, and
 * function placeholders to be used with an SBE 49 device connecting to the
 * MOD SOM board.
 * The ports definition are established in mod_bsp_cfg.h file.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_VECNAV_H_
#define MOD_SOM_VECNAV_H_
//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_VECNAV_OUTPUT0_SAMPLE_DATA_LENGTH    95
#define MOD_SOM_VECNAV_OUTPUT1_SAMPLE_DATA_LENGTH    24
#define MOD_SOM_VECNAV_OUTPUT2_SAMPLE_DATA_LENGTH    24
#define MOD_SOM_VECNAV_OUTPUT3_SAMPLE_DATA_LENGTH    40

#define MOD_SOM_VECNAV_DATA_FORMAT_DEFAULT                0
#define MOD_SOM_VECNAV_DATA_DEFAULT_SAMPLES_PER_RECORD    10
#define MOD_SOM_VECNAV_DATA_DEFAULT_SAMPLES_PER_BUFFER    640
#define MOD_SOM_VECNAV_TIMESTAMP_LENGTH                   8 //ALB
#define MOD_SOM_VECNAV_HEXTIMESTAMP_LENGTH                16 //ALB
#define MOD_SOM_VECNAV_DEFAULT_DATA_HEADER_TEXT           "VNAV"
#define MOD_SOM_VECNAV_HEADER_SYNC_LENGTH                 1
#define MOD_SOM_VECNAV_DEFAULT_DATA_HEADER_TEXT_LENGTH    4UL
#define MOD_SOM_VECNAV_SBE_CHANNEL_LENGTH                 8

#define MOD_SOM_VECNAV_SYNC_HEADER_LENGTH                 1
#define MOD_SOM_VECNAV_TAG_LENGTH                         4
#define MOD_SOM_VECNAV_HEXPAYLOAD_LENGTH                  8
#define MOD_SOM_VECNAV_HEADER_CHECKSUM_LENGTH             3

#define MOD_SOM_VECNAV_MAX_HEADER_SIZE                    100
#define MOD_SOM_VECNAV_CHECKSUM_SIZE                      5
#define MOD_SOM_VECNAV_STATUS_PREFIX 49U

#define MOD_SOM_VECNAV_STATUS_OK                      0U
#define MOD_SOM_VECNAV_STATUS_NOT_INITIALIZED         02U
#define MOD_SOM_VECNAV_STATUS_NOT_CONNECTED           03U
#define MOD_SOM_VECNAV_STATUS_FAIL_TO_ALLOCATE_MEMORY 04U
#define MOD_SOM_VECNAV_STATUS_FAIL_INIT_CMD           0x11U


#define MOD_SOM_VECNAV_STATUS_BUFFER_OVRFLW 05U

#define MOD_SOM_VECNAV_DATA_RX_LENGTH 64
#define MOD_SOM_VECNAV_DATA_LENGTH_TRUNCATION 2U

//ALB vec nav commands
#define MOD_SOM_VECNAV_CMD_VNASY1 "$VNASY,0*XX"
#define MOD_SOM_VECNAV_CMD_VNWRG1 "$VNWRG,05,115200*58"
#define MOD_SOM_VECNAV_CMD_VNWRG2 "$VNWRG,06,13*XX"
#define MOD_SOM_VECNAV_CMD_VNWNV  "$VNWNV*57"
#define MOD_SOM_VECNAV_CMD_VNASY2 "$VNASY,1*XX"

// data consumer
#define MOD_SOM_VECNAV_CONSUMER_TASK_PRIO             18u
#define MOD_SOM_VECNAV_CONSUMER_TASK_STK_SIZE       4096u
#define MOD_SOM_VECNAV_CONSUMER_DELAY                   2      // delay for consumer 2 (ex: 4000 = 1 time / 4 secs).
#define MOD_SOM_VECNAV_CONSUMER_PADDING                 5      // number of elements for padding for data consumer.
#define MOD_SOM_VECNAV_STATUS_FAIL_TO_START_CONSUMER_TASK 0x02u

//LDMA
#define MOD_SOM_VECNAV_LDMA_CH   2
#define MOD_SOM_VECNAV_LDMA_CLCK cmuClock_LDMA

//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------


/*******************************************************************************
 * @brief
 *     a typical record of VECNAV data
 * @description
 *     an VECNAV record would contain N element, a time stamp, and a producer count
 *     a header
 * @field timestamp
 *    time when first sample in record was taken
 * @field samples
 *     pointer of data samples
 * @field count
 *    count of samples recorded
 ******************************************************************************/
typedef struct{
    uint8_t*  elements_buffer;
    uint32_t * elements_map; //TODO figure out a way to move it to rec_buff.
    uint32_t  producer_indx; // mod sample count % max sample per buffer
}mod_som_vecnav_rec_buff_t, *mod_som_vecnav_rec_buff_ptr_t;



/*******************************************************************************
 * @brief
 *   Structure managing the VECNAV data consumers
 ******************************************************************************/
typedef struct{
  uint64_t  timestamp;

  uint32_t  record_length;           //ALB length of the streaming data buffer
  uint32_t  element_per_record;   //ALB maximum element per stream block
  uint32_t  payload_length;
  uint32_t  elmnts_skipped;
  uint32_t  header_chksum;

  uint8_t   data_ready_flg;          //ALB ???
  uint8_t   chksum;
  uint8_t   header[MOD_SOM_VECNAV_MAX_HEADER_SIZE];//TODO move this to consumer_ptr
  uint8_t   length_header;//TODO move this to consumer_ptr

  uint8_t * record_data_ptr;         //ALB pointer to stream data
  bool      consumed_flag;

}
mod_som_vecnav_data_consumer_t, *mod_som_vecnav_data_consumer_ptr_t;


/*******************************************************************************
 * @brief
 *   SBE sample structure
 ******************************************************************************/
typedef struct{
  float  pressure;
  float  temperature;
  float  salinity;
  float  conductivity;
  uint64_t timestamp;
}
mod_som_vecnav_sample_t, *mod_som_vecnav_sample_ptr_t;


/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM VECNAV
 * @description
 *   this structure stores configuration of MOD SOM VECNAV
 *   to set up default values, use mod_som_vecnav_default_config_f()
 *   afterward, user can modify fields inside if necessary before calling
 *   initialization functions
 * @field uart_port
 *     pointer to which UART port would this device use
 * @field tx
 *     configuration for TX - route location, port, and pin
 * @field rx
 *     configuration for RX - route location, port, and pin
 * @field en
 *     configuration for enabling - port, and pin
 * @field baud_rate
 *     frequency of communication via serial/UART
 * @field parity
 *     parity specification via serial/UART
 * @field sample_data_length
 *     length of data for each sample
 * @field samples_per_record
 *     number of samples to store for each record, each complete record is
 *     streamed out or stored in data files
 * @field data_header_text
 *     data header text - which is used to identify data records when streaming
 * @field status_header_text
 *     text header to identify device when indicate status via console
 * @field cmd_header_text
 *     command line header text so that we can identify device
 * @field direct_mod_exit_cmd
 *     command to keyword to exit direct communication mode
 ******************************************************************************/
typedef struct{

    mod_som_com_port_t port;
    uint32_t baud_rate;
    uint32_t sample_data_length;
    uint32_t element_length; //timstamp+sample_data_length
    uint32_t nb_record_per_buffer;
    uint32_t buffer_length;
    uint32_t elements_per_buffer;
    uint32_t  max_element_per_record;   //ALB maximum element per stream block
    uint32_t consumer_delay;
    uint32_t record_length;
    uint32_t header_length;

}mod_som_vecnav_config_t, *mod_som_vecnav_config_ptr_t;

/*******************************************************************************
 * @brief
 *     a settings structure VECNAV data
 * @description
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char data_header_text[8];
    char sn[4];
    uint32_t data_format;
    uint32_t elements_per_record; //i.e. stream/store frequency.
    uint32_t initialize_flag;
}mod_som_vecnav_settings_t, *mod_som_vecnav_settings_ptr_t;


/*******************************************************************************
 * @brief
 *     peripheral structure for MOD SOM VECNAV
 * @description
 *     this structure is based on the peripheral structure described in
 *     mod_som_common.h, mod_som_prf_t. The first 3 fields of the structure
 *     has to be the same as that of mod_som_prf_t.
 * @field handle_port
 *     a pointer to UART handle port (same as mod_som_prf_t)
 * @field irq_f
 *     pointer to interrupt callback function
 *     if UART port has two interrupt callback function, this would be the RX
 *     interrupt callback function
 * @field irq_extra_f
 *     pointer to additional interrupt callback function
 *     if UART port has two interrupt callback function, this would be the TX
 *     interrupt callback function
 * @field irqn
 *     interrupt number identification for the interrupt system
 ******************************************************************************/
typedef struct{
    void * handle_port;
    void (* irq_f)();
    void (* irq_extra_f)();
    IRQn_Type irqn;
}mod_som_vecnav_prf_t,*mod_som_vecnav_prf_ptr_t;



/*******************************************************************************
 * @brief
 *     run-time device structure for MOD SOM VECNAV
 * @description
 *     this structure is based on the peripheral structure described in
 *     mod_som_common.h, mod_som_prf_t. The first 3 fields of the structure
 *     has to be the same as that of mod_som_prf_t.
 * @field status
 *     status/error indicator of device via different calls
 * @field com_prf_ptr
 *     peripheral connection for the device communication and data stream
 * @field initialized_flag
 *     flag to indicate that this device is initialized via init function
 * @field connected_flag
 *     flag to indicate that this device is connected to its port
 * @field collect_data_flag
 *     flag to signal data collection
 * @field start_data_streaming_data_rec_flag
 *     flag to indicate start streaming full data records
 * @field done_data_streaming_data_rec_flag
 *     flag to indicate finish stream full data records
 * @field direct_connect_flag
 *     flag to indicate that we are directly connect to device for communication
 * @field config_ptr
 *     pointer to a copy of the configuration of device
 * @field data_header_length
 *    length of data header text
 * @field stat_header_length
 *    length of status header text
 * @field cmd_header_length
 *    length of command line header text
 * @field sample_data_length
 *    number of bytes per data sample
 * @field samples_per_record
 *    number of samples per records
 * @field cir_buff_length
 *    number of records in the circular buffer of device
 * @field cir_buff
 *     circular buffer for SBE 49 records
 * @field misc_ptr
 *    miscellaneous pointer where it all dependent on in implementation, where
 *    more data can be specified if need
 ******************************************************************************/
typedef struct{
    volatile mod_som_status_t status;
    mod_som_vecnav_prf_ptr_t com_prf_ptr;
    bool initialized_flag;
    bool connected_flag;
    uint8_t  consumer_mode;  //0:streaming, 1:SD_store, 2: on_board processing
    uint64_t timestamp;
    uint64_t sample_count;

    mod_som_vecnav_config_ptr_t        config_ptr;
    mod_som_vecnav_settings_ptr_t      settings_ptr;
    mod_som_vecnav_rec_buff_ptr_t      rec_buff_ptr;
    mod_som_vecnav_data_consumer_ptr_t consumer_ptr;
    mod_som_ldma_handle_t ldma;


    volatile bool data_ready_flag;
    volatile bool sampling_flag;
    volatile bool collect_data_flag;
    volatile uint32_t newline_flag;
    volatile uint32_t carriage_return_flag;

}mod_som_vecnav_t,*mod_som_vecnav_ptr_t;


//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
mod_som_status_t mod_som_vecnav_construct_settings_f();
mod_som_status_t mod_som_vecnav_construct_config_f();
mod_som_status_t mod_som_vecnav_construct_com_prf_f();
mod_som_status_t mod_som_vecnav_construct_rec_buff_f();
mod_som_status_t mod_som_vecnav_construct_consumer_ptr_f();

mod_som_vecnav_settings_t mod_som_vecnav_get_settings_f();
mod_som_vecnav_config_ptr_t mod_som_vecnav_get_config_f();

//mod_som_status_t mod_som_vecnav_check_memory_allocation_f(RTOS_ERR  err, void * struct_ptr);

/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_init_f();

/*******************************************************************************
 * @brief
 *   Initializes a connection to communication port indicated in configuration
 *   pointer
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_connect_f();
mod_som_status_t mod_som_vecnav_disconnect_f();

/*******************************************************************************
 * @brief
 *   Stream data to the main com port
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_start_collect_data_f();


void mod_som_vecnav_store_f();


mod_som_status_t mod_som_vecnav_stop_collect_data_f();
void mod_som_vecnav_id_f(CPU_INT16U argc,CPU_CHAR *argv[]);

static  void  mod_som_vecnav_consumer_task_f(void  *p_arg);
mod_som_status_t mod_som_vecnav_stop_consumer_task_f(void);
mod_som_status_t mod_som_vecnav_start_consumer_task_f(void);

mod_som_vecnav_sample_t mod_som_vecnav_parse_sample_f(uint8_t * element);

void mod_som_vecnav_ctd_variation_f(void);
void mod_som_vecnav_ctd_mean_f(void);

void mod_som_vecnav_header_f(mod_som_vecnav_data_consumer_ptr_t consumer_ptr);

mod_som_status_t mod_som_vecnav_init_ldma_f(mod_som_vecnav_ptr_t module_ptr);
void mod_som_vecnav_define_read_descriptor_f(mod_som_vecnav_ptr_t module_ptr);

mod_som_status_t mod_som_vecnav_consumer_mode_f(CPU_INT16U argc,
                                               CPU_CHAR *argv[]);

mod_som_status_t mod_som_vecnav_output_mode_f(CPU_INT16U argc,
                                               CPU_CHAR *argv[]);

mod_som_status_t mod_som_vecnav_gate_f(CPU_INT16U argc,
                                               CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   Send stuff out
 ******************************************************************************/
mod_som_status_t mod_som_vecnav_send_to_device_f(const uint8_t* data_to_send, uint32_t data_length);

#endif /* MOD_MOD_SOM_VECNAV_MOD_SOM_VECNAV_H_ */
