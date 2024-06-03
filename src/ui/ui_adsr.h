#include <IUIHandler.h>

#ifndef UIADSR_H
#define UIADSR_H

class UIAdsr : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    int16_t* attack;
    int16_t* decay;
    int16_t* sustain;
    int16_t* release;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIAdsr(
        uint8_t* displayStatus, uint8_t* displayCursor,
        int16_t* attack, int16_t* decay, int16_t* sustain, int16_t* release,
        LGFX_Sprite* pSprite, SynthManager* pSynth)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release = release;
        this->pSprite = pSprite;
        this->pSynth = pSynth;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> ADSR Editor", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // ADSR
        char a_sym = ':'; if (*displayCursor == 0x05) a_sym = '>';
        char d_sym = ':'; if (*displayCursor == 0x06) d_sym = '>';
        char s_sym = ':'; if (*displayCursor == 0x07) s_sym = '>';
        char r_sym = ':'; if (*displayCursor == 0x08) r_sym = '>';
        char a_chr[6]; sprintf(a_chr, "%d", *attack);
        char d_chr[6]; sprintf(d_chr, "%d", *decay);
        char s_chr[7]; sprintf(s_chr, "%.1f", *sustain / 10.0f);
        char r_chr[6]; sprintf(r_chr, "%d", *release);
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
                if (*attack + 100 <= 32000) {
                    *attack += 100; pSynth->setAttack(0xff, *attack);
                }
                break;
            case 0x06:
                if (*decay + 100 <= 32000) {
                    *decay += 100; pSynth->setDecay(0xff, *decay);
                }
                break;
            case 0x07:
                if (*sustain + 100 <= 1000) {
                    *sustain += 100; pSynth->setSustain(0xff, *sustain); 
                }
                break;
            case 0x08:
                if (*release + 100 <= 32000) {
                    *release += 100; pSynth->setRelease(0xff, *release);  
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
                if (*attack - 100 >= 0) {
                    *attack -= 100; pSynth->setAttack(0xff, *attack);
                }
                break;
            case 0x06:
                if (*decay - 100 >= 0) {
                    *decay -= 100; pSynth->setDecay(0xff, *decay);
                }
                break;
            case 0x07:
                if (*sustain - 100 >= 0) {
                    *sustain -= 100; pSynth->setSustain(0xff, *sustain);
                }
                break;
            case 0x08:
                if (*release - 100 >= 0) {
                    *release -= 100; pSynth->setRelease(0xff, *release);
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
                if (*attack - 1 >= 0) {
                    *attack -= 1; 
                }
                if(!longPush) pSynth->setAttack(0xff, *attack);

                break;
            case 0x02:
                if (*decay - 1 >= 0) {
                    *decay -= 1; 
                }
                if(!longPush) pSynth->setDecay(0xff, *decay);

                break;
            case 0x03:
                if (*sustain - 1 >= 0) {
                    *sustain -= 1; 
                }
                if(!longPush) pSynth->setSustain(0xff, *sustain);

                break;
            case 0x04:
                if (*release - 1 >= 0) {
                    *release -= 1; 
                }
                if(!longPush) pSynth->setRelease(0xff, *release);

                break;

            // 10倍
            case 0x05:
                if (*attack - 10 >= 0) {
                    *attack -= 10; 
                }
                if(!longPush) pSynth->setAttack(0xff, *attack);

                break;
            case 0x06:
                if (*decay - 10 >= 0) {
                    *decay -= 10; 
                }
                if(!longPush) pSynth->setDecay(0xff, *decay);

                break;
            case 0x07:
                if (*sustain - 10 >= 0) {
                    *sustain -= 10; 
                }
                if(!longPush) pSynth->setSustain(0xff, *sustain);

                break;
            case 0x08:
                if (*release - 10 >= 0) {
                    *release -= 10; 
                }
                if(!longPush) pSynth->setRelease(0xff, *release);

                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            // 通常
            case 0x01:
                if (*attack + 1 <= 32000) {
                    *attack += 1; 
                }
                if(!longPush) pSynth->setAttack(0xff, *attack); 

                break;
            case 0x02:
                if (*decay + 1 <= 32000) {
                    *decay += 1; 
                }
                if(!longPush) pSynth->setDecay(0xff, *decay);

                break;
            case 0x03:
                if (*sustain + 1 <= 1000) {
                    *sustain += 1; 
                }
                if(!longPush) pSynth->setSustain(0xff, *sustain); 

                break;
            case 0x04:
                if (*release + 1 <= 32000) {
                    *release += 1; 
                }
                if(!longPush) pSynth->setRelease(0xff, *release);  

                break;

            // 10倍
            case 0x05:
                if (*attack + 10 <= 32000) {
                    *attack += 10; 
                }
                if(!longPush) pSynth->setAttack(0xff, *attack); 

                break;
            case 0x06:
                if (*decay + 10 <= 32000) {
                    *decay += 10; 
                }
                if(!longPush) pSynth->setDecay(0xff, *decay);

                break;
            case 0x07:
                if (*sustain + 10 <= 1000) {
                    *sustain += 10; 
                }
                if(!longPush) pSynth->setSustain(0xff, *sustain); 

                break;
            case 0x08:
                if (*release + 10 <= 32000) {
                    *release += 10; 
                }
                if(!longPush) pSynth->setRelease(0xff, *release);

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
        *displayStatus = DISPST_COMMON;
    }
};

#endif // UIADSR_H