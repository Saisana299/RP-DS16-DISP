#include <IUIHandler.h>

#ifndef UIMIDIPLAYER_H
#define UIMIDIPLAYER_H

class UIMidiPlayer : public IUIHandler {
private:

    // Midi関連
    bool isRefreshFile = true;
    int fileIndex = 0;

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;
    CtrlManager* pCtrl;
    MidiManager* pMidi;
    FileManager* pFile;

    Files* files;
    FsFile* file_buff;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIMidiPlayer(
        LGFX_Sprite* pSprite, CtrlManager* pCtrl, uint8_t* displayStatus,
        uint8_t* displayCursor, MidiManager* pMidi, FileManager* pFile,
        Files* files, FsFile* file_buff)
    {
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->pSprite = pSprite;
        this->pCtrl = pCtrl;
        this->pMidi = pMidi;
        this->pFile = pFile;
        this->files = files;
        this->file_buff = file_buff;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        pSprite->drawString("MIDI Player", 2, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // 更新有効時にファイル取得
        if(isRefreshFile) {
            pFile->getFiles("/rp-ds16/midi", file_buff, 4, fileIndex);
            isRefreshFile = false;
        }
        delay(100); // 少し待つ
        uint8_t status = pMidi->getStatus();
        if(status == MIDI_IDLE && !pMidi->isPlayMidi){
            pSprite->drawString("MIDI_IDLE", 2, 16);
        } else if (status == MIDI_PLAYING && !pMidi->isLocking) {
            pSprite->drawString("MIDI_PAUSED", 2, 16);
        } else if (status == MIDI_PLAYING && pMidi->isLocking) {
            pSprite->drawString("MIDI_PLAYING", 2, 16);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        pMidi->pauseMidi();
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        pMidi->resumeMidi();
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        if(longPush) return;
        pMidi->stopMidi();
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        if(longPush) return;
        pMidi->playMidi("/rp-ds16/midi/test.mid");
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x03;
        *displayStatus = DISPST_MENU;
    }
};

#endif // UIMIDIPLAYER_H