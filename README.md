# RP-DS16-DISP
[![GitHub license](https://img.shields.io/badge/RP--DS16-Rev.3.0-seagreen)](https://github.com/Saisana299/RP-DS16)　

RP2040を利用したWavetableシンセ「RP-DS16」のUI部  
OLED制御、ボタン制御、TFカード制御、RP-DS16-CTRLとの通信

## 概要
- RP2040
    - オーバークロック - 200MHz
    - CTRLとの通信に I2C1 を使用
- ディスプレイ
    - SH1106 OLED 1.3inch 128x64 I2C White - [AliExpress](https://ja.aliexpress.com/item/1005004131362533.html)
    - 制御ライブラリ - [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
    - RP2040側 I2C0 を使用
- MicroSD (TF)
    - DM3AT-SF-PEJM5 - [LCSC](https://www.lcsc.com/product-detail/SD-Card-Memory-Card-Connector_HRS-Hirose-DM3AT-SF-PEJM5_C114218.html)
    - 制御ライブラリ - [Adafruit SdFat](https://registry.platformio.org/libraries/adafruit/SdFat%20-%20Adafruit%20Fork)
    - RP2040側 SPI0 を使用
- 機能
    - プリセットの管理
    - MIDIファイル再生 - [MD_MIDIFile](https://registry.platformio.org/libraries/majicdesigns/MD_MIDIFile)

## GPIO
| RP2040 | TF Card Reader | Note |
|:---:|:---:|:---------:|
| GP2 | SCK | - |
| GP3 | MOSI | - |
| GP4 | MISO | - |
| GP5 | CS | - |

| RP2040 | UART | Note |
|:---:|:---:|:---------:|
| GP8 | RX | - |
| GP9 | TX | - |

| RP2040 | Button | Note |
|:---:|:---:|:---------:|
| GP10 | - | Button 1 |
| GP11 | - | Button 2 |
| GP12 | - | Button 3 |
| GP13 | - | Button 4 |
| GP14 | - | Button 5 |
| GP15 | - | Button 6 |

| RP2040 | OLED | Note |
|:---:|:---:|:---------:|
| GP16 | SDA | - |
| GP17 | SCL | - |

| RP2040 | CTRL | Note |
|:---:|:---:|:---------:|
| GP18 | SDA | - |
| GP19 | SCL | - |
| GP20 | - | Switch Pin |