#include <IUIHandler.h>

#ifndef SETTINGS_H
#define SETTINGS_H

// #define FILTER_NONE 0x00
// #define FILTER_LPF  0x01
// #define FILTER_HPF  0x02
// #define FILTER_LHF  0x03

class Settings {
private:
public:
    Settings(void){}

    // シンセ関連 DUAL又はMULTIの場合、プリセット編集は0x01のみに限る
    uint8_t synthMode = SYNTH_POLY;
    uint8_t selectedPreset = 0x00;
    uint8_t selectedPreset2 = 0x00;

    uint8_t selectedWave = 0x00;
    uint8_t selectedWave2 = 0xff;
    uint8_t selectedWaveSub = 0xff;

    int16_t amp_gain = 1000;
    uint8_t pan = 50;

    int16_t attack = 1;
    int16_t decay = 1000;
    int16_t sustain = 1000; // max=1000
    int16_t release = 10;

    uint8_t osc1_voice = 1;
    uint8_t osc2_voice = 1;
    uint8_t osc1_detune = 20;
    uint8_t osc2_detune = 20;
    uint8_t osc1_spread = 0;
    uint8_t osc2_spread = 0;
    int8_t osc1_oct = 0;
    int8_t osc2_oct = 0;
    int8_t osc1_semi = 0;
    int8_t osc2_semi = 0;
    int8_t osc1_cent = 0;
    int8_t osc2_cent = 0;
    int16_t osc1_level = 1000;
    int16_t osc2_level = 1000;

    int8_t osc_sub_oct = 0;
    int8_t osc_sub_semi = 0;
    int8_t osc_sub_cent = 0;
    int16_t osc_sub_level = 1000;

    uint8_t filter_mode = 0x00; // None
    float lpf_freq = 1000.0f;
    float lpf_q = 1.0f/sqrt(2.0f);
    float hpf_freq = 500.0f;
    float hpf_q = 1.0f/sqrt(2.0f);

    int16_t delay_time = 250;
    int16_t delay_level = 300;
    int16_t delay_feedback = 500;
    uint8_t delay_status = 0x00;

    uint8_t mod_status = 0x00;

    bool isGlide = false;
    uint16_t glide_time = 15;

    bool isFirst = false;

    String default_presets[FACTORY_PRESETS] = {
        "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
    };
    String default_wavetables[FACTORY_WAVETABLES] = {
        "sine", "triangle", "saw", "square"
    };
    String modes[4] = {
        "POLY MODE", "MONO MODE", "DUAL MODE", "MULTI MODE"
    };

    // ユーザーファイル
    Preset user_presets[USER_PRESET_LIMIT];
    bool isUserPresetLoaded = false;
    Preset user_wavetables[USER_PRESET_LIMIT];
    bool isUserWaveLoaded = false;
    Preset midi_files[USER_PRESET_LIMIT];
    bool isMidiLoaded = false;
    Preset rlem_files[USER_PRESET_LIMIT];
    bool isRlemLoaded = false;

    // プリセット用バッファ
    int16_t wave_table_buff[2048];
};

#endif // SETTINGS_H