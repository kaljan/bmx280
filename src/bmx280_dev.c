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


static const struct of_device_id of_bmx280_match[] = {
    { .compatible = "bosch,bme280"},
    { .compatible = "bosch,bmp280"},
    {},
};

MODULE_DEVICE_TABLE(of, of_bmx280_match);


static const struct i2c_device_id bme280_id[] = {
    { "bme280", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, bme280_id);


static int bme280_i2c_check_id(struct bme280_dev *context) {
    int ret = 0;
    uint8_t id = 0;

    mutex_lock(&context->i2c_lock);
    ret = bmx280_i2c_read(context->i2c, BMX280_IDR, &id, 1);
    if (ret > 0) {
        if (id == BME280_DEVICE_ID) {
            ret = 1;
            dev_info(&context->i2c->dev, "BME280 detected; ID: 0x%02X", id);
        } else {
            dev_warn(&context->i2c->dev,
                "wrong device id; expected: 0x%02X; actual: 0x%02X\n"
                , BME280_DEVICE_ID, id);
            ret = 0;
        }
    } else {
        dev_err(&context->i2c->dev, "i2c read id failed\n");
        ret = -1;
    }

    mutex_unlock(&context->i2c_lock);

    return ret;
}

static int bme280_i2c_probe(struct i2c_client *i2c)
{
    int ret = 0;

    const struct attribute_group ** ptr = bme280_attr_groups;

    struct bme280_dev *context = devm_kzalloc(&i2c->dev,
        sizeof(struct bme280_dev), GFP_KERNEL);

    if (context == NULL) {
        return -ENOMEM;
    }

    mutex_init(&context->i2c_lock);
    context->i2c = i2c;
    i2c_set_clientdata(i2c, context);

    ret = bme280_i2c_check_id(context);
    if (ret <= 0) {
        return ret;
    }

    ret = bme280_update_cal(context);
    if (ret < 0) {
        return ret;
    }

    while (*ptr != NULL) {
        ret = devm_device_add_group(&i2c->dev, *ptr);
        if (ret < 0) {
            dev_err(&i2c->dev, "add group failed\n");
            break;
        }
        ptr++;
    }

    return ret;
}

static void bme280_i2c_remove(struct i2c_client *client) {

}

static const struct i2c_device_id bme280_i2c_id[] = {
    { "bme280", 0 },
    { }
};

static struct i2c_driver bme280_i2c_driver = {
    .driver = {
        .name           = "bme280",
        .owner          = THIS_MODULE,
        .of_match_table = of_bmx280_match,
    },
    .probe              = bme280_i2c_probe,
    .remove             = bme280_i2c_remove,
    .id_table           = bme280_i2c_id,
};

module_i2c_driver(bme280_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("BME280 pressure, humidity and temperature sensor driver");
MODULE_AUTHOR("Mikalai Naurotski <kaljan.nothern@gmail.com>");
