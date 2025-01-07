#include <IUIHandler.h>

#ifndef UIOSCWAVE_H
#define UIOSCWAVE_H

class UIOscWave : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;
    uint8_t* selectedSynth;

    uint8_t osc1_wave_id;
    uint8_t osc2_wave_id;
    uint8_t osc_sub_wave_id;

    uint8_t* selectedOsc;

    LGFX_Sprite* pSprite;
    SynthManager* pSynth;

    Settings* pSettings;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    bool canSetVoice(uint8_t osc, uint8_t voice, uint8_t wave_id) {
        if(osc == 0x01) {
            uint8_t sum = 0;
            if(wave_id != 0xff) sum += voice;
            if (pSettings->selectedWave2 != 0xff) sum += pSettings->osc2_voice;
            if (pSettings->selectedWaveSub != 0xff) sum += 1;

            return sum <= MAX_VOICE;
        }
        else if(osc == 0x02) {
            uint8_t sum = 0;
            if(wave_id != 0xff) sum += voice;
            if (pSettings->selectedWave != 0xff) sum += pSettings->osc1_voice;
            if(pSettings->selectedWaveSub != 0xff) sum += 1;

            return sum <= MAX_VOICE;
        }
        else if(osc == 0x03) {
            uint8_t sum = 0;
            if(wave_id != 0xff) sum += voice;
            if (pSettings->selectedWave != 0xff) sum += pSettings->osc1_voice;
            if(pSettings->selectedWave2 != 0xff) sum += pSettings->osc2_voice;

            return sum <= MAX_VOICE;
        }

        return false;
    }

public:
    UIOscWave(
        LGFX_Sprite* pSprite, SynthManager* pSynth,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* selectedOsc, uint8_t* selectedSynth, Settings* pSettings)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedOsc = selectedOsc;

        this->selectedSynth = selectedSynth;

        this->pSettings = pSettings;

        osc1_wave_id = pSettings->selectedWave;
        osc2_wave_id = pSettings->selectedWave2;
        osc_sub_wave_id = pSettings->selectedWaveSub;
    }

    /** @brief 画面更新 */
    void refreshUI() override {

        // 位置更新
        if(pSettings->isFirst) {
            if(pSettings->selectedWave < FACTORY_WAVETABLES || pSettings->selectedWave == 0xff) {
                osc1_wave_id = pSettings->selectedWave;
            }
            else osc1_wave_id = 0xfe;

            if(pSettings->selectedWave2 < FACTORY_WAVETABLES || pSettings->selectedWave2 == 0xff) {
                osc2_wave_id = pSettings->selectedWave2;
            }
            else osc2_wave_id = 0xfe;

            if(pSettings->selectedWaveSub < FACTORY_WAVETABLES || pSettings->selectedWaveSub == 0xff) {
                osc_sub_wave_id = pSettings->selectedWaveSub;
            }
            else osc_sub_wave_id = 0xfe;

            pSettings->isFirst = false;
        }

        // タイトル
        if(*selectedOsc == 0x01) pSprite->drawString("> OSC1 Wavetable", 2, 2);
        else if(*selectedOsc == 0x02) pSprite->drawString("> OSC2 Wavetable", 2, 2);
        else if(*selectedOsc == 0x03) pSprite->drawString("> Sub Wavetable", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
        pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

        // ID
        uint8_t wave_id = 0xff;
        if(*selectedOsc == 0x01) wave_id = osc1_wave_id;
        else if(*selectedOsc == 0x02) wave_id = osc2_wave_id;
        else if(*selectedOsc == 0x03) wave_id = osc_sub_wave_id;

        // wavetable
        uint8_t wave_x = pSprite->textWidth(" ");
        uint8_t wave_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;

        // ユーザーテーブルならクッション
        if(wave_id == 0xfe) {
            pSprite->drawString("U??? User Preset", wave_x, wave_y);

            if(*displayCursor == 0x01) {
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                cursorText(" U???", 0, y);
            }
        }
        else {
            char idstr[5];
            String fu = "F";
            String wave_name;

            if(wave_id == 0xff) {
                sprintf(idstr, "%03d ", 0);
                wave_name = "null";
            }
            else if(wave_id < FACTORY_WAVETABLES) {
                sprintf(idstr, "%03d ", wave_id+1);
                wave_name = pSettings->default_wavetables[wave_id];
            }
            else {
                fu = "U";
                sprintf(idstr, "%03d ", wave_id+1 - FACTORY_WAVETABLES);
                wave_name = pSettings->user_wavetables[wave_id - FACTORY_WAVETABLES].name;
            }
            pSprite->drawString(fu + idstr + wave_name, wave_x, wave_y);

            // 塗り
            if(*displayCursor == 0x01) {
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr[6];

                String fu = "F";
                if(wave_id < FACTORY_WAVETABLES || wave_id == 0xff) {
                    sprintf(idstr, "%03d", wave_id+1);
                }
                else {
                    fu = "U";
                    sprintf(idstr, "%03d", wave_id+1 - FACTORY_WAVETABLES);
                }
                cursorText(" " + fu + idstr, 0, y);
            }
        }

        // 下部テキスト
        if(*selectedOsc == 0x01)
        {
            if(pSettings->selectedWave == 0xff) pSprite->drawString("Wave is null", 2, 56);

            else if(pSettings->selectedWave < FACTORY_WAVETABLES)
                pSprite->drawString("Now=" + String((pSettings->selectedWave) + 1) + ":" + pSettings->default_wavetables[pSettings->selectedWave], 2, 56);

            else pSprite->drawString("User Wavetable", 2, 56);
        }
        else if(*selectedOsc == 0x02)
        {
            if(pSettings->selectedWave2 == 0xff) pSprite->drawString("Wave is null", 2, 56);

            else if(pSettings->selectedWave2 < FACTORY_WAVETABLES)
                pSprite->drawString("Now=" + String((pSettings->selectedWave2)+1) + ":" + pSettings->default_wavetables[pSettings->selectedWave2], 2, 56);

            else pSprite->drawString("User Wavetable", 2, 56);
        }
        else if(*selectedOsc == 0x03)
        {
            if(pSettings->selectedWaveSub == 0xff) pSprite->drawString("Wave is null", 2, 56);

            else if(pSettings->selectedWaveSub < FACTORY_WAVETABLES)
                pSprite->drawString("Now=" + String((pSettings->selectedWaveSub)+1) + ":" + pSettings->default_wavetables[pSettings->selectedWaveSub], 2, 56);

            else pSprite->drawString("User Wavetable", 2, 56);
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
        uint8_t wave_id = 0xff;
        if(*selectedOsc == 0x01) wave_id = osc1_wave_id;
        else if(*selectedOsc == 0x02) wave_id = osc2_wave_id;
        else if(*selectedOsc == 0x03) wave_id = osc_sub_wave_id;

        switch (*displayCursor) {
            case 0x01:
                if(wave_id == 0xfe) wave_id = FACTORY_WAVETABLES;
                else wave_id = (wave_id != 0xff) ? (wave_id - 1) : 128 + FACTORY_WAVETABLES - 1;
                if(*selectedOsc == 0x01) {
                    osc1_wave_id = wave_id;
                }
                else if(*selectedOsc == 0x02) {
                    osc2_wave_id = wave_id;
                }
                else if(*selectedOsc == 0x03) {
                    osc_sub_wave_id = wave_id;
                }
                if(!longPush) {
                    if(wave_id < FACTORY_WAVETABLES || wave_id == 0xff) {
                        if(canSetVoice(*selectedOsc, 1, wave_id)) {
                            pSynth->setShape(*selectedSynth, *selectedOsc, wave_id);
                            if(*selectedOsc == 0x01) {
                                pSettings->selectedWave = wave_id;
                                if(wave_id == 0xff) pSettings->osc1_voice = 1;
                            }
                            else if(*selectedOsc == 0x02) {
                                pSettings->selectedWave2 = wave_id;
                                if(wave_id == 0xff) pSettings->osc2_voice = 1;
                            }
                            else if(*selectedOsc == 0x03) {
                                pSettings->selectedWaveSub = wave_id;
                            }
                        }
                    }
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        uint8_t wave_id = 0xff;
        if(*selectedOsc == 0x01) wave_id = osc1_wave_id;
        else if(*selectedOsc == 0x02) wave_id = osc2_wave_id;
        else if(*selectedOsc == 0x03) wave_id = osc_sub_wave_id;

        switch (*displayCursor) {
            case 0x01:
                if(wave_id == 0xfe) wave_id = FACTORY_WAVETABLES;
                else wave_id = (wave_id != 128 + FACTORY_WAVETABLES - 1) ? (wave_id + 1) : 0xff;
                if(*selectedOsc == 0x01) {
                    osc1_wave_id = wave_id;
                }
                else if(*selectedOsc == 0x02) {
                    osc2_wave_id = wave_id;
                }
                else if(*selectedOsc == 0x03) {
                    osc_sub_wave_id = wave_id;
                }
                if(!longPush) {
                    if(wave_id < FACTORY_WAVETABLES || wave_id == 0xff) {
                        if(canSetVoice(*selectedOsc, 1, wave_id)) {
                            pSynth->setShape(*selectedSynth, *selectedOsc, wave_id);
                            if(*selectedOsc == 0x01) {
                                pSettings->selectedWave = wave_id;
                                if(wave_id == 0xff) pSettings->osc1_voice = 1;
                            }
                            else if(*selectedOsc == 0x02) {
                                pSettings->selectedWave2 = wave_id;
                                if(wave_id == 0xff) pSettings->osc2_voice = 1;
                            }
                            else if(*selectedOsc == 0x03) {
                                pSettings->selectedWaveSub = wave_id;
                            }
                        }
                    }
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
        *displayCursor = 0x05;
        *displayStatus = DISPST_OSC;
    }
};

#endif // UIOSCWAVE_H