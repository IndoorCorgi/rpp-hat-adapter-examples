/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "scd41.h"

#define AUTO_CAL_SET 0  // 自動キャリブレーションを有効にしたい場合は1, 無効にしたい場合は0.

uint64_t serial_number;

int main() {
  stdio_init_all();
  scd41_init_i2c();  // 通信に使うI2Cインスタンスとピンを初期化
  printf("-------------\n");
  scd41_stop_periodic_measurement(true);  // センサーが継続測定中の場合も想定して停止コマンド

  // シリアル番号取得コマンドでセンサーと通信できているか確認
  if (!scd41_get_serial_number(&serial_number)) {
    panic("Failed to communicate with sensor\n");  // 通信失敗なら終了
  }

  // 現在の自動キャリブレーションの状態を表示. 1なら有効, 0なら無効.
  printf("Auto calibration setting: %u\n", scd41_get_automatic_self_calibration_enabled());

  // 自動キャリブレーションの値を書き込む
  printf("Setting auto calibration to %u\n", AUTO_CAL_SET);
  scd41_set_automatic_self_calibration_enabled(AUTO_CAL_SET);
  printf("Auto calibration setting: %u\n", scd41_get_automatic_self_calibration_enabled());

  // 電源が切れても新しい設定が反映されるように保存する
  printf("Persist settings\n");
  scd41_persist_settings(true);

  printf("Finished\n");
}
