/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bme280_impl.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-22
 *
 * @brief   BME280 specific functionality implementation
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"


static int bme280_get_data(struct i2c_client* client, struct bme280_dev* context) {
    uint8_t temp[8];

    if (bmx280_i2c_read(client, BMX280_PR_MSB, temp, 8) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -EIO;
    } else if (bme280_conv_data(&context->data, &context->cal, temp, 8) < 0) {
        return -EINVAL;
    }

    return 0;
}

static ssize_t bme280_cal_show(void* user_data, uint8_t id, char* buf) {
    struct bme280_dev* context = (struct bme280_dev*)user_data;
    if (id == BMX280_TEMP_ID) {
        return bmx280_cal_temp_show(&context->cal.temp, buf);
    } else if (id == BMX280_PRES_ID) {
        return bmx280_cal_pres_show(&context->cal.pres, buf);
    } else if (id == BMX280_HMDT_ID) {
        return bme280_cal_hmdt_show(&context->cal.hmdt, buf);
    }

    return -EINVAL;
}

static ssize_t bme280_data_show(void* user_data,
    struct i2c_client* client, uint8_t id, char* buf) {
    struct bme280_dev* context = (struct bme280_dev*)user_data;
    if (id == BMX280_TEMP_ID) {
        if (bme280_get_data(client, context) < 0) {
            return -EIO;
        }
        return sprintf(buf, "%d\n", context->data.temperature);
    } else if (id == BMX280_PRES_ID) {
        return sprintf(buf, "%u\n", context->data.pressure);
    } else if (id == BMX280_HMDT_ID) {
        return sprintf(buf, "%u\n", context->data.humidity);
    } else if (id == BMX280_TFIN_ID) {
        return sprintf(buf, "%d\n", context->data.t_fine);
    }
    return -EINVAL;
}

static ssize_t bme280_osrs_show(void* user_data,
    struct i2c_client* client, uint8_t id, char* buf) {
    struct bme280_dev* context = (struct bme280_dev*)user_data;
    uint8_t ctrl_meas, ctrl_hum;

    if (bmx280_i2c_read(client, BMX280_CMR, &ctrl_meas, 1) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -EIO;
    } else if (bmx280_i2c_read(client, BMX280_CHR, &ctrl_hum, 1) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -EIO;
    }

    context->osrs.t = BMX280_GET_OSRS_T(ctrl_meas);
    context->osrs.p = BMX280_GET_OSRS_P(ctrl_meas);
    context->osrs.h = BMX280_GET_OSRS_P(ctrl_hum);

    if (id == BMX280_TEMP_ID) {
        return sprintf(buf, "%s\n", bmx280_osrs_to_str(context->osrs.t));
    } else if (id == BMX280_PRES_ID) {
        return sprintf(buf, "%s\n", bmx280_osrs_to_str(context->osrs.p));
    } else if (id == BMX280_HMDT_ID) {
        return sprintf(buf, "%s\n", bmx280_osrs_to_str(context->osrs.h));
    } else if (id == BMX280_ALL_ID) {
        return sprintf(buf, "osrs_t=%s\nosrs_p=%s\nosrs_h=%s\n"
            , bmx280_osrs_to_str(context->osrs.t)
            , bmx280_osrs_to_str(context->osrs.p)
            , bmx280_osrs_to_str(context->osrs.h));
    }
    return -EINVAL;
}

static ssize_t bme280_osrs_store(void* user_data,
    struct i2c_client* client, uint8_t id,
    const char* buf, size_t size) {
    struct bme280_dev* context = (struct bme280_dev*)user_data;
    int8_t value = bmx280_str_to_osrs(buf);
    uint8_t ctrl_meas, ctrl_hum;

    if (value < 0) {
        return -EINVAL;
    } else if (id == BMX280_TEMP_ID) {
        context->osrs.t = value;
        context->flags |= BME280_OSRS_T_FLAG;
    } else if (id == BMX280_PRES_ID) {
        context->osrs.p = value;
        context->flags |= BME280_OSRS_P_FLAG;
    } else if (id == BMX280_HMDT_ID) {
        context->osrs.h = value;
        context->flags |= BME280_OSRS_H_FLAG;
    } else if (id == BMX280_ALL_ID) {
        context->osrs.t = value;
        context->osrs.p = value;
        context->osrs.h = value;
        context->flags = BME280_OSRS_T_FLAG |
            BME280_OSRS_P_FLAG | BME280_OSRS_H_FLAG;
    } else {
        dev_err(&client->dev, "unknown sensor id\n");
        return -EINVAL;
    }

    if ((context->flags & BME280_OSRS_T_FLAG) &&
        (context->flags & BME280_OSRS_P_FLAG) &&
        (context->flags & BME280_OSRS_H_FLAG)) {

        if (bmx280_i2c_read(client, BMX280_CMR, &ctrl_meas, 1) <= 0) {
            dev_warn(&client->dev, "read data failed\n");
            return -EIO;
        } else if (bmx280_i2c_read(client, BMX280_CHR, &ctrl_hum, 1) <= 0) {
            dev_warn(&client->dev, "read data failed\n");
            return -EIO;
        }

        BMX280_SET_OSRS_T(context->osrs.t, ctrl_meas);
        BMX280_SET_OSRS_P(context->osrs.p, ctrl_meas);
        BMX280_SET_OSRS_H(context->osrs.h, ctrl_hum);

        if (bmx280_i2c_write_byte(client, BMX280_CHR, ctrl_hum) <= 0) {
            dev_warn(&client->dev, "write register ctrl_meas(0xF4) failed\n");
            return -EIO;
        } else if (bmx280_i2c_write_byte(client, BMX280_CMR, ctrl_meas) <= 0) {
            dev_warn(&client->dev, "write register ctrl_meas(0xF4) failed\n");
            return -EIO;
        }
        context->flags = 0;
    }

    return size;
}

static int bme280_stby_to_int(uint8_t value) {
    switch (value & 0x07) {
    case 0: return 500;
    case 1: return 62500;
    case 2: return 125000;
    case 3: return 250000;
    case 4: return 500000;
    case 5: return 1000000;
    case 6: return 2000000;
    case 7: return 4000000;
    default: break;
    }
    return 0;
}

static int8_t bme280_int_to_stby(int value) {
    switch (value) {
    case 500: return 0;
    case 62500: return 1;
    case 125000: return 2;
    case 250000: return 3;
    case 500000: return 4;
    case 1000000: return 5;
    case 10000: return 6;
    case 20000: return 7;
    default: break;
    }
    return -1;
}

static void bme280_reset(void* user_data) {
    struct bme280_dev* context = (struct bme280_dev*)user_data;
    context->flags = 0;
}

static int bme280_update_cal(struct i2c_client *client,
    struct bme280_dev* context) {
    int ret = 0;
    uint8_t* buf = kmalloc(BME280_CAL_SIZE, GFP_KERNEL);

    if (bmx280_i2c_read(client, BMX280_REG_CALIB00, buf, 24) <= 0) {
        dev_err(&client->dev, "read calibration registers[0x88..0x9F] failed\n");
        ret = -1;
    } else if (bmx280_i2c_read(client, BMX280_REG_CALIB25, &buf[24], 1) <= 0) {
        dev_err(&client->dev, "read calibration register[0xA1] failed\n");
        ret = -1;
    } else if (bmx280_i2c_read(client, BMX280_REG_CALIB26, &buf[25], 7) <= 0) {
        dev_err(&client->dev, "read calibration registers[0xE1..0xE7] failed\n");
        ret = -1;
    } else if (bme280_read_cal(&context->cal, buf, BME280_CAL_SIZE) <= 0) {
        dev_err(&client->dev, "parse calibration data failed\n");
        ret = -1;
    } else {
        ret = 0;
    }

    kfree(buf);
    buf = NULL;
    return ret;
}

int bme280_probe(struct i2c_client *client) {
    struct bmx280_dev* context = NULL;
    void * ptr = devm_kzalloc(&client->dev,
        sizeof(struct bmx280_dev) + sizeof(struct bme280_dev),
        GFP_KERNEL);

    if (ptr == NULL) {
        return -ENOMEM;
    }

    context = (struct bmx280_dev*)ptr;
    context->data = ptr + sizeof(struct bmx280_dev);
    context->client = client;

    context->reset = bme280_reset;
    context->cal_show = bme280_cal_show;
    context->data_show = bme280_data_show;
    context->osrs_show = bme280_osrs_show;
    context->osrs_store = bme280_osrs_store;
    context->stby_to_int = bme280_stby_to_int;
    context->int_to_stby = bme280_int_to_stby;

    mutex_init(&context->i2c_lock);
    i2c_set_clientdata(client, context);

    if (bme280_update_cal(client,
        (struct bme280_dev*)context->data) < 0) {
        return -EIO;
    }

    return bmx280_setup_attrs(&client->dev, bme280_attr_groups);
}
