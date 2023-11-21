#include <Arduino.h>
#include <Wire.h>
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

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    multicore_launch_core1(loop1);
}

void loop() {} // 使用しない

void loop1() {
    while(1){
        if (buttonPressed) {
            if (strcmp(pressedButton, "UP") == 0) {
                toggleCtrl(true);

                ctrl.beginTransmission(CTRL_I2C_ADDR);
                ctrl.write("T");
                ctrl.endTransmission();

                ctrl.requestFrom(CTRL_I2C_ADDR, 1);
                while(ctrl.available()) {
                    int a = ctrl.read();
                    // display.clearDisplay();
                    // display.setTextSize(2);
                    // display.setTextColor(SSD1306_WHITE);
                    // display.setCursor(20, 5);
                    // display.print(String(a));
                    // display.display();
                }

                toggleCtrl(false);
            } else if(strcmp(pressedButton, "DOWN") == 0) {
                // display.clearDisplay();
            }
            buttonPressed = false;
        }
    }
}