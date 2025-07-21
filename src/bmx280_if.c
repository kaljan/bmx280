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
    uint8_t reg, uint8_t mask, uint8_t offset, uint8_t value) {
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
    uint8_t mask, uint8_t offset, uint8_t value) {
    uint8_t data;
    if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read ctrl_meas(0xF4) register failed\n");
        return -1;
    } else if (data & BMX280_CMR) {
        dev_warn(&client->dev, "write to config register only in sleep mode\n");
        return -1;
    }

    return bmx280_modify_reg(client, BMX280_CR, mask, offset, value);
}

int bme280_get_data(struct bme280_dev* handle, struct bme280_data* data) {
    uint8_t temp[8];

    if (bmx280_i2c_read(handle->i2c, BMX280_PR_MSB, temp, 8) <= 0) {
        dev_warn(&handle->i2c->dev, "read data failed\n");
        return -1;
    }

    return bme280_conv_data(data, &handle->cal, temp, 8);
}


int bme280_update_cal(struct bme280_dev* context) {
    int ret = 0;
    uint8_t* buf = kmalloc(BME280_CAL_SIZE, GFP_KERNEL);

    do {

        ret = bmx280_i2c_read(context->i2c, BMX280_REG_CALIB00, buf, 24);
        if (ret <= 0) {
            dev_err(&context->i2c->dev, "read calibration registers[0x88..0x9F] failed\n");
            break;
        }

        ret = bmx280_i2c_read(context->i2c, BMX280_REG_CALIB25, &buf[24], 1);
        if (ret <= 0) {
            dev_err(&context->i2c->dev, "read calibration register[0xA1] failed\n");
            break;
        }

        ret = bmx280_i2c_read(context->i2c, BMX280_REG_CALIB26, &buf[24 + 1], 6);
        if (ret <= 0) {
            dev_err(&context->i2c->dev, "read calibration registers[0xE1..0xE7] failed\n");
            break;
        }

        ret = bme280_read_cal(&context->cal, buf, BME280_CAL_SIZE);
        if (ret > 0) {
            ret = 0;
        } else {
            dev_err(&context->i2c->dev, "parse calibration data failed\n");
            ret = -1;
        }
    } while (0);

    kfree(buf);
    buf = NULL;
    return ret;
}
