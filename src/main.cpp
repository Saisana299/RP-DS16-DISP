#include <Arduino.h>
#include <Wire.h>
#include <LGFXRP2040.h>
#include <debug.h>
#include <graphics.h>
#include <instructionSet.h>

// debug 関連
#define DEBUG_MODE 1 //0 or 1
Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

// CTRL 関連
#define CTRL_SDA_PIN 18
#define CTRL_SCL_PIN 19
#define CTRL_SW_PIN 20
#define CTRL_I2C_ADDR 0x0A
TwoWire& ctrl = Wire1;

// BUTTON 関連
#define BUTTON_COUNT 6
#define BUTTON_PINS {11,12,13,14,15,21}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

// DISP 関連
LGFXRP2040 display;

// その他
void loop1();
volatile bool buttonPressed = false;
#define BTN_NONE   0x00 // NONE
#define BTN_UP     0x01 // UP
#define BTN_DOWN   0x02 // DOWN
#define BTN_LEFT   0x03 // LEFT
#define BTN_RIGHT  0x04 // RIGHT
#define BTN_ENTER  0x05 // ENTER
#define BTN_CANCEL 0x06 // CANCEL
int8_t pressedButton = BTN_NONE;

/**
 * @brief CTRLとの通信を切り替えます
 * 
 * @param begin true:待機時間が追加されます
 */
void toggleCtrl(bool begin) {
    digitalWrite(CTRL_SW_PIN, HIGH);
    digitalWrite(CTRL_SW_PIN, LOW);
    if(begin){
        delay(100);
    }
}

void buttonISR() {
    // チャタリング対策
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 50;

    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTime < debounceDelay) {
        return;
    }

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (digitalRead(buttonPins[i]) == LOW) {
            pressedButton = (i == 5) ? BTN_UP :
                            (i == 1) ? BTN_DOWN :
                            (i == 0) ? BTN_LEFT :
                            (i == 4) ? BTN_ENTER :
                            (i == 2) ? BTN_RIGHT :
                            (i == 3) ? BTN_CANCEL : BTN_NONE;
            buttonPressed = true;
            lastDebounceTime = currentMillis;
            return;
        }
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CTRL_SW_PIN, OUTPUT);
    digitalWrite(CTRL_SW_PIN, LOW);

    ctrl.setSDA(CTRL_SDA_PIN);
    ctrl.setSCL(CTRL_SCL_PIN);
    ctrl.begin();

    debug.init();

    // ローディング処理
    display.init();
    display.fillScreen(TFT_BLACK);
    display.drawString("Loading...", 1, 1);
    delay(1000);

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    // CTRLとの接続を確認します
    toggleCtrl(true);
    ctrl.beginTransmission(CTRL_I2C_ADDR);
    uint8_t data[] = {INS_BEGIN, DISP_CONNECT};
    ctrl.write(data, sizeof(data));
    ctrl.endTransmission();

    uint8_t buffer_size = 1;
    ctrl.requestFrom(CTRL_I2C_ADDR, buffer_size);
    uint8_t received = 0x00;
    if (ctrl.available()) {
        received = ctrl.read();
    }
    toggleCtrl(false);

    // 応答が返ってくればOK
    if(received == RES_OK){
        display.showImage(Graphics::title);
    }else{
        display.drawString("Error:1101", 1, 1);
        display.drawString("Please check the conn", 1, 11);
        display.drawString("ection.", 1, 21);

        // 仮想的に終了
        while(1){
            delay(1000);
        }
    }

    multicore_launch_core1(loop1);
}

void loop() {} // 使用しない

void loop1() {
    while(1){
        if (buttonPressed) {
            switch (pressedButton) {
                case BTN_UP:
                    break;
                case BTN_DOWN:
                    break;
                case BTN_LEFT:
                    break;
                case BTN_RIGHT:
                    break;
                case BTN_ENTER:
                    break;
                case BTN_CANCEL:
                    break;
            }
            buttonPressed = false;
        }
    }
}