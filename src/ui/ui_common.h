#include <IUIHandler.h>

#ifndef UICOMMON_H
#define UICOMMON_H

class UICommon : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UICommon( LGFX_Sprite* pSprite, uint8_t* displayStatus, uint8_t* displayCursor) {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Common Settings", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        pSprite->drawString("ADSR Envelope", 2, 16);
        pSprite->drawString("Portamento", 2, 26);
        pSprite->drawString("Level: ---", 2, 36);
        pSprite->drawString("Pan: ---", 2, 46);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("ADSR Envelope", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Portamento", 2, 26);
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
                *displayCursor = 0x01;
                *displayStatus = DISPST_ADSR;
                break;
            case 0x02:
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

#endif // UICOMMON_H