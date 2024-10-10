#include <IUIHandler.h>

#ifndef UIPRESETS_H
#define UIPRESETS_H

class UIPresets : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;

    SynthManager* pSynth;
    FileManager* pFile;

    Settings* pSettings;

    uint8_t* selectedSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIPresets(
        LGFX_Sprite* pSprite, SynthManager* pSynth, FileManager* pFile,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* selectedSynth, Settings* pSettings)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->pFile = pFile;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedSynth = selectedSynth;
        this->pSettings = pSettings;
    }

    // todo: 展示用プリセット読み込み
    bool loaded = false;
    // ここまで

    /** @brief 画面更新 */
    void refreshUI() override {
        // todo: 展示用プリセット読み込み
        if(!loaded) {
            pSettings->selectedPreset = 3;
            pSynth->setPreset(pSettings->selectedPreset, 0xff);
            loaded = true;
        }
        // ここまで

        // シンセ選択状態リセット
        *selectedSynth = 0x00;

        uint8_t preset_x = pSprite->textWidth(" ");
        uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
        char idstr[5];
        char idstr2[5];
        String fu1 = "F";
        String fu2 = "F";

        String preset_name1, preset_name2;
        if(pSettings->selectedPreset < FACTORY_PRESETS) {
            sprintf(idstr, "%03d ", pSettings->selectedPreset+1);
            preset_name1 = pSettings->default_presets[pSettings->selectedPreset];
        }
        else {
            fu1 = "U";
            sprintf(idstr, "%03d ", pSettings->selectedPreset+1 - FACTORY_PRESETS);
            preset_name1 = pSettings->user_presets[pSettings->selectedPreset - FACTORY_PRESETS].name;
        }
        if(pSettings->selectedPreset2 < FACTORY_PRESETS) {
            sprintf(idstr2, "%03d ", pSettings->selectedPreset2+1);
            preset_name2 = pSettings->default_presets[pSettings->selectedPreset2];
        }
        else {
            fu2 = "U";
            sprintf(idstr2, "%03d ", pSettings->selectedPreset2+1 - FACTORY_PRESETS);
            preset_name2 = pSettings->user_presets[pSettings->selectedPreset2 - FACTORY_PRESETS].name;
        }

        if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y - 7);
            pSprite->drawString(fu2 + idstr2 + preset_name2, preset_x, preset_y + 7);
        }
        else {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y);
        }

        // MIDIチャンネル
        if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_MONO){
            pSprite->drawString("MIDI=1&2", 2, 2);
        }else{
            pSprite->drawString("MIDI=1", 2, 2);
        }

        // シンセモード
        uint8_t synth_x = pSprite->textWidth(pSettings->modes[pSettings->synthMode]);
        pSprite->drawString(pSettings->modes[pSettings->synthMode], 128 - 2 - synth_x, 2);

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
            if(pSettings->selectedPreset < FACTORY_PRESETS) {
                sprintf(idstr, "%03d", pSettings->selectedPreset+1);
            }
            else {
                fu = "U";
                sprintf(idstr, "%03d", pSettings->selectedPreset+1 - FACTORY_PRESETS);
            }

            if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL)
                cursorText(" " + fu1 + idstr, 0, y - 7);
            else
                cursorText(" " + fu1 + idstr, 0, y);
        }
        else if(*displayCursor == 0x02) {
            uint8_t synth_x = pSprite->textWidth(pSettings->modes[pSettings->synthMode]);
            cursorText(pSettings->modes[pSettings->synthMode], 128 - 2 - synth_x, 2);
        }
        else if(*displayCursor == 0x03) {
            // blank
        }
        else if(*displayCursor == 0x04) {
            if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr2[5];

                String fu2 = "F";
                if(pSettings->selectedPreset2 < FACTORY_PRESETS) {
                    sprintf(idstr2, "%03d", pSettings->selectedPreset2+1);
                }
                else {
                    fu2 = "U";
                    sprintf(idstr2, "%03d", pSettings->selectedPreset2+1 - FACTORY_PRESETS);
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
                *displayCursor = (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
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
                *displayCursor = (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x04 : 0x05;
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
                pSettings->selectedPreset = (pSettings->selectedPreset != 0x00) ? (pSettings->selectedPreset - 1) : 128 + FACTORY_PRESETS - 1;
                if(!longPush) {
                    pSynth->setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                pSettings->synthMode = (pSettings->synthMode == SYNTH_POLY) ? SYNTH_MULTI : (pSettings->synthMode - 1);
                pSynth->setGlideMode(0xff, false); // monophonic, glideは必ず全てのシンセに送信する
                pSettings->isGlide = false;
                pSynth->setSynthMode(pSettings->synthMode);
                if(pSettings->synthMode == SYNTH_MONO){
                    pSynth->setMonophonic(0xff, true);
                } else {
                    pSynth->setMonophonic(0xff, false);
                }
                pSynth->setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                    pSynth->setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                pSettings->selectedPreset2 = (pSettings->selectedPreset2 != 0x00) ? (pSettings->selectedPreset2 - 1) : 128 + FACTORY_PRESETS - 1;
                if(!longPush) {
                    pSynth->setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                pSettings->selectedPreset = (pSettings->selectedPreset != 128 + FACTORY_PRESETS - 1) ? (pSettings->selectedPreset + 1) : 0x00;
                if(!longPush) {
                    pSynth->setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                pSettings->synthMode = (pSettings->synthMode == SYNTH_MULTI) ? SYNTH_POLY : (pSettings->synthMode + 1);
                pSynth->setGlideMode(0xff, false); // monophonic, glideは必ず全てのシンセに送信する
                pSettings->isGlide = false;
                pSynth->setSynthMode(pSettings->synthMode);
                if(pSettings->synthMode == SYNTH_MONO){
                    pSynth->setMonophonic(0xff, true);
                } else {
                    pSynth->setMonophonic(0xff, false);
                }
                pSynth->setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                    pSynth->setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                pSettings->selectedPreset2 = (pSettings->selectedPreset2 != 128 + FACTORY_PRESETS - 1) ? (pSettings->selectedPreset2 + 1) : 0x00;
                if(!longPush) {
                    pSynth->setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x02:
                pSynth->resetSynth(0xff); // 全てリセット(MIDI PANIC)
                break;
            case 0x01:
                *displayCursor = 0x01;
                *displayStatus = DISPST_PRESET_EDIT;
                // dual又はmultiモードはシンセ1を選択それ以外はブロードキャスト
                if(pSettings->synthMode == SYNTH_DUAL || pSettings->synthMode == SYNTH_MULTI) {
                    *selectedSynth = 0x01;
                }
                else {
                    *selectedSynth = 0xff;
                }
                break;
            case 0x04:
                *displayCursor = 0x01;
                *displayStatus = DISPST_PRESET_EDIT;
                // dual又はmultiモード限定のためシンセ2を選択
                *selectedSynth = 0x02;
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