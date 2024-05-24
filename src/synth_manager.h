#include <ctrl_manager.h>

#ifndef SYNTHMANAGER_H
#define SYNTHMANAGER_H

class SynthManager {
private:
    CtrlManager* pCtrl;
    uint8_t cshape_buff[4096];

    // カスタムシェイプを設定
    void setCustomShape(uint8_t synth, uint8_t osc, int16_t* wave) {

        uint8_t received[1];

        // シンセリセット
        uint8_t reset_data[] = {INS_BEGIN, DISP_RESET_SYNTH, DATA_BEGIN, 0x01, 0xff};
        pCtrl->ctrlTransmission(reset_data, sizeof(reset_data), received, 1);

        // シンセ制御停止
        uint8_t stop_data[] = {INS_BEGIN, DISP_STOP_SYNTH};
        pCtrl->ctrlTransmission(stop_data, sizeof(stop_data), received, 1);

        memset(cshape_buff, 0, 4096 * sizeof(uint8_t)); // バッファをクリア
        uint16_t j = 0;
        for (uint16_t i = 0; i < 4096; i += 2) {
            cshape_buff[i] = static_cast<uint8_t>(wave[j] & 0xFF); // 下位バイト
            cshape_buff[i+1] = static_cast<uint8_t>((wave[j] >> 8) & 0xFF); // 上位バイト
            j++;
        }

        j = 0;

        while(1) {
            uint8_t data[30];
            for (uint16_t i = 0; i < 30; i++) {
                if(i == 0) data[i] = INS_BEGIN;
                else if(i == 1) data[i] = DISP_SET_CSHAPE;
                else if(i == 2) data[i] = DATA_BEGIN;
                else if(i == 3) data[i] = 0x24;
                else if(i == 4) data[i] = synth;
                else if(i == 5) data[i] = osc;
                else {
                    if(j == 4096) {
                        break;
                    }
                    else data[i] = cshape_buff[j];
                    j++;
                }
            }

            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);

            if(j == 4096) break;
            delay(10);
        }

        // シンセ制御再開
        uint8_t start_data[] = {INS_BEGIN, DISP_START_SYNTH};
        pCtrl->ctrlTransmission(start_data, sizeof(start_data), received, 1);
    }

public:
    SynthManager(CtrlManager* addr1) {
        pCtrl = addr1;
    }

    /**
     * @brief シンセの波形を設定します
     * 
     * @param synth 設定対象のシンセ
     * @param id プリセット番号
     */
    void setShape(uint8_t synth, uint8_t osc, uint8_t id, int16_t* wave = nullptr) {
        if(wave != nullptr){
            setCustomShape(synth, osc, wave);
        }else{
            uint8_t data[] = {INS_BEGIN, DISP_SET_SHAPE, DATA_BEGIN, 0x02, synth, id};
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
    }

    /**
     * @brief シンセのモードを設定します
     * 
     * @param mode シンセモード番号
     */
    void setSynthMode(uint8_t mode) {
        uint8_t data[] = {INS_BEGIN, DISP_SET_SYNTH, DATA_BEGIN, 0x01, mode};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    /**
     * @brief シンセのノートをリセットします (MIDI Panic)
     * 
     * @param synth 対象のシンセ
     */
    void resetSynth(uint8_t synth) {
        uint8_t data[] = {INS_BEGIN, DISP_RESET_SYNTH, DATA_BEGIN, 0x01, synth};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // アタックを設定
    void setAttack(uint8_t synth, int16_t attack) {
        // attack = 0-32sec + 0-999ms (max32sec)
        uint8_t attack_sec;
        uint8_t attack_ms[4] = {0,0,0,0};
        
        attack_sec = attack / 1000;

        // msを分割
        for(uint8_t i = 0; i <= ((attack - (attack_sec*1000)) / 255); i++) {
            if(i == ((attack - (attack_sec*1000)) / 255)) attack_ms[i] = (attack - (attack_sec*1000)) - (i*255);
            else attack_ms[i] = 255;
        }

        uint8_t data[] = {
            INS_BEGIN, DISP_SET_ATTACK, DATA_BEGIN,
            0x06, synth, attack_sec, attack_ms[0], attack_ms[1], attack_ms[2], attack_ms[3]
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // リリースを設定
    void setRelease(uint8_t synth, int16_t release) {
        // release = 0-32sec + 0-999ms (max32sec)
        uint8_t release_sec;
        uint8_t release_ms[4] = {0,0,0,0};
        
        release_sec = release / 1000;

        // msを分割
        for(uint8_t i = 0; i <= ((release - (release_sec*1000)) / 255); i++) {
            if(i == ((release - (release_sec*1000)) / 255)) release_ms[i] = (release - (release_sec*1000)) - (i*255);
            else release_ms[i] = 255;
        }

        uint8_t data[] = {
            INS_BEGIN, DISP_SET_RELEASE, DATA_BEGIN, 
            0x06, synth, release_sec, release_ms[0], release_ms[1], release_ms[2], release_ms[3]
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // ディケイを設定
    void setDecay(uint8_t synth, int16_t decay) {
        // decay = 0-32sec + 0-999ms (max32sec)
        uint8_t decay_sec;
        uint8_t decay_ms[4] = {0,0,0,0};

        decay_sec = decay / 1000;

        // msを分割
        for(uint8_t i = 0; i <= ((decay - (decay_sec*1000)) / 255); i++) {
            if(i == ((decay - (decay_sec*1000)) / 255)) decay_ms[i] = (decay - (decay_sec*1000)) - (i*255);
            else decay_ms[i] = 255;
        }

        uint8_t data[] = {
            INS_BEGIN, DISP_SET_DECAY, DATA_BEGIN,
            0x06, synth, decay_sec, decay_ms[0], decay_ms[1], decay_ms[2], decay_ms[3]
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // サステインを設定
    void setSustain(uint8_t synth, int16_t sustain) {
        // sustain = 0-1000
        uint8_t sustains[4] = {0,0,0,0};

        // 4分割
        for(uint8_t i = 0; i <= (sustain / 255); i++) {
            if(i == (sustain / 255)) sustains[i] = sustain - (i*255);
            else sustains[i] = 255;
        }

        uint8_t data[] = {
            INS_BEGIN, DISP_SET_SUSTAIN, DATA_BEGIN, 
            0x05, synth, sustains[0], sustains[1], sustains[2], sustains[3]
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }
};

#endif // SYNTHMANAGER_H