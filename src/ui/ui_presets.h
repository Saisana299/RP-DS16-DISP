#include <IUIHandler.h>

#ifndef UIPRESETS_H
#define UIPRESETS_H

#define FACTORY_PRESETS 4
#define FACTORY_WAVETABLES 4

class UIPresets : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    // シンセ関連
    uint8_t* synthMode;
    uint8_t* selectedPreset;
    uint8_t* selectedPreset2;

    String* default_presets;
    String* modes;

    // ユーザープリセット
    Preset* user_presets;

    // プリセット用バッファ
    int16_t* wave_table_buff;

    LGFX_Sprite* pSprite;

    SynthManager* pSynth;
    FileManager* pFile;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    void setPreset(uint8_t id, uint8_t synth) {
        if(id < FACTORY_PRESETS)
            // defaultプリセットはosc=0x01固定
            pSynth->setShape(synth, 0x01, id);
        else {
            JsonDocument doc;
            pFile->getJson(&doc, user_presets[id - FACTORY_PRESETS].path);
            String osc1_type = doc["osc1"]["type"];
            String osc2_type = doc["osc2"]["type"];
            
            // osc1
            if(osc1_type == "custom") {
                String wave = doc["osc1"]["wave"];
                pFile->getJson(&doc, "/rp-ds16/wavetable/" + wave);
                JsonArray waveTableArray = doc["wave_table"].as<JsonArray>();
                copyArray(waveTableArray, wave_table_buff, waveTableArray.size());
                pSynth->setShape(synth, 0x01, id, wave_table_buff);

            } else if(osc1_type == "default") {
                uint8_t osc1_id = doc["osc1"]["id"];
                pSynth->setShape(synth, 0x01, osc1_id);
            }

            // osc2
            if(osc2_type == "custom") {
                String wave = doc["osc2"]["wave"];
                pFile->getJson(&doc, "/rp-ds16/wavetable/" + wave);
                JsonArray waveTableArray = doc["wave_table"].as<JsonArray>();
                copyArray(waveTableArray, wave_table_buff, waveTableArray.size());
                pSynth->setShape(synth, 0x02, id, wave_table_buff);

            } else if(osc2_type == "default") {
                uint8_t osc2_id = doc["osc2"]["id"];
                pSynth->setShape(synth, 0x02, osc2_id);
            }

            // ADSR

            // todo
        }
    }

public:
    UIPresets(
        LGFX_Sprite* pSprite, SynthManager* pSynth, FileManager* pFile,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* synthMode, uint8_t* selectedPreset, uint8_t* selectedPreset2,
        String* default_presets, String* modes, Preset* user_presets, int16_t* wave_table_buff)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->pFile = pFile;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->synthMode = synthMode;
        this->selectedPreset = selectedPreset;
        this->selectedPreset2 = selectedPreset2;
        this->default_presets = default_presets;
        this->modes = modes;
        this->user_presets = user_presets;
        this->wave_table_buff = wave_table_buff;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        uint8_t preset_x = pSprite->textWidth(" ");
        uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
        char idstr[5]; 
        char idstr2[5];
        String fu1 = "F";
        String fu2 = "F";

        String preset_name1, preset_name2;
        if(*selectedPreset < 0x04) {
            sprintf(idstr, "%03d ", *selectedPreset+1);
            preset_name1 = default_presets[*selectedPreset];
        }
        else {
            fu1 = "U";
            sprintf(idstr, "%03d ", *selectedPreset+1 - FACTORY_PRESETS);
            preset_name1 = user_presets[*selectedPreset - FACTORY_PRESETS].name;
        }
        if(*selectedPreset2 < 0x04) {
            sprintf(idstr2, "%03d ", *selectedPreset2+1);
            preset_name2 = default_presets[*selectedPreset2];
        }
        else {
            fu2 = "U";
            sprintf(idstr2, "%03d ", *selectedPreset2+1 - FACTORY_PRESETS);
            preset_name2 = user_presets[*selectedPreset2 - FACTORY_PRESETS].name;
        }

        if(*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y - 7);
            pSprite->drawString(fu2 + idstr2 + preset_name2, preset_x, preset_y + 7);
        }
        else {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y);
        }
        
        // MIDIチャンネル
        if(*synthMode == SYNTH_MULTI){
            pSprite->drawString("MIDI=1&2", 2, 2);
        }else{
            pSprite->drawString("MIDI=1", 2, 2);
        }

        // シンセモード
        uint8_t synth_x = pSprite->textWidth(modes[*synthMode]);
        pSprite->drawString(modes[*synthMode], 128 - 2 - synth_x, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
        pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

        // メニュー
        uint8_t menu_x = pSprite->textWidth(">>");
        pSprite->drawString(">>", 128 - 2 - menu_x, 55);

        // 塗り
        if(*displayCursor == 0x01) {

            uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[6];

            String fu = "F";
            if(*selectedPreset < FACTORY_PRESETS) {
                sprintf(idstr, "%03d", *selectedPreset+1);
            }
            else {
                fu = "U";
                sprintf(idstr, "%03d", *selectedPreset+1 - FACTORY_PRESETS);
            }

            if(*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) 
                cursorText(" " + fu1 + idstr, 0, y - 7);
            
            else 
                cursorText(" " + fu1 + idstr, 0, y);
            
        }
        else if(*displayCursor == 0x02) {
            uint8_t synth_x = pSprite->textWidth(modes[*synthMode]);
            cursorText(modes[*synthMode], 128 - 2 - synth_x, 2);
        }
        else if(*displayCursor == 0x03) {
            // blank
        }
        else if(*displayCursor == 0x04) {
            if(*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) {
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr2[5];

                String fu2 = "F";
                if(*selectedPreset2 < FACTORY_PRESETS) {
                    sprintf(idstr2, "%03d", *selectedPreset2+1);
                }
                else {
                    fu2 = "U";
                    sprintf(idstr2, "%03d", *selectedPreset2+1 - FACTORY_PRESETS);
                }

                cursorText(" " + fu2 + idstr2, 0, y + 7);
            }
        }
        else if(*displayCursor == 0x05) {
            uint8_t menu_x = pSprite->textWidth(">>");
            cursorText(">>", 128 - 2 - menu_x, 55);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x00:
            case 0x01:
            case 0x02:
                (*displayCursor)++;
                break;
            case 0x03:
                *displayCursor = 0x05;
                break;
            case 0x04:
                *displayCursor = 0x01;
                break;
            case 0x05:
                *displayCursor = (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
                break;
        }
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x00:
            case 0x02:
                *displayCursor = 0x01;
                break;
            case 0x01:
                *displayCursor = (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x04 : 0x05;
                break;
            case 0x03:
                *displayCursor = 0x02;
                break;
            case 0x04:
                *displayCursor = 0x05;
                break;
            case 0x05:
                *displayCursor = 0x03;
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                *selectedPreset = (*selectedPreset != 0x00) ? (*selectedPreset - 1) : 0x83;
                if(!longPush) {
                    setPreset(*selectedPreset, (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                *synthMode = (*synthMode == SYNTH_SINGLE) ? SYNTH_MULTI : (*synthMode - 1);
                pSynth->setSynthMode(*synthMode);
                setPreset(*selectedPreset, (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) {
                    setPreset(*selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                *selectedPreset2 = (*selectedPreset2 != 0x00) ? (*selectedPreset2 - 1) : 0x83;
                if(!longPush) {
                    setPreset(*selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                *selectedPreset = (*selectedPreset != 0x83) ? (*selectedPreset + 1) : 0x00;
                if(!longPush) {
                    setPreset(*selectedPreset, (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                *synthMode = (*synthMode == SYNTH_MULTI) ? SYNTH_SINGLE : (*synthMode + 1);
                pSynth->setSynthMode(*synthMode);
                setPreset(*selectedPreset, (*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(*synthMode == SYNTH_MULTI || *synthMode == SYNTH_DUAL) {
                    setPreset(*selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                *selectedPreset2 = (*selectedPreset2 != 0x83) ? (*selectedPreset2 + 1) : 0x00;
                if(!longPush) {
                    setPreset(*selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x02:
                pSynth->resetSynth(0xff);
                break;
            case 0x01:
                *displayCursor = 0x01;
                *displayStatus = DISPST_PRESET_EDIT;
                break;
            case 0x00:
                *displayCursor = 0x01;
                break;
            case 0x05:
                *displayCursor = 0x01;
                *displayStatus = DISPST_MENU;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x00;
    }
};

#endif // UIPRESETS_H