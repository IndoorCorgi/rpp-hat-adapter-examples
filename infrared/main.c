/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "infrared.h"
#include "pico/stdlib.h"

#define BUFFER_LENGTH 2000

uint32_t data[BUFFER_LENGTH];

int main() {
  stdio_init_all();
  infrared_send_init();     // 赤外線送信機能を初期化. PIOを使用
  infrared_receive_init();  // 赤外線受信機能を初期化. PIOを使用

  while (1) {
    printf("Waiting for IR receive...\n");
    int rec_length = infrared_receive_blocking(data, BUFFER_LENGTH);  // 受信処理. ブロッキング.
    printf("Received data length: %d\n", rec_length);                 // 受信データの要素数
    for (int i = 0; i < rec_length; i++) {
      printf("%u, ", data[i]);  // 受信データを表示
    }
    printf("\n");
    sleep_ms(3000);  // 3秒待機

    printf("Sending IR...\n");
    infrared_send(data, rec_length, true);  // 受信したデータと同じものを送信

    sleep_ms(1000);
  }
}
