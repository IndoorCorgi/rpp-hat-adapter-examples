/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SCD41_H
#define SCD41_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------
// Configurations

#define SCD41_I2C_INST i2c_default                  // 使用するI2Cインスタンス. 例:i2c0
#define SCD41_I2C_BAUD 100000                       // I2C周波数[Hz]
#define SCD41_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN  // I2C SDAピン
#define SCD41_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN  // I2C SCLピン
#define SCD41_I2C_ADDRESS 0x62                      // I2Cデバイスアドレス
#define scd41_delay(x) sleep_ms(x)                  // xミリ秒待機

// -----------------

extern uint16_t scd41_co2;
extern float scd41_temperature;
extern float scd41_humidity;

void scd41_init_i2c();
bool scd41_read_registers(uint16_t reg_addr, uint8_t* data, uint32_t length);
bool scd41_write_registers(uint16_t reg_addr, uint8_t* data, uint32_t length);
uint8_t scd41_calculate_crc(uint8_t* data, uint32_t length);
void scd41_start_periodic_measurement();
void scd41_start_low_power_periodic_measurement();
bool scd41_measure_single_shot(uint timeout);
void scd41_stop_periodic_measurement(bool wait);
bool scd41_get_data_ready_status();
bool scd41_read_measurement(uint timeout);
void scd41_set_temperature_offset(float offset);
float scd41_get_temperature_offset();
void scd41_set_sensor_altitude(uint16_t altitude);
uint16_t scd41_get_sensor_altitude();
void scd41_set_ambient_pressure(uint16_t pressure);
bool scd41_perform_forced_recalibration(uint16_t target);
void scd41_set_automatic_self_calibration_enabled(bool enable);
bool scd41_get_automatic_self_calibration_enabled();
void scd41_persist_settings(bool wait);
void scd41_perform_factory_reset(bool wait);
void scd41_reinit();
bool scd41_get_serial_number(uint64_t* serial_number);

#ifdef __cplusplus
}
#endif

#endif  // SCD41_H