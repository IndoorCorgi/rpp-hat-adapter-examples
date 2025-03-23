/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "lcdaqm.h"

int main() {
  lcdaqm_init_i2c();  // I2C初期化. 別途初期化する場合は不要.
  lcdaqm_init();      // LCD初期化

  while (1) {
    lcdaqm_print("HELLO");  // 文字列を表示
    sleep_ms(1000);

    lcdaqm_goto_line(1);    // 2行目の先頭にカーソル移動
    lcdaqm_print("WORLD");  // 文字列を表示
    sleep_ms(1000);

    lcdaqm_clear();  // 画面をクリアしてカーソルを左上に移動
    sleep_ms(1000);
  }
}
