/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmp280_impl.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-22
 *
 * @brief   BMP280 specific functionality implementation
 */

#include "bmx280_dev.h"
#include "bmx280_reg.h"


static int bmp280_get_data(struct i2c_client* client, struct bmp280_dev* context) {
    uint8_t temp[6];

    if (bmx280_i2c_read(client, BMX280_PR_MSB, temp, 6) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -1;
    }

    return bmp280_conv_data(&context->data, &context->cal, temp, 8);
}

static ssize_t bmp280_cal_show(void* user_data, uint8_t id, char* buf) {
    struct bmp280_dev* dev = (struct bmp280_dev*)user_data;
    if (id == BMX280_TEMP_ID) {
        return bmx280_cal_temp_show(&dev->cal.temp, buf);
    } else if (id == BMX280_PRES_ID) {
        return bmx280_cal_pres_show(&dev->cal.pres, buf);
    }
    return -1;
}

static ssize_t bmp280_data_show(void* user_data,
    struct i2c_client* client, uint8_t id, char* buf) {
    struct bmp280_dev* context = (struct bmp280_dev*)user_data;
    if (id == BMX280_TEMP_ID) {
        if (bmp280_get_data(client, context) < 0) {
            return -1;
        }
        return sprintf(buf, "%d\n", context->data.temperature);
    } else if (id == BMX280_PRES_ID) {
        return sprintf(buf, "%u\n", context->data.pressure);
    } else if (id == BMX280_TFIN_ID) {
        return sprintf(buf, "%d\n", context->data.t_fine);
    }
    return -1;
}

static ssize_t bmp280_osrs_show(void* user_data,
    struct i2c_client* client, uint8_t id, char* buf) {
    uint8_t data, osrs_t, osrs_p;

    if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -1;
    }

    osrs_t = BMX280_GET_OSRS_T(data);
    osrs_p = BMX280_GET_OSRS_P(data);

    if (id == BMX280_TEMP_ID) {
        return sprintf(buf, "%s\n", bmx280_osrs_to_str(osrs_t));
    } else if (id == BMX280_PRES_ID) {
        return sprintf(buf, "%s\n", bmx280_osrs_to_str(osrs_p));
    } else if (id == BMX280_ALL_ID) {
        return sprintf(buf, "osrs_t=%s\nosrs_p=%s\n"
            , bmx280_osrs_to_str(osrs_t)
            , bmx280_osrs_to_str(osrs_p));
    }
    return 0;
}

static ssize_t bmp280_osrs_store(void* user_data,
    struct i2c_client* client, uint8_t id,
    const char* buf, size_t size) {
    uint8_t data;
    int8_t value = bmx280_str_to_osrs(buf);

    if (value < 0) {
        return -1;
    } else if (bmx280_i2c_read(client, BMX280_CMR, &data, 1) <= 0) {
        dev_warn(&client->dev, "read data failed\n");
        return -1;
    } else if (id == BMX280_TEMP_ID) {
        BMX280_SET_OSRS_T((uint8_t)value, data);
    } else if (id == BMX280_PRES_ID) {
        BMX280_SET_OSRS_P((uint8_t)value, data);
    } else if (id == BMX280_ALL_ID) {
        BMX280_SET_OSRS_T((uint8_t)value, data);
        BMX280_SET_OSRS_P((uint8_t)value, data);
    }

    if (bmx280_i2c_write_byte(client, BMX280_CMR, data) <= 0) {
        dev_warn(&client->dev, "write register ctrl_meas(0xF4) failed\n");
        return -1;
    }

    return size;
}

static int bmp280_stby_to_int(uint8_t value) {
    switch (value & 0x07) {
    case 0: return 500;
    case 1: return 62500;
    case 2: return 125000;
    case 3: return 250000;
    case 4: return 500000;
    case 5: return 1000000;
    case 6: return 10000;
    case 7: return 20000;
    default: break;
    }
    return 0;
}

static int8_t bmp280_int_to_stby(int value) {
    switch (value) {
    case 500: return 0;
    case 62500: return 1;
    case 125000: return 2;
    case 250000: return 3;
    case 500000: return 4;
    case 1000000: return 5;
    case 2000000: return 6;
    case 4000000: return 7;
    default: break;
    }
    return -1;
}

static int bmp280_update_cal(struct i2c_client* client,
    struct bmp280_dev* context) {
    int ret = 0;
    uint8_t* buf = kmalloc(BMP280_CAL_SIZE, GFP_KERNEL);

    do {

        ret = bmx280_i2c_read(client,
            BMX280_REG_CALIB00, buf, BMP280_CAL_SIZE);
        if (ret <= 0) {
            dev_err(&client->dev,
                "read calibration registers[0x88..0x9F] failed\n");
            break;
        }

        ret = bmp280_read_cal(&context->cal, buf, BMP280_CAL_SIZE);
        if (ret > 0) {
            ret = 0;
        } else {
            dev_err(&client->dev,
                "parse calibration data failed\n");
            ret = -1;
        }
    } while (0);

    kfree(buf);
    buf = NULL;
    return ret;
}

int bmp280_probe(struct i2c_client *client) {
    struct bmx280_dev* context = NULL;
    void * ptr = devm_kzalloc(&client->dev,
        sizeof(struct bmx280_dev) + sizeof(struct bmp280_dev),
        GFP_KERNEL);

    if (ptr == NULL) {
        return -ENOMEM;
    }

    context = (struct bmx280_dev*)ptr;
    context->data = ptr + sizeof(struct bmx280_dev);
    context->client = client;

    context->reset = NULL;
    context->cal_show = bmp280_cal_show;
    context->data_show = bmp280_data_show;
    context->osrs_show = bmp280_osrs_show;
    context->osrs_store = bmp280_osrs_store;
    context->stby_to_int = bmp280_stby_to_int;
    context->int_to_stby = bmp280_int_to_stby;

    mutex_init(&context->i2c_lock);
    i2c_set_clientdata(client, context);

    if (bmp280_update_cal(client,
        (struct bmp280_dev*)context->data) < 0) {
        return -1;
    }

    return bmx280_setup_attrs(&client->dev, bmp280_attr_groups);
}
