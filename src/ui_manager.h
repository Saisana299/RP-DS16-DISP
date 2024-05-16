#include <ctrl_manager.h>
#include <synth_manager.h>

#ifndef UIMANAGER_H
#define UIMANAGER_H

#define BUTTON_COUNT 6
#define BUTTON_PINS {10,11,12,13,14,15}

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

#define DISPST_IDLE    0x00
#define DISPST_TITLE   0x01
#define DISPST_PRESETS 0x02
#define DISPST_DETAIL  0x03
#define DISPST_DEBUG   0x04
#define DISPST_MENU    0x05

#define PUSH_SHORT  200
#define LONG_TOGGLE 55000
#define PUSH_LONG   50000

class UIManager {
private:
    TwoWire& ctrl = Wire1;

    // ボタン関連
    volatile bool buttonPressed = false;
    uint8_t pressedButton = BTN_NONE;
    int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

    // ディスプレイ関連
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

    String presets[4] = {
        "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
    };
    String modes[4] = {
        "SINGLE MODE", "OCTAVE MODE", "DUAL MODE", "MULTI MODE"
    };

    // ボタンの判定とチャタリング対策
    uint16_t intervalCount = 0;
    uint16_t pushCount[BUTTON_COUNT] = {0, 0, 0, 0, 0, 0};
    bool longPushed[BUTTON_COUNT] = {false, false, false, false, false, false};

    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;

    CtrlManager* pCtrl;
    SynthManager* pSynth;

public:
    UIManager(LGFXRP2040* addr1, LGFX_Sprite* addr2, CtrlManager* addr3, SynthManager* addr4) {
        pDisplay = addr1;
        pSprite = addr2;
        pCtrl = addr3;
        pSynth = addr4;
    }

    void init() {
        // ローディング処理
        pDisplay->init();
        pDisplay->fillScreen(TFT_BLACK);
        pDisplay->drawString("Loading...", 1, 1);
        delay(1000);

        // ボタン初期化
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            pinMode(buttonPins[i], INPUT_PULLUP);
        }
    }

    void goTitle() {
        pDisplay->showImage(TITLE_IMG);
        delay(1);
        pDisplay->fillRect(69, 31, 2, 2, TFT_BLACK); //謎の点消し
        displayStatus = DISPST_TITLE;
    }

    /** @brief UIを更新 */
    void refreshUI() {
        pSprite->createSprite(128, 64);

        pSprite->fillScreen(TFT_BLACK);
        pSprite->setTextColor(TFT_WHITE);

        if(displayStatus == DISPST_PRESETS){
            // プリセット ###################################################
            uint8_t preset_x = pSprite->textWidth(" ");
            uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[5]; sprintf(idstr, "%03d ", selectedPreset+1);
            char idstr2[5]; sprintf(idstr2, "%03d ", selectedPreset2+1);

            if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                pSprite->drawString(idstr + presets[selectedPreset], preset_x, preset_y - 7);
                pSprite->drawString(idstr2 + presets[selectedPreset2], preset_x, preset_y + 7);
            }
            else {
                pSprite->drawString(idstr + presets[selectedPreset], preset_x, preset_y);
            }
            
            // MIDIチャンネル
            if(synthMode == SYNTH_MULTI){
                pSprite->drawString("MIDI=1&2", 2, 2);
            }else{
                pSprite->drawString("MIDI=1", 2, 2);
            }

            // シンセモード
            uint8_t synth_x = pSprite->textWidth(modes[synthMode]);
            pSprite->drawString(modes[synthMode], 128 - 2 - synth_x, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
            pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

            // メニュー
            uint8_t menu_x = pSprite->textWidth("Menu->");
            pSprite->drawString("Menu->", 128 - 2 - menu_x, 55);

            // カーソル位置
            if(displayCursor == 0x01) {
                uint8_t x = pSprite->textWidth(" ");
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr[5]; sprintf(idstr, "%03d", selectedPreset+1);

                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    pSprite->fillRect(0, y-1-7, pSprite->textWidth(" 000"), pSprite->fontHeight()+1, TFT_WHITE);
                    pSprite->setTextColor(TFT_BLACK);
                    pSprite->drawString(idstr, x, y - 7);
                }
                else {
                    pSprite->fillRect(0, y-1, pSprite->textWidth(" 000"), pSprite->fontHeight()+1, TFT_WHITE);
                    pSprite->setTextColor(TFT_BLACK);
                    pSprite->drawString(idstr, x, y);
                }
            }
            else if(displayCursor == 0x02) {
                uint8_t synth_x = pSprite->textWidth(modes[synthMode]);
                pSprite->fillRect(128 - 4 - synth_x, 1, synth_x+3, pSprite->fontHeight()+1, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString(modes[synthMode], 128 - 2 - synth_x, 2);
            }
            else if(displayCursor == 0x03) {
                //todo
            }
            else if(displayCursor == 0x04) {
                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    uint8_t x = pSprite->textWidth(" ");
                    uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                    char idstr2[5]; sprintf(idstr2, "%03d", selectedPreset2+1);
                    pSprite->fillRect(0, y-1+7, pSprite->textWidth(" 000"), pSprite->fontHeight()+1, TFT_WHITE);
                    pSprite->setTextColor(TFT_BLACK);
                    pSprite->drawString(idstr2, x, y + 7);
                }
            }
            else if(displayCursor == 0x05) {
                uint8_t menu_x = pSprite->textWidth("Menu->");
                pSprite->fillRect(128 - 4 - menu_x, 54, menu_x+3, pSprite->fontHeight()+54, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString("Menu->", 128 - 2 - menu_x, 55);
            }
        }

        //アタックとかリリースとか ###################################################
        else if(displayStatus == DISPST_DETAIL) {
            // タイトル
            pSprite->drawString("Preset Editor", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // ADSR
            char a_sym = ':'; if (displayCursor == 0x05) a_sym = '>';
            char d_sym = ':'; if (displayCursor == 0x06) d_sym = '>';
            char s_sym = ':'; if (displayCursor == 0x07) s_sym = '>';
            char r_sym = ':'; if (displayCursor == 0x08) r_sym = '>';
            char a_chr[6]; sprintf(a_chr, "%d", attack);
            char d_chr[6]; sprintf(d_chr, "%d", decay);
            char s_chr[6]; sprintf(s_chr, "%d", sustain);
            char r_chr[6]; sprintf(r_chr, "%d", release);
            pSprite->drawString("Attack " + String(a_sym) + " " + String(a_chr) + " ms", 2, 16);
            pSprite->drawString("Decay  " + String(d_sym) + " " + String(d_chr) + " ms", 2, 26);
            pSprite->drawString("Sustain" + String(s_sym) + " " + String(s_chr), 2, 36);
            pSprite->drawString("Release" + String(r_sym) + " " + String(r_chr) + " ms", 2, 46);

            // 塗り
            if(displayCursor == 0x01 || displayCursor == 0x05) {
                pSprite->fillRect(1, 15, pSprite->textWidth("Attack"), 9, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString("Attack", 2, 16);
            }
            else if(displayCursor == 0x02 || displayCursor == 0x06) {
                pSprite->fillRect(1, 25, pSprite->textWidth("Decay")+1, 9, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString("Decay", 2, 26);
            }
            else if(displayCursor == 0x03 || displayCursor == 0x07) {
                pSprite->fillRect(1, 35, pSprite->textWidth("Sustain")+1, 9, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString("Sustain", 2, 36);
            }
            else if(displayCursor == 0x04 || displayCursor == 0x08) {
                pSprite->fillRect(1, 45, pSprite->textWidth("Release")+1, 9, TFT_WHITE);
                pSprite->setTextColor(TFT_BLACK);
                pSprite->drawString("Release", 2, 46);
            }
            
        }

        // デバッグモード ###################################################
        else if(displayStatus == DISPST_DEBUG) {
            // タイトル
            pSprite->drawString("Debug Mode", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // シンセモード
            uint8_t synth_x = pSprite->textWidth("MIDI-1.0");
            pSprite->drawString("MIDI-1.0", 128 - 2 - synth_x, 2);

            // データ表示部
            pSprite->drawString("----", 2, 16);
            pSprite->drawString(" ---- ---- ----", 2, 26);
            pSprite->drawString("(Waiting data input)", 2, 36);
        }

        // メニュー ###################################################
        else if(displayStatus == DISPST_MENU) {
            // タイトル
            pSprite->drawString("Menu", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            pSprite->drawString("Global Settings", 2, 16);
            pSprite->drawString("File Manager", 2, 26);
            pSprite->drawString("MIDI Player", 2, 36);
        }

        // 画面更新
        pSprite->pushSprite(0, 0);
        pSprite->deleteSprite();

        // デバッグ用
        char cursor[5];
        sprintf(cursor, "0x%02x", displayCursor);
        pDisplay->drawString(cursor, 2, 55);
    }

    // 上が押された場合の処理
    void handleButtonUp(bool longPush = false) {
        if (longPush) return;

        // プリセット ###################################################
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x00:
                case 0x01:
                case 0x02:
                    displayCursor++;
                    break;
                case 0x03:
                    displayCursor = 0x05;
                    break;
                case 0x04:
                    displayCursor = 0x01;
                    break;
                case 0x05:
                    displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
                    break;
            }
            refreshUI();

        // ADSRとか ###################################################
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 100倍
                case 0x05:
                    if (attack + 100 <= 32000) {
                        attack += 100; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay + 100 <= 32000) {
                        decay += 100; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain + 100 <= 1000) {
                        sustain += 100; pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x08:
                    if (release + 100 <= 32000) {
                        release += 100; pSynth->setRelease(0xff, release);  
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

        // プリセット ###################################################
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x00:
                case 0x02:
                    displayCursor = 0x01;
                    break;
                case 0x01:
                    displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x05;
                    break;
                case 0x03:
                    displayCursor = 0x02;
                    break;
                case 0x04:
                    displayCursor = 0x05;
                    break;
                case 0x05:
                    displayCursor = 0x03;
                    break;
            }
            refreshUI();

        // ADSRとか ###################################################
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 100倍
                case 0x05:
                    if (attack - 100 >= 0) {
                        attack -= 100; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay - 100 >= 0) {
                        decay -= 100; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain - 100 >= 0) {
                        sustain -= 100; pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x08:
                    if (release - 100 >= 0) {
                        release -= 100; pSynth->setRelease(0xff, release);
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

        // プリセット ###################################################
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0x00) ? (selectedPreset - 1) : 0x03;
                    pSynth->setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_SINGLE) ? SYNTH_MULTI : (synthMode - 1);
                    pSynth->setSynthMode(synthMode);
                    pSynth->setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        pSynth->setPreset(0x02, selectedPreset2);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0x00) ? (selectedPreset2 - 1) : 0x03;
                    pSynth->setPreset(0x02, selectedPreset2);
                    refreshUI(); 
                    break;
            }

        // ADSRとか ###################################################
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack - 1 >= 0) {
                        attack -= 1; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x02:
                    if (decay - 1 >= 0) {
                        decay -= 1; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain - 1 >= 0) {
                        sustain -= 1; pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x04:
                    if (release - 1 >= 0) {
                        release -= 1; pSynth->setRelease(0xff, release);
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack - 10 >= 0) {
                        attack -= 10; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay - 10 >= 0) {
                        decay -= 10; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain - 10 >= 0) {
                        sustain -= 10; pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x08:
                    if (release - 10 >= 0) {
                        release -= 10; pSynth->setRelease(0xff, release);
                    }
                    break;
            }
            refreshUI();
        }
    }

    // 右が押された場合の処理
    void handleButtonRight(bool longPush = false) {

        // プリセット ###################################################
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0x03) ? (selectedPreset + 1) : 0x00;
                    pSynth->setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_MULTI) ? SYNTH_SINGLE : (synthMode + 1);
                    pSynth->setSynthMode(synthMode);
                    pSynth->setPreset((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, selectedPreset);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        pSynth->setPreset(0x02, selectedPreset2);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0x03) ? (selectedPreset2 + 1) : 0x00;
                    pSynth->setPreset(0x02, selectedPreset2);
                    refreshUI(); 
                    break;
            }

        // ADSRとか ###################################################
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack + 1 <= 32000) {
                        attack += 1; pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x02:
                    if (decay + 1 <= 32000) {
                        decay += 1; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain + 1 <= 1000) {
                        sustain += 1; pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x04:
                    if (release + 1 <= 32000) {
                        release += 1; pSynth->setRelease(0xff, release);  
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack + 10 <= 32000) {
                        attack += 10; pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x06:
                    if (decay + 10 <= 32000) {
                        decay += 10; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain + 10 <= 1000) {
                        sustain += 10; pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x08:
                    if (release + 10 <= 32000) {
                        release += 10; pSynth->setRelease(0xff, release);  
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

        // タイトル ###################################################
        if (displayStatus == DISPST_TITLE) {
            displayStatus = DISPST_PRESETS;
            refreshUI();

        // プリセット ###################################################
        } else if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x02:
                    pSynth->resetSynth(0xff);
                    refreshUI(); 
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
                case 0x05:
                    displayCursor = 0x01;
                    displayStatus = DISPST_MENU;
                    refreshUI();
                    break;
            }
        }

        // ADSRとか ###################################################
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

        // タイトル ###################################################
        if (displayStatus == DISPST_TITLE) {
            if(longPush) {
                if(long_count_to_enter_debug_mode > 10) {
                    uint8_t data[] = {INS_BEGIN, DISP_DEBUG_ON};
                    uint8_t received[1];
                    pCtrl->ctrlTransmission(data, sizeof(data), received, 1);

                    if(received[0] != RES_OK) return;

                    displayStatus = DISPST_DEBUG;
                    ctrl.end();
                    ctrl.setSDA(CTRL_SDA_PIN);
                    ctrl.setSCL(CTRL_SCL_PIN);
                    ctrl.begin(CTRL_I2C_ADDR);
                    ctrl.setClock(1000000);
                    ctrl.onReceive(pCtrl->receiveWrapper);
                    refreshUI();
                    return;
                } else {
                    long_count_to_enter_debug_mode++;
                }
            } else {
                long_count_to_enter_debug_mode = 0;
            }

        // プリセット ###################################################
        } else if (displayStatus == DISPST_PRESETS) {
            if (longPush) return;
            displayCursor = 0x00;
            refreshUI();

        // ADSRとか ###################################################
        } else if (displayStatus == DISPST_DETAIL) {
            if (longPush) return;
            displayCursor = 0x01;
            displayStatus = DISPST_PRESETS;
            refreshUI();

        // メニュー ###################################################
        } else if (displayStatus == DISPST_MENU) {
            if (longPush) return;
            displayCursor = 0x00;
            displayStatus = DISPST_PRESETS;
            refreshUI();
        }
    }

    void buttonHandler() {
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

    void buttonListener() {
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
                if(pushCount[i] >= PUSH_SHORT && intervalCount >= PUSH_LONG) {
                    pressedButton = 
                        (i == 0) ? BTN_UP :
                        (i == 2) ? BTN_DOWN :
                        (i == 1) ? BTN_LEFT :
                        (i == 4) ? BTN_ENTER :
                        (i == 3) ? BTN_RIGHT :
                        (i == 5) ? BTN_CANCEL : BTN_NONE;
                    buttonPressed = true;
                    intervalCount = 0;
                }
                pushCount[i] = 0;
                longPushed[i] = false;
            }
        }
        if(intervalCount <= PUSH_LONG) intervalCount++;
    }
};

#endif // UIMANAGER_H