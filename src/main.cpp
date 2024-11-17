#include <ESP32Servo.h>

#include "M5Dial.h"

Servo servo1;

const int SV_PIN = G2;

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setTextColor(GREEN);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
    pinMode(SV_PIN, OUTPUT);
    servo1.setPeriodHertz(50);
    servo1.attach(SV_PIN, 500, 2500);
    servo1.write(0);
}

long oldPosition = -999;
long newPosition = 0;

/**
 * メロディを再生する
 */
void speaker_playTone() {
    for (int i = 0; i < 10; i++) {
        M5Dial.Speaker.tone(i * 1000, 50);
        delay(50);
    }
    for (int i = 10; i >= 0; i--) {
        M5Dial.Speaker.tone(i * 1000, 50);
        delay(50);
    }
}

/**
 * ロータリエンコーダの読み取り位置をclampする
 * 最大値を超えた場合、エンコーダ値を初期化する
 */
void encoderClamp() {
    if (newPosition >= 200) {
        newPosition = 200;
        M5Dial.Encoder.write(0);
    } else if (newPosition <= 0) {
        newPosition = 0;
        M5Dial.Encoder.write(0);
    }
}

void encoder_and_display() {
    newPosition = M5Dial.Encoder.read() * 3;

    if (newPosition != oldPosition) {
        M5Dial.Display.clear();
        oldPosition = newPosition;
        Serial.println(newPosition);
        M5Dial.Display.drawString(String(newPosition),
                                  M5Dial.Display.width() / 2,
                                  M5Dial.Display.height() / 2);
    }
    if (M5Dial.BtnA.wasPressed()) {
        M5Dial.Encoder.readAndReset();
    }
    if (M5Dial.BtnA.pressedFor(2000)) {
        M5Dial.Encoder.write(100);
        speaker_playTone();
    }
}

/**
 * サーボモーターを指定角度に回転させる
 */
void motor_rotate() { servo1.write(newPosition); }

void loop() {
    M5Dial.update();

    encoder_and_display();
    encoderClamp();
    motor_rotate();
}