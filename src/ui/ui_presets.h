#include <IUIHandler.h>

#ifndef UIPRESETS_H
#define UIPRESETS_H

class UIPresets : public IUIHandler {
private:

    // ディスプレイ関連
    uint8_t* displayStatus;
    uint8_t* displayCursor;

    LGFX_Sprite* pSprite;

    SynthManager* pSynth;
    FileManager* pFile;

    Settings* pSettings;

    uint8_t* selectedSynth;

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    // todo: synth manager に移動する。
    void setPreset(uint8_t id, uint8_t synth) {

        // 全ての項目を初期値に設定する
        // todo

        // defaultプリセットはosc=0x01固定
        if(id < FACTORY_PRESETS) {
            pSynth->setShape(synth, 0x01, id);
            pSettings->selectedWave = id;
            if(id == 0xff) pSettings->osc1_voice = 1;
        }

        // ユーザープリセット
        else {
            JsonDocument doc;
            pFile->getJson(&doc, pSettings->user_presets[id - FACTORY_PRESETS].path);

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
                        } else {
                            osc1_type = "disable";
                        }

                        // ユーザーのwavetableの場合
                        if(osc1_type == "custom") {
                            // 項目 osc1.wavetable.path が存在するか
                            if(osc1_wavetable.containsKey("path")) {

                                // String型か
                                if(osc1_wavetable["path"].is<String>()) {
                                    String wave = osc1_wavetable["path"];

                                    // todo: エラーハンドリング
                                    JsonDocument wt_doc;
                                    pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                                    JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                                    copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                                    pSynth->setShape(synth, 0x01, id, pSettings->wave_table_buff);
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

                                    pSynth->setShape(synth, 0x01, osc1_id);
                                    pSettings->selectedWave = osc1_id;
                                    if(osc1_id == 0xff) pSettings->osc1_voice = 1;
                                }
                            }
                        }
                    }
                }

                // 項目 osc1.unison が存在するか
                if(osc1.containsKey("unison")) {
                    uint8_t osc1_unison_voice = doc["osc1"]["unison"]["voice"];
                    uint8_t osc1_unison_detune = doc["osc1"]["unison"]["detune"];
                    uint8_t osc1_unison_spread = doc["osc1"]["unison"]["spread"];
                    pSynth->setVoice(synth, osc1_unison_voice, 0x01);
                    pSynth->setDetune(synth, osc1_unison_detune, 0x01);
                    pSynth->setSpread(synth, osc1_unison_spread, 0x01);
                }

                // 項目 osc1.pitch が存在するか
                if(osc1.containsKey("pitch")) {
                    int8_t osc1_pitch_octave = doc["osc1"]["pitch"]["octave"];
                    int8_t osc1_pitch_semitone = doc["osc1"]["pitch"]["semitone"];
                    int8_t osc1_pitch_cent = doc["osc1"]["pitch"]["cent"];
                    pSynth->setOscOctave(synth, 0x01, osc1_pitch_octave);
                    pSynth->setOscSemitone(synth, 0x01, osc1_pitch_semitone);
                    pSynth->setOscCent(synth, 0x01, osc1_pitch_cent);
                }

                // 項目 osc1.level が存在するか
                if(osc1.containsKey("level")) {
                    int16_t osc1_level = doc["osc1"]["level"];
                    pSynth->setOscLevel(synth, 0x01, osc1_level);
                }

                // 項目 osc1.pan が存在するか
                if(osc1.containsKey("pan")) {
                    // todo
                }
            }

            // OSC2 Wavetable
            String osc2_type = doc["osc2"]["wavetable"]["type"];
            if(osc2_type == "custom") {
                String wave = doc["osc2"]["wavetable"]["path"];

                JsonDocument wt_doc;
                pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                pSynth->setShape(synth, 0x02, id, pSettings->wave_table_buff);
                pSettings->selectedWave2 = FACTORY_PRESETS + 1;

            } else if(osc2_type == "default") {
                uint8_t osc2_id = doc["osc2"]["wavetable"]["path"];
                pSynth->setShape(synth, 0x02, osc2_id);
                pSettings->selectedWave2 = osc2_id;
                if(osc2_id == 0xff) pSettings->osc2_voice = 1;
            }

            // OSC2 Unison*
            uint8_t osc2_unison_voice = doc["osc2"]["unison"]["voice"];
            uint8_t osc2_unison_detune = doc["osc2"]["unison"]["detune"];
            uint8_t osc2_unison_spread = doc["osc2"]["unison"]["spread"];
            // pSynth->setVoice(synth, osc2_unison_voice, 0x02);
            // pSynth->setDetune(synth, osc2_unison_detune, 0x02);
            // pSynth->setSpread(synth, osc2_unison_spread, 0x02);

            // OSC2 Pitch*
            int8_t osc2_pitch_octave = doc["osc2"]["pitch"]["octave"];
            int8_t osc2_pitch_semitone = doc["osc2"]["pitch"]["semitone"];
            int8_t osc2_pitch_cent = doc["osc2"]["pitch"]["cent"];
            // pSynth->setOscOctave(synth, 0x02, osc2_pitch_octave);
            // pSynth->setOscSemitone(synth, 0x02, osc2_pitch_semitone);
            // pSynth->setOscCent(synth, 0x02, osc2_pitch_cent);

            // OSC2 Level*
            int16_t osc2_level = doc["osc2"]["level"];
            // pSynth->setOscLevel(synth, 0x02, osc2_level);

            // OSC2 Pan
            // todo

            // SUBOSC Wavetable
            String sub_type = doc["sub"]["wavetable"]["type"];
            if(sub_type == "custom") {
                // String wave = doc["sub"]["wavetable"]["path"];

                // JsonDocument wt_doc;
                // pFile->getJson(&wt_doc, "/rp-ds16/wavetable/" + wave);
                // JsonArray waveTableArray = wt_doc["wave_table"].as<JsonArray>();
                // copyArray(waveTableArray, pSettings->wave_table_buff, waveTableArray.size());

                // pSynth->setShape(synth, 0x03, id, pSettings->wave_table_buff);
                // pSettings->selectedWaveSub = FACTORY_PRESETS + 1;

            } else if(sub_type == "default") {
                // uint8_t oscsub_id = doc["sub"]["wavetable"]["path"];
                // pSynth->setShape(synth, 0x03, oscsub_id);
                // pSettings->selectedWaveSub = oscsub_id;
            }

            // SUBOSC Pitch*
            int8_t sub_pitch_octave = doc["sub"]["pitch"]["octave"];
            int8_t sub_pitch_semitone = doc["sub"]["pitch"]["semitone"];
            int8_t sub_pitch_cent = doc["sub"]["pitch"]["cent"];
            // pSynth->setOscOctave(synth, 0x03, sub_pitch_octave);
            // pSynth->setOscSemitone(synth, 0x03, sub_pitch_semitone);
            // pSynth->setOscCent(synth, 0x03, sub_pitch_cent);

            // SUBOSC Level*
            int16_t sub_level = doc["sub"]["level"];
            // pSynth->setOscLevel(synth, 0x03, sub_level);

            // SUBOSC Pan
            // todo

            // Noise
            // todo

            // Modulation*
            String modulation = doc["modulation"];
            if(modulation == "enable") {
                // pSynth->setMod(synth, 0x01);
            }

            // Amplifier Envelope*
            int16_t attack = doc["amp"]["envelope"]["attack"];
            int16_t decay = doc["amp"]["envelope"]["decay"];
            int16_t sustain = doc["amp"]["envelope"]["sustain"];
            int16_t release = doc["amp"]["envelope"]["release"];
            // pSynth->setAttack(synth, attack);
            // pSynth->setAttack(synth, decay);
            // pSynth->setAttack(synth, sustain);
            // pSynth->setAttack(synth, release);

            // Amplifier Glide
            // todo

            // Amplifier Level
            // todo

            // Amplifier Pan
            // todo

            // Filter*
            String filter_mode = doc["filter"]["mode"];
            float lpf_freq = doc["filter"]["lpf"]["freq"];
            float lpf_q = doc["filter"]["lpf"]["q"];
            float hpf_freq = doc["filter"]["hpf"]["freq"];
            float hpf_q = doc["filter"]["hpf"]["q"];
            if(filter_mode == "lpf") {
                // pSynth->setLowPassFilter(synth, 0x01, lpf_freq, lpf_q);
            }
            else if(filter_mode == "hpf") {
                // pSynth->setHighPassFilter(synth, 0x01, hpf_freq, hpf_q);
            }
            else if(filter_mode == "lpf+hpf") {
                // pSynth->setLowPassFilter(synth, 0x01, lpf_freq, lpf_q);
                // pSynth->setHighPassFilter(synth, 0x01, hpf_freq, hpf_q);
            }

            // Delay*
            String delay_mode = doc["delay"]["mode"];
            int16_t delay_time = doc["delay"]["time"];
            int16_t delay_level = doc["delay"]["level"];
            int16_t delay_feedback = doc["delay"]["feedback"];
            if(delay_mode == "enable") {
                //pSynth->setDelay(synth, 0x01, delay_time, delay_level, delay_feedback);
            }

            // todo: それぞれのプリセット値を反映する(送信項目が多すぎてバグる)
        }
    }

public:
    UIPresets(
        LGFX_Sprite* pSprite, SynthManager* pSynth, FileManager* pFile,
        uint8_t* displayStatus, uint8_t* displayCursor,
        uint8_t* selectedSynth, Settings* pSettings)
    {
        this->pSprite = pSprite;
        this->pSynth = pSynth;
        this->pFile = pFile;
        this->displayStatus = displayStatus;
        this->displayCursor = displayCursor;
        this->selectedSynth = selectedSynth;
        this->pSettings = pSettings;
    }

    // todo: 展示用プリセット読み込み
    bool loaded = false;
    // ここまで

    /** @brief 画面更新 */
    void refreshUI() override {
        // todo: 展示用プリセット読み込み
        if(!loaded) {
            pSettings->selectedPreset = 3;
            setPreset(pSettings->selectedPreset, 0xff);
            loaded = true;
        }
        // ここまで

        // シンセ選択状態リセット
        *selectedSynth = 0x00;

        uint8_t preset_x = pSprite->textWidth(" ");
        uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
        char idstr[5];
        char idstr2[5];
        String fu1 = "F";
        String fu2 = "F";

        String preset_name1, preset_name2;
        if(pSettings->selectedPreset < FACTORY_PRESETS) {
            sprintf(idstr, "%03d ", pSettings->selectedPreset+1);
            preset_name1 = pSettings->default_presets[pSettings->selectedPreset];
        }
        else {
            fu1 = "U";
            sprintf(idstr, "%03d ", pSettings->selectedPreset+1 - FACTORY_PRESETS);
            preset_name1 = pSettings->user_presets[pSettings->selectedPreset - FACTORY_PRESETS].name;
        }
        if(pSettings->selectedPreset2 < FACTORY_PRESETS) {
            sprintf(idstr2, "%03d ", pSettings->selectedPreset2+1);
            preset_name2 = pSettings->default_presets[pSettings->selectedPreset2];
        }
        else {
            fu2 = "U";
            sprintf(idstr2, "%03d ", pSettings->selectedPreset2+1 - FACTORY_PRESETS);
            preset_name2 = pSettings->user_presets[pSettings->selectedPreset2 - FACTORY_PRESETS].name;
        }

        if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y - 7);
            pSprite->drawString(fu2 + idstr2 + preset_name2, preset_x, preset_y + 7);
        }
        else {
            pSprite->drawString(fu1 + idstr + preset_name1, preset_x, preset_y);
        }

        // MIDIチャンネル
        if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_MONO){
            pSprite->drawString("MIDI=1&2", 2, 2);
        }else{
            pSprite->drawString("MIDI=1", 2, 2);
        }

        // シンセモード
        uint8_t synth_x = pSprite->textWidth(pSettings->modes[pSettings->synthMode]);
        pSprite->drawString(pSettings->modes[pSettings->synthMode], 128 - 2 - synth_x, 2);

        // 横線
        pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
        pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

        // メニュー
        uint8_t menu_x = pSprite->textWidth(">>");
        pSprite->drawString(">>", 128 - 2 - menu_x, 55);

        // 塗り
        if(*displayCursor == 0x01) {

            uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[6];

            String fu = "F";
            if(pSettings->selectedPreset < FACTORY_PRESETS) {
                sprintf(idstr, "%03d", pSettings->selectedPreset+1);
            }
            else {
                fu = "U";
                sprintf(idstr, "%03d", pSettings->selectedPreset+1 - FACTORY_PRESETS);
            }

            if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL)
                cursorText(" " + fu1 + idstr, 0, y - 7);
            else
                cursorText(" " + fu1 + idstr, 0, y);
        }
        else if(*displayCursor == 0x02) {
            uint8_t synth_x = pSprite->textWidth(pSettings->modes[pSettings->synthMode]);
            cursorText(pSettings->modes[pSettings->synthMode], 128 - 2 - synth_x, 2);
        }
        else if(*displayCursor == 0x03) {
            // blank
        }
        else if(*displayCursor == 0x04) {
            if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr2[5];

                String fu2 = "F";
                if(pSettings->selectedPreset2 < FACTORY_PRESETS) {
                    sprintf(idstr2, "%03d", pSettings->selectedPreset2+1);
                }
                else {
                    fu2 = "U";
                    sprintf(idstr2, "%03d", pSettings->selectedPreset2+1 - FACTORY_PRESETS);
                }

                cursorText(" " + fu2 + idstr2, 0, y + 7);
            }
        }
        else if(*displayCursor == 0x05) {
            uint8_t menu_x = pSprite->textWidth(">>");
            cursorText(">>", 128 - 2 - menu_x, 55);
        }
    }

    /** @brief 上ボタンが押された場合 */
    void handleButtonUp(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x00:
            case 0x01:
            case 0x02:
                (*displayCursor)++;
                break;
            case 0x03:
                *displayCursor = 0x05;
                break;
            case 0x04:
                *displayCursor = 0x01;
                break;
            case 0x05:
                *displayCursor = (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
                break;
        }
    }

    /** @brief 下ボタンが押された場合 */
    void handleButtonDown(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x00:
            case 0x02:
                *displayCursor = 0x01;
                break;
            case 0x01:
                *displayCursor = (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x04 : 0x05;
                break;
            case 0x03:
                *displayCursor = 0x02;
                break;
            case 0x04:
                *displayCursor = 0x05;
                break;
            case 0x05:
                *displayCursor = 0x03;
                break;
        }
    }

    /** @brief 左ボタンが押された場合 */
    void handleButtonLeft(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                pSettings->selectedPreset = (pSettings->selectedPreset != 0x00) ? (pSettings->selectedPreset - 1) : 128 + FACTORY_PRESETS - 1;
                if(!longPush) {
                    setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                pSettings->synthMode = (pSettings->synthMode == SYNTH_POLY) ? SYNTH_MULTI : (pSettings->synthMode - 1);
                pSynth->setGlideMode(0xff, false); // monophonic, glideは必ず全てのシンセに送信する
                pSettings->isGlide = false;
                pSynth->setSynthMode(pSettings->synthMode);
                if(pSettings->synthMode == SYNTH_MONO){
                    pSynth->setMonophonic(0xff, true);
                } else {
                    pSynth->setMonophonic(0xff, false);
                }
                setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                    setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                pSettings->selectedPreset2 = (pSettings->selectedPreset2 != 0x00) ? (pSettings->selectedPreset2 - 1) : 128 + FACTORY_PRESETS - 1;
                if(!longPush) {
                    setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 右ボタンが押された場合 */
    void handleButtonRight(bool longPush = false) override {
        switch (*displayCursor) {
            case 0x01:
                pSettings->selectedPreset = (pSettings->selectedPreset != 128 + FACTORY_PRESETS - 1) ? (pSettings->selectedPreset + 1) : 0x00;
                if(!longPush) {
                    setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                }
                break;
            case 0x02:
                if (longPush) return;
                pSettings->synthMode = (pSettings->synthMode == SYNTH_MULTI) ? SYNTH_POLY : (pSettings->synthMode + 1);
                pSynth->setGlideMode(0xff, false); // monophonic, glideは必ず全てのシンセに送信する
                pSettings->isGlide = false;
                pSynth->setSynthMode(pSettings->synthMode);
                if(pSettings->synthMode == SYNTH_MONO){
                    pSynth->setMonophonic(0xff, true);
                } else {
                    pSynth->setMonophonic(0xff, false);
                }
                setPreset(pSettings->selectedPreset, (pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) ? 0x01 : 0xff);
                if(pSettings->synthMode == SYNTH_MULTI || pSettings->synthMode == SYNTH_DUAL) {
                    setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
            case 0x04:
                pSettings->selectedPreset2 = (pSettings->selectedPreset2 != 128 + FACTORY_PRESETS - 1) ? (pSettings->selectedPreset2 + 1) : 0x00;
                if(!longPush) {
                    setPreset(pSettings->selectedPreset2, 0x02);
                }
                break;
        }
    }

    /** @brief 決定ボタンが押された場合 */
    void handleButtonEnter(bool longPush = false) override {
        if(longPush) return;
        switch (*displayCursor) {
            case 0x02:
                pSynth->resetSynth(0xff); // 全てリセット(MIDI PANIC)
                break;
            case 0x01:
                *displayCursor = 0x01;
                *displayStatus = DISPST_PRESET_EDIT;
                // dual又はmultiモードはシンセ1を選択それ以外はブロードキャスト
                if(pSettings->synthMode == SYNTH_DUAL || pSettings->synthMode == SYNTH_MULTI) {
                    *selectedSynth = 0x01;
                }
                else {
                    *selectedSynth = 0xff;
                }
                break;
            case 0x04:
                *displayCursor = 0x01;
                *displayStatus = DISPST_PRESET_EDIT;
                // dual又はmultiモード限定のためシンセ2を選択
                *selectedSynth = 0x02;
                break;
            case 0x00:
                *displayCursor = 0x01;
                break;
            case 0x05:
                *displayCursor = 0x01;
                *displayStatus = DISPST_MENU;
                break;
        }
    }

    /** @brief キャンセルボタンが押された場合 */
    void handleButtonCancel(bool longPush = false) override {
        if (longPush) return;
        *displayCursor = 0x00;
    }
};

#endif // UIPRESETS_H