#include <wokwi.h>

#ifndef CTRLMANAGER_H
#define CTRLMANAGER_H

#define CTRL_SDA_PIN 18
#define CTRL_SCL_PIN 19
#define CTRL_SW_PIN 20
#define CTRL_I2C_ADDR 0x0A

class CtrlManager {
private:
    TwoWire& ctrl = Wire1;
    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;
    static CtrlManager* instance;

public:
    CtrlManager(LGFXRP2040* addr1, LGFX_Sprite* addr2) {
        pDisplay = addr1;
        pSprite = addr2;
        instance = this;
    }

    void init() {
        ctrl.setSDA(CTRL_SDA_PIN);
        ctrl.setSCL(CTRL_SCL_PIN);
        ctrl.begin();
        ctrl.setClock(1000000);
    }

    bool checkConnection() {

        #if WOKWI_MODE == 1
            return true;
        #endif

        uint8_t data[] = {INS_BEGIN, CTRL_CONNECT};
        uint8_t received[1];
        ctrlTransmission(data, sizeof(data), received, 1);

        // 応答が返ってくればOK
        if(received[0] != RES_OK){
            pDisplay->fillScreen(TFT_BLACK);
            pDisplay->drawString("Error:1101", 1, 1);
            pDisplay->drawString("Please check the conn", 1, 11);
            pDisplay->drawString("ection.", 1, 21);
            return false;
        }
        return true;
    }

    /**
     * @brief CTRLとの通信を切り替えます
     * 
     * @param begin true:待機時間が追加されます
     */
    void toggleCtrl(bool begin) {
        digitalWrite(CTRL_SW_PIN, HIGH);
        digitalWrite(CTRL_SW_PIN, LOW);
        if(begin){
            delay(10);
        }
    }

    /**
     * @brief CTRLにデータを送信し、応答を取得します。
     * 
     * @param data 送信するデータ
     * @param size 送信するサイズ
     * @param received データを格納する配列
     * @param requestSize 要求するサイズ
     */
    void ctrlTransmission(uint8_t* data, size_t size, uint8_t* received, size_t requestSize) {

        #if WOKWI_MODE == 1
            *received = RES_OK;
            return;
        #endif
        
        digitalWrite(LED_BUILTIN, HIGH);
        toggleCtrl(true);
        ctrl.beginTransmission(CTRL_I2C_ADDR);
        ctrl.write(data, size);
        ctrl.endTransmission();
        ctrl.requestFrom(CTRL_I2C_ADDR, requestSize);

        int i = 0;
        while (ctrl.available()) {
            received[i] = ctrl.read();
            i++;
            if(i >= requestSize) {
                break;
            }
        }
        toggleCtrl(false);
        digitalWrite(LED_BUILTIN, LOW);
    }

    static void receiveWrapper(int bytes) {
        instance->receiveEvent(bytes);
    }
    
    // debugモード時に通信を受け取るためのコード
    void receiveEvent(int bytes) {
        // 2バイト以上のみ受け付ける
        if(bytes < 2) return;

        int i = 0;
        uint8_t receivedData[bytes];
        while (ctrl.available()) {
            uint8_t received = ctrl.read();
            receivedData[i] = received;
            i++;
            if (i >= bytes) {
                break;
            }
        }

        uint8_t instruction = 0x00; // コード種別
        if(receivedData[0] == INS_BEGIN) {
            instruction = receivedData[1];
        }

        if(instruction == CTRL_DEBUG_DATA) {
            // 例: {INS_BEGIN, CTRL_DEBUG_DATA, DETA_BEGIN, 0x04, 0x01, 0x11, 0xA2, 0x01}
            if(bytes < 6) return;
            uint8_t statusByte = receivedData[4];
            uint8_t dataByte[2] = {0xff, 0xff};
            uint8_t synthByte = receivedData[5];

            if(bytes > 7) {
                dataByte[0] = receivedData[5];
                dataByte[1] = receivedData[6];
                synthByte = receivedData[7];
            }
            else if(bytes > 6) {
                dataByte[0] = receivedData[5];
                synthByte = receivedData[6];
            }

            pSprite->createSprite(128, 64);

            // タイトル
            pSprite->drawString("Debug Mode", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // シンセモード
            uint8_t synth_x = pSprite->textWidth("MIDI-1.0");
            pSprite->drawString("MIDI-1.0", 128 - 2 - synth_x, 2);

            // データ表示部
            pSprite->fillRect(2, 16, 126, 9, TFT_BLACK);
            if(synthByte == 0x00) {
                pSprite->drawString("Global", 2, 16);
            }else{
                char sy_chr[12]; sprintf(sy_chr, "Synth%x", synthByte);
                pSprite->drawString(sy_chr, 2, 16);
            }

            char sb_chr[5]; sprintf(sb_chr, "0x%02x", statusByte);
            char db1_chr[5];
            if(dataByte[0] == 0xff) sprintf(db1_chr, "%s", "----");
            else sprintf(db1_chr, "0x%02x", dataByte[0]);
            char db2_chr[5];
            if (dataByte[1] == 0xff) sprintf(db2_chr, "%s", "----");
            else sprintf(db2_chr, "0x%02x", dataByte[1]);
            
            pSprite->fillRect(2, 26, 126, 9, TFT_BLACK);
            char msg[10]; sprintf(msg, " %s %s %s", sb_chr, db1_chr, db2_chr);
            pSprite->drawString(msg, 2, 26);

            pSprite->fillRect(2, 36, 126, 9, TFT_BLACK);
            char statusStr[13] = "";
            sprintf(statusStr, "%x", statusByte);
            if(statusByte == 0x90) strcpy(statusStr, "CH1_NOTE_ON");
            else if(statusByte == 0x80) strcpy(statusStr, "CH1_NOTE_OFF");
            else if(statusByte == 0x91) strcpy(statusStr, "CH2_NOTE_ON");
            else if(statusByte == 0x81) strcpy(statusStr, "CH2_NOTE_OFF");
            pSprite->drawString("("+ String(statusStr) +" "+ String(dataByte[0]) +" "+ String(dataByte[1]) +")", 2, 36);

            pSprite->pushSprite(0, 0);
            pSprite->deleteSprite();
        }
    }
};

#endif // CTRLMANAGER_H