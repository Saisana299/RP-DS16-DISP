#include <MD_MIDIFile.h>

#ifndef MIDIMANAGER_H
#define MIDIMANAGER_H

#define MIDI_IDLE    0x00
#define MIDI_PLAY    0x01
#define MIDI_PLAYING 0x02
#define MIDI_STOP    0x03
#define MIDI_PAUSE   0x04
#define MIDI_RESUME  0x05

class MidiManager {
private:
    // midi関連
    volatile bool isPlayMidi = false;
    volatile uint8_t flag = MIDI_IDLE;
    volatile bool isBackground = false;
    volatile char midiFile[13] = "test.mid";
    
    SdFs SD;
    MD_MIDIFile SMF;

    CtrlManager* pCtrl;

    // Core1で実行
    static void midiCallback(midi_event *pev) {
        if ((pev->data[0] >= 0x80) && (pev->data[0] <= 0xe0)) {
            Serial2.write(pev->data[0] | pev->channel);
            Serial2.write(&pev->data[1], pev->size - 1);
        } else {
            Serial2.write(pev->data, pev->size);
        }
    }

public:
    MidiManager(CtrlManager* addr1) {
        pCtrl = addr1;
    }

    void init() {
        // SdFs初期化
        SD.begin(SD_CS_PIN, SPI_FULL_SPEED);

        // MIDIFileの初期化
        SMF.begin(&SD);
        SMF.setMidiHandler(midiCallback); // MIDIハンドラを設定
        SMF.looping(false); // ループ再生を無効に設定

        // SMFのcore0からの利用はここまで //
    }

    void playMidi(String path) {
        uint8_t data[] = {INS_BEGIN, DISP_MIDI_ON};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        flag = MIDI_PLAY;
    }

    void pauseMidi() {
        flag = MIDI_PAUSE;
    }

    void resumeMidi() {
        flag = MIDI_RESUME;
    }

    void stopMidi() {
        uint8_t data[] = {INS_BEGIN, DISP_MIDI_OFF};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        flag = MIDI_STOP;
    }

    void setBackground(bool v) {
        isBackground = v;
    }

    // Core1で実行
    void midiHandler() {
        if(flag == MIDI_PLAY) {

            flag = MIDI_PLAYING;
            SMF.load(const_cast<const char*>(midiFile));
            isPlayMidi = true;
        }
        else if(flag == MIDI_PLAYING) {

            if (!isPlayMidi) return;
            if (!SMF.isEOF()) {
                SMF.getNextEvent();
            }
            else {
                isPlayMidi = false;
                SMF.close();
            }
        }
        else if(flag == MIDI_STOP) {
            SMF.close();
            isPlayMidi = false;
            flag = MIDI_IDLE;
        }
        else if(flag == MIDI_PAUSE) {
            SMF.pause(true);
            flag = MIDI_PLAYING;
        }
        else if(flag == MIDI_RESUME) {
            SMF.pause(false);
            flag = MIDI_PLAYING;
        }
    }
};

#endif // MIDIMANAGER_H