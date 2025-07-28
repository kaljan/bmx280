/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_dev.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Linux kernel module for BMP280/BME280 sensor
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"
#include "bmx280_common.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pm_runtime.h>
#include <linux/string.h>
#include <linux/kernel.h>


static int bmx280_i2c_probe(struct i2c_client *client)
{
    int ret = 0;
    uint8_t id = 0;

    ret = bmx280_i2c_read(client, BMX280_IDR, &id, 1);
    if (ret <= 0) {
        dev_err(&client->dev, "read device id failed\n");
        return -EIO;
    } else if ((id == BMP280_DEVICE_ID_SMP0) ||
        (id == BMP280_DEVICE_ID_SMP1) ||
        (id == BMP280_DEVICE_ID)) {
        dev_info(&client->dev, "BMP280 detected; ID: 0x%02X", id);
        return bmp280_probe(client);
    } else if (id == BME280_DEVICE_ID) {
        dev_info(&client->dev, "BME280 detected; ID: 0x%02X", id);
        return bme280_probe(client);
    } else {
        dev_warn(&client->dev,
            "wrong device id; expected: 0x%02X; actual: 0x%02X\n"
            , BME280_DEVICE_ID, id);
    }
    return -EIO;
}


static const struct of_device_id of_bmx280_match[] = {
    { .compatible = "bosch,bmp280"},
    { .compatible = "bosch,bme280"},
    { },
};

MODULE_DEVICE_TABLE(of, of_bmx280_match);

static const struct i2c_device_id bmx280_i2c_id[] = {
    { "bmp280" },
    { "bme280" },
    { }
};

MODULE_DEVICE_TABLE(i2c, bmx280_i2c_id);

static struct i2c_driver bmx280_i2c_driver = {
    .driver = {
        .name           = "bmx280",
        .owner          = THIS_MODULE,
        .of_match_table = of_bmx280_match,
    },
    .probe              = bmx280_i2c_probe,
    .id_table           = bmx280_i2c_id,
};

module_i2c_driver(bmx280_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("BME280 pressure, humidity and temperature sensor driver");
MODULE_AUTHOR("Mikalai Naurotski <kaljan.nothern@gmail.com>");
