/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "tsl2572.h"

int main() {
  stdio_init_all();
  tsl2572_init_i2c();  // 通信に使うI2Cインスタンスとピンを初期化
  printf("-------------\n");

  while (1) {
    // 測定を1回行い, 成功したらtsl2572_illuminance変数に明るさを入れる
    bool ret = tsl2572_single_auto_measure();
    if (ret) {
      // 測定成功
      printf("%.1f[lux]\n", tsl2572_illuminance);  // 明るさを表示
    } else {
      printf("TSL2572 not found\n");  // 測定失敗
    }
    sleep_ms(3000);
  }
}
