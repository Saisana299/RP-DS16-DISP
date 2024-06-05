#include <IUIHandler.h>

#ifndef UIOSCWAVE_H
#define UIOSCWAVE_H

class UIOscWave : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    String* default_wavetables;
    Preset* user_wavetables;

    uint8_t* selectedOsc;

    int16_t selectedWave = -1;

    LGFX_Sprite* pSprite;

    SynthManager* pSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIOscWave(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* selectedOsc, String* default_wavetables, Preset* user_wavetables)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedOsc = selectedOsc;
        this->default_wavetables = default_wavetables;
        this->user_wavetables = user_wavetables;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        if(*selectedOsc == 0x01) pSprite->drawString("> OSC1 Wavetable", 2, 2);
        else if(*selectedOsc == 0x02) pSprite->drawString("> OSC2 Wavetable", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
        pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

        // wavetable
        uint8_t wave_x = pSprite->textWidth(" ");
        uint8_t wave_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
        char idstr[5]; 
        String fu = "F";
        String wave_name;

        if(selectedWave == -1) {
            sprintf(idstr, "%03d ", selectedWave+1);
            wave_name = "nullptr";
        }
        else if(selectedWave < FACTORY_WAVETABLES) {
            sprintf(idstr, "%03d ", selectedWave+1);
            wave_name = default_wavetables[selectedWave];
        }
        else {
            fu = "U";
            sprintf(idstr, "%03d ", selectedWave+1 - FACTORY_WAVETABLES);
            wave_name = user_wavetables[selectedWave - FACTORY_WAVETABLES].name;
        }

        pSprite->drawString(fu + idstr + wave_name, wave_x, wave_y);

        // 塗り
        if(*displayCursor == 0x01) {

            uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[6];

            String fu = "F";
            if(selectedWave < FACTORY_WAVETABLES) {
                sprintf(idstr, "%03d", selectedWave+1);
            }
            else {
                fu = "U";
                sprintf(idstr, "%03d", selectedWave+1 - FACTORY_WAVETABLES);
            }
            cursorText(" " + fu + idstr, 0, y);
            
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                selectedWave = (selectedWave != -1) ? (selectedWave - 1) : 128 + FACTORY_WAVETABLES - 1;
                if(!longPush) {
                    // set
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                selectedWave = (selectedWave != 128 + FACTORY_WAVETABLES - 1) ? (selectedWave + 1) : -1;
                if(!longPush) {
                    // set
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
        *displayCursor = 0x04;
        *displayStatus = DISPST_OSC;
    }
};

#endif // UIOSCWAVE_H