/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "scd41.h"

int main() {
  stdio_init_all();
  scd41_init_i2c();  // 通信に使うI2Cインスタンスとピンを初期化
  printf("-------------\n");
  scd41_start_periodic_measurement();  // 5秒おきのセンサーの継続測定を開始

  // 10回測定する
  for (int i = 0; i < 10; i++) {
    if (scd41_read_measurement(10)) {       // センサーの測定結果が更新されるのを待つ. タイムアウト10秒.
      printf("CO2: %u[ppm]\n", scd41_co2);  // scd41_co2に入った測定結果を表示
    } else {
      // 測定失敗
      printf("Measurement failed\n");
      break;
    }
  }
  scd41_stop_periodic_measurement(true);  // センサーの継続測定を終了
  printf("Finished\n");
}
