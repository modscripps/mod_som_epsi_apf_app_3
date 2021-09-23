/*
 * mod_som_sbe49_obp.h
 *
 *  Created on: Apr 15, 2021
 *      Author: aleboyer
 */

#ifndef MOD_SBE49_MOD_SOM_SBE49_OBP_H_
#define MOD_SBE49_MOD_SOM_SBE49_OBP_H_

#include <mod_som_common.h>


#define MOD_SOM_SBE49_OBP_PTC_DEPTH 0
#define MOD_SOM_SBE49_OBP_CTC_DEPTH 0


// public typedef structure

typedef struct {
    char serialnum[25];
    char tcalDate[25];
    float ta0;
    float ta1;
    float ta2;
    float ta3;
    char ccalDate[25];
    float cg;
    float ch;
    float ci;
    float cj;
    float ctcor;
    float cpcor;
    char pcalDate[25];
    float pa0;
    float pa1;
    float pa2;
    float ptca0;
    float ptca1;
    float ptca2;
    float ptcb0;
    float ptcb1;
    float ptcb2;
    float ptempa0;
    float ptempa1;
    float ptempa2;
}mod_som_sbe49_obp_cal_coef_t, *mod_som_sbe49_obp_cal_coef_ptr_t;

typedef struct {
  uint64_t timestamp;
  float temperature;
  float pressure;
  float conductivity;
  float salinity;
  uint32_t ptc_depth;
  uint32_t ctc_depth;
  mod_som_sbe49_obp_cal_coef_ptr_t cal_coef;

}mod_som_sbe49_obp_t, *mod_som_sbe49_obp_ptr_t;


float mod_som_sbe49_calculate_temp(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, long hex_temp_sample);
float mod_som_sbe49_calculate_press(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, long hex_press_sample, long hex_presstemp_comp);
float mod_som_sbe49_calculate_cond(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, float temp, float press, long condInHex);
float mod_som_sbe49_calculate_soundvel(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef,float C, float T, float P);
float mod_som_sbe49_calculate_salt(float C,float T, float P);
void  mod_som_sbe49_obp_parsing_sample(char * sbe_sample,
                                       mod_som_sbe49_obp_cal_coef_ptr_t cal_coef,
                                       float * pressure,
                                       float * temperature,
                                       float * conductivity,
                                       float * salinity);


#endif /* MOD_SBE49_MOD_SOM_SBE49_OBP_H_ */
