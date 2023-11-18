#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <debug.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DEBUG_MODE 0 //0 or 1
Debug debug(DEBUG_MODE, Serial2, 8, 9, 115200);

#define CTRL_SCK 2
#define CTRL_TX 3
#define CTRL_RX 4
#define CTRL_CS 5

SPIClassRP2040& ctrl = SPI;
SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1

#define SDA_PIN 16
#define SCL_PIN 17
#define I2C_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_COUNT 6
#define BUTTON_PINS {10,11,12,13,14,15}
int buttonPins[BUTTON_COUNT] = BUTTON_PINS;

void midiLoop();

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

    ctrl.setRX(CTRL_RX);
    ctrl.setCS(CTRL_CS);
    ctrl.setSCK(CTRL_SCK);
    ctrl.setTX(CTRL_TX);
    ctrl.begin(true);

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

    debug.init();

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISR, FALLING);
    }

    pinMode(LED_BUILTIN, OUTPUT);

    multicore_launch_core1(midiLoop);
}

void loop() {
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    sprintf(msg, "note");
    ctrl.beginTransaction(spisettings);
    ctrl.transfer(msg, sizeof(msg));
    ctrl.endTransaction();

    if(String(msg) != "ok" && String(msg) != "none") {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(20, 5);
        display.print(String(msg));
        display.display();
    }

    delay(30);
}

void midiLoop() {
    // todo
}