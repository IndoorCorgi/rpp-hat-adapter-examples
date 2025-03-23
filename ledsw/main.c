/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "pico/stdlib.h"

#define LED_PIN 3  // LEDが接続されているPicoのGPIO番号
#define SW_PIN 8   // スイッチが接続されているPicoのGPIO番号

int main() {
  gpio_init(LED_PIN);               // LED用のGPIOを初期化
  gpio_set_dir(LED_PIN, GPIO_OUT);  // LED用のGPIOを出力に変更

  gpio_init(SW_PIN);     // スイッチ用のGPIOを初期化
  gpio_pull_up(SW_PIN);  // スイッチ用のGPIOをプルアップ
  sleep_ms(5);           // プルアップによるGPIOの電圧が安定するまで待機

  // 無限ループ
  while (1) {
    if (0 == gpio_get(SW_PIN)) {
      // スイッチが押されている場合 (スイッチが押されるとGNDと導通してLow, 0が入力される)
      gpio_put(LED_PIN, 1);  // LED点灯 (High, 1を出力するとLEDに電流が流れて点灯)
    } else {
      // スイッチが押されていない場合 (スイッチが押されていないとプルアップによりHigh, 1が入力される)
      gpio_put(LED_PIN, 0);  // LED点灯 (High, 1を出力するとLEDに電流が流れて点灯)
    }
  }
}
