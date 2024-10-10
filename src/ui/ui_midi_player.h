#include <IUIHandler.h>

#ifndef UIMIDIPLAYER_H
#define UIMIDIPLAYER_H

class UIMidiPlayer : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;
    CtrlManager* pCtrl;
    MidiManager* pMidi;
    FileManager* pFile;

    Settings* pSettings;

    // todo: 展示用
    uint8_t demo_n = 0x00;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIMidiPlayer(
        LGFX_Sprite* pSprite, CtrlManager* pCtrl, uint8_t* displayStatus,
        uint8_t* displayCursor, MidiManager* pMidi, FileManager* pFile, Settings* pSettings)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pCtrl = pCtrl;
        this->pMidi = pMidi;
        this->pFile = pFile;
        this->pSettings = pSettings;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("> MIDI Player", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // todo: 展示用
        if(pSettings->midi_playing) {
            pSprite->drawString("now playing", 2, 16);
            char num[2]; sprintf(num, "%d", demo_n + 1);
            pSprite->drawString("> demo" + String(num) + ".mid", 2, 26);
            pSprite->drawString("Press Enter to Stop", 2, 46);
        } else {
            pSprite->drawString("stopped", 2, 16);
            pSprite->drawString("Press Enter to Play", 2, 46);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(longPush) return;
        //todo: 展示用
        if(pSettings->midi_playing) {
            if(demo_n > 0x00) {
                --demo_n;
                pMidi->stopMidi();
                char num[2]; sprintf(num, "%d", demo_n+1);
                pMidi->playMidi("/rp-ds16/midi/demo" + String(num) + ".mid");
            }
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(longPush) return;
        //todo: 展示用
        if(pSettings->midi_playing) {
            if(demo_n < 0x01) {
                ++demo_n;
                pMidi->stopMidi();
                char num[2]; sprintf(num, "%d", demo_n+1);
                pMidi->playMidi("/rp-ds16/midi/demo" + String(num) + ".mid");
            }
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        //todo :展示用
        if(!pSettings->midi_playing) {
            pSettings->midi_playing = true;
            char num[2]; sprintf(num, "%d", demo_n+1);
            pMidi->playMidi("/rp-ds16/midi/demo" + String(num) + ".mid");
            pMidi->loopMidi(true);
        } else {
            pMidi->loopMidi(false);
            pMidi->stopMidi();
            pSettings->midi_playing = false;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x02;
        *displayStatus = DISPST_MENU;
    }
};

#endif // UIMIDIPLAYER_H