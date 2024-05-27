#include <IUIHandler.h>

#ifndef UIOSCUNISON_H
#define UIOSCUNISON_H

class UIOscUnison : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    uint8_t* osc1_voice;
    uint8_t* osc2_voice;
    uint8_t* osc1_detune;
    uint8_t* osc2_detune;

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
    UIOscUnison(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* osc1_voice, uint8_t* osc2_voice, uint8_t* osc1_detune, uint8_t* osc2_detune, uint8_t* selectedOsc)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->osc1_voice = osc1_voice;
        this->osc2_voice = osc2_voice;
        this->osc1_detune = osc1_detune;
        this->osc2_detune = osc2_detune;
        this->selectedOsc = selectedOsc;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        if(*selectedOsc == 0x01) pSprite->drawString("<OSC1 Unison Editor>", 2, 2);
        else if(*selectedOsc == 0x02) pSprite->drawString("<OSC2 Unison Editor>", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Unison
        if(*selectedOsc == 0x01) {
            char o1v_chr[6]; sprintf(o1v_chr, "%d", *osc1_voice);
            char o1d_chr[6]; sprintf(o1d_chr, "%d", *osc1_detune);
            pSprite->drawString("OSC1 Voice : " + String(o1v_chr) + "v", 2, 16);
            pSprite->drawString("OSC1 Detune: " + String(o1d_chr) + "%", 2, 26);
        }
        else if(*selectedOsc == 0x02) {
            char o2v_chr[6]; sprintf(o2v_chr, "%d", *osc2_voice);
            char o2d_chr[6]; sprintf(o2d_chr, "%d", *osc2_detune);
            pSprite->drawString("OSC2 Voice : " + String(o2v_chr) + "v", 2, 16);
            pSprite->drawString("OSC2 Detune: " + String(o2d_chr) + "%", 2, 26);
        }

        // 塗り
        if(*displayCursor == 0x01) {
            if(*selectedOsc == 0x01) cursorText("OSC1 Voice", 2, 16);
            else if(*selectedOsc == 0x02) cursorText("OSC2 Voice", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            if(*selectedOsc == 0x01) cursorText("OSC1 Detune", 2, 26);
            else if(*selectedOsc == 0x02) cursorText("OSC2 Detune", 2, 26);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x02;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x02) *displayCursor = 0x01;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*selectedOsc == 0x01){
                    if(*osc1_voice - 1 >= 1) {
                        *osc1_voice -= 1; 
                    }
                    if(!longPush) pSynth->setVoice(0xff, *osc1_voice, 0x01);
                }
                else if(*selectedOsc == 0x02){
                    if(*osc2_voice - 1 >= 1) {
                        *osc2_voice -= 1; 
                    }
                    if(!longPush) pSynth->setVoice(0xff, *osc2_voice, 0x02);
                }
                break;

            case 0x02:
                if(*selectedOsc == 0x01){
                    if(*osc1_detune - 1 >= 0) {
                        *osc1_detune -= 1; 
                    }
                    if(!longPush) pSynth->setDetune(0xff, *osc1_detune, 0x01);
                }
                else if(*selectedOsc == 0x02){
                    if(*osc2_detune - 1 >= 0) {
                        *osc2_detune -= 1; 
                    }
                    if(!longPush) pSynth->setDetune(0xff, *osc2_detune, 0x02);
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*selectedOsc == 0x01) {
                    if(*osc1_voice + 1 <= 8) {
                        *osc1_voice += 1; 
                    }
                    if(!longPush) pSynth->setVoice(0xff, *osc1_voice, 0x01);
                }
                else if(*selectedOsc == 0x02) {
                    if(*osc2_voice + 1 <= 8) {
                        *osc2_voice += 1; 
                    }
                    if(!longPush) pSynth->setVoice(0xff, *osc2_voice, 0x02);
                }
                break;

            case 0x02:
                if(*selectedOsc == 0x01) {
                    if(*osc1_detune + 1 <= 100) {
                        *osc1_detune += 1; 
                    }
                    if(!longPush) pSynth->setDetune(0xff, *osc1_detune, 0x01);
                }
                else if(*selectedOsc == 0x02) {
                    if(*osc2_detune + 1 <= 100) {
                        *osc2_detune += 1; 
                    }
                    if(!longPush) pSynth->setDetune(0xff, *osc2_detune, 0x02);
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
        *displayCursor = 0x02;
        *displayStatus = DISPST_OSC;
    }
};

#endif // UIOSCUNISON_H