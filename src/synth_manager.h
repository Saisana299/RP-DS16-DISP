#include <ctrl_manager.h>
#include <settings.h>
#include <file_manager.h>
#include <midi_manager.h>

#ifndef SYNTHMANAGER_H
#define SYNTHMANAGER_H

class SynthManager {
private:
    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;
    CtrlManager* pCtrl;
    Settings* pSettings;
    FileManager* pFile;
    MidiManager* pMidi;
    uint8_t cshape_buff[4096];

    // カスタムシェイプを設定
    void setCustomShape(uint8_t synth, uint8_t osc, int16_t* wave) {

        uint8_t received[1];

        // シンセリセット
        uint8_t reset_data[] = {CTRL_RESET_SYNTH, 0xff};
        pCtrl->ctrlTransmission(reset_data, sizeof(reset_data), received, 1);

        // シンセ制御停止
        uint8_t stop_data[] = {CTRL_STOP_SYNTH};
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
            pSprite->createSprite(64, 10);

            uint8_t data[27];
            for (uint16_t i = 0; i < 27; i++) {
                if(i == 0) data[i] = SYNTH_SET_CSHAPE;
                else if(i == 1) data[i] = synth;
                else if(i == 2) data[i] = osc;
                else {
                    if(j == 4096) {
                        break;
                    }
                    else data[i] = cshape_buff[j];
                    j++;
                }
            }

            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);

            pSprite->drawString(String((float(j)/4096.0f)*100) + "%", 2, 1);
            pSprite->pushSprite(0, 54);
            pSprite->deleteSprite();

            if(j == 4096) break;
        }

        // シンセ制御再開
        uint8_t start_data[] = {CTRL_START_SYNTH};
        pCtrl->ctrlTransmission(start_data, sizeof(start_data), received, 1);
    }

public:
    SynthManager(LGFXRP2040* addr1, LGFX_Sprite* addr2, CtrlManager* addr3, Settings* addr4, FileManager* addr5, MidiManager* addr6) {
        pDisplay = addr1;
        pSprite = addr2;
        pCtrl = addr3;
        pSettings = addr4;
        pFile = addr5;
        pMidi = addr6;
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
            uint8_t data[] = {SYNTH_SET_SHAPE, synth, id, osc};
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
        uint8_t data[] = {CTRL_SET_SYNTH, mode};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    /**
     * @brief シンセのノートをリセットします (MIDI Panic)
     * @param synth 対象のシンセ
     */
    void resetSynth(uint8_t synth) {
        uint8_t data[] = {CTRL_RESET_SYNTH, synth};
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    /**
     * @brief シンセのパラメータをリセットします
     * @param synth 対象のシンセ
     */
    void resetSynthParam(uint8_t synth) {
        uint8_t data[] = {SYNTH_RESET_PARAM, synth};
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
            SYNTH_SET_ATTACK, synth, attack_sec, attack_ms[0], attack_ms[1], attack_ms[2], attack_ms[3]
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
            SYNTH_SET_RELEASE, synth, release_sec, release_ms[0], release_ms[1], release_ms[2], release_ms[3]
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
            SYNTH_SET_DECAY, synth, decay_sec, decay_ms[0], decay_ms[1], decay_ms[2], decay_ms[3]
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
            SYNTH_SET_SUSTAIN, synth, sustains[0], sustains[1], sustains[2], sustains[3]
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // ボイス数を設定
    void setVoice(uint8_t synth, uint8_t voice, uint8_t osc) {
        uint8_t data[] = {
            SYNTH_SET_VOICE, synth, voice, osc
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // デチューンを設定
    void setDetune(uint8_t synth, uint8_t detune, uint8_t osc) {
        uint8_t data[] = {
            SYNTH_SET_DETUNE, synth, detune, osc
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // スプレドを設定
    void setSpread(uint8_t synth, uint8_t spread, uint8_t osc) {
        uint8_t data[] = {
            SYNTH_SET_SPREAD, synth, spread, osc
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // ローパスフィルタを設定
    void setLowPassFilter(uint8_t synth, uint8_t enable, float freq = 1000.0f, float q = 1.0f/sqrt(2.0f)) {
        if(enable == 0x01) {
            uint8_t d_freq[sizeof(float)];
            memcpy(d_freq, &freq, sizeof(float));
            uint8_t d_q[sizeof(float)];
            memcpy(d_q, &q, sizeof(float));

            uint8_t data[] = {
                SYNTH_SET_LPF, synth, enable, d_freq[0], d_freq[1], d_freq[2], d_freq[3], d_q[0], d_q[1], d_q[2], d_q[3]
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
        else {
            uint8_t data[] = {
                SYNTH_SET_LPF, synth, enable
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
    }

    // ハイパスフィルタを設定
    void setHighPassFilter(uint8_t synth, uint8_t enable, float freq = 1000.0f, float q = 1.0f/sqrt(2.0f)) {
        if(enable == 0x01) {
            uint8_t d_freq[sizeof(float)];
            memcpy(d_freq, &freq, sizeof(float));
            uint8_t d_q[sizeof(float)];
            memcpy(d_q, &q, sizeof(float));

            uint8_t data[] = {
                SYNTH_SET_HPF, synth, enable, d_freq[0], d_freq[1], d_freq[2], d_freq[3], d_q[0], d_q[1], d_q[2], d_q[3]
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
        else {
            uint8_t data[] = {
                SYNTH_SET_HPF, synth, enable
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
    }

    // oscレベルを設定
    void setOscLevel(uint8_t synth, uint8_t osc, int16_t level) {
        uint8_t data[] = {
            SYNTH_SET_OSC_LVL, synth, osc,
            static_cast<uint8_t>((level >> 8) & 0xFF),
            static_cast<uint8_t>(level & 0xFF)
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // oscオクターブを設定
    void setOscOctave(uint8_t synth, uint8_t osc, int8_t octave) {
        uint8_t data[] = {
            SYNTH_SET_OCT, synth, osc, static_cast<uint8_t>(octave)
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // oscセミトーンを設定
    void setOscSemitone(uint8_t synth, uint8_t osc, int8_t semitone) {
        uint8_t data[] = {
            SYNTH_SET_SEMI, synth, osc, static_cast<uint8_t>(semitone)
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // osc centを設定
    void setOscCent(uint8_t synth, uint8_t osc, int8_t cent) {
        uint8_t data[] = {
            SYNTH_SET_CENT, synth, osc, static_cast<uint8_t>(cent)
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // アンプレベルを設定
    void setAmpLevel(uint8_t synth, int16_t level) {
        uint8_t data[] = {
            SYNTH_SET_LEVEL, synth,
            static_cast<uint8_t>((level >> 8) & 0xFF),
            static_cast<uint8_t>(level & 0xFF)
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // アンプパンを設定
    void setAmpPan(uint8_t synth, uint8_t pan) {
        uint8_t data[] = {
            SYNTH_SET_PAN, synth, pan
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    // ディレイを設定
    void setDelay(uint8_t synth, uint8_t enable, int16_t time = 250, int16_t level = 300, int16_t feedback = 500) {
        if(enable == 0x01) {
            uint8_t data[] = {
                SYNTH_SET_DELAY, synth, enable,
                static_cast<uint8_t>((time >> 8) & 0xFF),
                static_cast<uint8_t>(time & 0xFF),
                static_cast<uint8_t>((level >> 8) & 0xFF),
                static_cast<uint8_t>(level & 0xFF),
                static_cast<uint8_t>((feedback >> 8) & 0xFF),
                static_cast<uint8_t>(feedback & 0xFF)
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
        else {
            uint8_t data[] = {SYNTH_SET_DELAY, synth, enable};
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
    }

    // モジュレーションを設定
    void setMod(uint8_t synth, uint8_t mod) {
        uint8_t data[] = {
            SYNTH_SET_MOD, synth, mod
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    void setMonophonic(uint8_t synth, bool enable) {
        uint8_t b = enable ? 0x01 : 0x00;
        uint8_t data[] = {
            SYNTH_SET_MONO, synth, b
        };
        uint8_t received[1];
        pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
    }

    void setGlideMode(uint8_t synth, bool enable, uint16_t time = 15) {
        uint8_t b = enable ? 0x01 : 0x00;
        if(!enable) {
            uint8_t data[] = {
                SYNTH_SET_GLIDE, synth, b
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }

        else {
            uint8_t data[] = {
                SYNTH_SET_GLIDE, synth, b,
                static_cast<uint8_t>((time >> 8) & 0xFF),
                static_cast<uint8_t>(time & 0xFF)
            };
            uint8_t received[1];
            pCtrl->ctrlTransmission(data, sizeof(data), received, 1);
        }
    }

    /**
     * @brief Set the Preset object
     * 
     * @param id 
     * @param synth 
     */
    void setPreset(uint8_t id, uint8_t synth) {

        // MIDI Player が動いている場合停止させる
        if(pMidi->getStatus() != MIDI_IDLE) {
            pMidi->stopMidi();
            pSettings->midi_playing = false;
        }

        // 全ての項目を初期値に設定する
        resetSynthParam(synth);
        pSettings->resetParam();

        // defaultプリセットはosc=0x01固定
        if(id < FACTORY_PRESETS) {
            setShape(synth, 0x01, id);
            pSettings->selectedWave = id;
            if(id == 0xff) pSettings->osc1_voice = 1;
        }

        // ユーザープリセット
        else {
            JsonDocument doc;
            bool result = pFile->getJson(&doc, pSettings->user_presets[id - FACTORY_PRESETS].path);
            if(!result) return;

            //   ___           _ _ _       _               _
            //  / _ \ ___  ___(_) | | __ _| |_ ___  _ __  / |
            // | | | / __|/ __| | | |/ _` | __/ _ \| '__| | |
            // | |_| \__ \ (__| | | | (_| | || (_) | |    | |
            //  \___/|___/\___|_|_|_|\__,_|\__\___/|_|    |_|

            // 項目 osc1 が存在するか
            if(doc.containsKey("osc1")) {
                JsonObject osc1 = doc["osc1"].as<JsonObject>();

                // 項目 osc1.wavetable が存在するか
                if(osc1.containsKey("wavetable")) {
                    JsonObject osc1_wavetable = osc1["wavetable"].as<JsonObject>();

                    // 項目 osc1.wavetable.type が存在するか
                    if(osc1_wavetable.containsKey("type")) {

                        // String型か
                        String osc1_type;
                        if(osc1_wavetable["type"].is<String>()) {
                            osc1_type = osc1_wavetable["type"].as<String>();
                        }

                        // ユーザーのwavetableの場合
                        if(osc1_type == "custom") {
                            // 項目 osc1.wavetable.path が存在するか
                            if(osc1_wavetable.containsKey("path")) {

                                // String型か
                                if(osc1_wavetable["path"].is<String>()) {
                                    String wave = osc1_wavetable["path"];

                                    JsonDocument wt_doc;
                                    bool result = pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                                    if(!result) return;

                                    // 項目 wave_table が存在するか
                                    if(!wt_doc.containsKey("wave_table")) return;
                                    JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                                    copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                                    setShape(synth, 0x01, id, pSettings->wave_table_buff);
                                    pSettings->selectedWave = FACTORY_PRESETS + 1;
                                }
                            }

                        // デフォルトのwavetableの場合
                        } else if(osc1_type == "default") {
                            // 項目 osc1.wavetable.path が存在するか
                            if(osc1_wavetable.containsKey("path")) {

                                // uint8_t型か
                                if(osc1_wavetable["path"].is<uint8_t>()) {
                                    uint8_t osc1_id = osc1_wavetable["path"];

                                    setShape(synth, 0x01, osc1_id);
                                    pSettings->selectedWave = osc1_id;
                                    if(osc1_id == 0xff) pSettings->osc1_voice = 1;
                                }
                            }
                        }
                    }
                }

                // 項目 osc1.unison が存在するか
                if(osc1.containsKey("unison")) {
                    JsonObject osc1_unison = osc1["unison"].as<JsonObject>();
                    // 項目 osc1.unison.voice が存在するか
                    if(osc1_unison.containsKey("voice")) {
                        // uint8_t型か
                        if(osc1_unison["voice"].is<uint8_t>()) {
                            uint8_t osc1_unison_voice = osc1_unison["voice"];
                            setVoice(synth, osc1_unison_voice, 0x01);
                            pSettings->osc1_voice = osc1_unison_voice;
                        }
                    }
                    // 項目 osc1.unison.detune が存在するか
                    if(osc1_unison.containsKey("detune")) {
                        // uint8_t型か
                        if(osc1_unison["detune"].is<uint8_t>()) {
                            uint8_t osc1_unison_detune = osc1_unison["detune"];
                            setDetune(synth, osc1_unison_detune, 0x01);
                            pSettings->osc1_detune = osc1_unison_detune;
                        }
                    }
                    // 項目 osc1.unison.spread が存在するか
                    if(osc1_unison.containsKey("spread")) {
                        // uint8_t型か
                        if(osc1_unison["spread"].is<uint8_t>()) {
                            uint8_t osc1_unison_spread = osc1_unison["spread"];
                            setSpread(synth, osc1_unison_spread, 0x01);
                            pSettings->osc1_spread = osc1_unison_spread;
                        }
                    }
                }

                // 項目 osc1.pitch が存在するか
                if(osc1.containsKey("pitch")) {
                    JsonObject osc1_pitch = osc1["pitch"].as<JsonObject>();
                    // 項目 osc1.pitch.octave が存在するか
                    if(osc1_pitch.containsKey("octave")) {
                        // int8_t型か
                        if(osc1_pitch["octave"].is<int8_t>()) {
                            int8_t osc1_pitch_octave = osc1_pitch["octave"];
                            setOscOctave(synth, 0x01, osc1_pitch_octave);
                            pSettings->osc1_oct = osc1_pitch_octave;
                        }
                    }
                    // 項目 osc1.pitch.semitone が存在するか
                    if(osc1_pitch.containsKey("semitone")) {
                        // int8_t型か
                        if(osc1_pitch["semitone"].is<int8_t>()) {
                            int8_t osc1_pitch_semitone = osc1_pitch["semitone"];
                            setOscSemitone(synth, 0x01, osc1_pitch_semitone);
                            pSettings->osc1_semi = osc1_pitch_semitone;
                        }
                    }
                    // 項目 osc1.pitch.cent が存在するか
                    if(osc1_pitch.containsKey("cent")) {
                        // int8_t型か
                        if(osc1_pitch["cent"].is<int8_t>()) {
                            int8_t osc1_pitch_cent = osc1_pitch["cent"];
                            setOscCent(synth, 0x01, osc1_pitch_cent);
                            pSettings->osc1_cent = osc1_pitch_cent;
                        }
                    }
                }

                // 項目 osc1.level が存在するか
                if(osc1.containsKey("level")) {
                    // int16_t型か
                    if(osc1["level"].is<int16_t>()) {
                        int16_t osc1_level = osc1["level"];
                        setOscLevel(synth, 0x01, osc1_level);
                        pSettings->osc1_level = osc1_level;
                    }
                }

                // 項目 osc1.pan が存在するか
                if(osc1.containsKey("pan")) {
                    // todo
                }
            }

            //   ___           _ _ _       _               ____
            //  / _ \ ___  ___(_) | | __ _| |_ ___  _ __  |___ \
            // | | | / __|/ __| | | |/ _` | __/ _ \| '__|   __) |
            // | |_| \__ \ (__| | | | (_| | || (_) | |     / __/
            //  \___/|___/\___|_|_|_|\__,_|\__\___/|_|    |_____|

            // 項目 osc2 が存在するか
            if(doc.containsKey("osc2")) {
                JsonObject osc2 = doc["osc2"].as<JsonObject>();

                // 項目 osc2.wavetable が存在するか
                if(osc2.containsKey("wavetable")) {
                    JsonObject osc2_wavetable = osc2["wavetable"].as<JsonObject>();

                    // 項目 osc2.wavetable.type が存在するか
                    if(osc2_wavetable.containsKey("type")) {

                        // String型か
                        String osc2_type;
                        if(osc2_wavetable["type"].is<String>()) {
                            osc2_type = osc2_wavetable["type"].as<String>();
                        }

                        // ユーザーのwavetableの場合
                        if(osc2_type == "custom") {
                            // 項目 osc2.wavetable.path が存在するか
                            if(osc2_wavetable.containsKey("path")) {

                                // String型か
                                if(osc2_wavetable["path"].is<String>()) {
                                    String wave = osc2_wavetable["path"];

                                    JsonDocument wt_doc;
                                    bool result = pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                                    if(!result) return;

                                    // 項目 wave_table が存在するか
                                    if(!wt_doc.containsKey("wave_table")) return;
                                    JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                                    copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                                    setShape(synth, 0x02, id, pSettings->wave_table_buff);
                                    pSettings->selectedWave2 = FACTORY_PRESETS + 1;
                                }
                            }

                        // デフォルトのwavetableの場合
                        } else if(osc2_type == "default") {
                            // 項目 osc2.wavetable.path が存在するか
                            if(osc2_wavetable.containsKey("path")) {

                                // uint8_t型か
                                if(osc2_wavetable["path"].is<uint8_t>()) {
                                    uint8_t osc2_id = osc2_wavetable["path"];

                                    setShape(synth, 0x02, osc2_id);
                                    pSettings->selectedWave2 = osc2_id;
                                    if(osc2_id == 0xff) pSettings->osc2_voice = 1;
                                }
                            }
                        }
                    }
                }

                // 項目 osc2.unison が存在するか
                if(osc2.containsKey("unison")) {
                    JsonObject osc2_unison = osc2["unison"].as<JsonObject>();
                    // 項目 osc2.unison.voice が存在するか
                    if(osc2_unison.containsKey("voice")) {
                        // uint8_t型か
                        if(osc2_unison["voice"].is<uint8_t>()) {
                            uint8_t osc2_unison_voice = osc2_unison["voice"];
                            setVoice(synth, osc2_unison_voice, 0x02);
                            pSettings->osc2_voice = osc2_unison_voice;
                        }
                    }
                    // 項目 osc2.unison.detune が存在するか
                    if(osc2_unison.containsKey("detune")) {
                        // uint8_t型か
                        if(osc2_unison["detune"].is<uint8_t>()) {
                            uint8_t osc2_unison_detune = osc2_unison["detune"];
                            setDetune(synth, osc2_unison_detune, 0x02);
                            pSettings->osc2_detune = osc2_unison_detune;
                        }
                    }
                    // 項目 osc2.unison.spread が存在するか
                    if(osc2_unison.containsKey("spread")) {
                        // uint8_t型か
                        if(osc2_unison["spread"].is<uint8_t>()) {
                            uint8_t osc2_unison_spread = osc2_unison["spread"];
                            setSpread(synth, osc2_unison_spread, 0x02);
                            pSettings->osc2_spread = osc2_unison_spread;
                        }
                    }
                }

                // 項目 osc2.pitch が存在するか
                if(osc2.containsKey("pitch")) {
                    JsonObject osc2_pitch = osc2["pitch"].as<JsonObject>();
                    // 項目 osc2.pitch.octave が存在するか
                    if(osc2_pitch.containsKey("octave")) {
                        // int8_t型か
                        if(osc2_pitch["octave"].is<int8_t>()) {
                            int8_t osc2_pitch_octave = osc2_pitch["octave"];
                            setOscOctave(synth, 0x02, osc2_pitch_octave);
                            pSettings->osc2_oct = osc2_pitch_octave;
                        }
                    }
                    // 項目 osc2.pitch.semitone が存在するか
                    if(osc2_pitch.containsKey("semitone")) {
                        // int8_t型か
                        if(osc2_pitch["semitone"].is<int8_t>()) {
                            int8_t osc2_pitch_semitone = osc2_pitch["semitone"];
                            setOscSemitone(synth, 0x02, osc2_pitch_semitone);
                            pSettings->osc2_semi = osc2_pitch_semitone;
                        }
                    }
                    // 項目 osc2.pitch.cent が存在するか
                    if(osc2_pitch.containsKey("cent")) {
                        // int8_t型か
                        if(osc2_pitch["cent"].is<int8_t>()) {
                            int8_t osc2_pitch_cent = osc2_pitch["cent"];
                             setOscCent(synth, 0x02, osc2_pitch_cent);
                             pSettings->osc2_cent = osc2_pitch_cent;
                        }
                    }
                }

                // 項目 osc2.level が存在するか
                if(osc2.containsKey("level")){
                    // int16_t型か
                    if(osc2["level"].is<int16_t>()) {
                        int16_t osc2_level = osc2["level"];
                        setOscLevel(synth, 0x02, osc2_level);
                        pSettings->osc2_level = osc2_level;
                    }
                }

                // 項目 osc2.pan が存在するか
                if(osc2.containsKey("pan")) {
                    // todo
                }
            }

            //  ____        _        ___
            // / ___| _   _| |__    / _ \ ___  ___
            // \___ \| | | | '_ \  | | | / __|/ __|
            //  ___) | |_| | |_) | | |_| \__ \ (__
            // |____/ \__,_|_.__/   \___/|___/\___|

            // 項目 sub が存在するか
            if(doc.containsKey("sub")) {
                JsonObject sub = doc["sub"].as<JsonObject>();

                // 項目 sub.wavetable が存在するか
                if(sub.containsKey("wavetable")) {
                    JsonObject sub_wavetable = sub["wavetable"].as<JsonObject>();

                    // 項目 sub.wabetable.type が存在するか
                    if(sub_wavetable.containsKey("type")) {

                        // String型か
                        String sub_type;
                        if(sub_wavetable["type"].is<String>()) {
                            sub_type = sub_wavetable["type"].as<String>();
                        }

                        // ユーザーのwavetableの場合
                        if(sub_type == "custom") {
                            // 項目 sub.wavetable.path が存在するか
                            if(sub_wavetable.containsKey("path")) {

                                // String型か
                                if(sub_wavetable["path"].is<String>()) {
                                    String wave = sub_wavetable["path"];

                                    JsonDocument wt_doc;
                                    bool result = pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                                    if(!result) return;

                                    // 項目 wave_table が存在するか
                                    if(!wt_doc.containsKey("wave_table")) return;
                                    JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                                    copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                                    setShape(synth, 0x03, id, pSettings->wave_table_buff);
                                    pSettings->selectedWaveSub = FACTORY_PRESETS + 1;
                                }
                            }

                        // デフォルトのwavetableの場合
                        } else if(sub_type == "default") {
                            // 項目 sub.wavetable.path が存在するか
                            if(sub_wavetable.containsKey("path")) {

                                // uint8_t型か
                                if(sub_wavetable["path"].is<uint8_t>()) {
                                    uint8_t oscsub_id = sub_wavetable["path"];
                                    setShape(synth, 0x03, oscsub_id);
                                    pSettings->selectedWaveSub = oscsub_id;
                                }
                            }
                        }
                    }
                }

                // 項目 sub.pitch が存在するか
                if(sub.containsKey("pitch")) {
                    JsonObject sub_pitch = sub["pitch"].as<JsonObject>();
                    // 項目 sub.pitch.octave が存在するか
                    if(sub_pitch.containsKey("octave")) {
                        // int8_t型か
                        if(sub_pitch["octave"].is<int8_t>()) {
                            int8_t sub_pitch_octave = sub_pitch["octave"];
                            setOscOctave(synth, 0x03, sub_pitch_octave);
                            pSettings->osc_sub_oct = sub_pitch_octave;
                        }
                    }
                    // 項目 sub.pitch.semitone が存在するか
                    if(sub_pitch.containsKey("semitone")) {
                        // int8_t型か
                        if(sub_pitch["semitone"].is<int8_t>()) {
                            int8_t sub_pitch_semitone = sub_pitch["semitone"];
                            setOscSemitone(synth, 0x03, sub_pitch_semitone);
                            pSettings->osc_sub_semi = sub_pitch_semitone;
                        }
                    }
                    // 項目 sub.pitch.cent が存在するか
                    if(sub_pitch.containsKey("cent")) {
                        // int8_t型か
                        if(sub_pitch["cent"].is<int8_t>()) {
                            int8_t sub_pitch_cent = sub_pitch["cent"];
                            setOscCent(synth, 0x03, sub_pitch_cent);
                            pSettings->osc_sub_cent = sub_pitch_cent;
                        }
                    }
                }

                // 項目 sub.level が存在するか
                if(sub.containsKey("level")) {
                    // int16_t型か
                    if(sub["level"].is<int16_t>()) {
                        int16_t sub_level = sub["level"];
                        setOscLevel(synth, 0x03, sub_level);
                        pSettings->osc_sub_level = sub_level;
                    }
                }

                // 項目 sub.pan が存在するか
                if(sub.containsKey("pan")) {
                    // todo
                }
            }

            //  _   _       _
            // | \ | | ___ (_)___  ___
            // |  \| |/ _ \| / __|/ _ \
            // | |\  | (_) | \__ \  __/
            // |_| \_|\___/|_|___/\___|

            // 項目 noise が存在するか
            if(doc.containsKey("noise")) {
                // todo
            }

            //  __  __           _       _       _   _
            // |  \/  | ___   __| |_   _| | __ _| |_(_) ___  _ __
            // | |\/| |/ _ \ / _` | | | | |/ _` | __| |/ _ \| '_ \
            // | |  | | (_) | (_| | |_| | | (_| | |_| | (_) | | | |
            // |_|  |_|\___/ \__,_|\__,_|_|\__,_|\__|_|\___/|_| |_|

            // 項目 modulation が存在するか
            if(doc.containsKey("modulation")) {
                // String型か
                if(doc["modulation"].is<String>()) {
                    String modulation = doc["modulation"];
                    if(modulation == "enable") {
                        setMod(synth, 0x01);
                        pSettings->mod_status = 0x01;
                    }
                }
            }

            //     _                    _ _  __ _
            //    / \   _ __ ___  _ __ | (_)/ _(_) ___ _ __
            //   / _ \ | '_ ` _ \| '_ \| | | |_| |/ _ \ '__|
            //  / ___ \| | | | | | |_) | | |  _| |  __/ |
            // /_/   \_\_| |_| |_| .__/|_|_|_| |_|\___|_|
            //                   |_|

            // 項目 amp が存在するか
            if(doc.containsKey("amp")) {
                JsonObject amp = doc["amp"].as<JsonObject>();

                // 項目 envelope が存在するか
                if(amp.containsKey("envelope")) {
                    JsonObject envelope = amp["envelope"].as<JsonObject>();

                    // 項目 attack が存在するか
                    if(envelope.containsKey("attack")) {
                        // int16_t型か
                        if(envelope["attack"].is<int16_t>()) {
                            int16_t attack = envelope["attack"];
                            setAttack(synth, attack);
                            pSettings->attack = attack;
                        }
                    }

                    // 項目 decay が存在するか
                    if(envelope.containsKey("decay")) {
                        // int16_t型か
                        if(envelope["decay"].is<int16_t>()) {
                            int16_t decay = envelope["decay"];
                            setDecay(synth, decay);
                            pSettings->decay = decay;
                        }
                    }

                    // 項目 sustain が存在するか
                    if(envelope.containsKey("sustain")) {
                        // int16_t型か
                        if(envelope["sustain"].is<int16_t>()) {
                            int16_t sustain = envelope["sustain"];
                            setSustain(synth, sustain);
                            pSettings->sustain = sustain;
                        }
                    }

                    // 項目 release が存在するか
                    if(envelope.containsKey("release")) {
                        // int16_t型か
                        if(envelope["release"].is<int16_t>()) {
                            int16_t release = envelope["release"];
                            setRelease(synth, release);
                            pSettings->release = release;
                        }
                    }
                }

                // 項目 glide が存在するか
                if(amp.containsKey("glide")) {
                    // todo
                }

                // 項目 level が存在するか
                if(amp.containsKey("level")) {
                    // todo
                }

                // 項目 pan が存在するか
                if(amp.containsKey("pan")) {
                    // todo
                }
            }

            //  _____ _ _ _
            // |  ___(_) | |_ ___ _ __
            // | |_  | | | __/ _ \ '__|
            // |  _| | | | ||  __/ |
            // |_|   |_|_|\__\___|_|

            // 項目 filter が存在するか
            if(doc.containsKey("filter")) {
                JsonObject filter = doc["filter"].as<JsonObject>();

                // 項目 mode が存在するか
                if(filter.containsKey("mode")) {
                    // String型か
                    if(filter["mode"].is<String>()) {
                        String filter_mode = filter["mode"];
                        if(filter_mode == "lpf") {
                            // 項目 lpf が存在するか
                            if(filter.containsKey("lpf")) {
                                JsonObject lpf = filter["lpf"].as<JsonObject>();
                                // 項目 freq と q が存在するか
                                if(lpf.containsKey("freq") && lpf.containsKey("q")) {
                                    // float型
                                    if(lpf["freq"].is<float>() && lpf["q"].is<float>()) {
                                        float lpf_freq = lpf["freq"];
                                        float lpf_q = lpf["q"];
                                        setLowPassFilter(synth, 0x01, lpf_freq, lpf_q);
                                        pSettings->filter_mode = 0x01;
                                        pSettings->lpf_freq = lpf_freq;
                                        pSettings->lpf_q = lpf_q;
                                    }
                                }
                                else {
                                    setLowPassFilter(synth, 0x01);
                                    pSettings->filter_mode = 0x01;
                                    pSettings->lpf_freq = 1000.0f;
                                    pSettings->lpf_q = 1.0f/sqrt(2.0f);
                                }
                            }
                            else {
                                setLowPassFilter(synth, 0x01);
                                pSettings->filter_mode = 0x01;
                                pSettings->lpf_freq = 1000.0f;
                                pSettings->lpf_q = 1.0f/sqrt(2.0f);
                            }
                        }
                        else if(filter_mode == "hpf") {
                            // 項目 hpf が存在するか
                            if(filter.containsKey("hpf")) {
                                JsonObject hpf = filter["hpf"].as<JsonObject>();
                                // 項目 freq と q が存在するか
                                if(hpf.containsKey("freq") && hpf.containsKey("q")) {
                                    // float型
                                    if(hpf["freq"].is<float>() && hpf["q"].is<float>()) {
                                        float hpf_freq = hpf["freq"];
                                        float hpf_q = hpf["q"];
                                        setHighPassFilter(synth, 0x01, hpf_freq, hpf_q);
                                        pSettings->filter_mode = 0x02;
                                        pSettings->hpf_freq = hpf_freq;
                                        pSettings->hpf_q = hpf_q;
                                    }
                                }
                                else {
                                    setHighPassFilter(synth, 0x01);
                                    pSettings->filter_mode = 0x02;
                                    pSettings->hpf_freq = 1000.0f;
                                    pSettings->hpf_q = 1.0f/sqrt(2.0f);
                                }
                            }
                            else {
                                setHighPassFilter(synth, 0x01);
                                pSettings->filter_mode = 0x02;
                                pSettings->hpf_freq = 1000.0f;
                                pSettings->hpf_q = 1.0f/sqrt(2.0f);
                            }
                        }
                        else if(filter_mode == "lpf+hpf") {
                            // 項目 lpf と hpf が存在するか
                            if(filter.containsKey("lpf") && filter.containsKey("hpf")) {
                                JsonObject lpf = filter["lpf"].as<JsonObject>();
                                JsonObject hpf = filter["hpf"].as<JsonObject>();
                                // 項目 freq と q が存在するか
                                if(lpf.containsKey("freq") && lpf.containsKey("q") && hpf.containsKey("freq") && hpf.containsKey("q")) {
                                    // float型
                                    if(lpf["freq"].is<float>() && lpf["q"].is<float>() && hpf["freq"].is<float>() && hpf["q"].is<float>()) {
                                        float lpf_freq = lpf["freq"];
                                        float lpf_q = lpf["q"];
                                        float hpf_freq = hpf["freq"];
                                        float hpf_q = hpf["q"];
                                        setLowPassFilter(synth, 0x01, lpf_freq, lpf_q);
                                        setHighPassFilter(synth, 0x01, hpf_freq, hpf_q);
                                        pSettings->filter_mode = 0x03;
                                        pSettings->lpf_freq = lpf_freq;
                                        pSettings->lpf_q = lpf_q;
                                        pSettings->hpf_freq = hpf_freq;
                                        pSettings->hpf_q = hpf_q;
                                    }
                                }
                                else {
                                    setLowPassFilter(synth, 0x01);
                                    setHighPassFilter(synth, 0x01);
                                    pSettings->filter_mode = 0x03;
                                    pSettings->lpf_freq = 1000.0f;
                                    pSettings->lpf_q = 1.0f/sqrt(2.0f);
                                    pSettings->hpf_freq = 1000.0f;
                                    pSettings->hpf_q = 1.0f/sqrt(2.0f);
                                }
                            }
                            else {
                                setLowPassFilter(synth, 0x01);
                                setHighPassFilter(synth, 0x01);
                                pSettings->filter_mode = 0x03;
                                pSettings->lpf_freq = 1000.0f;
                                pSettings->lpf_q = 1.0f/sqrt(2.0f);
                                pSettings->hpf_freq = 1000.0f;
                                pSettings->hpf_q = 1.0f/sqrt(2.0f);
                            }
                        }
                    }
                }
            }

            //  ____       _
            // |  _ \  ___| | __ _ _   _
            // | | | |/ _ \ |/ _` | | | |
            // | |_| |  __/ | (_| | |_| |
            // |____/ \___|_|\__,_|\__, |
            //                     |___/

            // 項目 delay が存在するか
            if(doc.containsKey("delay")) {
                JsonObject delay = doc["delay"].as<JsonObject>();

                // 項目 mode が存在するか
                if(delay.containsKey("mode")) {
                    String delay_mode = delay["mode"];
                    if(delay_mode == "enable") {
                        // 項目 time level feedback が存在するか
                        if(delay.containsKey("time") && delay.containsKey("level") && delay.containsKey("feedback")) {
                            // int16_t型
                            if(delay["time"].is<int16_t>() && delay["level"].is<int16_t>() && delay["feedback"].is<int16_t>()) {
                                int16_t delay_time = delay["time"];
                                int16_t delay_level = delay["level"];
                                int16_t delay_feedback = delay["feedback"];
                                setDelay(synth, 0x01, delay_time, delay_level, delay_feedback);
                                pSettings->delay_status = 0x01;
                                pSettings->delay_time = delay_time;
                                pSettings->delay_level = delay_level;
                                pSettings->delay_feedback = delay_feedback;
                            }
                        }
                        else {
                            setDelay(synth, 0x01);
                            pSettings->delay_status = 0x01;
                            pSettings->delay_time = 250;
                            pSettings->delay_level = 300;
                            pSettings->delay_feedback = 500;
                        }
                    }
                }
            }
        }
    }
};

#endif // SYNTHMANAGER_H