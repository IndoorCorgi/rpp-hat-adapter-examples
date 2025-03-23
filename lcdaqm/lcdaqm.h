/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LCDAQM_H
#define LCDAQM_H

#include "pico/stdlib.h"

// -----------------
// Configurations

#define LCDAQM_I2C_INST i2c_default                  // 使用するI2Cインスタンス. 例:i2c0
#define LCDAQM_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN  // I2C SDAピン
#define LCDAQM_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN  // I2C SCLピン
#define LCDAQM_I2C_ADDRESS 0x3E                      // I2Cデバイスアドレス
#define lcdaqm_delay(x) sleep_ms(x)                  // xミリ秒待機

// -----------------

void lcdaqm_init_i2c();
int lcdaqm_write_register(uint8_t reg_addr, uint8_t data);
void lcdaqm_init();
void lcdaqm_print(const char* str);
void lcdaqm_clear();
void lcdaqm_goto_line(uint line);

#endif