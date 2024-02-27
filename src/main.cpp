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
#define DISPST_DETAIL  0x03
uint8_t displayStatus = DISPST_IDLE;
uint8_t displayCursor = 0x00;

// シンセ関連
uint8_t synthMode = SYNTH_SINGLE;
uint8_t synthPan = LR_PAN_C;
uint8_t selectedPreset = 0x00;
uint8_t selectedPreset2 = 0x00;

int16_t attack = 1;
int16_t decay = 1000;
int16_t sustain = 1000; // max=1000
int16_t release = 1;

String presets[] = {
    "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
};
String modes[] = {
    "SINGLE MODE", "OCTAVE MODE", "DUAL MODE", "MULTI MODE"
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
 * @brief CTRLにデータを送信し、応答を取得します。
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

/** @brief UIを更新 */
void refreshUI() {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE);

    if(displayStatus == DISPST_PRESETS){
        // プリセット
        uint8_t preset_x = display.textWidth(" ");
        uint8_t preset_y = display.height() / 2 - display.fontHeight() / 2;
        char idstr[5]; sprintf(idstr, "%03d ", selectedPreset+1);
        char idstr2[5]; sprintf(idstr2, "%03d ", selectedPreset2+1);

        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
            display.drawString(idstr + presets[selectedPreset], preset_x, preset_y - 7);
            display.drawString(idstr2 + presets[selectedPreset2], preset_x, preset_y + 7);
        }
        else {
            display.drawString(idstr + presets[selectedPreset], preset_x, preset_y);
        }
        
        // MIDIチャンネル
        if(synthMode == SYNTH_MULTI){
            display.drawString("MIDI=1&2", 2, 2);
        }else{
            display.drawString("MIDI=1", 2, 2);
        }

        // シンセモード
        uint8_t synth_x = display.textWidth(modes[synthMode]);
        display.drawString(modes[synthMode], 128 - 2 - synth_x, 2);

        // 横線
        display.drawLine(0, 12, 127, 12, TFT_WHITE);
        display.drawLine(0, 51, 127, 51, TFT_WHITE);

        // カーソル位置
        if(displayCursor == 0x01) {
            uint8_t x = display.textWidth(" ");
            uint8_t y = display.height() / 2 - display.fontHeight() / 2;
            char idstr[5]; sprintf(idstr, "%03d", selectedPreset+1);

            if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                display.fillRect(0, y-1-7, display.textWidth(" 000"), display.fontHeight()+1, TFT_WHITE);
                display.setTextColor(TFT_BLACK);
                display.drawString(idstr, x, y - 7);
            }
            else {
                display.fillRect(0, y-1, display.textWidth(" 000"), display.fontHeight()+1, TFT_WHITE);
                display.setTextColor(TFT_BLACK);
                display.drawString(idstr, x, y);
            }
        }
        else if(displayCursor == 0x02) {
            uint8_t synth_x = display.textWidth(modes[synthMode]);
            display.fillRect(128 - 4 - synth_x, 1, synth_x+3, display.fontHeight()+1, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString(modes[synthMode], 128 - 2 - synth_x, 2);
        }
        else if(displayCursor == 0x03) {
            //todo
        }
        else if(displayCursor == 0x04) {
            if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                uint8_t x = display.textWidth(" ");
                uint8_t y = display.height() / 2 - display.fontHeight() / 2;
                char idstr2[5]; sprintf(idstr2, "%03d", selectedPreset2+1);
                display.fillRect(0, y-1+7, display.textWidth(" 000"), display.fontHeight()+1, TFT_WHITE);
                display.setTextColor(TFT_BLACK);
                display.drawString(idstr2, x, y + 7);
            }
        }
    }

    //アタックとかリリースとか
    else if(displayStatus == DISPST_DETAIL) {
        // タイトル
        display.drawString("Preset Editor", 2, 2);

        // 横線
        display.drawLine(0, 12, 127, 12, TFT_WHITE);

        // ADSR
        char a_chr[6]; sprintf(a_chr, "%d", attack);
        char d_chr[6]; sprintf(d_chr, "%d", decay);
        char s_chr[6]; sprintf(s_chr, "%d", sustain);
        char r_chr[6]; sprintf(r_chr, "%d", release);
        display.drawString("Attack : " + String(a_chr) + " ms", 2, 16);
        display.drawString("Decay  : " + String(d_chr) + " ms", 2, 26);
        display.drawString("Sustain: " + String(s_chr), 2, 36);
        display.drawString("Release: " + String(r_chr) + " ms", 2, 46);

        // 塗り
        if(displayCursor == 0x01) {
            display.fillRect(1, 15, display.textWidth("Attack"), 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Attack", 2, 16);
        }
        else if(displayCursor == 0x02) {
            display.fillRect(1, 25, display.textWidth("Decay")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Decay", 2, 26);
        }
        else if(displayCursor == 0x03) {
            display.fillRect(1, 35, display.textWidth("Sustain")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Sustain", 2, 36);
        }
        else if(displayCursor == 0x04) {
            display.fillRect(1, 45, display.textWidth("Release")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Release", 2, 46);
        }
        
    }
}

/**
 * @brief シンセのプリセットを設定します
 * 
 * @param synth 設定対象のシンセ
 * @param id プリセット番号
 */
void setPreset(uint8_t synth, uint8_t id) {
    uint8_t data[] = {INS_BEGIN, DISP_SET_SHAPE, DATA_BEGIN, 0x02, synth, id};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

/**
 * @brief シンセのモードを設定します
 * 
 * @param mode シンセモード番号
 */
void setSynthMode(uint8_t mode) {
    uint8_t data[] = {INS_BEGIN, DISP_SET_SYNTH, DATA_BEGIN, 0x01, mode};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

/**
 * @brief シンセのノートをリセットします (MIDI Panic)
 * 
 * @param synth 対象のシンセ
 */
void resetSynth(uint8_t synth) {
    uint8_t data[] = {INS_BEGIN, DISP_RESET_SYNTH, DATA_BEGIN, 0x01, synth};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

// アタックを設定
void setAttack(uint8_t synth, int16_t attack) {
    // attack = 0-32sec + 0-999ms (max32sec)
    uint8_t attack_sec;
    uint8_t attack_ms[4];
    
    attack_sec = attack / 1000;

    // msを分割
    for(uint8_t i = 0; i <= ((attack - (attack_sec*1000)) / 255); i++) {
        if(i == ((attack - (attack_sec*1000)) / 255)) attack_ms[i] = (attack - (attack_sec*1000)) - (i*255);
        else attack_ms[i] = 255;
    }

    uint8_t data[] = {
        INS_BEGIN, DISP_SET_ATTACK, DATA_BEGIN,
        0x06, synth, attack_sec, attack_ms[0], attack_ms[1], attack_ms[2], attack_ms[3]
    };
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

// リリースを設定
void setRelease(uint8_t synth, int16_t release) {
    // release = 0-32sec + 0-999ms (max32sec)
    uint8_t release_sec;
    uint8_t release_ms[4];
    
    release_sec = release / 1000;

    // msを分割
    for(uint8_t i = 0; i <= ((release - (release_sec*1000)) / 255); i++) {
        if(i == ((release - (release_sec*1000)) / 255)) release_ms[i] = (release - (release_sec*1000)) - (i*255);
        else release_ms[i] = 255;
    }

    uint8_t data[] = {
        INS_BEGIN, DISP_SET_RELEASE, DATA_BEGIN, 
        0x06, synth, release_sec, release_ms[0], release_ms[1], release_ms[2], release_ms[3]
    };
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

// ディケイを設定
void setDecay(uint8_t synth, int16_t decay) {
    // decay = 0-32sec + 0-999ms (max32sec)
    uint8_t decay_sec;
    uint8_t decay_ms[4] = {0,0,0,0};

    decay_sec = decay / 1000;

    // msを分割
    for(uint8_t i = 0; i <= ((decay - (decay_sec*1000)) / 255); i++) {
        if(i == ((decay - (decay_sec*1000)) / 255)) decay_ms[i] = (decay - (decay_sec*1000)) - (i*255);
        else decay_ms[i] = 255;
    }

    uint8_t data[] = {
        INS_BEGIN, DISP_SET_DECAY, DATA_BEGIN,
        0x06, synth, decay_sec, decay_ms[0], decay_ms[1], decay_ms[2], decay_ms[3]
    };
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

// サステインを設定
void setSustain(uint8_t synth, int16_t sustain) {
    // sustain = 0-1000
    uint8_t sustains[4] = {0,0,0,0};

    // 4分割
    for(uint8_t i = 0; i <= (sustain / 255); i++) {
        if(i == (sustain / 255)) sustains[i] = sustain - (i*255);
        else sustains[i] = 255;
    }

    uint8_t data[] = {
        INS_BEGIN, DISP_SET_SUSTAIN, DATA_BEGIN, 
        0x05, synth, sustains[0], sustains[1], sustains[2], sustains[3]
    };
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    refreshUI();
}

/** @brief タクトスイッチに関する処理 */
void buttonISR() {
    // チャタリング対策
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 10;

    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTime < debounceDelay) {
        return;
    }

    delay(10);

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
    File myFile = SD.open("settings.json", FILE_WRITE);
    myFile.close();

    // CTRLとの接続を確認します
    uint8_t data[] = {INS_BEGIN, DISP_CONNECT};
    uint8_t received[1];
    ctrlTransmission(data, sizeof(data), received, 1);

    // 応答が返ってくればOK
    if(received[0] == RES_OK){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(10);
        digitalWrite(LED_BUILTIN, LOW);
        display.showImage(Graphics::title);
        displayStatus = DISPST_TITLE;
    }else{
        display.drawString("Error:1101", 1, 1);
        display.drawString("Please check the conn", 1, 11);
        display.drawString("ection.", 1, 21);
        return;
    }
}

void loop() {
    if (buttonPressed) {
        switch (pressedButton) {
            case BTN_UP:
            {
                if(displayStatus == DISPST_PRESETS) {
                    if(displayCursor == 0x00){
                        displayCursor = 0x01;
                        refreshUI();
                    }
                    else if(displayCursor == 0x01){
                        displayCursor = 0x02;
                        refreshUI();
                    }
                    else if(displayCursor == 0x02){
                        displayCursor = 0x03;
                        refreshUI();
                    }
                    else if(displayCursor == 0x03){
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            displayCursor = 0x04;
                        }else{
                            displayCursor = 0x01;
                        }
                        refreshUI();
                    }
                    else if(displayCursor == 0x04){
                        displayCursor = 0x01;
                        refreshUI();
                    }
                }
                else if(displayStatus == DISPST_DETAIL) {
                    if(displayCursor == 0x01) {
                        displayCursor = 0x04;
                    }
                    else{
                        displayCursor -= 0x01;
                    }
                    refreshUI();
                }
            }
                break;

            case BTN_DOWN:
            {
                if(displayStatus == DISPST_PRESETS) {
                    if(displayCursor == 0x00 or displayCursor == 0x02){
                        displayCursor = 0x01;
                        refreshUI();
                    }
                    else if(displayCursor == 0x01){
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            displayCursor = 0x04;
                        }else{
                            displayCursor = 0x03;
                        }
                        refreshUI();
                    }
                    else if(displayCursor == 0x03){
                        displayCursor = 0x02;
                        refreshUI();
                    }
                    else if(displayCursor == 0x04){
                        displayCursor = 0x03;
                        refreshUI();
                    }
                }
                else if(displayStatus == DISPST_DETAIL) {
                    if(displayCursor == 0x04) {
                        displayCursor = 0x01;
                    }
                    else{
                        displayCursor += 0x01;
                    }
                    refreshUI();
                }
            }
                break;

            case BTN_LEFT:
                if(displayStatus == DISPST_PRESETS) {
                    if(displayCursor == 0x01){
                        if(selectedPreset != 0x00) selectedPreset--;
                        else selectedPreset = 0x03;
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            setPreset(0x01, selectedPreset);
                        }else{
                            setPreset(0xff, selectedPreset);
                        }

                    }else if(displayCursor == 0x02){
                        if(synthMode == SYNTH_SINGLE) synthMode = SYNTH_MULTI;
                        else synthMode--;
                        setSynthMode(synthMode);
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            setPreset(0x01, selectedPreset);
                            setPreset(0x02, selectedPreset2);
                        }else{
                            setPreset(0xff, selectedPreset);
                        }

                    }else if(displayCursor == 0x03){
                        //
                    }else if(displayCursor == 0x04){
                        if(selectedPreset2 != 0x00) selectedPreset2--;
                        else selectedPreset2 = 0x03;
                        setPreset(0x02, selectedPreset2);
                    }
                }
                else if(displayStatus == DISPST_DETAIL) {
                    if(displayCursor == 0x01) {
                        if(attack - 50 < 0) break;
                        attack -= 50;
                        setAttack(0xff, attack);
                        refreshUI();
                    }
                    else if(displayCursor == 0x02) {
                        if(decay - 50 < 0) break;
                        decay -= 50;
                        setDecay(0xff, decay);
                        refreshUI();
                    }
                    else if(displayCursor == 0x03) {
                        if(sustain - 100 < 0) break;
                        sustain -= 100;
                        setSustain(0xff, sustain);
                        refreshUI();
                    }
                    else if(displayCursor == 0x04) {
                        if(release - 50 < 0) break;
                        release -= 50;
                        setRelease(0xff, release);
                        refreshUI();
                    }
                }
                break;

            case BTN_RIGHT:
                if(displayStatus == DISPST_PRESETS) {
                    if(displayCursor == 0x01){
                        if(selectedPreset != 0x03) selectedPreset++;
                        else selectedPreset = 0x00;
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            setPreset(0x01, selectedPreset);
                        }else{
                            setPreset(0xff, selectedPreset);
                        }

                    }else if(displayCursor == 0x02){
                        if(synthMode == SYNTH_MULTI) synthMode = SYNTH_SINGLE;
                        else synthMode++;
                        setSynthMode(synthMode);
                        if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL){
                            setPreset(0x01, selectedPreset);
                            setPreset(0x02, selectedPreset2);
                        }else{
                            setPreset(0xff, selectedPreset);
                        }

                    }else if(displayCursor == 0x03){
                        //
                    }else if(displayCursor == 0x04){
                        if(selectedPreset2 != 0x03) selectedPreset2++;
                        else selectedPreset2 = 0x00;
                        setPreset(0x02, selectedPreset2);
                    }
                }
                else if(displayStatus == DISPST_DETAIL) {
                    if(displayCursor == 0x01) {
                        if(attack + 50 > 32000) break;
                        attack += 50;
                        setAttack(0xff, attack);
                        refreshUI();
                    }
                    else if(displayCursor == 0x02) {
                        if(decay + 50 > 32000) break;
                        decay += 50;
                        setDecay(0xff, decay);
                        refreshUI();
                    }
                    else if(displayCursor == 0x03) {
                        if(sustain + 100 > 1000) break;
                        sustain += 100;
                        setSustain(0xff, sustain);
                        refreshUI();
                    }
                    else if(displayCursor == 0x04) {
                        if(release + 50 > 32000) break;
                        release += 50;
                        setRelease(0xff, release);
                        refreshUI();
                    }
                }
                break;

            case BTN_ENTER:
                if(displayStatus == DISPST_TITLE) {
                    displayStatus = DISPST_PRESETS;
                    refreshUI();
                }
                else if(displayStatus == DISPST_PRESETS) {
                    if(displayCursor == 0x02) {
                        resetSynth(0xff);
                    }
                    else if(displayCursor == 0x01) {
                        displayCursor = 0x01;
                        displayStatus = DISPST_DETAIL;
                        refreshUI();
                    }
                    else if(displayCursor == 0x00) {
                        displayCursor = 0x01;
                        refreshUI();
                    }
                }
                else if(displayStatus == DISPST_DETAIL) {
                    //
                }
                break;

            case BTN_CANCEL:
                if(displayStatus == DISPST_PRESETS){
                    displayCursor = 0x00;
                    refreshUI();
                }
                else if(displayStatus == DISPST_DETAIL) {
                    displayCursor = 0x01;
                    displayStatus = DISPST_PRESETS;
                    refreshUI();
                }
                break;
        }
        buttonPressed = false;
    }
}