## 概要

Picoから拡張基板のPIR(人感)センサーを読み取るサンプルプログラムです。

PIR(人感)センサーが人や動物を検知していればLEDを点灯、検知していなければ消灯、という動作を繰り返します。


## 対応製品

- [RPZ-PIRS](https://www.indoorcorgielec.com/products/rpz-pirs/)


## 使い方

プロジェクトの開き方など、各サンプルプログラム共通の使い方は[こちら](../)を参照してください。


### 点灯するLEDの指定

[main.c](main.c)9行目で、点灯するLEDに対応するPicoのGPIO番号を指定しています。
~~~
#define LED_PIN 3  // LEDが接続されているPicoのGPIO番号
~~~

LEDとPico GPIO番号の対応は以下のとおりです。使用するLEDのPico GPIO番号に変更してください。
| 製品 | LED | Pico GPIO |
| ---- | ---- | ---- |
| RPZ-PIRS | LED1 赤 | 28  |
| RPZ-PIRS | LED1 緑 | 3   |
| RPZ-PIRS | LED1 青 | 7  |


### 動作確認

プログラムを書き込み、PIR(人感)センサーの上に手をかざすなどしてLEDが点灯すれば成功です！


### Pico2についての補足

[main.c](main.c)内にある"#if PICO_RP2350"と"#endif"で囲まれた箇所は, Pico2などのRP2350コントローラーを使用しているボードでのみコンパイルされます。
RP2350A2の欠陥であるErrata RP2350-E9の対策として必要なコードとなっており、
詳細は[RP2350データシート](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)を参照してください。

RP2040では不要なので削除しても構いません。