#include <LovyanGFX.hpp>
#include <wokwi.h>

class LGFXRP2040 : public lgfx::LGFX_Device {
#if WOKWI_MODE != 1
    lgfx::Panel_SH110x _panel_instance;
#else
    lgfx::Panel_SSD1306 _panel_instance;
#endif
    lgfx::Bus_I2C       _bus_instance;
public:
    LGFXRP2040(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port   = 0;      // 使用するI2Cポートを選択 (0 or 1)
            cfg.freq_write = 1000000; // 送信時のクロック
            cfg.freq_read  = 400000; // 受信時のクロック
            cfg.pin_sda    = 16;     // SDAを接続しているピン番号
            cfg.pin_scl    = 17;     // SCLを接続しているピン番号
            cfg.i2c_addr   = 0x3C;   // I2Cデバイスのアドレス

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           =    -1; // CSが接続されているピン番号   (-1 = disable)
            cfg.pin_rst          =    -1; // RSTが接続されているピン番号  (-1 = disable)
            cfg.pin_busy         =    -1; // BUSYが接続されているピン番号 (-1 = disable)

            cfg.panel_width      =   128; // 実際に表示可能な幅
            cfg.panel_height     =    64; // 実際に表示可能な高さ

            #if WOKWI_MODE != 1
                cfg.offset_x     =     2; // パネルのX方向オフセット量
            #else
                cfg.offset_x     =     0; // パネルのX方向オフセット量
            #endif

            cfg.offset_y         =     0; // パネルのY方向オフセット量
            cfg.offset_rotation  =     2; // 回転方向の値のオフセット 0~7 (4~7は上下反転)
            cfg.dummy_read_pixel =     8; // ピクセル読出し前のダミーリードのビット数
            cfg.dummy_read_bits  =     1; // ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable         = false; // データ読出しが可能な場合 trueに設定
            cfg.invert           = false; // パネルの明暗が反転してしまう場合 trueに設定
            cfg.rgb_order        = false; // パネルの赤と青が入れ替わってしまう場合 trueに設定
            cfg.dlen_16bit       = false; // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
            cfg.bus_shared       = false; // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }

    /**
     * @brief 画像をディスプレイに表示します。
     * 
     * @param data example: "0,1,1,0,1,1,0" "1:20,0:3,1:57,0,1,0,1:3"
     * @param x 描画開始位置：列
     * @param y 描画開始位置：行
     * @param w 描画範囲：長さ
     * @param h 描画範囲：高さ
     */
    void showImage(LGFX_Sprite* sprite, String data, int x = 0, int y = 0, int w = 128, int h = 64) {
        const uint16_t maxSize = 8192; // SSD1306 128x64 用 最大画素数
        uint8_t bitmap[maxSize];
        uint16_t count = 0;

        char *ptr = strtok(const_cast<char*>(data.c_str()), ",");
        while (ptr != nullptr && count < maxSize) {
            // コロンがあるかどうかを確認してRLEエンコーディングを検出
            char *colonPtr = strchr(ptr, ':');
            if (colonPtr != nullptr) {
                uint8_t value = atoi(ptr);
                uint16_t repeatCount = atoi(colonPtr + 1);
                // RLEエンコーディングを展開
                for (uint16_t i = 0; i < repeatCount && count < maxSize; ++i) {
                    bitmap[count++] = (value == 1) ? 255 : value;
                }
            } else {
                bitmap[count++] = (atoi(ptr) == 1) ? 255 : atoi(ptr); // 非RLEデータ
            }
            ptr = strtok(nullptr, ",");
        }

        // データを表示
        sprite->createSprite(128, 64);
        sprite->pushImage(x, y, w, h, bitmap);
        sprite->pushSprite(0, 0);
        sprite->deleteSprite();
    }
};