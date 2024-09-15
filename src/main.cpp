#include <Arduino.h>
#include <CAN.h>
#include <ESP32Servo.h>
#include <PS4Controller.h>

const int WHEELBASE_X = 1;
const int WHEELBASE_Y = 1;
const int DEAD_ZONE = 30;

const uint8_t SERVO1_PIN = 32;

const int8_t SET_DEGREE = 0;   // 初期位置 ストッパー
const int8_t TAKE_DEGREE = 80; // 発射

Servo launchingServo;
bool launchFlag;

const uint ID = 0x555; // ID
unsigned long long data;

uint8_t TxData[8];

struct Motor_RPMs {
  uint16_t frontLeft;
  uint16_t frontRight;
  uint16_t rearLeft;
  uint16_t rearRight;
};

unsigned long long combineMotorRPMs(Motor_RPMs RPMs) {
  unsigned long long RPMdata = 0;

  // 各モーターの RPM 値をシフトして結合（16ビットに制限）
  RPMdata |= ((unsigned long long)((uint16_t)RPMs.frontLeft) << 48);
  RPMdata |= ((unsigned long long)((uint16_t)RPMs.frontRight) << 32);
  RPMdata |= ((unsigned long long)((uint16_t)RPMs.rearLeft) << 16);
  RPMdata |= (unsigned long long)((uint16_t)RPMs.rearRight);

  return RPMdata;
}

Motor_RPMs calculateWheelRPMs(int x, int y, int rotation) { // メカナムの計算
  Motor_RPMs RPMs;

  RPMs.frontLeft = (int16_t)(-x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.frontRight = (int16_t)(x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.rearLeft = (int16_t)(-x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.rearRight = (int16_t)(x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  return RPMs;
}

void setup() {
  launchingServo.attach(SERVO1_PIN);
  launchingServo.write(SET_DEGREE);
  launchFlag = 0;

  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
    CAN.beginPacket(ID);
    CAN.write(0, 1);
    CAN.endPacket();
    while (1)
      ;
    delay(1);
  }
  PS4.begin("08:B6:1F:ED:5E:34");
}

void loop() {
  if (!PS4.isConnected()) {
    Serial.println("ERROR:Cant PS4Connect!!");
    return;
  }

  int left_x = PS4.LStickX();
  int left_y = PS4.LStickY();
  int right_x = PS4.RStickX();
  // int left_y = PS4.LStickY();

  Motor_RPMs RPMs;

  if (abs(left_x) < DEAD_ZONE && abs(left_y) < DEAD_ZONE &&
      abs(right_x) < DEAD_ZONE) {
    data = 0;
  } else{
    RPMs = calculateWheelRPMs(left_x,left_y,right_x); //メカナムの各モーターのＲＰＭを計算
    data = combineMotorRPMs(RPMs); //64bitのデータを取得
  }

  if (PS4.Circle()) {
    if (!launchFlag) {
      launchingServo.write(TAKE_DEGREE);
      delay(10);
    } else {
      launchFlag=!launchFlag;
      launchingServo.write(SET_DEGREE);
      delay(10);
    }
    launchFlag=!launchFlag;
  }

  Serial.printf("data: 0x%016llX\n", data);
  Serial.printf("FL::%x\r\n", uint16_t(RPMs.frontLeft));
  Serial.printf("FR::%X\r\n", uint16_t(RPMs.frontRight));
  Serial.printf("RL::%X\r\n", uint16_t(RPMs.rearLeft));
  Serial.printf("RR::%X\r\n", uint16_t(RPMs.rearRight));

  for (int i = 0; i < 8; i++) {
    TxData[i] = (data >> (56 - i * 8)) & 0xFF; //64bitのデータを8bitに分割する
  }

  CAN.beginPacket(ID);
  CAN.write(TxData, sizeof(TxData));  
  CAN.endPacket();  
  
  delay(10);
}