#include <IUIHandler.h>

#ifndef UIEFFECTDELAY_H
#define UIEFFECTDELAY_H

class UIEffectDelay : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    int16_t* delay_time;
    int16_t* delay_level;
    int16_t* delay_feedback;
    uint8_t* delay_status;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    void updateDelay() {
        pSynth->setDelay(0xff, *delay_status, *delay_time, *delay_level, *delay_feedback);
    }

public:
    UIEffectDelay(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor,
        int16_t* delay_time, int16_t* delay_level, int16_t* delay_feedback, uint8_t* delay_status)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->delay_time = delay_time;
        this->delay_level = delay_level;
        this->delay_feedback = delay_feedback;
        this->delay_status = delay_status;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Delay Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Delay
        String mode = "Disable";
        if(*delay_status == 0x01) mode = "Enable";

        char tm_chr[5]; sprintf(tm_chr, "%d", *delay_time);
        char lv_chr[6]; sprintf(lv_chr, "%.1f", (float)*delay_level / 10.0f);
        char fb_chr[6]; sprintf(fb_chr, "%.1f", (float)*delay_feedback / 10.0f);
        pSprite->drawString("Mode    : " + mode, 2, 16);
        pSprite->drawString("Time    : " + String(tm_chr) + "ms", 2, 26);
        pSprite->drawString("Level   : " + String(lv_chr) + "%", 2, 36);
        pSprite->drawString("Feedback: " + String(fb_chr) + "%", 2, 46);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("Mode", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Time", 2, 26);
        }
        else if(*displayCursor == 0x03) {
            cursorText("Level", 2, 36);
        }
        else if(*displayCursor == 0x04) {
            cursorText("Feedback", 2, 46);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x01) *displayCursor = 0x04;
                else (*displayCursor)--;
                break;
        }
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x04) *displayCursor = 0x01;
                else (*displayCursor)++;
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*delay_status == 0x00) *delay_status = 0x01;
                else *delay_status = 0x00;
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x02:
                if(*delay_time - 1 > 10) {
                    *delay_time -= 1; 
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x03:
                if(*delay_level - 1 > 0) {
                    *delay_level -= 1; 
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x04:
                if(*delay_feedback - 1 > 0) {
                    *delay_feedback -= 1; 
                }
                if(!longPush) {
                    updateDelay();
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*delay_status == 0x01) *delay_status = 0x00;
                else *delay_status = 0x01;
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x02:
                if(*delay_time + 1 <= 300) {
                    *delay_time += 1; 
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x03:
                if(*delay_level + 1 <= 1000) {
                    *delay_level += 1; 
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x04:
                if(*delay_feedback + 1 <= 900) {
                    *delay_feedback += 1; 
                }
                if(!longPush) {
                    updateDelay();
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
        *displayCursor = 0x01;
        *displayStatus = DISPST_EFFECT;
    }
};

#endif // UIEFFECTDELAY_H