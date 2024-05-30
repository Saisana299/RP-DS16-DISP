#include <ArduinoJson.h>
#include <midi_manager.h>

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
    MidiManager* pMidi;
    SdFs SD;

    bool lockSD() {
        if(pMidi->isLocking) pMidi->pauseMidi();
        uint8_t i;
        for(i = 0; i < 20; i++) {
            if(!pMidi->isLocking) break;
            delay(500);
        }
        if(i == 20) return false;
        return true;
    }

    void unlockSD() {
        if(pMidi->isPlayMidi) pMidi->resumeMidi();
    }

public:
    FileManager(LGFXRP2040* addr1, MidiManager* addr2) {
        pDisplay = addr1;
        pMidi = addr2;
    }

    void init() {
        SPI.setRX(SD_RX_PIN);
        SPI.setCS(SD_CS_PIN);
        SPI.setSCK(SD_SCK_PIN);
        SPI.setTX(SD_TX_PIN);
    }

    bool initJson(String dir, String file_name, String json = "{}") {
        if(!lockSD()) return false;
        try {
            // ディレクトリ生成
            SD.mkdir(dir);

            // ファイル初期化
            if (!SD.exists(dir + "/" + file_name)) {
                FsFile newFile = SD.open(dir + "/" + file_name, FILE_WRITE);
                newFile.println(json);
                newFile.close();
            }
        } catch (const char* error) {
            pDisplay->drawString("Error:1202", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            pDisplay->drawString(error, 1, 21);
            unlockSD();
            return false;
        }
        unlockSD();
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
        if(!lockSD()) return false;
        try {
            if(!SD.exists(path)) return false;
            FsFile file = SD.open(path, FILE_READ);
            deserializeJson(*doc, file);
            file.close();
        } catch (const char* error) {
            pDisplay->drawString("Error:1202", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            pDisplay->drawString(error, 1, 21);
            unlockSD();
            return false;
        }
        unlockSD();
        return true;
    }

    void getFiles(String path, FsFile *files, uint8_t count, int32_t offset = 0) {
        if(!lockSD()) return;
        FsFile dir = SD.open(path);
        FsFile file = dir.openNextFile();
        for (int32_t i = 0; i < count + offset; i++) {
            if(i - offset >= 0)    
                files[i - offset] = file;
            file = dir.openNextFile();
        }
        dir.close();
        unlockSD();
    }

    int getFileCount(String path) {
        if(!lockSD()) return 0;
        FsFile dir = SD.open(path);
        FsFile file = dir.openNextFile();
        int count = 0;
        while (file) {
            count++;
            file = dir.openNextFile();
        }
        dir.close();
        unlockSD();
        return count;
    }

    bool checkSD() {
        if(!lockSD()) return false;
        // SDカード確認
        if (!SD.begin(SD_CS_PIN)) {
            pDisplay->fillScreen(TFT_BLACK);
            pDisplay->drawString("Error:1201", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            unlockSD();
            return false;
        }

        bool result = initJson("/rp-ds16", "settings.json");
        if(!result) {
            unlockSD();
            return false;
        }

        SD.mkdir("/rp-ds16/preset"); // .json
        SD.mkdir("/rp-ds16/midi"); // .mid
        SD.mkdir("/rp-ds16/rlem"); // .rlem
        SD.mkdir("/rp-ds16/wavetable"); // .json
        
        unlockSD();
        return true;
    }
};

#endif // FILEMANAGER_H