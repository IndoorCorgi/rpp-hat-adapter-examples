/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "scd41.h"

#include <math.h>
#include <string.h>
#include "hardware/i2c.h"

uint16_t scd41_co2 = 0;
float scd41_temperature = 0.0f;
float scd41_humidity = 0.0f;

// I2Cインスタンスとピンを初期化
// すでに初期化している場合は不要
void scd41_init_i2c() {
  i2c_init(SCD41_I2C_INST, SCD41_I2C_BAUD);

  gpio_init(SCD41_I2C_SDA_PIN);
  gpio_pull_up(SCD41_I2C_SDA_PIN);
  gpio_set_function(SCD41_I2C_SDA_PIN, GPIO_FUNC_I2C);

  gpio_init(SCD41_I2C_SCL_PIN);
  gpio_pull_up(SCD41_I2C_SCL_PIN);
  gpio_set_function(SCD41_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

// I2Cでセンサーのレジスターのデータを連続して読み出す
//
// Args:
//   reg_addr: レジスターアドレス
//   data: 読み出しデータ格納バッファー
//   length: バイト数
//
// Returns: 成功でtrue, 失敗でfalse
bool scd41_read_registers(uint16_t reg_addr, uint8_t* data, uint32_t length) {
  uint8_t addr_write_data[] = {reg_addr >> 8, reg_addr & 0xFF};
  if (2 != i2c_write_timeout_us(SCD41_I2C_INST, SCD41_I2C_ADDRESS, addr_write_data, 2, true, 10000))
    return false;
  if (length != i2c_read_blocking(SCD41_I2C_INST, SCD41_I2C_ADDRESS, data, length, false))
    return false;
  return true;
}

// I2Cでセンサーのレジスターにデータを連続して書き込む
//
// Args:
//   reg_addr: レジスターアドレス
//   data: 書き込みデータ
//   length: バイト数. 0ならレジスターアドレスのみ書き込む(コマンド)
//
// Returns: 成功でtrue, 失敗でfalse
bool scd41_write_registers(uint16_t reg_addr, uint8_t* data, uint32_t length) {
  uint8_t write_data[length + 3];
  write_data[0] = reg_addr >> 8;
  write_data[1] = reg_addr & 0xFF;
  if (length == 0) {
    if (2 != i2c_write_blocking(SCD41_I2C_INST, SCD41_I2C_ADDRESS, write_data, 2, false))
      return false;
  } else {
    memcpy(write_data + 2, data, length);
    *(write_data + 2 + length) = scd41_calculate_crc(data, length);
    if (length + 3 != i2c_write_blocking(SCD41_I2C_INST, SCD41_I2C_ADDRESS,
                                         write_data, length + 3, false))
      return false;
  }
  return true;
}

// CRCを計算
//
// Args:
//   seed: シード値. デフォルト0xFF. 分割されたデータで計算を継続する場合は前回のCRC値.
//   data: CRCを計算するデータ
//   length: バイト数
//
// Returns: CRCの値
uint8_t scd41_calculate_crc(uint8_t* data, uint32_t length) {
  uint8_t crc = 0xFF;
  for (uint i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (0 == (crc & 0x80))
        crc <<= 1;
      else
        crc = (crc << 1) ^ 0x31;
    }
  }
  return crc;
}

// 5秒おきの定期測定を開始. 測定結果を読み出すにはscd41_read_measurementを呼ぶ必要がある.
// 測定中は使用できるコマンドが以下に制限される. データシート参照.
// - read_measurement
// - stop_periodic_measurement
// - set_ambient_pressure
// - get_data_ready_status
void scd41_start_periodic_measurement() {
  scd41_write_registers(0x21b1, NULL, 0);
}

// 30秒おきの定期測定を開始. 測定結果を読み出すにはscd41_read_measurementを呼ぶ必要がある.
// 測定中は使用できるコマンドが以下に制限される. データシート参照.
// - stop_periodic_measurement
// - set_ambient_pressure
// - get_data_ready_status
void scd41_start_low_power_periodic_measurement() {
  scd41_write_registers(0x21ac, NULL, 0);
}

// 単発の測定を開始. 測定完了まで5秒かかる. 電源投入後は, 正確な測定のため, 3回以上の測定が推奨.
//
//     Args:
//       timeout: 新しい測定データを待つ秒数. 0だと待たずに処理を返す. 推奨は10程度.
//
//     Returns: 成功でtrue, タイムアウトか失敗でfalse.
//              成功の場合はco2, temperature, humidityの値が更新される
bool scd41_measure_single_shot(uint timeout) {
  scd41_write_registers(0x219d, NULL, 0);
  return scd41_read_measurement(timeout);
}

// 定期測定を終了
//
// Args:
//   wait: trueなら停止までの500ms以上待機する. Falseならすぐに処理を返す.
void scd41_stop_periodic_measurement(bool wait) {
  scd41_write_registers(0x3f86, NULL, 0);
  if (wait) scd41_delay(600);
}

// 新しい測定データがあるか確認
//
// Returns: 新しいデータがあればtrue, 無ければfalse
bool scd41_get_data_ready_status() {
  uint8_t data[3] = {};
  scd41_read_registers(0xe4b8, data, 3);

  if (scd41_calculate_crc(data, 2) != data[2]) return false;  // CRC不一致
  if ((data[1] & 0x3) == 0) return false;
  return true;
}

// 測定データを読み出す. 成功した場合はco2, temperature, humidityの値が更新される
//
// Args:
//   timeout: 新しい測定データを待つ秒数. 0だと待たずに処理を返す.
//
// Returns: 成功でtrue, 失敗かタイムアウトでfalse
bool scd41_read_measurement(uint timeout) {
  // 100msおきに測定データがあるかチェック
  for (uint i = 0; i <= timeout * 10; i++) {
    if (scd41_get_data_ready_status()) break;  // 新しいデータがある
    if (i >= timeout * 10) return false;       // タイムアウト
    scd41_delay(100);
  }

  uint8_t data[9] = {};
  scd41_read_registers(0xec05, data, 9);

  // CRC確認
  for (uint i = 0; i < 3; i++) {
    if (scd41_calculate_crc(data + i * 3, 2) != data[i * 3 + 2]) return false;
  }

  scd41_co2 = ((uint16_t)data[0]) << 8 | data[1];
  scd41_temperature = -45 + 175 * ((((uint16_t)data[3]) << 8) + data[4]) / pow(2, 16);
  scd41_humidity = 100 * ((((uint16_t)data[6]) << 8) + data[7]) / pow(2, 16);
}

// 測定値補正用の温度オフセット値を書き込む.
// 温度測定の際にオフセット値が引かれる. 実際の使用環境の発熱を考慮して決める. デフォルトは4.
// 電源立ち下げ後も設定を保存するにはpersist_settingsコマンドを実行する必要がある.
//
// Args:
//   offset: オフセット[°C]の値
void scd41_set_temperature_offset(float offset) {
  uint16_t offset_w = (uint16_t)(offset * pow(2, 16) / 175);
  uint8_t data[] = {offset_w >> 8, offset_w & 0xFF};
  scd41_write_registers(0x241d, data, 2);
}

// 測定値補正用の温度オフセット値を読み出す
//
// Returns: 設定されているオフセット値を[°C]単位に直したもの
float scd41_get_temperature_offset() {
  uint8_t data[3];
  scd41_read_registers(0x2318, data, 3);

  float offset = 175 * ((((uint16_t)data[0]) << 8) + data[1]) / pow(2, 16);
  return offset;
}

// 測定値補正用の標高情報を書き込む.
// 電源立ち下げ後も設定を保存するにはpersist_settingsコマンドを実行する必要がある.
//
// Args:
//   altitude : 標高[m]の値
void scd41_set_sensor_altitude(uint16_t altitude) {
  uint8_t data[] = {altitude >> 8, altitude & 0xFF};
  scd41_write_registers(0x2427, data, 2);
}

// 測定値補正用の標高情報を読み出す
//
// Returns: 設定されている標高情報[m]
uint16_t scd41_get_sensor_altitude() {
  uint8_t data[3];
  scd41_read_registers(0x2322, data, 3);
  uint16_t altitude = data[0];
  altitude <<= 8;
  altitude |= data[1];
  return altitude;
}

// 測定値補正用の気圧情報を書き込む
//
// Args:
//   pressure: 気圧[hPa]の値
void scd41_set_ambient_pressure(uint16_t pressure) {
  uint8_t data[] = {pressure >> 8, pressure & 0xFF};
  scd41_write_registers(0xe000, data, 2);
}

// 手動キャリブレーション(FRC)
// 実施前に3分程度のCO2測定を行った後, 定期測定は停止しておく必要がある. データシート参照.
//
// Args:
//   target: 既知のCO2濃度[ppm]. 外気で行う場合は400.
//
// Returns: 成功ならtrue. 失敗ならfalse.
bool scd41_perform_forced_recalibration(uint16_t target) {
  uint8_t data[3];
  data[0] = target >> 8;
  data[1] = target & 0xFF;
  if (!scd41_write_registers(0x362f, data, 2)) return false;
  scd41_delay(500);  // 400ms以上待機

  if (!scd41_read_registers(0x362f, data, 3)) return false;
  if (data[0] == 0xFF && data[1] == 0xFF) return false;

  // キャリブレーションで、センサー内部のppm補正値を変化させた相対量が取得できる
  uint16_t frc_correction = (((uint16_t)data[0]) << 8) + data[1] - 0x8000;
  return true;
}

// 自動キャリブレーション(ASC)を有効/無効化する. デフォルトは有効.
// 電源立ち下げ後も設定を保存するにはpersist_settingsコマンドが必要.
// ASCを使う場合, 週1回以上の頻度で外気相当の400ppm環境が必要. データシート参照.
//
// Args:
//   enable: 有効化するならtrue. 無効化するならfalse.
void scd41_set_automatic_self_calibration_enabled(bool enable) {
  uint8_t data[] = {0, enable};
  scd41_write_registers(0x2416, data, 2);
}

// 自動キャリブレーション(ASC)状態を読み出す.
//
// Returns: 有効ならtrue. 無効ならfalse.
bool scd41_get_automatic_self_calibration_enabled() {
  uint8_t data[3] = {};
  scd41_read_registers(0x2313, data, 3);
  if (data[0] == 0 && data[1] == 1)
    return true;
  else
    return false;
}

// 設定情報をEEPROMに保存して, 電源を落としても保存されるようにする
//
// Args:
//   wait: trueなら完了までの800ms以上待機する. falseならすぐに処理を返す.
void scd41_persist_settings(bool wait) {
  scd41_write_registers(0x3615, NULL, 0);
  if (wait) scd41_delay(900);
}

// 工場出荷時の設定に戻す. 初期設定に戻り, EEPROMの設定, キャリブレーション情報も消去される.
//
// Args:
//   wait: trueなら完了までの1200ms以上待機する. falseならすぐに処理を返す.
void scd41_perform_factory_reset(bool wait) {
  scd41_write_registers(0x3632, NULL, 0);
  if (wait) scd41_delay(1300);
}

// EEPROMの設定を読み出して反映させる.
void scd41_reinit() {
  scd41_write_registers(0x3646, NULL, 0);
  scd41_delay(40);
}

// シリアル番号を取得
//
// Args:
//   serial_number: シリアル番号(48bit)格納変数. NULLなら取得できたかどうかのみ判定.
//
// Returns: 成功でtrue, 失敗でfalse
bool scd41_get_serial_number(uint64_t* serial_number) {
  uint8_t data[9];
  if (!scd41_read_registers(0x3682, data, 9)) return false;

  // CRC確認
  for (uint8_t i = 0; i < 3; i++) {
    if (scd41_calculate_crc(data + (i * 3), 2) != data[i * 3 + 2]) return false;
  }

  if (serial_number == NULL) return true;

  *serial_number = data[0];
  *serial_number <<= 8;
  *serial_number |= data[1];
  *serial_number <<= 8;
  *serial_number |= data[3];
  *serial_number <<= 8;
  *serial_number |= data[4];
  *serial_number <<= 8;
  *serial_number |= data[6];
  *serial_number <<= 8;
  *serial_number |= data[7];
  return true;
}