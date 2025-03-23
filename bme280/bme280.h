/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BME280_H
#define BME280_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// -----------------
// Configurations

#define bme280_delay(x) sleep_ms(x)  // xミリ秒待機

// -----------------

class BME280 {
 public:
  // モード
  static constexpr uint8_t MODE_SLEEP = 0;
  static constexpr uint8_t MODE_FORCED = 1;
  static constexpr uint8_t MODE_NORMAL = 3;

  // オーバーサンプリング
  static constexpr uint8_t OVER_SAMPLING_1 = 1;
  static constexpr uint8_t OVER_SAMPLING_2 = 2;
  static constexpr uint8_t OVER_SAMPLING_4 = 3;
  static constexpr uint8_t OVER_SAMPLING_8 = 4;
  static constexpr uint8_t OVER_SAMPLING_16 = 5;

  // 定期測定間隔 t_standby
  static constexpr uint8_t T_STANDBY_05MS = 0;
  static constexpr uint8_t T_STANDBY_62MS = 1;
  static constexpr uint8_t T_STANDBY_125MS = 2;
  static constexpr uint8_t T_STANDBY_250MS = 3;
  static constexpr uint8_t T_STANDBY_500MS = 4;
  static constexpr uint8_t T_STANDBY_1000MS = 5;
  static constexpr uint8_t T_STANDBY_10MS = 6;
  static constexpr uint8_t T_STANDBY_20MS = 7;

  // IIRフィルター
  static constexpr uint8_t FILTER_OFF = 0;
  static constexpr uint8_t FILTER_2 = 1;
  static constexpr uint8_t FILTER_4 = 2;
  static constexpr uint8_t FILTER_8 = 3;
  static constexpr uint8_t FILTER_16 = 4;

  // I2C
  i2c_inst_t* i2c;
  uint8_t i2c_addr;
  uint i2c_sda_pin;
  uint i2c_scl_pin;

  // キャリブレーションデータ
  static constexpr int CAL_LENGTH = 38;          // バイト数
  static constexpr int CAL_LENGTH_T_AND_P = 24;  // TとPパラメーター分のバイト数

  union CalibrationData {
    uint8_t byte[CAL_LENGTH] = {};  // バイト数指定用

    // 名前指定用
    struct {
      uint16_t dig_T1;
      int16_t dig_T2;
      int16_t dig_T3;
      uint16_t dig_P1;
      int16_t dig_P2;
      int16_t dig_P3;
      int16_t dig_P4;
      int16_t dig_P5;
      int16_t dig_P6;
      int16_t dig_P7;
      int16_t dig_P8;
      int16_t dig_P9;
      uint8_t dig_H1;
      uint8_t dummy;
      int16_t dig_H2;
      uint8_t dig_H3;
      uint8_t dummy2;
      int16_t dig_H4;
      int16_t dig_H5;
      int8_t dig_H6;
    };
  };

  union CalibrationData calibration_data;

  int32_t t_fine = 0;  // 計算用の値

  // ADCレジスターの値
  uint32_t adc_temperature = 0;
  uint32_t adc_pressure = 0;
  uint32_t adc_humidity = 0;

  float temperature = 0;  // 測定温度[℃]
  float pressure = 0;     // 測定気圧[hPa]
  float humidity = 0;     // 測定湿度[%]

  BME280(uint8_t i2c_addr = 0x76, i2c_inst_t* i2c = i2c_default,
         uint i2c_sda_pin = 4, uint i2c_scl_pin = 5);
  void init_i2c(uint baudrate = 100000);
  bool read_registers(uint8_t reg_addr, uint8_t* data, uint32_t length);
  uint8_t read_register(uint8_t reg_addr);
  bool write_register(uint8_t reg_addr, uint8_t data);
  bool check_id();
  uint8_t read_status();
  void read_calibration_data();
  void read_adc();
  void write_config(uint8_t t_standby = T_STANDBY_05MS, uint8_t filter = FILTER_OFF);
  void write_ctrl(uint8_t mode = MODE_SLEEP, uint8_t os_temperature = OVER_SAMPLING_1,
                  uint8_t os_pressure = OVER_SAMPLING_1, uint8_t os_humidity = OVER_SAMPLING_1);
  void write_reset();
  bool forced();
  void read_measured_values();
  float compensate_temperature();
  float compensate_pressure();
  float compensate_humidity();

  void print_calibration_data();
  void print_adc();
  void print_measurement_data();
};

#endif
