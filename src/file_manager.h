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

    bool checkSD() {
        // SDカード確認
        if(!SD.begin(SD_CS_PIN)) {
            pDisplay->drawString("Error:1201", 1, 1);
            pDisplay->drawString("SD card error.", 1, 11);
            return false;
        }
        File myFile = SD.open("settings.json", FILE_WRITE);
        myFile.close();
        return true;
    }
};