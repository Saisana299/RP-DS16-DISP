#include <IUIHandler.h>

#ifndef UIEFFECTDELAY_H
#define UIEFFECTDELAY_H

class UIEffectDelay : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;
    uint8_t* selectedSynth;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    Settings* pSettings;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    void updateDelay() {
        pSynth->setDelay(*selectedSynth, pSettings->delay_status, pSettings->delay_time, pSettings->delay_level, pSettings->delay_feedback);
    }

public:
    UIEffectDelay(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* selectedSynth, Settings* pSettings)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedSynth = selectedSynth;
        this->pSettings = pSettings;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Delay Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Delay
        String mode = "Disable";
        if(pSettings->delay_status == 0x01) mode = "Enable";

        char tm_chr[5]; sprintf(tm_chr, "%d", pSettings->delay_time);
        char lv_chr[6]; sprintf(lv_chr, "%.1f", (float)pSettings->delay_level / 10.0f);
        char fb_chr[6]; sprintf(fb_chr, "%.1f", (float)pSettings->delay_feedback / 10.0f);
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
                if(pSettings->delay_status == 0x00) pSettings->delay_status = 0x01;
                else pSettings->delay_status = 0x00;
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x02:
                if(pSettings->delay_time - 1 > 10) {
                    pSettings->delay_time -= 1;
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x03:
                if(pSettings->delay_level - 1 > 0) {
                    pSettings->delay_level -= 1;
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x04:
                if(pSettings->delay_feedback - 1 > 0) {
                    pSettings->delay_feedback -= 1;
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
                if(pSettings->delay_status == 0x01) pSettings->delay_status = 0x00;
                else pSettings->delay_status = 0x01;
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x02:
                if(pSettings->delay_time + 1 <= 300) {
                    pSettings->delay_time += 1;
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x03:
                if(pSettings->delay_level + 1 <= 1000) {
                    pSettings->delay_level += 1;
                }
                if(!longPush) {
                    updateDelay();
                }
                break;

            case 0x04:
                if(pSettings->delay_feedback + 1 <= 900) {
                    pSettings->delay_feedback += 1;
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