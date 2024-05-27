#include <IUIHandler.h>

#ifndef UIDEBUG_H
#define UIDEBUG_H

class UIDebug : public IUIHandler {
private:
    LGFX_Sprite* pSprite;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIDebug(LGFX_Sprite* pSprite) {
        this->pSprite = pSprite;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("Debug Mode", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // シンセモード
        uint8_t synth_x = pSprite->textWidth("MIDI-1.0");
        pSprite->drawString("MIDI-1.0", 128 - 2 - synth_x, 2);

        // データ表示部
        pSprite->drawString("----", 2, 16);
        pSprite->drawString(" ---- ---- ----", 2, 26);
        pSprite->drawString("(Waiting data input)", 2, 36);
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
    }
};

#endif // UIDEBUG_H