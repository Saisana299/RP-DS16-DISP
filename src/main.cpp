#include <Arduino.h>
#include <Wire.h>
#include <debug.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// debug 関連
#define DEBUG_MODE 0 //0 or 1
Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

// CTRL 関連
#define CTRL_SDA_PIN 18
#define CTRL_SCL_PIN 19
#define CTRL_SW_PIN 20
#define CTRL_I2C_ADDR 0x0A

TwoWire& ctrl = Wire1;

// SCREEN 関連
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1

#define OLED_SDA_PIN 16
#define OLED_SCL_PIN 17
#define OLED_I2C_ADDR 0x3C

TwoWire& oled = Wire;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &oled, OLED_RESET);

// BUTTON 関連
#define BUTTON_COUNT 6
#define BUTTON_PINS {11,12,13,14,15,21}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

// その他
void loop1();
volatile bool buttonPressed = false;

void buttonISR() {
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 50;

    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTime < debounceDelay) {
        return;
    }

    char pressedButton[7] = "N/A";
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

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 5);
    display.print(String(pressedButton));
    display.display();
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CTRL_SW_PIN, OUTPUT);
    digitalWrite(CTRL_SW_PIN, LOW);

    oled.setSDA(OLED_SDA_PIN);
    oled.setSCL(OLED_SCL_PIN);
    oled.begin();

    ctrl.setSDA(CTRL_SDA_PIN);
    ctrl.setSCL(CTRL_SCL_PIN);
    ctrl.begin();

    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        for(;;);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 5);
    display.print(F("RPS-F32"));
    display.setCursor(20, 25);
    display.print(F("FM-Synth"));
    display.setCursor(35, 45);
    display.print(F("Dev#3"));
    display.display();

    debug.init();

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    multicore_launch_core1(loop1);
}

void loop() {} // 使用しない

void loop1() {
    // todo
}