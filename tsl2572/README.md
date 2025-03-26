## 概要

Raspberry Pi PicoからTSL2572センサーで明るさ(照度)を測定するサンプルプログラムです。

プログラムを開始すると、3秒おきに測定した明るさをシリアルモニターに表示します。


## 対応製品

- [RPZ-IR-Sensor](https://www.indoorcorgielec.com/products/rpz-ir-sensor/)
- [RPZ-PIRS](https://www.indoorcorgielec.com/products/rpz-pirs/) 


## 使い方

プロジェクトの開き方やボードの切り替えなど、各サンプルプログラム共通の使い方は[こちら](../)を参照してください。


### シリアルモニター

USBとUART0/GP0の両方へ動作ログを出力します。
シリアルモニターでいずれかをモニターすることで、動作ログを確認することができます。
シリアルモニターの使い方については[こちらの記事](https://www.indoorcorgielec.com/resources/pico/serial-monitor/)を参照してください。


### 測定値のモニター

プログラム開始後、3秒おきにシリアルモニターに以下のような測定値が表示されれば成功です！
~~~
165.7[lux]
~~~
