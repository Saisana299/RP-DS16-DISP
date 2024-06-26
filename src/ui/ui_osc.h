#include <IUIHandler.h>
#include <synth_manager.h>

#ifndef UIOSC_H
#define UIOSC_H

class UIOsc : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    uint8_t* selectedOsc;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    int16_t* osc1_level;
    int16_t* osc2_level;
    int16_t* osc_sub_level;

    bool* isFirst;

    uint8_t* mod_status;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIOsc( LGFX_Sprite* pSprite, SynthManager* pSynth,
    uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedOsc,
    int16_t* osc1_level, int16_t* osc2_level, int16_t* osc_sub_level, bool* isFirst, uint8_t* mod_status)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->selectedOsc = selectedOsc;
        this->osc1_level = osc1_level;
        this->osc2_level = osc2_level;
        this->osc_sub_level = osc_sub_level;
        this->isFirst = isFirst;
        this->mod_status = mod_status;
    }

    /** @brief 画面更新 */
    void refreshUI() override {

        if(*displayCursor < 0x06) {
            // タイトル
            pSprite->drawString("> Select Oscillator", 2, 2);
            // オシレータ選択
            pSprite->drawString("OSC1", 2, 16);
            pSprite->drawString("OSC2", 2, 26);
            pSprite->drawString("Sub", 2, 36);
            pSprite->drawString("Noise", 2, 46);
            pSprite->drawString("Mod: ---", 2, 56);
        }
        else {
            char lv_chr[5];
            if(*selectedOsc == 0x01) {
                pSprite->drawString("> OSC1 Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)*osc1_level / 10.0f);
            }
            else if(*selectedOsc == 0x02) {
                pSprite->drawString("> OSC2 Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)*osc2_level / 10.0f);
            }
            else if(*selectedOsc == 0x03) {
                pSprite->drawString("> Sub Settings", 2, 2);
                sprintf(lv_chr, "%.1f", (float)*osc_sub_level / 10.0f);
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
            cursorText("Noise", 2, 46);
        }
        else if(*displayCursor == 0x05) {
            cursorText("Mod", 2, 56);
        }
        else if(*displayCursor == 0x06) {
            cursorText("Wavetable", 2, 16);
        }
        else if(*displayCursor == 0x07) {
            if(*selectedOsc != 0x03) cursorText("Unison", 2, 26);
            else cursorText("---", 2, 26);
        }
        else if(*displayCursor == 0x08) {
            cursorText("Pitch", 2, 36);
        }
        else if(*displayCursor == 0x09) {
            cursorText("Level", 2, 46);
        }
        else if(*displayCursor == 0x0A) {
            cursorText("Pan", 2, 56);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x05;
        else if(*displayCursor == 0x06) *displayCursor = 0x0A;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x05) *displayCursor = 0x01;
        else if(*displayCursor == 0x0A) *displayCursor = 0x06;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(*displayCursor == 0x09) {
            if(*selectedOsc == 0x01) {
                if(*osc1_level - 10 >= 0) *osc1_level -= 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x01, *osc1_level);
            }
            else if(*selectedOsc == 0x02) {
                if(*osc2_level - 10 >= 0) *osc2_level -= 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x02, *osc2_level);
            }
            else if(*selectedOsc == 0x03) {
                if(*osc_sub_level - 10 >= 0) *osc_sub_level -= 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x03, *osc_sub_level);
            }
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(*displayCursor == 0x09) {
            if(*selectedOsc == 0x01) {
                if(*osc1_level + 10 <= 1000) *osc1_level += 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x01, *osc1_level);
            }
            else if(*selectedOsc == 0x02) {
                if(*osc2_level + 10 <= 1000) *osc2_level += 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x02, *osc2_level);
            }
            else if(*selectedOsc == 0x03) {
                if(*osc_sub_level + 10 <= 1000) *osc_sub_level += 10;
                if(!longPush) pSynth->setOscLevel(0xff, 0x03, *osc_sub_level);
            }
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                *displayCursor = 0x06;
                *selectedOsc = 0x01;
                break;
            case 0x02:
                *displayCursor = 0x06;
                *selectedOsc = 0x02;
                break;
            case 0x03:
                *displayCursor = 0x06;
                *selectedOsc = 0x03;
                break;
            case 0x04:
                break;
            case 0x05:
                *mod_status = 0x01;
                pSynth->setMod(0xff, 0x01);
                break;
            case 0x06:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC_WAVE;
                *isFirst = true;
                break;
            case 0x07:
                if(*selectedOsc != 0x03) {
                    *displayCursor = 0x01;
                    *displayStatus = DISPST_OSC_UNISON;
                }
                break;
            case 0x08:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC_PITCH;
                break;
            case 0x09:
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        if(*displayCursor < 0x06) {
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