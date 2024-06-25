#include <IUIHandler.h>
#include <synth_manager.h>

#ifndef UIAMP_H
#define UIAMP_H

class UIAmp : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    int16_t* level;
    uint8_t* pan;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIAmp( LGFX_Sprite* pSprite, SynthManager* pSynth, uint8_t* displayStatus, uint8_t* displayCursor, int16_t* level, uint8_t* pan) {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->level = level;
        this->pan = pan;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Amplifier Settings", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        char lv_chr[5]; sprintf(lv_chr, "%.1f", (float)*level / 10.0f);
        char pn_chr[5]; sprintf(pn_chr, "%d", *pan);

        pSprite->drawString("Level Envelope", 2, 16);
        pSprite->drawString("Glide", 2, 26);
        pSprite->drawString("Level: " + String(lv_chr) + "%", 2, 36);
        pSprite->drawString("Pan  : " + String(pn_chr), 2, 46);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("Level Envelope", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Glide", 2, 26);
        }
        else if(*displayCursor == 0x03) {
            cursorText("Level", 2, 36);
        }
        else if(*displayCursor == 0x04) {
            cursorText("Pan", 2, 46);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x04;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x04) *displayCursor = 0x01;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(*displayCursor == 0x03) {
            if(*level - 10 >= 0) *level -= 10;
            if(!longPush) pSynth->setAmpLevel(0xff, *level);
        }
        else if(*displayCursor == 0x04) {
            if(*pan - 1 >= 0) *pan -= 1;
            if(!longPush) pSynth->setAmpPan(0xff, *pan);
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(*displayCursor == 0x03) {
            if(*level + 10 <= 1000) *level += 10;
            if(!longPush) pSynth->setAmpLevel(0xff, *level);
        }
        else if(*displayCursor == 0x04) {
            if(*pan + 1 <= 100) *pan += 1;
            if(!longPush) pSynth->setAmpPan(0xff, *pan);
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                *displayCursor = 0x01;
                *displayStatus = DISPST_AMP_ADSR;
                break;
            case 0x02:
                *displayCursor = 0x01;
                *displayStatus = DISPST_AMP_GLIDE;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x02;
        *displayStatus = DISPST_PRESET_EDIT;
    }
};

#endif // UIAMP_H