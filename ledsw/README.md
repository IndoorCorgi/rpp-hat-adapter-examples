## 概要

Picoから拡張基板のLEDとスイッチを制御するサンプルプログラムです。

スイッチが押されているかどうかを確認し、押されていればLEDを点灯、
押されていなければ消灯、という動作を繰り返します。


## 対応製品

- [RPi TPH Monitor](https://www.indoorcorgielec.com/products/rpi-tph-monitor-rev2/)
- [RPZ-IR-Sensor](https://www.indoorcorgielec.com/products/rpz-ir-sensor/)
- [RPZ-PIRS](https://www.indoorcorgielec.com/products/rpz-pirs/)
- [RPZ-CO2-Sensor](https://www.indoorcorgielec.com/products/rpz-co2-sensor/)


## 使い方

プロジェクトの開き方など、各サンプルプログラム共通の使い方は[こちら](../)を参照してください。


### 点灯するLEDの指定

[main.c](main.c)9行目で、点灯するLEDに対応するPicoのGPIO番号を指定しています。
~~~
#define LED_PIN 3  // LEDが接続されているPicoのGPIO番号
~~~

各製品のLEDとPico GPIO番号の対応は以下のとおりです。使用するLEDのPico GPIO番号に変更してください。
| 製品 | LED | Pico GPIO |
| ---- | ---- | ---- |
| RPZ-IR-Sensor | LED1 緑 | 3  |
| RPZ-IR-Sensor | LED2 黄 | 28 |
| RPZ-IR-Sensor | LED3 青 | 7  |
| RPZ-IR-Sensor | LED4 白 | 6  |
| RPZ-PIRS, RPZ-CO2-Sensor | LED1 赤 | 28  |
| RPZ-PIRS, RPZ-CO2-Sensor | LED1 緑 | 3   |
| RPZ-PIRS, RPZ-CO2-Sensor | LED1 青 | 7  |
| RPi TPH Monitor | D4 黄 | 8  |
| RPi TPH Monitor | D5 緑 | 9  |


### 使用するスイッチの指定

[main.c](main.c)10行目で、使用するスイッチに対応するPicoのGPIO番号を指定しています。
~~~
#define SW_PIN 8   // スイッチが接続されているPicoのGPIO番号
~~~

各製品のスイッチとPico GPIO番号の対応は以下のとおりです。使用するスイッチのPico GPIO番号に変更してください。
| 製品 | スイッチ | Pico GPIO |
| ---- | ---- | ---- |
| RPZ-IR-Sensor, RPZ-PIRS, RPZ-CO2-Sensor | SW1 赤 | 8  |
| RPZ-IR-Sensor, RPZ-PIRS, RPZ-CO2-Sensor | SW2 黒 | 9  |
| RPi TPH Monitor | SW1 | 7  |
| RPi TPH Monitor | SW2 | 27 |
| RPi TPH Monitor | SW3 | 26 |


### 動作確認

プログラムを書き込み、指定したスイッチを押している間LEDが点灯し、スイッチを離したら消灯する
動作をしていれば成功です！
