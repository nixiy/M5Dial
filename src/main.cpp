#include <ESP32Servo.h>
#include <PCA9685.h>  //PCA9685用ヘッダーファイル

#include "M5Dial.h"

Servo servo1;

const int SV_PIN = G2;

PCA9685 pwm = PCA9685(0x40);

#define SERVOMIN 150  // 最小パルス幅
#define SERVOMAX 500  // 最大パルス幅

#define RIGHT_WING_PIN 0
#define LEFT_WING_PIN 1

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

    pwm.begin();
    pwm.setPWMFreq(50);
}

void servo_write(int ch, int ang) {  // 動かすサーボチャンネルと角度を指定
    ang = map(ang, 0, 180, SERVOMIN,
              SERVOMAX);  // 角度（0～180）をPWMのパルス幅（150～500）に変換
    pwm.setPWM(ch, 0, ang);
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

/**
 * 回転角度を徐々に大きく、ゆっくり回転させる
 */
void motor_sample() {
    for (int j = 0; j <= 200; j += 5) {
        for (int i = 0; i < 16; i++) {
            servo_write(i, j);
        }
        delay(j);
    }
    for (int j = 200; j >= 0; j -= 5) {
        for (int i = 0; i < 16; i++) {
            servo_write(i, j);
        }
        delay(j);
    }
}

void motor_init() {
    for (int i = 0; i < 16; i++) {
        servo_write(i, 0);
    }
}

/**
 * サンプル2
 */
void motor_sample2() {
    for (int deg = 0; deg < 180; deg += 30) {
        for (int pin = 0; pin < 9; pin++) {
            servo_write(pin, deg);
            delay(deg * 2);
        }
    }

    motor_init();
}

#define RIGHT_OFFSET 200

int wing_ang(int pin, int ang) {
    if (pin == LEFT_WING_PIN) {
        return ang;
    } else if (pin == RIGHT_WING_PIN) {
        return RIGHT_OFFSET - ang;
    }
}

/**
 * 翼を稼働させる
 */
void motor_wing() {
    servo_write(RIGHT_WING_PIN, wing_ang(RIGHT_WING_PIN, 170));  // 150
    servo_write(LEFT_WING_PIN, wing_ang(LEFT_WING_PIN, 170));    // 30

    delay(2000);

    servo_write(RIGHT_WING_PIN, wing_ang(RIGHT_WING_PIN, 0));  // 0
    servo_write(LEFT_WING_PIN, wing_ang(LEFT_WING_PIN, 0));    // 180
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
    if (M5Dial.BtnA.pressedFor(1000)) {
        // M5Dial.Encoder.write(100);
        // speaker_playTone();
        motor_wing();
    }
}

/**
 * サーボモーターを指定角度に回転させる
 */
void motor_rotate() {
    servo1.write(newPosition);
    // for (int i = 0; i < 16; i++) {
    //     servo_write(i, newPosition);
    // }
}

void loop() {
    M5Dial.update();

    encoder_and_display();
    encoderClamp();
    motor_rotate();
}