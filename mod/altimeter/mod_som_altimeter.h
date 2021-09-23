/*******************************************************************************
 * @file mod_som_altimeter.h
 * @brief MOD SOM altimeter API
 * @date January 4th, 2021
 *
 * @description
 * This API implementation file defines functions to be used with ALTIMETER
 * module running on the MOD SOM board.
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/

#ifndef MOD_SOM_ALTIMETER_H_
#define MOD_SOM_ALTIMETER_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include <em_timer.h>

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_ALTIMETER_STATUS_PREFIX 99U

#define MOD_SOM_ALTIMETER_STATUS_FAIL_INIT_CMD   0x2U
#define MOD_SOM_ALTIMETER_CANNOT_ALLOCATE_SETUP  0x3u
#define MOD_SOM_ALTIMETER_CANNOT_ALLOCATE_CONFIG 0x4u
#define MOD_SOM_ALTIMETER_STATUS_FAIL_TO_START_ALTIMETER_TASK 0x5u

#define MOD_SOM_ALTIMETER_MAX_HEADER_SIZE           100
//#define MOD_SOM_ALTIMETER_DEFAULT_BLANKING_INTERVAL 75000 //in 50 MHz tick so it is 65000/50e6. 65000 is the value used on the SPROUL.
#define MOD_SOM_ALTIMETER_DEFAULT_BLANKING_INTERVAL 133500 //in 50 MHz tick. //MHA we want RT travel time of 2 m (distance from alti to past epsi's nose).  0.75 m is 1 ms RT travel time so we want 2.67ms.

#define MOD_SOM_ALTIMETER_HEADER                    "ALTI"
#define MOD_SOM_ALTIMETER_SETTINGS_STR_lENGTH       8
#define MOD_SOM_ALTIMETER_DEFAULT_REV               "000"
#define MOD_SOM_ALTIMETER_DEFAULT_SN                "001"
#define MOD_SOM_ALTIMETER_DEFAULT_TX_PULSE          25000 // in 25000/50e6 (time base is the 50MHz Xcal)
#define MOD_SOM_ALTIMETER_DEFAULT_TX_MODE           1
#define MOD_SOM_ALTIMETER_DEFAULT_TX_PERIOD         15000000

#define MOD_SOM_ALTIMETER_SOUND_SPEED          1500
#define MOD_SOM_ALTIMETER_CHCKSUM_LENGTH          5

#define MOD_SOM_ALTIMETER_SYNC_HEADER_LENGTH     1
#define MOD_SOM_ALTIMETER_TAG_LENGTH             4
#define MOD_SOM_ALTIMETER_HEXTIMESTAMP_LENGTH    16
#define MOD_SOM_ALTIMETER_HEXPAYLOAD_LENGTH      8
#define MOD_SOM_ALTIMETER_HEADER_CHECKSUM_LENGTH 3
#define MOD_SOM_ALTIMETER_RECORD_ENDBYTES_LENGTH 2


#define MOD_SOM_ALTIMETER_TASK_PRIO             18u
#define MOD_SOM_ALTIMETER_TASK_STK_SIZE       2048u
//#define MOD_SOM_ALTIMETER_DELAY                 500      // delay for consumer 2 (ex: 4000 = 1 time / 4 secs).
#define MOD_SOM_ALTIMETER_DELAY                 200      // MHA try to decrease rep period


//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------


//structure compile
//structure settings
//structure runtime

/*******************************************************************************
 * @brief
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    mod_som_timer_handle_ptr_t tx_timer_ptr;
    mod_som_timer_handle_ptr_t echo_timer_ptr;
    uint32_t tx_pulse_start;
    uint32_t element_length;
    uint32_t record_length;
    uint32_t header_length;
}mod_som_altimeter_config_t, *mod_som_altimeter_config_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM ALTIMETER
 *   tx_repetition_mode:
 *   0: tx one shot altimeter (default)
 *   1: units in kernel ticks
 *   2: unit of epsilon sample period
 *   3: in mili-seconds.
 *
 *   tx_pulse_width;
 *
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char header[MOD_SOM_ALTIMETER_SETTINGS_STR_lENGTH];
    char rev[MOD_SOM_ALTIMETER_SETTINGS_STR_lENGTH];
    char sn[MOD_SOM_ALTIMETER_SETTINGS_STR_lENGTH];
    uint32_t tx_repetition_period; // see details above
    uint8_t  tx_repetition_mode;   // see details above
    uint32_t tx_pulse_width;       // micro-seconds
    uint32_t blanking_interval;    // micro-seconds
    uint32_t ping_interval;    // micrium ticks - MHA
    uint32_t initialize_flag;
}
mod_som_altimeter_settings_t, *mod_som_altimeter_settings_ptr_t;


/*******************************************************************************
 * @brief
 *   circular buffer to store data
 *
 * @field recs
 *   an array of altimeter elements
 *
 * @field read_indx
 *  consumer index
 *
 * @field write_indx
 *  producer index
 ******************************************************************************/
typedef struct{
    uint8_t * altimeter_elements_buffer; // a circular buffer to hold all elements
    uint32_t producer_indx;
    uint8_t header[MOD_SOM_ALTIMETER_MAX_HEADER_SIZE];
    uint8_t length_header;
    uint8_t chksum;
    uint32_t elmnts_skipped;
} mod_som_altimeter_rec_buff_t, *mod_som_altimeter_rec_buff_ptr_t;


/*******************************************************************************
 * @brief
 *   Structure managing the stream of ALTIMETER data
 ******************************************************************************/
typedef struct{
  uint32_t stream_data_length;            //ALB length of the streaming data buffer
  uint32_t max_sample_per_stream;         //ALB maximum element per stream block
  uint32_t run_stream_data_length;        //ALB ???
  uint8_t * stream_data_ptr;              //ALB pointer to stream data
  uint8_t block_stream_flag;              //ALB ???
}
mod_som_altimeter_stream_t, *mod_som_altimeter_stream_ptr_t;


/*******************************************************************************
 * @brief
 *   Structure managing the store of ALTIMETER data
 ******************************************************************************/
typedef struct{
  uint32_t store_data_length;            //ALB length of the streaming data buffer
  uint32_t run_store_data_length;        //ALB length of the streaming data buffer
  uint8_t * store_data_ptr;              //ALB pointer to stream data
  uint8_t block_store_flag;
}
mod_som_altimeter_store_t, *mod_som_altimeter_store_ptr_t;




/*******************************************************************************
 * @brief
 *   Run-time structure for MOD SOM ALTIMETER
 *   the altimeter runs a timer in a task.
 *   the repetition time
 *
 *
 ******************************************************************************/
typedef struct{
    mod_som_status_t status;
    uint32_t initialized_flag;
    uint32_t error_flag;
    uint64_t timestamp;
    uint32_t t0;
    uint32_t t_echo;
    uint8_t mode;
    char    msg[100];
    uint32_t length_msg;
    uint8_t chksum;
    enum {standby,not_ready,collecting,ready}  data_state;
    enum {pulse,blanking,echo,wait} altimeter_state;
    bool consumed_flag;
    mod_som_altimeter_settings_ptr_t settings_ptr;      //
    mod_som_altimeter_config_ptr_t config_ptr;    //

}mod_som_altimeter_t,*mod_som_altimeter_ptr_t;




//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize ALTIMETER, if shell is available, then the command table is added
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_init_f();

/*******************************************************************************
 * @brief
 * allocate memory for config_ptr
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_allocate_config_f();
mod_som_status_t mod_som_altimeter_config_f(mod_som_altimeter_config_ptr_t config_ptr);
mod_som_status_t mod_som_altimeter_allocate_settings_f();
mod_som_status_t mod_som_altimeter_default_settings_f(mod_som_altimeter_settings_ptr_t settings_ptr);
mod_som_altimeter_settings_t mod_som_altimeter_get_settings_f();
mod_som_altimeter_config_ptr_t mod_som_altimeter_get_config_f();

mod_som_status_t  mod_som_altimeter_start_task_f();
mod_som_status_t  mod_som_altimeter_stop_task_f();
static  void  mod_som_altimeter_task_f(void  *p_arg);
/*******************************************************************************
 * @brief
 * allocate memory for tx timer and echo timer
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_altimeter_allocate_timer_f();

/*******************************************************************************
 * @function
 *     mod_som__encode_status_f
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
mod_som_status_t mod_som_altimeter_encode_status_f(uint8_t mod_som_io_status);

/*******************************************************************************
 * @function
 *     mod_som_io_decode_status_f
 * @abstract
 *     Decode mod_som_status to status of MOD SOM I/O error codes
 * @discussion TODO SN
 *     The status is system wide, so we only decode the bit 16-23 if the
 *     higher bits show the status code is of MOD SOM ALTIMETER
 * @param       mod_som_status
 *     MOD SOM general system-wide status
 * @return
 *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
 *     header file
 ******************************************************************************/
uint8_t mod_som_altimeter_decode_status_f(mod_som_status_t mod_som_status);


mod_som_status_t mod_som_altimeter_init_timer_f();
mod_som_status_t mod_som_altimeter_disable_hardware_f();
mod_som_status_t mod_som_altimeter_enable_hardware_f();
mod_som_status_t  mod_som_altimeter_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]);
mod_som_status_t  mod_som_altimeter_repmode_f(CPU_INT16U argc,CPU_CHAR *argv[]);
mod_som_status_t  mod_som_altimeter_set_blank_f(CPU_INT16U argc,CPU_CHAR *argv[]);
mod_som_status_t  mod_som_altimeter_set_reprate_f(CPU_INT16U argc,CPU_CHAR *argv[]); //MHA add reprate command

void mod_som_altimeter_id_f(CPU_INT16U argc,CPU_CHAR *argv[]);


#endif /* MOD_SOM_ALTIMETER_H_ */
