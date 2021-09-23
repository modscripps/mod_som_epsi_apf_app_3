/*
 * mod_efe_ldma.h
 *
 *  Created on: Jan 13, 2020
 *      Author: aleboyer
 */

#ifndef SRC_MOD_EFE_LDMA_H_
#define SRC_MOD_EFE_LDMA_H_

#include "em_ldma.h"
//TODO these are examples of private data and should be scoped as such MAG 7AUG2020

// define variables for LDMA number of bytes to be read and movement down linked list
// these are here just for readability later
#define LDMA_1_STEP             4   //1*4 for relative addressing of the next LDMA descriptor
#define LDMA_1_BYTE             0   // 1-1 transfer 1 bytes through LDMA
#define LDMA_3_BYTES            2   // 3-1 transfer 3 bytes through LDMA
#define LDMA_4_BYTES            3   // 4-1 transfer 4 bytes through LDMA
#define LDMA_RESET_BYTES		5

#if defined(MOD_SOM_EFE_REV3)|defined(MOD_SOM_EFE_REV4)
//define the number of step needed to read the adc data register with LDMA
#define MOD_SOM_EFE_LDMA_READ_STEP 6 //phases per channel for LDMA read in continuous data mode
#define TRAILING_STEP (2+2) //TODO MAG &AUG2020 phases per trailing part of LDMA read in continuous data mode. We might add two more to fetch the 64 bit time stamp
#define MOD_SOM_EFE_LDMA_CONFIG_STEP 9
#define MOD_SOM_EFE_LDMA_READ_CONFIG_STEP 5
#define MOD_SOM_EFE_LDMA_RESET_STEP 5

#else

#define MOD_SOM_EFE_LDMA_READ_STEP 5 //phases per channel for LDMA read in continuous data mode
#define TRAILING_STEP 1 //TODO MAG &AUG2020 phases per trailing part of LDMA read in continuous data mode. We might add two more to fetch the 64 bit time stamp
#define MOD_SOM_EFE_LDMA_CONFIG_STEP 6
#define MOD_SOM_EFE_LDMA_READ_CONFIG_STEP 5
#define MOD_SOM_EFE_LDMA_RESET_STEP 5

#endif



// dummy bytes to use only for LDMA timing
//volatile uint8_t dummy_bytes[4];


#endif /* SRC_MOD_EFE_LDMA_H_ */
