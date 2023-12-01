#include <Arduino.h>
#include <Wire.h>
#include <LGFXRP2040.h>
#include <debug.h>
#include <graphics.h>

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
int8_t pressedButton = 0x00; // 0x00:none, 0x01:UP, 0x02:DOWN, 0x03:LEFT, 0x04:RIGHT, 0x05:ENTER, 0x06:CANCEL

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
            pressedButton = (i == 5) ? 0x01 :
                            (i == 1) ? 0x02 :
                            (i == 0) ? 0x03 :
                            (i == 4) ? 0x05 :
                            (i == 2) ? 0x04 :
                            (i == 3) ? 0x06 : 0x00;
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
    ctrl.write("connect");
    ctrl.endTransmission();

    uint8_t buffer_size = 10;
    char receivedString[buffer_size];
    ctrl.requestFrom(CTRL_I2C_ADDR, buffer_size);
    uint8_t bi = 0;
    while (ctrl.available()) {
        char receivedChar = ctrl.read();
        receivedString[bi] = receivedChar;
        bi++;
        if (bi >= buffer_size) {
            break; 
        }
    }
    toggleCtrl(false);

    // 応答が返ってくればOK
    if(strncmp(receivedString, "connect:ok", buffer_size) == 0){
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
                case 0x01:
                    break;
                case 0x02:
                    break;
                case 0x03:
                    break;
                case 0x04:
                    break;
                case 0x05:
                    break;
                case 0x06:
                    break;
            }
            buttonPressed = false;
        }
    }
}