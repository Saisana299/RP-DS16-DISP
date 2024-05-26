#include <ctrl_manager.h>
#include <synth_manager.h>
#include <file_manager.h>
#include <ArduinoJson.h>
#include <graphics.h>
#include <wokwi.h>

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
#define DISPST_FILEMAN 0x06

#if WOKWI_MODE != 1
    #define PUSH_SHORT  200
    #define LONG_TOGGLE 120000
    #define PUSH_LONG   80000
#else
    #define PUSH_SHORT  1
    #define LONG_TOGGLE 5
    #define PUSH_LONG   5
#endif

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
    uint8_t synthMode = SYNTH_SINGLE;
    uint8_t synthPan = LR_PAN_C;
    uint8_t selectedPreset = 0x00;
    uint8_t selectedPreset2 = 0x00;

    int16_t attack = 1;
    int16_t decay = 1000;
    int16_t sustain = 1000; // max=1000
    int16_t release = 1;

    String default_presets[4] = {
        "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
    };
    String default_wavetables[4] = {
        "sine", "triangle", "saw", "square"
    };
    String modes[4] = {
        "SINGLE MODE", "OCTAVE MODE", "DUAL MODE", "MULTI MODE"
    };

    // ユーザープリセット
    struct Preset {
        String name; // 最大30文字
        String path;
    };
    Preset user_presets[252];
    Preset user_wavetables[252];

    // プリセット用バッファ
    int16_t wave_table_buff[2048];

    // UI用保存変数
    int fileman_index = 0;
    String currentDir = "/rp-ds16";
    struct Files {
        bool type;
        String name;
        String path;
    };
    Files files[4];
    File file_buff[4];
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

public:
    UIManager(LGFXRP2040* addr1, LGFX_Sprite* addr2, CtrlManager* addr3, SynthManager* addr4, FileManager* addr5) {
        pDisplay = addr1;
        pSprite = addr2;
        pCtrl = addr3;
        pSynth = addr4;
        pFile = addr5;
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
        String msg, dir_path, key;
        Preset* preset;
        if(type == "preset")
        {
            msg = "PRESET LOADING...";
            dir_path = "/rp-ds16/preset";
            key = "preset_name";
            preset = user_presets;
        }
        else if(type == "wavetable")
        {
            msg = "WAVETABLE LOADING...";
            dir_path = "/rp-ds16/wavetable";
            key = "wave_name";
            preset = user_wavetables;
        }
        else {
            return;
        }

        uint8_t index = 0;
        for(uint8_t i = 0; i < 252; i+=4) {

            pSprite->createSprite(128, 64);
            pSprite->fillScreen(TFT_BLACK);
            pSprite->setTextColor(TFT_WHITE);
            pSprite->drawString(msg, 2, 2);

            pFile->getFiles(dir_path, file_buff, 4, i);
            for(uint8_t j = 0; j < 4; j++) {
                bool isNull = false;
                if(file_buff[j].name() == nullptr) isNull = true;
                else {
                    if(!pFile->hasExtension(file_buff[j].name(), ".json")) continue;
                    if(file_buff[j].name() == "---") continue;
                    if(strlen(file_buff[j].name()) > 30) continue;
                    if(file_buff[j].isDirectory()) continue;
                }

                if(!isNull) {
                    String path = dir_path + "/" + String(file_buff[j].name());
                    JsonDocument doc ;
                    pFile->getJson(&doc, path);
                    String name = doc[key];
                    preset[index].name = name;
                    preset[index].path = path;
                } else {
                    preset[index].name = "---";
                    preset[index].path = "";
                }
                file_buff[j].close();
                index++;
            }

            pSprite->drawString(String(i + 8) + "/256", 2, 12);
            pSprite->drawLine(0, 63, (float(i+8)/256.0f)*127, 63, TFT_WHITE);
            pSprite->pushSprite(0, 0);
            pSprite->deleteSprite();
        }

        // 最後に埋める continueでskipされた分
        while(index < 252) {
            preset[index].name = "---";
            preset[index].path = "";
            index++;
        }

        delay(1000);
    }

    void goTitle() {
        displayStatus = DISPST_TITLE;
    }

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    void setPreset(uint8_t id, uint8_t synth) {
        if(id < 0x04)
            // defaultプリセットはosc=0x01固定
            pSynth->setShape(synth, 0x01, id);
        else {
            JsonDocument doc;
            pFile->getJson(&doc, user_presets[id - 4].path);
            String osc1_type = doc["osc1"]["type"];
            String osc2_type = doc["osc2"]["type"];
            
            // osc1
            if(osc1_type == "custom") {
                String wave = doc["osc1"]["wave"];
                pFile->getJson(&doc, "/rp-ds16/wavetable/" + wave);
                copyArray(doc["wave_table"], wave_table_buff);
                pSynth->setShape(synth, 0x01, id, wave_table_buff);

            } else if(osc1_type == "default") {
                uint8_t osc1_id = doc["osc1"]["id"];
                pSynth->setShape(synth, 0x01, osc1_id);
            }

            // osc2
            if(osc2_type == "custom") {
                String wave = doc["osc2"]["wave"];
                pFile->getJson(&doc, "/rp-ds16/wavetable/" + wave);
                copyArray(doc["wave_table"], wave_table_buff);
                pSynth->setShape(synth, 0x02, id, wave_table_buff);

            } else if(osc2_type == "default") {
                uint8_t osc2_id = doc["osc2"]["id"];
                pSynth->setShape(synth, 0x02, osc2_id);
            }

            // ADSR

            // todo
        }
    }
    
    //            __               _     
    //  _ __ ___ / _|_ __ ___  ___| |__  
    // | '__/ _ \ |_| '__/ _ \/ __| '_ \ 
    // | | |  __/  _| | |  __/\__ \ | | |
    // |_|  \___|_| |_|  \___||___/_| |_|
    //
    /** @brief UIを更新 */
    void refreshUI() {
        pSprite->createSprite(128, 64);

        pSprite->fillScreen(TFT_BLACK);
        pSprite->setTextColor(TFT_WHITE);

        // デバッグ用
        // char cursor[5];
        // sprintf(cursor, "0x%02x", displayCursor);
        // pSprite->drawString(cursor, 2, 55);

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if(displayStatus == DISPST_PRESETS){
            uint8_t preset_x = pSprite->textWidth(" ");
            uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[5]; sprintf(idstr, "%03d ", selectedPreset+1);
            char idstr2[5]; sprintf(idstr2, "%03d ", selectedPreset2+1);

            String preset_name1, preset_name2;
            if(selectedPreset < 0x04) preset_name1 = default_presets[selectedPreset];
            else preset_name1 = user_presets[selectedPreset - 4].name;
            if(selectedPreset2 < 0x04) preset_name2 = default_presets[selectedPreset2];
            else preset_name2 = user_presets[selectedPreset2 - 4].name;

            if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                pSprite->drawString(idstr + preset_name1, preset_x, preset_y - 7);
                pSprite->drawString(idstr2 + preset_name2, preset_x, preset_y + 7);
            }
            else {
                pSprite->drawString(idstr + preset_name1, preset_x, preset_y);
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
            uint8_t menu_x = pSprite->textWidth("Menu>>");
            pSprite->drawString("Menu>>", 128 - 2 - menu_x, 55);

            // 塗り
            if(displayCursor == 0x01) {

                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr[6]; sprintf(idstr, " %03d", selectedPreset+1);

                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) 
                    cursorText(idstr, 0, y - 7);
                
                else 
                    cursorText(idstr, 0, y);
                
            }
            else if(displayCursor == 0x02) {
                uint8_t synth_x = pSprite->textWidth(modes[synthMode]);
                cursorText(modes[synthMode], 128 - 2 - synth_x, 2);
            }
            else if(displayCursor == 0x03) {
                // blank
            }
            else if(displayCursor == 0x04) {
                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                    char idstr2[5]; sprintf(idstr2, " %03d", selectedPreset2+1);
                    cursorText(idstr2, 0, y + 7);
                }
            }
            else if(displayCursor == 0x05) {
                uint8_t menu_x = pSprite->textWidth("Menu>>");
                cursorText("Menu>>", 128 - 2 - menu_x, 55);
            }
        }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
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
                cursorText("Attack", 2, 16);
            }
            else if(displayCursor == 0x02 || displayCursor == 0x06) {
                cursorText("Decay", 2, 26);
            }
            else if(displayCursor == 0x03 || displayCursor == 0x07) {
                cursorText("Sustain", 2, 36);
            }
            else if(displayCursor == 0x04 || displayCursor == 0x08) {
                cursorText("Release", 2, 46);
            }
            
        }

        /*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*/
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

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if(displayStatus == DISPST_MENU) {
            // タイトル
            pSprite->drawString("RP-DS16 Menu", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            pSprite->drawString("Global Settings", 2, 16);
            pSprite->drawString("File Manager", 2, 26);
            pSprite->drawString("MIDI Player", 2, 36);
            pSprite->drawString("Wavetable Viewer", 2, 46);

            // 塗り
            if(displayCursor == 0x01) {
                cursorText("Global Settings", 2, 16);
            }
            else if(displayCursor == 0x02) {
                cursorText("File Manager", 2, 26);
            }
            else if(displayCursor == 0x03) {
                cursorText("MIDI Player", 2, 36);
            }
            else if(displayCursor == 0x04) {
                cursorText("Wavetable Viewer", 2, 46);
            }
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if(displayStatus == DISPST_FILEMAN) {

            // タイトル
            if(currentDir.length() > 18) {
                String title = "..." + currentDir.substring(currentDir.length() - 18);
                pSprite->drawString(title, 2, 2);
            }else{
                pSprite->drawString(currentDir, 2, 2);
            }

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // ファイルを取得 4つ スクロール時のみ取得
            if(fileManRefresh) {
                pFile->getFiles(currentDir, file_buff, 4, fileman_index);
                for(int8_t i = 0; i < 4; i++) {
                    files[i].type = file_buff[i].isDirectory();
                    files[i].name = file_buff[i].name();
                    if(currentDir == "/")
                        files[i].path = "/" + files[i].name;
                    else
                        files[i].path = currentDir + "/" + files[i].name;
                    file_buff[i].close();
                }
                fileManRefresh = false;
            }
            
            // ファイルを表示
            for(int8_t i = 0; i < 4; i++) {
                pSprite->drawString(files[i].name, 2, 16 + i * 10);
            }

            bool isDir = false;

            // 塗り
            if(displayCursor == 0x01) {
                cursorText(files[0].name, 2, 16);
                if (files[0].type) isDir = true;
            }
            else if(displayCursor == 0x02) {
                cursorText(files[1].name, 2, 26);
                if (files[1].type) isDir = true;
            }
            else if(displayCursor == 0x03) {
                cursorText(files[2].name, 2, 36);
                if (files[2].type) isDir = true;
            }
            else if(displayCursor == 0x04) {
                cursorText(files[3].name, 2, 46);
                if (files[3].type) isDir = true;
            }
            else if(displayCursor == 0x05) {
                cursorText("CD TO ../", 2, 55);
            }

            // ファイルタイプ
            if(displayCursor > 0x00 && displayCursor < 0x05) {
                if (isDir) {
                    pSprite->drawString("DIR", 128 - 2 - pSprite->textWidth("DIR"), 55);
                    isEndOfFile = false;

                } else if(files[displayCursor - 1].name != nullptr) {
                    pSprite->drawString("FILE", 128 - 2 - pSprite->textWidth("FILE"), 55);
                    isEndOfFile = false;
                }
                else
                    isEndOfFile = true;
            }
        }

        // 画面更新
        pSprite->pushSprite(0, 0);
        pSprite->deleteSprite();
    }

    //  _   _ _ __  
    // | | | | '_ \ 
    // | |_| | |_) |
    //  \__,_| .__/ 
    //       |_|    
    //
    // 上が押された場合の処理
    void handleButtonUp(bool longPush = false) {
        if (longPush) return;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
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

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
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

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            if(displayCursor == 0x01) displayCursor = 0x04;
            else displayCursor--;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if(displayCursor == 0x01) {
                if(fileman_index != 0) {
                    fileman_index--;
                    fileManRefresh = true;
                    displayCursor = 0x01;
                }
            }
            else displayCursor--;
            refreshUI();
        }
    }

    //      _                     
    //   __| | _____      ___ __  
    //  / _` |/ _ \ \ /\ / / '_ \ 
    // | (_| | (_) \ V  V /| | | |
    //  \__,_|\___/ \_/\_/ |_| |_|
    //                           
    // 下が押された場合の処理
    void handleButtonDown(bool longPush = false) {
        if (longPush) return;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
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

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
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

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            if(displayCursor == 0x04) displayCursor = 0x01;
            else displayCursor++;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if(displayCursor == 0x04) {
                if(!isEndOfFile){
                    fileman_index++;
                    fileManRefresh = true;
                    displayCursor = 0x04;
                }
            } else if(displayCursor == 0x05) {
                displayCursor = 0x01;
            }
            else displayCursor++;
            refreshUI();
        }
    }

    //  _       __ _   
    // | | ___ / _| |_ 
    // | |/ _ \ |_| __|
    // | |  __/  _| |_ 
    // |_|\___|_|  \__|
    //                
    // 左が押された場合の処理
    void handleButtonLeft(bool longPush = false) {
        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0x00) ? (selectedPreset - 1) : 0xff;
                    if(!longPush) {
                        setPreset(selectedPreset, (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                    }
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_SINGLE) ? SYNTH_MULTI : (synthMode - 1);
                    pSynth->setSynthMode(synthMode);
                    setPreset(selectedPreset, (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        setPreset(selectedPreset2, 0x02);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0x00) ? (selectedPreset2 - 1) : 0xff;
                    if(!longPush) {
                        setPreset(selectedPreset2, 0x02);
                    }
                    refreshUI(); 
                    break;
            }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack - 1 >= 0) {
                        attack -= 1; 
                        if(!longPush) pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x02:
                    if (decay - 1 >= 0) {
                        decay -= 1; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain - 1 >= 0) {
                        sustain -= 1; 
                        if(!longPush) pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x04:
                    if (release - 1 >= 0) {
                        release -= 1; 
                        if(!longPush) pSynth->setRelease(0xff, release);
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack - 10 >= 0) {
                        attack -= 10; 
                        if(!longPush) pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay - 10 >= 0) {
                        decay -= 10; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain - 10 >= 0) {
                        sustain -= 10; 
                        if(!longPush) pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x08:
                    if (release - 10 >= 0) {
                        release -= 10; 
                        if(!longPush) pSynth->setRelease(0xff, release);
                    }
                    break;
            }
            refreshUI();
        }
    }

    //       _       _     _   
    //  _ __(_) __ _| |__ | |_ 
    // | '__| |/ _` | '_ \| __|
    // | |  | | (_| | | | | |_ 
    // |_|  |_|\__, |_| |_|\__|
    //         |___/           
    //
    // 右が押された場合の処理
    void handleButtonRight(bool longPush = false) {

        //todo
        uint8_t osc = 0x01;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0xff) ? (selectedPreset + 1) : 0x00;
                    if(!longPush) {
                        setPreset(selectedPreset, (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                    }
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_MULTI) ? SYNTH_SINGLE : (synthMode + 1);
                    pSynth->setSynthMode(synthMode);
                    setPreset(selectedPreset, (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        setPreset(selectedPreset2, 0x02);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0xff) ? (selectedPreset2 + 1) : 0x00;
                    if(!longPush) {
                        setPreset(selectedPreset2, 0x02);
                    }
                    refreshUI(); 
                    break;
            }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack + 1 <= 32000) {
                        attack += 1; 
                        if(!longPush) pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x02:
                    if (decay + 1 <= 32000) {
                        decay += 1; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain + 1 <= 1000) {
                        sustain += 1; 
                        if(!longPush) pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x04:
                    if (release + 1 <= 32000) {
                        release += 1; 
                        if(!longPush) pSynth->setRelease(0xff, release);  
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack + 10 <= 32000) {
                        attack += 10; 
                        if(!longPush) pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x06:
                    if (decay + 10 <= 32000) {
                        decay += 10; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain + 10 <= 1000) {
                        sustain += 10; 
                        if(!longPush) pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x08:
                    if (release + 10 <= 32000) {
                        release += 10; 
                        if(!longPush) pSynth->setRelease(0xff, release);  
                    }
                    break;
            }
            refreshUI();
        }
    }

    //             _            
    //   ___ _ __ | |_ ___ _ __ 
    //  / _ \ '_ \| __/ _ \ '__|
    // |  __/ | | | ||  __/ |   
    //  \___|_| |_|\__\___|_|   
    //                        
    // エンターが押された場合の処理
    void handleButtonEnter(bool longPush = false) {
        if (longPush) return;

        /*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*/
        if (displayStatus == DISPST_TITLE) {
            displayStatus = DISPST_PRESETS;
            refreshUI();

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
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

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
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

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            switch (displayCursor) {
                case 0x01:
                    break;
                case 0x02:
                    displayCursor = 0x00;
                    displayStatus = DISPST_FILEMAN;
                    refreshUI();
                    break;
                case 0x03:
                    break;
                case 0x04:
                    break;
            }
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            switch (displayCursor) {
                case 0x01:
                    if(files[0].type) {
                        currentDir = files[0].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x02:
                    if(files[1].type) {
                        currentDir = files[1].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x03:
                    if(files[2].type) {
                        currentDir = files[2].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x04:
                    if(files[3].type) {
                        currentDir = files[3].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x05:
                    int index = currentDir.lastIndexOf('/');
                    if (index == -1) currentDir = "/";
                    currentDir = currentDir.substring(0, index);
                    if (currentDir == "") currentDir = "/";
                    displayCursor = 0x00;
                    fileman_index = 0;
                    break;
            }
            fileManRefresh = true;
            refreshUI();
        }
    }

    //                           _ 
    //   ___ __ _ _ __   ___ ___| |
    //  / __/ _` | '_ \ / __/ _ \ |
    // | (_| (_| | | | | (_|  __/ |
    //  \___\__,_|_| |_|\___\___|_|
    //                            
    // キャンセルが押された場合の処理
    void handleButtonCancel(bool longPush = false) {

        /*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*/
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

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        } else if (displayStatus == DISPST_PRESETS) {
            if (longPush) return;
            displayCursor = 0x00;
            refreshUI();

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            if (longPush) return;
            displayCursor = 0x01;
            displayStatus = DISPST_PRESETS;
            refreshUI();

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        } else if (displayStatus == DISPST_MENU) {
            if (longPush) return;
            displayCursor = 0x00;
            displayStatus = DISPST_PRESETS;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if (longPush) return;
            if(displayCursor == 0x05) {
                displayCursor = 0x00;
                displayStatus = DISPST_MENU;
            }
            else displayCursor = 0x05;
            refreshUI();
        }
    }

    // ==================================================================================================================================================================== //

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