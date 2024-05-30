#include <IUIHandler.h>

#ifndef UIFILEMAN_H
#define UIFILEMAN_H

class UIFileMan : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    // UI用保存変数
    int* fileman_index;
    String* currentDir;
    Files* files;
    FsFile* file_buff;
    bool* isEndOfFile;
    bool* fileManRefresh;

    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;
    FileManager* pFile;

    LGFX_Sprite newSprite;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

public:
    UIFileMan(
        LGFXRP2040* pDisplay, LGFX_Sprite* pSprite, FileManager* pFile, uint8_t* displayStatus, uint8_t* displayCursor,
        int* fileman_index, String* currentDir, Files* files, FsFile* file_buff,
        bool* isEndOfFile, bool* fileManRefresh)
    {
        this->pDisplay = pDisplay;
        this->pSprite = pSprite;
        this->pFile = pFile;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->fileman_index = fileman_index;
        this->currentDir = currentDir;
        this->files = files;
        this->file_buff = file_buff;
        this->isEndOfFile = isEndOfFile;
        this->fileManRefresh = fileManRefresh;
    }

    /** @brief 画面更新 */
    void refreshUI() override {
        // タイトル
        if(currentDir->length() > 18) {
            String title = "..." + currentDir->substring(currentDir->length() - 18);
            pSprite->drawString(title, 2+9, 2);
        }else{
            pSprite->drawString(*currentDir, 2+9, 2);
        }
        pDisplay->showImage(&newSprite, FOLDER_8x7_IMG, 2, 2, 8, 7, pSprite);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

        // ファイルを取得 4つ スクロール時のみ取得
        if(*fileManRefresh) {
            pFile->getFiles(*currentDir, file_buff, 4, *fileman_index);
            for(int8_t i = 0; i < 4; i++) {
                char name[50];
                file_buff[i].getName(name, sizeof(name));
                files[i].type = file_buff[i].isDirectory();
                files[i].name = name;
                if(*currentDir == "/")
                    files[i].path = "/" + files[i].name;
                else
                    files[i].path = *currentDir + "/" + files[i].name;
                file_buff[i].close();
            }
            *fileManRefresh = false;
        }
        
        // ファイルを表示
        for(int8_t i = 0; i < 4; i++) {
            pSprite->drawString(files[i].name, 2, 16 + i * 10);
        }

        bool isDir = false;

        // 塗り
        if(*displayCursor == 0x01) {
            cursorText(files[0].name, 2, 16);
            if (files[0].type) isDir = true;
        }
        else if(*displayCursor == 0x02) {
            cursorText(files[1].name, 2, 26);
            if (files[1].type) isDir = true;
        }
        else if(*displayCursor == 0x03) {
            cursorText(files[2].name, 2, 36);
            if (files[2].type) isDir = true;
        }
        else if(*displayCursor == 0x04) {
            cursorText(files[3].name, 2, 46);
            if (files[3].type) isDir = true;
        }
        else if(*displayCursor == 0x05) {
            cursorText("CD TO ../", 2, 55);
        }

        // ファイルタイプ
        if(*displayCursor > 0x00 && *displayCursor < 0x05) {
            if (isDir) {
                pSprite->drawString("DIR", 128 - 2 - pSprite->textWidth("DIR"), 55);
                *isEndOfFile = false;

            } else if(files[*displayCursor - 1].name != nullptr) {
                pSprite->drawString("FILE", 128 - 2 - pSprite->textWidth("FILE"), 55);
                *isEndOfFile = false;
            }
            else
                *isEndOfFile = true;
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(*displayCursor == 0x01) {
            if(*fileman_index != 0) {
                (*fileman_index)--;
                *fileManRefresh = true;
                *displayCursor = 0x01;
            }
        }
        else (*displayCursor)--;
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(*displayCursor == 0x04) {
            if(!*isEndOfFile){
                (*fileman_index)++;
                *fileManRefresh = true;
                *displayCursor = 0x04;
            }
        } else if(*displayCursor == 0x05) {
            *displayCursor = 0x01;
        }
        else (*displayCursor)++;
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x01:
                if(files[0].type) {
                    *currentDir = files[0].path;
                    *displayCursor = 0x00;
                    *fileman_index = 0;
                }
                break;
            case 0x02:
                if(files[1].type) {
                    *currentDir = files[1].path;
                    *displayCursor = 0x00;
                    *fileman_index = 0;
                }
                break;
            case 0x03:
                if(files[2].type) {
                    *currentDir = files[2].path;
                    *displayCursor = 0x00;
                    *fileman_index = 0;
                }
                break;
            case 0x04:
                if(files[3].type) {
                    *currentDir = files[3].path;
                    *displayCursor = 0x00;
                    *fileman_index = 0;
                }
                break;
            case 0x05:
                int index = currentDir->lastIndexOf('/');
                if (index == -1) *currentDir = "/";
                *currentDir = currentDir->substring(0, index);
                if (*currentDir == "") *currentDir = "/";
                *displayCursor = 0x00;
                *fileman_index = 0;
                break;
        }
        *fileManRefresh = true;
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        if(*displayCursor == 0x05) {
            *displayCursor = 0x02;
            *displayStatus = DISPST_MENU;
        }
        else *displayCursor = 0x05;
    }
};

#endif // UIFILEMAN_H