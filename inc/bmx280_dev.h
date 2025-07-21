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


struct bme280_dev {
    struct device *dev;
    struct i2c_client *i2c;
    struct mutex i2c_lock;
    struct bme280_cal cal;
    struct bme280_data data;
    struct {
        uint8_t t;
        uint8_t p;
        uint8_t h;
    } osrs;
};


extern struct attribute_group bmx280_temp_attr_group;
extern struct attribute_group bmx280_pres_attr_group;
extern struct attribute_group bme280_hmdt_attr_group;

extern const struct attribute_group *bmp280_attr_groups[];
extern const struct attribute_group *bme280_attr_groups[];


int bmx280_i2c_write_byte(struct i2c_client *hi2c, uint8_t reg, uint8_t data);
int bmx280_i2c_read(struct i2c_client *hi2c, uint8_t reg, uint8_t* data, uint8_t size);

int bmx280_modify_reg(struct i2c_client *client,
    uint8_t reg, uint8_t mask, uint8_t offset, uint8_t value);

int bmx280_modify_config_reg(struct i2c_client *client,
    uint8_t mask, uint8_t offset, uint8_t value);

int bme280_get_data(struct bme280_dev* handle, struct bme280_data* data);
int bme280_update_cal(struct bme280_dev* context);

#endif /* BMX280_DEV_H */
