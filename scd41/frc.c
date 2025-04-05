/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "scd41.h"

#define FRC_TARGET 450  // 手動キャリブレーションを行う環境のCO2濃度[ppm]. 外気で行う場合は400-450.
#define LED_PIN 3       // ステータース表示用LEDのピン番号.

uint64_t serial_number;

int main() {
  stdio_init_all();
  scd41_init_i2c();                 // 通信に使うI2Cインスタンスとピンを初期化
  gpio_init(LED_PIN);               // LEDピン初期化
  gpio_set_dir(LED_PIN, GPIO_OUT);  // LEDピンを出力に設定

  printf("-------------\n");
  scd41_stop_periodic_measurement(true);  // センサーが継続測定中の場合も想定して停止コマンド

  // シリアル番号取得コマンドでセンサーと通信できているか確認
  if (!scd41_get_serial_number(&serial_number)) {
    panic("Failed to communicate with sensor\n");  // 通信失敗なら終了
  }

  // 継続測定を開始
  printf("Start periodic measurement\n");
  scd41_start_periodic_measurement();

  // 6分待機. LED点滅.
  for (int i = 0; i < 360; i++) {
    gpio_put(LED_PIN, 1);
    sleep_ms(100);
    gpio_put(LED_PIN, 0);
    sleep_ms(900);
    if (scd41_read_measurement(0)) {
      printf("CO2: %u[ppm]\n", scd41_co2);  // 測定値が更新されたら表示
    }
  }

  // 継続測定を停止
  printf("Stop periodic measurement\n");
  scd41_stop_periodic_measurement(true);

  // 手動キャリブレーション開始
  printf("Perform forced recalibration(FRC)\n");
  scd41_perform_forced_recalibration(FRC_TARGET);

  // LED点灯
  gpio_put(LED_PIN, 1);

  printf("Finished\n");

  while (1);
}
