/*
 * mod_som_sbe49_obp.c
 *
 * Function allowing for the computation of pressure temp salinity sound speed
 * from a SBE49.
 *
 * most of it comes from the SOM-acq written by San
 *
 *
 *  Created on: Apr 15, 2021
 *      Author: aleboyer
 */


#include <mod_som_sbe49_obp.h>
#include <math.h>


/*
 Model: FAST CAT CTD 49
 CTD's data format:  Raw data in Hexadecimal - 24 chars: 22 ASCII chars (hex) + 2 chars (carriage return & line feed) : 11 bytes per scan
 example: ttttttccccccppppppvvvv = 0A53711BC7220C14C17D82
 Temperature = tttttt = 0A5371 (676721 decimal)
 tempeatrue A/D counts = 676721
    Conductivity = cccccc = 1BC722 (1820450)
    conductivity frequency = 1820450/256 = 7111.133 Hz
    Pressure = pppppp = 0C14C1 (791745 decimal)
    pressure A/D counts = 791745
    Pressure temperature compensation = vvvv = 7D82 (32,130 decimal)
    pressure temperature = 32,130 / 13,107 = 2.4514 volts

 Model: 911 CTD:
 There are 12 words per scan, 3 bytes/word -> 36 bytes/scan.
    Word 0    Byte 0 -> 2:    Primary Temperature
    Word 1    Byte 3 -> 5:    Primary Conductivity
    Word 2    Byte 6 -> 8:    Pressure
    Word 11    Byte 33       :    Pressure Sensor Temperature MSBs
 Byte 34       :    4 MSB = Pressure Sensor Temperature LSBs
 CALCULATION:    ( perform in ConvertBinData() )
 1. Frequencies:
    FT (Temperature) = Byte(0)*256 + Byte(1) + Byte(2)/256        (Hz)
    FC (Temperature) = Byte(3)*256 + Byte(4) + Byte(5)/256        (Hz)
    FP (Temperature) = Byte(6)*256 + Byte(7) + Byte(8)/256        (Hz)
 2. Temperature:
 T = 1/{Ta+Tb*[ln(Tfo/FT)]+Tc*[ln(Tfo/FT)]^2+Td*[ln(Tfo/FT)]^3} - 273.15    (˚C)
 3. Pressure:
    Pressure Temperature Compensation:
 12-bit pressure temperature compensation word =
 Byte(33)*16 + Byte(34)/16
 U = M * (12-bit pressure temperature compensation word) + B        (˚C)
 Pc = Pc1 + Pc2*U + Pc3*U^2
 Pd = Pd1 + Pd2*U
 Pto = Pt1 + Pt2*U + Pt3*U^2 + Pt4*U^3 + Pt5*U^4        (µsec)

 freq = (Pto/FT)^2
 P = Pc*[1 - freq]*[1 - Pd*(1 - freq)]                (psia)

 or the other way of calculation: (we use this following formular)

 freq = (Pto*FT)^2*1e-12;                1e-12: convert from µsec -> sec
 P = {Pc*(1 - freq)*[1 - Pd*(1 - freq)]}/1.47 - 10    (dbar)
 1.47 = convert from psia to decibar
 10   = offset, the pressure at the surface of the ocean = 10

 4. Conductivity:
 C = (Ca*FC^Cm + Cb*FC^2 + Cc + Cd*T)/[10(1 + CPCor*P)]

 */

float mod_som_sbe49_calculate_temp(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, long temp_sample)
{
    float MV, R;
    float temp = 0.0;

    MV = (temp_sample - 524288)/1.6e+007;
    R = (MV * 2.295e+10 + 9.216e+8) / (6.144e+4 - MV*5.3e+5);
    temp = 1/( cal_coef->ta0
              + cal_coef->ta1*log(R)
              + cal_coef->ta2*log(R)*log(R)
              + cal_coef->ta3*log(R)*log(R)*log(R)) - 273.15;
    return temp;
}

//float mod_som_sbe49_calculate_press(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, long press_sample, long presstemp_comp)
//{
//    float y = 0.0, t = 0.0, x = 0.0, n = 0.0;
//    float press = 0.0;
//
//    y = press_sample/13107;
//    //JMK 29Aug06 fixed to be a0+T*a1+T^2*a2
//    t = cal_coef->ptempa0 + cal_coef->ptempa1*y + cal_coef->ptempa2*y*y;
//    x = presstemp_comp - cal_coef->ptca0 - cal_coef->ptca1*t- cal_coef->ptca2*t*t;
//    n = x*cal_coef->ptcb0/(cal_coef->ptcb0+ cal_coef->ptcb1*t+ cal_coef->ptcb2*t*t);
//    press = cal_coef->pa0 + cal_coef->pa1*n+ cal_coef->pa2*n*n;
//    press = (press - 14.6959488) * 0.689476;  // convert psia to decibars
//    return press;
//}
float mod_som_sbe49_calculate_press(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, long press_sample, long presstemp_comp)
{
    float y = 0.0, t = 0.0, x = 0.0, n = 0.0;
    float press = 0.0;
    //MHA: these incorrectly have pressure and tcomp switched. Fixed below.
//    y = press_sample/13107;
//    //JMK 29Aug06 fixed to be a0+T*a1+T^2*a2
//    t = cal_coef->ptempa0 + cal_coef->ptempa1*y + cal_coef->ptempa2*y*y;
//    x = presstemp_comp - cal_coef->ptca0 - cal_coef->ptca1*t- cal_coef->ptca2*t*t;
//    n = x*cal_coef->ptcb0/(cal_coef->ptcb0+ cal_coef->ptcb1*t+ cal_coef->ptcb2*t*t);
//    press = cal_coef->pa0 + cal_coef->pa1*n+ cal_coef->pa2*n*n;
//    press = (press - 14.6959488) * 0.689476;  // convert psia to decibars

    y = presstemp_comp/13107;
    //JMK 29Aug06 fixed to be a0+T*a1+T^2*a2
    t = cal_coef->ptempa0 + cal_coef->ptempa1*y + cal_coef->ptempa2*y*y;
    x = press_sample - cal_coef->ptca0 - cal_coef->ptca1*t- cal_coef->ptca2*t*t;
    n = x*cal_coef->ptcb0/(cal_coef->ptcb0+ cal_coef->ptcb1*t+ cal_coef->ptcb2*t*t);
    press = cal_coef->pa0 + cal_coef->pa1*n+ cal_coef->pa2*n*n;
    press = (press - 14.6959488) * 0.689476;  // convert psia to decibars


    return press;
}

float mod_som_sbe49_calculate_cond(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef, float temp, float press, long condInHex)
{
    float condFreq = 0.0, f= 0.0, cond = 0.0, cond1 = 0.0, cond2 = 0.0;

    condFreq = condInHex/256;
    f = condFreq/1000.0;
    // in Siemens/meter unit
    cond1 = ( cal_coef->cg+ cal_coef->ch*f*f+ cal_coef->ci*f*f*f+ cal_coef->cj*f*f*f*f);
    cond2 = 1 + cal_coef->ctcor*temp + cal_coef->cpcor*press;
    cond = cond1/cond2;
    return cond;
}

float mod_som_sbe49_calculate_soundvel(mod_som_sbe49_obp_cal_coef_ptr_t cal_coef,float C, float T, float P)
{
//     float sound_vel;
     P /= 10;  // convert db to bars as used in UNESCO routines

     //------------
     // eqn 34 p.46
     //------------
     float c00 = 1402.388;
     float c01 =    5.03711;
     float c02 =   -5.80852e-2;
     float c03 =    3.3420e-4;
     float c04 =   -1.47800e-6;
     float c05 =    3.1464e-9;

     float c10 =  0.153563;
     float c11 =  6.8982e-4;
     float c12 = -8.1788e-6;
     float c13 =  1.3621e-7;
     float c14 = -6.1185e-10;

     float c20 =  3.1260e-5;
     float c21 = -1.7107e-6;
     float c22 =  2.5974e-8;
     float c23 = -2.5335e-10;
     float c24 =  1.0405e-12;

     float c30 = -9.7729e-9;
     float c31 =  3.8504e-10;
     float c32 = -2.3643e-12;

     float Cw =    c00 + (c01 + (c02 + (c03 + (c04 + c05*T)*T)*T)*T)*T;
     Cw += (c10 + (c11 + (c12 + (c13 + c14*T)*T)*T)*T)*P;
     Cw +=(c20 + (c21 + (c22 + (c23 + c24*T)*T)*T)*T)*P*P;
     Cw += (c30 + c31*T + c32*T*T)*P*P*P;

     //-------------
     // eqn 35. p.47
     //-------------
     float a00 =  1.389;
     float a01 = -1.262e-2;
     float a02 =  7.164e-5;
     float a03 =  2.006e-6;
     float a04 = -3.21e-8;

     float a10 =  9.4742e-5;
     float a11 = -1.2580e-5;
     float a12 = -6.4885e-8;
     float a13 =  1.0507e-8;
     float a14 = -2.0122e-10;

     float a20 = -3.9064e-7;
     float a21 =  9.1041e-9;
     float a22 = -1.6002e-10;
     float a23 =  7.988e-12;

     float a30 =  1.100e-10;
     float a31 =  6.649e-12;
     float a32 = -3.389e-13;

     float A = a00 + a01*T + a02*T*T + a03*T*T*T + a04*T*T*T*T;
     A += (a10 + a11*T + a12*T*T + a13*T*T*T + a14*T*T*T*T)*P;
     A += (a20 + a21*T + a22*T*T + a23*T*T*T)*P*P;
     A +=(a30 + a31*T + a32*T*T)*P*P*P;


     //------------
     // eqn 36 p.47
     //------------
     float b00 = -1.922e-2;
     float b01 = -4.42e-5;
     float b10 =  7.3637e-5;
     float b11 =  1.7945e-7;

     float B = b00 + b01*T + (b10 + b11*T)*P;
     //------------
     // eqn 37 p.47
     //------------
     float d00 =  1.727e-3;
     float d10 = -7.9836e-6;

     float D = d00 + d10*P;

     float S = mod_som_sbe49_calculate_salt(C, T, P);

     //------------
     // eqn 33 p.46
     //------------
     float svel = Cw + A*S + B*S*sqrt(S) + D*S*S;

    return svel;
 }

 float mod_som_sbe49_calculate_salt(float C,float T, float P){
     float c3515 = 42.914;

     float R = C*10/c3515;

     //sw_salrt
     float c0 =  0.6766097;
     float c1 =  2.00564e-2;
     float c2 =  1.104259e-4;
     float c3 = -6.9698e-7;
     float c4 =  1.0031e-9;

     float rt = c0 + (c1 + (c2 + (c3 + c4*T)*T)*T)*T;

     //sw_salrp
     float d1 =  3.426e-2;
     float d2 =  4.464e-4;
     float d3 =  4.215e-1;
     float d4 = -3.107e-3;

     float e1 =  2.070e-5;
     float e2 = -6.370e-10;
     float e3 =  3.989e-15;

     float Rp = 1 + ( P*(e1 + e2*P + e3*P*P) )/(1 + d1*T + d2*T*T +(d3 + d4*T)*R);

     float Rt = R/(Rp*rt);

     //sw_sals
     float a0 =  0.0080;
     float a1 = -0.1692;
     float a2 = 25.3851;
     float a3 = 14.0941;
     float a4 = -7.0261;
     float a5 =  2.7081;

     float b0 =  0.0005;
     float b1 = -0.0056;
     float b2 = -0.0066;
     float b3 = -0.0375;
     float b4 =  0.0636;
     float b5 = -0.0144;

     float k  =  0.0162;

     float Rtx   = sqrt(Rt);
     float del_T = T - 15;
     float del_S = (del_T/(1+k*del_T))*(b0 + (b1 + (b2+ (b3 + (b4 + b5*Rtx)*Rtx)*Rtx)*Rtx)*Rtx);

     float S = a0 + (a1 + (a2 + (a3 + (a4 + a5*Rtx)*Rtx)*Rtx)*Rtx)*Rtx;

     S = S + del_S;

     return S;
 }



uint32_t hex2int(char *hex);
uint32_t hex2int(char *hex)
{
    uint32_t val = 0;
    while (*hex)
    {
        uint8_t byte = *hex++;
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
        val = (val << 4) | (byte & 0xf);
    }
    return val;
}
// take the string with the
uint32_t hex2int_n(char *hex, uint32_t numChar)
{
    uint32_t val = 0;
    int i =0;
    for (i =0; i< numChar; i++)
    {
        uint8_t byte = *hex++;
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
        val = (val << 4) | (byte & 0xf);
    }
    return val;
}


//Parsing the SBE sample
#define TCP_NUM_DIGIT 6 // see the SBE49 manual: outpuFormat = 0 (raw data in Hexadecimal)
#define PTComp_NUM_DIGIT   4 // Pressure temperature compsensation = vvvv
void mod_som_sbe49_obp_parsing_sample(char * sbe_sample,
                                      mod_som_sbe49_obp_cal_coef_ptr_t cal_coef,
                                      float * pressure,
                                      float * temperature,
                                      float * conductivity,
                                      float * salinity)
{
  int indx, j;
  long val = 0, tempInDec, pressInDec, condInDec, pressTempInDec;
  int sbe_indx = 0;
  char *strPtr = NULL;

  // *** convert data: temp, cond, press in hex to decimal
  // read the SBE's data and convert it into the engineer unit
  // SBE's format: ttttttccccccppppppvvvv
  // TCP_NUM_DIGIT = 6, PTComp_NUM_DIGIT = 4
  for(j=0; j<4; j++)
    {
      indx = sbe_indx + TCP_NUM_DIGIT*j;  //start at sbe data
      strPtr = sbe_sample+indx;
      //   printf("strPtr: %d: %s",j, strPtr);
      // ADDED mnbui 12Mar2021: convert 6 or 4 characters to get temperature, cond & pressure
      if (j<3)    // for temp,cond,pressure
        val = hex2int_n(strPtr, TCP_NUM_DIGIT);
      else
        val = hex2int_n(strPtr, PTComp_NUM_DIGIT);
      switch(j)
      {
        case 0:
          tempInDec = val;
          // printf("temp = %ld\n",val);
          break;
        case 1:
          condInDec = val;
          //  printf("cond = %ld\n",val);
          break;
        case 2:
          pressInDec = val;
          //   printf("press = %ld\n",val);
          // define whether fish is up or down.
          break;
        case 3:
          pressTempInDec = val;
          break;
      }   // end of switch(j)
    }   // end of for(j=0; j<4; j++)

  // *** Calculate temp, press, cond with engineer unit
  *temperature = mod_som_sbe49_calculate_temp(cal_coef, tempInDec);

  *pressure = mod_som_sbe49_calculate_press
      (cal_coef, pressInDec, pressTempInDec);

  *conductivity = mod_som_sbe49_calculate_cond
      (cal_coef, *temperature, *pressure, condInDec);
  *salinity = mod_som_sbe49_calculate_salt(*conductivity,*temperature,*pressure);
}

