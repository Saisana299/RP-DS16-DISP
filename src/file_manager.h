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

    bool hasExtension(const String &filename, const String &extension) {
        int extIndex = filename.lastIndexOf(extension);
        if (extIndex == -1) {
            return false;
        }
        // 拡張子がファイル名の末尾にあるかどうかを確認
        return (extIndex + extension.length() == filename.length());
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

    void getFiles(String path, File *files, uint8_t count, int offset = 0) {
        File dir = SD.open(path);
        File file = dir.openNextFile();
        for (uint8_t i = 0; i < count + offset; i++) {
            if(i - offset >= 0)    
                files[i - offset] = file;
            file = dir.openNextFile();
        }
        dir.close();
    }

    int getFileCount(String path) {
        File dir = SD.open(path);
        File file = dir.openNextFile();
        int count = 0;
        while (file) {
            count++;
            file = dir.openNextFile();
        }
        dir.close();
        return count;
    }

    bool checkSD() {
        // SDカード確認
        if (!SD.begin(SD_CS_PIN)) {
            pDisplay->fillScreen(TFT_BLACK);
            pDisplay->drawString("Error:1201", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            return false;
        }

        bool result = initJson("/rp-ds16", "settings.json");
        if(!result) return false;

        SD.mkdir("/rp-ds16/preset"); // .json
        SD.mkdir("/rp-ds16/midi"); // .mid
        SD.mkdir("/rp-ds16/rlem"); // .rlem
        SD.mkdir("/rp-ds16/wavetable"); // .json
        
        return true;
    }
};

#endif // FILEMANAGER_H