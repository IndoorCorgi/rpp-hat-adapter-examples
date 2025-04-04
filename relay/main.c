/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "pico/stdlib.h"

#define RELAY_PIN 11    // リレーが接続されているPicoのGPIO番号
#define RED_SW_PIN 8    // 赤スイッチが接続されているPicoのGPIO番号
#define BLACK_SW_PIN 9  // 黒スイッチが接続されているPicoのGPIO番号

int main() {
  gpio_init(RELAY_PIN);               // リレー用のGPIOを初期化
  gpio_set_dir(RELAY_PIN, GPIO_OUT);  // リレー用のGPIOを出力に変更

  // スイッチ用のGPIOを初期化
  gpio_init(RED_SW_PIN);
  gpio_init(BLACK_SW_PIN);

  // スイッチ用のGPIOをプルアップ
  gpio_pull_up(RED_SW_PIN);
  gpio_pull_up(BLACK_SW_PIN);
  sleep_ms(5);  // プルアップによるGPIOの電圧が安定するまで待機

  // 無限ループ
  while (1) {
    if (0 == gpio_get(RED_SW_PIN)) {
      // 赤スイッチが押されている場合 (スイッチが押されるとGNDと導通してLow, 0が入力される)
      gpio_put(RELAY_PIN, 1);  // リレーON (High, 1を出力するとリレーがONする)
    } else if (0 == gpio_get(BLACK_SW_PIN)) {
      // 黒スイッチが押されている場合 (スイッチが押されるとGNDと導通してLow, 0が入力される)
      gpio_put(RELAY_PIN, 0);  // リレーOFF (Low, 0を出力するとリレーがOFFする)
    }
  }
}
