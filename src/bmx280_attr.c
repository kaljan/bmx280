/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_attr.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Attributes for BMP280/BME280 sensor
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"

#include <linux/device.h>
#include <linux/string.h>


int bmx280_setup_attrs(struct device* dev,
    const struct attribute_group ** attrs) {
    int ret = 0;
    while (*attrs != NULL) {
        if (devm_device_add_group(dev, *attrs) < 0) {
            dev_err(dev, "add group failed\n");
            ret = -1;
            break;
        }
        attrs++;
    }
    return ret;
}

int bmx280_cal_temp_show(struct bmx280_cal_temp* cal, char* buf) {
    return sprintf(buf, "T1=%d\nT2=%d\nT3=%d\n",
        (int)cal->dig_T1, (int)cal->dig_T2, (int)cal->dig_T3);
}

int bmx280_cal_pres_show(struct bmx280_cal_pres* cal, char* buf) {
    return sprintf(buf, "P1=%d\nP2=%d\nP3=%d\nP4=%d\n"
        "P5=%d\nP6=%d\nP7=%d\nP8=%d\nP9=%d\n",
        (int)cal->dig_P1, (int)cal->dig_P2, (int)cal->dig_P3,
        (int)cal->dig_P4, (int)cal->dig_P5, (int)cal->dig_P6,
        (int)cal->dig_P7, (int)cal->dig_P8, (int)cal->dig_P9
    );
}

int bme280_cal_hmdt_show(struct bme280_cal_hmdt* cal, char* buf) {
    return sprintf(buf, "H1=%d\nH2=%d\nH3=%d\nH4=%d\nH5=%d\nH6=%d\n",
        (int)cal->dig_H1, (int)cal->dig_H2, (int)cal->dig_H3,
        (int)cal->dig_H4, (int)cal->dig_H5, (int)cal->dig_H6
    );
}


static ssize_t version_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    return sprintf(buf, "1.0.0\n");
}

static ssize_t status_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    uint8_t data;

    if (bmx280_i2c_read(client, BMX280_SR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    return sprintf(buf, "MEASURING=%d\nIM_UPDATE=%d\n"
        , (data & BMX280_SR_MEAS) ? 1 : 0
        , (data & BMX280_SR_IMUP) ? 1 : 0);
}

static ssize_t stby_time_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);
    uint8_t data;
    int t_stby = 0;

    if (handle->stby_to_int == NULL) {
        return -1;
    } else if (bmx280_i2c_read(client, BMX280_CR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    t_stby = handle->stby_to_int(data >> BMX280_CR_T_SB_OFFSET);

    return sprintf(buf, "%d\n", t_stby);
}

static ssize_t stby_time_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);
    int temp = 0;
    int8_t val;

    if (handle->stby_to_int == NULL) {
        return -1;
    } else if (kstrtoint(buf, 10, &temp) == 0) {
        val = handle->int_to_stby(temp);
        if ((val < 0) || (bmx280_modify_config_reg(client, val,
            BMX280_CR_T_SB_MASK, BMX280_CR_T_SB_OFFSET) < 0)) {
            return -1;
        }
    }

    return count;
}

static ssize_t iir_filter_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    uint8_t data;
    const char* iir_filt = 0;

    if (bmx280_i2c_read(client, BMX280_CR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    iir_filt = bmx280_filt_to_str(data >> BMX280_CR_FILTER_OFFSET);

    return sprintf(buf, "%s\n", iir_filt);
}

static ssize_t iir_filter_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    int8_t val = bmx280_str_to_filt(buf);

    if ((val < 0) || (bmx280_modify_config_reg(client, val,
        BMX280_CR_FILTER_MASK, BMX280_CR_FILTER_OFFSET) < 0)) {
        return -1;
    }

    return count;
}

static ssize_t oversampling_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->osrs_show != NULL) {
        return handle->osrs_show(handle->data, client, BMX280_ALL_ID, buf);
    }
    return -1;
}

static ssize_t oversampling_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    struct bmx280_dev *handle = i2c_get_clientdata(client);

    if (handle->osrs_show != NULL) {
        return handle->osrs_store(handle->data, client, BMX280_ALL_ID, buf, count);
    }

    return -1;
}

static ssize_t mode_show(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct i2c_client *client = to_i2c_client(dev);
    uint8_t data;
    const char* mode = 0;

    if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read standby time failed\n");
        return -1;
    }

    mode = bmx280_mode_to_str(data);

    return sprintf(buf, "%s\n", mode);
}

static ssize_t mode_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = to_i2c_client(dev);
    int8_t val = bmx280_str_to_mode(buf);

    if (val < 0) {
        dev_warn(&client->dev, "convert mode failed\n");
        return -1;
    } else if (bmx280_modify_reg(client, BMX280_CMR, val, BMX280_CMR_MODE_MASK, 0) < 0) {
        return -1;
    }

    return count;
}

static ssize_t reset_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    if ((strlen("reset") == strlen(buf)) && (strcmp(buf, "reset") == 0)) {
        struct i2c_client *client = to_i2c_client(dev);
        struct bmx280_dev *handle = i2c_get_clientdata(client);

        if (bmx280_i2c_write_byte(client, BMX280_RSTR, BMX280_RSTR_KEY) <= 0) {
            dev_warn(&client->dev, "write to reset(0xE0) register failed\n");
            return -1;
        } else if (handle->reset != NULL) {
            handle->reset(handle->data);
        }
    }
    return count;
}


static DEVICE_ATTR_RO(version);
static DEVICE_ATTR_RO(status);
static DEVICE_ATTR_RW(stby_time);
static DEVICE_ATTR_RW(iir_filter);
static DEVICE_ATTR_RW(mode);
static DEVICE_ATTR_RW(oversampling);
static DEVICE_ATTR_WO(reset);

static struct attribute *bmx280_control_attrs[] = {
    &dev_attr_version.attr,
    &dev_attr_status.attr,
    &dev_attr_stby_time.attr,
    &dev_attr_iir_filter.attr,
    &dev_attr_mode.attr,
    &dev_attr_oversampling.attr,
    &dev_attr_reset.attr,
    NULL
};

struct attribute_group bmx280_control_attr_group = {
    .name = "control",
    .attrs = bmx280_control_attrs,
};

const struct attribute_group *bmp280_attr_groups[] = {
    &bmx280_control_attr_group,
    &bmx280_pres_attr_group,
    &bmx280_temp_attr_group,
    NULL
};

const struct attribute_group *bme280_attr_groups[] = {
    &bmx280_control_attr_group,
    &bmx280_pres_attr_group,
    &bmx280_temp_attr_group,
    &bme280_hmdt_attr_group,
    NULL
};
