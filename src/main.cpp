#include <Arduino.h>
#include <CAN.h>
#include <PS4Controller.h>

const int WHEELBASE_X = 1;
const int WHEELBASE_Y = 1;
const int DEAD_ZONE = 30;

const unsigned int ID = 0x555; // ID

uint8_t canData[8];

struct Motor_RPMs {
  int16_t frontLeft;
  int16_t frontRight;
  int16_t rearLeft;
  int16_t rearRight;
};

Motor_RPMs calculateWheelRPMs(int x, int y, int rotation) { // メカナムの計算
  Motor_RPMs RPMs;

  RPMs.frontLeft = (int16_t)(-x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.frontRight = (int16_t)(x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.rearLeft = (int16_t)(-x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  RPMs.rearRight = (int16_t)(x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation);
  return RPMs;
}

void setup() {
  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
    while (1)
      ;
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

  if (abs(left_x) < DEAD_ZONE) {
    left_x = 0;
  }
  if (abs(left_y) < DEAD_ZONE) {
    left_y = 0;
  }
  if (abs(right_x) < DEAD_ZONE) {
    right_x = 0;
  }

  Motor_RPMs RPMs;

  RPMs = calculateWheelRPMs(left_x, left_y, right_x);

  Serial.printf("FL::%x\r\n", uint16_t(RPMs.frontLeft));
  Serial.printf("FR::%X\r\n", uint16_t(RPMs.frontRight));
  Serial.printf("RL::%X\r\n", uint16_t(RPMs.rearLeft));
  Serial.printf("RR::%X\r\n", uint16_t(RPMs.rearRight));

  CAN.beginPacket(ID);
  CAN.write((uint8_t *)&RPMs, sizeof(RPMs));
  CAN.endPacket();

  delay(10);
}