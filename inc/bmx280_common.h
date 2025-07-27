/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_common.h
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Common functionality for BMP280/BME280 sensor
 */

#ifndef BMX280_COMMON_H
#define BMX280_COMMON_H

#include <linux/types.h>

#define BMX280_TEMP_ID          0
#define BMX280_PRES_ID          1
#define BMX280_HMDT_ID          2
#define BMX280_TFIN_ID          3
#define BMX280_ALL_ID           4

#define BMX280_CAL_TEMP_SIZE    6
#define BMX280_CAL_PRES_SIZE    18
#define BME280_CAL_HMDT_SIZE    8

#define BMP280_CAL_SIZE         ((BMX280_CAL_TEMP_SIZE) + \
                                 (BMX280_CAL_PRES_SIZE))

#define BME280_CAL_SIZE         ((BMX280_CAL_TEMP_SIZE) + \
                                 (BMX280_CAL_PRES_SIZE) + \
                                 (BME280_CAL_HMDT_SIZE))

#define BME280_OSRS_TEMP_FLAG   0x01
#define BME280_OSRS_PRES_FLAG   0x02
#define BME280_OSRS_HMDT_FLAG   0x04


struct bme280_data {
    int32_t t_fine;
    int32_t temperature;
    uint32_t pressure;
    uint32_t humidity;
};

struct bmp280_data {
    int32_t t_fine;
    int32_t temperature;
    uint32_t pressure;
};

struct bmx280_cal_temp {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
};

struct bmx280_cal_pres {
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
};

struct bme280_cal_hmdt {
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
};

struct bmp280_cal {
    struct bmx280_cal_temp temp;
    struct bmx280_cal_pres pres;
};

struct bme280_cal {
    struct bmx280_cal_temp temp;
    struct bmx280_cal_pres pres;
    struct bme280_cal_hmdt hmdt;
};


static inline uint16_t bme280_read_u16l(const uint8_t* data) {
    return ((((uint16_t)(data[1])) << 8) & 0xFF00) |
        (((uint16_t)(data[0])) & 0x00FF);
}

static inline int16_t bme280_read_i16l(const uint8_t* data) {
    return ((((int16_t)(data[1])) << 8) & 0xFF00) | ((int16_t)(data[0]));
}

int bmx280_get_data_reg(uint32_t* const dst, const uint8_t* data, size_t size);

int bmx280_read_cal_temp(struct bmx280_cal_temp* context,
    const uint8_t* data, uint32_t size);
int bmx280_read_cal_pres(struct bmx280_cal_pres* context,
    const uint8_t* data, uint32_t size);
int bme280_read_cal_hmdt(struct bme280_cal_hmdt* context,
    const uint8_t* data, uint32_t size);
int bmp280_read_cal(struct bmp280_cal* context,
    const uint8_t* data, uint32_t size);
int bme280_read_cal(struct bme280_cal* context,
    const uint8_t* data, uint32_t size);

int32_t bmx280_comp_temp(const struct bmx280_cal_temp* cal,
    int32_t adc_T, int32_t* const tfptr);
uint32_t bmx280_comp_pres(const struct bmx280_cal_pres* cal,
    int32_t adc_P, int32_t t_fine);
uint32_t bme280_comp_hmdt(const struct bme280_cal_hmdt* cal,
    int32_t adc_H, int32_t t_fine);

int bmp280_conv_data(struct bmp280_data* dst, struct bmp280_cal* cal,
    const uint8_t * data, uint32_t size);
int bme280_conv_data(struct bme280_data* dst, struct bme280_cal* cal,
    const uint8_t * data, uint32_t size);

const char* bmx280_osrs_to_str(uint8_t value);
int8_t bmx280_str_to_osrs(const char* str);
const char* bmx280_mode_to_str(uint8_t value);
int8_t bmx280_str_to_mode(const char* str);
const char* bmx280_filt_to_str(uint8_t value);
int8_t bmx280_str_to_filt(const char* str);

#endif /* BMX280_COMMON_H */
