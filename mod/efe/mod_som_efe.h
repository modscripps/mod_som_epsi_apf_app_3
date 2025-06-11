/*
 * mod_som_efe.h
 *
 *  Created on: Feb 13, 2020
 *      Author: aleboyer
 */

#ifndef EFE_MOD_SOM_EFE_H_
#define EFE_MOD_SOM_EFE_H_


//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <mod_som_common.h>

#include "mod_efe_ldma.h"
#include "mod_efe_config.h"
#include "mod_AD7124.h"
#include "em_timer.h"

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------
#define MOD_SOM_EFE_STATUS_PREFIX 99U

#define MOD_SOM_EFE_STATUS_FAIL_INIT_CMD 0x2U

#define MOD_SOM_EFE_MAX_CHANNEL        7
#define MOD_SOM_EFE_USER_NBCHANNEL     4
#define MOD_SOM_EFE_RECORD_PER_BUFFER  10
#define MOD_SOM_EFE_SAMPLE_PER_RECORD  80 //ALB minimum is 30
#define MOD_SOM_EFE_MAX_HEADER_SIZE    100

#define MOD_SOM_EFE_STATUS_OK                0
#define MOD_SOM_EFE_CANNOT_OPEN_COM_CHANNEL  1
#define MOD_SOM_EFE_CANNOT_OPEN_REC_BUFF     2
#define MOD_SOM_EFE_CANNOT_OPEN_CONFIG       3
#define MOD_SOM_EFE_CANNOT_ALLOCATE_SETUP    4
#define MOD_SOM_EFE_NOT_INITILIAZED          5


#if defined(MOD_SOM_EFE_REV4)
#define MOD_SOM_EFE_DEFAULT_EFE_REV_NAME     "rev4"
#define MOD_SOM_EFE_HEADER     "EFE4"
#endif
#if defined(MOD_SOM_EFE_REV3)
#define MOD_SOM_EFE_DEFAULT_EFE_REV_NAME     "rev3"
#define MOD_SOM_EFE_HEADER     "EFE3"
#endif


#define MOD_SOM_EFE_DEFAULT_EFE_SN           "001"



//ALB header definition
#define MOD_SOM_EFE_SETTINGS_STR_lENGTH        8
#define MOD_SOM_EFE_HEADER_LENGTH_HEXTIMESTAMP 16
#define MOD_SOM_EFE_SYNC_TAG_LENGTH            1
#define MOD_SOM_EFE_HEADER_TAG_LENGTH          4
#define MOD_SOM_EFE_HEADER_NB_COMAS            2
#define MOD_SOM_EFE_LENGTH_HEADER_CHECKSUM     3
#define MOD_SOM_EFE_LENGTH_CHECKSUM            5

// 64 bits 8 bytes timestamps from sleetimer
// TODO move the define in the MOD kernel
#define MOD_SOM_EFE_TIMESTAMP_LENGTH 8


// data consumer
#define MOD_SOM_EFE_CONSUMER_TASK_PRIO             18u
#define MOD_SOM_EFE_CONSUMER_TASK_STK_SIZE       4096u
#define MOD_SOM_EFE_CONSUMER_DELAY                   2      // delay for consumer 2 (ex: 4000 = 1 time / 4 secs).
#define MOD_SOM_EFE_CONSUMER_PADDING                 5      // number of elements for padding for data consumer.


//SN status codes
#define MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY     0x01u
#define MOD_SOM_EFE_STATUS_FAIL_TO_START_CONSUMER_TASK 0x02u
#define MOD_SOM_EFE_STATUS_FAIL_TOP_SAMPLING           0x03u


//------------------------------------------------------------------------------
// EFE global variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// TYPEDEFS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   circular buffer to store data
 *
 * @field recs
 *   an array of efe elements
 *
 * @field read_indx
 *  consumer index
 *
 * @field write_indx
 *  producer index
 ******************************************************************************/
typedef struct{
    uint8_t * efe_elements_buffer; // a circular buffer to hold all elements
    uint32_t producer_indx;
} mod_som_efe_rec_buff_t, *mod_som_efe_rec_buff_ptr_t;



/*******************************************************************************
 * @brief
 *   EFE spi communication handle
 *
 * @field usart
 *   USART0
 * @field usartclock
 *   cmuClock_USART0
 * @field miso
 *   port gpioPortC and pin 10
 * @field mosi
 *  port gpioPortC and Pin 11
 * @field baudrate
 *   spi baudrate 2000000
 * @field msbf
 *   endianness bool true
 * @field clockmode
 *   usartClockMode3
 * @field route_location
 *   TODO I do not know how to factor this in the code.
 *
 ******************************************************************************/
typedef struct{
	void *usart;
	mod_som_gpio_port_t miso;
	mod_som_gpio_port_t mosi;
	mod_som_gpio_port_t clck;
	uint32_t baudrate;
	bool msbf;
	uint32_t route_location;
} mod_som_efe_comm_handle_t, *mod_som_efe_comm_handle_ptr_t;

/*******************************************************************************
 * @brief
 *   EFE LDMA handle
 *
 * @field init
 *    LDMA_INIT_DEFAULT
 *    LDMA_Descriptors
 *    LDMA channel
 ******************************************************************************/
typedef struct{
	LDMA_Init_t init;
	int ch;
} mod_som_efe_ldma_handle_t, *mod_som_efe_ldma_handle_ptr_t;



/*******************************************************************************
 * @brief
 *   Structure to store set up config parameters for MOD SOM EFE
 *
 * @field is_initialized_flag
 *   flag to indicate that this structure is initialized properly
 *
 * @field communication_config_usart
 *   USART communication handle.
 *
 * @field communication_config_leuart
 *   LEUART driver instance initialization structure.
 *
 *   @field element_map
 *   element map contains the addresses of all elements contained in the element buffer.
 *   element map is used to fill the elements buffer during the LDMA transfer in the EFE interrupt
 *   element map is a pointer because the size varies with the nb of channel activated
 *
 *   @field element_per_buffer
 *   Nb of element per buffer
 *
 *   @field element_length
 *   length in bytes of one element (default timestamp + ADC sample x nb channels)
 *
 *   @field record_length
 *   length in bytes of one record (element length * nb element per record)
 *
 *   @field buffer_length
 *   length in bytes of the element buffer (record length * nb record per buffer)
 *
 *
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    mod_som_efe_comm_handle_t communication; // currently define USART0 spi port
    mod_som_timer_handle_ptr_t mclock;
    mod_som_timer_handle_ptr_t sync;
    uint32_t pin_interrupt_address;
    mod_som_gpio_port_t pin_interrupt;
    uint32_t gpio_a_base_address;
    uint32_t * element_map;      // ALB array of adresses to each element in the element buffer TODO move it to rec_buff
    uint32_t element_per_buffer; // ALB nb of element per buffer
    uint32_t element_length;     // ALB length of an elements in bytes
    uint32_t header_length;
    uint32_t record_length;      // ALB length of a record in bytes
    uint32_t buffer_length;      // ALB length of the circular buffer in bytes

}mod_som_efe_config_t, *mod_som_efe_config_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure to store set up configuration for MOD SOM EFE
 *
 *ALB Usually settings SHOULD be words or multiple.
 *ALB I have an issue here because sensor_spec is here
 *ALB and it contains AD7124 register structure
 *
 *
 * @field is_initialized_flag
 *   flag to indicate that this structure is initialized properly
 *
 * @field communication_config_usart
 *   USART communication handle.
 *
 * @field communication_config_leuart
 *   LEUART driver instance initialization structure.
 ******************************************************************************/
typedef struct{
    uint32_t size;
    char header[MOD_SOM_EFE_SETTINGS_STR_lENGTH];
    char rev[MOD_SOM_EFE_SETTINGS_STR_lENGTH];
    char sn[MOD_SOM_EFE_SETTINGS_STR_lENGTH];
    uint32_t number_of_channels;
    uint32_t nb_sample_per_record;
    uint32_t nb_record_per_buffer;
    uint32_t spi_baudrate;
    sensor_spec_t sensors[MOD_SOM_EFE_MAX_CHANNEL];
    uint32_t initialize_flag;
}
mod_som_efe_settings_t, *mod_som_efe_settings_ptr_t;

/*******************************************************************************
 * @brief
 *   Structure managing the stream of EFE data
 ******************************************************************************/
typedef struct{
  uint32_t  record_length;             //ALB length of the streaming data buffer
  uint32_t  max_sample_per_record;   //ALB maximum element per stream block
  uint32_t  payload_length;
  uint32_t  elmnts_skipped;
  uint64_t  cnsmr_cnt;

  uint8_t   data_ready_flg;           //ALB ???
  uint8_t   chksum;
  uint8_t   header[MOD_SOM_EFE_MAX_HEADER_SIZE];
  uint8_t   length_header;
  uint8_t   header_chksum;
  bool      consumed_flag;

  uint8_t * record_data_ptr;        //ALB pointer to stream data


}
mod_som_efe_data_consumer_t, *mod_som_efe_data_consumer_ptr_t;


/*******************************************************************************
 * @brief
 *   Run-time structure for MOD SOM EFE
 *
 * @field is_initialized_flag
 *   flag to indicate that this structure is initialized properly
 *
 * @field handle
 *   UART Driver handle
 *
 * @field rec_buff
 *   Circular buffer for SBE 49 data
 *
 * @field rx_buff
 *   Receive buffer to store data during each receive
 *
 * @field timestamp_of_last_rx
 *  Timestamp to last data request received
 *
 * @field done_streaming_flag
 *  Flag to indicate stop streaming
 ******************************************************************************/
typedef struct{
    uint32_t initialized_flag;
    uint32_t error_flag;
    uint64_t sample_count;
    uint64_t timestamp;
    mod_som_efe_settings_ptr_t settings_ptr;      //
    mod_som_efe_config_ptr_t config_ptr;    //
    mod_som_efe_data_consumer_ptr_t consumer_ptr;
    void * communication_handle;            // currently it is USART0
    mod_som_efe_rec_buff_ptr_t rec_buff;
    uint8_t consumer_mode;  //0:streaming, 1:SD_store, 2: on_board processing
    uint32_t sampling_flag;
    uint8_t data_ready_flag;
    mod_som_status_t status;
    uint32_t  ldma_spi_read_descriptor_offset; //MAG 11Aug2020 added  descriptor offset to LDMA read a/d spi data descriptor offset set in mod_som_efe_define_read_descriptor_f()
    uint32_t  ldma_channel_isr_mask;           // ALB to be used in the ldma handler to determine what channel trigger the interrupt signal.
    mod_som_efe_ldma_handle_t ldma;
    uint8_t sigramp_flag;
    uint32_t voltage;

}mod_som_efe_t,*mod_som_efe_ptr_t;




//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_f();

/*******************************************************************************
 * @brief
 *   enable EFE hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_initialize_hardware_f();
mod_som_status_t mod_som_efe_enable_hardware_f();

/*******************************************************************************
 * @brief
 *   disable EFE hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_disable_hardware_f();


/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_settings_ptr_f(void);
mod_som_status_t mod_som_efe_allocate_mclock_ptr_f(void);
mod_som_status_t mod_som_efe_construct_config_ptr_f(void);
mod_som_status_t mod_som_efe_allocate_communication_ptr_f(void);
mod_som_status_t mod_som_efe_allocate_record_ptr_f(void);
mod_som_status_t mod_som_efe_allocate_consumer_ptr_f(void);

mod_som_status_t mod_som_efe_config_f(mod_som_efe_config_ptr_t config_ptr);
/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/

mod_som_status_t mod_som_efe_default_settings_f(mod_som_efe_settings_ptr_t settings_ptr);
void mod_som_efe_set_settings_f(mod_som_efe_settings_ptr_t settings_ptr);
mod_som_efe_settings_t mod_som_efe_get_settings_f();
mod_som_efe_settings_ptr_t mod_som_efe_get_settings_ptr_f();
mod_som_efe_config_ptr_t mod_som_efe_get_config_f();

mod_som_efe_ptr_t mod_som_efe_get_runtime_ptr_f();

/*******************************************************************************
 * @brief
 *   Stream data to the main com port
 *
 * @param
 ******************************************************************************/
void mod_som_efe_stream_f(uint32_t stream_data_length);

/*******************************************************************************
 * @brief
 *   Store data on SD card
 *
 * @param
 ******************************************************************************/
void mod_som_efe_store_f();


/*******************************************************************************
 * @brief
 *   Initialize LDMA
 *
 * @param mod_som_efe_ldma_handle_ptr_t
 *   ldma configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_ldma_f();

/*******************************************************************************
 * @brief
 *   Initialize mclock for efe adcs
 *
 * @param mod_som_efe_mclock_handle_ptr_t
 *   timer mclock configuration pointer
 * @param mod_som_efe_sync_handle_ptr_t
 *   timer sync configuration pointer (to synchromnize the adcs)
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_mclock_f(mod_som_timer_handle_ptr_t config_mclock_ptr,mod_som_timer_handle_ptr_t config_sync_ptr);

/*******************************************************************************
 * @brief
 *   Initialize spi port for efe adc
 *
 * @param mod_som_efe_comm_handle_ptr_t
 *   usart configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_spi_f(mod_som_efe_comm_handle_ptr_t comm_handle);

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_start_mclock_f();

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_stop_mclock_f();
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_config_adc_f();
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_read_config_adc_f();
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_reset_adc_f();
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_read_adc_f();
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_define_read_descriptor_f(mod_som_efe_ptr_t module_ptr);
/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_adc_reset_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor);
/*******************************************************************************
 * @brief
 * generate a descriptor to config the adc config register
 * transfer using LDMA the config registers
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_adc_config_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor, uint8_t reg_address, uint32_t reg_data, uint8_t nb_bytes);

 /* *******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_adc_read_config_register_f(int ldma_ch, sensor_spec_ptr_t sensor, uint8_t reg_address, uint32_t sensor_reg_address, uint8_t nb_bytes);

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_alloc_rec_buffer_f();

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_header_f(mod_som_efe_data_consumer_ptr_t consumer_ptr);

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_id_f(CPU_INT16U argc,CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *
 *
 * @param
 *
 ******************************************************************************/
void mod_som_efe_probe_id_f(CPU_INT16U argc,CPU_CHAR *argv[]);

/*******************************************************************************
 * @function
 *     mod_som_efe_encode_status_f
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
mod_som_status_t mod_som_efe_encode_status_f(uint8_t mod_som_io_status);

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
uint8_t mod_som_efe_decode_status_f(mod_som_status_t mod_som_status);


// ALB command functions //SN edited to be functions called in shell //SN need to edit comments
/*******************************************************************************
 * @brief
 *   Shell command function for the 'efe_stream' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_consumer_mode_f(CPU_INT16U argc,CPU_CHAR *argv[]);

/*******************************************************************************
 * @brief
 *   Shell command function for the 'efe_sample' command.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_sampling_f();

/*******************************************************************************
 * @brief
 *   Shell command function for the efe_stop command.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_stop_sampling_f();


/*******************************************************************************
 * @brief
 *   create a fake ramp signal at sensor 0.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_sigramp_f();


static  void  mod_som_efe_consumer_task_f (void  *p_arg);

mod_som_status_t mod_som_efe_start_consumer_task_f (void);
mod_som_status_t mod_som_efe_stop_consumer_task_f(void);

void mod_som_efe_ldma_irq_handler_f( void );

#endif /* EFE_MOD_SOM_EFE_H_ */
