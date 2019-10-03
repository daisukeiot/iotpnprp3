#include "grove_bme280.h"
#include <stdint.h>

BME280_CALIBRATE bme280_calibrate_data;
#define SWAP_2BYTES(x) (((x & 0xFFFF) >> 8) | ((x & 0xFF) << 8))

int read_raw(int fd, int reg)
{
    int raw = SWAP_2BYTES(wiringPiI2CReadReg16(fd, reg));
    raw <<= 8;
    raw = raw | wiringPiI2CReadReg8(fd, reg + 2);
    raw >>= 4;
    return raw;
}

int32_t compensate_temp(int raw_temp)
{
    int t1 = (((raw_temp >> 3) - (bme280_calibrate_data.dig_T1 << 1)) * (bme280_calibrate_data.dig_T2)) >> 11;
    int t2 = (((((raw_temp >> 4) - (bme280_calibrate_data.dig_T1)) *
        ((raw_temp >> 4) - (bme280_calibrate_data.dig_T1))) >> 12) *
        (bme280_calibrate_data.dig_T3)) >> 14;
    return t1 + t2;
}

float read_temperature(int fd)
{
    int raw_temp = read_raw(fd, BME280_TEMPDATA);
    int compensated_temp = compensate_temp(raw_temp);
    return (float)((compensated_temp * 5 + 128) >> 8) / 100;
}

void cablirate_sensor(int fd)
{
    bme280_calibrate_data.dig_T1 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_T1);
    bme280_calibrate_data.dig_T2 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_T2);
    bme280_calibrate_data.dig_T3 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_T3);
    bme280_calibrate_data.dig_P1 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P1);
    bme280_calibrate_data.dig_P2 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P2);
    bme280_calibrate_data.dig_P3 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P3);
    bme280_calibrate_data.dig_P4 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P4);
    bme280_calibrate_data.dig_P5 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P5);
    bme280_calibrate_data.dig_P6 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P6);
    bme280_calibrate_data.dig_P7 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P7);
    bme280_calibrate_data.dig_P8 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P8);
    bme280_calibrate_data.dig_P9 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_P9);

    bme280_calibrate_data.dig_H1 = wiringPiI2CReadReg8(fd, BME280_REG_DIG_H1);
    bme280_calibrate_data.dig_H2 = wiringPiI2CReadReg16(fd, BME280_REG_DIG_H2);
    bme280_calibrate_data.dig_H3 = wiringPiI2CReadReg8(fd, BME280_REG_DIG_H3);
    bme280_calibrate_data.dig_H4 = wiringPiI2CReadReg8(fd, BME280_REG_DIG_H4);
    bme280_calibrate_data.dig_H5 = wiringPiI2CReadReg8(fd, BME280_REG_DIG_H5);
    bme280_calibrate_data.dig_H6 = wiringPiI2CReadReg8(fd, BME280_REG_DIG_H6);
}

int initialize_sensor()
{
    int fd,ret;
    int ID = 0x76;

    fd = wiringPiI2CSetup(ID);
    
    if (0x60 != wiringPiI2CReadReg8(fd, 0xD0))
    {
        fprintf(stderr, "Unsupported chip\n");
        return -2;
    }

    cablirate_sensor(fd);

    wiringPiI2CWriteReg8(fd, BME280_CONTROL, 0x3F);

    return fd;
}