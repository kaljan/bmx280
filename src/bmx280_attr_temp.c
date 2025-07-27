/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_attr_temp.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Temperature attributes for BMP280/BME280 sensor
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"

#include <linux/device.h>
#include <linux/string.h>


static ssize_t calibration_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->cal_show != NULL) {
        return handle->cal_show(handle->data, BMX280_TEMP_ID, buf);
    }
    return -1;
}

static ssize_t value_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->data_show != NULL) {
        return handle->data_show(handle->data, client, BMX280_TEMP_ID, buf);
    }

    return -1;
}

static ssize_t scale_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    return sprintf(buf, "100\n");
}

static ssize_t oversampling_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->osrs_show != NULL) {
        return handle->osrs_show(handle->data, client, BMX280_TEMP_ID, buf);
    }

    return -1;
}

static ssize_t oversampling_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->osrs_store != NULL) {
        return handle->osrs_store(handle->data, client, BMX280_TEMP_ID, buf, count);
    }

    return -1;
}


static DEVICE_ATTR_RO(calibration);
static DEVICE_ATTR_RO(value);
static DEVICE_ATTR_RO(scale);
static DEVICE_ATTR_RW(oversampling);

static struct attribute *bmx280_temp_attrs[] = {
    &dev_attr_calibration.attr,
    &dev_attr_value.attr,
    &dev_attr_scale.attr,
    &dev_attr_oversampling.attr,
    NULL
};

struct attribute_group bmx280_temp_attr_group = {
    .name = "temperature",
    .attrs = bmx280_temp_attrs,
};
