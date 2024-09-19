#include <IUIHandler.h>

#ifndef UIAMPADSR_H
#define UIAMPADSR_H

class UIAmpAdsr : public IUIHandler {
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

public:
    UIAmpAdsr(
        uint8_t* displayStatus, uint8_t* displayCursor,
        LGFX_Sprite* pSprite, SynthManager* pSynth, uint8_t* selectedSynth, Settings* pSettings)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->pSettings = pSettings;
        this->selectedSynth = selectedSynth;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Level Env. Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // ADSR
        char a_sym = ':'; if (*displayCursor == 0x05) a_sym = '>';
        char d_sym = ':'; if (*displayCursor == 0x06) d_sym = '>';
        char s_sym = ':'; if (*displayCursor == 0x07) s_sym = '>';
        char r_sym = ':'; if (*displayCursor == 0x08) r_sym = '>';
        char a_chr[6]; sprintf(a_chr, "%d", pSettings->attack);
        char d_chr[6]; sprintf(d_chr, "%d", pSettings->decay);
        char s_chr[7]; sprintf(s_chr, "%.1f", pSettings->sustain / 10.0f);
        char r_chr[6]; sprintf(r_chr, "%d", pSettings->release);
        pSprite->drawString("Attack " + String(a_sym) + " " + String(a_chr) + "ms", 2, 16);
        pSprite->drawString("Decay  " + String(d_sym) + " " + String(d_chr) + "ms", 2, 26);
        pSprite->drawString("Sustain" + String(s_sym) + " " + String(s_chr) + "%" , 2, 36);
        pSprite->drawString("Release" + String(r_sym) + " " + String(r_chr) + "ms", 2, 46);

        // 塗り
        if(*displayCursor == 0x01 || *displayCursor == 0x05) {
            cursorText("Attack", 2, 16);
        }
        else if(*displayCursor == 0x02 || *displayCursor == 0x06) {
            cursorText("Decay", 2, 26);
        }
        else if(*displayCursor == 0x03 || *displayCursor == 0x07) {
            cursorText("Sustain", 2, 36);
        }
        else if(*displayCursor == 0x04 || *displayCursor == 0x08) {
            cursorText("Release", 2, 46);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            // 100倍
            case 0x05:
                if (pSettings->attack + 100 <= 32000) {
                    pSettings->attack += 100; pSynth->setAttack(*selectedSynth, pSettings->attack);
                }
                break;
            case 0x06:
                if (pSettings->decay + 100 <= 32000) {
                    pSettings->decay += 100; pSynth->setDecay(*selectedSynth, pSettings->decay);
                }
                break;
            case 0x07:
                if (pSettings->sustain + 100 <= 1000) {
                    pSettings->sustain += 100; pSynth->setSustain(*selectedSynth, pSettings->sustain);
                }
                break;
            case 0x08:
                if (pSettings->release + 100 <= 32000) {
                    pSettings->release += 100; pSynth->setRelease(*selectedSynth, pSettings->release);
                }
                break;
            // 通常
            default:
                *displayCursor = (*displayCursor == 0x01) ? 0x04 : (*displayCursor - 0x01);
                break;
        }
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            // 100倍
            case 0x05:
                if (pSettings->attack - 100 >= 0) {
                    pSettings->attack -= 100; pSynth->setAttack(*selectedSynth, pSettings->attack);
                }
                break;
            case 0x06:
                if (pSettings->decay - 100 >= 0) {
                    pSettings->decay -= 100; pSynth->setDecay(*selectedSynth, pSettings->decay);
                }
                break;
            case 0x07:
                if (pSettings->sustain - 100 >= 0) {
                    pSettings->sustain -= 100; pSynth->setSustain(*selectedSynth, pSettings->sustain);
                }
                break;
            case 0x08:
                if (pSettings->release - 100 >= 0) {
                    pSettings->release -= 100; pSynth->setRelease(*selectedSynth, pSettings->release);
                }
                break;
            // 通常
            default:
                *displayCursor = (*displayCursor == 0x04) ? 0x01 : (*displayCursor + 0x01);
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            // 通常
            case 0x01:
                if (pSettings->attack - 1 >= 0) {
                    pSettings->attack -= 1;
                }
                if(!longPush) pSynth->setAttack(*selectedSynth, pSettings->attack);

                break;
            case 0x02:
                if (pSettings->decay - 1 >= 0) {
                    pSettings->decay -= 1;
                }
                if(!longPush) pSynth->setDecay(*selectedSynth, pSettings->decay);

                break;
            case 0x03:
                if (pSettings->sustain - 1 >= 0) {
                    pSettings->sustain -= 1;
                }
                if(!longPush) pSynth->setSustain(*selectedSynth, pSettings->sustain);

                break;
            case 0x04:
                if (pSettings->release - 1 >= 0) {
                    pSettings->release -= 1;
                }
                if(!longPush) pSynth->setRelease(*selectedSynth, pSettings->release);

                break;

            // 10倍
            case 0x05:
                if (pSettings->attack - 10 >= 0) {
                    pSettings->attack -= 10;
                }
                if(!longPush) pSynth->setAttack(*selectedSynth, pSettings->attack);

                break;
            case 0x06:
                if (pSettings->decay - 10 >= 0) {
                    pSettings->decay -= 10;
                }
                if(!longPush) pSynth->setDecay(*selectedSynth, pSettings->decay);

                break;
            case 0x07:
                if (pSettings->sustain - 10 >= 0) {
                    pSettings->sustain -= 10;
                }
                if(!longPush) pSynth->setSustain(*selectedSynth, pSettings->sustain);

                break;
            case 0x08:
                if (pSettings->release - 10 >= 0) {
                    pSettings->release -= 10;
                }
                if(!longPush) pSynth->setRelease(*selectedSynth, pSettings->release);

                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            // 通常
            case 0x01:
                if (pSettings->attack + 1 <= 32000) {
                    pSettings->attack += 1;
                }
                if(!longPush) pSynth->setAttack(*selectedSynth, pSettings->attack);

                break;
            case 0x02:
                if (pSettings->decay + 1 <= 32000) {
                    pSettings->decay += 1;
                }
                if(!longPush) pSynth->setDecay(*selectedSynth, pSettings->decay);

                break;
            case 0x03:
                if (pSettings->sustain + 1 <= 1000) {
                    pSettings->sustain += 1;
                }
                if(!longPush) pSynth->setSustain(*selectedSynth, pSettings->sustain);

                break;
            case 0x04:
                if (pSettings->release + 1 <= 32000) {
                    pSettings->release += 1;
                }
                if(!longPush) pSynth->setRelease(*selectedSynth, pSettings->release);

                break;

            // 10倍
            case 0x05:
                if (pSettings->attack + 10 <= 32000) {
                    pSettings->attack += 10;
                }
                if(!longPush) pSynth->setAttack(*selectedSynth, pSettings->attack);

                break;
            case 0x06:
                if (pSettings->decay + 10 <= 32000) {
                    pSettings->decay += 10;
                }
                if(!longPush) pSynth->setDecay(*selectedSynth, pSettings->decay);

                break;
            case 0x07:
                if (pSettings->sustain + 10 <= 1000) {
                    pSettings->sustain += 10;
                }
                if(!longPush) pSynth->setSustain(*selectedSynth, pSettings->sustain);

                break;
            case 0x08:
                if (pSettings->release + 10 <= 32000) {
                    pSettings->release += 10;
                }
                if(!longPush) pSynth->setRelease(*selectedSynth, pSettings->release);

                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01: *displayCursor = 0x05;
                break;
            case 0x02: *displayCursor = 0x06;
                break;
            case 0x03: *displayCursor = 0x07;
                break;
            case 0x04: *displayCursor = 0x08;
                break;

            case 0x05: *displayCursor = 0x01;
                break;
            case 0x06: *displayCursor = 0x02;
                break;
            case 0x07: *displayCursor = 0x03;
                break;
            case 0x08: *displayCursor = 0x04;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x01;
        *displayStatus = DISPST_AMP;
    }
};

#endif // UIAMPADSR_H