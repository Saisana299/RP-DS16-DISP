#include <IUIHandler.h>

#ifndef UIMIDIPLAYER_H
#define UIMIDIPLAYER_H

class UIMidiPlayer : public IUIHandler {
private:
    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;
    CtrlManager* pCtrl;
    MidiManager* pMidi;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIMidiPlayer(LGFX_Sprite* pSprite, CtrlManager* pCtrl, uint8_t* displayStatus, uint8_t* displayCursor, MidiManager* pMidi) {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pCtrl = pCtrl;
        this->pMidi = pMidi;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        pMidi->pauseMidi();
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        pMidi->resumeMidi();
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(longPush) return;
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(longPush) return;
        pMidi->stopMidi();
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        pMidi->playMidi("test.mid");
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x03;
        *displayStatus = DISPST_MENU;
    }
};

#endif // UIMIDIPLAYER_H