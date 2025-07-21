/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_common.c
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Common functionality for BMP280/BME280 sensor
 */

#include "bmx280_common.h"

#include <linux/math64.h>
#include <linux/string.h>
#include <linux/kernel.h>


int bme280_get_data_reg(uint32_t* const dst, const uint8_t* data, size_t size) {
    if ((data != NULL) && (dst != NULL)) {
        if (size == 2) {
            *dst = ((((uint32_t)(*data)) & 0x000000FFUL) << 8) |
                ((uint32_t)(*data + 1) & 0x000000FFUL);
            return 0;
        } else if (size == 3) {
            *dst = ((((uint32_t)(*data)) & 0x000000FFUL) << 12) |
                ((((uint32_t)(*(data + 1))) & 0x000000FFUL) << 4) |
                ((((uint32_t)(*(data + 2))) & 0x000000F0UL) >> 4);
            return 0;
        }
    }
    return -1;
}

int bmx280_read_cal_temp(struct bmx280_cal_temp* context,
    const uint8_t* data, uint32_t size) {
    if ((context == NULL) || (data == NULL) ||
        (size < BMX280_CAL_TEMP_SIZE)) {
        return -1;
    }

    context->dig_T1 = bme280_read_u16l(data);
    context->dig_T2 = bme280_read_i16l(&data[2]);
    context->dig_T3 = bme280_read_i16l(&data[4]);
    return BMX280_CAL_TEMP_SIZE;
}

int bmx280_read_cal_pres(struct bmx280_cal_pres* context,
    const uint8_t* data, uint32_t size) {
    if ((context == NULL) || (data == NULL) ||
        (size < BMX280_CAL_PRES_SIZE)) {
        return -1;
    }

    context->dig_P1 = bme280_read_u16l(data);
    data += sizeof(uint16_t);

    context->dig_P2 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P3 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P4 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P5 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P6 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P7 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P8 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_P9 = bme280_read_i16l(data);
    return BMX280_CAL_PRES_SIZE;
}

int bme280_read_cal_hmdt(struct bme280_cal_hmdt* context,
    const uint8_t* data, uint32_t size) {
    if ((context == NULL) || (data == NULL) ||
        (size < BME280_CAL_HMDT_SIZE)) {
        return -1;
    }

    context->dig_H1 = *data;
    data++;

    context->dig_H2 = bme280_read_i16l(data);
    data += sizeof(uint16_t);

    context->dig_H3 = *data;
    data++;

    context->dig_H4 = (((int16_t)(*data)) << 4) & 0x0FF0;
    data++;

    context->dig_H4 |= (((int16_t)(*data)) & 0x000F);
    data++;

    context->dig_H5 = (((int16_t)(*data)) & 0x00F0) >> 4;
    data++;

    context->dig_H5 |= (((int16_t)(*data)) << 4) & 0x0FF0;
    data++;

    context->dig_H6 = (int8_t)(*data);

    return BME280_CAL_HMDT_SIZE;
}

int bmp280_read_cal(struct bmp280_cal* context,
    const uint8_t* data, uint32_t size) {
    int ret;
    if (context == NULL) {
        return -1;
    }
    ret = bmx280_read_cal_temp(&context->temp, data, size);
    if (ret <= 0) {
        return ret;
    }
    size -= ret;
    data += ret;

    ret = bmx280_read_cal_pres(&context->pres, data, size);
    if (ret <= 0) {
        return ret;
    }

    return BME280_CAL_SIZE;
}

int bme280_read_cal(struct bme280_cal* context,
    const uint8_t* data, uint32_t size) {
    int ret;
    if (context == NULL) {
        return -1;
    }
    ret = bmx280_read_cal_temp(&context->temp, data, size);
    if (ret <= 0) {
        return ret;
    }
    size -= ret;
    data += ret;

    ret = bmx280_read_cal_pres(&context->pres, data, size);
    if (ret <= 0) {
        return ret;
    }

    size -= ret;
    data += ret;

    ret = bme280_read_cal_hmdt(&context->hmdt, data, size);
    if (ret <= 0) {
        return ret;
    }
    return BME280_CAL_SIZE;
}

int32_t bmx280_comp_temp(const struct bmx280_cal_temp* cal,
    int32_t adc_T, int32_t* const tfptr) {

    int32_t t_fine;
	int32_t var1, var2, T;

	var1 = ((((adc_T >> 3) - ((int32_t)cal->dig_T1 << 1))) *
        ((int32_t)cal->dig_T2)) >> 11;

	var2 = (((((adc_T >> 4) - ((int32_t)cal->dig_T1)) *
        ((adc_T >> 4) - ((int32_t)cal->dig_T1))) >> 12) *
        ((int32_t)cal->dig_T3)) >> 14;

	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
    if (tfptr) {
        *tfptr = t_fine;
    }
	return T;
}

uint32_t bmx280_comp_pres(const struct bmx280_cal_pres* cal,
    int32_t adc_P, int32_t t_fine) {

	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)cal->dig_P6;
	var2 = var2 + ((var1 * (int64_t)cal->dig_P5) << 17);
	var2 = var2 + (((int64_t)cal->dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)cal->dig_P3)>>8) +
        ((var1 * (int64_t)cal->dig_P2) << 12);

	var1 = (((((int64_t)1) << 47) + var1)) *
        ((int64_t)cal->dig_P1) >> 33;

	if (var1 == 0) {
		return 0;
	}
	p = 1048576 - adc_P;

    p = div64_s64((((p << 31) - var2) * 3125), var1);
	var1 = (((int64_t)cal->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)cal->dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)cal->dig_P7) << 4);
	return (uint32_t)p;
}

uint32_t bme280_comp_hmdt(const struct bme280_cal_hmdt* cal,
    int32_t adc_H, int32_t t_fine) {

	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)cal->dig_H4) << 20) -
        (((int32_t)cal->dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
        (((((((v_x1_u32r * ((int32_t)cal->dig_H6)) >> 10) *
        (((v_x1_u32r * ((int32_t)cal->dig_H3)) >> 11) +
        ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
		((int32_t)cal->dig_H2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) *
        (v_x1_u32r >> 15)) >> 7) * ((int32_t)cal->dig_H1)) >> 4));

	v_x1_u32r = ((v_x1_u32r < 0) ? 0 : v_x1_u32r);
	v_x1_u32r = ((v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r);

	return (uint32_t)(v_x1_u32r >> 12);
}

int bmp280_conv_data(struct bmp280_data* dst, struct bmp280_cal* cal,
    const uint8_t * data, uint32_t size) {
    uint32_t t_raw, p_raw;
    if ((dst == NULL) || (data == NULL) || (cal == NULL)) {
        return -1;
    } else if (bme280_get_data_reg(&t_raw, data, 3) < 0) {
        return -1;
    } else if (bme280_get_data_reg(&p_raw, &data[3], 3) < 0) {
        return -1;
    }

    dst->temperature = bmx280_comp_temp(&cal->temp, t_raw, &dst->t_fine);
    dst->pressure = bmx280_comp_pres(&cal->pres, p_raw, dst->t_fine);

    return 0;
}

int bme280_conv_data(struct bme280_data* dst, struct bme280_cal* cal,
    const uint8_t * data, uint32_t size) {
    uint32_t t_raw, p_raw, h_raw;
    if ((dst == NULL) || (data == NULL) || (cal == NULL)) {
        return -1;
    } else if (bme280_get_data_reg(&p_raw, data, 3) < 0) {
        return -1;
    } else if (bme280_get_data_reg(&t_raw, &data[3], 3) < 0) {
        return -1;
    } else if (bme280_get_data_reg(&h_raw, &data[6], 2) < 0) {
        return -1;
    }

    dst->temperature = bmx280_comp_temp(&cal->temp, t_raw, &dst->t_fine);
    dst->pressure = bmx280_comp_pres(&cal->pres, p_raw, dst->t_fine);
    dst->humidity = bme280_comp_hmdt(&cal->hmdt, h_raw, dst->t_fine);

    return 0;
}


const char* bmx280_osrs_to_str(uint8_t value) {
    switch (value & 0x07) {
    case 0: return "skipped";
    case 1: return "1";
    case 2: return "2";
    case 3: return "4";
    case 4: return "8";
    default:
        break;
    }
    return "16";
}

int8_t bmx280_str_to_osrs(const char* str) {
    int val;
    uint8_t ret = -1;
    if ((strcmp(str, "skipped") == 0) ||
        (strcmp(str, "skip") == 0)) {
        ret = 0;
    } else if (kstrtoint(str, 10, &val) == 0) {
        switch (val) {
        case 1: ret = 1; break;
        case 2: ret = 2; break;
        case 4: ret = 3; break;
        case 8: ret = 4; break;
        default: break;
        }
    }
    return ret;
}

const char* bmx280_mode_to_str(uint8_t value) {
    switch (value & 0x03) {
    case 0: return "sleep";
    case 1: return "forced_1";
    case 2: return "forced_2";
    case 3: return "normal";
    default:
        break;
    }
    return "undefined";
}

int8_t bmx280_str_to_mode(const char* str) {
    if (strcmp(str, "normal") == 0) {
        return 3;
    } else if (strcmp(str, "forced_2") == 0) {
        return 2;
    } else if ((strcmp(str, "forced_1") == 0) ||
        (strcmp(str, "forced") == 0)) {
        return 1;
    } else if (strcmp(str, "sleep") == 0) {
        return 0;
    }
    return -1;
}

int bmx280_stby_to_int(uint8_t value) {
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

int8_t bmx280_int_to_stby(int value) {
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

const char* bmx280_filt_to_str(uint8_t value) {
    switch (value & 0x07) {
    case 0: return "off";
    case 1: return "2";
    case 2: return "4";
    case 3: return "8";
    default: break;
    }
    return "16";
}

int8_t bmx280_str_to_filt(const char* str) {
    int val;
    int8_t ret = -1;
    if ((strcmp(str, "off") == 0) ||
        (strcmp(str, "0") == 0)) {
        ret = 0;
    } else if (kstrtoint(str, 10, &val) == 0) {
        switch (val) {
        case 1: ret = 0; break;
        case 2: ret = 1; break;
        case 4: ret = 2; break;
        case 8: ret = 3; break;
        default: break;
        }
    }
    return ret;
}
