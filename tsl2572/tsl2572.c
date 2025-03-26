/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "tsl2572.h"

#include "hardware/i2c.h"

uint16_t tsl2572_adc_ch0 = 0;
uint16_t tsl2572_adc_ch1 = 0;
uint tsl2572_again = TSL2572_AGAIN_1;
uint tsl2572_integ_cycles = 1;
float tsl2572_illuminance = 0;

// I2Cインスタンスとピンを初期化
// すでに初期化している場合は不要
void tsl2572_init_i2c() {
  i2c_init(TSL2572_I2C_INST, TSL2572_I2C_BAUD);

  gpio_init(TSL2572_I2C_SDA_PIN);
  gpio_pull_up(TSL2572_I2C_SDA_PIN);
  gpio_set_function(TSL2572_I2C_SDA_PIN, GPIO_FUNC_I2C);

  gpio_init(TSL2572_I2C_SCL_PIN);
  gpio_pull_up(TSL2572_I2C_SCL_PIN);
  gpio_set_function(TSL2572_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

// I2Cでセンサーのレジスターのデータを連続して読み出す
//
// Args:
//   reg_addr: レジスターアドレス
//   data: 読み出しデータ格納バッファー
//   length: バイト数
//
// Returns: 成功でtrue, 失敗でfalse
bool tsl2572_read_registers(uint8_t reg_addr, uint8_t* data, uint32_t length) {
  uint8_t addr_write_data = reg_addr | 0xA0;
  if (1 != i2c_write_blocking(TSL2572_I2C_INST, TSL2572_I2C_ADDRESS, &addr_write_data, 1, true))
    return false;
  if (length != i2c_read_blocking(TSL2572_I2C_INST, TSL2572_I2C_ADDRESS, data, length, false))
    return false;
  return true;
}

// I2Cでセンサーのレジスターのデータを読み出す
//
// Args:
//   reg_addr: レジスターアドレス
//
// Returns: データ
uint8_t tsl2572_read_register(uint8_t reg_addr) {
  uint8_t read_buf = 0;
  tsl2572_read_registers(reg_addr, &read_buf, 1);
  return read_buf;
}

// I2Cでセンサーのレジスターにデータを書き込む
//
// Args:
//   reg_addr: レジスターアドレス
//   data: データ
//
// Returns: 成功でtrue, 失敗でfalse
bool tsl2572_write_register(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[] = {reg_addr | 0xA0, data};
  if (sizeof(buf) != i2c_write_blocking(TSL2572_I2C_INST, TSL2572_I2C_ADDRESS, buf, sizeof(buf), false))
    return false;
  return true;
}

// センサーからIDを読み出して期待値と一致するか確認
//
// Returns: 成功でtrue, 失敗でfalse
bool tsl2572_check_id() {
  uint8_t id = tsl2572_read_register(0x12);

  // 3.3V TSL25721は0x34, 1.8V TSL25723は0x3D
  if ((id != 0x34) && (id != 0x3D)) {
    return false;
  }
  return true;
}

// Enableレジスターに書き込む. 機能の有効, 無効を切り替える.
//
// Args:
//   pon: trueでPower ON
//   aen: trueで定期的に測定開始
//   wen: trueで測定間に待機時間を入れる
void tsl2572_write_enable(bool pon, bool aen, bool wen) {
  uint8_t data = 0;
  if (pon) data |= 0x1;
  if (aen) data |= 0x2;
  if (wen) data |= 0x8;
  tsl2572_write_register(0x0, data);
}

// ALS integration time (測定時間)を書き込む
//
// Args:
//   integ_cycles: 1-256の整数. atime = integ_cycles x 2.73[ms]
void tsl2572_write_atime(uint integ_cycles) {
  if (integ_cycles < 1 || integ_cycles > 256) return;  // 範囲外
  tsl2572_write_register(0x1, 256 - integ_cycles);
}

// ALS integration gain (倍率)を書き込む
//
// Args:
//   again: AGAIN_xで指定
void tsl2572_write_again(uint again) {
  if (TSL2572_AGAIN_016 == again) {
    tsl2572_write_register(0xD, 0x4);
    tsl2572_write_register(0xF, 0x0);
  } else if (TSL2572_AGAIN_1 == again) {
    tsl2572_write_register(0xD, 0x0);
    tsl2572_write_register(0xF, 0x0);
  } else if (TSL2572_AGAIN_8 == again) {
    tsl2572_write_register(0xD, 0x0);
    tsl2572_write_register(0xF, 0x1);
  } else if (TSL2572_AGAIN_16 == again) {
    tsl2572_write_register(0xD, 0x0);
    tsl2572_write_register(0xF, 0x2);
  } else if (TSL2572_AGAIN_120 == again) {
    tsl2572_write_register(0xD, 0x0);
    tsl2572_write_register(0xF, 0x3);
  }
}

// ステータスレジスターの値を読み出す
//
// Returns: ステータスレジスターの値.
//   bit#0: AVALID bit. 1で測定結果あり. 0で測定結果なし.
//   bit#4: AINT bit. 1で割り込み発生. 0で割り込みなし.
uint8_t tsl2572_read_status() {
  return tsl2572_read_register(0x13);
}

// ADCレジスターの値を読み出してadc_ch0, adc_ch1に入れる
void tsl2572_read_adc() {
  uint8_t data[4] = {};
  tsl2572_read_registers(0x14, data, 4);
  tsl2572_adc_ch0 = (data[1] << 8) | data[0];
  tsl2572_adc_ch1 = (data[3] << 8) | data[2];
}

// integ_cycles, againの設定で1回測定を行い, adc_ch0, adc_ch1にADCレジスターの値を入れる.
void tsl2572_single_als_integration() {
  tsl2572_write_enable(true, false, false);  // 一度測定を停止
  tsl2572_write_atime(tsl2572_integ_cycles);
  tsl2572_write_again(tsl2572_again);
  tsl2572_write_enable(true, true, false);  // 測定開始

  while (1) {
    uint8_t status = tsl2572_read_status();
    if (status == 0x11) {
      tsl2572_write_enable(false, false, false);  // 測定を停止
      break;
    } else {
      tsl2572_delay(10);
    }
  }

  tsl2572_read_adc();
}

// adc_ch0, adc_ch1, integ_cycles, againから照度(明るさ)を計算し, illuminanceに入れる.
void tsl2572_calculate_lux() {
  float t = tsl2572_integ_cycles * 2.73;
  float g;

  switch (tsl2572_again) {
    case TSL2572_AGAIN_016:
      g = 0.16;
      break;
    case TSL2572_AGAIN_1:
      g = 1.0f;
      break;
    case TSL2572_AGAIN_8:
      g = 8.0f;
      break;
    case TSL2572_AGAIN_16:
      g = 16.0f;
      break;
    case TSL2572_AGAIN_120:
      g = 120.0f;
      break;
    default:
      return;
  }

  float cpl = t * g / 60;
  float lux1 = (tsl2572_adc_ch0 - 1.87 * tsl2572_adc_ch1) / cpl;
  float lux2 = (0.63 * tsl2572_adc_ch0 - tsl2572_adc_ch1) / cpl;
  if (lux1 > lux2)
    tsl2572_illuminance = lux1;
  else
    tsl2572_illuminance = lux2;
}

// 条件を自動で調整しながら1回測定を行い, luxに結果を入れる
//
// Returns:
//   bool: 成功でTrue, IDチェック失敗でFalse
bool tsl2572_single_auto_measure() {
  if (!tsl2572_check_id()) return false;

  // 1度短い時間で測定する
  tsl2572_integ_cycles = 4;
  tsl2572_again = TSL2572_AGAIN_1;
  tsl2572_single_als_integration();
  uint16_t adc_max = MAX(tsl2572_adc_ch0, tsl2572_adc_ch1);
  float margin = 0.8;  // 判定マージン用倍率. ADCレジスターの上限 x margin以上に達したら条件を変える.

  // 1度目の結果をもとにinteg_cyclesとagainを決める
  if (adc_max < 8.53 * margin) {
    tsl2572_integ_cycles = 256;
    tsl2572_again = TSL2572_AGAIN_120;
  } else if (adc_max < 128 * margin) {
    tsl2572_integ_cycles = 128;
    tsl2572_again = TSL2572_AGAIN_16;
  } else if (adc_max < 512 * margin) {
    tsl2572_integ_cycles = 64;
    tsl2572_again = TSL2572_AGAIN_8;
  } else if (adc_max < 4096 * margin) {
    tsl2572_integ_cycles = 64;
    tsl2572_again = TSL2572_AGAIN_1;
  } else {
    tsl2572_integ_cycles = 64;
    tsl2572_again = TSL2572_AGAIN_016;
  }

  // 本番の測定
  tsl2572_single_als_integration();
  tsl2572_calculate_lux();
}