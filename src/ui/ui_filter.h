#include <IUIHandler.h>

#ifndef UIFILTER_H
#define UIFILTER_H

class UIFilter : public IUIHandler {
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

    // フィルターは全シンセに適用する
    void updateFilter() {
        if(pSettings->filter_mode == 0x00) {
            pSynth->setLowPassFilter(*selectedSynth, 0x00);
            pSynth->setHighPassFilter(*selectedSynth, 0x00);
        }
        else if(pSettings->filter_mode == 0x01) {
            pSynth->setLowPassFilter(*selectedSynth, 0x01, pSettings->lpf_freq, pSettings->lpf_q);
            pSynth->setHighPassFilter(*selectedSynth, 0x00);
        }
        else if(pSettings->filter_mode == 0x02) {
            pSynth->setLowPassFilter(*selectedSynth, 0x00);
            pSynth->setHighPassFilter(*selectedSynth, 0x01, pSettings->hpf_freq, pSettings->hpf_q);
        }
        else if(pSettings->filter_mode == 0x03) {
            pSynth->setLowPassFilter(*selectedSynth, 0x01, pSettings->lpf_freq, pSettings->lpf_q);
            pSynth->setHighPassFilter(*selectedSynth, 0x01, pSettings->hpf_freq, pSettings->hpf_q);
        }
    }

public:
    UIFilter(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* selectedSynth, Settings* pSettings)
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
        pSprite->drawString("> Filter Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Filter
        if(*displayCursor < 0x06 || *displayCursor > 0x07) {
            String mode = "Disable";
            if(pSettings->filter_mode == 0x01) mode = "LPF";
            else if(pSettings->filter_mode == 0x02) mode = "HPF";
            else if(pSettings->filter_mode == 0x03) mode = "LPF+HPF";
            char sym_2 = ':'; if (*displayCursor == 0x09) sym_2 = '>';
            char sym_3 = ':'; if (*displayCursor == 0x0A) sym_3 = '>';
            char sym_4 = ':'; if (*displayCursor == 0x0B) sym_4 = '>';
            char sym_5 = ':'; if (*displayCursor == 0x0C) sym_5 = '>';
            char lpff_chr[10]; sprintf(lpff_chr, "%.2f", pSettings->lpf_freq);
            char lpfq_chr[6]; sprintf(lpfq_chr, "%.2f", pSettings->lpf_q);
            char hpff_chr[10]; sprintf(hpff_chr, "%.2f", pSettings->hpf_freq);
            char hpfq_chr[6]; sprintf(hpfq_chr, "%.2f", pSettings->hpf_q);
            pSprite->drawString("Mode    : " + mode, 2, 16);
            pSprite->drawString("LPF Freq" + String(sym_2) + " " + String(lpff_chr) + "Hz", 2, 26);
            pSprite->drawString("LPF Q   " + String(sym_3) + " " + String(lpfq_chr), 2, 36);
            pSprite->drawString("HPF Freq" + String(sym_4) + " " + String(hpff_chr) + "Hz", 2, 46);
            pSprite->drawString("HPF Q   " + String(sym_5) + " " + String(hpfq_chr), 2, 56);
        } else {
            pSprite->drawString("Envelope", 2, 16);
            pSprite->drawString("LFO", 2, 26);
        }

        // 塗り
        if(*displayCursor == 0x01 || *displayCursor == 0x08) {
            cursorText("Mode", 2, 16);
        }
        else if(*displayCursor == 0x02 || *displayCursor == 0x09) {
            cursorText("LPF Freq", 2, 26);
        }
        else if(*displayCursor == 0x03 || *displayCursor == 0x0A) {
            cursorText("LPF Q", 2, 36);
        }
        else if(*displayCursor == 0x04 || *displayCursor == 0x0B) {
            cursorText("HPF Freq", 2, 46);
        }
        else if(*displayCursor == 0x05 || *displayCursor == 0x0C) {
            cursorText("HPF Q", 2, 56);
        }
        else if(*displayCursor == 0x06) {
            cursorText("Envelope", 2, 16);
        }
        else if(*displayCursor == 0x07) {
            cursorText("LFO", 2, 26);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x01) *displayCursor = 0x07;
                else if(*displayCursor == 0x09) *displayCursor = 0x0C;
                else (*displayCursor)--;
                break;
        }
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x07) *displayCursor = 0x01;
                else if(*displayCursor == 0x0C) *displayCursor = 0x09;
                else (*displayCursor)++;
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(pSettings->filter_mode == 0x00) pSettings->filter_mode = 0x03;
                else (pSettings->filter_mode)--;
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x02:
                if(pSettings->lpf_freq - 100 > 0) {
                    pSettings->lpf_freq -= 100;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x03:
                if(pSettings->lpf_q - 0.1 > 0) {
                    pSettings->lpf_q -= 0.1;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x04:
                if(pSettings->hpf_freq - 100 > 0) {
                    pSettings->hpf_freq -= 100;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x05:
                if(pSettings->hpf_q - 0.1 > 0) {
                    pSettings->hpf_q -= 0.1;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(pSettings->filter_mode == 0x03) pSettings->filter_mode = 0x00;
                else (pSettings->filter_mode)++;
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x02:
                if(pSettings->lpf_freq + 100 <= 20000) {
                    pSettings->lpf_freq += 100;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x03:
                if(pSettings->lpf_q + 0.1 <= 40) {
                    pSettings->lpf_q += 0.1;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x04:
                if(pSettings->hpf_freq + 100 <= 20000) {
                    pSettings->hpf_freq += 100;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x05:
                if(pSettings->hpf_q + 0.1 <= 40) {
                    pSettings->hpf_q += 0.1;
                }
                if(!longPush) {
                    updateFilter();
                }
                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x02: *displayCursor = 0x09;
                break;
            case 0x03: *displayCursor = 0x0A;
                break;
            case 0x04: *displayCursor = 0x0B;
                break;
            case 0x05: *displayCursor = 0x0C;
                break;

            case 0x09: *displayCursor = 0x02;
                break;
            case 0x0A: *displayCursor = 0x03;
                break;
            case 0x0B: *displayCursor = 0x04;
                break;
            case 0x0C: *displayCursor = 0x05;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x03;
        *displayStatus = DISPST_PRESET_EDIT;
    }
};

#endif // UIFILTER_H