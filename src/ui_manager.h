#include <ctrl_manager.h>
#include <synth_manager.h>
#include <file_manager.h>
#include <midi_manager.h>
#include <ArduinoJson.h>
#include <graphics.h>
#include <wokwi.h>
#include <IUIHandler.h>
#include <ui_debug.h>
#include <ui_fileman.h>
#include <ui_menu.h>
#include <ui_amp_adsr.h>
#include <ui_osc_unison.h>
#include <ui_osc_wave.h>
#include <ui_osc.h>
#include <ui_preset_edit.h>
#include <ui_presets.h>
#include <ui_title.h>
#include <ui_midi_player.h>
#include <ui_amp.h>
#include <ui_filter.h>
#include <ui_osc_wave.h>
#include <ui_osc_pitch.h>
#include <ui_effect.h>
#include <ui_effect_delay.h>
#include <ui_amp_glide.h>

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

#if WOKWI_MODE != 1
    #define PUSH_SHORT  200
    #define LONG_TOGGLE 120000
    #define PUSH_LONG   65000
#else
    #define PUSH_SHORT  1
    #define LONG_TOGGLE 5
    #define PUSH_LONG   5
#endif

#define FILTER_NONE 0x00
#define FILTER_LPF  0x01
#define FILTER_HPF  0x02
#define FILTER_LHF  0x03

class UIManager {
private:
    TwoWire& ctrl = Wire1;

    // ボタン関連
    volatile bool buttonPressed = false;
    uint8_t pressedButton = BTN_NONE;
    int buttonPins[BUTTON_COUNT] = BUTTON_PINS;
    uint8_t long_count_to_enter_debug_mode = 0;

    // ディスプレイ関連
    uint8_t displayStatus = DISPST_IDLE;
    uint8_t displayCursor = 0x00;

    // シンセ関連
    uint8_t synthMode = SYNTH_POLY;
    uint8_t selectedPreset = 0x00;
    uint8_t selectedPreset2 = 0x00;

    uint8_t selectedWave = 0x00;
    uint8_t selectedWave2 = 0xff;
    uint8_t selectedWaveSub = 0xff;

    int16_t amp_gain = 1000;
    uint8_t pan = 50;

    int16_t attack = 1;
    int16_t decay = 1000;
    int16_t sustain = 1000; // max=1000
    int16_t release = 10;

    uint8_t osc1_voice = 1;
    uint8_t osc2_voice = 1;
    uint8_t osc1_detune = 20;
    uint8_t osc2_detune = 20;
    uint8_t osc1_spread = 0;
    uint8_t osc2_spread = 0;
    int8_t osc1_oct = 0;
    int8_t osc2_oct = 0;
    int8_t osc1_semi = 0;
    int8_t osc2_semi = 0;
    int8_t osc1_cent = 0;
    int8_t osc2_cent = 0;
    int16_t osc1_level = 1000;
    int16_t osc2_level = 1000;

    int8_t osc_sub_oct = 0;
    int8_t osc_sub_semi = 0;
    int8_t osc_sub_cent = 0;
    int16_t osc_sub_level = 1000;

    uint8_t filter_mode = FILTER_NONE;
    float lpf_freq = 1000.0f;
    float lpf_q = 1.0f/sqrt(2.0f);
    float hpf_freq = 500.0f;
    float hpf_q = 1.0f/sqrt(2.0f);

    int16_t delay_time = 250;
    int16_t delay_level = 300;
    int16_t delay_feedback = 500;
    uint8_t delay_status = 0x00;

    uint8_t mod_status = 0x00;

    bool isGlide = false;
    uint16_t glide_time = 15;

    bool isFirst = false;

    String default_presets[FACTORY_PRESETS] = {
        "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
    };
    String default_wavetables[FACTORY_WAVETABLES] = {
        "sine", "triangle", "saw", "square"
    };
    String modes[4] = {
        "POLY MODE", "MONO MODE", "DUAL MODE", "MULTI MODE"
    };

    // ユーザーファイル
    Preset user_presets[USER_PRESET_LIMIT];
    bool isUserPresetLoaded = false;
    Preset user_wavetables[USER_PRESET_LIMIT];
    bool isUserWaveLoaded = false;
    Preset midi_files[USER_PRESET_LIMIT];
    bool isMidiLoaded = false;
    Preset rlem_files[USER_PRESET_LIMIT];
    bool isRlemLoaded = false;

    // プリセット用バッファ
    int16_t wave_table_buff[2048];

    // UI用保存変数
    int fileman_index = 0;
    String currentDir = "/rp-ds16";
    uint8_t selectedOsc = 1;

    // ファイル管理
    Files files[4];
    FsFile file_buff[4];
    bool isEndOfFile = false;
    bool fileManRefresh = true;

    // ボタンの判定とチャタリング対策
    uint32_t intervalCount = 0;
    uint32_t pushCount[BUTTON_COUNT] = {0, 0, 0, 0, 0, 0};
    bool longPushed[BUTTON_COUNT] = {false, false, false, false, false, false};

    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;

    CtrlManager* pCtrl;
    SynthManager* pSynth;
    FileManager* pFile;
    MidiManager* pMidi;

    IUIHandler* ui_handler[18];

public:
    UIManager(LGFXRP2040* addr1, LGFX_Sprite* addr2, CtrlManager* addr3, SynthManager* addr4, FileManager* addr5, MidiManager* addr6) {
        pDisplay = addr1;
        pSprite = addr2;
        pCtrl = addr3;
        pSynth = addr4;
        pFile = addr5;
        pMidi = addr6;

        ui_handler[DISPST_DEBUG] = new UIDebug(pSprite);

        ui_handler[DISPST_FILEMAN] = new UIFileMan(
            pDisplay, pSprite, pFile, &displayStatus, &displayCursor,
            &fileman_index, &currentDir, files, file_buff,
            &isEndOfFile, &fileManRefresh
        );

        ui_handler[DISPST_MENU] = new UIMenu(pSprite, &displayStatus, &displayCursor);

        ui_handler[DISPST_AMP_ADSR] = new UIAmpAdsr(
            &displayStatus, &displayCursor,
            &attack, &decay, &sustain, &release,
            pSprite, pSynth
        );

        ui_handler[DISPST_OSC_UNISON] = new UIOscUnison(
            pSprite, pSynth, &displayStatus, &displayCursor,
            &osc1_voice, &osc2_voice, &osc1_detune, &osc2_detune,
            &selectedOsc, &selectedWave, &selectedWave2, &selectedWaveSub ,&osc1_spread, &osc2_spread
        );

        ui_handler[DISPST_OSC_PITCH] = new UIOscPitch(
            pSprite, pSynth, &displayStatus, &displayCursor, &selectedOsc,
            &osc1_oct, &osc2_oct, &osc1_semi, &osc2_semi, &osc1_cent, &osc2_cent,
            &osc_sub_oct, &osc_sub_semi, &osc_sub_cent
        );

        ui_handler[DISPST_OSC_WAVE] = new UIOscWave(
            pSprite, pSynth, &displayStatus, &displayCursor,
            &osc1_voice, &osc2_voice,
            &selectedOsc, &selectedWave, &selectedWave2, &selectedWaveSub, default_wavetables, user_wavetables, &isFirst
        );

        ui_handler[DISPST_OSC] = new UIOsc(
            pSprite, pSynth, &displayStatus, &displayCursor, &selectedOsc, &osc1_level, &osc2_level, &osc_sub_level, &isFirst, &mod_status
        );

        ui_handler[DISPST_PRESET_EDIT] = new UIPresetEdit(
            pSprite, &displayStatus, &displayCursor
        );

        ui_handler[DISPST_EFFECT] = new UIEffect(
            pSprite, &displayStatus, &displayCursor
        );

        ui_handler[DISPST_DELAY] = new UIEffectDelay(
            pSprite, pSynth, &displayStatus, &displayCursor,
            &delay_time, &delay_level, &delay_feedback, &delay_status
        );

        ui_handler[DISPST_AMP] = new UIAmp(
            pSprite, pSynth, &displayStatus, &displayCursor, &amp_gain, &pan
        );

        ui_handler[DISPST_AMP_GLIDE] = new UIAmpGlide(
            pSprite, pSynth, &displayStatus, &displayCursor, &synthMode, &isGlide, &glide_time
        );

        ui_handler[DISPST_PRESETS] = new UIPresets(
            pSprite, pSynth, pFile, &displayStatus, &displayCursor,
            &synthMode, &selectedPreset, &selectedPreset2,
            &osc1_voice, &osc2_voice, &selectedWave, &selectedWave2,
            default_presets, modes, user_presets, wave_table_buff, &isGlide
        );

        ui_handler[DISPST_TITLE] = new UITitle(
            pSprite, pCtrl, &displayStatus, &long_count_to_enter_debug_mode
        );

        ui_handler[DISPST_MIDI_PLAYER] = new UIMidiPlayer(
            pSprite, pCtrl, &displayStatus, &displayCursor, pMidi, pFile, midi_files
        );

        ui_handler[DISPST_FILTER] = new UIFilter(
            pSprite, pSynth, &displayStatus, &displayCursor,
            &filter_mode, &lpf_freq, &lpf_q, &hpf_freq, &hpf_q
        );
    }

    void init() {
        // ローディング処理
        pDisplay->init();
        pDisplay->fillScreen(TFT_BLACK);

        //powered by raspberry pi
        pDisplay->showImage(pSprite, POWERED_IMG);
        delay(3000);

        pDisplay->fillScreen(TFT_BLACK);
        pDisplay->drawString("init devices", 1, 1);
        String message = "...";
        uint8_t x = 1+6*12;
        for (int i = 0; i < message.length(); i++) {
            delay(500);
            pDisplay->drawString(message.substring(i, i + 1), x, 1);
            x += 6;
        }
        delay(500);

        // ボタン初期化
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            pinMode(buttonPins[i], INPUT_PULLUP);
        }
    }

    void loadUserFiles(String type) {
        // ユーザーファイルを読み込む
        uint8_t size;
        String msg, dir_path, key, ext;
        Preset* preset;
        if(type == "preset")
        {
            msg = "PRESET LOADING...";
            dir_path = "/rp-ds16/preset";
            key = "preset_name";
            preset = user_presets;
            ext = ".json";
            size = USER_PRESET_LIMIT;
        }
        else if(type == "wavetable")
        {
            msg = "WAVETABLE LOADING...";
            dir_path = "/rp-ds16/wavetable";
            key = "wave_name";
            preset = user_wavetables;
            ext = ".json";
            size = USER_PRESET_LIMIT;
        }
        else if(type == "midi")
        {
            msg = "MIDI LOADING...";
            dir_path = "/rp-ds16/midi";
            key = "";
            preset = midi_files;
            ext = ".mid";
            size = USER_PRESET_LIMIT;
        }
        else {
            return;
        }

        uint8_t index = 0;
        int32_t counter = 0;

        while(index < size) {
            pSprite->createSprite(128, 64);
            pSprite->fillScreen(TFT_BLACK);
            pSprite->setTextColor(TFT_WHITE);
            pSprite->drawString(msg, 2, 2);

            pFile->getFiles(dir_path, file_buff, 4, counter);

            for(uint8_t j = 0; j < 4; j++) {
                bool isNull = false;
                char file_name[50];
                file_buff[j].getName(file_name, sizeof(file_name));
                if(strcmp(file_name, "") == 0) isNull = true;
                else {
                    bool isContinue = false;
                    if(!pFile->hasExtension(file_name, ext)) isContinue = true;
                    else if(strcmp(file_name, "---") == 0) isContinue = true;
                    else if(strlen(file_name) > 30) isContinue = true;
                    else if(file_buff[j].isDirectory()) isContinue = true;
                    
                    if(isContinue) {
                        file_buff[j].close();
                        continue;
                    }
                }

                if(!isNull) {
                    String path = dir_path + "/" + String(file_name);
                    if(key != "") {
                        JsonDocument doc ;
                        pFile->getJson(&doc, path);
                        String name = doc[key];
                        preset[index].name = name;
                        preset[index].path = path;
                    }
                    else {
                        preset[index].name = file_name;
                        preset[index].path = path;
                    }
                } else {
                    preset[index].name = "---";
                    preset[index].path = "";
                }
                file_buff[j].close();
                index++;

                // presetが埋まったら終了
                if(index >= size) break;
            }

            pSprite->drawString(String(index) + "/" + String(size), 2, 12);
            pSprite->drawLine(0, 63, (float(index)/(float)size)*127, 63, TFT_WHITE);
            pSprite->pushSprite(0, 0);
            pSprite->deleteSprite();

            counter += 4;
        }

        delay(1000);
    }

    void goTitle() {
        displayStatus = DISPST_TITLE;
    }
    
    /** @brief UIを更新 */
    void refreshUI() {
        if(displayStatus == DISPST_TITLE) return;
        else if(displayStatus == DISPST_PRESETS) {
            if(!isUserPresetLoaded) {
                loadUserFiles("preset");
                isUserPresetLoaded = true;
            }
        }
        else if(displayStatus == DISPST_OSC_WAVE) {
            if(!isUserWaveLoaded) {
                loadUserFiles("wavetable");
                isUserWaveLoaded = true;
            }
        }
        else if(displayStatus == DISPST_MIDI_PLAYER) {
            if(!isMidiLoaded) {
                loadUserFiles("midi");
                isMidiLoaded = true;
            }
        }

        pSprite->createSprite(128, 64);

        pSprite->fillScreen(TFT_BLACK);
        pSprite->setTextColor(TFT_WHITE);

        // デバッグ用
        // char cursor[5];
        // sprintf(cursor, "0x%02x", displayCursor);
        // pSprite->drawString(cursor, 2, 55);

        ui_handler[displayStatus]->refreshUI();

        // 画面更新
        pSprite->pushSprite(0, 0);
        pSprite->deleteSprite();
    }

    /** @brief 上が押された場合の処理 */
    void handleButtonUp(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonUp(longPush);
        refreshUI();
    }
                       
    /** @brief 下が押された場合の処理 */
    void handleButtonDown(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonDown(longPush);
        refreshUI();
    }

    /** @brief 左が押された場合の処理 */
    void handleButtonLeft(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonLeft(longPush);
        refreshUI();
    }

    /** @brief 右が押された場合の処理 */
    void handleButtonRight(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonRight(longPush);
        refreshUI();
    }

    /** @brief 決定が押された場合の処理 */
    void handleButtonEnter(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonEnter(longPush);
        refreshUI();
    }
                        
    /** @brief キャンセルが押された場合の処理 */
    void handleButtonCancel(bool longPush = false) {
        ui_handler[displayStatus]->handleButtonCancel(longPush);
        if(displayStatus != DISPST_TITLE) refreshUI();
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
                        buttonHandler();
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
                if(longPushed[i] || (pushCount[i] >= PUSH_SHORT && intervalCount >= PUSH_LONG)) {
                    pressedButton = 
                        (i == 0) ? BTN_UP :
                        (i == 2) ? BTN_DOWN :
                        (i == 1) ? BTN_LEFT :
                        (i == 4) ? BTN_ENTER :
                        (i == 3) ? BTN_RIGHT :
                        (i == 5) ? BTN_CANCEL : BTN_NONE;
                    longPushed[i] = false;
                    buttonPressed = true;
                    buttonHandler();
                    intervalCount = 0;
                }
                pushCount[i] = 0;
            }
        }
        if(intervalCount <= PUSH_LONG) intervalCount++;
    }
};

#endif // UIMANAGER_H