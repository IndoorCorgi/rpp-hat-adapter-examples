/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "bme280.h"

#include <stdio.h>

// コンストラクタ
//
// Args:
//   i2c_addr: I2Cデバイスアドレス
//   i2c: 使用するI2Cインスタンス. 例:i2c0
//   i2c_sda_pin: I2C SDAピン
//   i2c_scl_pin: I2C SCLピン
BME280::BME280(uint8_t i2c_addr, i2c_inst_t* i2c, uint i2c_sda_pin, uint i2c_scl_pin)
    : i2c_addr(i2c_addr), i2c(i2c), i2c_sda_pin(i2c_sda_pin), i2c_scl_pin(i2c_scl_pin) {
}

// I2Cを初期化
// すでに初期化している場合は不要
void BME280::init_i2c(uint baudrate) {
  i2c_init(i2c, baudrate);

  gpio_init(i2c_sda_pin);
  gpio_pull_up(i2c_sda_pin);
  gpio_set_function(i2c_sda_pin, GPIO_FUNC_I2C);

  gpio_init(i2c_scl_pin);
  gpio_pull_up(i2c_scl_pin);
  gpio_set_function(i2c_scl_pin, GPIO_FUNC_I2C);
}

// I2Cでセンサーのレジスターのデータを連続して読み出す
//
// Args:
//   reg_addr: レジスターアドレス
//   data: 読み出しデータ格納バッファー
//   length: バイト数
//
// Returns: 成功でtrue, 失敗でfalse
bool BME280::read_registers(uint8_t reg_addr, uint8_t* data, uint32_t length) {
  if (1 != i2c_write_blocking(i2c, i2c_addr, &reg_addr, 1, true))
    return false;
  if (length != i2c_read_blocking(i2c, i2c_addr, data, length, false))
    return false;
  return true;
}

// I2Cでセンサーのレジスターのデータを読み出す
//
// Args:
//   reg_addr: レジスターアドレス
//
// Returns: データ
uint8_t BME280::read_register(uint8_t reg_addr) {
  uint8_t read_buf = 0;
  read_registers(reg_addr, &read_buf, 1);
  return read_buf;
}

// I2Cでセンサーのレジスターにデータを書き込む
//
// Args:
//   reg_addr: レジスターアドレス
//   data: データ
//
// Returns: 成功でtrue, 失敗でfalse
bool BME280::write_register(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[] = {reg_addr, data};
  if (sizeof(buf) != i2c_write_blocking(i2c, i2c_addr, buf, sizeof(buf), false))
    return false;
  return true;
}

// センサーからIDを読み出して期待値と一致するか確認
//
// Returns: 成功でtrue, 失敗でfalse
bool BME280::check_id() {
  if (read_register(0xD0) != 0x60) {
    return false;
  }
  return true;
}

// ステータスレジスターの値を読み出す
//
// Returns:
//   bit#3: measuring bit. 1で測定中. 0で測定中ではない.
//   bit#0: im_update bit. 1で転送中. 0で転送中ではない.
uint8_t BME280::read_status() {
  return read_register(0xF3) & 0x9;
}

// センサーからキャリブレーションレジスターの値を読み出し, calibration_dataに入れる
void BME280::read_calibration_data() {
  uint8_t buf[2];

  // T, P
  read_registers(0x88, calibration_data.byte, CAL_LENGTH_T_AND_P);

  // H1
  calibration_data.byte[24] = read_register(0xA1);

  // H2
  calibration_data.byte[26] = read_register(0xE1);
  calibration_data.byte[27] = read_register(0xE2);

  // H3
  calibration_data.byte[28] = read_register(0xE3);

  // H4
  read_registers(0xE4, buf, 2);
  calibration_data.byte[30] = ((buf[0] & 0xF) << 4) + (buf[1] & 0xF);
  calibration_data.byte[31] = buf[0] >> 4;

  // H5
  read_registers(0xE5, buf, 2);
  calibration_data.byte[32] = (buf[0] >> 4) + ((buf[1] & 0xF) << 4);
  calibration_data.byte[33] = buf[1] >> 4;

  // H6
  calibration_data.byte[34] = read_register(0xE7);
}

//  ADCレジスターの値を読み出してadc_temperature, adc_pressure, adc_humidityに入れる
void BME280::read_adc() {
  uint8_t buf[8];
  read_registers(0xF7, buf, 8);
  adc_pressure = (buf[0] << 12) + (buf[1] << 4) + (buf[2] >> 4);
  adc_temperature = (buf[3] << 12) + (buf[4] << 4) + (buf[5] >> 4);
  adc_humidity = (buf[6] << 8) + buf[7];
}

// コンフィグレジスター(config)にNormal mode測定間隔, フィルター設定を書き込む
//
// Args:
//   t_standby: Normal mode測定間隔. T_STANDBY_xで指定.
//   filter: 測定値の平滑化を行うかどうか. FILTER_xで指定.
void BME280::write_config(uint8_t t_standby, uint8_t filter) {
  write_register(0xF5, (t_standby << 5) | (filter << 2));
}

// コントロールレジスター(ctrl)にオーバーサンプリング設定, 測定モードを書き込む
//
// Args:
//   mode: センサーモード. MODE_xで指定. FORCED, NORMALを書くと測定が始まる
//   os_temperature: 温度のオーバーサンプリング. OVER_SAMPLING_xで指定.
//   os_pressure: 気圧のオーバーサンプリング. OVER_SAMPLING_xで指定.
//   os_humidity: 湿度のオーバーサンプリング. OVER_SAMPLING_xで指定.
void BME280::write_ctrl(uint8_t mode, uint8_t os_temperature, uint8_t os_pressure, uint8_t os_humidity) {
  write_register(0xF2, os_humidity);
  write_register(0xF4, (os_temperature << 5) | (os_pressure << 2) | mode);
}

// リセットレジスターに書き込んでソフトウェアリセットする
void BME280::write_reset() {
  write_register(0xE0, 0xB6);
}

// Forcedモードで測定を行い, 結果をtemperature, pressure, humidityに入れる
//
// Returns:
//   bool: 成功でTrue, IDチェック失敗でFalse
bool BME280::forced() {
  if (!check_id()) return false;
  write_config();
  write_ctrl(MODE_FORCED, OVER_SAMPLING_16, OVER_SAMPLING_16, OVER_SAMPLING_16);
  while (0 != read_status()) {
    bme280_delay(1);
  }
  read_measured_values();
  return true;
}

// キャリブレーションデータとADCレジスターを読み出し, 結果をtemperature, pressure, humidityに入れる
void BME280::read_measured_values() {
  while (0 != (read_status() & 0x8)) bme280_delay(1);  // 測定中の場合は待機
  read_calibration_data();
  read_adc();

  temperature = compensate_temperature();
  pressure = compensate_pressure();
  humidity = compensate_humidity();
}

// calibration_dataとadc_temperatureの値から気圧を計算し, temperatureに入れる
float BME280::compensate_temperature() {
  int32_t var1, var2, T;
  var1 = ((((adc_temperature >> 3) - ((int32_t)calibration_data.dig_T1 << 1))) * ((int32_t)calibration_data.dig_T2)) >> 11;
  var2 = (((((adc_temperature >> 4) - ((int32_t)calibration_data.dig_T1)) * ((adc_temperature >> 4) - ((int32_t)calibration_data.dig_T1))) >> 12) *
          ((int32_t)calibration_data.dig_T3)) >>
         14;
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return ((float)T / 100);
}

// calibration_dataとadc_pressureの値から気圧を計算し, pressureに入れる
// compensate_temperatureで計算したt_fineの値を利用するので前もって実行が必要
float BME280::compensate_pressure() {
  int64_t var1, var2, p;
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)calibration_data.dig_P6;
  var2 = var2 + ((var1 * (int64_t)calibration_data.dig_P5) << 17);
  var2 = var2 + (((int64_t)calibration_data.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)calibration_data.dig_P3) >> 8) + ((var1 * (int64_t)calibration_data.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calibration_data.dig_P1) >> 33;
  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_pressure;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)calibration_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)calibration_data.dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)calibration_data.dig_P7) << 4);
  return ((float)(uint32_t)p) / 25600;
}

// calibration_dataとadc_humidityの値から気圧を計算し, humidityに入れる
// compensate_temperatureで計算したt_fineの値を利用するので前もって実行が必要
float BME280::compensate_humidity() {
  int32_t v_x1_u32r;
  v_x1_u32r = (t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_humidity << 14) - (((int32_t)calibration_data.dig_H4) << 20) -
                  (((int32_t)calibration_data.dig_H5) * v_x1_u32r)) +
                 ((int32_t)16384)) >>
                15) *
               (((((((v_x1_u32r * ((int32_t)calibration_data.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)calibration_data.dig_H3)) >> 11) +
                     ((int32_t)32768))) >>
                   10) +
                  ((int32_t)2097152)) *
                     ((int32_t)calibration_data.dig_H2) +
                 8192) >>
                14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)calibration_data.dig_H1)) >>
                            4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  return ((float)(uint32_t)(v_x1_u32r >> 12)) / 1024;
}

void BME280::print_calibration_data() {
  printf("Calibration Data\n");
  printf(" dig_T1 : %d\n", calibration_data.dig_T1);
  printf(" dig_T2 : %d\n", calibration_data.dig_T2);
  printf(" dig_T3 : %d\n", calibration_data.dig_T3);
  printf(" dig_P1 : %d\n", calibration_data.dig_P1);
  printf(" dig_P2 : %d\n", calibration_data.dig_P2);
  printf(" dig_P3 : %d\n", calibration_data.dig_P3);
  printf(" dig_P4 : %d\n", calibration_data.dig_P4);
  printf(" dig_P5 : %d\n", calibration_data.dig_P5);
  printf(" dig_P6 : %d\n", calibration_data.dig_P6);
  printf(" dig_P7 : %d\n", calibration_data.dig_P7);
  printf(" dig_P8 : %d\n", calibration_data.dig_P8);
  printf(" dig_P9 : %d\n", calibration_data.dig_P9);
  printf(" dig_H1 : %d\n", calibration_data.dig_H1);
  printf(" dig_H2 : %d\n", calibration_data.dig_H2);
  printf(" dig_H3 : %d\n", calibration_data.dig_H3);
  printf(" dig_H4 : %d\n", calibration_data.dig_H4);
  printf(" dig_H5 : %d\n", calibration_data.dig_H5);
  printf(" dig_H6 : %d\n", calibration_data.dig_H6);
}

void BME280::print_adc() {
  printf("ADC data\n");
  printf(" adc_temperature : 0x%X\n", adc_temperature);
  printf(" adc_pressure : 0x%X\n", adc_pressure);
  printf(" adc_humitiy : 0x%X\n", adc_humidity);
}

void BME280::print_measurement_data() {
  printf("Measurement Data\n");
  printf(" Temp : %.1fC\n", temperature);
  printf(" Pressure : %.1fhPa\n", pressure);
  printf(" Humidity : %.1f%%\n", humidity);
}
