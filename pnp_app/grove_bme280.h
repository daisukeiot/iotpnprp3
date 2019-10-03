#ifndef GROVE_BME280_H_
#define GROVE_BME280_H_

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define BME280_I2CADDR 0x76
#define BME280_CHIPID  0xD0

/* BME280 Registers */
#define BME280_REG_DIG_T1 0x88  /* R   Unsigned Calibration data (16 bits) */
#define BME280_REG_DIG_T2 0x8A  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_T3 0x8C  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P1 0x8E  /* R   Unsigned Calibration data (16 bits) */
#define BME280_REG_DIG_P2 0x90  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P3 0x92  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P4 0x94  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P5 0x96  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P6 0x98  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P7 0x9A  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P8 0x9C  /* R   Signed Calibration data (16 bits) */
#define BME280_REG_DIG_P9 0x9E  /* R   Signed Calibration data (16 bits) */

#define BME280_REG_DIG_H1 0xA1
#define BME280_REG_DIG_H2 0xE1
#define BME280_REG_DIG_H3 0xE3
#define BME280_REG_DIG_H4 0xE4
#define BME280_REG_DIG_H5 0xE5
#define BME280_REG_DIG_H6 0xE7

#define BME280_CONTROL      0xF4
#define BME280_RESET        0xE0
#define BME280_CONFIG       0xF5
#define BME280_PRESSUREDATA 0xF7
#define BME280_TEMPDATA     0xFA

typedef struct BME280_CALIBRATE_TAG
{
    unsigned short int dig_T1;
    short int  dig_T2;
    short int  dig_T3;

    unsigned short int dig_P1;
    short int  dig_P2;
    short int  dig_P3;
    short int  dig_P4;
    short int  dig_P5;
    short int  dig_P6;
    short int  dig_P7;
    short int  dig_P8;
    short int  dig_P9;

    int  dig_H1;
    short int  dig_H2;
    short int  dig_H3;
    short int  dig_H4;
    short int  dig_H5;
    int   dig_H6;
} BME280_CALIBRATE;

int initialize_sensor();
float read_temperature(int fd);

#endif  // GROVE_BME280_H_

