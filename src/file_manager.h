#include <ArduinoJson.h>

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

    bool folderExists(const char* path) {
        File root = SD.open("/");
        File entry;
        while (entry = root.openNextFile()) {
            if (entry.isDirectory() && strcmp(entry.name(), path) == 0) {
                entry.close();
                root.close();
                return true;
            }
            entry.close();
        }
        root.close();
        return false;
    }

    bool checkSD() {
        // SDカード確認
        if (!SD.begin(SD_CS_PIN)) {
            pDisplay->drawString("Error:1201", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            return false;
        }
        
        try {
            // ディレクトリ確認
            if (!folderExists("rp-ds16")) {
                SD.mkdir("rp-ds16");
            }
            // 設定ファイル確認
            if (!SD.exists("/rp-ds16/settings.json")) {
                File newFile = SD.open("/rp-ds16/settings.json", FILE_WRITE);
                newFile.println("{}");
                newFile.close();
            }
            File file = SD.open("/rp-ds16/settings.json", FILE_WRITE);
            
            file.close();
            return true;

        } catch (const char* error) {
            pDisplay->drawString("Error:1202", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            pDisplay->drawString(error, 1, 21);
            return false;
        }
    }
};