/*
 * mod_som_efe.c
 *
 *  Created on: May 21, 2020
 *      Author: aleboyer
 */


#include "mod_som_efe.h"
#include "mod_som_efe_bsp.h"
#include "mod_som_io.h"
#include "mod_som_sdio.h"
#include "mod_som.h"
#include "mod_som_priv.h"
#include "em_msc.h"


#ifdef MOD_SOM_CALENDAR_EN
	#include "mod_som_calendar.h"
#endif
#ifdef MOD_SOM_SETTINGS_EN
	#include <mod_som_settings.h>
#endif
#ifdef MOD_SOM_AGGREGATOR_EN
  #include <mod_som_aggregator.h>
#endif



#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include "mod_som_efe_cmd.h"
#endif

//MHA
#include <mod_som_calendar.h> //MHA add calendar .h so we have access to the calendar settings pointer for the poweron_offset.

//MHA calendar
mod_som_calendar_settings_t mod_som_calendar_settings;


// ALB EFE GOLBAL VARIABLES. BAD PRACTICE I NEED SAN's MAGIC TO GET RID OF IT
// -----------------------------------------------------------------------------

//ALB EFE STATUS
sl_status_t mystatus;

// ALB mask to toggle port A pins directly through LDMA. As per peripheral tests on GG11
const uint32_t  bit_set_offset   = 0x06000000;
const uint32_t  bit_clear_offset = 0x04000000;

GPIO_Port_TypeDef selectort_port_enable=gpioPortA;
uint8_t selector_pin_A0=14;
uint8_t selector_pin_A1=13;
uint8_t selector_pin_A2=12;
uint8_t selector_pin_A3=7;
uint8_t selector_pin_OE=4;

static uint32_t  oe_bit_mask=0x10;
uint32_t  all_bit_mask=0x7080;

//LDMA
// ALB EFE always uses LDMA system, must have enough LDAM channels
// ALB make sure we choose a free LDMA channel. Look in the LDMA lib there should an easy way to do it.
// SN  use LDMA for the main com port

LDMA_TransferCfg_t spitx_init= LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXEMPTY);

//ALB Place holder for cs_assert. It will be change in config_adc to bring the selected CS low.
LDMA_Descriptor_t cs_assert   = LDMA_DESCRIPTOR_LINKREL_WRITE(0,0,0);
//ALB Place holder for cs_deassert. It will be change in config_adc to bring the selected CS high.
LDMA_Descriptor_t cs_deassert = LDMA_DESCRIPTOR_LINKREL_WRITE(0,0,0);
//ALB Place holder for adc_write xfer USART0 TX to ADC. It will be change in config_adc.
LDMA_Descriptor_t adc_write =LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(0,0,0,0);
//ALB Place holder for adc_read from USART0 RX to databuffer. It will be change in config adc.
LDMA_Descriptor_t adc_read = LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(0,0,0,0);
//ALB Place holder for single_cs_deassert. This should the last descriptor of the LDMA list.
//ALB It will be change in config_adc to bring the selected CS high.
LDMA_Descriptor_t single_cs_deassert = LDMA_DESCRIPTOR_SINGLE_WRITE(0,0);

//ALB Place holder for memory to memory transfer
LDMA_Descriptor_t timestamp_write = LDMA_DESCRIPTOR_LINKREL_M2M_BYTE(0,0,0,0);

LDMA_Descriptor_t descriptor_link_read1[MOD_SOM_EFE_MAX_CHANNEL*MOD_SOM_EFE_LDMA_READ_STEP+TRAILING_STEP];//MAG 7AUG2020  Added trailing step to calc to allow for more descriptors after the last channel
LDMA_Descriptor_t descriptor_link_readconfig[MOD_SOM_EFE_LDMA_READ_CONFIG_STEP];
LDMA_Descriptor_t descriptor_link_config[MOD_SOM_EFE_LDMA_CONFIG_STEP];


// Data consumer
static CPU_STK efe_consumer_task_stk[MOD_SOM_EFE_CONSUMER_TASK_STK_SIZE];
static OS_TCB  efe_consumer_task_tcb;



//ALB TODO rename sensor_spec_t
//ALB TODO rename to match the global naming convention
//ALB TODO change the name of the typedef efe_a2d_spec_t
//ALB I tried with mod_efe_adc_spec_t but for some reason it does not config
#ifdef MOD_SOM_EFE_REV3
sensor_spec_t ch1  = {"t1","999",99, MOD_AD7124_TEMP, 0x7080};
sensor_spec_t ch2  = {"t2","999",99, MOD_AD7124_TEMP, 0x3080};
sensor_spec_t ch3  = {"s1","999",99, MOD_AD7124_SHEAR, 0x5080};
sensor_spec_t ch4  = {"s2","999",99, MOD_AD7124_SHEAR, 0x1080};
sensor_spec_t ch5  = {"a1","000",0, MOD_AD7124_ACCELL, 0x6080};
sensor_spec_t ch6  = {"a2","000",0, MOD_AD7124_ACCELL, 0x2080};
sensor_spec_t ch7  = {"a3","000",0, MOD_AD7124_ACCELL, 0x4080};
#endif


//MHA Use this code with channels 3 and 4 set for EFE or FCTD to sample all 7 channels (normal operation).
//MHA This is a kluge.  ch3 and ch4 must be set to the correct pins for efe and FCTD

#ifdef MOD_SOM_EFE_REV4
//ALB I am putting default Sv, dTdV values
sensor_spec_t ch1  = {"t1","999",99, MOD_AD7124_TEMP, 0x7080};
sensor_spec_t ch2  = {"t2","999",99, MOD_AD7124_TEMP, 0x3080};
sensor_spec_t ch3  = {"s1","999",99, MOD_AD7124_SHEAR, 0x5080};//epsi
sensor_spec_t ch4  = {"s2","999",99, MOD_AD7124_SHEAR, 0x1080};//epsi
//sensor_spec_t ch3  = {"s1","000",0, MOD_AD7124_FLUO, 0x5080};//FCTD
//sensor_spec_t ch4  = {"s2","000",0, MOD_AD7124_UCOND, 0x1080};//FCTD
sensor_spec_t ch5  = {"a1","000",0, MOD_AD7124_ACCELLX, 0x6080};
sensor_spec_t ch6  = {"a2","000",0, MOD_AD7124_ACCELLY, 0x2080};
sensor_spec_t ch7  = {"a3","000",0, MOD_AD7124_ACCELLZ, 0x4080};
#endif

////MHA STANDALONE EFE CODE.  6/28/2021
//#ifdef MOD_SOM_EFE_REV4
//sensor_spec_t ch1  = {"t1","000",0, MOD_AD7124_TEMP, 0x7080};
//sensor_spec_t ch2  = {"t2","000",0, MOD_AD7124_TEMP, 0x3080};
//sensor_spec_t ch3  = {"s1","000",0, MOD_AD7124_FLUO, 0x5080};//s1 is FLUO for standalone
//sensor_spec_t ch4  = {"s2","000",0, MOD_AD7124_SHEAR, 0x1080};//s2 is a real shear probe for standalone
//sensor_spec_t ch5  = {"a1","000",0, MOD_AD7124_ACCELLX, 0x6080};
//sensor_spec_t ch6  = {"a2","000",0, MOD_AD7124_ACCELLY, 0x2080};
//sensor_spec_t ch7  = {"a3","000",0, MOD_AD7124_ACCELLZ, 0x4080};
//#endif


//MHA this section disables some of the ADC channels for standalone operation (t1, t2, fluor, accelly).
//MHA This is a kluge.  ch3 and ch4 must be set to the correct pins for efe and FCTD

//#ifdef MOD_SOM_EFE_REV4
//sensor_spec_t ch1  = {"t1","000",0, MOD_AD7124_TEMP, 0x7080};
//sensor_spec_t ch2  = {"t2","000",0, MOD_AD7124_TEMP, 0x3080};
//sensor_spec_t ch3  = {"s1","000",0, MOD_AD7124_FLUO, 0x5080};
//sensor_spec_t ch4  = {"s2","000",0, MOD_AD7124_SHEAR_DISABLED, 0x1080};
//sensor_spec_t ch5  = {"a1","000",0, MOD_AD7124_ACCELL_DISABLED, 0x6080};
//sensor_spec_t ch6  = {"a2","000",0, MOD_AD7124_ACCELLY, 0x2080};
//sensor_spec_t ch7  = {"a3","000",0, MOD_AD7124_ACCELL_DISABLED, 0x4080};
//#endif


#if defined(MOD_SOM_EFE_REV2)| defined(MOD_SOM_EFE_REV1)| defined(MOD_SOM_EFE_REV0)
sensor_spec_t ch1  = {"t1","000",0, MOD_AD7124_TEMP, {MOD_SOM_EFE_CS1_PORT, MOD_SOM_EFE_CS1_PIN}};  //PA4
sensor_spec_t ch2  = {"t2","000",0, MOD_AD7124_TEMP, {MOD_SOM_EFE_CS2_PORT, MOD_SOM_EFE_CS2_PIN}};  //PA5
sensor_spec_t ch3  = {"s1","000",0, MOD_AD7124_SHEAR, {MOD_SOM_EFE_CS3_PORT, MOD_SOM_EFE_CS3_PIN}};  //PA7
sensor_spec_t ch4  = {"s2","000",0, MOD_AD7124_SHEAR, {MOD_SOM_EFE_CS4_PORT, MOD_SOM_EFE_CS4_PIN}};  //PA12
sensor_spec_t ch5  = {"a1","000",0, MOD_AD7124_ACCELL, {MOD_SOM_EFE_CS5_PORT, MOD_SOM_EFE_CS5_PIN}};  //PA13
sensor_spec_t ch6  = {"a2","000",0, MOD_AD7124_ACCELL, {MOD_SOM_EFE_CS6_PORT, MOD_SOM_EFE_CS6_PIN}};  //PA14
sensor_spec_t ch7  = {"a3","000",0, MOD_AD7124_ACCELL, {MOD_SOM_EFE_CS7_PORT, MOD_SOM_EFE_CS7_PIN}};  //PA15
sensor_spec_t dummy_ch  = {"dum","000",0, MOD_AD7124_DEFAULT, {gpioPortA, MOD_SOM_EFE_CS7_PIN}};  //PA15
#endif

sensor_spec_t dummy_ch  = {"dum","000",0, MOD_AD7124_DEFAULT, 0x4080};  //PA15
sensor_spec_ptr_t dummy_ch_ptr  = {&dummy_ch};  //PA15

//ALB place holder for dummy of the adc. for timing purpose during the ldma transfer
uint8_t dummyread[16]={0};   //13 bytes
//ALB place holder for ad7124 read command
uint8_t readCmd[17] ={0};
uint8_t dummybytes[1]={0};   //13 bytes
uint8_t config_cmd[4]={0};
uint8_t read_cmd[4]={0};

volatile uint8_t read_adc_cmd[4];

//ALB make fake data to check circular buffer
volatile uint8_t fake_adc_data[3];
uint8_t read_dummy[3];



static mod_som_efe_ptr_t mod_som_efe_ptr;

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
uint8_t mod_som_efe_decode_status_f(mod_som_status_t mod_som_status){
	if(mod_som_status==MOD_SOM_STATUS_OK)
		return MOD_SOM_STATUS_OK;
	uint8_t status_prefix;
	uint8_t decoded_status;
	MOD_SOM_DECODE_STATUS(mod_som_status,&status_prefix,&decoded_status);
	if(status_prefix != MOD_SOM_EFE_STATUS_PREFIX){
		return 0xffU;
	}
	return decoded_status;
}

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
mod_som_status_t mod_som_efe_encode_status_f(uint8_t mod_som_io_status){
	if(mod_som_io_status==MOD_SOM_STATUS_OK)
		return MOD_SOM_STATUS_OK;
	return MOD_SOM_ENCODE_STATUS(MOD_SOM_EFE_STATUS_PREFIX, mod_som_io_status);
}

//ALB EFE specific functions

/*******************************************************************************
 * @brief
 *   Initialize EFE, if shell is available, then the command table is added
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_f(){

	mod_som_status_t status=MOD_SOM_STATUS_OK;
	RTOS_ERR         err;

	//ALB initialize EFE shell command

	//ALB TODO In the main replace mod_som_efe_init_f() by mod_som_efe_init_shellcmd_f()
	//ALB TODO I want to simply initialize the CMDs and let the user or the default process initalizing the module

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
	status = mod_som_efe_init_shellcmd_f();
	//ALB checking if the shell comds are initalized. return a error if shell cmd initialization failed.
	if(status != MOD_SOM_STATUS_OK){
	    //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_INIT_CMD);
	}
#endif


	// ALB allocate memory for the module_ptr.
	// ALB The module_ptr is also the "scope" of the runtime_ptr
	// ALB but the module_ptr also contains the settings_ptr and the config_ptr
	// ALB The settings_ptr an config_ptr should allocated and defined during the module initialization
	mod_som_efe_ptr = (mod_som_efe_ptr_t)Mem_SegAlloc(
			"MOD SOM EFE RUNTIME Memory",DEF_NULL,
			sizeof(mod_som_efe_t),
			&err);

	//SN Check error code
	//ALB WARNING: The following line hangs in a while loop -
	//ALB WARNING: - if the previous allocation fails.
	//ALB TODO change return -1 to return the an appropriate error code.
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr==DEF_NULL){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return -1;
	}

	//ALB Initialize the runtime flag module_ptr->initialized_flag to false.
	//ALB It will be set to true once the module is initialized at the end of mod_som_efe_init_f().
	mod_som_efe_ptr->initialized_flag = false;

	//2025 06 14 adding this for monitoring the task
	mod_som_efe_ptr->efe_consumer_task_stk_ptr = efe_consumer_task_stk;
	mod_som_efe_ptr->efe_consumer_task_tcb_ptr = &efe_consumer_task_tcb;

	// ALB allocate memory for the settings_ptr.
	// ALB WARNING: The setup pointer CAN NOT have pointers inside.
	status |= mod_som_efe_allocate_settings_ptr_f();
	if (status!=MOD_SOM_STATUS_OK){
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}


	// ALB checking if a previous EFE setup exist, from the setup module (i.e. setup file or UserData setup)
#ifdef MOD_SOM_SETTINGS_EN
    mod_som_settings_struct_ptr_t local_settings_ptr=mod_som_settings_get_settings_f();
    mod_som_efe_ptr->settings_ptr=&local_settings_ptr->mod_som_efe_settings;
#else
    mod_som_efe_ptr->settings_ptr->initialize_flag=false;
#endif

	// ALB If no pre-existing settings, use the default settings
    if (!mod_som_efe_ptr->settings_ptr->initialize_flag){
    	// initialize the setup structure.
    	status |= mod_som_efe_default_settings_f(mod_som_efe_ptr->settings_ptr);
    	if (status!=MOD_SOM_STATUS_OK){
          //ALB change the printf to a report status function.
    	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
    		return status;
    	}
    }

    // ALB Allocate memory for the config pointer, using the settings_ptr variable
    status |= mod_som_efe_construct_config_ptr_f();
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}

    // ALB Allocate memory for the com pointer,
    status |= mod_som_efe_allocate_communication_ptr_f();
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}

	// ALB Allocate memory for the data buffer pointer.
	// ALB It includes the circular buffer and read write indexes
    status |= mod_som_efe_allocate_record_ptr_f();
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}

	// ALB Allocate memory for the consumer_ptr,
	// ALB contains the consumer stream data_ptr and stream_data length.
	// ALB This pointer is also used to store the data on the SD card
    status |= mod_som_efe_allocate_consumer_ptr_f();
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}

	//turn on EFE hardware
	status |= mod_som_efe_enable_hardware_f();
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}
	//ALB initialize EFE SPI port
	status |= mod_som_efe_init_spi_f(&(mod_som_efe_ptr->config_ptr->communication));
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}
	//ALB initialize EFE timers
	status |= mod_som_efe_init_mclock_f(mod_som_efe_ptr->config_ptr->mclock,mod_som_efe_ptr->config_ptr->sync);
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}
	//ALB initialize EFE LDMA sampling
	status |= mod_som_efe_init_ldma_f(mod_som_efe_ptr);
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
	    printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}
	//ALB configure the adc
	status |= mod_som_efe_config_adc_f(mod_som_efe_ptr);
	if (status!=MOD_SOM_STATUS_OK){
      //ALB change the printf to a report status function.
		printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
		return status;
	}


	//ALB initialize the runtime flags
	mod_som_efe_ptr->sample_count=0;
	mod_som_efe_ptr->consumer_mode=2; //3=do nothing
	mod_som_efe_ptr->sampling_flag=0;
	mod_som_efe_ptr->data_ready_flag=0;
  mod_som_efe_ptr->error_flag=0;
  mod_som_efe_ptr->voltage=0;
	mod_som_efe_ptr->sigramp_flag=0;

  //turn on EFE hardware
  status |= mod_som_efe_disable_hardware_f();
  if (status!=MOD_SOM_STATUS_OK){
      printf("%s not initialized\n",MOD_SOM_EFE_HEADER);
    return status;
  }


	mod_som_efe_ptr->initialized_flag = true;
	printf("%s initialized\r\n",MOD_SOM_EFE_HEADER);//MHA

	//MHA
//	// U16
//	//                              (U1_40)
//	#define MOD_SOM_U16_3_PORT      gpioPortB
//	#define MOD_SOM_U16_3_PIN       12

#ifdef STANDALONE_FLUOROMETER
	GPIO_PinModeSet(gpioPortB, 12,
	                  gpioModePushPull, 1);
#endif

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct settings_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_settings_ptr_f(){

	RTOS_ERR  err;

	//ALB alloc memory for setup pointer
	//set up default configuration
	mod_som_efe_ptr->settings_ptr =
			(mod_som_efe_settings_ptr_t)Mem_SegAlloc(
					"MOD SOM EFE setup.",DEF_NULL,
					sizeof(mod_som_efe_settings_t),
					&err);

	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr->settings_ptr==NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return MOD_SOM_EFE_CANNOT_ALLOCATE_SETUP;
	}

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct config_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_construct_config_ptr_f(){

	RTOS_ERR  err;
	//ALB Start allocating  memory for config pointer
	mod_som_efe_ptr->config_ptr =
			(mod_som_efe_config_ptr_t)Mem_SegAlloc(
					"MOD SOM EFE config.",DEF_NULL,
					sizeof(mod_som_efe_config_t),
					&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr->config_ptr==NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return MOD_SOM_EFE_CANNOT_OPEN_CONFIG;
	}

	//ALB Allocate memory for pointers of the master-clock and sync-timer.
	//ALB N.B: These pointers are inside the config structure.
	mod_som_efe_allocate_mclock_ptr_f();


	//ALB initialize config structure
	mod_som_efe_config_ptr_t config_ptr=mod_som_efe_ptr->config_ptr;
  config_ptr->initialized_flag = false;

  config_ptr->communication.usart = (void *)MOD_SOM_EFE_SPI_COM;

  config_ptr->communication.miso.port = MOD_SOM_EFE_MISO_PORT;
  config_ptr->communication.miso.pin  = MOD_SOM_EFE_MISO_PIN;

  config_ptr->communication.mosi.port = MOD_SOM_EFE_MOSI_PORT;
  config_ptr->communication.mosi.pin  = MOD_SOM_EFE_MOSI_PIN;

  config_ptr->communication.clck.port = MOD_SOM_EFE_CLCK_PORT;
  config_ptr->communication.clck.pin  = MOD_SOM_EFE_CLCK_PIN;

  config_ptr->communication.baudrate = mod_som_efe_ptr->settings_ptr->spi_baudrate;
  config_ptr->communication.msbf = true;

  config_ptr->mclock->timer_pin.port = MOD_SOM_EFE_MCLOCK_PORT;
  config_ptr->mclock->timer_pin.pin  = MOD_SOM_EFE_MCLOCK_PIN;
  config_ptr->mclock->timer = MOD_SOM_EFE_MCLOCK_TIMER;
  config_ptr->mclock->top=MOD_SOM_EFE_MCLOCK_CC_VALUE;
  config_ptr->mclock->compare_value=MOD_SOM_EFE_MCLOCK_PHASE_SHIFT;

  config_ptr->sync->timer_pin.port = MOD_SOM_EFE_SYNC_PORT;
  config_ptr->sync->timer_pin.pin = MOD_SOM_EFE_SYNC_PIN;
  config_ptr->sync->timer = MOD_SOM_EFE_SYNC_TIMER;
  config_ptr->sync->compare_value=MOD_SOM_EFE_SYNC_PHASE_SHIFT;
  config_ptr->sync->top=0; //ALB Not used but initialized anyway.

  config_ptr->pin_interrupt.port=MOD_SOM_EFE_ADC_INTERUPT_PORT;
  config_ptr->pin_interrupt.pin=MOD_SOM_EFE_ADC_INTERUPT_PIN;
  config_ptr->pin_interrupt_address=MOD_SOM_EFE_ADC_INTERUPT_ADDRESS;

  config_ptr->gpio_a_base_address= (uint32_t) &GPIO->P[gpioPortA].DOUT;

  config_ptr->element_per_buffer=
       mod_som_efe_ptr->settings_ptr->nb_sample_per_record* \
       mod_som_efe_ptr->settings_ptr->nb_record_per_buffer;
  //ALB sample length = timestamp+ nb channel x ADC sample.

  config_ptr->element_length=MOD_SOM_EFE_TIMESTAMP_LENGTH+ \
                            mod_som_efe_ptr->settings_ptr->number_of_channels* \
                            AD7124_SAMPLE_LENGTH;

  config_ptr->header_length=
      MOD_SOM_EFE_SYNC_TAG_LENGTH+
      MOD_SOM_EFE_HEADER_TAG_LENGTH+
      MOD_SOM_EFE_HEADER_LENGTH_HEXTIMESTAMP +
      MOD_SOM_EFE_SETTINGS_STR_lENGTH+
      MOD_SOM_EFE_LENGTH_HEADER_CHECKSUM;



  //ALB record length = sample length x nb sample per record
  config_ptr->record_length=
      config_ptr->header_length+
      config_ptr->element_length*
      mod_som_efe_ptr->settings_ptr->nb_sample_per_record+
      MOD_SOM_EFE_LENGTH_CHECKSUM;

  //ALB buffer length = record length x nb record per buffer
  config_ptr->buffer_length=config_ptr->record_length*\
                            mod_som_efe_ptr->settings_ptr->nb_record_per_buffer;

  // allocate memory for the element map
  // element map contains the addresses of all elements contained in the element buffer.
  // element map is used to fill the elements buffer during the LDMA transfer in the EFE interrupt
  config_ptr->element_map = (uint32_t *)Mem_SegAlloc(
      "MOD SOM EFE element map ptr",DEF_NULL,
      sizeof(uint32_t)* config_ptr->element_per_buffer,
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(config_ptr->element_map ==DEF_NULL)
  {
    mod_som_efe_ptr = DEF_NULL;
    return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

  config_ptr->initialized_flag = true;



	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   allocate memory for the master clock and sync timer handles
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_mclock_ptr_f(){

  RTOS_ERR  err;
  //ALB Start allocating  memory for mclock timer handle pointer
  mod_som_efe_ptr->config_ptr->mclock =
      (mod_som_timer_handle_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE mclock.",DEF_NULL,
          sizeof(mod_som_timer_handle_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_efe_ptr->config_ptr->mclock==NULL)
  {
    mod_som_efe_ptr = DEF_NULL;
    return MOD_SOM_EFE_CANNOT_OPEN_CONFIG;
  }

  //ALB Start allocating  memory for sync timer handle pointer
  mod_som_efe_ptr->config_ptr->sync =
      (mod_som_timer_handle_ptr_t)Mem_SegAlloc(
          "MOD SOM EFE sync.",DEF_NULL,
          sizeof(mod_som_timer_handle_t),
          &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(mod_som_efe_ptr->config_ptr->sync==NULL)
  {
    mod_som_efe_ptr = DEF_NULL;
    return MOD_SOM_EFE_CANNOT_OPEN_CONFIG;
  }


  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   construct communication_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_communication_ptr_f(){
	RTOS_ERR  err;

	//ALB alloc memory for comm handle
	mod_som_efe_ptr->communication_handle =
			(void *)Mem_SegAlloc(
					"MOD SOM EFE com. handle",DEF_NULL,
					sizeof(mod_som_efe_comm_handle_t),
					&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr->communication_handle==NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return MOD_SOM_EFE_CANNOT_OPEN_COM_CHANNEL;
	}
	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct rec_buff_ptr
 *   rec->efe_elements_buffer->timestamp
 *   rec->efe_elements_buffer->efe sample_ptr
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_record_ptr_f(){
	RTOS_ERR  err;


	//ALB alloc memory for record buffer
	mod_som_efe_ptr->rec_buff =
			(mod_som_efe_rec_buff_ptr_t)Mem_SegAlloc(
					"MOD SOM EFE rec buff",DEF_NULL,
					sizeof(mod_som_efe_rec_buff_t),
					&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr->rec_buff==NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return MOD_SOM_EFE_CANNOT_OPEN_REC_BUFF;
	}

	// efe_elements_buffer is like the base_ptr in cbTest
	mod_som_efe_ptr->rec_buff->efe_elements_buffer =
			(uint8_t *) Mem_SegAlloc(
					"MOD SOM EFE elements buff",DEF_NULL,
					mod_som_efe_ptr->config_ptr->buffer_length,
					&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(mod_som_efe_ptr->rec_buff->efe_elements_buffer==NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return MOD_SOM_EFE_CANNOT_OPEN_REC_BUFF;
	}

	//ALB initialize element buffer to 0
	for (int i=0; i< mod_som_efe_ptr->config_ptr->buffer_length; i++){
	    mod_som_efe_ptr->rec_buff->efe_elements_buffer[i]=0;
	}


	// Now that the memory for the circular buffer is allocated and initialized
	// fill the element map with the addresses of each elements.
  for (int i=0; i< mod_som_efe_ptr->config_ptr->element_per_buffer; i++){
      mod_som_efe_ptr->config_ptr->element_map[i] =                            \
            (uint32_t) &mod_som_efe_ptr->rec_buff->efe_elements_buffer[                   \
                             i*mod_som_efe_ptr->config_ptr->element_length];
  }


	mod_som_efe_ptr->rec_buff->producer_indx=0;


	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   construct stream structure
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_allocate_consumer_ptr_f(){

	RTOS_ERR  err;


  // allocate memory for streaming_ptr
  mod_som_efe_ptr->consumer_ptr = (mod_som_efe_data_consumer_ptr_t)Mem_SegAlloc(
      "MOD SOM EFE consumer ptr",DEF_NULL,
      sizeof(mod_som_efe_data_consumer_t),
      &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  if(mod_som_efe_ptr->consumer_ptr==DEF_NULL)
  {
    mod_som_efe_ptr = DEF_NULL;
    return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
  }

	// streaming data buffer the length is computed in the stream task.
  mod_som_efe_ptr->consumer_ptr->record_length = 0;


	mod_som_efe_ptr->consumer_ptr->max_sample_per_record=mod_som_efe_ptr->settings_ptr->nb_sample_per_record;


	//ALB Set efe_header length: EFEtimestamp,record length,element skipped,flags
	//ALB 1char($) + 16 char (hex timestamp)+ 8 char (hex payload )+
	//ALB 1 char (*) + 2 char (hex chksum)= 32 bytes

	  mod_som_efe_ptr->consumer_ptr->length_header=
	                                           MOD_SOM_EFE_SYNC_TAG_LENGTH+
	                                           MOD_SOM_EFE_HEADER_TAG_LENGTH+
	                                           MOD_SOM_EFE_HEADER_LENGTH_HEXTIMESTAMP +
	                                           MOD_SOM_EFE_SETTINGS_STR_lENGTH+
	                                           MOD_SOM_EFE_LENGTH_HEADER_CHECKSUM;


	// allocate max memory for streaming_data_ptr.
	// ALB Right now I allocate the same space as the circular buffer.
	// ALB TODO adjust the size to a record size? this size could/should
	// ALB      be computed in relation with MOD_SOM_STREAM_DELAY
	mod_som_efe_ptr->consumer_ptr->record_data_ptr = (uint8_t *)Mem_SegAlloc(
			"MOD SOM EFE consumer data ptr",DEF_NULL,
			sizeof(uint8_t)*mod_som_efe_ptr->config_ptr->buffer_length,
			&err);
	// Check error code
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
		return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
	if(mod_som_efe_ptr->consumer_ptr->record_data_ptr==DEF_NULL)
	{
		mod_som_efe_ptr = DEF_NULL;
		return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_ALLOCATE_MEMORY));
	}


	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief  mod_som_efe_consumer_start_task_f
 *   start the consumer task (i.e. efe stream consumer or SD store)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

mod_som_status_t  mod_som_efe_start_consumer_task_f(){

  RTOS_ERR err;
  // Consumer Task 2
   mod_som_efe_ptr->consumer_ptr->consumed_flag=true;
   mod_som_efe_ptr->consumer_ptr->cnsmr_cnt=0;

   OSTaskCreate(&efe_consumer_task_tcb,
                        "efe consumer task",
                        mod_som_efe_consumer_task_f,
                        DEF_NULL,
                        MOD_SOM_EFE_CONSUMER_TASK_PRIO,
            &efe_consumer_task_stk[0],
            (MOD_SOM_EFE_CONSUMER_TASK_STK_SIZE / 10u),
            MOD_SOM_EFE_CONSUMER_TASK_STK_SIZE,
            0u,
            0u,
            DEF_NULL,
            (OS_OPT_TASK_STK_CLR),
            &err);

  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
    return (mod_som_efe_ptr->status = mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TO_START_CONSUMER_TASK));
  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/*******************************************************************************
 * @brief
 *   mod_som_efe_stream_stop_task_f
 *   stop the stream task (i.e. efe stream consumer)
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 *   Mike's note:   MOD_SOM_STATUS_OK if deletion is successful
 ******************************************************************************/

mod_som_status_t  mod_som_efe_stop_consumer_task_f(){

  mod_som_status_t status = MOD_SOM_STATUS_OK;
  RTOS_ERR err;

  if(efe_consumer_task_tcb.TaskState != OS_TASK_STATE_DEL){
      // delete the task
      OSTaskDel(&efe_consumer_task_tcb,
                &err);

      if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
        status = 1;
  }
  return status;
}



/*******************************************************************************
 * @brief
 *   mod_som_efe_consumer_task_f
 *   conusmer task (i.e. efe stream consumer or SD store consumer)
 *
 *   This task is a while loop, thus, it will run continuously once the task started
 *   cnsmr_cnt increases indefinitely until the task is stopped
 *
 *   - Every MOD_SOM_EFE_CONSUMER_DELAY the task will gather the EFE data stored
 *     in the circular buffer one element at a time.
 *   - Once there no more data available, compute the checksum and append it to the block (*FF\r\n)
 *   - The task gather the current timestamp,
 *   - Build a block header (EFEtimestamp,recordsize,elementskipped,voltage,errorflag)
 *   - Prefix the header to the block
 *   - Add the header for this block and send a message
 *     (header+block) to the IO stream task or SDIO stream task.
 *
 *   1 element    = 1 EFE sample = 3bytes x nb channels
 *   sample_count = producer count
 *   cnsmr_cnt    =
 *
 *   ALB TODO convert this consumer task in a state machine (collect/consume/clear?)
 *   ALB TODO copy the aggregator consumer state machine.
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/

void  mod_som_efe_consumer_task_f(void  *p_arg){
    RTOS_ERR  err;
    int error_cnt = 0;

    //cb_elmnt_ptr curr_read_elmnt_ptr = test_cb_param_block.base_ptr;
    // get local efe element ptr and local efe streamer ptr.
    uint8_t * curr_data_ptr   = mod_som_efe_ptr->rec_buff->efe_elements_buffer;
    uint8_t * curr_consumer_element_ptr = mod_som_efe_ptr->consumer_ptr->record_data_ptr+\
                                mod_som_efe_ptr->consumer_ptr->length_header;
    uint8_t * base_consumer_element_ptr = mod_som_efe_ptr->consumer_ptr->record_data_ptr+\
                                mod_som_efe_ptr->consumer_ptr->length_header;

//    uint8_t fake_data_ptr[29]={"ABCDEFGHIJKLMNOPQRSTUVWXYZ123"};
//    int32_t cnsmr_cnt = 0;
    uint64_t tick;


    int elmnts_avail=0, reset_cnsmr_cnt=0;
    int data_elmnts_offset=0;


    int padding = MOD_SOM_EFE_CONSUMER_PADDING; // the padding should include the variance.

    mod_som_sdio_ptr_t local_mod_som_sdio_ptr_t=
        mod_som_sdio_get_runtime_ptr_f();

    mod_som_sdio_file_ptr_t rawfile_ptr =
        local_mod_som_sdio_ptr_t->rawdata_file_ptr;

    //        printf("In Consumer Task 2\n");
    while (DEF_ON) {

        if (mod_som_efe_ptr->sampling_flag){
            elmnts_avail = mod_som_efe_ptr->sample_count -
                           mod_som_efe_ptr->consumer_ptr->cnsmr_cnt;  //calculate number of elements available have been produced
            // LOOP without delay until caught up to latest produced element
            while (elmnts_avail > 0)
              {
                // When have circular buffer overflow: have produced data bigger than consumer data: 1 circular buffer (n_elmnts)
                // calculate new consumer count to skip ahead to the tail of the circular buffer (with optional padding),
                // calculate the number of data we skipped, report number of elements skipped.
                // Reset the consumers cnt equal with producer data plus padding
                if (elmnts_avail>(mod_som_efe_ptr->config_ptr->element_per_buffer)){ // checking over flow. TODO check adding padding is correct.
                    // reset the consumer count less one buffer than producer count plus padding
                    //ALB I think I want to change this line from the cb example. The "-" only works if you overflowed once.
                    reset_cnsmr_cnt = mod_som_efe_ptr->sample_count -
                                      mod_som_efe_ptr->config_ptr->element_per_buffer +
                                      padding;
                    // calculate the number of skipped elements
                    mod_som_efe_ptr->consumer_ptr->elmnts_skipped = reset_cnsmr_cnt -
                                           mod_som_efe_ptr->consumer_ptr->cnsmr_cnt;

                    mod_som_io_print_f("\n efe stream task: CB overflow: sample count = %lu,"
                                       "cnsmr_cnt = %lu,skipped %lu elements \r\n ", \
                           (uint32_t)mod_som_efe_ptr->sample_count, \
                           (uint32_t)mod_som_efe_ptr->consumer_ptr->cnsmr_cnt, \
                           mod_som_efe_ptr->consumer_ptr->elmnts_skipped);

                    mod_som_efe_ptr->consumer_ptr->cnsmr_cnt = reset_cnsmr_cnt;
                }

                // calculate the offset for current pointer
                data_elmnts_offset = mod_som_efe_ptr->consumer_ptr->cnsmr_cnt %
                                     mod_som_efe_ptr->config_ptr->element_per_buffer;

                // update the current element pointer using the element map
                curr_data_ptr   =
                   (uint8_t*)
                   mod_som_efe_ptr->config_ptr->element_map[data_elmnts_offset];

                //ALB move the stream ptr to the next element
                curr_consumer_element_ptr = base_consumer_element_ptr + \
                    (mod_som_efe_ptr->consumer_ptr->cnsmr_cnt-
                     reset_cnsmr_cnt)*\
                     mod_som_efe_ptr->config_ptr->element_length;

                //ALB copy the the local element in the streamer
                memcpy(curr_consumer_element_ptr,curr_data_ptr,mod_som_efe_ptr->config_ptr->element_length);
//                memcpy(curr_consumer_element_ptr,fake_data_ptr,
//                       mod_som_efe_ptr->config_ptr->element_length);


                mod_som_efe_ptr->consumer_ptr->cnsmr_cnt++;  // increment cnsmr count
                elmnts_avail = mod_som_efe_ptr->sample_count -
                               mod_som_efe_ptr->consumer_ptr->cnsmr_cnt; //elements available have been produced
                if((mod_som_efe_ptr->consumer_ptr->cnsmr_cnt %
                    mod_som_efe_ptr->settings_ptr->nb_sample_per_record) ==0){
                    mod_som_efe_ptr->consumer_ptr->data_ready_flg=1;
                    break;
                }
              }  // end of while (elemts_avail > 0)
            // No more data available. All data are stored in the stream buffer.


            if (mod_som_efe_ptr->consumer_ptr->data_ready_flg &
                mod_som_efe_ptr->consumer_ptr->consumed_flag){

                // We are almost ready to send. Just need to get the header, compute the chcksum, append it
                // to the stream buffer and send to the stream task

                //get the timestamp for the record header
                tick=sl_sleeptimer_get_tick_count64();
                mystatus = sl_sleeptimer_tick64_to_ms(tick,\
                                                      &mod_som_efe_ptr->timestamp);

                //MHA: Now augment timestamp by poweron_offset_ms
                mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
                mod_som_efe_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

                //ALB move the current consumer ptr
                curr_consumer_element_ptr = base_consumer_element_ptr + \
                    (mod_som_efe_ptr->consumer_ptr->cnsmr_cnt-reset_cnsmr_cnt)*\
                    mod_som_efe_ptr->config_ptr->element_length;

                //ALB get the length of the stream block
                mod_som_efe_ptr->consumer_ptr->payload_length= \
                    (int) &curr_consumer_element_ptr[0]- \
                    (int) base_consumer_element_ptr;

                //ALB create header
                mod_som_efe_header_f(mod_som_efe_ptr->consumer_ptr);
                //add header to the beginning of the stream block
                memcpy(mod_som_efe_ptr->consumer_ptr->record_data_ptr, \
                       mod_som_efe_ptr->consumer_ptr->header,
                       mod_som_efe_ptr->consumer_ptr->length_header);


                //ALB compute checksum
                mod_som_efe_ptr->consumer_ptr->chksum=0;
                for(int i=0;i<mod_som_efe_ptr->consumer_ptr->payload_length;i++)
                  {
                    mod_som_efe_ptr->consumer_ptr->chksum ^=\
                        base_consumer_element_ptr[i];
                  }


                //ALB the curr_consumer_element_ptr should be at the right place to
                //ALB write checksum at the end of the record.
                *(curr_consumer_element_ptr++) = '*';
                *((uint16_t*)curr_consumer_element_ptr) = \
                    mod_som_int8_2hex_f(mod_som_efe_ptr->consumer_ptr->chksum);
                curr_consumer_element_ptr += 2;
                *(curr_consumer_element_ptr++) = '\r';
                *(curr_consumer_element_ptr++) = '\n';

                //ALB get the length of the record with the checksum
                mod_som_efe_ptr->consumer_ptr->record_length= \
                    (int) &curr_consumer_element_ptr[0]- \
                    (int) &mod_som_efe_ptr->consumer_ptr->record_data_ptr[0];



                // ALB Do we want to send a fix length block or send a variable length block
                //
                switch(mod_som_efe_ptr->consumer_mode){
                  case 0:
                    mod_som_efe_ptr->consumer_ptr->consumed_flag=false;
                    mod_som_io_stream_data_f(
                        mod_som_efe_ptr->consumer_ptr->record_data_ptr,
                        mod_som_efe_ptr->consumer_ptr->record_length,
                        &mod_som_efe_ptr->consumer_ptr->consumed_flag);
                    break;
                  case 1:
                    mod_som_efe_ptr->consumer_ptr->consumed_flag=false;
                    mod_som_sdio_write_data_f(rawfile_ptr,
                        mod_som_efe_ptr->consumer_ptr->record_data_ptr,
                        mod_som_efe_ptr->consumer_ptr->record_length,
                        &mod_som_efe_ptr->consumer_ptr->consumed_flag);
                    break;
                  default:
                    break;
                }

                mod_som_efe_ptr->consumer_ptr->elmnts_skipped = 0;
                // reset the stream ptr.
                curr_consumer_element_ptr = base_consumer_element_ptr;

                //ALB update reset_cnsmr_cnt so we can fill the stream block from 0 again
                reset_cnsmr_cnt=mod_som_efe_ptr->consumer_ptr->cnsmr_cnt;
                mod_som_efe_ptr->consumer_ptr->data_ready_flg=0;

            }//end if (mod_som_efe_ptr->sampling_flag)
        } // data_ready_flg

        // Delay Start Task execution for
        OSTimeDly( MOD_SOM_EFE_CONSUMER_DELAY,             //   consumer delay is #define at the beginning OS Ticks
                   OS_OPT_TIME_DLY,          //   from now.
                   &err);
        if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
            error_cnt = 0;
        }
        else{
            error_cnt++;
        }
        if(error_cnt>MOD_SOM_MAX_ERROR_CNT){
            mod_som_io_print_f("%s error accumulation maxed\r\n",__func__);
            return;
        }
//        //   Check error code.
//        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
    } // end of while (DEF_ON)

    PP_UNUSED_PARAM(p_arg);                                     // Prevent config warning.
}

//static  void  mod_som_efe_store_start_task_f (){
//
//}


/*******************************************************************************
 * @brief
 *   enable EFE hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_enable_hardware_f(){

	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.
	//ALB TODO change the port and pin syntax using the som headers

	/* Analog Enable*/
	GPIO_PinModeSet(gpioPortB, 7, gpioModePushPull, 1); 	// power the ADC

#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
	//	GPIO_PinModeSet(gpioPortA,  4, gpioModeWiredAndPullUp, 0);
	GPIO_PinModeSet(gpioPortA,  4, gpioModePushPull, 1);  // OE
	GPIO_PinModeSet(gpioPortA, 14, gpioModePushPull, 0);  // A0
	GPIO_PinModeSet(gpioPortA, 13, gpioModePushPull, 0);  // A1
	GPIO_PinModeSet(gpioPortA, 12, gpioModePushPull, 0);  // A2
	GPIO_PinModeSet(gpioPortA,  7, gpioModePushPull, 0);  // A3
#endif

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   disable EFE hardware
 *
 * @return
 *   MOD_SOM_STATUS_OK if initialization goes well
 *   or otherwise
 ******************************************************************************/
mod_som_status_t mod_som_efe_disable_hardware_f(){
	//ALB TODO change the port and pin syntax using the som hearders

	/* Analog Enable*/
	GPIO_PinModeSet(gpioPortB, 7, gpioModePushPull, 0); 	// power the ADC

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

}


/*******************************************************************************
 * @brief
 *   Initialize setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_default_settings_f(mod_som_efe_settings_ptr_t settings_ptr){

	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.

	//ALB set default efe rev number (rev3)
	strcpy(settings_ptr->rev,MOD_SOM_EFE_DEFAULT_EFE_REV_NAME);
	//ALB set default efe sn (000)
	strcpy(settings_ptr->sn, MOD_SOM_EFE_DEFAULT_EFE_SN);

	//ALB if you want change the number of channel change the
	//ALB settings_ptr->number_of_channels

	settings_ptr->number_of_channels   = MOD_SOM_EFE_USER_NBCHANNEL;
	settings_ptr->nb_sample_per_record = MOD_SOM_EFE_SAMPLE_PER_RECORD;
	settings_ptr->nb_record_per_buffer = MOD_SOM_EFE_RECORD_PER_BUFFER;
	settings_ptr->spi_baudrate         = MOD_SOM_EFE_SPI_BAUDRATE;
	//ALB this not factorized. We need a solution to select the number of channels easily.
	settings_ptr->sensors[0]=ch1; //t1
  settings_ptr->sensors[1]=ch3; //s1
  settings_ptr->sensors[2]=ch7; //a3
  settings_ptr->sensors[3]=ch6; //a2. I need to config a2. TODO
  settings_ptr->sensors[4]=ch5; //a1
	settings_ptr->sensors[5]=ch2; //t2
	settings_ptr->sensors[6]=ch4; //s2

	strncpy(settings_ptr->header,MOD_SOM_EFE_HEADER,MOD_SOM_EFE_SETTINGS_STR_lENGTH );
	settings_ptr->initialize_flag=true;
	settings_ptr->size=sizeof(*settings_ptr);

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_efe_settings_t mod_som_efe_get_settings_f(){
	return *mod_som_efe_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get the setup struct ptr
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_efe_settings_ptr_t mod_som_efe_get_settings_ptr_f(){
  return mod_som_efe_ptr->settings_ptr;
}

/*******************************************************************************
 * @brief
 *   get the runtime struct ptr
 *
 * @param
 *
 ******************************************************************************/
mod_som_efe_ptr_t mod_som_efe_get_runtime_ptr_f(){
  return mod_som_efe_ptr;
}


/*******************************************************************************
 * @brief
 *   set the setup struct ptr.
 *   This function should get a new setup struct from the setup module (could be from SD card or other source)
 *   it should copy the value from that new setup structure into the the "official" efe-setup_struct
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
void mod_som_efe_set_setup_f(mod_som_efe_settings_ptr_t settings_ptr){
//	mod_som_efe_ptr->settings_ptr=settings_ptr;
}


/*******************************************************************************
 * @brief
 *   Initialize configuration pointer with default data defined by
 *   mod_bsp_config.h
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_status_t mod_som_efe_config_f(mod_som_efe_config_ptr_t config_ptr){

	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.


	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

}

/*******************************************************************************
 * @brief
 *   get the config pointer.
 *   to be used by other modules
 *
 * @param config_ptr
 *   configuration pointer
 ******************************************************************************/
mod_som_efe_config_ptr_t mod_som_efe_get_config_f(){
return  mod_som_efe_ptr->config_ptr;
}


/***************************************************************************//**
 * @brief
 *   Initialize USART 0 for adc SPI communication
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_spi_f(mod_som_efe_comm_handle_ptr_t config_com_ptr)
{
	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.

	USART_InitSync_TypeDef  initusart = USART_INITSYNC_DEFAULT;
	CMU_Clock_TypeDef       efe_spi_clk;
	USART_TypeDef           *usart_ptr;

	usart_ptr  = (USART_TypeDef *)config_com_ptr->usart;
	efe_spi_clk = MOD_SOM_EFE_SPI_CLK;

	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(efe_spi_clk, true);

	/* IO configuration */
	GPIO_PinModeSet(config_com_ptr->miso.port, config_com_ptr->miso.pin,	gpioModeInput, 0);  /* MISO */
	GPIO_PinModeSet(config_com_ptr->mosi.port, config_com_ptr->mosi.pin,	gpioModePushPull, 0);     /* MOSI */
	GPIO_PinModeSet(config_com_ptr->clck.port, config_com_ptr->clck.pin,	gpioModePushPull, 1);  /* CLK */

	/* Initialize USART in SPI master mode. */
	initusart.baudrate = config_com_ptr->baudrate;			// baudRate defined in common.h
	initusart.msbf     = config_com_ptr->msbf; 			// Analog devices is big enDian

	//ALB I do not know how to make it not system dependent by calling ClockMode3
	//TODO make it not system dependent
	// ALB coose ClockMode3 to match the EFE ADC spec-sheet
	initusart.clockMode = usartClockMode3;
	//set USART in a known state
	USART_Reset(usart_ptr);
	//initialize a SPI port
	USART_InitSync(usart_ptr, &initusart);

	/* define route for usart0 location2*/
	//ALB I use direct manipulation of USART0 register because I do not know how to do it differently
	//ALB it makes this part VERY system dependent
	//TODO change it so it is not system dependent.
	usart_ptr->ROUTELOC0 = (USART0->ROUTELOC0
			& ~(_USART_ROUTELOC0_TXLOC_MASK
					| _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK) )
					   | USART_ROUTELOC0_TXLOC_LOC2
					   | USART_ROUTELOC0_RXLOC_LOC2
					   | USART_ROUTELOC0_CLKLOC_LOC2;
	/* Enabling pins and setting location, SPI /CS bus controlled independently */
	usart_ptr->ROUTEPEN = USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN| USART_ROUTEPEN_CLKPEN;

	/* enable SPI */
	USART_Enable(usart_ptr, usartEnable);

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

}


/***************************************************************************//**
 * @brief
 *   Initialize USART 0 for adc SPI communication
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_mclock_f(mod_som_timer_handle_ptr_t config_mclock_ptr,mod_som_timer_handle_ptr_t config_sync_ptr)
{

	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.

	CMU_Clock_TypeDef efe_mclock_clk = MOD_SOM_EFE_MCLOCK_CLK;
	CMU_Clock_TypeDef efe_sync_clk   = MOD_SOM_EFE_SYNC_CLK;

	/* enable MCLOCK - using wide timer 0 */
	GPIO_PinModeSet(config_mclock_ptr->timer_pin.port, \
			config_mclock_ptr->timer_pin.pin, \
			gpioModePushPull, 1); // PB2 in ouptput mode to send the MCLOCK (WTIMER0) to the adc
	CMU_ClockEnable(efe_mclock_clk, true);

	// ALB Initilize capture compare timer. This timer generate the EFE adc master clock
	// ALB at a defined GPIO pin connected to the ADCs
	TIMER_InitCC_TypeDef init_wtimer0=TIMER_INITCC_DEFAULT;
	init_wtimer0.cmoa=timerOutputActionToggle;
	init_wtimer0.mode=timerCCModeCompare;
	init_wtimer0.outInvert=true;
	TIMER_TopSet(config_mclock_ptr->timer, config_mclock_ptr->top);
	TIMER_CompareSet(config_mclock_ptr->timer,2,config_mclock_ptr->compare_value);
	TIMER_InitCC(config_mclock_ptr->timer,2,&init_wtimer0);

	//ALB it makes this part VERY system dependent
	//TODO change it so it is not system dependent.
	//counting up
	WTIMER0->CTRL =(WTIMER0->CTRL &~_WTIMER_CTRL_MODE_MASK)| WTIMER_CTRL_MODE_UP;
	//	WTIMER0->CTRL =(WTIMER0->CTRL &~_WTIMER_CTRL_DEBUGRUN_MASK)| WTIMER_CTRL_DEBUGRUN;
	//set route cc2 loc 5
	WTIMER0->ROUTELOC0 =(WTIMER0->ROUTELOC0 &~_WTIMER_ROUTELOC0_MASK)| WTIMER_ROUTELOC0_CC2LOC_LOC5;
	//enable cc2 pin out
	WTIMER0->ROUTEPEN =(WTIMER0->ROUTEPEN &~_WTIMER_ROUTEPEN_MASK)| WTIMER_ROUTEPEN_CC2PEN;


	//	//configure TIMER1 to help SYNC the ADC
	CMU_ClockEnable(efe_sync_clk, true);
	GPIO_PinModeSet(config_sync_ptr->timer_pin.port, \
			config_sync_ptr->timer_pin.pin, \
			gpioModePushPull, 1); // PC7 in ouptput mode to send the SYNC (WTIMER1) to the adc
	TIMER_Init_TypeDef init_wtimer1=TIMER_INIT_DEFAULT;
	init_wtimer1.oneShot=true;
	init_wtimer1.sync=false;
	init_wtimer1.enable=false;
	TIMER_Init(config_sync_ptr->timer,&init_wtimer1);


	//ALB I use direct manipulation of WTIMER1 register because I do not know how to do it differently
	//ALB it makes this part VERY system dependent
	//TODO change it so it is not system dependent.
	WTIMER1->CTRL =(WTIMER1->CTRL &~_WTIMER_CTRL_CLKSEL_MASK)| WTIMER_CTRL_CLKSEL_TIMEROUF;
	WTIMER1->CC[0].CTRL =(WTIMER1->CC[0].CTRL &~_WTIMER_CC_CTRL_MODE_MASK)| WTIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
	WTIMER1->CC[0].CTRL =(WTIMER1->CC[0].CTRL &~_WTIMER_CC_CTRL_OUTINV_MASK)| WTIMER_CC_CTRL_OUTINV;
	WTIMER1->CC[0].CTRL =(WTIMER1->CC[0].CTRL &~_WTIMER_CC_CTRL_CMOA_MASK)| WTIMER_CC_CTRL_CMOA_TOGGLE;
	WTIMER1->CC[0].CTRL =(WTIMER1->CC[0].CTRL &~_WTIMER_CC_CTRL_INSEL_MASK)| WTIMER_CC_CTRL_INSEL_PIN;
	TIMER_TopSet(config_sync_ptr->timer, config_sync_ptr->compare_value);
	WTIMER1->ROUTELOC0 =(WTIMER1->ROUTELOC0 &~_WTIMER_ROUTELOC0_MASK)| WTIMER_ROUTELOC0_CC0LOC_LOC3; //
	WTIMER1->ROUTEPEN =(WTIMER1->ROUTELOC0 &~_WTIMER_ROUTELOC0_MASK)| WTIMER_ROUTEPEN_CC0PEN;

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);

}

/***************************************************************************//**
 * @brief
 *   Initialize LDMA
 ******************************************************************************/
mod_som_status_t mod_som_efe_init_ldma_f(mod_som_efe_ptr_t module_ptr)
{
	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.

	mod_som_efe_ptr->ldma.ch=MOD_SOM_EFE_LDMA_CH;

	//ALB initialize the LDMA clock
	CMU_Clock_TypeDef efe_ldma_clk = MOD_SOM_EFE_LDMA_CLCK;
	CMU_ClockEnable(efe_ldma_clk, true);
	LDMA_Init_t init = LDMA_INIT_DEFAULT;
	//ALB LDMA IRQ default priority is 3.
	LDMA_Init( &init );

	// set LDMA descriptors for Direct Memory access and transfer -> ADC to memory using CS cascade
	// Define LDMA_TransferCfg_t for ADC config LDMA transfer

	// ALB create read descriptor list
	// ALB this the main descriptor list used during the sampling.
	// ALB The LDMA transfer define by this list is called inside the GPIO interrupt handler
	// ALB after the ADC send their interrupt signal
	mod_som_efe_define_read_descriptor_f(module_ptr);

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}


/***************************************************************************//**
 * @brief
 *   build header data
 *   TODO add different flag as parameters like
 *        - data overlap
 *        -
 *   TODO
 ******************************************************************************/
void mod_som_efe_header_f(mod_som_efe_data_consumer_ptr_t consumer_ptr)
{

  //time stamp
  uint32_t t_hex[2];
  uint8_t * local_header;


  t_hex[0] = (uint32_t) (mod_som_efe_ptr->timestamp>>32);
  t_hex[1] = (uint32_t) mod_som_efe_ptr->timestamp;

	//header  contains $EFE,flags. Currently flags are hard coded to 0x1e
	//time stamp will come at the end of header
	sprintf((char*) mod_som_efe_ptr->consumer_ptr->header,  \
			"$%s%08x%08x%08x*FF", \
			mod_som_efe_ptr->settings_ptr->header, \
			(int) t_hex[0],\
			(int) t_hex[1],\
			(int) consumer_ptr->payload_length);

  consumer_ptr->header_chksum=0;
  for(int i=0;i<consumer_ptr->length_header-
               MOD_SOM_EFE_LENGTH_HEADER_CHECKSUM;i++) // 29 = sync char(1)+ tag (4) + hextimestamp (16) + payload size (8).
    {
      consumer_ptr->header_chksum ^=\
          consumer_ptr->header[i];
    }


  // the curr_consumer_element_ptr should be at the right place to
  // write the checksum already
  //write checksum at the end of the steam block (record).
  local_header = &consumer_ptr->header[consumer_ptr->length_header-
                                       MOD_SOM_EFE_LENGTH_HEADER_CHECKSUM+1];
  *((uint16_t*)local_header) = \
      mod_som_int8_2hex_f(consumer_ptr->header_chksum);

}

/*******************************************************************************
 * @brief
 *   a function to get the efe id
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_efe_id_f(CPU_INT16U argc,CPU_CHAR *argv[]){

	if (argc==1){
		printf("EFE, %s, %s.\r\n",(char*) mod_som_efe_ptr->settings_ptr->rev ,(char*) mod_som_efe_ptr->settings_ptr->sn);
	}
	else{
		//ALB switch statement easy to handle all user input cases.
		switch (argc){
		case 3:
			strcpy(mod_som_efe_ptr->settings_ptr->rev,argv[1]);
			strcpy(mod_som_efe_ptr->settings_ptr->sn,argv[2]);
			break;
		default:
			printf("format: efe.id rev3 001\r\n");
			break;
		}
	}
}

/*******************************************************************************
 * @brief
 *   a function to get the efe id
 *
 * @return
 *   MOD_SOM_STATUS_OK if function execute nicely
 ******************************************************************************/
void mod_som_efe_probe_id_f(CPU_INT16U argc,CPU_CHAR *argv[]){
	int sensor_index;
	float cal;
	RTOS_ERR  p_err;

	if (argc==1){
		printf("EFE probe1, %s, %s, %3.2f.\r\n",(char*) mod_som_efe_ptr->settings_ptr->sensors[0].name , \
				                              (char*) mod_som_efe_ptr->settings_ptr->sensors[0].sn,    \
											          mod_som_efe_ptr->settings_ptr->sensors[0].cal);
		printf("EFE probe2, %s, %s, %3.2f.\r\n",(char*) mod_som_efe_ptr->settings_ptr->sensors[1].name , \
				                              (char*) mod_som_efe_ptr->settings_ptr->sensors[1].sn,    \
											          mod_som_efe_ptr->settings_ptr->sensors[1].cal);
		printf("EFE probe3, %s, %s, %3.2f.\r\n",(char*) mod_som_efe_ptr->settings_ptr->sensors[2].name , \
				                              (char*) mod_som_efe_ptr->settings_ptr->sensors[2].sn   , \
											          mod_som_efe_ptr->settings_ptr->sensors[2].cal);
		printf("EFE probe4, %s, %s, %3.2f.\r\n",(char*) mod_som_efe_ptr->settings_ptr->sensors[3].name , \
				                              (char*) mod_som_efe_ptr->settings_ptr->sensors[3].sn   , \
											          mod_som_efe_ptr->settings_ptr->sensors[3].cal);
	}
	else{
		//ALB switch statement easy to handle all user input cases.
		switch (argc){
		case 5:
			sensor_index=shellStrtol(argv[1],&p_err);
			cal=strtof(argv[4],DEF_NULL);
			strcpy(mod_som_efe_ptr->settings_ptr->sensors[sensor_index-1].name,argv[2]);
			strcpy(mod_som_efe_ptr->settings_ptr->sensors[sensor_index-1].sn,argv[3]);
			mod_som_efe_ptr->settings_ptr->sensors[sensor_index-1].cal=cal;
			break;
		default:
			printf("format: efe.id probe_id channel_name probe_sn probe_cal\r\n");
			break;
		}
	}
}


/***************************************************************************//**
 * @brief
 * This is THE function starting the EFE sampling.
 *   Start timer for ADC mclock
 *   Start timer to sync the adc
 *   setup gpio interrupt
 *   WTIMER0 WTIMER1 and enable the GPIO interrupt on PE5 - data ready
 ******************************************************************************/
void mod_som_efe_start_mclock_f(mod_som_efe_ptr_t module_ptr)
{

	/*start mclock timer, default 625kHz send to the GPIO pin connected to the ADCs */
	TIMER_Enable(module_ptr->config_ptr->mclock->timer, true);
	/*start sync timer. This timer is used to synchronized the ADCs*/
	TIMER_Enable(module_ptr->config_ptr->sync->timer, true);



	/*gpio configuration for interrupt signal  PE11*/
	//TODO use Port and pin define from the header.
	GPIO_PinModeSet(module_ptr->config_ptr->pin_interrupt.port, \
			module_ptr->config_ptr->pin_interrupt.pin, gpioModeInputPull, 1);

	/*clear interrupt flags for GPIO ODD. Putting interrupt in a known state*/
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	/*set interrupt priority to 1*/
	//TODO move the interrupt priority value in the header.
	NVIC_SetPriority(GPIO_ODD_IRQn, 1);
	/*Enable GPIO request*/
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

	//TODO use Port and pin define from the header.
	// risingEdge is false,
	// fallingEdge is true,
	// GPIO int is not enabled now. Will do in the next lines.
	GPIO_ExtIntConfig(module_ptr->config_ptr->pin_interrupt.port,    \
			module_ptr->config_ptr->pin_interrupt.pin,     \
			module_ptr->config_ptr->pin_interrupt.pin, false, true, false);

	// clear and enable the gpio interrupt.
	GPIO_IntClear(module_ptr->config_ptr->pin_interrupt_address);
	GPIO_IntEnable(module_ptr->config_ptr->pin_interrupt_address);


	// manually bring the first CS to low;
	// TODO bring the CS of the first channel in sensors list to low
#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
	//selector output enable to high all CS high
	//selector A0,A1,A2,A3 low first ADC (i.e. CS1) low
	USART0->CMD = USART_CMD_CLEARTX;

	//	GPIO_PinModeSet(selectort_port_enable,selector_pin_OE, gpioModePushPull, 1);
	//	GPIO_PinModeSet(selectort_port_enable,selector_pin_OE, gpioModeWiredAndPullUp, 1);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_OE, gpioModePushPull, 1);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_A0, gpioModePushPull, 0);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_A1, gpioModePushPull, 0);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_A2, gpioModePushPull, 0);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_A3, gpioModePushPull, 0);
	GPIO_PinModeSet(selectort_port_enable,selector_pin_OE, gpioModePushPull, 0);
#else
	GPIO_PinModeSet(module_ptr->settings_ptr->sensors[0].csLoc.gpioPort,  \
			module_ptr->settings_ptr->sensors[0].csLoc.gpioPin, gpioModePushPull, 0);
#endif


}


/***************************************************************************//**
 * @brief
 *   stop mclock
 *
 ******************************************************************************/
void mod_som_efe_stop_mclock_f(mod_som_efe_ptr_t module_ptr)
{
	uint32_t delay;

	//disable gpio interupt (to trigger another LDMA transfer).
	GPIO_IntDisable(module_ptr->config_ptr->pin_interrupt_address);
	//clear GPIO IF
	GPIO_IntClear(module_ptr->config_ptr->pin_interrupt_address);

	//stop mclock timer.
	TIMER_Enable(module_ptr->config_ptr->mclock->timer, false);
	//stop sync timer.
	TIMER_Enable(module_ptr->config_ptr->mclock->timer, false);

	//stop current LDMA transfer.
	LDMA_StopTransfer(module_ptr->ldma.ch);
	//some delay to ensure everything is done.
	delay = 0xFF;
	while (delay--); // Delay after power on to ensure all DMA transfer are done.

#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
	GPIO_PinModeSet(gpioPortA,selector_pin_OE, gpioModePushPull, 1);  // all CS high
#else
	/*set all CS to high*/
	for (int i=0;i<module_ptr->settings_ptr->number_of_channels;i++){
		GPIO_PinModeSet(module_ptr->settings_ptr->sensors[i].csLoc.gpioPort, \
				module_ptr->settings_ptr->sensors[i].csLoc.gpioPin, gpioModePushPull, 1);  // CS high
	}
#endif
}



/***************************************************************************//**
 * @brief
 *   Configure ADCsInitialize USART 0 for adc SPI communication
 ******************************************************************************/
mod_som_status_t mod_som_efe_config_adc_f(mod_som_efe_ptr_t module_ptr)
{

	// ALB configure the ADCs using LDMA. Note: LDMA is not a requirement
	// but I used it as way to learn about how LDMA works and kept the code

	// TODO It will always return MOD_SOM_STATUS_OK.
	// TODO I do not know how to check if everything is fine here.


#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
	GPIO_PinModeSet(gpioPortA,selector_pin_OE, gpioModePushPull, 1);  // all CS high
#else
	/*set all CS to high*/
	for (int i=0;i<module_ptr->settings_ptr->number_of_channels;i++){
		GPIO_PinModeSet(module_ptr->settings_ptr->sensors[i].csLoc.gpioPort, \
				module_ptr->settings_ptr->sensors[i].csLoc.gpioPin, gpioModePushPull, 1);  // CS high
	}
#endif

	// config adc
	// clear TX since TX is the source trigger
	USART0->CMD = USART_CMD_CLEARTX;
	// config descriptor list
	// ALB define config descriptor needs to be invoke here. if defined after right after define_reset_descriptor the 1st CS is too short
	// ALB I do not know why.
	for (uint8_t i=0;i<module_ptr->settings_ptr->number_of_channels;i++){
		// ALB reset the the select ADC to put it in a know state.
		mod_som_efe_adc_reset_register_f(module_ptr->ldma.ch,\
				&module_ptr->settings_ptr->sensors[i]);

		// ALB config config0 register
		mod_som_efe_adc_config_register_f(module_ptr->ldma.ch,&module_ptr->settings_ptr->sensors[i],AD7124_REG_CFG0 ,(uint32_t)module_ptr->settings_ptr->sensors[i].registers.CONFIG_0,LCFG0);
		// ALB config channel0 register
		mod_som_efe_adc_config_register_f(module_ptr->ldma.ch,&module_ptr->settings_ptr->sensors[i],AD7124_REG_CH0  ,(uint32_t)module_ptr->settings_ptr->sensors[i].registers.CHANNEL_0,LCH0);
		// ALB config filter0 register (define sampling frequency and filter shape)
		mod_som_efe_adc_config_register_f(module_ptr->ldma.ch,&module_ptr->settings_ptr->sensors[i],AD7124_REG_FLTR0,(uint32_t)module_ptr->settings_ptr->sensors[i].registers.FILTER_0,LFLTR0);
		mod_som_efe_adc_config_register_f(module_ptr->ldma.ch,&module_ptr->settings_ptr->sensors[i],AD7124_REG_IOCTRL1,(uint32_t)module_ptr->settings_ptr->sensors[i].registers.IO_CONTROL_1,LIOCTRL1);
		// ALB config control register.
		// ALB Note: the control register HAS to be the last one because
		// ALB if the ADC are configured in a continuous mode the user cannot write anything to the ADC after this register
		mod_som_efe_adc_config_register_f(module_ptr->ldma.ch,&module_ptr->settings_ptr->sensors[i],AD7124_REG_CTRL ,(uint32_t)module_ptr->settings_ptr->sensors[i].registers.ADC_CONTROL,LCTRL);

	}

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}
/***************************************************************************//**
 * @brief
 *   reset adc's via ldma
 ******************************************************************************/
void mod_som_efe_reset_adc_f(mod_som_efe_ptr_t module_ptr)
{
	// clear USART channel
	USART0->CMD = USART_CMD_CLEARTX;
	// send LDMA transfer to reset ADCs
	for (uint8_t i=0;i<module_ptr->settings_ptr->number_of_channels;i++){
		// reset descriptor list
		mod_som_efe_adc_reset_register_f(module_ptr->ldma.ch,\
				&module_ptr->settings_ptr->sensors[i]);
	}
}


/***************************************************************************//**
 * @brief
 *   // LDMA A/D SPI transfer descriptor list generator
 ******************************************************************************/
#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
void mod_som_efe_define_read_descriptor_f(mod_som_efe_ptr_t module_ptr)
{
	uint32_t number_of_channels=module_ptr->settings_ptr->number_of_channels;
	uint32_t j;

	read_adc_cmd[0]=0x00;
	read_adc_cmd[1]=0x00;
	read_adc_cmd[2]=0x00;

  // initialization phase: get the 64bits timestamp and write it in the circular buffer
  // initialization phase: deassert OE (all CS are disabled, clear all GPIO bits)
	// phase 1: deassert OE (ensure TXEMPTY)
	// phase 2: set GPIO code for new selector address (immediate)
	// phase 3: assert OE (ensure TXEMPTY)

//  //ALB first step is to write the 64 bits timestamps in the circular buffer.
//  j=0;// Descriptor index 0: De-assert selector output enable
//  descriptor_link_read1[j] = timestamp_write;
//  descriptor_link_read1[j].xfer.srcAddr=(uint32_t) (&mod_som_efe_ptr->timestamp);
//  descriptor_link_read1[j].xfer.dstAddr=(uint32_t)(mod_som_efe_ptr->config_ptr->element_map[0]);
//  descriptor_link_read1[j].xfer.linkAddr=LDMA_1_STEP;
//  descriptor_link_read1[j].xfer.size=ldmaCtrlSizeWord;
//  descriptor_link_read1[j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;
//  descriptor_link_read1[j].xfer.xferCnt=(2)-1;
//  descriptor_link_read1[j].xfer.doneIfs=0;

  //MAG pre-index j to 0 to point first descriptor (No need to repeat this step for every channel)
	j=0;// Descriptor index 0: De-assert selector output enable
	descriptor_link_read1[j] = cs_assert;
	descriptor_link_read1[j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_read1[j].wri.immVal=oe_bit_mask;
	descriptor_link_read1[j].wri.linkAddr=LDMA_1_STEP;//MAG 7AUG2020 TODO make this part of the cs assert define

	j++;// Descriptor index 1: Set all 4 address bits to ones //TODO make this positive logic so we would clear them all here
	descriptor_link_read1[j] = cs_assert;
	descriptor_link_read1[j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_read1[j].wri.immVal=all_bit_mask;
	descriptor_link_read1[j].wri.linkAddr=LDMA_1_STEP;
	//Now that we've done the initial clear of the OE* we can loop for each channel
	for (int i=0;i<number_of_channels;i++){

		j++;// Descriptor index 2: Clear all bits not in address mask  //TODO make this positive logic so we would set the selected address here
		descriptor_link_read1[j] = cs_assert;
		descriptor_link_read1[j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
		descriptor_link_read1[j].wri.immVal= module_ptr->settings_ptr->sensors[i].selector_cs_code;
		descriptor_link_read1[j].wri.linkAddr=LDMA_1_STEP;

		j++;// Descriptor index 3: Assert selector output enable
		descriptor_link_read1[j] = cs_assert;
		descriptor_link_read1[j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
		descriptor_link_read1[j].wri.immVal= oe_bit_mask;
		descriptor_link_read1[j].wri.linkAddr=LDMA_1_STEP;

		j++;// Descriptor index 4: Perform 3-byte SPI transfer from ADC by sending 3 bytes (one per transfer) to the SPI TXDATA register (this will wait on TXBL signal)
		descriptor_link_read1[j] = adc_write;
		descriptor_link_read1[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
		descriptor_link_read1[j].xfer.srcAddr=(uint32_t) (&read_adc_cmd);
		descriptor_link_read1[j].xfer.linkAddr=LDMA_1_STEP;
		descriptor_link_read1[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
		descriptor_link_read1[j].xfer.xferCnt=(3)-1;
		descriptor_link_read1[j].xfer.doneIfs=0;
		descriptor_link_read1[j].xfer.structReq=0;

		j++;// Descriptor index 5: // De-assert selector output enable after TXBL signal (transmit buffer is empty)
		//MAG 7AUG2020 use xfer descriptor (instead if wrt descriptor that ignores structReq=0) so we can wait until the txbl  trigger (structReq=0) signaling the end of the
		descriptor_link_read1[j] = adc_read;
		descriptor_link_read1[j].xfer.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
		descriptor_link_read1[j].xfer.srcInc=ldmaCtrlDstIncNone;
		descriptor_link_read1[j].xfer.srcAddr=(uint32_t) (&oe_bit_mask);//MAG 7AUG2020 this pointer must be static as the LDMA runs out of this scope.
		descriptor_link_read1[j].xfer.linkAddr=LDMA_1_STEP;
		descriptor_link_read1[j].xfer.size=2;//MAG 7AUG2020 unit size = word (4 bytes)
		descriptor_link_read1[j].xfer.xferCnt=0;//MAG 7AUG2020 count 0 is the setting for 1 transfer
		descriptor_link_read1[j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;//MAG 7AUG2020 1 block so the result of these 3 parameters is 1 32 bit word transfered at one time
		descriptor_link_read1[j].xfer.doneIfs=0;
		descriptor_link_read1[j].xfer.structReq=0;

		j++;// Descriptor index 6: Set all 4 address bits to ones //TODO make this positive logic so we would clear them all here
		descriptor_link_read1[j] = cs_assert;
		descriptor_link_read1[j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
		descriptor_link_read1[j].wri.immVal=all_bit_mask;
		descriptor_link_read1[j].wri.linkAddr=LDMA_1_STEP;

		j++;// Descriptor index : // After TXBL signal (transmit buffer is empty)	one byte at a time read the 3 bytes of rx data from the SPI RXDATA register and write them to the circular buffer
		descriptor_link_read1[j] = adc_read;
		descriptor_link_read1[j].xfer.dstAddr=(uint32_t) (mod_som_efe_ptr->config_ptr->element_map[0]+(MOD_SOM_EFE_TIMESTAMP_LENGTH+3*i));
		descriptor_link_read1[j].xfer.dstInc=ldmaCtrlDstIncOne;
		descriptor_link_read1[j].xfer.srcAddr=(uint32_t) (&USART0->RXDATA);
		descriptor_link_read1[j].xfer.linkAddr=LDMA_1_STEP;
		descriptor_link_read1[j].xfer.xferCnt=(3)-1;
		descriptor_link_read1[j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;
		descriptor_link_read1[j].xfer.doneIfs=0;
		if(i==0)//MAG 11AUG2020 store offset to first RXDATA read descriptor so we can use it in the LDMA interrupt routine to index the circular buffer
			module_ptr->ldma_spi_read_descriptor_offset = j;

	}

	//MAG 7AUG2020 add extra phase to allow address lines to settle before OE (CS)is enabled
	j++;// Descriptor index 2nd from last: //select initial address
	descriptor_link_read1[j] = cs_assert;
	descriptor_link_read1[j].wri.dstAddr = module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_read1[j].wri.immVal = module_ptr->settings_ptr->sensors[0].selector_cs_code;
	descriptor_link_read1[j].wri.linkAddr = LDMA_1_STEP;

	j++;// Descriptor index last: Assert selector output enable
	descriptor_link_read1[j]=single_cs_deassert;
	descriptor_link_read1[j].wri.dstAddr = module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_read1[j].wri.immVal = oe_bit_mask;
	descriptor_link_read1[j].wri.doneIfs = 1;
	//	descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*(number_of_channels-1)+j].wri.linkAddr=LDMA_1_STEP;
}

#else
void mod_som_efe_define_read_descriptor_f(mod_som_efe_ptr_t module_ptr)
{
	uint32_t number_of_channels=module_ptr->settings_ptr->number_of_channels;
	int j;
	read_adc_cmd[0]=0x00;
	read_adc_cmd[1]=0x00;
	read_adc_cmd[2]=0x00;

	for (int i=0;i<number_of_channels;i++){
		// ALB CS of the selected ADC low
		  j=0;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j] = cs_assert;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.immVal=1<<mod_som_efe_ptr->settings_ptr->sensors[i].csLoc.gpioPin;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.linkAddr=LDMA_1_STEP;
    	  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.structReq=0;


  		// ALB send 3 empty byters to read ADc data. Note: ADc should be in continuous read
    	// ALB otherwise you need to send 4 bytes the first one being the read command
    	  j=1;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j] = adc_write;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.srcAddr=(uint32_t) (&read_adc_cmd);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.linkAddr=LDMA_1_STEP;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.xferCnt=(3)-1;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.doneIfs=0;

	  	// ALB get the ADC from the RX FIFO buffer and store them in the circular buffer
		// ALB the address of the next ADC sample in circular buffer is updated in the LDMA interrupt handler
    	  j=2;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j] = adc_read;
		  //ALB VERY IMPORTANT this where we fill the data buffer.
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.dstAddr=(uint32_t) (&module_ptr->rec_buff->efe_data_buffer[3*i]);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.srcAddr=(uint32_t) (&USART0->RXDATA);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.linkAddr=LDMA_1_STEP;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.xferCnt=(3)-1;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.doneIfs=0;

		// ALB same transfer just for LDMA transfer timing issue
    	  j=3;
    	  mod_som_efe_ptr->ldma_spi_read_descriptor_offset=j;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j] = adc_read;
		  //ALB VERY IMPORTANT this where we fill the data buffer.
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.dstAddr=(uint32_t) (&module_ptr->rec_buff->efe_data_buffer[3*i]);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.srcAddr=(uint32_t) (&USART0->RXDATA);
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.srcInc=ldmaCtrlDstIncOne;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.linkAddr=LDMA_1_STEP;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.xferCnt=(3)-1;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].xfer.doneIfs=0;

		// ALB  CS of the selected ADC high
    	  j=4;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j] = cs_deassert;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.immVal=1<<mod_som_efe_ptr->settings_ptr->sensors[i].csLoc.gpioPin;
		  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.linkAddr=LDMA_1_STEP;
    	  descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*i+j].wri.structReq=0;
		}
	// ALB  CS of the first ADC low to trigger the next ADC interrupt signal
	j=5;
	descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*(number_of_channels-1)+j]=single_cs_deassert;
    descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*(number_of_channels-1)+j].wri.dstAddr=module_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*(number_of_channels-1)+j].wri.immVal=1<<module_ptr->settings_ptr->sensors[0].csLoc.gpioPin;
	descriptor_link_read1[MOD_SOM_EFE_LDMA_READ_STEP*(number_of_channels-1)+j].wri.structReq=0;
}

#endif

/***************************************************************************//**
 * @brief
 *   // reset descriptor list
 ******************************************************************************/
#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
void mod_som_efe_adc_reset_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor)
{
	//	LDMA_TransferCfg_t spitx_init= LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXEMPTY);
	static uint8_t resetBytes=0xFF;
	uint8_t dummy_bytes[4];
	uint8_t j;
	uint32_t delay;
	LDMA_Descriptor_t descriptor_link_reset[MOD_SOM_EFE_LDMA_RESET_STEP];

	j=0;
	descriptor_link_reset[j] = cs_assert;
	descriptor_link_reset[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_reset[j].wri.immVal= all_bit_mask|oe_bit_mask;
	descriptor_link_reset[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[j].wri.doneIfs=0;
	j=1;
	descriptor_link_reset[j] = cs_assert;
	descriptor_link_reset[j].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_reset[j].wri.immVal=sensor->selector_cs_code|oe_bit_mask;
	descriptor_link_reset[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[j].wri.doneIfs=0;


	// we send 3 bytes. regMode is block so it is like
	j=2;
	descriptor_link_reset[j] = adc_write;
	descriptor_link_reset[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_reset[j].xfer.srcAddr=(uint32_t) (&resetBytes);
	descriptor_link_reset[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_reset[j].xfer.xferCnt=RESET_CYCLE;
	descriptor_link_reset[j].xfer.srcInc=ldmaCtrlSrcIncNone; //No increment
	descriptor_link_reset[j].xfer.doneIfs=0;

	j=3;
	descriptor_link_reset[j] = adc_read;
	descriptor_link_reset[j].xfer.dstAddr=(uint32_t) (&dummy_bytes);
	descriptor_link_reset[j].xfer.srcAddr=(uint32_t) (&USART0->RXDATA);
	descriptor_link_reset[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[j].xfer.xferCnt=(1)-1;
	descriptor_link_reset[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_reset[j].xfer.doneIfs=0;


	j=4;
	descriptor_link_reset[j] = single_cs_deassert;
	descriptor_link_reset[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_reset[j].wri.immVal=sensor->selector_cs_code|oe_bit_mask;
	descriptor_link_reset[j].wri.structReq=0;
	descriptor_link_reset[j].wri.doneIfs=0;


	LDMA_StartTransfer( ldma_ch ,(void*)&spitx_init, (void*)descriptor_link_reset);
	delay = 0xFFFF;
	while (delay--); // Delay after power on to ensure all DMA transfer are done.

}

#else
void mod_som_efe_adc_reset_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor)
{

	static uint8_t resetBytes=0xFF;
	uint8_t dummy_bytes[4];
	uint32_t delay;
	LDMA_Descriptor_t descriptor_link_reset[4];


	// ALB CS of the selected ADC low
	descriptor_link_reset[0] = cs_assert;
	descriptor_link_reset[0].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_reset[0].wri.immVal=1<<sensor->csLoc.gpioPin;
	descriptor_link_reset[0].wri.linkAddr=LDMA_1_STEP;

	// we send 7 0xFF bytes to reset the ADC
	descriptor_link_reset[1] = adc_write;
	descriptor_link_reset[1].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_reset[1].xfer.srcAddr=(uint32_t) (&resetBytes);
	descriptor_link_reset[1].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[1].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_reset[1].xfer.xferCnt=RESET_CYCLE;
	descriptor_link_reset[1].xfer.srcInc=ldmaCtrlSrcIncNone; //No increment

	// ALB dummy read
	// ALB TODO find out why tis is necessary
	descriptor_link_reset[2] = adc_read;
	descriptor_link_reset[2].xfer.dstAddr=(uint32_t) (&dummy_bytes);
	descriptor_link_reset[2].xfer.srcAddr=(uint32_t) (&USART0->RXDATA);
	descriptor_link_reset[2].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_reset[2].xfer.xferCnt=(1)-1;
	descriptor_link_reset[2].xfer.blockSize=ldmaCtrlBlockSizeUnit3;


	// ALB CS of the selected ADC high
	descriptor_link_reset[3] = single_cs_deassert;
	descriptor_link_reset[3].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_reset[3].wri.immVal=1<<sensor->csLoc.gpioPin;
	descriptor_link_reset[3].wri.structReq=0;

	// ALB tigger the LDMA transfer
	LDMA_StartTransfer( ldma_ch ,(void*)&spitx_init, (void*)descriptor_link_reset);

	// ALB delay
	delay = 0xFFFF;
	while (delay--); // Delay after power on to ensure all DMA transfer are done.

}
#endif
/***************************************************************************//**
 * @brief
 *   create config descriptor list for a specific register of the adc
 *   start the ldma Xfer
 *   wait some time
 ******************************************************************************/
#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
void mod_som_efe_adc_config_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor, uint8_t reg_address, uint32_t reg_data, uint8_t nb_bytes)
{

	//	//TODO factor the timing of the of configuration to work for different power setting.

	//ALB I kept the SPITX ldam transfer inside this function because i call it inside mod_som_efe_read_config_adc_f
	//ALB I call it 4 times for the 4 configuration adc register
	//	LDMA_TransferCfg_t spitx_init= LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXEMPTY);

	uint32_t delay;
	uint8_t j;

	uint8_t dummy_bytes[4];

	//
	config_cmd[0] = AD7124_COMM_WRITE | reg_address;
	for (int i=0;i<nb_bytes;i++){
		config_cmd[i+1] = (uint8_t) (reg_data>>(nb_bytes-i-1)*8);
	}

	read_cmd[0] = AD7124_COMM_READ | reg_address;

	//CS low;

	j=0;
	descriptor_link_config[j] = cs_assert;
	descriptor_link_config[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_config[j].wri.immVal=  oe_bit_mask;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.doneIfs=0;

	j=1;
	descriptor_link_config[j] = cs_assert;
	descriptor_link_config[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_config[j].wri.immVal= all_bit_mask;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.doneIfs=0;


	j=2;
	descriptor_link_config[j] = cs_assert;
	descriptor_link_config[j].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_config[j].wri.immVal=sensor->selector_cs_code;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.doneIfs=0;

	j=3;
	descriptor_link_config[j] = cs_assert;
	descriptor_link_config[j].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_config[j].wri.immVal=oe_bit_mask;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.doneIfs=0;

	// register
	j=4;
	descriptor_link_config[j] = adc_write;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t) (&config_cmd);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;
	descriptor_link_config[j].xfer.doneIfs=0;

	j=5;
	descriptor_link_config[j] = adc_write;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t) (&read_cmd);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].xfer.blockSize=ldmaCtrlBlockSizeUnit1;
	descriptor_link_config[j].xfer.structReq=0;
	descriptor_link_config[j].xfer.doneIfs=0;


	//TODO use this read to check is the config succeed. currently it is just a dummy read;
	j=6;
	descriptor_link_config[j] = adc_read;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t)(&dummy_bytes);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t)(&USART0->RXDATA);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].wri.doneIfs=0;

	//OE high;
	j=7;
	descriptor_link_config[j] = cs_deassert;
	descriptor_link_config[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_config[j].wri.immVal=sensor->selector_cs_code;
	descriptor_link_config[j].wri.structReq=0;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.doneIfs=0;

	j=8;
	descriptor_link_config[j] = single_cs_deassert;
	descriptor_link_config[j].wri.dstAddr= mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_config[j].wri.immVal=oe_bit_mask;
	descriptor_link_config[j].wri.structReq=0;
	descriptor_link_config[j].wri.doneIfs=0;


	LDMA_StartTransfer(ldma_ch ,(void*)&spitx_init, (void*)descriptor_link_config);
	delay = 0xFFFF;
	while (delay--); // Delay after power on to ensure all DMA transfer are done.
}

#else
void mod_som_efe_adc_config_register_f(uint8_t ldma_ch, sensor_spec_ptr_t sensor, uint8_t reg_address, uint32_t reg_data, uint8_t nb_bytes)
{

	//	//TODO factor the timing of the of configuration to work for different power setting.

	//ALB I kept the SPITX ldam transfer inside this function because i call it inside mod_som_efe_read_config_adc_f
	//ALB I call it 4 times for the 4 configuration adc register
	LDMA_Descriptor_t descriptor_link_config[MOD_SOM_EFE_LDMA_CONFIG_STEP];

	uint32_t delay;

	uint8_t dummy_bytes[4];
	uint8_t config_cmd[4]={0};
	uint8_t read_cmd[4]={0};

//
    config_cmd[0] = AD7124_COMM_WRITE | reg_address;
    read_cmd[0]   = AD7124_COMM_READ | reg_address;
    for (int i=0;i<nb_bytes;i++){
        config_cmd[i+1] = (uint8_t) (reg_data>>(nb_bytes-i-1)*8);
    }
    int j=0;
	//CS low;
	descriptor_link_config[j] = cs_assert;
	descriptor_link_config[j].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_clear_offset;
	descriptor_link_config[j].wri.immVal=1<<sensor->csLoc.gpioPin;
	descriptor_link_config[j].wri.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].wri.structReq=1;

	// register
	j=1;
	descriptor_link_config[j] = adc_write;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t) (&config_cmd);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_config[j].xfer.structReq=0;


	//TODO use this read to check is the config succeed. currently it is just a dummy read;
	j=2;
	descriptor_link_config[j] = adc_write;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t)(&USART0->TXDATA);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t) (&read_cmd);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_config[j].xfer.structReq=0;

	j=3;
	descriptor_link_config[j] = adc_read;
	descriptor_link_config[j].xfer.dstAddr=(uint32_t) (&dummy_bytes);
	descriptor_link_config[j].xfer.srcAddr=(uint32_t)(&USART0->RXDATA);
	descriptor_link_config[j].xfer.linkAddr=LDMA_1_STEP;
	descriptor_link_config[j].xfer.xferCnt=nb_bytes;
	descriptor_link_config[j].xfer.blockSize=ldmaCtrlBlockSizeUnit3;
	descriptor_link_config[j].xfer.structReq=0;

	//CS high;
	j=4;
	descriptor_link_config[j]=single_cs_deassert;
	descriptor_link_config[j].wri.dstAddr=mod_som_efe_ptr->config_ptr->gpio_a_base_address+bit_set_offset;
	descriptor_link_config[j].wri.immVal=1<<sensor->csLoc.gpioPin;
	descriptor_link_config[j].wri.structReq=1;

	LDMA_StartTransfer(ldma_ch ,(void*)&spitx_init, (void*)descriptor_link_config);
	delay = 0xFFFF;
	while (delay--); // Delay after power on to ensure all DMA transfer are done.
	USART0->CMD = USART_CMD_CLEARTX;

}
#endif

//ALB command functions
//SN edited to be functions called in shell
//SN need to edit comments
/*******************************************************************************
 * @brief
 *   raise stream flag for the 'efe_stream' command.
 ******************************************************************************/
mod_som_status_t mod_som_efe_consumer_mode_f(CPU_INT16U argc,CPU_CHAR *argv[])
{

  RTOS_ERR  p_err;

  if (argc==1){
      printf("efe.mode %u\r\n.", mod_som_efe_ptr->consumer_mode);
  }
  else{
    //ALB switch statement easy to handle all user input cases.
    switch (argc){
    case 2:
      mod_som_efe_ptr->consumer_mode=shellStrtol(argv[1],&p_err);
      break;
    default:
      printf("format: efe.mode mode (0:stream, 1:SD store, 2: on board processing)\r\n");
      break;
    }
  }

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}
/*******************************************************************************
 * @brief
 *   Shell command function for the 'efe_sample' command.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_sampling_f()
{
  //MHA: do not start if already started
  if (!mod_som_efe_ptr->sampling_flag) {

  // initialize the sample count.
  mod_som_efe_ptr->sample_count=0;

  //ALB turn on EFE hardware
  mod_som_efe_enable_hardware_f();

  mod_som_efe_config_adc_f(mod_som_efe_ptr);

  //MHA: need to enable the EFE MEZZ board for FCTD
  //    /* EFE Enable: configure the LEUART pins and EFE MEZZ EN (send power to the EFE MEZZ)*/
//    GPIO_PinModeSet(mod_som_efe_ptr->config_ptr->port.en_port, mod_som_sbe49_ptr->config_ptr->port.en_pin,
//                    gpioModePushPull, 1);
#ifdef MOD_SOM_FCTD_EN
  GPIO_PinModeSet(MOD_SOM_EFE_MEZZ_EN_PORT, MOD_SOM_EFE_MEZZ_EN_PIN,
                    gpioModePushPull, 1);
#endif

  switch (mod_som_efe_ptr->consumer_mode){
    case 2:
      //ALB on board processing do nothing
      break;
    default:
      mod_som_efe_start_consumer_task_f();
      break;
  }
  mod_som_efe_start_mclock_f(mod_som_efe_ptr);
  mod_som_efe_ptr->sampling_flag = 1;
  } // MHA end if block

	return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}

/*******************************************************************************
 * @brief
 *   Shell command function for the efe_stop command.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_stop_sampling_f()
{
  if (!mod_som_efe_ptr->sampling_flag) {
      return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
  }

  mod_som_status_t status = MOD_SOM_STATUS_OK;
//  RTOS_ERR  err;

	// Stop the timer drive the master clock controlling the ADCs

  switch (mod_som_efe_ptr->consumer_mode){
    case 2:
      //ALB on board processing do nothing
      break;
    default:
      status = mod_som_efe_stop_consumer_task_f();
      break;
  }

  mod_som_efe_stop_mclock_f(mod_som_efe_ptr);
	// lower all the pertinent flags
	mod_som_efe_ptr->sampling_flag = 0;
	mod_som_efe_ptr->data_ready_flag=0;

  //ALB turn off EFE hardware
  mod_som_efe_disable_hardware_f();

	//MHA kluge for FCTD
#ifdef MOD_SOM_FCTD_EN
  GPIO_PinModeSet(MOD_SOM_EFE_MEZZ_EN_PORT, MOD_SOM_EFE_MEZZ_EN_PIN,
                    gpioModePushPull, 0);
#endif

//  if(RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE)
//    return mod_som_efe_encode_status_f(MOD_SOM_EFE_STATUS_FAIL_TOP_SAMPLING);

	return status;
}


/*******************************************************************************
 * @brief
 *   Shell command function for the efe_sigramp.
 *
 ******************************************************************************/
mod_som_status_t mod_som_efe_sigramp_f()
{

  if (mod_som_efe_ptr->sigramp_flag==0){
      mod_som_efe_ptr->sigramp_flag=1;

  }else{
      mod_som_efe_ptr->sigramp_flag=0;
      descriptor_link_read1[mod_som_efe_ptr->ldma_spi_read_descriptor_offset].xfer.srcAddr=
                                                (uint32_t) (&USART0->RXDATA);
      descriptor_link_read1[mod_som_efe_ptr->ldma_spi_read_descriptor_offset].xfer.srcInc=ldmaCtrlSrcIncNone;

  }

  return mod_som_efe_encode_status_f(MOD_SOM_STATUS_OK);
}




/*******************************************************************************
 * ADC RDY interrupt handler, sends LDMA transfer to pull data from ADCs with CS cascade
 *******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  uint64_t tick;
  uint64_t * local_element;
	// ALB check if the we receive the correct GPIO interrupt. ADC intterupt is on MISO high.
	// ALB MISO is GPIO E 11.
	// ALB The interrupt handler is disable AFTER the check
	// ALB and re-enable once the LDMA transfer is done
	// ALB TODO get the timestamp here
	if (!GPIO_PinInGet(gpioPortE, 11))
	{
		//ALB disable interrupt handler
		GPIO_IntDisable(mod_som_efe_ptr->config_ptr->pin_interrupt_address);
		//ALB clear the interrupt flag
		GPIO_IntClear(mod_som_efe_ptr->config_ptr->pin_interrupt_address);

	  CORE_DECLARE_IRQ_STATE;
	  CORE_ENTER_ATOMIC();

		//ALB get timestamp here right before gathering ADC samples
    tick=sl_sleeptimer_get_tick_count64();
    mystatus = sl_sleeptimer_tick64_to_ms(tick,\
           &mod_som_efe_ptr->timestamp);

    //MHA: Now augment timestamp by poweron_offset_ms
    mod_som_calendar_settings=mod_som_calendar_get_settings_f(); //get the calendar settings pointer
    mod_som_efe_ptr->timestamp += mod_som_calendar_settings.poweron_offset_ms;

    if(mystatus != SL_STATUS_OK)
    {
      //ALB TODO handle get timestamps errors
    }

    //ALB copy the timestamp in the elements_buffer
    local_element=(uint64_t*)mod_som_efe_ptr->config_ptr->element_map[mod_som_efe_ptr->rec_buff->producer_indx];
    memcpy(local_element,(uint64_t*) &mod_som_efe_ptr->timestamp, sizeof(uint64_t));


		//ALB trigger LDMA transfer. Using channel 0. The channel is hardcoded.
		//ALB TODO abstract the choice of channel 0.
		//ALB Note: Interrupt handler enable inside the LDMA interrupt handler
		LDMA_StartTransfer(mod_som_efe_ptr->ldma.ch,
		            (void*)&spitx_init,
		            (void*)&descriptor_link_read1);
    CORE_EXIT_ATOMIC();
	}
}



/***************************************************************************//**
 * @brief
 *   LDMA IRQ handler. This interrupt is considered PRODUCER 0: The main producer of EFE data.
 *   Function of the handler
 *   This core should
 *   - Respond to hardware LDMA producer: When the read_descriptor1 is executed the LDMA send a interrupt signal.
 *   - Organize the data in a circular buffer. The element of circular buffer:
 *   	- sample count
 *   	- timestamp
 *   	- all ADC sample
 *   	- We are not adding a checksum here because It should be within a consumer store-stream scope
 *
 *   - Increment the sample count. A sample is samplecount-timestamp-all ADCsamples
 *   - Update the writing address of the circular buffer.
 *
 *   When should it try to reset?
 *   What error should it respond to?
 *   What flag are used?
 *   What status should it return?
 *
 *
 *    THIS IS THE ONLY LDMA INTERRUPT HANDLER!!!
 *    THIS HANDLER SHOULD BE AS SHORT-SIMPLE AS POSSIBLE!!!!
 *    We ll enter here when channel is done AND error.
 *    If we ever gone an error this would be an AHB bus error and no information is given on the type of error and what channel is it related to.
 *    - If error LDMA should be reset?
 *
 ******************************************************************************/
//void LDMA_IRQHandler( void )
void mod_som_efe_ldma_irq_handler_f( void )
{
	//ALB ch0_bit_position is hardcoded we should be abstracted in the run time pointer.
	//ALB TODO
  uint32_t descriptor_index;

	/* Check which LDMA channel is done*/
	//ALB TODO document the LDMA->IF. Channel 0 is bit position 1.
	if (mod_som_efe_ptr->sampling_flag==1)
	{

  	mod_som_efe_ptr->sample_count++;

		mod_som_efe_ptr->rec_buff->producer_indx=  mod_som_efe_ptr->sample_count % \
		    mod_som_efe_ptr->config_ptr->element_per_buffer;


		//ALB update data buffer address in   descriptor_link_read1
		//ALB TODO figure out a way to increment the addresses with LDMA. Am I being stupid right now.


    // update the address of the next ADC samples in the descriptor list.
		for (int i=0;i<mod_som_efe_ptr->settings_ptr->number_of_channels;i++)
		  {
		    descriptor_index=MOD_SOM_EFE_LDMA_READ_STEP*i+ mod_som_efe_ptr->ldma_spi_read_descriptor_offset;
		    descriptor_link_read1[descriptor_index].xfer.dstAddr= \
			    (uint32_t)(mod_som_efe_ptr->config_ptr->element_map[mod_som_efe_ptr->rec_buff->producer_indx]+ \
			                                        MOD_SOM_EFE_TIMESTAMP_LENGTH +\
			                                        (AD7124_SAMPLE_LENGTH*i));
		  }

		//ALB sigramp command: make fake data at sensor 0 location
		if(mod_som_efe_ptr->sigramp_flag){
		    uint32_t adc_sample=0;
		    //ALB debug
        fake_adc_data[0]=(uint8_t) (mod_som_efe_ptr->sample_count>> 16);
        fake_adc_data[1]=(uint8_t) (mod_som_efe_ptr->sample_count>> 8);
        fake_adc_data[2]=(uint8_t) (mod_som_efe_ptr->sample_count);
		    descriptor_link_read1[mod_som_efe_ptr->ldma_spi_read_descriptor_offset].xfer.srcAddr= \
		          (uint32_t) (&fake_adc_data);
        descriptor_link_read1[mod_som_efe_ptr->ldma_spi_read_descriptor_offset].xfer.srcInc=ldmaCtrlSrcIncOne;

		    for (int i=0;i<AD7124_SAMPLE_LENGTH;i++){
		        adc_sample=adc_sample<<8|
		            mod_som_efe_ptr->rec_buff->efe_elements_buffer[                \
		                                     ((mod_som_efe_ptr->sample_count-1) % \
		                                      mod_som_efe_ptr->config_ptr->element_per_buffer)*\
		                                     mod_som_efe_ptr->config_ptr->element_length+\
		                                     MOD_SOM_EFE_TIMESTAMP_LENGTH+i];
//		        adc_sample=adc_sample<<8;
		    }
//		    printf("\nsensor 0 %lu",adc_sample);
		}


		// enable interrupt
		GPIO_IntClear(mod_som_efe_ptr->config_ptr->pin_interrupt_address);
		GPIO_IntEnable(mod_som_efe_ptr->config_ptr->pin_interrupt_address);
	}
}






