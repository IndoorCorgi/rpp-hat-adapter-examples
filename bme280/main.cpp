/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "bme280.h"
#include "pico/stdlib.h"

// センサー制御クラスのインスタンスを作成
// 引数にI2Cデバイスアドレスを指定
BME280 bme280(0x76);

int main() {
  stdio_init_all();
  bme280.init_i2c();  // 通信に使うI2Cインスタンスとピンを初期化

  while (1) {
    bool ret = bme280.forced();  // 測定を1回行い, 成功したらtemperature, pressure, humidity変数に結果を入れる
    printf("----------------\n");
    if (ret) {
      // 測定成功
      printf("Measurement Data\n");
      printf(" Temp : %.1fC\n", bme280.temperature);     // 温度を表示
      printf(" Pressure : %.1fhPa\n", bme280.pressure);  // 気圧を表示
      printf(" Humidity : %.1f%%\n", bme280.humidity);   // 湿度を表示
    } else {
      printf("BME280 not found\n");  // 測定失敗
    }
    sleep_ms(3000);
  }
}
