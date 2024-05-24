#include <ctrl_manager.h>
#include <synth_manager.h>
#include <file_manager.h>
#include <ArduinoJson.h>
#include <graphics.h>
#include <wokwi.h>

#ifndef UIMANAGER_H
#define UIMANAGER_H

#define BUTTON_COUNT 6
#define BUTTON_PINS {10,11,12,13,14,15}

#define BTN_NONE   0x00
#define BTN_UP     0x01
#define BTN_DOWN   0x02
#define BTN_LEFT   0x03
#define BTN_RIGHT  0x04
#define BTN_ENTER  0x05
#define BTN_CANCEL 0x06

#define BTN_LONG_UP     0x11
#define BTN_LONG_DOWN   0x12
#define BTN_LONG_LEFT   0x13
#define BTN_LONG_RIGHT  0x14
#define BTN_LONG_ENTER  0x15
#define BTN_LONG_CANCEL 0x16

#define DISPST_IDLE    0x00
#define DISPST_TITLE   0x01
#define DISPST_PRESETS 0x02
#define DISPST_DETAIL  0x03
#define DISPST_DEBUG   0x04
#define DISPST_MENU    0x05
#define DISPST_FILEMAN 0x06

#if WOKWI_MODE != 1
    #define PUSH_SHORT  200
    #define LONG_TOGGLE 60000
    #define PUSH_LONG   50000
#else
    #define PUSH_SHORT  1
    #define LONG_TOGGLE 5
    #define PUSH_LONG   5
#endif

class UIManager {
private:
    TwoWire& ctrl = Wire1;

    // ボタン関連
    volatile bool buttonPressed = false;
    uint8_t pressedButton = BTN_NONE;
    int buttonPins[BUTTON_COUNT] = BUTTON_PINS;
    uint8_t long_count_to_enter_debug_mode = 0;

    // ディスプレイ関連
    uint8_t displayStatus = DISPST_IDLE;
    uint8_t displayCursor = 0x00;

    // シンセ関連
    uint8_t synthMode = SYNTH_SINGLE;
    uint8_t synthPan = LR_PAN_C;
    uint8_t selectedPreset = 0x00;
    uint8_t selectedPreset2 = 0x00;

    int16_t attack = 1;
    int16_t decay = 1000;
    int16_t sustain = 1000; // max=1000
    int16_t release = 1;

    //todo
    int16_t new_wave[2048] = {0,211,423,635,847,1058,1270,1481,1693,1904,2115,2326,2537,2748,2959,3169,3379,3590,3799,4009,4218,4427,4636,4845,5053,5261,5469,5676,5883,6089,6296,6502,6707,6912,7117,7321,7525,7728,7931,8133,8335,8536,8737,8938,9138,9337,9536,9734,9931,10128,10325,10520,10715,10910,11104,11297,11490,11681,11873,12063,12253,12442,12630,12818,13004,13191,13376,13560,13744,13927,14109,14290,14471,14650,14829,15007,15184,15360,15536,15710,15884,16056,16228,16399,16569,16738,16906,17073,17239,17404,17568,17731,17893,18054,18215,18374,18532,18689,18845,19000,19154,19307,19459,19610,19760,19908,20056,20202,20348,20492,20636,20778,20919,21059,21198,21335,21472,21607,21742,21875,22007,22138,22268,22396,22524,22650,22775,22899,23022,23144,23264,23384,23502,23619,23735,23849,23963,24075,24186,24296,24405,24512,24618,24724,24828,24930,25032,25132,25232,25330,25427,25522,25617,25710,25802,25893,25983,26071,26159,26245,26330,26414,26496,26578,26658,26737,26815,26892,26968,27043,27116,27188,27259,27329,27398,27465,27532,27597,27661,27725,27787,27847,27907,27966,28023,28080,28135,28189,28243,28295,28346,28396,28444,28492,28539,28585,28629,28673,28716,28757,28798,28837,28876,28913,28950,28985,29020,29053,29086,29118,29148,29178,29207,29235,29262,29288,29313,29337,29360,29383,29404,29425,29444,29463,29481,29499,29515,29531,29545,29559,29572,29585,29596,29607,29617,29626,29634,29642,29649,29655,29661,29666,29670,29673,29676,29678,29679,29680,29680,29679,29678,29676,29674,29671,29667,29663,29658,29653,29647,29641,29634,29626,29618,29609,29600,29591,29581,29570,29559,29548,29536,29524,29511,29498,29484,29470,29456,29441,29426,29410,29395,29378,29362,29345,29328,29310,29293,29274,29256,29238,29219,29199,29180,29160,29141,29121,29100,29080,29059,29038,29017,28996,28975,28953,28931,28910,28888,28866,28844,28821,28799,28777,28754,28732,28709,28686,28664,28641,28618,28595,28572,28550,28527,28504,28481,28458,28435,28413,28390,28367,28345,28322,28300,28278,28255,28233,28211,28189,28167,28145,28124,28102,28081,28059,28038,28017,27996,27976,27955,27935,27915,27895,27875,27855,27836,27816,27797,27778,27760,27741,27723,27705,27687,27670,27653,27636,27619,27602,27586,27570,27554,27538,27523,27508,27493,27479,27464,27450,27437,27423,27410,27397,27385,27372,27360,27348,27337,27326,27315,27304,27294,27284,27274,27265,27256,27247,27238,27230,27222,27214,27207,27200,27193,27186,27180,27174,27168,27163,27158,27153,27149,27145,27141,27137,27134,27130,27128,27125,27123,27121,27119,27118,27116,27115,27115,27114,27114,27114,27114,27115,27116,27117,27118,27119,27121,27123,27125,27128,27130,27133,27136,27139,27142,27146,27150,27154,27158,27162,27166,27171,27176,27181,27186,27191,27196,27202,27207,27213,27219,27225,27231,27237,27243,27249,27256,27262,27269,27275,27282,27289,27295,27302,27309,27316,27323,27329,27336,27343,27350,27357,27364,27370,27377,27384,27391,27397,27404,27410,27417,27423,27429,27435,27441,27447,27453,27459,27464,27469,27475,27480,27485,27489,27494,27498,27503,27507,27510,27514,27517,27520,27523,27526,27528,27530,27532,27534,27535,27536,27537,27537,27537,27537,27537,27536,27535,27533,27531,27529,27526,27523,27520,27516,27512,27507,27502,27497,27491,27484,27478,27471,27463,27455,27446,27437,27428,27418,27407,27396,27385,27373,27360,27347,27334,27319,27305,27290,27274,27257,27240,27223,27205,27186,27167,27147,27127,27106,27084,27062,27039,27015,26991,26966,26941,26915,26888,26860,26832,26803,26774,26744,26713,26682,26649,26616,26583,26549,26514,26478,26441,26404,26366,26328,26289,26249,26208,26166,26124,26081,26037,25993,25947,25901,25855,25807,25759,25710,25660,25609,25558,25506,25453,25400,25345,25290,25234,25177,25120,25061,25002,24942,24882,24820,24758,24695,24631,24567,24501,24435,24368,24301,24232,24163,24093,24022,23951,23878,23805,23732,23657,23582,23505,23428,23351,23272,23193,23113,23032,22951,22869,22786,22702,22618,22532,22447,22360,22273,22185,22096,22006,21916,21825,21734,21641,21548,21454,21360,21265,21169,21073,20976,20878,20780,20681,20581,20481,20380,20278,20176,20073,19970,19866,19761,19656,19550,19444,19337,19229,19121,19012,18903,18794,18683,18573,18461,18350,18237,18124,18011,17897,17783,17669,17553,17438,17322,17205,17088,16971,16853,16735,16617,16498,16379,16259,16139,16019,15898,15777,15656,15534,15412,15290,15168,15045,14922,14799,14675,14551,14427,14303,14179,14054,13929,13804,13679,13554,13429,13303,13177,13051,12925,12799,12673,12547,12421,12294,12168,12042,11915,11789,11662,11536,11409,11283,11157,11030,10904,10778,10652,10526,10400,10274,10148,10023,9897,9772,9647,9522,9397,9272,9148,9024,8900,8776,8652,8529,8406,8283,8161,8039,7917,7795,7674,7553,7433,7313,7193,7073,6954,6836,6717,6599,6482,6365,6248,6132,6017,5902,5787,5673,5559,5446,5333,5221,5109,4998,4888,4778,4668,4559,4451,4344,4236,4130,4024,3919,3815,3711,3607,3505,3403,3302,3201,3101,3002,2904,2806,2709,2613,2517,2423,2329,2235,2143,2051,1960,1870,1781,1692,1604,1517,1431,1346,1261,1178,1095,1013,932,852,772,694,616,539,463,388,314,241,169,97,27,-42,-110,-178,-245,-311,-376,-440,-503,-565,-627,-687,-746,-805,-862,-919,-974,-1029,-1083,-1135,-1187,-1238,-1288,-1336,-1384,-1431,-1477,-1522,-1566,-1609,-1651,-1692,-1732,-1771,-1810,-1847,-1883,-1918,-1952,-1986,-2018,-2050,-2080,-2109,-2138,-2165,-2192,-2218,-2242,-2266,-2289,-2311,-2332,-2351,-2370,-2389,-2406,-2422,-2437,-2452,-2465,-2477,-2489,-2500,-2510,-2518,-2526,-2534,-2540,-2545,-2550,-2553,-2556,-2558,-2559,-2559,-2558,-2557,-2554,-2551,-2547,-2542,-2536,-2530,-2523,-2515,-2506,-2496,-2486,-2475,-2463,-2450,-2437,-2423,-2408,-2392,-2376,-2359,-2341,-2323,-2303,-2284,-2263,-2242,-2220,-2198,-2175,-2151,-2127,-2102,-2077,-2051,-2024,-1997,-1969,-1941,-1912,-1883,-1853,-1822,-1791,-1760,-1728,-1695,-1663,-1629,-1595,-1561,-1527,-1492,-1456,-1420,-1384,-1347,-1310,-1273,-1235,-1197,-1159,-1120,-1081,-1042,-1003,-963,-923,-883,-842,-801,-760,-719,-678,-636,-595,-553,-511,-469,-426,-384,-342,-299,-256,-214,-171,-128,-85,-42,0,42,85,128,171,214,256,299,342,384,426,469,511,553,595,636,678,719,760,801,842,883,923,963,1003,1042,1081,1120,1159,1197,1235,1273,1310,1347,1384,1420,1456,1492,1527,1561,1595,1629,1663,1695,1728,1760,1791,1822,1853,1883,1912,1941,1969,1997,2024,2051,2077,2102,2127,2151,2175,2198,2220,2242,2263,2284,2303,2323,2341,2359,2376,2392,2408,2423,2437,2450,2463,2475,2486,2496,2506,2515,2523,2530,2536,2542,2547,2551,2554,2557,2558,2559,2559,2558,2556,2553,2550,2545,2540,2534,2526,2518,2510,2500,2489,2477,2465,2452,2437,2422,2406,2389,2370,2351,2332,2311,2289,2266,2242,2218,2192,2165,2138,2109,2080,2050,2018,1986,1952,1918,1883,1847,1810,1771,1732,1692,1651,1609,1566,1522,1477,1431,1384,1336,1288,1238,1187,1135,1083,1029,974,919,862,805,746,687,627,565,503,440,376,311,245,178,110,42,-27,-97,-169,-241,-314,-388,-463,-539,-616,-694,-772,-852,-932,-1013,-1095,-1178,-1261,-1346,-1431,-1517,-1604,-1692,-1781,-1870,-1960,-2051,-2143,-2235,-2329,-2423,-2517,-2613,-2709,-2806,-2904,-3002,-3101,-3201,-3302,-3403,-3505,-3607,-3711,-3815,-3919,-4024,-4130,-4236,-4343,-4451,-4559,-4668,-4778,-4888,-4998,-5109,-5221,-5333,-5446,-5559,-5673,-5787,-5902,-6017,-6132,-6248,-6365,-6482,-6599,-6717,-6836,-6954,-7073,-7193,-7313,-7433,-7553,-7674,-7795,-7917,-8039,-8161,-8283,-8406,-8529,-8652,-8776,-8900,-9024,-9148,-9272,-9397,-9522,-9647,-9772,-9897,-10023,-10148,-10274,-10400,-10526,-10652,-10778,-10904,-11030,-11157,-11283,-11409,-11536,-11662,-11789,-11915,-12042,-12168,-12294,-12421,-12547,-12673,-12799,-12925,-13051,-13177,-13303,-13429,-13554,-13679,-13804,-13929,-14054,-14179,-14303,-14427,-14551,-14675,-14799,-14922,-15045,-15168,-15290,-15412,-15534,-15656,-15777,-15898,-16019,-16139,-16259,-16379,-16498,-16617,-16735,-16853,-16971,-17088,-17205,-17322,-17438,-17553,-17669,-17783,-17897,-18011,-18124,-18237,-18350,-18461,-18573,-18683,-18794,-18903,-19012,-19121,-19229,-19337,-19444,-19550,-19656,-19761,-19866,-19970,-20073,-20176,-20278,-20380,-20481,-20581,-20681,-20780,-20878,-20976,-21073,-21169,-21265,-21360,-21454,-21548,-21641,-21734,-21825,-21916,-22006,-22096,-22185,-22273,-22360,-22447,-22532,-22618,-22702,-22786,-22869,-22951,-23032,-23113,-23193,-23272,-23351,-23428,-23505,-23582,-23657,-23732,-23805,-23878,-23951,-24022,-24093,-24163,-24232,-24301,-24368,-24435,-24501,-24567,-24631,-24695,-24758,-24820,-24882,-24942,-25002,-25061,-25120,-25177,-25234,-25290,-25345,-25400,-25453,-25506,-25558,-25609,-25660,-25710,-25759,-25807,-25855,-25901,-25947,-25993,-26037,-26081,-26124,-26166,-26208,-26249,-26289,-26328,-26366,-26404,-26441,-26478,-26514,-26549,-26583,-26616,-26649,-26682,-26713,-26744,-26774,-26803,-26832,-26860,-26888,-26915,-26941,-26966,-26991,-27015,-27039,-27062,-27084,-27106,-27127,-27147,-27167,-27186,-27205,-27223,-27240,-27257,-27274,-27290,-27305,-27319,-27334,-27347,-27360,-27373,-27385,-27396,-27407,-27418,-27428,-27437,-27446,-27455,-27463,-27471,-27478,-27484,-27491,-27497,-27502,-27507,-27512,-27516,-27520,-27523,-27526,-27529,-27531,-27533,-27535,-27536,-27537,-27537,-27537,-27537,-27537,-27536,-27535,-27534,-27532,-27530,-27528,-27526,-27523,-27520,-27517,-27514,-27510,-27507,-27503,-27498,-27494,-27489,-27485,-27480,-27475,-27469,-27464,-27459,-27453,-27447,-27441,-27435,-27429,-27423,-27417,-27410,-27404,-27397,-27391,-27384,-27377,-27370,-27364,-27357,-27350,-27343,-27336,-27329,-27323,-27316,-27309,-27302,-27295,-27289,-27282,-27275,-27269,-27262,-27256,-27249,-27243,-27237,-27231,-27225,-27219,-27213,-27207,-27202,-27196,-27191,-27186,-27181,-27176,-27171,-27166,-27162,-27158,-27154,-27150,-27146,-27142,-27139,-27136,-27133,-27130,-27128,-27125,-27123,-27121,-27119,-27118,-27117,-27116,-27115,-27114,-27114,-27114,-27114,-27115,-27115,-27116,-27118,-27119,-27121,-27123,-27125,-27128,-27130,-27134,-27137,-27141,-27145,-27149,-27153,-27158,-27163,-27168,-27174,-27180,-27186,-27193,-27200,-27207,-27214,-27222,-27230,-27238,-27247,-27256,-27265,-27274,-27284,-27294,-27304,-27315,-27326,-27337,-27348,-27360,-27372,-27385,-27397,-27410,-27423,-27437,-27450,-27464,-27479,-27493,-27508,-27523,-27538,-27554,-27570,-27586,-27602,-27619,-27636,-27653,-27670,-27687,-27705,-27723,-27741,-27760,-27778,-27797,-27816,-27836,-27855,-27875,-27895,-27915,-27935,-27955,-27976,-27996,-28017,-28038,-28059,-28081,-28102,-28124,-28145,-28167,-28189,-28211,-28233,-28255,-28278,-28300,-28322,-28345,-28367,-28390,-28413,-28435,-28458,-28481,-28504,-28527,-28550,-28572,-28595,-28618,-28641,-28664,-28686,-28709,-28732,-28754,-28777,-28799,-28821,-28844,-28866,-28888,-28910,-28931,-28953,-28975,-28996,-29017,-29038,-29059,-29080,-29100,-29121,-29141,-29160,-29180,-29199,-29219,-29238,-29256,-29274,-29293,-29310,-29328,-29345,-29362,-29378,-29395,-29410,-29426,-29441,-29456,-29470,-29484,-29498,-29511,-29524,-29536,-29548,-29559,-29570,-29581,-29591,-29600,-29609,-29618,-29626,-29634,-29641,-29647,-29653,-29658,-29663,-29667,-29671,-29674,-29676,-29678,-29679,-29680,-29680,-29679,-29678,-29676,-29673,-29670,-29666,-29661,-29655,-29649,-29642,-29634,-29626,-29617,-29607,-29596,-29585,-29572,-29559,-29545,-29531,-29515,-29499,-29481,-29463,-29444,-29425,-29404,-29383,-29360,-29337,-29313,-29288,-29262,-29235,-29207,-29178,-29148,-29118,-29086,-29053,-29020,-28985,-28950,-28913,-28876,-28837,-28798,-28757,-28716,-28673,-28629,-28585,-28539,-28492,-28444,-28396,-28346,-28295,-28243,-28189,-28135,-28080,-28023,-27966,-27907,-27847,-27787,-27725,-27661,-27597,-27532,-27465,-27398,-27329,-27259,-27188,-27116,-27043,-26968,-26892,-26815,-26737,-26658,-26578,-26496,-26414,-26330,-26245,-26159,-26071,-25983,-25893,-25802,-25710,-25617,-25522,-25427,-25330,-25232,-25132,-25032,-24930,-24828,-24724,-24618,-24512,-24405,-24296,-24186,-24075,-23963,-23849,-23735,-23619,-23502,-23384,-23264,-23144,-23022,-22899,-22775,-22650,-22524,-22396,-22268,-22138,-22007,-21875,-21742,-21607,-21472,-21335,-21198,-21059,-20919,-20778,-20636,-20492,-20348,-20202,-20056,-19908,-19760,-19610,-19459,-19307,-19154,-19000,-18845,-18689,-18532,-18374,-18215,-18054,-17893,-17731,-17568,-17404,-17239,-17073,-16906,-16738,-16569,-16399,-16228,-16056,-15884,-15710,-15536,-15360,-15184,-15007,-14829,-14650,-14471,-14290,-14109,-13927,-13744,-13560,-13376,-13191,-13004,-12818,-12630,-12442,-12253,-12063,-11873,-11681,-11490,-11297,-11104,-10910,-10715,-10520,-10325,-10128,-9931,-9734,-9536,-9337,-9138,-8938,-8737,-8536,-8335,-8133,-7931,-7728,-7525,-7321,-7117,-6912,-6707,-6502,-6296,-6089,-5883,-5676,-5469,-5261,-5053,-4845,-4636,-4427,-4218,-4009,-3799,-3590,-3379,-3169,-2959,-2748,-2537,-2326,-2115,-1904,-1693,-1481,-1270,-1058,-847,-635,-423,-211};

    String default_presets[4] = {
        "Basic Sine", "Basic Triangle", "Basic Saw", "Basic Square"
    };
    String modes[4] = {
        "SINGLE MODE", "OCTAVE MODE", "DUAL MODE", "MULTI MODE"
    };

    // ユーザープリセット
    struct Preset {
        String name; // 最大30文字
        String path;
    };
    Preset user_presets[252];
    File preset_buff[4]; // バッファ

    // UI用保存変数
    int fileman_index = 0;
    String currentDir = "/rp-ds16";
    struct Files {
        bool type;
        String name;
        String path;
    };
    Files files[4];
    File file_buff[4];
    bool isEndOfFile = false;
    bool fileManRefresh = true;

    // ボタンの判定とチャタリング対策
    uint16_t intervalCount = 0;
    uint16_t pushCount[BUTTON_COUNT] = {0, 0, 0, 0, 0, 0};
    bool longPushed[BUTTON_COUNT] = {false, false, false, false, false, false};

    LGFXRP2040* pDisplay;
    LGFX_Sprite* pSprite;

    CtrlManager* pCtrl;
    SynthManager* pSynth;
    FileManager* pFile;

public:
    UIManager(LGFXRP2040* addr1, LGFX_Sprite* addr2, CtrlManager* addr3, SynthManager* addr4, FileManager* addr5) {
        pDisplay = addr1;
        pSprite = addr2;
        pCtrl = addr3;
        pSynth = addr4;
        pFile = addr5;
    }

    void init() {
        // ローディング処理
        pDisplay->init();
        pDisplay->fillScreen(TFT_BLACK);
        pDisplay->drawString("Loading...", 1, 1);
        delay(1000);

        // ボタン初期化
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            pinMode(buttonPins[i], INPUT_PULLUP);
        }
    }

    void loadAllPreset() {
        uint8_t index = 0;
        for(uint8_t i = 0; i < 252; i+=4) {
            pFile->getFiles("/rp-ds16/preset", preset_buff, 4, i);
            for(uint8_t j = 0; j < 4; j++) {
                bool isNull = false;
                if(preset_buff[j].name() == nullptr) isNull = true;
                else {
                    if(!pFile->hasExtension(preset_buff[j].name(), ".json")) continue;
                    if(strlen(preset_buff[j].name()) > 30) continue;
                    if(preset_buff[j].isDirectory()) continue;
                }

                if(!isNull) {
                    String path = "/rp-ds16/preset/" + String(preset_buff[j].name());
                    JsonDocument doc ;
                    pFile->getJson(&doc, path);
                    String name = doc["preset_name"];
                    user_presets[index].name = name;
                    user_presets[index].path = path;
                } else {
                    user_presets[index].name = "---";
                    user_presets[index].path = "";
                }
                preset_buff[j].close();
                index++;
            }
        }

        // 最後に埋める
        while(index < 252) {
            user_presets[index].name = "---";
            user_presets[index].path = "";
            index++;
        }
    }

    void goTitle() {
        displayStatus = DISPST_TITLE;
    }

    void cursorText(String text, uint8_t x, uint8_t y, uint8_t ex_width = 0, uint8_t ex_height = 0) {
        pSprite->fillRect(x-1, y-1, pSprite->textWidth(text)+1 + ex_width, pSprite->fontHeight()+1 + ex_height, TFT_WHITE);
        pSprite->setTextColor(TFT_BLACK);
        pSprite->drawString(text, x, y);
        pSprite->setTextColor(TFT_WHITE);
    }

    
    //            __               _     
    //  _ __ ___ / _|_ __ ___  ___| |__  
    // | '__/ _ \ |_| '__/ _ \/ __| '_ \ 
    // | | |  __/  _| | |  __/\__ \ | | |
    // |_|  \___|_| |_|  \___||___/_| |_|
    //
    /** @brief UIを更新 */
    void refreshUI() {
        pSprite->createSprite(128, 64);

        pSprite->fillScreen(TFT_BLACK);
        pSprite->setTextColor(TFT_WHITE);

        // デバッグ用
        // char cursor[5];
        // sprintf(cursor, "0x%02x", displayCursor);
        // pSprite->drawString(cursor, 2, 55);

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if(displayStatus == DISPST_PRESETS){
            uint8_t preset_x = pSprite->textWidth(" ");
            uint8_t preset_y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
            char idstr[5]; sprintf(idstr, "%03d ", selectedPreset+1);
            char idstr2[5]; sprintf(idstr2, "%03d ", selectedPreset2+1);

            String preset_name1, preset_name2;
            if(selectedPreset < 0x04) preset_name1 = default_presets[selectedPreset];
            else preset_name1 = user_presets[selectedPreset - 4].name;
            if(selectedPreset2 < 0x04) preset_name2 = default_presets[selectedPreset2];
            else preset_name2 = user_presets[selectedPreset2 - 4].name;

            if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                pSprite->drawString(idstr + preset_name1, preset_x, preset_y - 7);
                pSprite->drawString(idstr2 + preset_name2, preset_x, preset_y + 7);
            }
            else {
                pSprite->drawString(idstr + preset_name1, preset_x, preset_y);
            }
            
            // MIDIチャンネル
            if(synthMode == SYNTH_MULTI){
                pSprite->drawString("MIDI=1&2", 2, 2);
            }else{
                pSprite->drawString("MIDI=1", 2, 2);
            }

            // シンセモード
            uint8_t synth_x = pSprite->textWidth(modes[synthMode]);
            pSprite->drawString(modes[synthMode], 128 - 2 - synth_x, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);
            pSprite->drawLine(0, 51, 127, 51, TFT_WHITE);

            // メニュー
            uint8_t menu_x = pSprite->textWidth("Menu>>");
            pSprite->drawString("Menu>>", 128 - 2 - menu_x, 55);

            // 塗り
            if(displayCursor == 0x01) {

                uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                char idstr[6]; sprintf(idstr, " %03d", selectedPreset+1);

                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) 
                    cursorText(idstr, 0, y - 7);
                
                else 
                    cursorText(idstr, 0, y);
                
            }
            else if(displayCursor == 0x02) {
                uint8_t synth_x = pSprite->textWidth(modes[synthMode]);
                cursorText(modes[synthMode], 128 - 2 - synth_x, 2);
            }
            else if(displayCursor == 0x03) {
                // blank
            }
            else if(displayCursor == 0x04) {
                if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                    uint8_t y = pSprite->height() / 2 - pSprite->fontHeight() / 2;
                    char idstr2[5]; sprintf(idstr2, " %03d", selectedPreset2+1);
                    cursorText(idstr2, 0, y + 7);
                }
            }
            else if(displayCursor == 0x05) {
                uint8_t menu_x = pSprite->textWidth("Menu>>");
                cursorText("Menu>>", 128 - 2 - menu_x, 55);
            }
        }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        else if(displayStatus == DISPST_DETAIL) {
            // タイトル
            pSprite->drawString("Preset Editor", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // ADSR
            char a_sym = ':'; if (displayCursor == 0x05) a_sym = '>';
            char d_sym = ':'; if (displayCursor == 0x06) d_sym = '>';
            char s_sym = ':'; if (displayCursor == 0x07) s_sym = '>';
            char r_sym = ':'; if (displayCursor == 0x08) r_sym = '>';
            char a_chr[6]; sprintf(a_chr, "%d", attack);
            char d_chr[6]; sprintf(d_chr, "%d", decay);
            char s_chr[6]; sprintf(s_chr, "%d", sustain);
            char r_chr[6]; sprintf(r_chr, "%d", release);
            pSprite->drawString("Attack " + String(a_sym) + " " + String(a_chr) + " ms", 2, 16);
            pSprite->drawString("Decay  " + String(d_sym) + " " + String(d_chr) + " ms", 2, 26);
            pSprite->drawString("Sustain" + String(s_sym) + " " + String(s_chr), 2, 36);
            pSprite->drawString("Release" + String(r_sym) + " " + String(r_chr) + " ms", 2, 46);

            // 塗り
            if(displayCursor == 0x01 || displayCursor == 0x05) {
                cursorText("Attack", 2, 16);
            }
            else if(displayCursor == 0x02 || displayCursor == 0x06) {
                cursorText("Decay", 2, 26);
            }
            else if(displayCursor == 0x03 || displayCursor == 0x07) {
                cursorText("Sustain", 2, 36);
            }
            else if(displayCursor == 0x04 || displayCursor == 0x08) {
                cursorText("Release", 2, 46);
            }
            
        }

        /*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*//*DEBUG*/
        else if(displayStatus == DISPST_DEBUG) {
            // タイトル
            pSprite->drawString("Debug Mode", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // シンセモード
            uint8_t synth_x = pSprite->textWidth("MIDI-1.0");
            pSprite->drawString("MIDI-1.0", 128 - 2 - synth_x, 2);

            // データ表示部
            pSprite->drawString("----", 2, 16);
            pSprite->drawString(" ---- ---- ----", 2, 26);
            pSprite->drawString("(Waiting data input)", 2, 36);
        }

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if(displayStatus == DISPST_MENU) {
            // タイトル
            pSprite->drawString("RP-DS16 Menu", 2, 2);

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            pSprite->drawString("Global Settings", 2, 16);
            pSprite->drawString("File Manager", 2, 26);
            pSprite->drawString("MIDI Player", 2, 36);
            pSprite->drawString("Wavetable Viewer", 2, 46);

            // 塗り
            if(displayCursor == 0x01) {
                cursorText("Global Settings", 2, 16);
            }
            else if(displayCursor == 0x02) {
                cursorText("File Manager", 2, 26);
            }
            else if(displayCursor == 0x03) {
                cursorText("MIDI Player", 2, 36);
            }
            else if(displayCursor == 0x04) {
                cursorText("Wavetable Viewer", 2, 46);
            }
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if(displayStatus == DISPST_FILEMAN) {

            // タイトル
            if(currentDir.length() > 18) {
                String title = "..." + currentDir.substring(currentDir.length() - 18);
                pSprite->drawString(title, 2, 2);
            }else{
                pSprite->drawString(currentDir, 2, 2);
            }

            // 横線
            pSprite->drawLine(0, 12, 127, 12, TFT_WHITE);

            // ファイルを取得 4つ スクロール時のみ取得
            if(fileManRefresh) {
                pFile->getFiles(currentDir, file_buff, 4, fileman_index);
                for(int8_t i = 0; i < 4; i++) {
                    files[i].type = file_buff[i].isDirectory();
                    files[i].name = file_buff[i].name();
                    if(currentDir == "/")
                        files[i].path = "/" + files[i].name;
                    else
                        files[i].path = currentDir + "/" + files[i].name;
                    file_buff[i].close();
                }
                fileManRefresh = false;
            }
            
            // ファイルを表示
            for(int8_t i = 0; i < 4; i++) {
                pSprite->drawString(files[i].name, 2, 16 + i * 10);
            }

            bool isDir = false;

            // 塗り
            if(displayCursor == 0x01) {
                cursorText(files[0].name, 2, 16);
                if (files[0].type) isDir = true;
            }
            else if(displayCursor == 0x02) {
                cursorText(files[1].name, 2, 26);
                if (files[1].type) isDir = true;
            }
            else if(displayCursor == 0x03) {
                cursorText(files[2].name, 2, 36);
                if (files[2].type) isDir = true;
            }
            else if(displayCursor == 0x04) {
                cursorText(files[3].name, 2, 46);
                if (files[3].type) isDir = true;
            }
            else if(displayCursor == 0x05) {
                cursorText("CD TO ../", 2, 55);
            }

            // ファイルタイプ
            if(displayCursor > 0x00 && displayCursor < 0x05) {
                if (isDir) {
                    pSprite->drawString("DIR", 128 - 2 - pSprite->textWidth("DIR"), 55);
                    isEndOfFile = false;

                } else if(files[displayCursor - 1].name != nullptr) {
                    pSprite->drawString("FILE", 128 - 2 - pSprite->textWidth("FILE"), 55);
                    isEndOfFile = false;
                }
                else
                    isEndOfFile = true;
            }
        }

        // 画面更新
        pSprite->pushSprite(0, 0);
        pSprite->deleteSprite();
    }

    //  _   _ _ __  
    // | | | | '_ \ 
    // | |_| | |_) |
    //  \__,_| .__/ 
    //       |_|    
    //
    // 上が押された場合の処理
    void handleButtonUp(bool longPush = false) {
        if (longPush) return;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x00:
                case 0x01:
                case 0x02:
                    displayCursor++;
                    break;
                case 0x03:
                    displayCursor = 0x05;
                    break;
                case 0x04:
                    displayCursor = 0x01;
                    break;
                case 0x05:
                    displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x01;
                    break;
            }
            refreshUI();

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 100倍
                case 0x05:
                    if (attack + 100 <= 32000) {
                        attack += 100; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay + 100 <= 32000) {
                        decay += 100; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain + 100 <= 1000) {
                        sustain += 100; pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x08:
                    if (release + 100 <= 32000) {
                        release += 100; pSynth->setRelease(0xff, release);  
                    }
                    break;
                // 通常
                default:
                    displayCursor = (displayCursor == 0x01) ? 0x04 : (displayCursor - 0x01);
                    break;
            }
            refreshUI();
        }

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            if(displayCursor == 0x01) displayCursor = 0x04;
            else displayCursor--;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if(displayCursor == 0x01) {
                if(fileman_index != 0) {
                    fileman_index--;
                    fileManRefresh = true;
                    displayCursor = 0x01;
                }
            }
            else displayCursor--;
            refreshUI();
        }
    }

    //      _                     
    //   __| | _____      ___ __  
    //  / _` |/ _ \ \ /\ / / '_ \ 
    // | (_| | (_) \ V  V /| | | |
    //  \__,_|\___/ \_/\_/ |_| |_|
    //                           
    // 下が押された場合の処理
    void handleButtonDown(bool longPush = false) {
        if (longPush) return;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x00:
                case 0x02:
                    displayCursor = 0x01;
                    break;
                case 0x01:
                    displayCursor = (synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x04 : 0x05;
                    break;
                case 0x03:
                    displayCursor = 0x02;
                    break;
                case 0x04:
                    displayCursor = 0x05;
                    break;
                case 0x05:
                    displayCursor = 0x03;
                    break;
            }
            refreshUI();

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 100倍
                case 0x05:
                    if (attack - 100 >= 0) {
                        attack -= 100; pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay - 100 >= 0) {
                        decay -= 100; pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain - 100 >= 0) {
                        sustain -= 100; pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x08:
                    if (release - 100 >= 0) {
                        release -= 100; pSynth->setRelease(0xff, release);
                    }
                    break;
                // 通常
                default:
                    displayCursor = (displayCursor == 0x04) ? 0x01 : (displayCursor + 0x01);
                    break;
            }
            refreshUI();
        }

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            if(displayCursor == 0x04) displayCursor = 0x01;
            else displayCursor++;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if(displayCursor == 0x04) {
                if(!isEndOfFile){
                    fileman_index++;
                    fileManRefresh = true;
                    displayCursor = 0x04;
                }
            } else if(displayCursor == 0x05) {
                displayCursor = 0x01;
            }
            else displayCursor++;
            refreshUI();
        }
    }

    //  _       __ _   
    // | | ___ / _| |_ 
    // | |/ _ \ |_| __|
    // | |  __/  _| |_ 
    // |_|\___|_|  \__|
    //                
    // 左が押された場合の処理
    void handleButtonLeft(bool longPush = false) {

        //todo
        uint8_t osc = 0x01;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0x00) ? (selectedPreset - 1) : 0xff;
                    if(!longPush) pSynth->setShape((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, osc, selectedPreset);
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_SINGLE) ? SYNTH_MULTI : (synthMode - 1);
                    pSynth->setSynthMode(synthMode);
                    pSynth->setShape((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, osc, selectedPreset);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        pSynth->setShape(0x02, osc, selectedPreset2);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0x00) ? (selectedPreset2 - 1) : 0xff;
                    if(!longPush) pSynth->setShape(0x02, osc, selectedPreset2);
                    refreshUI(); 
                    break;
            }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack - 1 >= 0) {
                        attack -= 1; 
                        if(!longPush) pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x02:
                    if (decay - 1 >= 0) {
                        decay -= 1; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain - 1 >= 0) {
                        sustain -= 1; 
                        if(!longPush) pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x04:
                    if (release - 1 >= 0) {
                        release -= 1; 
                        if(!longPush) pSynth->setRelease(0xff, release);
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack - 10 >= 0) {
                        attack -= 10; 
                        if(!longPush) pSynth->setAttack(0xff, attack);
                    }
                    break;
                case 0x06:
                    if (decay - 10 >= 0) {
                        decay -= 10; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain - 10 >= 0) {
                        sustain -= 10; 
                        if(!longPush) pSynth->setSustain(0xff, sustain);
                    }
                    break;
                case 0x08:
                    if (release - 10 >= 0) {
                        release -= 10; 
                        if(!longPush) pSynth->setRelease(0xff, release);
                    }
                    break;
            }
            refreshUI();
        }
    }

    //       _       _     _   
    //  _ __(_) __ _| |__ | |_ 
    // | '__| |/ _` | '_ \| __|
    // | |  | | (_| | | | | |_ 
    // |_|  |_|\__, |_| |_|\__|
    //         |___/           
    //
    // 右が押された場合の処理
    void handleButtonRight(bool longPush = false) {

        //todo
        uint8_t osc = 0x01;

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x01:
                    selectedPreset = (selectedPreset != 0xff) ? (selectedPreset + 1) : 0x00;
                    if(!longPush) pSynth->setShape((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, osc, selectedPreset);
                    refreshUI(); 
                    break;
                case 0x02:
                    if (longPush) return;
                    synthMode = (synthMode == SYNTH_MULTI) ? SYNTH_SINGLE : (synthMode + 1);
                    pSynth->setSynthMode(synthMode);
                    pSynth->setShape((synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) ? 0x01 : 0xff, osc, selectedPreset);
                    if(synthMode == SYNTH_MULTI || synthMode == SYNTH_DUAL) {
                        pSynth->setShape(0x02, osc, selectedPreset2);
                    }
                    refreshUI(); 
                    break;
                case 0x04:
                    selectedPreset2 = (selectedPreset2 != 0xff) ? (selectedPreset2 + 1) : 0x00;
                    if(!longPush) pSynth->setShape(0x02, osc, selectedPreset2);
                    refreshUI(); 
                    break;
            }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                // 通常
                case 0x01:
                    if (attack + 1 <= 32000) {
                        attack += 1; 
                        if(!longPush) pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x02:
                    if (decay + 1 <= 32000) {
                        decay += 1; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x03:
                    if (sustain + 1 <= 1000) {
                        sustain += 1; 
                        if(!longPush) pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x04:
                    if (release + 1 <= 32000) {
                        release += 1; 
                        if(!longPush) pSynth->setRelease(0xff, release);  
                    }
                    break;

                // 10倍
                case 0x05:
                    if (attack + 10 <= 32000) {
                        attack += 10; 
                        if(!longPush) pSynth->setAttack(0xff, attack); 
                    }
                    break;
                case 0x06:
                    if (decay + 10 <= 32000) {
                        decay += 10; 
                        if(!longPush) pSynth->setDecay(0xff, decay);
                    }
                    break;
                case 0x07:
                    if (sustain + 10 <= 1000) {
                        sustain += 10; 
                        if(!longPush) pSynth->setSustain(0xff, sustain); 
                    }
                    break;
                case 0x08:
                    if (release + 10 <= 32000) {
                        release += 10; 
                        if(!longPush) pSynth->setRelease(0xff, release);  
                    }
                    break;
            }
            refreshUI();
        }
    }

    //             _            
    //   ___ _ __ | |_ ___ _ __ 
    //  / _ \ '_ \| __/ _ \ '__|
    // |  __/ | | | ||  __/ |   
    //  \___|_| |_|\__\___|_|   
    //                        
    // エンターが押された場合の処理
    void handleButtonEnter(bool longPush = false) {
        if (longPush) return;

        /*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*/
        if (displayStatus == DISPST_TITLE) {
            displayStatus = DISPST_PRESETS;
            refreshUI();

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        } else if (displayStatus == DISPST_PRESETS) {
            switch (displayCursor) {
                case 0x02:
                    pSynth->resetSynth(0xff);
                    refreshUI(); 
                    break;
                case 0x01:
                    displayCursor = 0x01;
                    displayStatus = DISPST_DETAIL;
                    refreshUI();
                    break;
                case 0x00:
                    displayCursor = 0x01;
                    refreshUI();
                    break;
                case 0x05:
                    displayCursor = 0x01;
                    displayStatus = DISPST_MENU;
                    refreshUI();
                    break;
            }
        }

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        else if (displayStatus == DISPST_DETAIL) {
            switch (displayCursor) {
                case 0x01: displayCursor = 0x05;
                    break;
                case 0x02: displayCursor = 0x06;
                    break;
                case 0x03: displayCursor = 0x07;
                    break;
                case 0x04: displayCursor = 0x08;
                    break;

                case 0x05: displayCursor = 0x01;
                    break;
                case 0x06: displayCursor = 0x02;
                    break;
                case 0x07: displayCursor = 0x03;
                    break;
                case 0x08: displayCursor = 0x04;
                    break;
            }
            refreshUI();
        }

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        else if (displayStatus == DISPST_MENU) {
            switch (displayCursor) {
                case 0x01:
                    break;
                case 0x02:
                    displayCursor = 0x00;
                    displayStatus = DISPST_FILEMAN;
                    refreshUI();
                    break;
                case 0x03:
                    break;
                case 0x04:
                    break;
            }
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            switch (displayCursor) {
                case 0x01:
                    if(files[0].type) {
                        currentDir = files[0].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x02:
                    if(files[1].type) {
                        currentDir = files[1].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x03:
                    if(files[2].type) {
                        currentDir = files[2].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x04:
                    if(files[3].type) {
                        currentDir = files[3].path;
                        displayCursor = 0x00;
                        fileman_index = 0;
                    }
                    break;
                case 0x05:
                    int index = currentDir.lastIndexOf('/');
                    if (index == -1) currentDir = "/";
                    currentDir = currentDir.substring(0, index);
                    if (currentDir == "") currentDir = "/";
                    displayCursor = 0x00;
                    fileman_index = 0;
                    break;
            }
            fileManRefresh = true;
            refreshUI();
        }
    }

    //                           _ 
    //   ___ __ _ _ __   ___ ___| |
    //  / __/ _` | '_ \ / __/ _ \ |
    // | (_| (_| | | | | (_|  __/ |
    //  \___\__,_|_| |_|\___\___|_|
    //                            
    // キャンセルが押された場合の処理
    void handleButtonCancel(bool longPush = false) {

        /*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*//*TITLE*/
        if (displayStatus == DISPST_TITLE) {
            if(longPush) {
                if(long_count_to_enter_debug_mode > 10) {
                    uint8_t data[] = {INS_BEGIN, DISP_DEBUG_ON};
                    uint8_t received[1];
                    pCtrl->ctrlTransmission(data, sizeof(data), received, 1);

                    if(received[0] != RES_OK) return;

                    displayStatus = DISPST_DEBUG;
                    ctrl.end();
                    ctrl.setSDA(CTRL_SDA_PIN);
                    ctrl.setSCL(CTRL_SCL_PIN);
                    ctrl.begin(CTRL_I2C_ADDR);
                    ctrl.setClock(1000000);
                    ctrl.onReceive(pCtrl->receiveWrapper);
                    refreshUI();
                    return;
                } else {
                    long_count_to_enter_debug_mode++;
                }
            } else {
                long_count_to_enter_debug_mode = 0;
            }

        /*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*//*PRESET*/
        } else if (displayStatus == DISPST_PRESETS) {
            if (longPush) return;
            displayCursor = 0x00;
            refreshUI();

        /*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*//*DETAIL*/
        } else if (displayStatus == DISPST_DETAIL) {
            if (longPush) return;
            displayCursor = 0x01;
            displayStatus = DISPST_PRESETS;
            refreshUI();

        /*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*//*MENU*/
        } else if (displayStatus == DISPST_MENU) {
            if (longPush) return;
            displayCursor = 0x00;
            displayStatus = DISPST_PRESETS;
            refreshUI();
        }

        /*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*//*FILEMAN*/
        else if (displayStatus == DISPST_FILEMAN) {
            if (longPush) return;
            if(displayCursor == 0x05) {
                displayCursor = 0x00;
                displayStatus = DISPST_MENU;
            }
            else displayCursor = 0x05;
            refreshUI();
        }
    }

    // ==================================================================================================================================================================== //

    void buttonHandler() {
        // ボタンが押されているかどうかを確認
        if (buttonPressed) {
            // ボタンに応じた処理を実行
            switch (pressedButton) {
                case BTN_UP: handleButtonUp(); break;
                case BTN_DOWN: handleButtonDown(); break;
                case BTN_LEFT: handleButtonLeft(); break;
                case BTN_RIGHT: handleButtonRight(); break;
                case BTN_ENTER: handleButtonEnter(); break;
                case BTN_CANCEL: handleButtonCancel(); break;

                case BTN_LONG_UP: handleButtonUp(true); break;
                case BTN_LONG_DOWN: handleButtonDown(true); break;
                case BTN_LONG_LEFT: handleButtonLeft(true); break;
                case BTN_LONG_RIGHT: handleButtonRight(true); break;
                case BTN_LONG_ENTER: handleButtonEnter(true); break;
                case BTN_LONG_CANCEL: handleButtonCancel(true); break;     
            }
            // ボタン処理が完了したので、フラグをリセット
            buttonPressed = false;
        }
    }

    void buttonListener() {
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            // ボタンが押されているときの判定
            if (digitalRead(buttonPins[i]) == LOW) {
                if (pushCount[i] <= PUSH_SHORT) pushCount[i]++;
                else {
                    if (longPushed[i] && pushCount[i] >= PUSH_LONG) {
                        pressedButton = 
                            (i == 0) ? BTN_LONG_UP :
                            (i == 2) ? BTN_LONG_DOWN :
                            (i == 1) ? BTN_LONG_LEFT :
                            (i == 4) ? BTN_LONG_ENTER :
                            (i == 3) ? BTN_LONG_RIGHT :
                            (i == 5) ? BTN_LONG_CANCEL : BTN_NONE;
                        buttonPressed = true;
                        buttonHandler();
                        pushCount[i] = 0;
                    }
                    else if(pushCount[i] >= LONG_TOGGLE) {
                        longPushed[i] = true;
                        pushCount[i] = 0;
                    }
                    else {
                        pushCount[i]++;
                    }
                }
            }
            // ボタンを離しているときの判定
            else {
                if(longPushed[i] || (pushCount[i] >= PUSH_SHORT && intervalCount >= PUSH_LONG)) {
                    pressedButton = 
                        (i == 0) ? BTN_UP :
                        (i == 2) ? BTN_DOWN :
                        (i == 1) ? BTN_LEFT :
                        (i == 4) ? BTN_ENTER :
                        (i == 3) ? BTN_RIGHT :
                        (i == 5) ? BTN_CANCEL : BTN_NONE;
                    longPushed[i] = false;
                    buttonPressed = true;
                    buttonHandler();
                    intervalCount = 0;
                }
                pushCount[i] = 0;
            }
        }
        if(intervalCount <= PUSH_LONG) intervalCount++;
    }
};

#endif // UIMANAGER_H