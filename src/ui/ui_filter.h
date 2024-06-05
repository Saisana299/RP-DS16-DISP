#include <IUIHandler.h>

#ifndef UIFILTER_H
#define UIFILTER_H

class UIFilter : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    uint8_t *filter_mode;
    float *lpf_freq, *lpf_q, *hpf_freq, *hpf_q;

    LGFX_Sprite* pSprite;

    SynthManager* pSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    void updateFilter() {
        if(*filter_mode == 0x00) {
            pSynth->setLowPassFilter(0xff, 0x00);
            pSynth->setHighPassFilter(0xff, 0x00);
        }
        else if(*filter_mode == 0x01) {
            pSynth->setLowPassFilter(0xff, 0x01, *lpf_freq, *lpf_q);
            pSynth->setHighPassFilter(0xff, 0x00);
        }
        else if(*filter_mode == 0x02) {
            pSynth->setLowPassFilter(0xff, 0x00);
            pSynth->setHighPassFilter(0xff, 0x01, *hpf_freq, *hpf_q);
        }
        else if(*filter_mode == 0x03) {
            pSynth->setLowPassFilter(0xff, 0x01, *lpf_freq, *lpf_q);
            pSynth->setHighPassFilter(0xff, 0x01, *hpf_freq, *hpf_q);
        }
    }

public:
    UIFilter(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor, uint8_t* filter_mode,
        float* lpf_freq, float* lpf_q, float* hpf_freq, float* hpf_q)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->filter_mode = filter_mode;
        this->lpf_freq = lpf_freq;
        this->lpf_q = lpf_q;
        this->hpf_freq = hpf_freq;
        this->hpf_q = hpf_q;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> Filter Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // Filter
        String mode = "Disable";
        if(*filter_mode == 0x01) mode = "LPF";
        else if(*filter_mode == 0x02) mode = "HPF";
        else if(*filter_mode == 0x03) mode = "LPF+HPF";
        char sym_2 = ':'; if (*displayCursor == 0x07) sym_2 = '>';
        char sym_3 = ':'; if (*displayCursor == 0x08) sym_3 = '>';
        char sym_4 = ':'; if (*displayCursor == 0x09) sym_4 = '>';
        char sym_5 = ':'; if (*displayCursor == 0x0A) sym_5 = '>';
        char lpff_chr[10]; sprintf(lpff_chr, "%.2f", *lpf_freq);
        char lpfq_chr[6]; sprintf(lpfq_chr, "%.2f", *lpf_q);
        char hpff_chr[10]; sprintf(hpff_chr, "%.2f", *hpf_freq);
        char hpfq_chr[6]; sprintf(hpfq_chr, "%.2f", *hpf_q);
        pSprite->drawString("Mode    : " + mode, 2, 16);
        pSprite->drawString("LPF Freq" + String(sym_2) + " " + String(lpff_chr) + "Hz", 2, 26);
        pSprite->drawString("LPF Q   " + String(sym_3) + " " + String(lpfq_chr), 2, 36);
        pSprite->drawString("HPF Freq" + String(sym_4) + " " + String(hpff_chr) + "Hz", 2, 46);
        pSprite->drawString("HPF Q   " + String(sym_5) + " " + String(hpfq_chr), 2, 56);

        // 塗り
        if(*displayCursor == 0x01 || *displayCursor == 0x06) {
            cursorText("Mode", 2, 16);
        }
        else if(*displayCursor == 0x02 || *displayCursor == 0x07) {
            cursorText("LPF Freq", 2, 26);
        }
        else if(*displayCursor == 0x03 || *displayCursor == 0x08) {
            cursorText("LPF Q", 2, 36);
        }
        else if(*displayCursor == 0x04 || *displayCursor == 0x09) {
            cursorText("HPF Freq", 2, 46);
        }
        else if(*displayCursor == 0x05 || *displayCursor == 0x0A) {
            cursorText("HPF Q", 2, 56);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x01) *displayCursor = 0x05;
                else (*displayCursor)--;
                break;
        }
        
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            default:
                if(*displayCursor == 0x05) *displayCursor = 0x01;
                else (*displayCursor)++;
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                if(*filter_mode == 0x00) *filter_mode = 0x03;
                else (*filter_mode)--;
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x02:
                if(*lpf_freq - 100 > 0) {
                    *lpf_freq -= 100; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x03:
                if(*lpf_q - 0.1 > 0) {
                    *lpf_q -= 0.1; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x04:
                if(*hpf_freq - 100 > 0) {
                    *hpf_freq -= 100; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;
            
            case 0x05:
                if(*hpf_q - 0.1 > 0) {
                    *hpf_q -= 0.1; 
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
                if(*filter_mode == 0x03) *filter_mode = 0x00;
                else (*filter_mode)++; 
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x02:
                if(*lpf_freq + 100 <= 20000) {
                    *lpf_freq += 100; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x03:
                if(*lpf_q + 0.1 <= 40) {
                    *lpf_q += 0.1; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;

            case 0x04:
                if(*hpf_freq + 100 <= 20000) {
                    *hpf_freq += 100; 
                }
                if(!longPush) {
                    updateFilter();
                }
                break;
            
            case 0x05:
                if(*hpf_q + 0.1 <= 40) {
                    *hpf_q += 0.1; 
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
            case 0x02: *displayCursor = 0x07;
                break;
            case 0x03: *displayCursor = 0x08;
                break;
            case 0x04: *displayCursor = 0x09;
                break;
            case 0x05: *displayCursor = 0x0A;
                break;

            case 0x07: *displayCursor = 0x02;
                break;
            case 0x08: *displayCursor = 0x03;
                break;
            case 0x09: *displayCursor = 0x04;
                break;
            case 0x0A: *displayCursor = 0x05;
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