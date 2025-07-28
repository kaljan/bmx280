/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_dev.h
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   General kernel module header for BMP280/BME280 sensor
 */

#ifndef BMX280_DEV_H
#define BMX280_DEV_H

#include "bmx280_common.h"

#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>


#define BME280_OSRS_T_FLAG      0x01U
#define BME280_OSRS_P_FLAG      0x02U
#define BME280_OSRS_H_FLAG      0x04U


struct bme280_dev {
    uint8_t flags;
    struct bme280_cal cal;
    struct bme280_data data;
    struct {
        uint8_t t;
        uint8_t p;
        uint8_t h;
    } osrs;
};

struct bmp280_dev {
    struct bmp280_cal cal;
    struct bmp280_data data;
};

struct bmx280_dev {
    struct i2c_client *client;
    struct mutex i2c_lock;
    void* data;
    void (*reset)(void*);
    ssize_t (*cal_show)(void*, uint8_t, char*);
    ssize_t (*data_show)(void*, struct i2c_client*, uint8_t, char*);
    ssize_t (*osrs_show)(void*, struct i2c_client*, uint8_t, char*);
    ssize_t (*osrs_store)(void*, struct i2c_client*, uint8_t, const char*, size_t);
    int (*stby_to_int)(uint8_t);
    int8_t (*int_to_stby)(int);
};


extern struct attribute_group bmx280_temp_attr_group;
extern struct attribute_group bmx280_pres_attr_group;
extern struct attribute_group bme280_hmdt_attr_group;

extern const struct attribute_group *bmp280_attr_groups[];
extern const struct attribute_group *bme280_attr_groups[];


int bmx280_cal_temp_show(struct bmx280_cal_temp* cal, char* buf);
int bmx280_cal_pres_show(struct bmx280_cal_pres* cal, char* buf);
int bme280_cal_hmdt_show(struct bme280_cal_hmdt* cal, char* buf);

int bmx280_i2c_write_byte(struct i2c_client *hi2c, uint8_t reg, uint8_t data);
int bmx280_i2c_read(struct i2c_client *hi2c,
    uint8_t reg, uint8_t* data, uint8_t size);
int bmx280_modify_reg(struct i2c_client *client,
    uint8_t reg, uint8_t value, uint8_t mask, uint8_t offset);
int bmx280_modify_config_reg(struct i2c_client *client,
    uint8_t value, uint8_t mask, uint8_t offset);

int bmx280_setup_attrs(struct device* dev,
    const struct attribute_group ** attrs);
int bmp280_probe(struct i2c_client *client);
int bme280_probe(struct i2c_client *client);

const char* bmx280_osrs_to_str(uint8_t value);
int8_t bmx280_str_to_osrs(const char* str);
const char* bmx280_mode_to_str(uint8_t value);
int8_t bmx280_str_to_mode(const char* str);
const char* bmx280_filt_to_str(uint8_t value);
int8_t bmx280_str_to_filt(const char* str);

#endif /* BMX280_DEV_H */
