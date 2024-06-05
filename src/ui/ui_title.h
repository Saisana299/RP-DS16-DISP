#include <IUIHandler.h>

#ifndef UITITLE_H
#define UITITLE_H

class UITitle : public IUIHandler {
private:
    TwoWire& ctrl = Wire1;

    // ボタン関連
    uint8_t* long_count_to_enter_debug_mode;

    // ディスプレイ関連
    uint8_t* displayStatus;

    LGFX_Sprite* pSprite;

    CtrlManager* pCtrl;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UITitle(
        LGFX_Sprite* pSprite, CtrlManager* pCtrl,
        uint8_t* displayStatus, uint8_t* long_count_to_enter_debug_mode)
    {
        this->displayStatus = displayStatus;
        this->pSprite = pSprite;
        this->pCtrl = pCtrl;
        this->long_count_to_enter_debug_mode = long_count_to_enter_debug_mode;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
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
        if(longPush) return;
        *displayStatus = DISPST_PRESETS;
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if(longPush) {
            if(*long_count_to_enter_debug_mode > 10) {
                uint8_t data[] = {CTRL_DEBUG_ON};
                uint8_t received[1];
                pCtrl->ctrlTransmission(data, sizeof(data), received, 1);

                if(received[0] != RES_OK) return;

                *displayStatus = DISPST_DEBUG;
                ctrl.end();
                ctrl.setSDA(CTRL_SDA_PIN);
                ctrl.setSCL(CTRL_SCL_PIN);
                ctrl.begin(CTRL_I2C_ADDR);
                ctrl.setClock(1000000);
                ctrl.onReceive(pCtrl->receiveWrapper);
                return;
            } else {
                (*long_count_to_enter_debug_mode)++;
            }
        } else {
            *long_count_to_enter_debug_mode = 0;
        }
    }
};

#endif // UITITLE_H