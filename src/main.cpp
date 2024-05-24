#include <Arduino.h>
#include <Wire.h>
#include <graphics.h>
#include <lgfx_rp2040.h>
#include <instruction_set.h>
#include <SPI.h>
#include <SD.h>
#include <file_manager.h>
#include <ctrl_manager.h>
#include <ui_manager.h>
#include <synth_manager.h>
#include <wokwi.h>

// 共通変数
LGFXRP2040 display;
static LGFX_Sprite sprite(&display);

// 各種制御クラス
CtrlManager* CtrlManager::instance = nullptr;
CtrlManager ctrl(&display, &sprite);
SynthManager synth(&ctrl);
FileManager file(&display);
UIManager ui(
    &display, &sprite, &ctrl, &synth, &file
);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CTRL_SW_PIN, OUTPUT);

    // CTRLとの接続の調整
    for (int i = 0; i < 5; i++) {
        digitalWrite(CTRL_SW_PIN, HIGH);
        digitalWrite(CTRL_SW_PIN, LOW);
    }

    ctrl.init();
    file.init();

    // DebugPin
    Serial2.setTX(8);
    Serial2.setRX(9);
    Serial2.begin(115200);

    ui.init();

    // SDカード確認
    if(!file.checkSD()) return;

    // CTRLとの接続を確認します
    if(!ctrl.checkConnection()) return;

    // 全てのユーザーファイルをロードする
    ui.loadUserFiles("preset");
    ui.loadUserFiles("wavetable");

    display.showImage(&sprite, TITLE_IMG);
    delay(1);
    sprite.createSprite(2, 2);
    sprite.fillRect(0, 0, 2, 2, TFT_BLACK); //謎の点消し
    sprite.pushSprite(69, 31);
    sprite.deleteSprite();
    ui.goTitle();
}

void loop() {
    while(1) {
        #if WOKWI_MODE == 1
            delay(10);
        #endif
        ui.buttonListener();
    }
}

// #if WOKWI_MODE != 1
// void loop1() {
//     //todo
// }
// #endif