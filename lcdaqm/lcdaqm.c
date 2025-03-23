/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "lcdaqm.h"

#include "hardware/i2c.h"

uint8_t cursor_line = 0;  // 現在の行. 1行目なら0, 2行目なら1.
uint8_t cursor_char = 0;  // 現在の入力位置. 左端が0.

// I2Cインスタンスとピンを初期化
// すでに初期化している場合は不要
void lcdaqm_init_i2c() {
  i2c_init(LCDAQM_I2C_INST, 100000);

  gpio_init(LCDAQM_I2C_SDA_PIN);
  gpio_pull_up(LCDAQM_I2C_SDA_PIN);
  gpio_set_function(LCDAQM_I2C_SDA_PIN, GPIO_FUNC_I2C);

  gpio_init(LCDAQM_I2C_SCL_PIN);
  gpio_pull_up(LCDAQM_I2C_SCL_PIN);
  gpio_set_function(LCDAQM_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

// LCDにI2Cで書き込み
// LCDはACKに応答しないため戻り値を確認しないで書き込みを継続する
//
// Args:
//   reg_addr: レジスターアドレス
//   data: データ
//
// Returns: i2c_write_blockingの戻り値.
int lcdaqm_write_register(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[] = {reg_addr, data};
  return i2c_write_blocking(LCDAQM_I2C_INST, LCDAQM_I2C_ADDRESS, buf, sizeof(buf), false);
}

// LCDの初期化
void lcdaqm_init() {
  lcdaqm_write_register(0, 0x38);
  lcdaqm_write_register(0, 0x39);
  lcdaqm_write_register(0, 0x14);
  lcdaqm_write_register(0, 0x70);
  lcdaqm_write_register(0, 0x56);
  lcdaqm_write_register(0, 0x6C);
  lcdaqm_delay(250);
  lcdaqm_write_register(0, 0x38);
  lcdaqm_write_register(0, 0x0C);
  lcdaqm_clear();
}

// LCDに文字列を書き込み
// 行末に達した場合は自動的に改行する
void lcdaqm_print(const char* str) {
  for (int i = 0; i < 16; i++) {
    if (str[i] == 0) return;  // NULL文字

    lcdaqm_write_register(0x40, str[i]);

    cursor_char++;
    if (cursor_char > 7) {
      if (cursor_line == 1) {
        lcdaqm_goto_line(0);
      } else {
        lcdaqm_goto_line(1);
      }
    }
  }
}

// 画面をクリアしてカーソル左上に戻す
void lcdaqm_clear() {
  cursor_char = 0;
  cursor_line = 0;
  lcdaqm_write_register(0, 1);
  lcdaqm_delay(2);
}

// カーソルを指定行の一番左に戻す
//
// Args:
//   line: 1行目なら0, 2行目なら1
void lcdaqm_goto_line(uint line) {
  if (line > 1) return;
  cursor_char = 0;
  cursor_line = line;
  switch (line) {
    case 0:
      lcdaqm_write_register(0, 2);
      break;
    case 1:
      lcdaqm_write_register(0, 0xC0);
      break;
  }
  lcdaqm_delay(2);
}
