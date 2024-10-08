#include <IUIHandler.h>
#include <synth_manager.h>

#ifndef UIAMPGLIDE_H
#define UIAMPGLIDE_H

class UIAmpGlide : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    Settings* pSettings;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIAmpGlide(LGFXRP2040* pDisplay, LGFX_Sprite* pSprite, SynthManager* pSynth, uint8_t* displayStatus, uint8_t* displayCursor, Settings* pSettings) {
        this->pDisplay = pDisplay;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->pSettings = pSettings;
    }

    /** @brief 画面更新 */
    void refreshUI() override {

        // タイトル
        pSprite->drawString("> Glide Settings", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        char gt_chr[5]; sprintf(gt_chr, "%d", pSettings->glide_time);

        if(pSettings->isGlide) {
            pSprite->drawString("Glide: Enable", 2, 16);
        } else {
            pSprite->drawString("Glide: Disable", 2, 16);
        }

        pSprite->drawString("Time : "+ String(gt_chr) +"ms", 2, 26);

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText("Glide", 2, 16);
        }
        else if(*displayCursor == 0x02) {
            cursorText("Time", 2, 26);
        }

        if(popup_message != "") {
            pDisplay->popupMessage(pSprite, popup_message);
            popup_message = "";
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
        if(longPush) return;
        if(*displayCursor == 0x01) {
            if(pSettings->isGlide) {
                pSettings->isGlide = false;
                pSynth->setGlideMode(0xff, false); // glideは必ず全てのシンセに送信する
            }
            else {
                if(pSettings->synthMode == SYNTH_MONO) {
                    pSettings->isGlide = true;
                    pSynth->setGlideMode(0xff, true, pSettings->glide_time);
                }else{
                    popup_message = "Only MONO mode";
                }
            }
        }
        else if(*displayCursor == 0x02) {
            if(pSettings->glide_time - 10 >= 1) pSettings->glide_time -= 10;
            if(pSettings->isGlide) {
                pSynth->setGlideMode(0xff, true, pSettings->glide_time);
            }
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(longPush) return;
        if(*displayCursor == 0x01) {
            if(pSettings->isGlide) {
                pSettings->isGlide = false;
                pSynth->setGlideMode(0xff, false); // glideは必ず全てのシンセに送信する
            }
            else {
                if(pSettings->synthMode == SYNTH_MONO) {
                    pSettings->isGlide = true;
                    pSynth->setGlideMode(0xff, true);
                }else{
                    popup_message = "Only MONO mode";
                }
            }
        }
        else if(*displayCursor == 0x02) {
            if(pSettings->glide_time + 10 <= 3000) pSettings->glide_time += 10;
            if(pSettings->isGlide) {
                pSynth->setGlideMode(0xff, true, pSettings->glide_time);
            }
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x02;
        *displayStatus = DISPST_AMP;
    }
};

#endif // UIAMPGLIDE_H