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

#define BTN_LONG_UP     0x11
#define BTN_LONG_DOWN   0x12
#define BTN_LONG_LEFT   0x13
#define BTN_LONG_RIGHT  0x14
#define BTN_LONG_ENTER  0x15
#define BTN_LONG_CANCEL 0x16
uint8_t pressedButton = BTN_NONE;

// ディスプレイ関連
#define DISPST_IDLE    0x00
#define DISPST_TITLE   0x01
#define DISPST_PRESETS 0x02
#define DISPST_DETAIL  0x03
#define DISPST_DEBUG   0x04
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
        delay(10);
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

// debugモード時に通信を受け取るためのコード
void refreshUI();
void receiveEvent(int bytes) {
    // 2バイト以上のみ受け付ける
    if(bytes < 2) return;

    int i = 0;
    uint8_t receivedData[bytes];
    while (ctrl.available()) {
        uint8_t received = ctrl.read();
        receivedData[i] = received;
        i++;
        if (i >= bytes) {
            break;
        }
    }

    uint8_t instruction = 0x00; // コード種別
    if(receivedData[0] == INS_BEGIN) {
        instruction = receivedData[1];
    }

    if(instruction == DISP_DEBUG_DATA) {
        // 例: {INS_BEGIN, DISP_DEBUG_DATA, DETA_BEGIN, 0x04, 0x01, 0x11, 0xA2, 0x01}
        if(bytes < 6) return;
        uint8_t statusByte = receivedData[4];
        uint8_t dataByte[2] = {0xff, 0xff};
        uint8_t synthByte = receivedData[5];

        if(bytes > 7) {
            dataByte[0] = receivedData[5];
            dataByte[1] = receivedData[6];
            synthByte = receivedData[7];
        }
        else if(bytes > 6) {
            dataByte[0] = receivedData[5];
            synthByte = receivedData[6];
        }

        display.fillRect(2, 16, 10, 10);
        if(synthByte == 0x00) {
            display.drawString("Synth: None", 2, 16);
        }else{
            char sy_chr[4]; sprintf(sy_chr, "0x%02x", synthByte);
            display.drawString("Synth: " + String(sy_chr));
        }

        char sb_chr[4]; sprintf(sb_chr, "0x%02x", statusByte);
        char db1_chr[4];
        if(dataByte[0] == 0xff) db1_chr = "----";
        else sprintf(db1_chr, "0x%02x", dataByte[0]);
        char db2_chr[4];
        if(dataByte[1] == 0xff) db2_chr = "----";
        else sprintf(db2_chr, "0x%02x", dataByte[1]);
        
        display.fillRect(2, 26, 10, 10);
        display.drawString(" " + String(sb_chr) + " " + String(db1_chr) + " " + String(sb2_chr), 2, 26);
    }
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
        char a_sym = ':'; if (displayCursor == 0x05) a_sym = '>';
        char d_sym = ':'; if (displayCursor == 0x06) d_sym = '>';
        char s_sym = ':'; if (displayCursor == 0x07) s_sym = '>';
        char r_sym = ':'; if (displayCursor == 0x08) r_sym = '>';
        char a_chr[6]; sprintf(a_chr, "%d", attack);
        char d_chr[6]; sprintf(d_chr, "%d", decay);
        char s_chr[6]; sprintf(s_chr, "%d", sustain);
        char r_chr[6]; sprintf(r_chr, "%d", release);
        display.drawString("Attack " + String(a_sym) + " " + String(a_chr) + " ms", 2, 16);
        display.drawString("Decay  " + String(d_sym) + " " + String(d_chr) + " ms", 2, 26);
        display.drawString("Sustain" + String(s_sym) + " " + String(s_chr), 2, 36);
        display.drawString("Release" + String(r_sym) + " " + String(r_chr) + " ms", 2, 46);

        // 塗り
        if(displayCursor == 0x01 || displayCursor == 0x05) {
            display.fillRect(1, 15, display.textWidth("Attack"), 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Attack", 2, 16);
        }
        else if(displayCursor == 0x02 || displayCursor == 0x06) {
            display.fillRect(1, 25, display.textWidth("Decay")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Decay", 2, 26);
        }
        else if(displayCursor == 0x03 || displayCursor == 0x07) {
            display.fillRect(1, 35, display.textWidth("Sustain")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Sustain", 2, 36);
        }
        else if(displayCursor == 0x04 || displayCursor == 0x08) {
            display.fillRect(1, 45, display.textWidth("Release")+1, 9, TFT_WHITE);
            display.setTextColor(TFT_BLACK);
            display.drawString("Release", 2, 46);
        }
        
    }

    // デバッグモード
    else if(displayStatus == DISPST_DEBUG) {
        // タイトル
        display.drawString("Debug Mode", 2, 2);

        // 横線
        display.drawLine(0, 12, 127, 12, TFT_WHITE);

        // シンセモード
        uint8_t synth_x = display.textWidth("MIDI-1.0");
        display.drawString("MIDI-1.0", 128 - 2 - synth_x, 2);
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
    uint8_t attack_ms[4] = {0,0,0,0};
    
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
    uint8_t release_ms[4] = {0,0,0,0};
    
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

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CTRL_SW_PIN, OUTPUT);

    digitalWrite(CTRL_SW_PIN, LOW);

    ctrl.setSDA(CTRL_SDA_PIN);
    ctrl.setSCL(CTRL_SCL_PIN);
    ctrl.begin();
    ctrl.setClock(1000000);

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

// 上が押された場合の処理
void handleButtonUp(bool longPush = false) {
    if (longPush) return;
    if (displayStatus == DISPST_PRESETS) {
        switch (displayCursor) {
            case 0x00:
            case 0x01:
            case 0x02:
                displayCursor++;
                break;
            case 0x03:
                displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
                break;
            case 0x04:
                displayCursor = 0x01;
                break;
        }
        refreshUI();
    } else if (displayStatus == DISPST_DETAIL) {
        switch (displayCursor) {
            // 100倍
            case 0x05:
                if (attack + 100 <= 32000) {
                    attack += 100; setAttack(0xff, attack); 
                }
                break;
            case 0x06:
                if (decay + 100 <= 32000) {
                    decay += 100; setDecay(0xff, decay);
                }
                break;
            case 0x07:
                if (sustain + 100 <= 1000) {
                    sustain += 100; setSustain(0xff, sustain); 
                }
                break;
            case 0x08:
                if (release + 100 <= 32000) {
                    release += 100; setRelease(0xff, release);  
                }
                break;
            // 通常
            default:
                displayCursor = (displayCursor == 0x01) ? 0x04 : (displayCursor - 0x01);
                break;
        }
        refreshUI();
    }
}

// 下が押された場合の処理
void handleButtonDown(bool longPush = false) {
    if (longPush) return;
    if (displayStatus == DISPST_PRESETS) {
        switch (displayCursor) {
            case 0x00:
            case 0x02:
                displayCursor = 0x01;
                break;
            case 0x01:
                displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x03;
                break;
            case 0x03:
                displayCursor = 0x02;
                break;
            case 0x04:
                displayCursor = 0x03;
                break;
        }
        refreshUI();
    } else if (displayStatus == DISPST_DETAIL) {
        switch (displayCursor) {
            // 100倍
            case 0x05:
                if (attack - 100 >= 0) {
                    attack -= 100; setAttack(0xff, attack);
                }
                break;
            case 0x06:
                if (decay - 100 >= 0) {
                    decay -= 100; setDecay(0xff, decay);
                }
                break;
            case 0x07:
                if (sustain - 100 >= 0) {
                    sustain -= 100; setSustain(0xff, sustain);
                }
                break;
            case 0x08:
                if (release - 100 >= 0) {
                    release -= 100; setRelease(0xff, release);
                }
                break;
            // 通常
            default:
                displayCursor = (displayCursor == 0x04) ? 0x01 : (displayCursor + 0x01);
                break;
        }
        refreshUI();
    }
}

// 左が押された場合の処理
void handleButtonLeft(bool longPush = false) {
    if (displayStatus == DISPST_PRESETS) {
        switch (displayCursor) {
            case 0x01:
                selectedPreset = (selectedPreset != 0x00) ? (selectedPreset - 1) : 0x03;
                setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                break;
            case 0x02:
                if (longPush) return;
                synthMode = (synthMode == SYNTH_SINGLE) ? SYNTH_MULTI : (synthMode - 1);
                setSynthMode(synthMode);
                setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    setPreset(0x02, selectedPreset2);
                }
                break;
            case 0x04:
                selectedPreset2 = (selectedPreset2 != 0x00) ? (selectedPreset2 - 1) : 0x03;
                setPreset(0x02, selectedPreset2);
                break;
        }
    } else if (displayStatus == DISPST_DETAIL) {
        switch (displayCursor) {
            // 通常
            case 0x01:
                if (attack - 1 >= 0) {
                    attack -= 1; setAttack(0xff, attack);
                }
                break;
            case 0x02:
                if (decay - 1 >= 0) {
                    decay -= 1; setDecay(0xff, decay);
                }
                break;
            case 0x03:
                if (sustain - 1 >= 0) {
                    sustain -= 1; setSustain(0xff, sustain);
                }
                break;
            case 0x04:
                if (release - 1 >= 0) {
                    release -= 1; setRelease(0xff, release);
                }
                break;

            // 10倍
            case 0x05:
                if (attack - 10 >= 0) {
                    attack -= 10; setAttack(0xff, attack);
                }
                break;
            case 0x06:
                if (decay - 10 >= 0) {
                    decay -= 10; setDecay(0xff, decay);
                }
                break;
            case 0x07:
                if (sustain - 10 >= 0) {
                    sustain -= 10; setSustain(0xff, sustain);
                }
                break;
            case 0x08:
                if (release - 10 >= 0) {
                    release -= 10; setRelease(0xff, release);
                }
                break;
        }
        refreshUI();
    }
}

// 右が押された場合の処理
void handleButtonRight(bool longPush = false) {
    if (displayStatus == DISPST_PRESETS) {
        switch (displayCursor) {
            case 0x01:
                selectedPreset = (selectedPreset != 0x03) ? (selectedPreset + 1) : 0x00;
                setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                break;
            case 0x02:
                if (longPush) return;
                synthMode = (synthMode == SYNTH_MULTI) ? SYNTH_SINGLE : (synthMode + 1);
                setSynthMode(synthMode);
                setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    setPreset(0x02, selectedPreset2);
                }
                break;
            case 0x04:
                selectedPreset2 = (selectedPreset2 != 0x03) ? (selectedPreset2 + 1) : 0x00;
                setPreset(0x02, selectedPreset2);
                break;
        }
    } else if (displayStatus == DISPST_DETAIL) {
        switch (displayCursor) {
            // 通常
            case 0x01:
                if (attack + 1 <= 32000) {
                    attack += 1; setAttack(0xff, attack); 
                }
                break;
            case 0x02:
                if (decay + 1 <= 32000) {
                    decay += 1; setDecay(0xff, decay);
                }
                break;
            case 0x03:
                if (sustain + 1 <= 1000) {
                    sustain += 1; setSustain(0xff, sustain); 
                }
                break;
            case 0x04:
                if (release + 1 <= 32000) {
                    release += 1; setRelease(0xff, release);  
                }
                break;

            // 10倍
            case 0x05:
                if (attack + 10 <= 32000) {
                    attack += 10; setAttack(0xff, attack); 
                }
                break;
            case 0x06:
                if (decay + 10 <= 32000) {
                    decay += 10; setDecay(0xff, decay);
                }
                break;
            case 0x07:
                if (sustain + 10 <= 1000) {
                    sustain += 10; setSustain(0xff, sustain); 
                }
                break;
            case 0x08:
                if (release + 10 <= 32000) {
                    release += 10; setRelease(0xff, release);  
                }
                break;
        }
        refreshUI();
    }
}

uint8_t long_count_to_enter_debug_mode = 0;

// エンターが押された場合の処理
void handleButtonEnter(bool longPush = false) {
    if (longPush) return;
    if (displayStatus == DISPST_TITLE) {
        displayStatus = DISPST_PRESETS;
        refreshUI();
    } else if (displayStatus == DISPST_PRESETS) {
        switch (displayCursor) {
            case 0x02:
                resetSynth(0xff);
                break;
            case 0x01:
                displayCursor = 0x01;
                displayStatus = DISPST_DETAIL;
                refreshUI();
                break;
            case 0x00:
                displayCursor = 0x01;
                refreshUI();
                break;
        }
    }
    else if (displayStatus == DISPST_DETAIL) {
        switch (displayCursor) {
            case 0x01: displayCursor = 0x05;
                break;
            case 0x02: displayCursor = 0x06;
                break;
            case 0x03: displayCursor = 0x07;
                break;
            case 0x04: displayCursor = 0x08;
                break;

            case 0x05: displayCursor = 0x01;
                break;
            case 0x06: displayCursor = 0x02;
                break;
            case 0x07: displayCursor = 0x03;
                break;
            case 0x08: displayCursor = 0x04;
                break;
        }
        refreshUI();
    }
}

// キャンセルが押された場合の処理
void handleButtonCancel(bool longPush = false) {
    if (displayStatus == DISPST_TITLE) {
        if(longPush) {
            if(long_count_to_enter_debug_mode > 10) {
                uint8_t data[] = {INS_BEGIN, DISP_DEBUG_ON};
                uint8_t received[1];
                ctrlTransmission(data, sizeof(data), received, 1);

                if(received[0] != RES_OK) return;

                displayStatus = DISPST_DEBUG;
                ctrl.end();
                ctrl.setSDA(CTRL_SDA_PIN);
                ctrl.setSCL(CTRL_SCL_PIN);
                ctrl.begin(CTRL_I2C_ADDR);
                ctrl.setClock(1000000);
                ctrl.onReceive(receiveEvent);
                refreshUI();
                return;
            } else {
                long_count_to_enter_debug_mode++;
            }
        } else {
            long_count_to_enter_debug_mode = 0;
        }
    } else if (displayStatus == DISPST_PRESETS) {
        if (longPush) return;
        displayCursor = 0x00;
        refreshUI();
    } else if (displayStatus == DISPST_DETAIL) {
        if (longPush) return;
        displayCursor = 0x01;
        displayStatus = DISPST_PRESETS;
        refreshUI();
    }
}

void loop() {
    // ボタンが押されているかどうかを確認
    if (buttonPressed) {
        // ボタンに応じた処理を実行
        switch (pressedButton) {
            case BTN_UP: handleButtonUp(); break;
            case BTN_DOWN: handleButtonDown(); break;
            case BTN_LEFT: handleButtonLeft(); break;
            case BTN_RIGHT: handleButtonRight(); break;
            case BTN_ENTER: handleButtonEnter(); break;
            case BTN_CANCEL: handleButtonCancel(); break;

            case BTN_LONG_UP: handleButtonUp(true); break;
            case BTN_LONG_DOWN: handleButtonDown(true); break;
            case BTN_LONG_LEFT: handleButtonLeft(true); break;
            case BTN_LONG_RIGHT: handleButtonRight(true); break;
            case BTN_LONG_ENTER: handleButtonEnter(true); break;
            case BTN_LONG_CANCEL: handleButtonCancel(true); break;     
        }
        // ボタン処理が完了したので、フラグをリセット
        buttonPressed = false;
    }
}

// ボタンの判定とチャタリング対策
#define PUSH_SHORT  200
#define LONG_TOGGLE 55000
#define PUSH_LONG   50000
uint16_t pushCount[] = {0, 0, 0, 0, 0, 0};
bool longPushed[] = {false, false, false, false, false, false};
void loop1() {
    while(1){
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            // ボタンが押されているときの判定
            if (digitalRead(buttonPins[i]) == LOW) {
                if (pushCount[i] <= PUSH_SHORT) pushCount[i]++;
                else {
                    if (longPushed[i] && pushCount[i] >= PUSH_LONG) {
                        pressedButton = 
                            (i == 0) ? BTN_LONG_UP :
                            (i == 2) ? BTN_LONG_DOWN :
                            (i == 1) ? BTN_LONG_LEFT :
                            (i == 4) ? BTN_LONG_ENTER :
                            (i == 3) ? BTN_LONG_RIGHT :
                            (i == 5) ? BTN_LONG_CANCEL : BTN_NONE;
                        buttonPressed = true;
                        pushCount[i] = 0;
                    }
                    else if(pushCount[i] >= LONG_TOGGLE) {
                        longPushed[i] = true;
                        pushCount[i] = 0;
                    }
                    else {
                        pushCount[i]++;
                    }
                }
            }
            // ボタンを離しているときの判定
            else {
                if(pushCount[i] >= PUSH_SHORT) {
                    pressedButton = 
                        (i == 0) ? BTN_UP :
                        (i == 2) ? BTN_DOWN :
                        (i == 1) ? BTN_LEFT :
                        (i == 4) ? BTN_ENTER :
                        (i == 3) ? BTN_RIGHT :
                        (i == 5) ? BTN_CANCEL : BTN_NONE;
                    buttonPressed = true;
                }
                pushCount[i] = 0;
                longPushed[i] = false;
            }
        }
    }
}