/* SPDX-License-Identifier: GPL-2.0-only */
/**
 * @file    bmx280_reg.h
 * @author  Mikalai Naurotski (kaljan@nothern.com)
 * @version 0.0.0
 * @date    2025-07-20
 *
 * @brief   Register definitions for BMP280/BME280 sensor
 */

#ifndef BMX280_REG_H
#define BMX280_REG_H

#define BMX280_GET_FIELD(reg, mask, offset) \
    (((reg) >> (offset)) & (mask))

#define BMX280_SET_FIELD(value, reg, mask, offset) \
    ((reg) = (((reg) & (~((mask) << (offset)))) | (((value) & (mask)) << (offset))))

/* I2C Address */
#define BMX280_I2C_ADDR0       0x76
#define BMX280_I2C_ADDR1       0x77

/* Device ID */
#define BME280_DEVICE_ID       0x60
#define BMP280_DEVICE_ID       0x58
#define BMP280_DEVICE_ID_SMP1  0x57
#define BMP280_DEVICE_ID_SMP0  0x56

/* Oversampling value */
#define BMX280_OVERSAMPLING_MASK    0x07
#define BMX280_OVERSAMPLING_SKIP    0x00
#define BMX280_OVERSAMPLING1        0x01
#define BMX280_OVERSAMPLING2        0x02
#define BMX280_OVERSAMPLING4        0x03
#define BMX280_OVERSAMPLING8        0x04
#define BMX280_OVERSAMPLING16       0x05
#define BMX280_OVERSAMPLING32       0x06
#define BMX280_OVERSAMPLING64       0x07

/* 5.4.9 Register 0xFD…0xFE “hum” (_msb, _lsb) */
#define BMX280_HR_LSB              0xFE
#define BMX280_HR_MSB              0xFD

/* 5.4.8 Register 0xFA…0xFC “temp” (_msb, _lsb, _xlsb) */
#define BMX280_TR_XLSB             0xFC
#define BMX280_TR_LSB              0xFB
#define BMX280_TR_MSB              0xFA

/* 5.4.7 Register 0xF7…0xF9 “press” (_msb, _lsb, _xlsb) */
#define BMX280_PR_XLSB             0xF9
#define BMX280_PR_LSB              0xF8
#define BMX280_PR_MSB              0xF7

/* 5.4.6 Register 0xF5 “config” */
#define BMX280_CR                   0xF5
#define BMX280_CR_T_SB_OFFSET       5
#define BMX280_CR_T_SB_MASK         0x07
#define BMX280_CR_T_SB_0_5          0x00
#define BMX280_CR_T_SB_62_5         0x01
#define BMX280_CR_T_SB_125          0x02
#define BMX280_CR_T_SB_250          0x03
#define BMX280_CR_T_SB_500          0x04
#define BMX280_CR_T_SB_1000         0x05
#define BMX280_CR_T_SB_10           0x06
#define BMX280_CR_T_SB_20           0x07
#define BMX280_CR_FILTER_OFFSET     2
#define BMX280_CR_FILTER_MASK       0x07
#define BMX280_CR_SPI3W_EN          0x01

/* 5.4.5 Register 0xF4 “ctrl_meas” */
#define BMX280_CMR                  0xF4
#define BMX280_CMR_OSRS_T_OFFSET    5
#define BMX280_CMR_OSRS_P_OFFSET    2
#define BMX280_CMR_MODE_MASK        0x03
#define BMX280_CMR_MODE_SLEEP       0x00
#define BMX280_CMR_MODE_FORCED      0x01
#define BMX280_CMR_MODE_NORMAL      0x03

#define BMX280_GET_OSRS_T(reg)     BMX280_GET_FIELD(reg, \
    BMX280_OVERSAMPLING_MASK, BMX280_CMR_OSRS_T_OFFSET)

#define BMX280_GET_OSRS_P(reg)     BMX280_GET_FIELD(reg, \
    BMX280_OVERSAMPLING_MASK, BMX280_CMR_OSRS_P_OFFSET)

#define BMX280_SET_OSRS_T(value, reg)     BMX280_SET_FIELD(value, \
    reg, BMX280_OVERSAMPLING_MASK, BMX280_CMR_OSRS_T_OFFSET)

#define BMX280_SET_OSRS_P(value, reg)     BMX280_SET_FIELD(value, \
    reg, BMX280_OVERSAMPLING_MASK, BMX280_CMR_OSRS_P_OFFSET)

/* 5.4.4 Register 0xF3 “status” */
#define BMX280_SR                   0xF3
#define BMX280_SR_MEAS              0x08
#define BMX280_SR_IMUP              0x01

/* 5.4.3 Register 0xF2 “ctrl_hum” */
#define BMX280_CHR                  0xF2
#define BMX280_CHR_OSRS_H_OFFSET    0

#define BMX280_GET_OSRS_H(reg)     BMX280_GET_FIELD(reg, \
    BMX280_OVERSAMPLING_MASK, BMX280_CMR_OSRS_H_OFFSET)

#define BMX280_SET_OSRS_H(value, reg)     BMX280_SET_FIELD(value, \
    reg, BMX280_OVERSAMPLING_MASK, BMX280_CHR_OSRS_H_OFFSET)



/* Calibration data registers */
#define BMX280_REG_CALIB00          0x88
#define BMX280_REG_CALIB25          0xA1
#define BMX280_REG_CALIB26          0xE1
#define BMX280_REG_CALIB41          0xF0

/* 5.4.2 Register 0xE0 “reset” */
#define BMX280_RSTR                 0xE0
#define BMX280_RSTR_KEY             0xB6

/* 5.4.1 Register 0xD0 “id” */
#define BMX280_IDR                  0xD0

#endif /* BMX280_REG_H */
