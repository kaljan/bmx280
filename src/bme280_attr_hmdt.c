/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_attr_hmdt.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Humidity attributes for BME280 sensor
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"

#include <linux/device.h>
#include <linux/string.h>


static ssize_t calibration_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bme280_dev *handle = i2c_get_clientdata(client);
    return sprintf(buf,
        "H1=%d\n"
        "H2=%d\n"
        "H3=%d\n"
        "H4=%d\n"
        "H5=%d\n"
        "H6=%d\n"
        , (int)handle->cal.hmdt.dig_H1
        , (int)handle->cal.hmdt.dig_H2
        , (int)handle->cal.hmdt.dig_H3
        , (int)handle->cal.hmdt.dig_H4
        , (int)handle->cal.hmdt.dig_H5
        , (int)handle->cal.hmdt.dig_H6
    );
}

static ssize_t value_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bme280_dev *handle = i2c_get_clientdata(client);
    struct bme280_data data;

    if (bme280_get_data(handle, &data) < 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -1;
    }

    return sprintf(buf, "%u\n", data.humidity);
}

static ssize_t scale_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    return sprintf(buf, "1024\n");
}

static ssize_t oversampling_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    uint8_t data;
    const char* osrs = 0;

    if (bmx280_i2c_read(client, BMX280_CHR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    osrs = bmx280_osrs_to_str(data);
    return sprintf(buf, "%s\n", osrs);
}

static ssize_t oversampling_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bme280_dev *handle = i2c_get_clientdata(client);
    int8_t val = bmx280_str_to_osrs(buf);

    if (val < 0) {
        return -1;
    }

    handle->osrs.t = val;

    return count;
}


static DEVICE_ATTR_RO(calibration);
static DEVICE_ATTR_RO(value);
static DEVICE_ATTR_RO(scale);
static DEVICE_ATTR_RW(oversampling);

static struct attribute *bme280_hmdt_attrs[] = {
    &dev_attr_calibration.attr,
    &dev_attr_value.attr,
    &dev_attr_scale.attr,
    &dev_attr_oversampling.attr,
    NULL
};

struct attribute_group bme280_hmdt_attr_group = {
    .name = "humidity",
    .attrs = bme280_hmdt_attrs,
};
