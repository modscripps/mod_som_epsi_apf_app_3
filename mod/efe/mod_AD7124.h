/***************************************************************************//**
 *   @file   AD7124.h
 *   @brief  Header file of AD7124 Driver.
 *   @author DNechita (Dan.Nechita@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 903
*******************************************************************************/
#ifndef SRC_MOD_AD7124_H_
#define SRC_MOD_AD7124_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/



/******************************************************************************/
/******************************** Defines *************************************/
/******************************************************************************/

#define _24SWAPEND(x) ((x>>16)&0xff) | 			\
                    	((x<<16)&0xff0000) | 	\
                    	((x)&0xff00)

#define _16SWAPEND(x) ((x>>8)&0xff) | 			\
                    	((x<<8)&0xff00) 	\

#define LHI 1
#define LLO 0

#define LFLTR0 3
#define LCH0 2
#define LCTRL 2
#define LCFG0 2
#define LIOCTRL1 3

#define RESET_CYCLE 7;

#define MOD_SOM_EFE_SENSOR_STR_lENGTH 4

#define AD7124_RESET_DEFAULT                             	             	    \
  { 0x0,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8001,																		\
	0x860,																		\
	0x60180,																	\
	0x800000,																	\
	0x500000																	\
  }

//  describe the register
//	uint32_t config0=0x0860;   mod default (bipolar is the default)
//	uint32_t config_0=0x0060; (unipolar config for accelerometer)
//	uint32_t channel_0=0x8001; mod default
//	uint32_t channel_0=0x8210; temperature sensor

//	uint32_t filter0=0x06003c; 320 Hz
//	uint32_t adc_control=0x02c2; fullpower - adc continous conversion mode- external clock
//	uint32_t adc_control=0x0ac2; continuous read - fullpower - adc continous conversion mode- external clock
//	uint32_t adc_error_en=AD7124_ERREN_REG_REF_DET_ERR_EN | AD7124_ERREN_REG_SPI_IGNORE_ERR_EN; //3 bytes


#define MOD_AD7124_DEFAULT                             	                 	    \
  { 0x0,																		\
	0x0,																		\
	0x0ac2,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8001,																    	\
	0x860,																		\
	0x06003c,																	\
	0x800000,																	\
	0x500000																	\
  }

#define MOD_AD7124_ACCELL                             	                 	    \
  { 0x0,																		\
	0x0,																		\
	0x0882,																		\
	0x0,																		\
	0x0c0000,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8001,																    	\
	0x01e0,																		\
	0x06003c,																	\
	0x800000,																	\
	0x500000																	\
  }

//ALB ACCELX,Y,Z are used with EFE_REV_4
#define MOD_AD7124_ACCELLX                                                     \
  { 0x0,                                    \
  0x0,                                    \
  0x0882,                                   \
  0x0,                                    \
  0x0c0000,                                   \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x8001,                                     \
  0x01e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }
#define MOD_AD7124_ACCELLY                                                     \
  { 0x0,                                    \
  0x0,                                    \
  0x0882,                                   \
  0x0,                                    \
  0x880000,                                   \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x8001,                                     \
  0x01e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }
#define MOD_AD7124_ACCELLZ                                                     \
  { 0x0,                                    \
  0x0,                                    \
  0x0882,                                   \
  0x0,                                    \
  0x0,                                   \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x8001,                                     \
  0x01e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }

#define MOD_AD7124_SHEAR                             	                 	    \
  { 0x0,																		\
	0x0,																		\
	0x0882,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8001,																    	\
	0x09e0,																		\
	0x06003c,																	\
	0x800000,																	\
	0x500000																	\
  }

  //MHA add a disabled channel for shear and accel
 //MHA: Set control bits (3rd arg) for quiescent mode.  Disables the ADC.
#define MOD_AD7124_SHEAR_DISABLED                                                    \
  { 0x0,                                    \
  0x0,                                    \
  0xc,                                   \
  0x0,                                    \
  0x0,                                    \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x8001,                                     \
  0x09e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }

 //MHA DISABLED ACCEL CHANNEL
  //MHA: Set control bits (3rd arg) for quiescent mode.  Disables the ADC.
 #define MOD_AD7124_ACCELL_DISABLED                                                   \
   { 0x0,                                    \
   0x0,                                    \
   0xc,                                   \
   0x0,                                    \
   0x0c0000,                                   \
   0x0,                                    \
   0x2,                                    \
   0x0,                                    \
   0x40,                                   \
   0x0,                                    \
   0x8001,                                     \
   0x01e0,                                   \
   0x06003c,                                 \
   0x800000,                                 \
   0x500000                                  \
   }

//MHA fix settings 6/23/2021 - 8067 -> 80c7
//MHA 6/26/2021 Change bipolar to unipolar - 4th to last line, control0
#define MOD_AD7124_UCOND                                                    \
  { 0x0,                                    \
  0x0,                                    \
  0x0882,                                   \
  0x0,                                    \
  0x0,                                    \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x80c7,                                     \
  0x01e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }

//MHA fix settings 6/23/2021
//MHA 6/26/2021 Change bipolar to unipolar - 4th to last line, control0
#define MOD_AD7124_FLUO                                                   \
  { 0x0,                                    \
  0x0,                                    \
  0x0882,                                   \
  0x0,                                    \
  0x0,                                    \
  0x0,                                    \
  0x2,                                    \
  0x0,                                    \
  0x40,                                   \
  0x0,                                    \
  0x80c7,                                     \
  0x01e0,                                   \
  0x06003c,                                 \
  0x800000,                                 \
  0x500000                                  \
  }


#define MOD_AD7124_TEMP_DIFF                             	                 	    \
  { 0x0,																		\
	0x0,																		\
	0x0882,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8001,																    	\
	0x09e0,																		\
	0x06003c,																	\
	0x800000,																	\
	0x500000																	\
  }

#define MOD_AD7124_TEMP                             	                 	    \
  { 0x0,																		\
	0x0,																		\
	0x0882,																		\
	0x0,																		\
	0x0,																		\
	0x0,																		\
	0x2,																		\
	0x0,																		\
	0x40,																		\
	0x0,																		\
	0x8043,																    	\
	0x01e0,																		\
	0x06003c,																	\
	0x800000,																	\
	0x500000																	\
  }


typedef struct AD7124_REGISTERS {
	uint8_t 	COMMS;
	uint8_t		STATUS;
	uint16_t	ADC_CONTROL;
	uint32_t 	DATA;
	uint32_t 	IO_CONTROL_1;
	uint16_t	IO_CONTROL_2;
	uint8_t		ID;
	uint32_t	ERROR;
	uint32_t	ERROR_EN;
	uint8_t		MCLK_COUNT;
	uint16_t	CHANNEL_0;
	uint16_t	CONFIG_0;
	uint32_t	FILTER_0;
	uint32_t	OFFSET_0;
	uint32_t	GAIN_0;
} AD7124;

typedef struct {
	uint8_t gpioPort;
	uint8_t gpioPin;
} csLocation;

typedef struct sensor_spec_t{
	char			 	name[MOD_SOM_EFE_SENSOR_STR_lENGTH]; 		// Sensor Name/ID
	char			 	sn[MOD_SOM_EFE_SENSOR_STR_lENGTH]; 		    // Sensor serial Number
	float		    	cal; 		    // cal is the calibration coefficient
	AD7124 				registers; 		// Software register emulators
#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
	uint32_t			selector_cs_code; 			// Location
#else
	csLocation			csLoc; 			// Location
#endif
} sensor_spec_t, *sensor_spec_ptr_t;

/******************************************************************************/
/************************ external variable       *****************************/
/******************************************************************************/
#define AD7124_SAMPLE_LENGTH 3

/******************************************************************************/
/************************ AD7124 Register Defines *****************************/
/******************************************************************************/

/* AD7124 Register Map */
#define AD7124_REG_COMM			0x00		// Communications Register 					(WO, 8-bit)
#define AD7124_REG_STATUS		0x00 		// Status Register         					(RO, 8-bit)
#define AD7124_REG_CTRL			0x01 		// Ctrl Register           					(RW, 16-bit)
#define AD7124_REG_DATA			0x02 		// Data Register							(RO, 24-bit)
#define AD7124_REG_IOCTRL1		0x03 		// I/O Control 1 Register					(RW, 24-bit)
#define AD7124_REG_IOCTRL2		0x04 		// I/O Control 2 Register   				(RW, 16-bit)
#define AD7124_REG_ID			0x05 		// ID Register             					(R, 8-bit)
#define AD7124_REG_ERROR		0x06 		// Error Register        					(RW, 24-bit)
#define AD7124_REG_ERROREN		0x07 		// Error Enable Register 					(RW, 24-bit)
#define AD7124_REG_MCLK			0x08		// MCLK Register							(R, 8-bit)
#define AD7124_REG_CH0			0x09		// Ch 0 Register							(RW, 16-bit)
#define AD7124_REG_CH1			0x0A		// Ch 1 Register							(RW, 16-bit)
#define AD7124_REG_CFG0			0x19		// Ch 0 Cfg Register						(RW, 16-bit)
#define AD7124_REG_CFG1			0x1A		// Ch 1 Cfg Register						(RW, 16-bit)
#define AD7124_REG_FLTR0		0x21		// Ch 0 Filter Register						(RW, 24-bit)
#define AD7124_REG_FLTR1		0x22		// Ch 1 Filter Register						(RW, 24-bit)
#define AD7124_REG_CH0OFFSET	0x29		// Ch 0 Offset Register						(RW, 24-bit)
#define AD7124_REG_CH1OFFSET	0x2A		// Ch 1 Offset Register						(RW, 24-bit)
#define AD7124_REG_GAIN0		0x31		// Ch 0 Gain Register						(RW, 24-bit)
#define AD7124_REG_GAIN1		0x32		// Ch 1 Gain Register						(RW, 24-bit)

/* TODO:  Communications Register Bit Designations (AD7124_REG_COMM) setup like ADC CTRL*/
#define AD7124_COMM_WRITE			(0x0 << 6)						// Write Operation.
#define AD7124_COMM_READ			(0x1 << 6)						// Read Operation.

/* TODO:  Status Register Bit Designations (AD7124_REG_STAT) setup like ADC CTRL*/
#define AD7124_STATUS_RDY				(1 << 7)			// Ready.
#define AD7124_STATUS_ERROR				(1 << 6)			// ADC error bit.
#define AD7124_STATUS_POR				(1 << 4)			// Power-on Reset Flag
#define AD7124_STATUS_CH_START_POSITION			(0xF)				// ADC Status Channel Mask
#define AD7124_STATUS_REG_CH_ACTIVE(x) 	((x) & 0xF)			// Quick macro for checking active CH
#define AD7124_STATUS_CH(x)				(x)					// Channel 0-15.

/* Adc Control Register */
#define AD7124_CTRL_DEFAULT_MASK			(0x0000)	//Valid bits 0:12

#define AD7124_CTRL_DOUT_RDY_DELAY_START_POSITION		12  //Increase delay to 100ns for last SCK to RDY high time
#define AD7124_CTRL_DOUT_RDY_DELAY_NUM_BITS	1
#define AD7124_CTRL_DOUT_RDY_DELAY			1
#define AD7124_CTRL_DOUT_RDY_NO_DELAY		0

#define AD7124_CTRL_CONT_READ_START_POSITION			11  //Enabling removes the need for a MOSI address before
#define AD7124_CTRL_CONT_READ_NUM_BITS		1	//clocking out data
#define AD7124_CTRL_CONT_READ_ENABLE		1
#define AD7124_CTRL_CONT_READ_DISABLE		0

#define AD7124_CTRL_DATA_STATUS_START_POSITION		10  //Output status after each data read
#define AD7124_CTRL_DATA_STATUS_NUM_BITS	1
#define AD7124_CTRL_DATA_STATUS_ENABLE		1
#define AD7124_CTRL_DATA_STATUS_DISABLE		0

#define AD7124_CTRL_CS_EN_START_POSITION				9   //Enabling keeps DOUT from changing to RDY logic after
#define AD7124_CTRL_CS_EN_NUM_BITS			1	//last SCK with CS still low
#define AD7124_CTRL_CS_EN_ENABLE			1   //This is useful for diagnostic error output
#define AD7124_CTRL_CS_EN_DISABLE			0

#define AD7124_CTRL_REF_EN_START_POSITION				8   //Enable to turn on internal AVdd reference
#define AD7124_CTRL_REF_EN_NUM_BITS			1	//and provide reference on REFOUT pin
#define AD7124_CTRL_REF_EN_ENABLE			1
#define AD7124_CTRL_REF_EN_DISABLE			0

#define AD7124_CTRL_POWER_MODE_START_POSITION			6
#define AD7124_CTRL_POWER_MODE_NUM_BITS		2
#define AD7124_CTRL_POWER_MODE_HI			3
#define AD7124_CTRL_POWER_MODE_HIGH			2	//We use 2, but ..._HI works the same, I think
#define AD7124_CTRL_POWER_MODE_MID			1
#define AD7124_CTRL_POWER_MODE_LOW			0

#define AD7124_CTRL_MODE_START_POSITION				2	//Conversion, Calibration, Power-down modes
#define AD7124_CTRL_MODE_NUM_BITS			4
#define AD7124_CTRL_MODE_CONTINUOUS			0	//Continuous (default)
#define AD7124_CTRL_MODE_SINGLE				1	//Single Conversion
#define AD7124_CTRL_MODE_STANDBY			2	//Standby
#define AD7124_CTRL_MODE_POWER_DOWN			3	//Power-down
												//TODO: all the other modes of operation
#define AD7124_CTRL_CLKSEL_START_POSITION				0
#define AD7124_CTRL_CLKSEL_NUM_BITS			2
#define AD7124_CTRL_CLKSEL_INT				0	//Internal 614kHz clock, no clk out
#define AD7124_CTRL_CLKSEL_INT_OUT			1	//Internal 614kHz clock, clk pin out
#define AD7124_CTRL_CLKSEL_EXT				2	//External 614kHz clock (make sure its between 585 and 630KHz)
#define AD7124_CTRL_CLKSEL_EXT_DIV4			3	//External clock gets internally divided by 4

/* TODO: IO_Control_1 Register bits defined like AD7124 CTRL */
#define AD7124_IO_CTRL1_GPIO_DAT2     (1 << 23)
#define AD7124_IO_CTRL1_GPIO_DAT1     (1 << 22)
#define AD7124_IO_CTRL1_GPIO_CTRL2    (1 << 19)
#define AD7124_IO_CTRL1_GPIO_CTRL1    (1 << 18)
#define AD7124_IO_CTRL1_PDSW          (1 << 15)
#define AD7124_IO_CTRL1_IOUT1(x)      (((x) & 0x7) << 11)
#define AD7124_IO_CTRL1_IOUT0(x)      (((x) & 0x7) << 8)
#define AD7124_IO_CTRL1_IOUT_CH1(x)   (((x) & 0xF) << 4)
#define AD7124_IO_CTRL1_IOUT_CH0(x)   (((x) & 0xF) << 0)

/* TODO: IO_Control_2 Register bits defined like AD7124 CTRL */
#define AD7124_IO_CTRL2_GPIO_VBIAS7   (1 << 15)
#define AD7124_IO_CTRL2_GPIO_VBIAS6   (1 << 14)
#define AD7124_IO_CTRL2_GPIO_VBIAS5   (1 << 11)
#define AD7124_IO_CTRL2_GPIO_VBIAS4   (1 << 10)
#define AD7124_IO_CTRL2_GPIO_VBIAS3   (1 << 5)
#define AD7124_IO_CTRL2_GPIO_VBIAS2   (1 << 4)
#define AD7124_IO_CTRL2_GPIO_VBIAS1   (1 << 1)
#define AD7124_IO_CTRL2_GPIO_VBIAS0   (1 << 0)

/* TODO: ID Register bits defined like AD7124 CTRL */
#define AD7124_ID_REG_DEVICE_ID(x)   (((x) & 0xF) << 4)
#define AD7124_ID_REG_SILICON_REV(x) (((x) & 0xF) << 0)

/* TODO: Error Register bits defined like AD7124 CTRL */
#define AD7124_ERR_REG_LDO_CAP_ERR        (1 << 19)
#define AD7124_ERR_REG_ADC_CAL_ERR        (1 << 18)
#define AD7124_ERR_REG_ADC_CONV_ERR       (1 << 17)
#define AD7124_ERR_REG_ADC_SAT_ERR        (1 << 16)
#define AD7124_ERR_REG_AINP_OV_ERR        (1 << 15)
#define AD7124_ERR_REG_AINP_UV_ERR        (1 << 14)
#define AD7124_ERR_REG_AINM_OV_ERR        (1 << 13)
#define AD7124_ERR_REG_AINM_UV_ERR        (1 << 12)
#define AD7124_ERR_REG_REF_DET_ERR        (1 << 11)
#define AD7124_ERR_REG_DLDO_PSM_ERR       (1 << 9)
#define AD7124_ERR_REG_ALDO_PSM_ERR       (1 << 7)
#define AD7124_ERR_REG_SPI_IGNORE_ERR     (1 << 6)
#define AD7124_ERR_REG_SPI_SLCK_CNT_ERR   (1 << 5)
#define AD7124_ERR_REG_SPI_READ_ERR       (1 << 4)
#define AD7124_ERR_REG_SPI_WRITE_ERR      (1 << 3)
#define AD7124_ERR_REG_SPI_CRC_ERR        (1 << 2)
#define AD7124_ERR_REG_MM_CRC_ERR         (1 << 1)

/* TODO: Error_En Register bits defined like AD7124 CTRL */
#define AD7124_ERREN_REG_MCLK_CNT_EN           (1 << 22)
#define AD7124_ERREN_REG_LDO_CAP_CHK_TEST_EN   (1 << 21)
#define AD7124_ERREN_REG_LDO_CAP_CHK(x)        (((x) & 0x3) << 19)
#define AD7124_ERREN_REG_ADC_CAL_ERR_EN        (1 << 18)
#define AD7124_ERREN_REG_ADC_CONV_ERR_EN       (1 << 17)
#define AD7124_ERREN_REG_ADC_SAT_ERR_EN        (1 << 16)
#define AD7124_ERREN_REG_AINP_OV_ERR_EN        (1 << 15)
#define AD7124_ERREN_REG_AINP_UV_ERR_EN        (1 << 14)
#define AD7124_ERREN_REG_AINM_OV_ERR_EN        (1 << 13)
#define AD7124_ERREN_REG_AINM_UV_ERR_EN        (1 << 12)
#define AD7124_ERREN_REG_REF_DET_ERR_EN        (1 << 11)
#define AD7124_ERREN_REG_DLDO_PSM_TRIP_TEST_EN (1 << 10)
#define AD7124_ERREN_REG_DLDO_PSM_ERR_ERR      (1 << 9)
#define AD7124_ERREN_REG_ALDO_PSM_TRIP_TEST_EN (1 << 8)
#define AD7124_ERREN_REG_ALDO_PSM_ERR_EN       (1 << 7)
#define AD7124_ERREN_REG_SPI_IGNORE_ERR_EN     (1 << 6)
#define AD7124_ERREN_REG_SPI_SCLK_CNT_ERR_EN   (1 << 5)
#define AD7124_ERREN_REG_SPI_READ_ERR_EN       (1 << 4)
#define AD7124_ERREN_REG_SPI_WRITE_ERR_EN      (1 << 3)
#define AD7124_ERREN_REG_SPI_CRC_ERR_EN        (1 << 2)
#define AD7124_ERREN_REG_MM_CRC_ERR_EN         (1 << 1)

/* Channel Register: AD7124_CONF_CHAN(x) options */
#define AD7124_CH0_DEFAULT_MASK			(0x8000)	//Valid bits 15:0
													//Bit15 CH0 defaults ON, other CH Bit15 default OFF

#define AD7124_CH_EN_START_POSITION					15		//Enables the channel setup
#define AD7124_CH_EN_NUM_BITS				1
#define AD7124_CH_ENABLE					1
#define AD7124_CH_DISABLE					0

#define AD7124_CH_SETUP_START_POSITION				12		//Chooses the Channel Setup number
#define AD7124_CH_SETUP_NUM_BITS			3
#define AD7124_CH_SETUP_0					0		//SETUP 1 of 8
#define AD7124_CH_SETUP_1					1		//SETUP 2 of 8
#define AD7124_CH_SETUP_2					2		//SETUP 3 of 8
#define AD7124_CH_SETUP_3					3		//SETUP 4 of 8
#define AD7124_CH_SETUP_4					4		//SETUP 5 of 8
#define AD7124_CH_SETUP_5					5		//SETUP 6 of 8
#define AD7124_CH_SETUP_6					6		//SETUP 7 of 8
#define AD7124_CH_SETUP_7					7		//SETUP 8 of 8

#define AD7124_CH_AINP_START_POSITION					5
#define AD7124_CH_AINP_NUM_BITS				5
#define AD7124_CH_AINP_AIN0					0
#define AD7124_CH_AINP_AIN1					1
#define AD7124_CH_AINP_INT_REF				18
#define AD7124_CH_AINP_TEMP_SENSOR			16
//TODO: remaining channels

#define AD7124_CH_AINM_START_POSITION					0
#define AD7124_CH_AINM_NUM_BITS				5
#define AD7124_CH_AINM_AIN1					1
#define AD7124_CH_AINM_AVSS					17
#define AD7124_CH_AINM_TEMP_SENSOR			16
//TODO: remaining channels

/* Configuration Register Bit Designations (AD7124_REG_CONF) */
#define AD7124_CONFIG_DEFAULT_MASK			(0x0860)  		//Bits 11:0 are valid

#define AD7124_CONFIG_BIPOLAR_START_POSITION			11		// Bipolar/Unipolar on/off.
#define AD7124_CONFIG_BIPOLAR_NUM_BITS		1
#define AD7124_CONFIG_BIPOLAR_ENABLE		1
#define AD7124_CONFIG_BIPOLAR_DISABLE		0		// Unipolar

#define AD7124_CONFIG_BURNOUT_START_POSITION			9
#define AD7124_CONFIG_BURNOUT_NUM_BITS		2
#define AD7124_CONFIG_BURNOUT_OFF      		0		// off
#define AD7124_CONFIG_BURNOUT_05UA      	1		// 0.5uA
#define AD7124_CONFIG_BURNOUT_2UA      		2		// 2uA
#define AD7124_CONFIG_BURNOUT_4UA      		3		// 4uA

#define AD7124_CONFIG_REF_BUFP_START_POSITION			8		// Buffer ref positive input on/off
#define AD7124_CONFIG_REF_BUFP_NUM_BITS		1
#define AD7124_CONFIG_REF_BUFP_ON			1
#define AD7124_CONFIG_REF_BUFP_OFF			0

#define AD7124_CONFIG_REF_BUFM_START_POSITION			7		// Buffer ref negative input on/off
#define AD7124_CONFIG_REF_BUFM_NUM_BITS		1
#define AD7124_CONFIG_REF_BUFM_ON			1
#define AD7124_CONFIG_REF_BUFM_OFF			0

#define AD7124_CONFIG_AIN_BUFP_START_POSITION			6		// Analog In Buffer ref positive input on/off
#define AD7124_CONFIG_AIN_BUFP_NUM_BITS		1
#define AD7124_CONFIG_AIN_BUFP_ON			1
#define AD7124_CONFIG_AIN_BUFP_OFF			0

#define AD7124_CONFIG_AIN_BUFM_START_POSITION			5		// Analog In Buffer ref negative input on/off
#define AD7124_CONFIG_AIN_BUFM_NUM_BITS		1
#define AD7124_CONFIG_AIN_BUFM_ON			1
#define AD7124_CONFIG_AIN_BUFM_OFF			0

#define AD7124_CONFIG_REF_SEL_START_POSITION			3		// Analog In Buffer ref negative input on/off
#define AD7124_CONFIG_REF_SEL_NUM_BITS		2		// Analog In Buffer ref negative input on/off
#define AD7124_CONFIG_REF_SEL1				0		// Ref 1 Select
#define AD7124_CONFIG_REF_SEL2				1		// Ref 2 Select
#define AD7124_CONFIG_INT_REF				2		// Internal Ref
#define AD7124_CONFIG_REF_AVDD				3		// AVdd

#define AD7124_CONFIG_PGA_START_POSITION				0		// Analog In Buffer ref negative input on/off
#define AD7124_CONFIG_PGA_NUM_BITS			3		// Analog In Buffer ref negative input on/off
#define AD7124_CONFIG_PGA_GAIN_1			0 		// Gain 1
#define AD7124_CONFIG_PGA_GAIN_2			1 		// Gain 2
#define AD7124_CONFIG_PGA_GAIN_4			2 		// Gain 4
#define AD7124_CONFIG_PGA_GAIN_8			3 		// Gain 8
#define AD7124_CONFIG_PGA_GAIN_16			4 		// Gain 16
#define AD7124_CONFIG_PGA_GAIN_32			5 		// Gain 32
#define AD7124_CONFIG_PGA_GAIN_64			6 		// Gain 64
#define AD7124_CONFIG_PGA_GAIN_128			7 		// Gain 128

/* TODO: Filter Register 0-7 bits like AD7124 */

#define AD7124_FILT_REG_FILTER(x)         (((x) & 0x7) << 21)
#define AD7124_FILT_REG_REJ60             (1 << 20)
#define AD7124_FILT_REG_POST_FILTER(x)    (((x) & 0x7) << 17)
#define AD7124_FILT_REG_SINGLE_CYCLE      (1 << 16)
#define AD7124_FILT_REG_FS(x)             (((x) & 0x7FF) << 0)


#endif /* SRC_MOD_AD7124_H_ */
