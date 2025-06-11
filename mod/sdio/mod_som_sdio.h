/*******************************************************************************
 * @file mod_som_sdio.h
 * @brief MOD SOM foo bar API
 * @date Mar 26, 2020
 * @author San Nguyen (stn004@ucsd.edu) - Multiscale Ocean Dynamics - SIO - UCSD
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

#ifndef MOD_SOM_SDIO_H_
#define MOD_SOM_SDIO_H_

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>
#include "ff.h"

#ifdef MOD_SOM_EFE_EN
#include "mod_som_efe.h"
#endif

#ifdef MOD_SOM_APF_EN
#include "mod_som_apf.h"
#endif


//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
//#define MOD_SOM_SDIO_STATUS_PREFIX 99U

#define MOD_SOM_SDIO_STATUS_OK              0x0U
#define MOD_SOM_SDIO_STATUS_FAIL_INIT_CMD   0x1U
#define MOD_SOM_SDIO_STATUS_FAIL_OPENFILE   0x2U
#define MOD_SOM_SDIO_STATUS_FAIL_CLOSEFILE  0x3U
#define MOD_SOM_SDIO_STATUS_FAIL_READFILE   0x4U
#define MOD_SOM_SDIO_STATUS_FAIL_WRITEFILE  0x5U
#define MOD_SOM_SDIO_STATUS_FAIL_READVOLUME 0x6U
#define MOD_SOM_SDIO_CANNOT_OPEN_CONF_FILE  0x7U
#define MOD_SOM_SDIO_CANNOT_OPEN_DATA_FILE  0x8U
#define MOD_SOM_SDIO_OPEN_PREV_PROCESS_FILE 0x9U
#define MOD_SOM_SDIO_NO_PROCESS_FILE        0x10U


/*******************************************************************************
 * @define MOD_SOM_SDIO_PRINT_TASK_PRIORITY
 *     priority value of MOD SOM I/O background task
 ******************************************************************************/
#ifndef MOD_SOM_SDIO_PRINT_TASK_PRIORITY
#define MOD_SOM_SDIO_PRINT_TASK_PRIORITY 16u
#endif

/*******************************************************************************
 * @define MOD_SOM_IO_TASK_STK_SIZE
 *     size of the tcb stack of the task
 ******************************************************************************/
#ifndef MOD_SOM_SDIO_TASK_STK_SIZE
//#define MOD_SOM_SDIO_TASK_STK_SIZE 128u
//ALB increase stack size to try to fix the hang up during sd write task
#define MOD_SOM_SDIO_TASK_STK_SIZE 256u
#endif

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_PREFIX
 *     16 bit identifying prefix for MOD SOM I/O status
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_PREFIX 0x02U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY
 *     error status value: fail to allocate dynamic memory pool for MOD SOM
 *     I/O usage
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_DYNAMIC_MEMORY 0x01U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY
 *     error status value: fail to allocate memory from dynamic memory pool
 *     for MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ALLOCATE_MEMORY 0x02U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_FREE_MEMORY
 *     error status value: fail to free memory from dynamic memory pool
 *     for a MOD SOM I/O data transfer item in queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_FREE_MEMORY 0x03U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_MSG_QUEUE 0x010U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_FLUSH_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_FLUSH_QUEUE 0x011U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_POST_MSG_TO_QUEUE 0x012U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ACCESS_MSG_FROM_QUEUE
 *     error status value: fail to post message to queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ACCESS_MSG_FROM_QUEUE 0x013U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_SEMAPHORE 0x020U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_PEND_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_PEND_SEMAPHORE 0x021U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_POST_SEMAPHORE
 *     error status value: fail to create message queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_POST_SEMAPHORE 0x022U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_TASK
 *     error status value: fail use OSCreateTask to initiate MOD SOM I/O
 *     background task to pipe data
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_CREATE_TASK 0x04U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT
 *     error status value: value cannot be set after initialization
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_VALUE_CANT_BE_SET_AFTER_INIT 0x05U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ENQUE
 *     fail to enqueue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_FAIL_TO_ENQUEUE 0x06U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_NOT_INITIALIZED 0x07U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_NOT_STARTED 0x08U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_ALREADY_STARTED
 *     not initialized
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_ALREADY_STARTED 0x09U

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_QUEUE_EMPTY
 *     empty queue error
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_QUEUE_EMPTY 0x0AU

/*******************************************************************************
 * @define MOD_SOM_SDIO_STATUS_ERR_QUEUE_ITEM_NOT_FOUND
 *     error: can't find item in queue
 ******************************************************************************/
#define MOD_SOM_SDIO_STATUS_ERR_QUEUE_ITEM_NOT_FOUND 0x0BU


/* local variable */
#define MAXBUFLEN 5000
#define MOD_SOM_SDIO_BLOCK_LENGTH 512
#define VOLUME_ADDRESS 0
//TODO rename this define. follow the MOD convention.
#define SDIO_OVF_MSG_LIST_THERSHOLD 75

//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

typedef struct{
    uint8_t initialized_flag;
    uint8_t is_open_flag;
    volatile bool is_writing_flag; //adding here to ensure we do not close file if it is writing
    volatile bool is_reading_flag; //adding here to ensure we do not close file if it is writing
    FIL *fp;
    char * file_name;
    uint8_t len_filename;
}mod_som_sdio_file_t, *mod_som_sdio_file_ptr_t;

typedef struct{
	uint32_t size;
	char  header[8];
	uint32_t file_duration;
	char prefix_file[40];
	uint32_t initialize_flag;
}mod_som_sdio_settings_t, *mod_som_sdio_settings_ptr_t;

typedef struct{

FATFS fat_fs;
char path[100];

char buff_data[MAXBUFLEN];
char read_buff[MOD_SOM_SDIO_BLOCK_LENGTH];

mod_som_sdio_file_ptr_t rawdata_file_ptr;
mod_som_sdio_file_ptr_t processdata_file_ptr;
//mod_som_sdio_file_ptr_t config_file_ptr;

//FIL fstrc_config;
//FIL fstrc_data;
//bool CNS2_FLAG = 0;

bool initialized_flag;
bool started_flag;
bool listoverflow_flag;
bool enable_flag;
bool fatfs_mounted;

uint32_t file_number;
mod_som_sdio_settings_ptr_t mod_som_sdio_settings_ptr;
sl_sleeptimer_timestamp_t open_file_time;
bool new_file_flag;
MEM_DYN_POOL dyn_mem_pool;
uint32_t max_size_of_complete_data_block;
CPU_STK print_task_stack[MOD_SOM_SDIO_TASK_STK_SIZE];
OS_TCB print_task_tcb;
OS_Q msg_queue;

}mod_som_sdio_t,*mod_som_sdio_ptr_t;



//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize FOO BAR, if shell is available, then the command table is added
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sdio_init_f();

/*******************************************************************************
 * @brief
 *   enable hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_enable_hardware_f();

/*******************************************************************************
 * @brief
 *   disable hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/

mod_som_status_t mod_som_sdio_disable_hardware_f();

mod_som_status_t mod_som_sdio_mount_fatfs_f();
mod_som_status_t mod_som_sdio_allocate_memory_f();
mod_som_status_t mod_som_sdio_default_settings_f();


/*******************************************************************************
 * @brief
 *    start SDIO task,
 *
 *  ALB: How do I kill a task?
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
//mod_som_status_t mod_som_sdio_start_task(mod_som_efe_device_ptr_t device_ptr);

/*******************************************************************************
 * @brief
 *   get settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_sdio_settings_t mod_som_sdio_get_settings_f();

/*******************************************************************************
 * @brief
 *   get sdio run time _ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_sdio_ptr_t mod_som_sdio_get_runtime_ptr_f();


/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_define_filename_f(CPU_CHAR* filename);
mod_som_status_t mod_som_sdio_open_processfilename_f(CPU_CHAR* filename);
mod_som_status_t mod_som_sdio_new_processfilename_f(CPU_CHAR* filename);
/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_opentoread_processfilename_f(CPU_CHAR* filename);

/*******************************************************************************
 * @brief
 *   Read the meta data from the on board processing file.
 *   - f_seek the read_pointer at the begining of the file
 *   - read the sizeof(metadata) and store the data
 *   - set back the read_pointer to the previous location (i.e., the end of the file)
 *
 *   - Note sizeof metadata < 512 so I can read the data in 1 chunk
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_OBPfile_metadata(
     mod_som_sdio_file_ptr_t processdata_file_ptr);

/*******************************************************************************
 * @brief
 *   a function to open a file with a name from given from the shell
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_open_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr);


/*******************************************************************************
 * @brief
 *   a function to close a file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_close_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr);


/*******************************************************************************
 * @brief
 *   a function to write a block of data generated in the LDMA interrupt handle from the efe
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
//void mod_som_sdio_write_data_block_f(mod_som_efe_device_ptr_t mod_som_efe_device_ptr);

/*******************************************************************************
 * @brief
 *   a function to write config file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_write_config_f(uint8_t *data_ptr,
                                                  uint32_t data_length,
                                                  mod_som_sdio_file_ptr_t file_ptr);
/*******************************************************************************
 * @brief
 *   a function to write config file
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_stop_store_f();


/*******************************************************************************
 * @brief
 *   a function read the config file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_config_sd_f(char * filename);

/*******************************************************************************
 * @brief
 *   a function read  the data file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_data_sd_f(char * filename,uint32_t number_of_files);



/*******************************************************************************
 * @brief
 *   a function read a file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_read_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr);
mod_som_status_t mod_som_sdio_read_data_file_f(mod_som_sdio_file_ptr_t mod_som_sdio_file_ptr);
mod_som_status_t mod_som_sdio_read_processfile_f(uint8_t * databuffer,
                                                 uint32_t obp_file_bytes,
                                                 uint32_t seek_idx);
/*******************************************************************************
 * @brief
 *   a function read a file and streaming it to the main comm port
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_ls_sd_f();

/*******************************************************************************
 * @brief
 *   a function to remove a file from the volume
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
mod_som_status_t mod_som_sdio_rm_sd_f(char* path);



///*******************************************************************************
// * @function
// *     mod_som_sdio_encode_status_f
// * @abstract
// *     Encode status of MOD SOM I/O error code to MOD SOM general status
// * @discussion TODO SN
// *     Encode status of MOD SOM IO status code to MOD SOM general status, where
// *     top 8 bits are module identifier, bit 16-23 store 8-bit status code,
// *     lower 16 bits are reserved for flags
// * @param       mod_som_io_status
// *     MOD SOM I/O error code
// * @return
// *     MOD SOM status code
// ******************************************************************************/
//mod_som_status_t mod_som_sdio_encode_status_f(uint8_t mod_som_io_status);
//
///*******************************************************************************
// * @function
// *     mod_som_io_decode_status_f
// * @abstract
// *     Decode mod_som_status to status of MOD SOM I/O error codes
// * @discussion TODO SN
// *     The status is system wide, so we only decode the bit 16-23 if the
// *     higher bits show the status code is of MOD SOM FOO BAR
// * @param       mod_som_status
// *     MOD SOM general system-wide status
// * @return
// *     0xff if non-MOD SOM I/O status, otherwise status, describes in general
// *     header file
// ******************************************************************************/
//uint8_t mod_som_sdio_decode_status_f(mod_som_status_t mod_som_status);

/*******************************************************************************
 * @function
 *     mod_som_sdio_set_max_size_of_complete_data_block_f
 * @abstract
 *     Set maximum size in bytes of the largest data block
 * @discussion
 *     Set maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O, this must be set before initialization
 * @return
 *     status would indicate error if set after initialization
 ******************************************************************************/
mod_som_status_t mod_som_sdio_set_max_size_of_complete_data_block_f(uint32_t max_size);

/*******************************************************************************
 * @function
 *     mod_som_sdio_get_max_size_of_complete_data_block_f
 * @abstract
 *     Get maximum size in bytes of the largest data block
 * @discussion
 *     Get maximum size in bytes of the largest data block that we would expect
 *     to send via MOD SOM I/O
 * @return
 *     size as a unsigned 32 bit number
 ******************************************************************************/
uint32_t mod_som_sdio_get_max_size_of_complete_data_block_f(void);

/*******************************************************************************
 * @function
 *     mod_som_sdio_init_f
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
mod_som_status_t mod_som_sdio_init_f(void);

/*******************************************************************************
 * @function
 *     mod_som_sdio_start_f
 * @abstract
 *     Start the background task for MOD SOM I/O
 * @discussion
 *
 * @return
 *     status would indicate error in initialization
 ******************************************************************************/
mod_som_status_t mod_som_sdio_start_f(void);

/*******************************************************************************
 * @function
 *     mod_som_sdio_print_f
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
mod_som_status_t mod_som_sdio_print_f(const char *format, ...) _ATTRIBUTE ((__format__ (__printf__, 1, 2)));
/*******************************************************************************
 * @brief
 *   stop print sdio task
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_sdio_stop_f();

/*******************************************************************************
 * @function
 *     mod_som_sdio_write_data_f
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
mod_som_status_t mod_som_sdio_write_data_f(
        mod_som_sdio_file_ptr_t file_ptr,
        const uint8_t *data_ptr, uint32_t data_length,
        volatile bool * done_streaming_flag_ptr);

/*******************************************************************************
 * @function
 *     mod_som_sdio_putchar_f
 * @abstract
 *     print a character to the streaming pipe
 * @discussion
 *     this function take only a single character
 * @param c
 *     character to output
 * @return
 *     status would indicate error in execution
 ******************************************************************************/
mod_som_status_t mod_som_sdio_putchar_f(char c);


///*******************************************************************************
// * @function
// *     mod_som_sdio_stream_data_with_header_separate_f
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
//mod_som_status_t mod_som_sdio_stream_data_with_header_separate_f(
//        const char *data_header, uint32_t data_header_length,
//        uint64_t data_timestamp,
//        const uint8_t *data_ptr, uint32_t data_length,
//        volatile bool * done_streaming_flag_ptr);
//
//mod_som_status_t mod_som_sdio_register_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr);
//mod_som_status_t mod_som_sdio_deregister_cir_buff_f(mod_som_cir_buff_ptr_t cir_buff_ptr);
#endif /* MOD_SOM_SDIO_H_ */
