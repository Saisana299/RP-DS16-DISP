#include <ArduinoJson.h>

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

class FileManager {
private:
    // SDカード関連
    #define SD_SCK_PIN 2
    #define SD_TX_PIN 3
    #define SD_RX_PIN 4
    #define SD_CS_PIN 5

    LGFXRP2040* pDisplay;

public:
    FileManager(LGFXRP2040* addr1) {
        pDisplay = addr1;
    }

    void init() {
        SPI.setRX(SD_RX_PIN);
        SPI.setCS(SD_CS_PIN);
        SPI.setSCK(SD_SCK_PIN);
        SPI.setTX(SD_TX_PIN);
    }

    bool initJson(String dir, String file_name, String json = "{}") {
        try {
            // ディレクトリ生成
            SD.mkdir(dir);

            // ファイル初期化
            if (!SD.exists(dir + "/" + file_name)) {
                File newFile = SD.open(dir + "/" + file_name, FILE_WRITE);
                newFile.println(json);
                newFile.close();
            }
        } catch (const char* error) {
            pDisplay->drawString("Error:1202", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            pDisplay->drawString(error, 1, 21);
            return false;
        }
        return true;
    }

    bool getJson(JsonDocument* doc, String path) {
        try {
            if(!SD.exists(path)) return false;
            File file = SD.open(path, FILE_READ);
            deserializeJson(*doc, file);
            file.close();
        } catch (const char* error) {
            pDisplay->drawString("Error:1202", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            pDisplay->drawString(error, 1, 21);
            return false;
        }
        return true;
    }

    bool checkSD() {
        // SDカード確認
        if (!SD.begin(SD_CS_PIN)) {
            pDisplay->drawString("Error:1201", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            return false;
        }

        bool result = initJson("/rp-ds16", "settings.json");
        if(!result) return false;

        result = initJson("/rp-ds16", "presets.json");
        if(!result) return false;

        SD.mkdir("/rp-ds16/midi");
        SD.mkdir("/rp-ds16/rlev");
        
        return true;
    }
};

#endif // FILEMANAGER_H