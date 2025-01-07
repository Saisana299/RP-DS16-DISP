#include <IUIHandler.h>
#include <synth_manager.h>

#ifndef UIOSC_H
#define UIOSC_H

class UIOsc : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;
    uint8_t* selectedSynth;

    uint8_t* selectedOsc;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    Settings* pSettings;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIOsc( LGFX_Sprite* pSprite, SynthManager* pSynth,
    uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedOsc, uint8_t* selectedSynth, Settings* pSettings)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->selectedOsc = selectedOsc;
        this->selectedSynth = selectedSynth;
        this->pSettings = pSettings;
    }

    /** @brief 画面更新 */
    void refreshUI() override {

        if(*displayCursor < 0x05) {
            // タイトル
            pSprite->drawString("> Select Oscillator", 2, 2);
            // オシレータ選択
            pSprite->drawString("OSC1", 2, 16);
            pSprite->drawString("OSC2", 2, 26);
            pSprite->drawString("Sub", 2, 36);
            String mod_status = "Disable";
            if(pSettings->mod_status == 0x01) mod_status = "Ring";
            pSprite->drawString("Mod: " + mod_status, 2, 46);
        }
        else {
            char lv_chr[5];
            if(*selectedOsc == 0x01) {
                pSprite->drawString("> OSC1 Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)pSettings->osc1_level / 10.0f);
            }
            else if(*selectedOsc == 0x02) {
                pSprite->drawString("> OSC2 Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)pSettings->osc2_level / 10.0f);
            }
            else if(*selectedOsc == 0x03) {
                pSprite->drawString("> Sub Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)pSettings->osc_sub_level / 10.0f);
            }
            pSprite->drawString("Wavetable", 2, 16);
            if(*selectedOsc != 0x03) pSprite->drawString("Unison", 2, 26);
            else pSprite->drawString("---", 2, 26);
            pSprite->drawString("Pitch", 2, 36);
            pSprite->drawString("Level: " + String(lv_chr) + "%", 2, 46);
            pSprite->drawString("Pan  : ---", 2, 56);
        }

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("OSC1", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("OSC2", 2, 26);
        }
        else if(*displayCursor == 0x03) {
            cursorText("Sub", 2, 36);
        }
        else if(*displayCursor == 0x04) {
            cursorText("Mod", 2, 46);
        }
        else if(*displayCursor == 0x05) {
            cursorText("Wavetable", 2, 16);
        }
        else if(*displayCursor == 0x06) {
            if(*selectedOsc != 0x03) cursorText("Unison", 2, 26);
            else cursorText("---", 2, 26);
        }
        else if(*displayCursor == 0x07) {
            cursorText("Pitch", 2, 36);
        }
        else if(*displayCursor == 0x08) {
            cursorText("Level", 2, 46);
        }
        else if(*displayCursor == 0x09) {
            cursorText("Pan", 2, 56);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x04;
        else if(*displayCursor == 0x05) *displayCursor = 0x09;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x04) *displayCursor = 0x01;
        else if(*displayCursor == 0x09) *displayCursor = 0x05;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(*displayCursor == 0x08) {
            if(*selectedOsc == 0x01) {
                if(pSettings->osc1_level - 10 >= 0) pSettings->osc1_level -= 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x01, pSettings->osc1_level);
            }
            else if(*selectedOsc == 0x02) {
                if(pSettings->osc2_level - 10 >= 0) pSettings->osc2_level -= 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x02, pSettings->osc2_level);
            }
            else if(*selectedOsc == 0x03) {
                if(pSettings->osc_sub_level - 10 >= 0) pSettings->osc_sub_level -= 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x03, pSettings->osc_sub_level);
            }
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(*displayCursor == 0x08) {
            if(*selectedOsc == 0x01) {
                if(pSettings->osc1_level + 10 <= 1000) pSettings->osc1_level += 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x01, pSettings->osc1_level);
            }
            else if(*selectedOsc == 0x02) {
                if(pSettings->osc2_level + 10 <= 1000) pSettings->osc2_level += 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x02, pSettings->osc2_level);
            }
            else if(*selectedOsc == 0x03) {
                if(pSettings->osc_sub_level + 10 <= 1000) pSettings->osc_sub_level += 10;
                if(!longPush) pSynth->setOscLevel(*selectedSynth, 0x03, pSettings->osc_sub_level);
            }
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                *displayCursor = 0x05;
                *selectedOsc = 0x01;
                break;
            case 0x02:
                *displayCursor = 0x05;
                *selectedOsc = 0x02;
                break;
            case 0x03:
                *displayCursor = 0x05;
                *selectedOsc = 0x03;
                break;
            case 0x04:
                if(pSettings->mod_status == 0x00) {
                    pSettings->mod_status = 0x01;
                }else{
                    pSettings->mod_status = 0x00;
                }
                pSynth->setMod(*selectedSynth, pSettings->mod_status);
                break;
            case 0x05:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC_WAVE;
                pSettings->isFirst = true;
                break;
            case 0x06:
                if(*selectedOsc != 0x03) {
                    *displayCursor = 0x01;
                    *displayStatus = DISPST_OSC_UNISON;
                }
                break;
            case 0x07:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC_PITCH;
                break;
            case 0x08:
                break;
            case 0x09:
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        if(*displayCursor < 0x05) {
            *displayCursor = 0x01;
            *displayStatus = DISPST_PRESET_EDIT;
        }
        else {
            if(*selectedOsc == 0x01) *displayCursor = 0x01;
            else if(*selectedOsc == 0x02) *displayCursor = 0x02;
            else if(*selectedOsc == 0x03) *displayCursor = 0x03;
        }
    }
};

#endif // UIOSC_H