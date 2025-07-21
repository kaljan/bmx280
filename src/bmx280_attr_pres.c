/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_attr_pres.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Pressure attributes for BMP280/BME280 sensor
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
        "P1=%d\n"
        "P2=%d\n"
        "P3=%d\n"
        "P4=%d\n"
        "P5=%d\n"
        "P6=%d\n"
        "P7=%d\n"
        "P8=%d\n"
        "P9=%d\n"
        , (int)handle->cal.pres.dig_P1
        , (int)handle->cal.pres.dig_P2
        , (int)handle->cal.pres.dig_P3
        , (int)handle->cal.pres.dig_P4
        , (int)handle->cal.pres.dig_P5
        , (int)handle->cal.pres.dig_P6
        , (int)handle->cal.pres.dig_P7
        , (int)handle->cal.pres.dig_P8
        , (int)handle->cal.pres.dig_P9
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

    return sprintf(buf, "%u\n", data.pressure);
}

static ssize_t scale_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    return sprintf(buf, "256\n");
}

static ssize_t oversampling_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    uint8_t data;
    const char* osrs = 0;

    if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    osrs = bmx280_osrs_to_str(data >> BMX280_CMR_OSRS_P_OFFSET);
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

    handle->osrs.p = val;

    return count;
}


static DEVICE_ATTR_RO(calibration);
static DEVICE_ATTR_RO(value);
static DEVICE_ATTR_RO(scale);
static DEVICE_ATTR_RW(oversampling);

static struct attribute *bmx280_pres_attrs[] = {
    &dev_attr_calibration.attr,
    &dev_attr_value.attr,
    &dev_attr_scale.attr,
    &dev_attr_oversampling.attr,
    NULL
};

struct attribute_group bmx280_pres_attr_group = {
    .name = "pressure",
    .attrs = bmx280_pres_attrs,
};
