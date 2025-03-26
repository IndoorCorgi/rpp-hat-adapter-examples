/*
 * Copyright (c) 2025 Indoor Corgi
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TSL2572_H
#define TSL2572_H

#include "pico/stdlib.h"

// -----------------
// Configurations

#define TSL2572_I2C_INST i2c_default                  // 使用するI2Cインスタンス. 例:i2c0
#define TSL2572_I2C_BAUD 100000                       // I2C周波数[Hz]
#define TSL2572_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN  // I2C SDAピン
#define TSL2572_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN  // I2C SCLピン
#define TSL2572_I2C_ADDRESS 0x39                      // I2Cデバイスアドレス
#define tsl2572_delay(x) sleep_ms(x)                  // xミリ秒待機

// -----------------

// 測定の倍率(ゲイン)
#define TSL2572_AGAIN_016 0  // 0.16倍
#define TSL2572_AGAIN_1 1    // 1倍
#define TSL2572_AGAIN_8 2    // 8倍
#define TSL2572_AGAIN_16 3   // 16倍
#define TSL2572_AGAIN_120 4  // 120倍

extern uint tsl2572_again;         // 測定の倍率(ゲイン). AGAIN_xで指定
extern uint tsl2572_integ_cycles;  // 測定の時間を決めるサイクル数. 1-256の整数.
extern float tsl2572_illuminance;   // 測定した照度(明るさ)の値[lux]

void tsl2572_init_i2c();
bool tsl2572_read_registers(uint8_t reg_addr, uint8_t* data, uint32_t length);
uint8_t tsl2572_read_register(uint8_t reg_addr);
bool tsl2572_write_register(uint8_t reg_addr, uint8_t data);
bool tsl2572_check_id();
void tsl2572_write_enable(bool pon, bool aen, bool wen);
void tsl2572_write_atime(uint integ_cycles);
void tsl2572_write_again(uint again);
uint8_t tsl2572_read_status();
void tsl2572_read_adc();
void tsl2572_single_als_integration();
void tsl2572_calculate_lux();
bool tsl2572_single_auto_measure();

#endif