#include <Arduino.h>
#include <Wire.h>
#include <LGFXRP2040.h>
#include <debug.h>
#include <graphics.h>
#include <instructionSet.h>
#include <SPI.h>
#include <SD.h>

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
#define BUTTON_PINS {10,11,12,13,14,15}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

// DISP 関連
LGFXRP2040 display;

// SDカード関連
#define SD_SCK_PIN 2
#define SD_TX_PIN 3
#define SD_RX_PIN 4
#define SD_CS_PIN 5

// ボタン関連
volatile bool buttonPressed = false;
#define BTN_NONE   0x00
#define BTN_UP     0x01
#define BTN_DOWN   0x02
#define BTN_LEFT   0x03
#define BTN_RIGHT  0x04
#define BTN_ENTER  0x05
#define BTN_CANCEL 0x06
uint8_t pressedButton = BTN_NONE;

// ディスプレイ関連
#define DISPST_IDLE    0x00
#define DISPST_TITLE   0x01
#define DISPST_PRESETS 0x02
uint8_t displayStatus = DISPST_IDLE;
uint8_t displayCursor = 0x00;

// シンセ関連
uint8_t synthMode = SYNTH_SINGLE;
uint8_t synthPan = LR_PAN_C;
uint8_t selectedPreset = 0x00;

// その他
void loop1();

// todo
String presets[] = {
    "Basic Sine", "Basic Square", "Basic Saw", "Basic Triangle"
};

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

/**
 * @brief CTRLにデータを送信し、応答を取得します。返却された配列は必ず解放処理をしてください。
 * 
 * @param data 送信するデータ
 * @param size 送信するサイズ
 * @param received データを格納する配列
 * @param requestSize 要求するサイズ
 */
void ctrlTransmission(uint8_t* data, size_t size, uint8_t* received, size_t requestSize) {
    toggleCtrl(true);
    ctrl.beginTransmission(CTRL_I2C_ADDR);
    ctrl.write(data, size);
    ctrl.endTransmission();
    ctrl.requestFrom(CTRL_I2C_ADDR, requestSize);

    int i = 0;
    while (ctrl.available()) {
        received[i] = ctrl.read();
        i++;
        if(i >= requestSize) {
            break;
        }
    }
    toggleCtrl(false);
}

// todo
void refreshUI() {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE);
    uint8_t x = display.textWidth(" ");
    uint8_t y = display.height() / 2 - display.fontHeight() / 2;
    char idstr[5]; sprintf(idstr, "%03d ", selectedPreset+1);
    display.drawString(idstr + presets[selectedPreset], x, y);
}

// todo
void setPreset(uint8_t synth, uint8_t id) {
    uint8_t data[] = {INS_BEGIN, DISP_SET_PRESET, DATA_BEGIN, 0x02, synth, id};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

// todo
void setSynthMode(uint8_t mode) {
    uint8_t data[] = {INS_BEGIN, DISP_SET_SYNTH, DATA_BEGIN, 0x01, mode};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
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
            pressedButton = (i == 0) ? BTN_UP :
                            (i == 2) ? BTN_DOWN :
                            (i == 1) ? BTN_LEFT :
                            (i == 4) ? BTN_ENTER :
                            (i == 3) ? BTN_RIGHT :
                            (i == 5) ? BTN_CANCEL : BTN_NONE;
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

    SPI.setRX(SD_RX_PIN);
    SPI.setCS(SD_CS_PIN);
    SPI.setSCK(SD_SCK_PIN);
    SPI.setTX(SD_TX_PIN);

    debug.init();

    // ローディング処理
    display.init();
    display.fillScreen(TFT_BLACK);
    display.drawString("Loading...", 1, 1);
    delay(1000);

    // ボタン初期化
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    // SDカード確認
    if(!SD.begin(SD_CS_PIN)) {
        display.drawString("Error:1201", 1, 1);
        display.drawString("SD card error.", 1, 11);
        return;
    }
    File myFile = SD.open("example.txt", FILE_WRITE);
    myFile.close();

    // CTRLとの接続を確認します
    uint8_t data[] = {INS_BEGIN, DISP_CONNECT};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    // 応答が返ってくればOK
    if(received[0] == RES_OK){
        display.showImage(Graphics::title);
        displayStatus = DISPST_TITLE;
    }else{
        display.drawString("Error:1101", 1, 1);
        display.drawString("Please check the conn", 1, 11);
        display.drawString("ection.", 1, 21);
        return;
    }

    multicore_launch_core1(loop1);
}

void loop() {} // 使用しない

void loop1() {
    while(1){
        if (buttonPressed) {
            switch (pressedButton) {
                case BTN_UP:
                {
                    // todo
                    // モードによって変化させる
                    if(displayStatus == DISPST_PRESETS && displayCursor == 0x00){
                        uint8_t x = display.textWidth(" ");
                        uint8_t y = display.height() / 2 - display.fontHeight() / 2;
                        char idstr[5]; sprintf(idstr, "%03d", selectedPreset+1);
                        display.fillRect(0, y+1, display.textWidth(" 000"), display.fontHeight()-2, TFT_WHITE);
                        display.setTextColor(TFT_BLACK);
                        display.drawString(idstr, x, y);
                    }
                }
                    break;

                case BTN_DOWN:
                {
                    if(displayStatus == DISPST_PRESETS && displayCursor == 0x00){
                        uint8_t x = display.textWidth(" ");
                        uint8_t y = display.height() / 2 - display.fontHeight() / 2;
                        char idstr[5]; sprintf(idstr, "%03d", selectedPreset+1);
                        display.fillRect(0, y+1, display.textWidth(" 000"), display.fontHeight()-2, TFT_WHITE);
                        display.setTextColor(TFT_BLACK);
                        display.drawString(idstr, x, y);
                    }
                }
                    break;

                case BTN_LEFT:
                    if(displayStatus == DISPST_PRESETS){
                        if(selectedPreset != 0x00){
                            selectedPreset--;
                            setPreset(0xff, selectedPreset);
                        }
                    }
                    break;

                case BTN_RIGHT:
                    if(displayStatus == DISPST_PRESETS){
                        if(selectedPreset != 0x03){
                            selectedPreset++;
                            setPreset(0xff, selectedPreset);
                        }
                    }
                    break;

                case BTN_ENTER:
                    if(displayStatus == DISPST_TITLE) {
                        display.fillScreen(TFT_BLACK);
                        display.setFont(&fonts::Font2);
                        uint8_t x = display.textWidth(" ");
                        uint8_t y = display.height() / 2 - display.fontHeight() / 2;
                        display.drawString("001 " + presets[0], x, y);
                        displayStatus = DISPST_PRESETS;
                    }
                    break;

                case BTN_CANCEL:
                    if(synthMode == SYNTH_MULTI) synthMode = SYNTH_SINGLE;
                    else synthMode++;
                    setSynthMode(synthMode);
                    break;
            }
            buttonPressed = false;
        }
    }
}