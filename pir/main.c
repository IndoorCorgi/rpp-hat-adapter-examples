/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include "pico/stdlib.h"

#define LED_PIN 3  // LEDが接続されているPicoのGPIO番号
#define PIR_PIN 6  // PIRセンサーが接続されているPicoのGPIO番号

int main() {
  gpio_init(LED_PIN);               // LED用のGPIOを初期化
  gpio_set_dir(LED_PIN, GPIO_OUT);  // LED用のGPIOを出力に変更
  gpio_init(PIR_PIN);               // PIR用のGPIOを初期化

  // 無限ループ
  while (1) {
#if PICO_RP2350
    // Errata RP2350-E9対策のためPico2のみコンパイルされる部分
    hw_set_bits(&pads_bank0_hw->io[PIR_PIN], PADS_BANK0_GPIO0_IE_BITS);  // GPIO読み出し前にinput enableを有効化
    busy_wait_at_least_cycles(1);                                        // すぐに読み出すとうまく動かないので少し待つ
#endif

    if (1 == gpio_get(PIR_PIN)) {
      // PIRセンサーが検知している場合
      gpio_put(LED_PIN, 1);  // LED点灯 (High, 1を出力するとLEDに電流が流れて点灯)
    } else {
      // PIRセンサーが検知していない場合
      gpio_put(LED_PIN, 0);  // LED点灯 (High, 1を出力するとLEDに電流が流れて点灯)
    }

#if PICO_RP2350
    // Errata RP2350-E9対策のためPico2のみコンパイルされる部分
    hw_clear_bits(&pads_bank0_hw->io[PIR_PIN], PADS_BANK0_GPIO0_IE_BITS);  // GPIO読み出し後にinput enableを無効化
    busy_wait_at_least_cycles(50);                                         // 効果が出るまで少し待つ
#endif
  }
}
