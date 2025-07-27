/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_if.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Interface functionality for BMP280/BME280 sensor
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"

int bmx280_i2c_write_byte(struct i2c_client *hi2c, uint8_t reg, uint8_t data) {
    int ret;
    uint8_t buf[2] = {reg, data};
    ret = i2c_master_send(hi2c, buf, 2);
    if (ret < 0) {
        dev_warn(&hi2c->dev, "i2c write byte failed\n");
    }
    return ret;
}

int bmx280_i2c_read(struct i2c_client *hi2c, uint8_t reg, uint8_t* data, uint8_t size) {
    int ret = 0;

    ret = i2c_master_send(hi2c, &reg, 1);
    if (ret < 0) {
        dev_warn(&hi2c->dev, "i2c write byte failed\n");
        return ret;
    }

    ret = i2c_master_recv(hi2c, data, size);
    if (ret < 0) {
        dev_warn(&hi2c->dev, "i2c read byte failed\n");
        return ret;
    }
    return ret;
}

int bmx280_modify_reg(struct i2c_client *client,
    uint8_t reg, uint8_t value, uint8_t mask, uint8_t offset) {
    uint8_t data;

    if (bmx280_i2c_read(client, reg, &data, 1) <= 0) {
        dev_warn(&client->dev, "read register 0x%02X failed\n", reg);
        return -1;
    }

    data = (data & (~(mask << offset))) | ((value & mask) << offset);

    if (bmx280_i2c_write_byte(client, reg, data) <= 0) {
        dev_warn(&client->dev, "write register 0x%02X failed\n", reg);
        return -1;
    }
    return 0;
}

int bmx280_modify_config_reg(struct i2c_client *client,
    uint8_t value, uint8_t mask, uint8_t offset) {
    uint8_t data;
    if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read ctrl_meas(0xF4) register failed\n");
        return -1;
    } else if (data & BMX280_CMR) {
        dev_warn(&client->dev, "write to config register only in sleep mode\n");
        return -1;
    }

    return bmx280_modify_reg(client, BMX280_CR, value, mask, offset);
}
