#include <ctrl_manager.h>

#ifndef SYNTHMANAGER_H
#define SYNTHMANAGER_H

class SynthManager {
private:
    CtrlManager* pCtrl;

public:
    SynthManager(CtrlManager* addr1) {
        pCtrl = addr1;
    }

    /**
     * @brief シンセのプリセットを設定します
     * 
     * @param synth 設定対象のシンセ
     * @param id プリセット番号
     */
    void setPreset(uint8_t synth, uint8_t id) {
        uint8_t data[] = {INS_BEGIN, DISP_SET_SHAPE, DATA_BEGIN, 0x02, synth, id};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
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