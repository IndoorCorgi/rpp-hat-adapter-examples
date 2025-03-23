## 概要

Raspberry Pi PicoからBME280温度、湿度、気圧センサーで測定するサンプルプログラムです。

プログラムを開始すると、3秒おきに測定した温度、湿度、気圧をシリアルモニターに表示します。


## 対応製品

- [RPi TPH Monitor](https://www.indoorcorgielec.com/products/rpi-tph-monitor-rev2/)
- [RPZ-IR-Sensor](https://www.indoorcorgielec.com/products/rpz-ir-sensor/)
- [RPZ-PIRS](https://www.indoorcorgielec.com/products/rpz-pirs/) (別売りの外付けセンサーセットが必要)
- [RPZ-CO2-Sensor](https://www.indoorcorgielec.com/products/rpz-co2-sensor/) (別売りの外付けセンサーセットが必要)


## 使い方

プロジェクトの開き方やボードの切り替えなど、各サンプルプログラム共通の使い方は[こちら](../)を参照してください。


### シリアルモニター

USBとUART0/GP0の両方へ動作ログを出力します。
シリアルモニターでいずれかをモニターすることで、動作ログを確認することができます。
シリアルモニターの使い方については[こちらの記事](https://www.indoorcorgielec.com/resources/pico/serial-monitor/)を参照してください。


### センサーのアドレス設定

製品によっては2つのBME280センサーを使用することができます。
センサーのアドレスを指定することで、どのセンサーを使用するか決定します。

[main.cpp](main.cpp)14行目で、以下の通りセンサーのI2Cデバイスアドレスを設定しています。デフォルトは0x76です。
アドレスを使用する製品、センサーに合わせて変更してください。
~~~
BME280 bme280(0x76);
~~~

| 製品 | センサー | アドレス |
| ---- | ---- | ---- |
| RPi TPH Monitor | P2端子(同梱品) or P3端子 | 0x76 |
| RPi TPH Monitor | P4端子 | 0x77 |
| RPZ-IR-Sensor   | 外付け | 0x76 |
| RPZ-IR-Sensor   | 基板上 | 0x77 |
| RPZ-PIRS        | 外付け | 0x76 |
| RPZ-CO2-Sensor  | 外付け | 0x76 |


### 測定値のモニター

プログラム開始後、3秒おきにシリアルモニターに以下のような測定値が表示されれば成功です！
~~~
----------------
Measurement Data
 Temp : 23.8C
 Pressure : 1002.2hPa
 Humidity : 40.4%
~~~
