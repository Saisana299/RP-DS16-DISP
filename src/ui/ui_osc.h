#include <IUIHandler.h>

#ifndef UIOSC_H
#define UIOSC_H

class UIOsc : public IUIHandler {
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
    UIOsc( LGFX_Sprite* pSprite, uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedOsc) {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->selectedOsc = selectedOsc;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        if(*selectedOsc == 0x01) pSprite->drawString("> OSC1 Settings", 2, 2);
        else if(*selectedOsc == 0x02) pSprite->drawString("> OSC2 Settings", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        pSprite->drawString("Wavetable", 2, 16);
        pSprite->drawString("Unison", 2, 26);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("Wavetable", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Unison", 2, 26);
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
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                break;
            case 0x02:
                *displayCursor = 0x01;
                *displayStatus = DISPST_OSC_UNISON;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        if(*selectedOsc == 0x01) *displayCursor = 0x01;
        else if(*selectedOsc == 0x02) *displayCursor = 0x02;
        *displayStatus = DISPST_PRESET_EDIT;
    }
};

#endif // UIOSC_H