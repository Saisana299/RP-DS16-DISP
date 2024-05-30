#include <IUIHandler.h>

#ifndef UIPRESETEDIT_H
#define UIPRESETEDIT_H

class UIPresetEdit : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    uint8_t* selectedOsc;

    LGFX_Sprite* pSprite;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIPresetEdit( LGFX_Sprite* pSprite, uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedOsc) {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->selectedOsc = selectedOsc;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Preset Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        pSprite->drawString("Oscillator1", 2, 16);
        pSprite->drawString("Oscillator2", 2, 26);
        pSprite->drawString("ADSR Envelope", 2, 36);
        pSprite->drawString("Filter", 2, 46);
        pSprite->drawString("Effector", 2, 56);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("Oscillator1", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Oscillator2", 2, 26);
        }
        else if(*displayCursor == 0x03) {
            cursorText("ADSR Envelope", 2, 36);
        }
        else if(*displayCursor == 0x04) {
            cursorText("Filter", 2, 46);
        }
        else if(*displayCursor == 0x05) {
            cursorText("Effector", 2, 56);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) *displayCursor = 0x05;
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x05) *displayCursor = 0x01;
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC;
                *selectedOsc = 0x01;
                break;
            case 0x02:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC;
                *selectedOsc = 0x02;
                break;
            case 0x03:
                *displayCursor = 0x01;
                *displayStatus = DISPST_ADSR;
                break;
            case 0x04:
                break;
            case 0x05:
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x01;
        *displayStatus = DISPST_PRESETS;
    }
};

#endif // UIPRESETEDIT_H