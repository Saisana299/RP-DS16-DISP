#include <IUIHandler.h>

#ifndef UIOSCPITCH_H
#define UIOSCPITCH_H

class UIOscPitch : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;
    uint8_t* selectedSynth;

    int8_t* osc1_oct;
    int8_t* osc2_oct;
    int8_t* osc1_semi;
    int8_t* osc2_semi;
    int8_t* osc1_cent;
    int8_t* osc2_cent;
    int8_t* osc_sub_oct;
    int8_t* osc_sub_semi;
    int8_t* osc_sub_cent;

    uint8_t* selectedOsc;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIOscPitch(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedOsc,
        int8_t* osc1_oct, int8_t* osc2_oct,
        int8_t* osc1_semi, int8_t* osc2_semi,
        int8_t* osc1_cent, int8_t* osc2_cent,
        int8_t* osc_sub_oct, int8_t* osc_sub_semi, int8_t* osc_sub_cent, uint8_t* selectedSynth)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedOsc = selectedOsc;
        this->osc1_oct = osc1_oct;
        this->osc2_oct = osc2_oct;
        this->osc1_semi = osc1_semi;
        this->osc2_semi = osc2_semi;
        this->osc1_cent = osc1_cent;
        this->osc2_cent = osc2_cent;
        this->osc_sub_oct = osc_sub_oct;
        this->osc_sub_semi = osc_sub_semi;
        this->osc_sub_cent = osc_sub_cent;
        this->selectedSynth = selectedSynth;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        if(*selectedOsc == 0x01) pSprite->drawString("> OSC1 Pitch Editor", 2, 2);
        else if(*selectedOsc == 0x02) pSprite->drawString("> OSC2 Pitch Editor", 2, 2);
        else if(*selectedOsc == 0x03) pSprite->drawString("> Sub Pitch Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Unison
        if(*selectedOsc == 0x01) {
            char o1o_chr[6]; sprintf(o1o_chr, "%d", *osc1_oct);
            char o1s_chr[6]; sprintf(o1s_chr, "%d", *osc1_semi);
            char o1c_chr[6]; sprintf(o1c_chr, "%d", *osc1_cent);
            pSprite->drawString("OSC1 Octave  : " + String(o1o_chr), 2, 16);
            pSprite->drawString("OSC1 Semitone: " + String(o1s_chr), 2, 26);
            pSprite->drawString("OSC1 Cent    : " + String(o1c_chr), 2, 36);
        }
        else if(*selectedOsc == 0x02) {
            char o2o_chr[6]; sprintf(o2o_chr, "%d", *osc2_oct);
            char o2s_chr[6]; sprintf(o2s_chr, "%d", *osc2_semi);
            char o2c_chr[6]; sprintf(o2c_chr, "%d", *osc2_cent);
            pSprite->drawString("OSC2 Octave  : " + String(o2o_chr), 2, 16);
            pSprite->drawString("OSC2 Semitone: " + String(o2s_chr), 2, 26);
            pSprite->drawString("OSC2 Cent    : " + String(o2c_chr), 2, 36);
        }
        else if(*selectedOsc == 0x03) {
            char oso_chr[6]; sprintf(oso_chr, "%d", *osc_sub_oct);
            char oss_chr[6]; sprintf(oss_chr, "%d", *osc_sub_semi);
            char osc_chr[6]; sprintf(osc_chr, "%d", *osc_sub_cent);
            pSprite->drawString("Sub Octave  : " + String(oso_chr), 2, 16);
            pSprite->drawString("Sub Semitone: " + String(oss_chr), 2, 26);
            pSprite->drawString("Sub Cent    : " + String(osc_chr), 2, 36);
        }

        // 塗り
        if(*displayCursor == 0x01) {
            if(*selectedOsc == 0x01) cursorText("OSC1 Octave", 2, 16);
            else if(*selectedOsc == 0x02) cursorText("OSC2 Octave", 2, 16);
            else if(*selectedOsc == 0x03) cursorText("Sub Octave", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            if(*selectedOsc == 0x01) cursorText("OSC1 Semitone", 2, 26);
            else if(*selectedOsc == 0x02) cursorText("OSC2 Semitone", 2, 26);
            else if(*selectedOsc == 0x03) cursorText("Sub Semitone", 2, 26);
        }
        else if(*displayCursor == 0x03) {
            if(*selectedOsc == 0x01) cursorText("OSC1 Cent", 2, 36);
            else if(*selectedOsc == 0x02) cursorText("OSC2 Cent", 2, 36);
            else if(*selectedOsc == 0x03) cursorText("Sub Cent", 2, 36);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x03;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x03) *displayCursor = 0x01;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*selectedOsc == 0x01){
                    if(*osc1_oct - 1 >= -4) {
                        *osc1_oct -= 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x01, *osc1_oct);
                }
                else if(*selectedOsc == 0x02){
                    if(*osc2_oct - 1 >= -4) {
                        *osc2_oct -= 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x02, *osc2_oct);
                }
                else if(*selectedOsc == 0x03){
                    if(*osc_sub_oct - 1 >= -4) {
                        *osc_sub_oct -= 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x03, *osc_sub_oct);
                }
                break;

            case 0x02:
                if(*selectedOsc == 0x01){
                    if(*osc1_semi - 1 >= -12) {
                        *osc1_semi -= 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x01, *osc1_semi);
                }
                else if(*selectedOsc == 0x02){
                    if(*osc2_semi - 1 >= -12) {
                        *osc2_semi -= 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x02, *osc2_semi);
                }
                else if(*selectedOsc == 0x03){
                    if(*osc_sub_semi - 1 >= -12) {
                        *osc_sub_semi -= 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x03, *osc_sub_semi);
                }
                break;

            case 0x03:
                if(*selectedOsc == 0x01){
                    if(*osc1_cent - 1 >= -100) {
                        *osc1_cent -= 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x01, *osc1_cent);
                }
                else if(*selectedOsc == 0x02){
                    if(*osc2_cent - 1 >= -100) {
                        *osc2_cent -= 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x02, *osc2_cent);
                }
                else if(*selectedOsc == 0x03){
                    if(*osc_sub_cent - 1 >= -100) {
                        *osc_sub_cent -= 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x03, *osc_sub_cent);
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*selectedOsc == 0x01) {
                    if(*osc1_oct + 1 <= 4) {
                        *osc1_oct += 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x01, *osc1_oct);
                }
                else if(*selectedOsc == 0x02) {
                    if(*osc2_oct + 1 <= 4) {
                        *osc2_oct += 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x02, *osc2_oct);
                }
                else if(*selectedOsc == 0x03) {
                    if(*osc_sub_oct + 1 <= 4) {
                        *osc_sub_oct += 1;
                    }
                    if(!longPush) pSynth->setOscOctave(*selectedSynth, 0x03, *osc_sub_oct);
                }
                break;

            case 0x02:
                if(*selectedOsc == 0x01) {
                    if(*osc1_semi + 1 <= 12) {
                        *osc1_semi += 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x01, *osc1_semi);
                }
                else if(*selectedOsc == 0x02) {
                    if(*osc2_semi + 1 <= 12) {
                        *osc2_semi += 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x02, *osc2_semi);
                }
                else if(*selectedOsc == 0x03) {
                    if(*osc_sub_semi + 1 <= 12) {
                        *osc_sub_semi += 1;
                    }
                    if(!longPush) pSynth->setOscSemitone(*selectedSynth, 0x03, *osc_sub_semi);
                }
                break;

            case 0x03:
                if(*selectedOsc == 0x01) {
                    if(*osc1_cent + 1 <= 100) {
                        *osc1_cent += 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x01, *osc1_cent);
                }
                else if(*selectedOsc == 0x02) {
                    if(*osc2_cent + 1 <= 100) {
                        *osc2_cent += 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x02, *osc2_cent);
                }
                else if(*selectedOsc == 0x03) {
                    if(*osc_sub_cent + 1 <= 100) {
                        *osc_sub_cent += 1;
                    }
                    if(!longPush) pSynth->setOscCent(*selectedSynth, 0x03, *osc_sub_cent);
                }
                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x06;
        *displayStatus = DISPST_OSC;
    }
};

#endif // UIOSCPITCH_H