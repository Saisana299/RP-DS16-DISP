#include <Arduino.h>
#include <Wire.h>
#include <LGFXRP2040.h>
#include <debug.h>

// debug 関連
#define DEBUG_MODE 1 //0 or 1
Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

// CTRL 関連
#define CTRL_SDA_PIN 18
#define CTRL_SCL_PIN 19
#define CTRL_SW_PIN 20
#define CTRL_I2C_ADDR 0x0A

TwoWire& ctrl = Wire1;

// BUTTON 関連
#define BUTTON_COUNT 6
#define BUTTON_PINS {11,12,13,14,15,21}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

// DISP 関連
LGFXRP2040 display;

// その他
void loop1();
volatile bool buttonPressed = false;
char pressedButton[7] = "N/A";

String title = "0:1929,1:12,0:4,1:11,0:7,1:9,0:21,1:14,0:4,1:9,0:7,1:10,0:20,1:12,0:4,1:11,0:7,1:9,0:21,1:14,0:4,1:9,0:7,1:10,0:20,1:5,0:4,1:3,0:4,1:5,0:4,1:2,0:7,1:3,0:4,1:2,0:21,1:5,0:13,1:3,0:4,1:2,0:7,1:3,0:4,1:3,0:20,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:18,1:5,0:11,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:18,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:18,1:5,0:11,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:18,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:2,1:5,0:27,1:5,0:20,1:5,0:11,1:5,0:18,1:5,0:4,1:5,0:2,1:5,0:4,1:5,0:2,1:5,0:27,1:5,0:20,1:5,0:11,1:5,0:18,1:12,0:4,1:5,0:4,1:5,0:4,1:9,0:21,1:12,0:9,1:6,0:14,1:3,0:20,1:12,0:4,1:5,0:4,1:5,0:4,1:9,0:21,1:12,0:9,1:6,0:12,1:5,0:20,1:5,0:2,1:5,0:4,1:5,0:4,1:5,0:11,1:5,0:18,1:5,0:20,1:5,0:9,1:5,0:20,1:5,0:2,1:5,0:4,1:11,0:14,1:5,0:2,1:14,0:2,1:5,0:20,1:5,0:7,1:4,0:23,1:5,0:2,1:5,0:4,1:11,0:14,1:5,0:2,1:14,0:2,1:5,0:20,1:5,0:7,1:4,0:23,1:5,0:4,1:5,0:2,1:5,0:20,1:5,0:18,1:5,0:20,1:5,0:4,1:5,0:25,1:5,0:4,1:5,0:2,1:5,0:20,1:5,0:18,1:5,0:20,1:5,0:4,1:5,0:25,1:5,0:4,1:5,0:2,1:5,0:11,1:5,0:4,1:5,0:18,1:5,0:11,1:5,0:4,1:5,0:2,1:5,0:27,1:5,0:4,1:5,0:2,1:5,0:11,1:5,0:4,1:5,0:18,1:5,0:11,1:5,0:4,1:5,0:2,1:5,0:27,1:5,0:4,1:5,0:2,1:5,0:13,1:3,0:4,1:2,0:21,1:5,0:13,1:3,0:4,1:2,0:5,1:5,0:27,1:5,0:4,1:5,0:2,1:5,0:13,1:9,0:21,1:5,0:13,1:9,0:5,1:14,0:18,1:5,0:4,1:5,0:2,1:5,0:13,1:9,0:21,1:5,0:13,1:9,0:5,1:14,0:330,1,0:127,1,0:82,1:5,0,1,0:3,1,0:9,1:3,0:16,1,0:5,1,0:19,1,0:62,1,0:5,1:2,0,1:2,0:8,1,0:3,1,0:15,1,0:5,1,0:82,1,0:5,1,0,1,0,1,0:8,1,0:5,1,0:3,1,0:2,1:4,0:2,1:4,0:3,1:4,0:3,1:3,0:4,1:4,0,1:2,0:4,1:5,0:3,1:3,0:2,1:4,0:41,1:4,0:2,1,0,1,0,1,0:9,1:3,0:2,1,0:3,1,0:2,1,0:3,1,0:2,1,0:5,1,0:3,1,0,1,0:6,1,0:6,1,0:7,1,0:3,1,0:5,1,0:3,1,0:40,1,0:5,1,0:3,1,0:11,1,0:2,1,0:3,1,0:2,1,0:3,1,0:2,1,0:5,1,0:3,1,0,1:5,0:3,1:3,0:3,1,0:6,1,0:4,1:5,0,1,0:44,1,0:5,1,0:3,1,0:12,1,0,1,0:3,1,0:2,1,0:3,1,0:2,1,0:5,1,0:3,1,0,1,0:9,1,0:3,1,0:11,1,0:5,1,0:44,1,0:5,1,0:3,1,0:12,1,0,1,0:3,1,0:2,1,0:3,1,0:2,1,0:5,1,0:3,1,0,1,0:10,1,0:2,1,0:5,1,0:5,1,0:5,1,0:44,1,0:5,1,0:3,1,0:8,1,0:3,1,0,1,0:3,1,0:2,1,0:3,1,0:2,1,0:5,1,0:3,1,0,1,0:3,1,0:6,1,0:2,1,0:4,1,0:6,1,0:3,1,0,1,0:44,1,0:5,1,0:3,1,0:9,1:3,0:3,1:4,0:2,1,0:3,1,0:3,1:3,0:2,1,0:3,1,0:2,1:3,0:3,1:4,0:4,1:2,0:2,1:5,0:3,1:3,0:2,1,0:73,1,0:127,1,0:124,1:3,0:1871";

void showImage(String data, int x = 0, int y = 0, int w = 128, int h = 64) {
    const uint16_t maxSize = 8192;//todo
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
            // 非RLEデータ
            bitmap[count++] = (atoi(ptr) == 1) ? 255 : atoi(ptr);
        }

        ptr = strtok(nullptr, ",");
    }

    // データを表示
    display.pushImage(x, y, w, h, bitmap);
    display.display();
}

void toggleCtrl(bool begin) {
    digitalWrite(CTRL_SW_PIN, HIGH);
    digitalWrite(CTRL_SW_PIN, LOW);
    if(begin){
        delay(100);
    }
}

void buttonISR() {
    // チャタリング対策どうする
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 50;

    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTime < debounceDelay) {
        return;
    }

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (digitalRead(buttonPins[i]) == LOW) {
            switch (i) {
                case 0:
                    strcpy(pressedButton, "LEFT");
                    break;
                case 1:
                    strcpy(pressedButton, "DOWN");
                    break;
                case 2:
                    strcpy(pressedButton, "RIGHT");
                    break;
                case 3:
                    strcpy(pressedButton, "CANCEL");
                    break;
                case 4:
                    strcpy(pressedButton, "ENTER");
                    break;
                case 5:
                    strcpy(pressedButton, "UP");
                    break;
            }
        }
    }

    buttonPressed = true;
    lastDebounceTime = currentMillis;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CTRL_SW_PIN, OUTPUT);
    digitalWrite(CTRL_SW_PIN, LOW);

    ctrl.setSDA(CTRL_SDA_PIN);
    ctrl.setSCL(CTRL_SCL_PIN);
    ctrl.begin();

    debug.init();

    display.init();
    display.fillScreen(TFT_BLACK);
    display.drawString("Loading...", 1, 1);
    delay(1000);

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    toggleCtrl(true);
    ctrl.beginTransmission(CTRL_I2C_ADDR);
    ctrl.write("connect");
    ctrl.endTransmission();

    uint8_t buffer_size = 10;
    char receivedString[buffer_size];
    ctrl.requestFrom(CTRL_I2C_ADDR, buffer_size);
    uint8_t bi = 0;
    while (ctrl.available()) {
        char receivedChar = ctrl.read();
        receivedString[bi] = receivedChar;
        bi++;
        if (bi >= buffer_size) {
            break; 
        }
    }
    toggleCtrl(false);

    if(strncmp(receivedString, "connect:ok", buffer_size) == 0){
        showImage(title);
    }else{
        display.drawString("Error:1001", 1, 1);
        display.drawString("Please check the conn", 1, 11);
        display.drawString("ection.", 1, 21);
    }

    multicore_launch_core1(loop1);
}

void loop() {} // 使用しない

void loop1() {
    while(1){
        if (buttonPressed) {
            if (strcmp(pressedButton, "ENTER") == 0) {
                showImage("1:1061,0:6,1:119,0:3,1:6,0,1:117,0,1:126,0,1:126,0,1:126,0:2,1:126,0,1:181,0:2,1:117,0:4,1,0:4,1:122,0:2,1:473,0:2,1:125,0:4,1:35,0:2,1:87,0:4,1:34,0:4,1:86,0:4,1:33,0:5,1:86,0:4,1:33,0:5,1:87,0:2,1:34,0:4,1:445,0:9,1:116,0:3,1:9,0:7,1:128,0:4,1:127,0,1:54,0:5,1:93,0,1:26,0:3,1:5,0:6,1:86,0:3,1:125,0:3,1:95,0:2,1:29,0,1:91,0:7,1:29,0,1:27,0:6,1:53,0:5,1:35,0:2,1:126,0:3,1:124,0,1:2,0,1:10,0,1:112,0,1:3,0,1:10,0:2,1:26,0:3,1:6,0:3,1:64,0,1:7,0:2,1:4,0,1:10,0,1:25,0,1:5,0:4,1:3,0:2,1:45,0:2,1:15,0:2,1:4,0:2,1:7,0,1:8,0:2,1:23,0:2,1:5,0,1:9,0:2,1:40,0:3,1:18,0:6,1:9,0:2,1:5,0:2,1:13,0:4,1:7,0,1:5,0,1,0,1:10,0,1:37,0:2,1:26,0:2,1:9,0:7,1:17,0:2,1:5,0,1:5,0,1:3,0,1:10,0,1:32,0:4,1:29,0:3,1:7,0,1:25,0:2,1,0:2,1:5,0,1:16,0,1:30,0,1:35,0:3,1:4,0:2,1:26,0:3,1:5,0,1,0,1:16,0,1:26,0:3,1:39,0:5,1:28,0,1:5,0,1:3,0,1:15,0,1:101,0,1:4,0,1:21,0,1:99,0,1:4,0,1,0,1:20,0,1:99,0,1:4,0,1:2,0,1:19,0,1:99,0,1:4,0,1:23,0,1:97,0,1:5,0,1:23,0,1:97,0,1:4,0,1:24,0,1:97,0,1:4,0,1:24,0,1:97,0:2,1:3,0,1:25,0,1:97,0,1:4,0,1:24,0:2,1:96,0,1:4,0,1:25,0,1:97,0,1:4,0,1:25,0,1:101,0,1:26,0,1:101,0,1:25,0,1:102,0,1:25,0,1:3");
            }
            buttonPressed = false;
        }
    }
}