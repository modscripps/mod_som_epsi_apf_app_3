/*******************************************************************************
 * @file mod_som_bsp.h
 * @brief Board support package for MOD SOM
 * @date Mar 6, 2020
 * @author Riley Baird (rbaird@ucsd.edu)
 *         Multiscale Ocean Dynamics - SIO - UCSD
 * @author Sean Lastuka (slastuka@ucsd.edu)
 *         Multiscale Ocean Dynamics - SIO - UCSD
 *
 * @description
 * Board support package for MOD SOM
 * Table 1 -
 * Table 2 -
 * Table 3 -
 *
 * This work is intended to support research, teaching, and private study.
 * Use of this work beyond that allowed by "fair use" or any license applied to
 * this work requires written permission of the Multiscale Ocean Dynamics group
 * at the Scripps Institution of Oceanography, UCSD. Responsibility for
 * obtaining permissions and any use and distribution of this work rests
 * exclusively with the user.
 ******************************************************************************/


#ifndef SRC_MOD_SOM_BSP_H_
#define SRC_MOD_SOM_BSP_H_

#define MOD_SOM_REVISION 3


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Table 1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                  */
/*                          Defines pin mapping                     */
/*                                                                  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


// Main Connector J1
//                              (U1_92)
#define MOD_SOM_J1_3_PORT       gpioPortE   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J1_3_PIN        8
//                              (U1_93)
#define MOD_SOM_J1_4_PORT       gpioPortE   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J1_4_PIN        9
//                              (U1_55)
#define MOD_SOM_J1_6_PORT       gpioPortC   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J1_6_PIN        6
//                              (U1_51)
#define MOD_SOM_J1_9_PORT       gpioPortD   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J1_9_PIN        5
//                              (U1_50)
#define MOD_SOM_J1_10_PORT      gpioPortD   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J1_10_PIN       4



// Auxiliary Connector J2
//                              (U1_89)
#define MOD_SOM_J2_3_PORT       gpioPortD   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J2_3_PIN        10
//                              (U1_88)
#define MOD_SOM_J2_4_PORT       gpioPortD   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J2_4_PIN        9
//                              (U1_90)
#define MOD_SOM_J2_5_PORT       gpioPortD   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J2_5_PIN        11
//                              (U1_91)
#define MOD_SOM_J2_6_PORT       gpioPortD   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_J2_6_PIN        12



// Program & Debug Connector J8
//                              (U1_77)
#define MOD_SOM_J8_2_PORT       gpioPortF
#define MOD_SOM_J8_2_PIN        1
//                              (U1_76)
#define MOD_SOM_J8_4_PORT       gpioPortF
#define MOD_SOM_J8_4_PIN        0
//                              (U1_78)
#define MOD_SOM_J8_6_PORT       gpioPortF
#define MOD_SOM_J8_6_PIN        2



// Mezzanine Header J9
//                              (U1_96)
#define MOD_SOM_J9_2_PORT       gpioPortE
#define MOD_SOM_J9_2_PIN        12
//                              (U1_97)
#define MOD_SOM_J9_4_PORT       gpioPortE
#define MOD_SOM_J9_4_PIN        13
//                              (U1_9)
#define MOD_SOM_J9_5_PORT       gpioPortB
#define MOD_SOM_J9_5_PIN        0
//                              (U1_24)
#define MOD_SOM_J9_6_PORT       gpioPortB   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_6_PIN        7
//                              (U1_10)
#define MOD_SOM_J9_7_PORT       gpioPortB
#define MOD_SOM_J9_7_PIN        1
//                              (U1_63)
#define MOD_SOM_J9_8_PORT       gpioPortE   // See secondary pin under Analog Header J11, U16, and U27.
#define MOD_SOM_J9_8_PIN        3
//                              (U1_14)
#define MOD_SOM_J9_10_PORT      gpioPortB   // See secondary pin under Q4 and U27.
#define MOD_SOM_J9_10_PIN       5
//                              (U1_22)
#define MOD_SOM_J9_11_PORT      gpioPortC
#define MOD_SOM_J9_11_PIN       4
//                              (U1_23)
#define MOD_SOM_J9_13_PORT      gpioPortC
#define MOD_SOM_J9_13_PIN       5
//                              (U1_48)
#define MOD_SOM_J9_14_PORT      gpioPortD
#define MOD_SOM_J9_14_PIN       2
//                              (U1_28)
#define MOD_SOM_J9_15_PORT      gpioPortA
#define MOD_SOM_J9_15_PIN       9
//                              (U1_49)
#define MOD_SOM_J9_16_PORT      gpioPortD
#define MOD_SOM_J9_16_PIN       3
//                              (U1_29)
#define MOD_SOM_J9_17_PORT      gpioPortA
#define MOD_SOM_J9_17_PIN       10
//                              (U1_55)
#define MOD_SOM_J9_20_PORT      gpioPortC   // See secondary pin under Main Connector J1.
#define MOD_SOM_J9_20_PIN       6
//                              (U1_37)
#define MOD_SOM_J9_21_PORT      gpioPortB
#define MOD_SOM_J9_21_PIN       9
//                              (U1_30)
#define MOD_SOM_J9_22_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_22_PIN       11
//                              (U1_38)
#define MOD_SOM_J9_23_PORT      gpioPortB
#define MOD_SOM_J9_23_PIN       10
//                              (U1_18)
#define MOD_SOM_J9_24_PORT      gpioPortC
#define MOD_SOM_J9_24_PIN       0
//                              (U1_66)
#define MOD_SOM_J9_26_PORT      gpioPortE
#define MOD_SOM_J9_26_PIN       6
//                              (U1_67)
#define MOD_SOM_J9_27_PORT      gpioPortE
#define MOD_SOM_J9_27_PIN       7
//                              (U1_21)
#define MOD_SOM_J9_28_PORT      gpioPortC
#define MOD_SOM_J9_28_PIN       3
//                              (U1_53)
#define MOD_SOM_J9_29_PORT      gpioPortD
#define MOD_SOM_J9_29_PIN       7
//                              (U1_64)
#define MOD_SOM_J9_31_PORT      gpioPortE
#define MOD_SOM_J9_31_PIN       4
//                              (U1_68)
#define MOD_SOM_J9_32_PORT      gpioPortC   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_32_PIN       8
//                              (U1_61)
#define MOD_SOM_J9_35_PORT      gpioPortE
#define MOD_SOM_J9_35_PIN       1
//                              (U1_19)
#define MOD_SOM_J9_36_PORT      gpioPortC
#define MOD_SOM_J9_36_PIN       1
//                              (U1_60)
#define MOD_SOM_J9_37_PORT      gpioPortE
#define MOD_SOM_J9_37_PIN       0
//                              (U1_47)
#define MOD_SOM_J9_39_PORT      gpioPortD
#define MOD_SOM_J9_39_PIN       1
//                              (U1_20)
#define MOD_SOM_J9_40_PORT      gpioPortC
#define MOD_SOM_J9_40_PIN       2
//                              (U1_50)
#define MOD_SOM_J9_41_PORT      gpioPortD   // See secondary pin under Main Connector J1.
#define MOD_SOM_J9_41_PIN       4
//                              (U1_27)
#define MOD_SOM_J9_42_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_42_PIN       8
//                              (U1_100)
#define MOD_SOM_J9_44_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_44_PIN       15
//                              (U1_51)
#define MOD_SOM_J9_45_PORT      gpioPortD   // See secondary pin under Main Connector J1.
#define MOD_SOM_J9_45_PIN       5
//                              (U1_35)
#define MOD_SOM_J9_46_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_46_PIN       14
//                              (U1_5)
#define MOD_SOM_J9_47_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_47_PIN       4
//                              (U1_69)
#define MOD_SOM_J9_50_PORT      gpioPortC   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_50_PIN       9
//                              (U1_6)
#define MOD_SOM_J9_51_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_51_PIN       5
//                              (U1_11)
#define MOD_SOM_J9_52_PORT      gpioPortB   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_52_PIN       2
//                              (U1_26)
#define MOD_SOM_J9_53_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_53_PIN       7
//                              (U1_70)
#define MOD_SOM_J9_56_PORT      gpioPortC   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_56_PIN       10          // Connected to PE5 (U1_65)!
//                              (U1_65)
#define MOD_SOM_J9_56_INT_PORT  gpioPortE   // See secondary pin under Analog Header J11.   // TODO: Naming convention
#define MOD_SOM_J9_56_INT_PIN   5           // Connected to PC10 (U1_70)!                   // TODO: Naming convention
//                              (U1_33)
#define MOD_SOM_J9_57_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_57_PIN       12
//                              (U1_71)
#define MOD_SOM_J9_58_PORT      gpioPortC   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_58_PIN       11
//                              (U1_34)
#define MOD_SOM_J9_59_PORT      gpioPortA   // See secondary pin under Analog Header J11.
#define MOD_SOM_J9_59_PIN       13



// Mezzanine Header J10
//                              (U1_93)
#define MOD_SOM_J10_6_PORT      gpioPortE   // See secondary pin under Main Connector J1.
#define MOD_SOM_J10_6_PIN       9
//                              (U1_3)
#define MOD_SOM_J10_7_PORT      gpioPortA   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_7_PIN       2
//                              (U1_92)
#define MOD_SOM_J10_8_PORT      gpioPortE   // See secondary pin under Main Connector J1.
#define MOD_SOM_J10_8_PIN       8
//                              (U1_4)
#define MOD_SOM_J10_9_PORT      gpioPortA   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_9_PIN       3
//                              (U1_39)
#define MOD_SOM_J10_10_PORT     gpioPortB
#define MOD_SOM_J10_10_PIN      11
//                              (U1_42)
#define MOD_SOM_J10_12_PORT     gpioPortB   // See secondary pin under U11.
#define MOD_SOM_J10_12_PIN      13
//                              (U1_99)
#define MOD_SOM_J10_13_PORT     gpioPortE   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_13_PIN      15
//                              (U1_89)
#define MOD_SOM_J10_14_PORT     gpioPortD   // See secondary pin under Connector J2.
#define MOD_SOM_J10_14_PIN      10
//                              (U1_98)
#define MOD_SOM_J10_15_PORT     gpioPortE   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_15_PIN      14
//                              (U1_88)
#define MOD_SOM_J10_16_PORT     gpioPortD   // See secondary pin under Connector J2.
#define MOD_SOM_J10_16_PIN      9
//                              (U1_90)
#define MOD_SOM_J10_18_PORT     gpioPortD   // See secondary pin under Connector J2.
#define MOD_SOM_J10_18_PIN      11
//                              (U1_2)
#define MOD_SOM_J10_19_PORT     gpioPortA   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_19_PIN      1
//                              (U1_91)
#define MOD_SOM_J10_20_PORT     gpioPortD   // See secondary pin under Connector J2.
#define MOD_SOM_J10_20_PIN      12
//                              (U1_1)
#define MOD_SOM_J10_21_PORT     gpioPortA   // See secondary pin under SD Card Slot.
#define MOD_SOM_J10_21_PIN      0
//                              (U1_86)
#define MOD_SOM_J10_22_PORT     gpioPortF
#define MOD_SOM_J10_22_PIN      8
//                              (U1_84)
#define MOD_SOM_J10_24_PORT     gpioPortF
#define MOD_SOM_J10_24_PIN      6
//                              (U1_94)
#define MOD_SOM_J10_25_PORT     gpioPortE   // See secondary pin under U4 and U8.
#define MOD_SOM_J10_25_PIN      10
//                              (U1_85)
#define MOD_SOM_J10_26_PORT     gpioPortF
#define MOD_SOM_J10_26_PIN      7
//                              (U1_13)
#define MOD_SOM_J10_27_PORT     gpioPortB   // See secondary pin under U28.
#define MOD_SOM_J10_27_PIN      4
//                              (U1_87)
#define MOD_SOM_J10_28_PORT     gpioPortF
#define MOD_SOM_J10_28_PIN      9
//                              (U1_81)
#define MOD_SOM_J10_30_PORT     gpioPortF
#define MOD_SOM_J10_30_PIN      5
//                              (U1_80)
#define MOD_SOM_J10_32_PORT     gpioPortF
#define MOD_SOM_J10_32_PIN      12
//                              (U1_12)
#define MOD_SOM_J10_35_PORT     gpioPortB
#define MOD_SOM_J10_35_PIN      3
//                              (U1_75)
#define MOD_SOM_J10_36_PORT     gpioPortF
#define MOD_SOM_J10_36_PIN      11
//                              (U1_74)
#define MOD_SOM_J10_38_PORT     gpioPortF
#define MOD_SOM_J10_38_PIN      10



// Analog Header J11
//                              (U1_24)
#define MOD_SOM_J11_3_PORT      gpioPortB   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_3_PIN       7
//                              (U1_63)
#define MOD_SOM_J11_5_PORT      gpioPortE   // See secondary pin under Mezzanine Header J9, U16, and U27.
#define MOD_SOM_J11_5_PIN       3
//                              (U1_68)
#define MOD_SOM_J11_7_PORT      gpioPortC   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_7_PIN       8
//                              (U1_27)
#define MOD_SOM_J11_9_PORT      gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_9_PIN       8
//                              (U1_95)
#define MOD_SOM_J11_11_PORT     gpioPortE
#define MOD_SOM_J11_11_PIN      11
//                              (U1_30)
#define MOD_SOM_J11_13_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_13_PIN      11
//                              (U1_100)
#define MOD_SOM_J11_27_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_27_PIN      15
//                              (U1_35)
#define MOD_SOM_J11_29_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_29_PIN      14
//                              (U1_34)
#define MOD_SOM_J11_31_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_31_PIN      13
//                              (U1_33)
#define MOD_SOM_J11_33_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_33_PIN      12
//                              (U1_26)
#define MOD_SOM_J11_35_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_35_PIN      7
//                              (U1_6)
#define MOD_SOM_J11_37_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_37_PIN      5
//                              (U1_5)
#define MOD_SOM_J11_39_PORT     gpioPortA   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_39_PIN      4
//                              (U1_56)
#define MOD_SOM_J11_41_PORT     gpioPortC
#define MOD_SOM_J11_41_PIN      7
//                              (U1_70)
#define MOD_SOM_J11_43_PORT     gpioPortC   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_43_PIN      10          // Connected to PE5 (U1_65)!
//                              (U1_65)
#define MOD_SOM_J11_43_INT_PORT gpioPortE   // See secondary pin under Mezzanine Header J9.     // TODO: Naming convention
#define MOD_SOM_J11_43_INT_PIN  5           // Connected to PC10 (U1_70)!                       // TODO: Naming convention
//                              (U1_69)
#define MOD_SOM_J11_45_PORT     gpioPortC   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_45_PIN      9
//                              (U1_71)
#define MOD_SOM_J11_47_PORT     gpioPortC   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_47_PIN      11
//                              (U1_11)
#define MOD_SOM_J11_49_PORT     gpioPortB   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_J11_49_PIN      2



// Magnetic Switch Connector J13
//                              (U1_7)
#define MOD_SOM_J13_2_PORT      gpioPortA   // See secondary pin under Magnetic Switch.
#define MOD_SOM_J13_2_PIN       6



// SD Card Slot
//                              (U1_3)
#define MOD_SOM_SD1_1_PORT      gpioPortA   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_1_PIN       2
//                              (U1_4)
#define MOD_SOM_SD1_2_PORT      gpioPortA   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_2_PIN       3
//                              (U1_99)
#define MOD_SOM_SD1_3_PORT      gpioPortE   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_3_PIN       15
//                              (U1_98)
#define MOD_SOM_SD1_5_PORT      gpioPortE   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_5_PIN       14
//                              (U1_1)
#define MOD_SOM_SD1_7_PORT      gpioPortA   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_7_PIN       0
//                              (U1_2)
#define MOD_SOM_SD1_8_PORT      gpioPortA   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_SD1_8_PIN       1



// Magnetic Switch
//                              (U1_7)
#define MOD_SOM_SW1_PORT        gpioPortA   // See secondary pin under Magnetic Switch Connector J13.
#define MOD_SOM_SW1_PIN         6



// Q1
//                              (U1_14)
#define MOD_SOM_Q1_1_PORT       gpioPortB   // See secondary pin under Mezzanine Header J9.
#define MOD_SOM_Q1_1_PIN        5



// U3
//                              (U1_25)
#define MOD_SOM_U3_2_PORT       gpioPortB
#define MOD_SOM_U3_2_PIN        8
//                              (U1_62)
#define MOD_SOM_U3_8_PORT       gpioPortE
#define MOD_SOM_U3_8_PIN        2



// U4
//                              (U1_43)
#define MOD_SOM_U4_5_PORT       gpioPortB
#define MOD_SOM_U4_5_PIN        14
//                              (U1_94)
#define MOD_SOM_U4_8_PORT       gpioPortE   // See secondary pin under Mezzanine Header J10 and U8.
#define MOD_SOM_U4_8_PIN        10



// U8
//                              (U1_94)
#define MOD_SOM_U8_3_PORT       gpioPortE   // See secondary pin under Mezzanine Header J10 and U4.
#define MOD_SOM_U8_3_PIN        10



// U10
//                              (U1_52)
#define MOD_SOM_U10_6_PORT      gpioPortD
#define MOD_SOM_U10_6_PIN       6



// U11
//                              (U1_42)
#define MOD_SOM_U11_6_PORT      gpioPortB   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_U11_6_PIN       13



// U13
//                              (U1_15)
#define MOD_SOM_U13_4_PORT      gpioPortB   // See secondary pin under U20.
#define MOD_SOM_U13_4_PIN       6



// U16
//                              (U1_40)
#define MOD_SOM_U16_3_PORT      gpioPortB
#define MOD_SOM_U16_3_PIN       12
//                              (U1_63)
#define MOD_SOM_U16_4_PORT      gpioPortE   // See secondary pin under Mezzanine Header J9, Analog Header J11, and U27.
#define MOD_SOM_U16_4_PIN       3



// U20
//                              (U1_15)
#define MOD_SOM_U20_16_PORT     gpioPortB   // See secondary pin under U13.
#define MOD_SOM_U20_16_PIN      6



// U27
//                              (U1_63)
#define MOD_SOM_U27_8_PORT      gpioPortE   // See secondary pin under Mezzanine Header J9, Analog Header J11, and U16.
#define MOD_SOM_U27_8_PIN       3
#define MOD_SOM_U27_8_PORT      gpioPortB   // See secondary pin under Mezzanine Header J9, pin 10.
#define MOD_SOM_U27_8_PIN       5


// U28
//                              (U1_13)
#define MOD_SOM_U28_16_PORT     gpioPortB   // See secondary pin under Mezzanine Header J10.
#define MOD_SOM_U28_16_PIN      4



// EXT_VDIV
//                              (U1_46)
#define MOD_SOM_EXT_VDIV_PORT   gpioPortD
#define MOD_SOM_EXT_VDIV_PIN    0



// BT1
//                              (U1_54)
#define MOD_SOM_BT1_1_PORT      gpioPortD
#define MOD_SOM_BT1_1_PIN       8



/* ----- NON-GPIO Pins ----- */
// IOVDD0                       (U1_8)
// VSS                          (U1_16)
// IOVDD0                       (U1_17)
// IOVDD0                       (U1_31)
// VSS                          (U1_32)
// RESETn                       (U1_36)
// AVDD                         (U1_41)
// IOVDD0                       (U1_44)
// AVDD                         (U1_45)
// DVDD                         (U1_57)
// VSS                          (U1_58)
// DECOUPLE                     (U1_59)
// VREGI                        (U1_72)
// VREG0                        (U1_73)
// VBUS                         (U1_79)
// IOVDD0                       (U1_82)
// VSS                          (U1_83)






















/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Table 2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                  */
/*                          Defines functionality                   */
/*                                                                  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/




// Function: Analog Front End Connector
// Primary Signal Group:  GPIO for CS signals and USART0 for SPI signals
// These signals interface with up to 9 ADCs or other sensors.  Additional
// GPIO is used for power enable ect.

// CS1 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_47_GPIO_PORT             MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_GPIO_PIN              MOD_SOM_J9_47_PIN
#define MOD_SOM_GPIO_CS1_PORT               MOD_SOM_J11_39_PORT
#define MOD_SOM_GPIO_CS1_PIN                MOD_SOM_J11_39_PIN

// CS2 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_51_GPIO_PORT             MOD_SOM_J9_51_PORT
#define MOD_SOM_J9_51_GPIO_PIN              MOD_SOM_J9_51_PIN
#define MOD_SOM_J11_37_GPIO_PORT            MOD_SOM_J11_37_PORT
#define MOD_SOM_J11_37_GPIO_PIN             MOD_SOM_J11_37_PIN

// CS3 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_53_GPIO_PORT             MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_GPIO_PIN              MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_GPIO_PORT            MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_GPIO_PIN             MOD_SOM_J11_35_PIN

// CS4 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_57_GPIO_PORT             MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_GPIO_PIN              MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_GPIO_PORT            MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_GPIO_PIN             MOD_SOM_J11_33_PIN

// CS5 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_59_GPIO_PORT             MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_GPIO_PIN              MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_GPIO_PORT            MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_GPIO_PIN             MOD_SOM_J11_31_PIN

// CS6 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_46_GPIO_PORT             MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_GPIO_PIN              MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_GPIO_PORT            MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_GPIO_PIN             MOD_SOM_J11_29_PIN

// CS7 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_44_GPIO_PORT             MOD_SOM_J9_44_PORT
#define MOD_SOM_J9_44_GPIO_PIN              MOD_SOM_J9_44_PIN
#define MOD_SOM_J11_27_GPIO_PORT            MOD_SOM_J11_27_PORT
#define MOD_SOM_J11_27_GPIO_PIN             MOD_SOM_J11_27_PIN

// CS8 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_42_GPIO_PORT             MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_GPIO_PIN              MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_GPIO_PORT             MOD_SOM_J11_9_PORT
#define MOD_SOM_J11_9_GPIO_PIN              MOD_SOM_J11_9_PIN

// CS9 - This signal is present on the J9 Mezzanine Header.
#define MOD_SOM_J11_11_GPIO_PORT            MOD_SOM_J11_11_PORT
#define MOD_SOM_J11_11_GPIO_PIN             MOD_SOM_J11_11_PIN

// GPIO (PA11) - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.   //TODO Naming convention?
#define MOD_SOM_J9_22_GPIO_PORT             MOD_SOM_J9_22_PORT
#define MOD_SOM_J9_22_GPIO_PIN              MOD_SOM_J9_22_PIN
#define MOD_SOM_J11_13_GPIO_PORT            MOD_SOM_J11_13_PORT
#define MOD_SOM_J11_13_GPIO_PIN             MOD_SOM_J11_13_PIN



// Function: SD Card - SDIO Mode
// Primary Signal Group: SDIO Mode on the SOM's SD Card
// These signals are present on the J10 Mezzanine Header and SD card slot.
#define MOD_SOM_J10_21_SDIO_DAT0_L1_PORT    MOD_SOM_J10_21_PORT
#define MOD_SOM_J10_21_SDIO_DAT0_L1_PIN     MOD_SOM_J10_21_PIN
#define MOD_SOM_SD1_7_SDIO_DAT0_L1_PORT     MOD_SOM_SD1_7_PORT
#define MOD_SOM_SD1_7_SDIO_DAT0_L1_PIN      MOD_SOM_SD1_7_PIN

#define MOD_SOM_SDIO_DAT1_L1_PORT       MOD_SOM_J10_19_PORT
#define MOD_SOM_SDIO_DAT1_L1_PIN        MOD_SOM_J10_19_PIN
//#define MOD_SOM_SDIO_DAT1_L1_PORT       MOD_SOM_SD1_8_PORT
//#define MOD_SOM_SDIO_DAT1_L1_PIN        MOD_SOM_SD1_8_PIN

#define MOD_SOM_J10_7_SDIO_DAT2_L1_PORT     MOD_SOM_J10_7_PORT
#define MOD_SOM_J10_7_SDIO_DAT2_L1_PIN      MOD_SOM_J10_7_PIN
#define MOD_SOM_SD1_1_SDIO_DAT2_L1_PORT     MOD_SOM_SD1_1_PORT
#define MOD_SOM_SD1_1_SDIO_DAT2_L1_PIN      MOD_SOM_SD1_1_PIN

#define MOD_SOM_J10_9_SDIO_DAT3_L1_PORT     MOD_SOM_J10_9_PORT
#define MOD_SOM_J10_9_SDIO_DAT3_L1_PIN      MOD_SOM_J10_9_PIN
#define MOD_SOM_SD1_2_SDIO_DAT3_L1_PORT     MOD_SOM_SD1_2_PORT
#define MOD_SOM_SD1_2_SDIO_DAT3_L1_PIN      MOD_SOM_SD1_2_PIN

#define MOD_SOM_J10_15_SDIO_CLK_L1_PORT     MOD_SOM_J10_15_PORT
#define MOD_SOM_J10_15_SDIO_CLK_L1_PIN      MOD_SOM_J10_15_PIN
#define MOD_SOM_SD1_5_SDIO_CLK_L1_PORT      MOD_SOM_SD1_5_PORT
#define MOD_SOM_SD1_5_SDIO_CLK_L1_PIN       MOD_SOM_SD1_5_PIN

#define MOD_SOM_J10_13_SDIO_CMD_L1_PORT     MOD_SOM_J10_13_PORT
#define MOD_SOM_J10_13_SDIO_CMD_L1_PIN      MOD_SOM_J10_13_PIN
#define MOD_SOM_SD1_3_SDIO_CMD_L1_PORT      MOD_SOM_SD1_3_PORT
#define MOD_SOM_SD1_3_SDIO_CMD_L1_PIN       MOD_SOM_SD1_3_PIN


// Function: TODO
// Primary Signal Group: UART0
#define MOD_SOM_J9_11_U0_TX_L4_PORT         MOD_SOM_J9_11_PORT
#define MOD_SOM_J9_11_U0_TX_L4_PIN          MOD_SOM_J9_11_PIN

#define MOD_SOM_J9_13_U0_RX_L4_PORT         MOD_SOM_J9_13_PORT
#define MOD_SOM_J9_13_U0_RX_L4_PIN          MOD_SOM_J9_13_PIN



// Function: TODO
// Primary Signal Group: UART1
#define MOD_SOM_J9_21_U1_TX_L2_PORT         MOD_SOM_J9_21_PORT
#define MOD_SOM_J9_21_U1_TX_L2_PIN          MOD_SOM_J9_21_PIN

#define MOD_SOM_J9_23_U1_RX_L2_PORT         MOD_SOM_J9_23_PORT
#define MOD_SOM_J9_23_U1_RX_L2_PIN          MOD_SOM_J9_23_PIN



// Function: TODO
// Primary Signal Group: USART0
// This signal is present on the J9 Mezzanine Connector and J11 ANALOG_COM header.
#define MOD_SOM_J9_56_US0_RX_L2_PORT        MOD_SOM_J9_56_PORT        // Connected to PE5 (U1_65)
#define MOD_SOM_J9_56_US0_RX_L2_PIN         MOD_SOM_J9_56_PIN         // Connected to PE5 (U1_65)
#define MOD_SOM_J11_43_US0_RX_L2_PORT       MOD_SOM_J11_43_PORT       // Connected to PE5 (U1_65)
#define MOD_SOM_J11_43_US0_RX_L2_PIN        MOD_SOM_J11_43_PIN        // Connected to PE5 (U1_65)

#define MOD_SOM_J9_58_US0_TX_L2_PORT        MOD_SOM_J9_58_PORT
#define MOD_SOM_J9_58_US0_TX_L2_PIN         MOD_SOM_J9_58_PIN
#define MOD_SOM_J11_47_US0_TX_L2_PORT       MOD_SOM_J11_47_PORT
#define MOD_SOM_J11_47_US0_TX_L2_PIN        MOD_SOM_J11_47_PIN

#define MOD_SOM_J9_50_US0_CLK_L2_PORT       MOD_SOM_J9_50_PORT
#define MOD_SOM_J9_50_US0_CLK_L2_PIN        MOD_SOM_J9_50_PIN
#define MOD_SOM_J11_45_US0_CLK_L2_PORT      MOD_SOM_J11_45_PORT
#define MOD_SOM_J11_45_US0_CLK_L2_PIN       MOD_SOM_J11_45_PIN

#define MOD_SOM_J9_32_US0_CS_L2_PORT        MOD_SOM_J9_32_PORT
#define MOD_SOM_J9_32_US0_CS_L2_PIN         MOD_SOM_J9_32_PIN
#define MOD_SOM_J11_7_US0_CS_L2_PORT        MOD_SOM_J11_7_PORT
#define MOD_SOM_J11_7_US0_CS_L2_PIN         MOD_SOM_J11_7_PIN



// Function: TODO
// Primary Signal Group: USART1
// This signal is present on the J9 Mezzanine Header.
#define MOD_SOM_J9_36_US1_TX_L4_PORT        MOD_SOM_J9_36_PORT
#define MOD_SOM_J9_36_US1_TX_L4_PIN         MOD_SOM_J9_36_PIN

#define MOD_SOM_J9_40_US1_RX_L4_PORT        MOD_SOM_J9_40_PORT
#define MOD_SOM_J9_40_US1_RX_L4_PIN         MOD_SOM_J9_40_PIN

#define MOD_SOM_J9_28_US1_CLK_L4_PORT       MOD_SOM_J9_28_PORT
#define MOD_SOM_J9_28_US1_CLK_L4_PIN        MOD_SOM_J9_28_PIN

#define MOD_SOM_J9_24_US1_CS_L4_PORT        MOD_SOM_J9_24_PORT
#define MOD_SOM_J9_24_US1_CS_L4_PIN         MOD_SOM_J9_24_PIN



// Function: AUX_COM5_TTL
// Primary Signal Group: USART2
// TTL level SPI signals are available on the mezzanine
// header J10.
// No control signals external to the microcontroller are
// required to use this function.
// If the SOM SDIO SD-card doesn't work, these signals would
// be used for a SPI-mode SD card interface on the
// mezzanine board.
#define MOD_SOM_J10_24_US2_TX_L4_PORT       MOD_SOM_J10_24_PORT
#define MOD_SOM_J10_24_US2_TX_L4_PIN        MOD_SOM_J10_24_PIN

#define MOD_SOM_J10_26_US2_RX_L4_PORT       MOD_SOM_J10_26_PORT
#define MOD_SOM_J10_26_US2_RX_L4_PIN        MOD_SOM_J10_26_PIN

#define MOD_SOM_J10_22_US2_CLK_L4_PORT      MOD_SOM_J10_22_PORT
#define MOD_SOM_J10_22_US2_CLK_L4_PIN       MOD_SOM_J10_22_PIN

#define MOD_SOM_J10_28_US2_CS_L4_PORT       MOD_SOM_J10_28_PORT
#define MOD_SOM_J10_28_US2_CS_L4_PIN        MOD_SOM_J10_28_PIN



// Function: AUX_COM2_TTL
// Primary Signal Group: USART3
// TTL level SPI signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_29_US3_CLK_L1_PORT       MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_US3_CLK_L1_PIN        MOD_SOM_J9_29_PIN

#define MOD_SOM_J9_31_US3_CS_L1_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_US3_CS_L1_PIN         MOD_SOM_J9_31_PIN

#define MOD_SOM_J9_26_US3_TX_L1_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_US3_TX_L1_PIN         MOD_SOM_J9_26_PIN

#define MOD_SOM_J9_27_US3_RX_L1_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_US3_RX_L1_PIN         MOD_SOM_J9_27_PIN



// Function: AUX_COM6_TTL
// Primary Signal Group: USART4
// TTL level SPI signals are available on the mezzanine
// header J10 and the auxiliary connector J2.
// No control signals external to the microcontroller are
// required to use this function.  However, there is a
// switchable 3.6V power supply on J2 which is controlled
// by SPARE_PWR_EN.
// TODO: swap #define signal names for SPARE_PWR_EN
#define MOD_SOM_J10_20_US4_CS_L1_PORT       MOD_SOM_J10_20_PORT
#define MOD_SOM_J10_20_US4_CS_L1_PIN        MOD_SOM_J10_20_PIN
#define MOD_SOM_J2_6_US4_CS_L1_PORT         MOD_SOM_J2_6_PORT
#define MOD_SOM_J2_6_US4_CS_L1_PIN          MOD_SOM_J2_6_PIN

#define MOD_SOM_J10_18_US4_CLK_L1_PORT      MOD_SOM_J10_18_PORT
#define MOD_SOM_J10_18_US4_CLK_L1_PIN       MOD_SOM_J10_18_PIN
#define MOD_SOM_J2_5_US4_CLK_L1_PORT        MOD_SOM_J2_5_PORT
#define MOD_SOM_J2_5_US4_CLK_L1_PIN         MOD_SOM_J2_5_PIN

#define MOD_SOM_J10_14_US4_RX_L1_PORT       MOD_SOM_J10_14_PORT
#define MOD_SOM_J10_14_US4_RXL1_PIN         MOD_SOM_J10_14_PIN
#define MOD_SOM_J2_3_US4_RX_L1_PORT         MOD_SOM_J2_3_PORT
#define MOD_SOM_J2_3_US4_RX_L1_PIN           MOD_SOM_J2_3_PIN

#define MOD_SOM_J10_16_US4_TX_L1_PORT       MOD_SOM_J10_16_PORT
#define MOD_SOM_J10_16_US4_TX_L1_PIN        MOD_SOM_J10_16_PIN
#define MOD_SOM_J2_4_US4_TX_L1_PORT         MOD_SOM_J2_4_PORT
#define MOD_SOM_J2_4_US4_TX_L1_PIN          MOD_SOM_J2_4_PIN



// Function: MAIN_COM_RS232
// Primary Signal Group: USART5
// RS232 level RX and TX signals are available on the mezzanine
// header J10 and main connector J1.  The US5 clock signal
// is not available on the main connector J1.
// Before using this function, the RS232 driver IC must be powered
// up with the URT_EN signal.  After power-up, the IC is enabled
// with the SERIAL_COMMS_EN signal.
// TODO: swap #define signal names for URT_EN, SERIAL_COMMS_EN
#define MOD_SOM_J1_4_US5_RX_L0_PORT         MOD_SOM_J1_4_PORT
#define MOD_SOM_J1_4_US5_RX_L0_PIN          MOD_SOM_J1_4_PIN
#define MOD_SOM_J10_6_US5_RX_L0_PORT        MOD_SOM_J10_6_PORT
#define MOD_SOM_J10_6_US5_RX_L0_PIN         MOD_SOM_J10_6_PIN

#define MOD_SOM_J1_3_US5_TX_L0_PORT         MOD_SOM_J1_3_PORT
#define MOD_SOM_J1_3_US5_TX_L0_PIN          MOD_SOM_J1_3_PIN
#define MOD_SOM_J10_8_US5_TX_L0_PORT        MOD_SOM_J10_8_PORT
#define MOD_SOM_J10_8_US5_TX_L0_PIN         MOD_SOM_J10_8_PIN

#define MOD_SOM_J10_10_US5_CLK_L0_PORT      MOD_SOM_J10_10_PORT
#define MOD_SOM_J10_10_US5_CLK_L0_PIN       MOD_SOM_J10_10_PIN



// Function: AUX_COM_RS232
// Primary Signal Group: Low Energy UART0
// RS232 level RX and TX signals are available on the mezzanine
// header J9 and main power connector J1.
// Before using this function, the RS232 driver IC must be powered
// up with the URT_EN signal.  After power-up, the IC is enabled
// with the SBE_EN signal.
#define MOD_SOM_J9_45_LEU0_RX_L0_PORT       MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_LEU0_RX_L0_PIN        MOD_SOM_J9_45_PIN

#define MOD_SOM_J1_9_LEU0_RX_L0_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_LEU0_RX_L0_PIN         MOD_SOM_J1_9_PIN

#define MOD_SOM_J9_41_LEU0_TX_L0_PORT       MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_LEU0_TX_L0_PIN        MOD_SOM_J9_41_PIN

#define MOD_SOM_J1_10_LEU0_TX_L0_PORT       MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_LEU0_TX_L0_PIN        MOD_SOM_J1_10_PIN



// Function: AUX_COM4_TTL
// Primary Signal Group: Low Energy UART1
// TTL level RX & TX signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_16_LEU1_RX_L2_PORT       MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_LEU1_RX_L2_PIN        MOD_SOM_J9_16_PIN

#define MOD_SOM_J9_14_LEU1_TX_L2_PORT       MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_LEU1_TX_L2_PIN        MOD_SOM_J9_14_PIN



// Function: USB
// Primary Signal Group: USB
// USB signals are available on the mezzanine
// header J10.
// There is only one location for USB signals on the EFM32GG11.
#define MOD_SOM_J10_36_USB_DP_PORT          MOD_SOM_J10_36_PORT     // Positive diff signal
#define MOD_SOM_J10_36_USB_DP_PIN           MOD_SOM_J10_36_PIN

#define MOD_SOM_J10_38_USB_DN_PORT          MOD_SOM_J10_38_PORT     // Negative diff signal
#define MOD_SOM_J10_38_USB_DN_PIN           MOD_SOM_J10_38_PIN

#define MOD_SOM_J10_32_USB_ID_PORT          MOD_SOM_J10_32_PORT
#define MOD_SOM_J10_32_USB_ID_PIN           MOD_SOM_J10_32_PIN

#define MOD_SOM_J10_30_USB_VBUSEN_PORT      MOD_SOM_J10_30_PORT
#define MOD_SOM_J10_30_USB_VBUSEN_PIN       MOD_SOM_J10_30_PIN



// Function: TODO
// Primary Signal Group: I2C0
#define MOD_SOM_J9_2_I2C0_SDA_L6_PORT       MOD_SOM_J9_2_PORT
#define MOD_SOM_J9_2_I2C0_SDA_L6_PIN        MOD_SOM_J9_2_PIN

#define MOD_SOM_J9_4_I2C0_SCL_L6_PORT       MOD_SOM_J9_4_PORT
#define MOD_SOM_J9_4_I2C0_SCL_L6_PIN        MOD_SOM_J9_4_PIN



// Function: AUX_COM3_TTL
// Primary Signal Group: I2C1
// TTL level I2C signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_35_I2C1_SCL_L2_PORT      MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_I2C1_SCL_L2_PIN       MOD_SOM_J9_35_PIN

#define MOD_SOM_J9_37_I2C1_SDA_L2_PORT      MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_I2C1_SDA_L2_PIN       MOD_SOM_J9_37_PIN



// ANA_EN
// This signal is present on the J9 Mezzanine Connector and J11 ANALOG_COM header.
#define MOD_SOM_J9_15_GPIO_PORT             MOD_SOM_J9_6_PORT
#define MOD_SOM_J9_15_GPIO_PIN              MOD_SOM_J9_6_PIN
#define MOD_SOM_J11_3_GPIO_PORT             MOD_SOM_J11_3_PORT
#define MOD_SOM_J11_3_GPIO_PIN              MOD_SOM_J11_3_PIN



// MCLK
// This signal is present on the J9 Mezzanine Connector and J11 ANALOG_COM Header.
#define MOD_SOM_J9_52_MCLK_PORT             MOD_SOM_J9_52_PORT          //TODO Naming convention?
#define MOD_SOM_J9_52_MCLK_PIN              MOD_SOM_J9_52_PIN           //TODO Naming convention?
#define MOD_SOM_J11_49_MCLK_PORT            MOD_SOM_J11_49_PORT         //TODO Naming convention?
#define MOD_SOM_J11_49_MCLK_PIN             MOD_SOM_J11_49_PIN          //TODO Naming convention?



// SYNC
// This signal is present on the J11 ANALOG_COM Header.
#define MOD_SOM_J11_41_SYNC_PORT            MOD_SOM_J11_41_PORT         //TODO Naming convention?
#define MOD_SOM_J11_41_SYNC_PIN             MOD_SOM_J11_41_PIN          //TODO Naming convention?



// MISO_ADC_INT
// This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_56_MISO_ADC_INT_PORT     MOD_SOM_J9_56_INT_PORT      // TODO Naming convention?
#define MOD_SOM_J9_56_MISO_ADC_INT_PIN      MOD_SOM_J9_56_INT_PIN       // Connected to PC10 (U1_70)
#define MOD_SOM_J11_43_MISO_ADC_INT_PORT    MOD_SOM_J11_43_INT_PORT     // TODO Naming convention?
#define MOD_SOM_J11_43_MISO_ADC_INT_PIN     MOD_SOM_J11_43_INT_PIN      // Connected to PC10 (U1_70)



// Function: Reed Switch
// TODO describe functionality
// This signal is present on the J13 Connector and Reed Switch.
#define MOD_SOM_SW1_EM4WU1_PORT     MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_EM4WU1_PIN          MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_EM4WU1_PORT      MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_EM4WU1_PIN       MOD_SOM_J13_2_PIN



// Function: Debug-Interface
// TODO describe functionality
#define MOD_SOM_J8_4_DBG_SWCLKTCK_PORT      MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_DBG_SWCLKTCK_PIN       MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_2_DBG_SWDIOTMS_PORT      MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_DBG_SWDIOTMS_PIN       MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_6_DBG_SWO_L0_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_DBG_SWO_L0_PIN         MOD_SOM_J8_6_PIN



// Function: TIMER1
// TODO describe functionality
#define MOD_SOM_J9_5_TIM1_CC0_L2_PORT       MOD_SOM_J9_5_PORT
#define MOD_SOM_J9_5_TIM1_CC0_L2_PIN        MOD_SOM_J9_5_PIN

#define MOD_SOM_J9_7_TIM1_CC1_L2_PORT       MOD_SOM_J9_7_PORT
#define MOD_SOM_J9_7_TIM1_CC1_L2_PIN        MOD_SOM_J9_7_PIN



// Function: Altimeter
// Primary Signal Group:  WTIMER2
// TODO describe functionality
#define MOD_SOM_J9_15_WTIM2_CC0_L0_PORT     MOD_SOM_J9_15_PORT
#define MOD_SOM_J9_15_WTIM2_CC0_L0_PIN      MOD_SOM_J9_15_PIN

#define MOD_SOM_J9_17_WTIM2_CC1_L0_PORT     MOD_SOM_J9_17_PORT
#define MOD_SOM_J9_17_WTIM2_CC1_L0_PIN      MOD_SOM_J9_17_PIN


// Function: ADC0
// TODO describe functionality
#define MOD_SOM_EXT_VDIV_ADC0_CH0_PORT      MOD_SOM_R14_PORT        //TODO Naming convention
#define MOD_SOM_EXT_VDIV_ADC0_CH0_PIN       MOD_SOM_R14_PIN         //TODO Naming convention

#define MOD_SOM_J9_39_ADC0_CH1_PORT         MOD_SOM_J9_39_PORT          //TODO Naming convention
#define MOD_SOM_J9_39_ADC0_CH1_PIN          MOD_SOM_J9_39_PIN           //TODO Naming convention



// Function: LED
// This signal is present on the J9 Mezzanine Header and J1 Main Connector.
// TODO describe functionality
#define MOD_SOM_J9_20_GPIO_PORT             MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_GPIO_PIN              MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_GPIO_PORT              MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_GPIO_PIN               MOD_SOM_J1_6_PIN



// Function: Backup Battery
#define MOD_SOM_BT1_1_BU_VIN_PORT           MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_BU_VIN_PIN            MOD_SOM_BT1_1_PIN

#define MOD_SOM_U3_8_BU_VOUT_PORT           MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_BU_VOUT_PIN            MOD_SOM_U3_8_PIN



// GPIO & SER_COMMS_DE
// This signal is present on the J10 Mezzanine Header.
#define MOD_SOM_J10_35_GPIO_PORT            MOD_SOM_J10_35_PORT
#define MOD_SOM_J10_35_GPIO_PIN             MOD_SOM_J10_35_PIN



// SER_COMMS_EN
// This signal is present on the J10 Mezzanine Header and U28.
#define MOD_SOM_J10_27_GPIO_PORT            MOD_SOM_J10_27_PORT         //To Mezzanine
#define MOD_SOM_J10_27_GPIO_PIN             MOD_SOM_J10_27_PIN          //To Mezzanine
#define MOD_SOM_U28_16_GPIO_PORT            MOD_SOM_U28_16_PORT         //To Primary MAX322
#define MOD_SOM_U28_16_GPIO_PIN             MOD_SOM_U28_16_PIN          //To Primaty MAX322



// PS_EN
// This signal is present on the J9 Mezzanine Header and Q1.
#define MOD_SOM_J9_10_GPIO_PORT             MOD_SOM_J9_10_PORT
#define MOD_SOM_J9_10_GPIO_PIN              MOD_SOM_J9_10_PIN
#define MOD_SOM_Q1_1_GPIO_PORT              MOD_SOM_Q1_1_PORT
#define MOD_SOM_Q1_1_GPIO_PIN               MOD_SOM_Q1_1_PIN



// SBE_EN
// This signal is present on U20 and U13.
#define MOD_SOM_U20_16_GPIO_PORT            MOD_SOM_U20_16_PORT         //To SBE49 MAX322
#define MOD_SOM_U20_16_GPIO_PIN             MOD_SOM_U20_16_PIN          //To SBE49 MAX322
#define MOD_SOM_U13_4_GPIO_PORT             MOD_SOM_U13_4_PORT          //Enables 12V supply
#define MOD_SOM_U13_4_GPIO_PIN              MOD_SOM_U13_4_PIN           //Enables 12V supply



// SPARE_PWR_EN
// This signal is present on U16.
#define MOD_SOM_U16_3_GPIO_PORT             MOD_SOM_U16_3_PORT
#define MOD_SOM_U16_3_GPIO_PIN              MOD_SOM_U16_3_PIN



// SD1 SD_EN
// Enables 3.3V_SD to SD card slot
#define MOD_SOM_U10_6_SD1_SPIMODE_ENABLE_PORT   MOD_SOM_U10_6_PORT
#define MOD_SOM_U10_6_SD1_SPIMODE_ENABLE_PIN    MOD_SOM_U10_6_PIN

#define MOD_SOM_U10_6_SD1_SDMODE_ENABLE_PORT    MOD_SOM_U10_6_PORT      //TODO Naming convention?
#define MOD_SOM_U10_6_SD1_SDMODE_ENABLE_PIN     MOD_SOM_U10_6_PIN       //TODO Naming convention?




// URT_EN
// This signal is present on the J10 Mezzanine Header and U11.
#define MOD_SOM_J10_12_GPIO_PORT            MOD_SOM_J10_12_PORT
#define MOD_SOM_J10_12_GPIO_PIN             MOD_SOM_J10_12_PIN
#define MOD_SOM_U11_6_GPIO_PORT             MOD_SOM_U11_6_PORT
#define MOD_SOM_U11_6_GPIO_PIN              MOD_SOM_U11_6_PIN



// LFXTAL_N
#define MOD_SOM_U3_2_LFXTAL_N_PORT          MOD_SOM_U3_2_PORT
#define MOD_SOM_U3_2_LFXTAL_N_PIN           MOD_SOM_U3_2_PIN



// HFXTAL_N
#define MOD_SOM_U4_5_HFXTAL_N_PORT          MOD_SOM_U4_5_PORT
#define MOD_SOM_U4_5_HFXTAL_N_PIN           MOD_SOM_U4_5_PIN



// HFXO OE
// This signal is present on the J10 Mezzanine Header, U4, and U8.
#define MOD_SOM_J10_25_GPIO_PORT            MOD_SOM_J10_25_PORT
#define MOD_SOM_J10_25_GPIO_PIN             MOD_SOM_J10_25_PIN
#define MOD_SOM_U4_8_GPIO_PORT              MOD_SOM_U4_8_PORT
#define MOD_SOM_U4_8_GPIO_PIN               MOD_SOM_U4_8_PIN
#define MOD_SOM_U8_3_GPIO_PORT              MOD_SOM_U8_3_PORT
#define MOD_SOM_U8_3_GPIO_PIN               MOD_SOM_U8_3_PIN



// Function: FAULT
// This signal is present on the J9 Mezzanine Header, J11 ANALOG_COM Header, U27, and U16.
// TODO describe functionality
#define MOD_SOM_J9_8_FAULT_PORT             MOD_SOM_J9_8_PORT
#define MOD_SOM_J9_8_FAULT_PIN              MOD_SOM_J9_8_PIN

#define MOD_SOM_J11_5_FAULT_PORT            MOD_SOM_J11_5_PORT
#define MOD_SOM_J11_5_FAULT_PIN             MOD_SOM_J11_5_PIN

#define MOD_SOM_U27_8_FAULT_PORT            MOD_SOM_U27_8_PORT
#define MOD_SOM_U27_8_FAULT_PIN             MOD_SOM_U27_8_PIN

#define MOD_SOM_U16_4_FAULT_PORT            MOD_SOM_U16_4_PORT
#define MOD_SOM_U16_4_FAULT_PIN             MOD_SOM_U16_4_PIN









/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Table 3~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                  */
/*                      Alternative functionality                   */
/*                          WORK IN PROGRESS                        */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/




/*-----ADC Chip Selects-----*/

// CS1 - This signal is present on the J9 Mezzanine Connector and J11 ANALOG_COM header.
#define MOD_SOM_J9_47_TIM0_CDTI1_L0_PORT    MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_TIM0_CDTI1_L0_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_TIM0_CDTI1_L0_PORT    MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_TIM0_CDTI1_L0_PIN    MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_TIM3_CC1_L5_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_TIM3_CC1_L5_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_TIM3_CC1_L5_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_TIM3_CC1_L5_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_ETH_RMIICRSDV_L0_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_ETH_RMIICRSDV_L0_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_ETH_RMIICRSDV_L0_PORT    MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_ETH_RMIICRSDV_L0_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_ETH_MIITXD0_L0_PORT    MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_ETH_MIITXD0_L0_PIN    MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_ETH_MIITXD0_L0_PORT    MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_ETH_MIITXD0_L0_PIN    MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_SDIO_DAT4_L1_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_SDIO_DAT4_L1_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_SDIO_DAT4_L1_PORT    MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_SDIO_DAT4_L1_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_US3_CTS_L0_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_US3_CTS_L0_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_US3_CTS_L0_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_US3_CTS_L0_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_U0_RX_L2_PORT            MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_U0_RX_L2_PIN            MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_U0_RX_L2_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_U0_RX_L2_PIN            MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_QSPI0_DQ2_L1_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_QSPI0_DQ2_L1_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_QSPI0_DQ2_L1_PORT    MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_QSPI0_DQ2_L1_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_LES_ALTEX3_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_LES_ALTEX3_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_LES_ALTEX3_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_LES_ALTEX3_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_PRS_CH16_L0_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_PRS_CH16_L0_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_PRS_CH16_L0_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_PRS_CH16_L0_PIN        MOD_SOM_J11_39_PIN

#define MOD_SOM_J9_47_ETM_TD2_L3_PORT        MOD_SOM_J9_47_PORT
#define MOD_SOM_J9_47_ETM_TD2_L3_PIN        MOD_SOM_J9_47_PIN
#define MOD_SOM_J11_39_ETM_TD2_L3_PORT        MOD_SOM_J11_39_PORT
#define MOD_SOM_J11_39_ETM_TD2_L3_PIN        MOD_SOM_J11_39_PIN


// CS3 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_53_TIM0_CC2_L5_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_TIM0_CC2_L5_PIN        MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_TIM0_CC2_L5_PORT        MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_TIM0_CC2_L5_PIN        MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_TIM1_OUT0_L0_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_TIM1_OUT0_L0_PIN        MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_TIM1_OUT0_L0_PORT    MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_TIM1_OUT0_L0_PIN        MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_PCNT1_S0IN_L4_PORT    MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_PCNT1_S0IN_L4_PIN        MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_PCNT1_S0IN_L4_PORT    MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_PCNT1_S0IN_L4_PIN    MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_US2_TX_L2_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_US2_TX_L2_PIN            MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_US2_TX_L2_PORT        MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_US2_TX_L2_PIN        MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_US4_CTS_L0_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_US4_CTS_L0_PIN        MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_US4_CTS_L0_PORT        MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_US4_CTS_L0_PIN        MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_US5_RX_L1_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_US5_RX_L1_PIN            MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_US5_RX_L1_PORT        MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_US5_RX_L1_PIN        MOD_SOM_J11_35_PIN

#define MOD_SOM_J9_53_PRS_CH7_L1_PORT        MOD_SOM_J9_53_PORT
#define MOD_SOM_J9_53_PRS_CH7_L1_PIN        MOD_SOM_J9_53_PIN
#define MOD_SOM_J11_35_PRS_CH7_L1_PORT        MOD_SOM_J11_35_PORT
#define MOD_SOM_J11_35_PRS_CH7_L1_PIN        MOD_SOM_J11_35_PIN


// CS4 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_57_TIM2_CC0_L1_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_TIM2_CC0_L1_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_TIM2_CC0_L1_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_TIM2_CC0_L1_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_WTIM0_CDTI0_L2_PORT    MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_WTIM0_CDTI0_L2_PIN    MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_WTIM0_CDTI0_L2_PORT    MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_WTIM0_CDTI0_L2_PIN    MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_WTIM2_CC0_L1_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_WTIM2_CC0_L1_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_WTIM2_CC0_L1_PORT    MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_WTIM2_CC0_L1_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_LETIM1_OUT0_L2_PORT    MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_LETIM1_OUT0_L2_PIN    MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_LETIM1_OUT0_L2_PORT    MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_LETIM1_OUT0_L2_PIN    MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_PCNT1_S0IN_L5_PORT    MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_PCNT1_S0IN_L5_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_PCNT1_S0IN_L5_PORT    MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_PCNT1_S0IN_L5_PIN    MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_CAN1_RX_L5_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_CAN1_RX_L5_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_CAN1_RX_L5_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_CAN1_RX_L5_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_US0_CLK_L5_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_US0_CLK_L5_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_US0_CLK_L5_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_US0_CLK_L5_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_US2_RTS_L2_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_US2_RTS_L2_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_US2_RTS_L2_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_US2_RTS_L2_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_CMU_CLK0_L5_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_CMU_CLK0_L5_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_CMU_CLK0_L5_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_CMU_CLK0_L5_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_PRS_CH12_L0_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_PRS_CH12_L0_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_PRS_CH12_L0_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_PRS_CH12_L0_PIN        MOD_SOM_J11_33_PIN

#define MOD_SOM_J9_57_ACMP1_O_L3_PORT        MOD_SOM_J9_57_PORT
#define MOD_SOM_J9_57_ACMP1_O_L3_PIN        MOD_SOM_J9_57_PIN
#define MOD_SOM_J11_33_ACMP1_O_L3_PORT        MOD_SOM_J11_33_PORT
#define MOD_SOM_J11_33_ACMP1_O_L3_PIN        MOD_SOM_J11_33_PIN


// CS5 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_59_TIM0_CC2_L7_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_TIM0_CC2_L7_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_TIM0_CC2_L7_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_TIM0_CC2_L7_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_TIM2_CC1_L1_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_TIM2_CC1_L1_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_TIM2_CC1_L1_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_TIM2_CC1_L1_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_WTIM0_CDTI1_L2_PORT    MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_WTIM0_CDTI1_L2_PIN    MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_WTIM0_CDTI1_L2_PORT    MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_WTIM0_CDTI1_L2_PIN    MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_WTIM2_CC1_L1_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_WTIM2_CC1_L1_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_WTIM2_CC1_L1_PORT    MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_WTIM2_CC1_L1_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_LETIM1_OUT1_L1_PORT    MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_LETIM1_OUT1_L1_PIN    MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_LETIM1_OUT1_L1_PORT    MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_LETIM1_OUT1_L1_PIN    MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_PCNT1_S1IN_L5_PORT    MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_PCNT1_S1IN_L5_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_PCNT1_S1IN_L5_PORT    MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_PCNT1_S1IN_L5_PIN    MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_CAN1_TX_L5_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_CAN1_TX_L5_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_CAN1_TX_L5_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_CAN1_TX_L5_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_US0_CS_L5_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_US0_CS_L5_PIN            MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_US0_CS_L5_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_US0_CS_L5_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_US2_TX_L3_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_US2_TX_L3_PIN            MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_US2_TX_L3_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_US2_TX_L3_PIN        MOD_SOM_J11_31_PIN

#define MOD_SOM_J9_59_PRS_CH13_L0_PORT        MOD_SOM_J9_59_PORT
#define MOD_SOM_J9_59_PRS_CH13_L0_PIN        MOD_SOM_J9_59_PIN
#define MOD_SOM_J11_31_PRS_CH13_L0_PORT        MOD_SOM_J11_31_PORT
#define MOD_SOM_J11_31_PRS_CH13_L0_PIN        MOD_SOM_J11_31_PIN


// CS6 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_46_TIM2_CC2_L1_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_TIM2_CC2_L1_PIN        MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_TIM2_CC2_L1_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_TIM2_CC2_L1_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_WTIM0_CDTI2_L2_PORT    MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_WTIM0_CDTI2_L2_PIN    MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_WTIM0_CDTI2_L2_PORT    MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_WTIM0_CDTI2_L2_PIN    MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_WTIM2_CC2_L1_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_WTIM2_CC2_L1_PIN        MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_WTIM2_CC2_L1_PORT    MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_WTIM2_CC2_L1_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_LETIM1_OUT1_L2_PORT    MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_LETIM1_OUT1_L2_PIN    MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_LETIM1_OUT1_L2_PORT    MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_LETIM1_OUT1_L2_PIN    MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_US1_TX_L6_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_US1_TX_L6_PIN            MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_US1_TX_L6_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_US1_TX_L6_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_US2_RX_L3_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_US2_RX_L3_PIN            MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_US2_RX_L3_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_US2_RX_L3_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_US3_RTS_L2_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_US3_RTS_L2_PIN        MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_US3_RTS_L2_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_US3_RTS_L2_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_PRS_CH14_L0_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_PRS_CH14_L0_PIN        MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_PRS_CH14_L0_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_PRS_CH14_L0_PIN        MOD_SOM_J11_29_PIN

#define MOD_SOM_J9_46_ACMP1_O_L4_PORT        MOD_SOM_J9_46_PORT
#define MOD_SOM_J9_46_ACMP1_O_L4_PIN        MOD_SOM_J9_46_PIN
#define MOD_SOM_J11_29_ACMP1_O_L4_PORT        MOD_SOM_J11_29_PORT
#define MOD_SOM_J11_29_ACMP1_O_L4_PIN        MOD_SOM_J11_29_PIN


// CS7 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define    MOD_SOM_J9_44_TIM3_CC2_L0_PORT        MOD_SOM_J9_44_PORT
#define    MOD_SOM_J9_44_TIM3_CC2_L0_PIN        MOD_SOM_J9_44_PIN
#define    MOD_SOM_J11_27_TIM3_CC2_L0_PORT        MOD_SOM_J11_27_PORT
#define    MOD_SOM_J11_27_TIM3_CC2_L0_PIN        MOD_SOM_J11_27_PIN

#define    MOD_SOM_J9_44_US2_CLK_L3_PORT        MOD_SOM_J9_44_PORT
#define    MOD_SOM_J9_44_US2_CLK_L3_PIN        MOD_SOM_J9_44_PIN
#define    MOD_SOM_J11_27_US2_CLK_L3_PORT        MOD_SOM_J11_27_PORT
#define    MOD_SOM_J11_27_US2_CLK_L3_PIN        MOD_SOM_J11_27_PIN

#define    MOD_SOM_J9_44_PRS_CH15_L0_PORT        MOD_SOM_J9_44_PORT
#define    MOD_SOM_J9_44_PRS_CH15_L0_PIN        MOD_SOM_J9_44_PIN
#define    MOD_SOM_J11_27_PRS_CH15_L0_PORT        MOD_SOM_J11_27_PORT
#define    MOD_SOM_J11_27_PRS_CH15_L0_PIN        MOD_SOM_J11_27_PIN


// CS8 - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.
#define MOD_SOM_J9_42_TIM2_CC0_L0_PORT        MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_TIM2_CC0_L0_PIN        MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_TIM2_CC0_L0_PORT        MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_TIM2_CC0_L0_PIN        MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_TIM0_CC0_L6_PORT        MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_TIM0_CC0_L6_PIN        MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_TIM0_CC0_L6_PORT        MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_TIM0_CC0_L6_PIN        MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_LETIM0_OUT0_L6_PORT    MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_LETIM0_OUT0_L6_PIN    MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_LETIM0_OUT0_L6_PORT    MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_LETIM0_OUT0_L6_PIN    MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_PCNT1_S1IN_L4_PORT    MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_PCNT1_S1IN_L4_PIN        MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_PCNT1_S1IN_L4_PORT    MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_PCNT1_S1IN_L4_PIN        MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_US2_RX_L2_PORT        MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_US2_RX_L2_PIN            MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_US2_RX_L2_PORT        MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_US2_RX_L2_PIN            MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_US4_RTS_L0_PORT        MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_US4_RTS_L0_PIN        MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_US4_RTS_L0_PORT        MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_US4_RTS_L0_PIN        MOD_SOM_J11_9_PIN

#define MOD_SOM_J9_42_PRS_CH8_L0_PORT        MOD_SOM_J9_42_PORT
#define MOD_SOM_J9_42_PRS_CH8_L0_PIN        MOD_SOM_J9_42_PIN
#define MOD_SOM_J11_9_PRS_CH8_L0_PORT        MOD_SOM_J11_9_PORT
#define    MOD_SOM_J11_9_PRS_CH8_L0_PIN        MOD_SOM_J11_9_PIN


// CS9
#define    MOD_SOM_J11_11_TIM1_CC1_L1_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_TIM1_CC1_L1_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_TIM4_CC2_L7_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_TIM4_CC2_L7_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_WTIM0_CDTI1_L0_PORT    MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_WTIM0_CDTI1_L0_PIN    MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_SDIO_DAT0_L0_PORT    MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_SDIO_DAT0_L0_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_QSPI0_DQ7_L0_PORT    MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_QSPI0_DQ7_L0_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_US0_RX_L0_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_US0_RX_L0_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_LES_ALTEX5_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_LES_ALTEX5_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_PRS_CH3_L2_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_PRS_CH3_l2_PIN        MOD_SOM_J11_11_PIN

#define    MOD_SOM_J11_11_ETM_TCLK_L4_PORT        MOD_SOM_J11_11_PORT
#define    MOD_SOM_J11_11_ETM_TCLK_L4_PIN        MOD_SOM_J11_11_PIN


// GPIO (PA11) - This signal is present on the J9 Mezzanine Header and J11 ANALOG_COM Header.    //TODO Naming convention?
#define MOD_SOM_J9_22_WTIM2_CC2_L0_PORT        MOD_SOM_J9_22_PORT
#define MOD_SOM_J9_22_WTIM2_CC2_L0_PIN        MOD_SOM_J9_22_PIN
#define MOD_SOM_J11_13_WTIM2_CC2_L0_PORT    MOD_SOM_J11_13_PORT
#define MOD_SOM_J11_13_WTIM2_CC2_L0_PIN        MOD_SOM_J11_13_PIN

#define MOD_SOM_J9_22_LETIM1_OUT0_L1_PORT    MOD_SOM_J9_22_PORT
#define MOD_SOM_J9_22_LETIM1_OUT0_L1_PIN    MOD_SOM_J9_22_PIN
#define MOD_SOM_J11_13_LETIM1_OUT0_L1_PORT    MOD_SOM_J11_13_PORT
#define MOD_SOM_J11_13_LETIM1_OUT0_L1_PIN    MOD_SOM_J11_13_PIN

#define MOD_SOM_J9_22_US2_CTS_L2_PORT        MOD_SOM_J9_22_PORT
#define MOD_SOM_J9_22_US2_CTS_L2_PIN        MOD_SOM_J9_22_PIN
#define MOD_SOM_J11_13_US2_CTS_L2_PORT        MOD_SOM_J11_13_PORT
#define MOD_SOM_J11_13_US2_CTS_L2_PIN        MOD_SOM_J11_13_PIN

#define MOD_SOM_J9_22_PRS_CH11_L0_PORT        MOD_SOM_J9_22_PORT
#define MOD_SOM_J9_22_PRS_CH11_L0_PIN        MOD_SOM_J9_22_PIN
#define MOD_SOM_J11_13_PRS_CH11_L0_PORT        MOD_SOM_J11_13_PORT
#define MOD_SOM_J11_13_PRS_CH11_L0_PIN        MOD_SOM_J11_13_PIN





// Function: Reed Switch
// TODO describe functionality
// This signal is present on the J13 Connector and Reed Switch.
#define MOD_SOM_SW1_TIM3_CC0_L6_PORT        MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_TIM3_CC0_L6_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_TIM3_CC0_L6_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_TIM3_CC0_L6_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_WTIM0_CC0_L1_PORT        MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_WTIM0_CC0_L1_PIN        MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_WTIM0_CC0_L1_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_WTIM0_CC0_L1_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_TIM1_OUT1_L0_PORT        MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_TIM1_OUT1_L0_PIN        MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_TIM1_OUT1_L0_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_TIM1_OUT1_L0_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_PCNT1_S1IN_L0_PORT        MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_PCNT1_S1IN_L0_PIN        MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_PCNT1_S1IN_L0_PORT    MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_PCNT1_S1IN_L0_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_SDIO_CD_L2_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_SDIO_CD_L2_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_SDIO_CD_L2_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_SDIO_CD_L2_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_US5_TX_L1_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_US5_TX_L1_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_US5_TX_L1_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_US5_TX_L1_PIN            MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_U0_RTS_L2_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_U0_RTS_L2_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_U0_RTS_L2_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_U0_RTS_L2_PIN            MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_LEU1_RX_L1_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_LEU1_RX_L1_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_LEU1_RX_L1_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_LEU1_RX_L1_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_PRS_CH6_L0_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_PRS_CH6_L0_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_PRS_CH6_L0_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_PRS_CH6_L0_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_ACMP0_O_L4_PORT            MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_ACMP0_O_L4_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_ACMP0_O_L4_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_ACMP0_O_L4_PIN        MOD_SOM_J13_2_PIN

#define MOD_SOM_SW1_ETM_TCLK_L3_PORT        MOD_SOM_SW1_PORT
#define MOD_SOM_SW1_ETM_TCLK_L3_PIN            MOD_SOM_SW1_PIN
#define MOD_SOM_J13_2_ETM_TCLK_L3_PORT        MOD_SOM_J13_2_PORT
#define MOD_SOM_J13_2_ETM_TCLK_L3_PIN        MOD_SOM_J13_2_PIN



// Function: Debug-Interface
// TODO describe functionality
// DBG_SWCLK
#define MOD_SOM_J8_4_TIM0_CC0_L4_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_TIM0_CC0_L4_PIN        MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_WTIM0_CC1_L4_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_WTIM0_CC1_L4_PIN        MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_LETIM0_OUT0_L2_PORT    MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_LETIM0_OUT0_L2_PIN        MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_US2_TX_L5_PORT            MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_US2_TX_L5_PIN            MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_CAN0_RX_L1_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_CAN0_RX_L1_PIN            MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_US1_CLK_L2_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_US1_CLK_L2_PIN            MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_LEU0_TX_L3_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_LEU0_TX_L3_PIN            MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_I2C0_SDA_L5_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_I2C0_SDA_L5_PIN        MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_PRS_CH15_L2_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_PRS_CH15_L2_PIN        MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_ACMP3_O_L0_PORT        MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_ACMP3_O_L0_PIN            MOD_SOM_J8_4_PIN

#define MOD_SOM_J8_4_BOOT_TX_PORT            MOD_SOM_J8_4_PORT
#define MOD_SOM_J8_4_BOOT_TX_PIN            MOD_SOM_J8_4_PIN


// DBG_SWDIO
#define MOD_SOM_J8_2_TIM0_CC1_L4_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_TIM0_CC1_L4_PIN        MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_WTIM0_CC2_L4_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_WTIM0_CC2_L4_PIN        MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_LETIM0_OUT1_L2_PORT    MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_LETIM0_OUT1_L2_PIN        MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_US2_RX_L5_PORT            MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_US2_RX_L5_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_CAN1_RX_L1_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_CAN1_RX_L1_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_US1_CS_L2_PORT            MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_US1_CS_L2_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_U0_TX_L5_PORT            MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_U0_TX_L5_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_LEU0_RX_L3_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_LEU0_RX_L3_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_I2C0_SCL_L5_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_I2C0_SCL_L5_PIN        MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_PRS_CH4_L2_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_PRS_CH4_L2_PIN            MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_GPIO_EM4WU3_PORT        MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_GPIO_EM4WU3_PIN        MOD_SOM_J8_2_PIN

#define MOD_SOM_J8_2_BOOT_RX_PORT            MOD_SOM_J8_2_PORT
#define MOD_SOM_J8_2_BOOT_RX_PIN            MOD_SOM_J8_2_PIN


// DBG_SWO
#define MOD_SOM_J8_6_TIM0_CC2_L4_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_TIM0_CC2_L4_PIN        MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_TIM1_CC0_L5_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_TIM1_CC0_L5_PIN        MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_TIM2_CC0_L3_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_TIM2_CC0_L3_PIN        MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_US2_CLK_L5_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_US2_CLK_L5_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_CAN0_TX_L1_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_CAN0_TX_L1_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_US1_TX_L5_PORT            MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_US1_TX_L5_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_U0_RX_L5_PORT            MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_U0_RX_L5_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_LEU0_TX_L4_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_LEU0_TX_L4_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_I2C1_SCL_L4_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_I2C1_SCL_L4_PIN        MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_CMU_CLK0_L4_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_CMU_CLK0_L4_PIN        MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_PRS_CH0_L3_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_PRS_CH0_L3_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_ACMP1_O_L0_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_ACMP1_O_L0_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_DBG_TDO_PORT            MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_DBG_TDO_PIN            MOD_SOM_J8_6_PIN

#define MOD_SOM_J8_6_GPIO_EM4WU4_PORT        MOD_SOM_J8_6_PORT
#define MOD_SOM_J8_6_GPIO_EM4WU4_PIN        MOD_SOM_J8_6_PIN




// Function: LED
// This signal is present on the J9 Mezzanine Header and J1 Main Connector.
// TODO describe functionality
#define MOD_SOM_J9_20_WTIM1_CC3_L2_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_WTIM1_CC3_L2_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_WTIM1_CC3_L2_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_WTIM1_CC3_L2_PIN            MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_US0_RTS_L2_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_US0_RTS_L2_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_US0_RTS_L2_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_US0_RTS_L2_PIN                MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_US1_CTS_L3_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_US1_CTS_L3_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_US1_CTS_L3_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_US1_CTS_L3_PIN                MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_LEU1_TX_L0_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_LEU1_TX_L0_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_LEU1_TX_L0_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_LEU1_TX_L0_PIN                MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_I2C0_SDA_L2_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_I2C0_SDA_L2_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_I2C0_SDA_L2_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_I2C0_SDA_L2_PIN            MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_LES_CH6_PORT                MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_LES_CH6_PIN                MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_LES_CH6_PORT                MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_LES_CH6_PIN                MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_PRS_CH14_L1_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_PRS_CH14_L1_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_PRS_CH14_L1_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_PRS_CH14_L1_PIN            MOD_SOM_J1_6_PIN

#define MOD_SOM_J9_20_ETM_TCLK_L2_PORT            MOD_SOM_J9_20_PORT
#define MOD_SOM_J9_20_ETM_TCLK_L2_PIN            MOD_SOM_J9_20_PIN
#define MOD_SOM_J1_6_ETM_TCLK_L2_PORT            MOD_SOM_J1_6_PORT
#define MOD_SOM_J1_6_ETM_TCLK_L2_PIN            MOD_SOM_J1_6_PIN




// Function: Backup Battery
// BU_VIN
#define MOD_SOM_BT1_1_WTIM1_CC2_L2_PORT        MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_WTIM1_CC2_L2_PIN        MOD_SOM_BT1_1_PIN

#define MOD_SOM_BT1_1_US2_RTS_L5_PORT        MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_US2_RTS_L5_PIN        MOD_SOM_BT1_1_PIN

#define MOD_SOM_BT1_1_CMU_CLK1_L1_PORT        MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_CMU_CLK1_L1_PIN        MOD_SOM_BT1_1_PIN

#define MOD_SOM_BT1_1_PRS_CH12_L2_PORT        MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_PRS_CH12_L2_PIN        MOD_SOM_BT1_1_PIN

#define MOD_SOM_BT1_1_ACMP2_O_L0_PORT        MOD_SOM_BT1_1_PORT
#define MOD_SOM_BT1_1_ACMP2_O_L0_PIN        MOD_SOM_BT1_1_PIN

// BU_VOUT
#define MOD_SOM_U3_8_TIM3_CC2_L1_PORT        MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_TIM3_CC2_L1_PIN        MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_WTIM1_CC3_L3_PORT        MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_WTIM1_CC3_L3_PIN        MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_US0_RTS_L1_PORT        MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_US0_RTS_L1_PIN            MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_U0_CTS_L1_PORT            MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_U0_CTS_L1_PIN            MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_U1_TX_L3_PORT            MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_U1_TX_L3_PIN            MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_PRS_CH20_L2_PORT        MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_PRS_CH20_L2_PIN        MOD_SOM_U3_8_PIN

#define MOD_SOM_U3_8_ACMP0_O_L1_PORT        MOD_SOM_U3_8_PORT
#define MOD_SOM_U3_8_ACMP0_O_L1_PIN            MOD_SOM_U3_8_PIN





// Function: AUX_COM2_TTL
// Primary Signal Group: USART3
// TTL level SPI signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_29_US1_TX_L2_PORT        MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_US1_TX_L2_PIN            MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_U0_TX_L6_PORT            MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_U0_TX_L6_PIN            MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_I2C0_SCL_L1_PORT        MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_I2C0_SCL_L1_PIN        MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_TIM1_CC1_L4_PORT        MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_TIM1_CC1_L4_PIN        MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_WTIM1_CC1_L2_PORT        MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_WTIM1_CC1_L2_PIN        MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_LETIM0_OUT1_L0_PORT    MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_LETIM0_OUT1_L0_PIN    MOD_SOM_J9_29_PIN
#define MOD_SOM_J9_29_BUSADC0Y_PORT            MOD_SOM_J9_29_PORT
#define MOD_SOM_J9_29_BUSADC0Y_PIN            MOD_SOM_J9_29_PIN

#define MOD_SOM_J9_31_US0_CS_L1_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_US0_CS_L1_PIN            MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_US1_CS_L5_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_US1_CS_L5_PIN            MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_U0_RX_L6_PORT            MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_U0_RX_L6_PIN            MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_I2C0_SDA_L7_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_I2C0_SDA_L7_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_TIM3_CC1_L2_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_TIM3_CC1_L2_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_TIM5_CC0_L0_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_TIM5_CC0_L0_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_TIM6_CTDI1_L2_PORT    MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_TIM6_CTDI1_L2_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_WTIM0_CC0_L0_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_WTIM0_CC0_L0_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_WTIM1_CC1_L4_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_WTIM1_CC1_L4_PIN        MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_BUSCX_PORT            MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_BUSCX_PIN                MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_BUSDY_PORT            MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_BUSDY_PIN                MOD_SOM_J9_31_PIN
#define MOD_SOM_J9_31_PRS_CH16_L2_PORT        MOD_SOM_J9_31_PORT
#define MOD_SOM_J9_31_PRS_CH16_L2_PIN        MOD_SOM_J9_31_PIN

#define MOD_SOM_J9_26_US0_RX_L1_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_US0_RX_L1_PIN            MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_TIM3_CC1_L3_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_TIM3_CC1_L3_PIN        MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_TIM5_CC2_L0_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_TIM5_CC2_L0_PIN        MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_TIM6_CTDI2_L2_PORT    MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_TIM6_CTDI2_L2_PIN        MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_WTIM0_CC2_L0_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_WTIM0_CC2_L0_PIN        MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_WTIM1_CC3_L4_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_WTIM1_CC3_L4_PIN        MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_BUSCX_PORT            MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_BUSCX_PIN                MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_BUSDY_PORT            MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_BUSDY_PIN                MOD_SOM_J9_26_PIN
#define MOD_SOM_J9_26_PRS_CH6_L2_PORT        MOD_SOM_J9_26_PORT
#define MOD_SOM_J9_26_PRS_CH6_L2_PIN        MOD_SOM_J9_26_PIN

#define MOD_SOM_J9_27_US0_TX_L1_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_US0_TX_L1_PIN            MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_TIM3_CC2_L3_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_TIM3_CC2_L3_PIN        MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_TIM5_CC0_L1_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_TIM5_CC0_L1_PIN        MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_WTIM1_CC0_L5_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_WTIM1_CC0_L5_PIN        MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_BUSDX_PORT            MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_BUSDX_PIN                MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_BUSCY_PORT            MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_BUSCY_PIN                MOD_SOM_J9_27_PIN
#define MOD_SOM_J9_27_PRS_CH7_L2_PORT        MOD_SOM_J9_27_PORT
#define MOD_SOM_J9_27_PRS_CH7_L2_PIN        MOD_SOM_J9_27_PIN



// Function: AUX_COM3_TTL
// Primary Signal Group: I2C1
// TTL level I2C signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_35_U0_RX_L1_PORT            MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_U0_RX_L1_PIN            MOD_SOM_J9_35_PIN
#define MOD_SOM_J9_35_PRS_CH23_L1_PORT        MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_PRS_CH23_L1_PIN        MOD_SOM_J9_35_PIN
#define MOD_SOM_J9_35_TIM3_CC1_L1_PORT        MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_TIM3_CC1_L1_PIN        MOD_SOM_J9_35_PIN
#define MOD_SOM_J9_35_WTIM1_CC2_L3_PORT        MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_WTIM1_CC2_L3_PIN        MOD_SOM_J9_35_PIN
#define MOD_SOM_J9_35_BUSCY_PORT            MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_BUSCY_PIN                MOD_SOM_J9_35_PIN
#define MOD_SOM_J9_35_BUSDX_PORT            MOD_SOM_J9_35_PORT
#define MOD_SOM_J9_35_BUSDX_PIN                MOD_SOM_J9_35_PIN

#define MOD_SOM_J9_37_U0_TX_L1_PORT            MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_U0_TX_L1_PIN            MOD_SOM_J9_37_PIN
#define MOD_SOM_J9_37_PRS_CH22_L1_PORT        MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_PRS_CH22_L1_PIN        MOD_SOM_J9_37_PIN
#define MOD_SOM_J9_37_TIM3_CC0_L1_PORT        MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_TIM3_CC0_L1_PIN        MOD_SOM_J9_37_PIN
#define MOD_SOM_J9_37_WTIM1_CC1_L3_PORT        MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_WTIM1_CC1_L3_PIN        MOD_SOM_J9_37_PIN
#define MOD_SOM_J9_37_BUSCX_PORT            MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_BUSCX_PIN                MOD_SOM_J9_37_PIN
#define MOD_SOM_J9_37_BUSDY_PORT            MOD_SOM_J9_37_PORT
#define MOD_SOM_J9_37_BUSDY_PIN                MOD_SOM_J9_37_PIN



// Function: AUX_COM_RS232
// Primary Signal Group: Low Energy UART0
// RS232 level RX and TX signals are available on the mezzanine
// header J9 and main power connector J1.
// Before using this function, the RS232 driver IC must be powered
// up with the URT_EN signal.  After power-up, the IC is enabled
// with the SBE_EN signal.
#define MOD_SOM_J9_45_I2C1_SCL_L3_PORT         MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_I2C1_SCL_L3_PIN         MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_PRS_CH11_L2_PORT         MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_PRS_CH11_L2_PIN         MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_TIM6_CC1_L7_PORT         MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_TIM6_CC1_L7_PIN         MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_WTIM0_CTDI1_L4_PORT     MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_WTIM0_CTDI1_L4_PIN     MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_WTIM3_CC3_L1_PORT        MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_WTIM3_CC3_L1_PIN        MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_WTIM2_CC2_L5_PORT        MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_WTIM2_CC2_L5_PIN        MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_BUSADC0Y_PORT         MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_BUSADC0Y_PIN             MOD_SOM_J9_45_PIN
#define MOD_SOM_J9_45_BUSADC0X_PORT         MOD_SOM_J9_45_PORT
#define MOD_SOM_J9_45_BUSADC0X_PIN             MOD_SOM_J9_45_PIN

#define MOD_SOM_J1_9_I2C1_SCL_L3_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_I2C1_SCL_L3_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_PRS_CH11_L2_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_PRS_CH11_L2_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_TIM6_CC1_L7_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_TIM6_CC1_L7_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_WTIM0_CTDI1_L4_PORT    MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_WTIM0_CTDI1_L4_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_WTIM3_CC3_L1_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_WTIM3_CC3_L1_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_WTIM2_CC2_L5_PORT        MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_WTIM2_CC2_L5_PIN        MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_BUSADC0Y_PORT            MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_BUSADC0Y_PIN            MOD_SOM_J1_9_PIN
#define MOD_SOM_J1_9_BUSADC0X_PORT            MOD_SOM_J1_9_PORT
#define MOD_SOM_J1_9_BUSADC0X_PIN            MOD_SOM_J1_9_PIN

#define MOD_SOM_J9_41_I2C1_SDA_L3_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_I2C1_SDA_L3_PIN         MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_US3_CLK_L2_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_US3_CLK_L2_PIN         MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_PRS_CH10_L2_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_PRS_CH10_L2_PIN         MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_TIM6_CC0_L7_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_TIM6_CC0_L7_PIN         MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_WTIM0_CTDI0_L4_PORT     MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_WTIM0_CTDI0_L4_PIN     MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_WTIM2_CC1_L5_PORT        MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_WTIM2_CC1_L5_PIN        MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_WTIM1_CC2_L1_PORT        MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_WTIM1_CC2_L1_PIN        MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_BUSADC0Y_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_BUSADC0Y_PIN             MOD_SOM_J9_41_PIN
#define MOD_SOM_J9_41_BUSADC0X_PORT         MOD_SOM_J9_41_PORT
#define MOD_SOM_J9_41_BUSADC0X_PIN             MOD_SOM_J9_41_PIN

#define MOD_SOM_J1_10_I2C1_SDA_L3_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_I2C1_SDA_L3_PIN         MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_US3_CLK_L2_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_US3_CLK_L2_PIN         MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_PRS_CH10_L2_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_PRS_CH10_L2_PIN         MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_TIM6_CC0_L7_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_TIM6_CC0_L7_PIN         MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_WTIM0_CTDI0_L4_PORT     MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_WTIM0_CTDI0_L4_PIN     MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_WTIM2_CC1_L5_PORT        MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_WTIM2_CC1_L5_PIN        MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_WTIM1_CC2_L1_PORT        MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_WTIM1_CC2_L1_PIN        MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_BUSADC0Y_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_BUSADC0Y_PIN             MOD_SOM_J1_10_PIN
#define MOD_SOM_J1_10_BUSADC0X_PORT         MOD_SOM_J1_10_PORT
#define MOD_SOM_J1_10_BUSADC0X_PIN             MOD_SOM_J1_10_PIN






// Function: AUX_COM4_TTL
// Primary Signal Group: Low Energy UART1
// TTL level RX & TX signals are available on the mezzanine
// header J9.
// No control signals external to the microcontroller are
// required to use this function.
#define MOD_SOM_J9_16_US1_CS_L1_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_US1_CS_L1_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_TIM4_CDTI2_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_TIM4_CDTI2_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_TIM0_CC2_L2_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_TIM0_CC2_L2_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_TIM6_CC2_L6_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_TIM6_CC2_L6_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_WTIM1_CC1_L1_PORT     MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_WTIM1_CC1_L1_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_WTIM2_CC0_L5_PORT     MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_WTIM2_CC0_L5_PIN         MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_BUSADC0Y_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_BUSADC0Y_PIN             MOD_SOM_J9_16_PIN
#define MOD_SOM_J9_16_BUSADC0X_PORT         MOD_SOM_J9_16_PORT
#define MOD_SOM_J9_16_BUSADC0X_PIN             MOD_SOM_J9_16_PIN

#define MOD_SOM_J9_14_US1_CLK_L1_PORT         MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_US1_CLK_L1_PIN         MOD_SOM_J9_14_PIN
#define MOD_SOM_J9_14_TIM0_CC1_L2_PORT         MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_TIM0_CC1_L2_PIN         MOD_SOM_J9_14_PIN
#define MOD_SOM_J9_14_TIM6_CC1_L6_PORT         MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_TIM6_CC1_L6_PIN         MOD_SOM_J9_14_PIN
#define MOD_SOM_J9_14_WTIM1_CC0_L1_PORT     MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_WTIM1_CC0_L1_PIN         MOD_SOM_J9_14_PIN
#define MOD_SOM_J9_14_BUSADC0Y_PORT         MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_BUSADC0Y_PIN             MOD_SOM_J9_14_PIN
#define MOD_SOM_J9_14_BUSADC0X_PORT         MOD_SOM_J9_14_PORT
#define MOD_SOM_J9_14_BUSADC0X_PIN             MOD_SOM_J9_14_PIN













#endif /* SRC_MOD_SOM_BSP_H_ */
