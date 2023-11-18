#include <Arduino.h>
#include <Wire.h>
#include <debug.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// debug 関連
//#define DEBUG_MODE 0 //0 or 1
//Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

// CTRL 関連
#define CTRL_TX_PIN 0
#define CTRL_RX_PIN 1

// SCREEN 関連
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1

#define SDA_PIN 16
#define SCL_PIN 17
#define I2C_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BUTTON 関連
#define BUTTON_COUNT 6
#define BUTTON_PINS {10,11,12,13,14,15}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

// その他
void loop1();

void buttonISR() {
    int pressedButton = -1;
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (digitalRead(buttonPins[i]) == LOW) {
            pressedButton = i + 1;
            break;
        }
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 5);
    display.print(String(pressedButton));
    display.display();
}

void setup() {
    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();

    //todo
    Serial1.setTX(CTRL_TX_PIN);
    Serial1.setRX(CTRL_RX_PIN);
    Serial1.begin(31250);

    if(!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
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

    //debug.init();

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    pinMode(LED_BUILTIN, OUTPUT);

    multicore_launch_core1(loop1);
}

void loop() {
    // todo
}

void loop1() {
    while(1){
        Serial1.print("T");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(10);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }
}