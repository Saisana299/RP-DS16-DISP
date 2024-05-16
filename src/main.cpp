#include <Arduino.h>
#include <Wire.h>
#include <debug.h>
#include <graphics.h>
#include <lgfx_rp2040.h>
#include <instruction_set.h>
#include <SPI.h>
#include <SD.h>
#include <file_manager.h>
#include <ctrl_manager.h>
#include <ui_manager.h>
#include <synth_manager.h>

// debug 関連
#define DEBUG_MODE 1 //0 or 1
Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

// 共通変数
LGFXRP2040 display;
static LGFX_Sprite sprite(&display);

// 各種制御クラス
CtrlManager* CtrlManager::instance = nullptr;
CtrlManager ctrl(&display, &sprite);
SynthManager synth(&ctrl);
UIManager ui(
    &display, &sprite, &ctrl, &synth
);
FileManager file(&display);

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
    debug.init();
    ui.init();

    // SDカード確認
    if(!file.checkSD()) return;

    // CTRLとの接続を確認します
    if(!ctrl.checkConnection()) return;
    else ui.goTitle();
}

void loop() {
    while(1) ui.buttonHandler();
}

void loop1() {
    while(1) ui.buttonListener();
}