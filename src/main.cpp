#include <Arduino.h>
#include <CAN.h>
#include <ESP32Servo.h>
#include <PS4Controller.h>

const int32_t WHEELBASE_X = 1;
const int32_t WHEELBASE_Y = 1;
const int32_t DEAD_ZONE = 30;
const uint8_t LAUNCHING_SERVO_PIN = 32;
const uint32_t ID = 0x555; // ID
const int32_t SET_DEGREE = 0;      // 装填角度
const int32_t LAUNCH_DEGREE = 45;  // 発射角度
const int32_t DEBOUNCE_DELAY = 50; // チャタリング防止

bool circle_pressed = false;
uint32_t circle_debounce_time = 0;
uint8_t can_data[8];
Servo launchingServo;
bool launch_flag;

class MotorRpms {
  public:
    MotorRpms(
      int16_t front_left,
      int16_t front_right,
      int16_t rear_left,
      int16_t rear_right
    ) {
      this->front_left = front_left;
      this->front_right = front_right;
      this->rear_left = rear_left;
      this->rear_right = rear_right;
    }
    int16_t frontLeft() {
      return this->front_left;
    }
    int16_t frontRight() {
      return this->front_right;
    }
    int16_t rearLeft() {
      return this->rear_left;
    }
    int16_t rearRight() {
      return this->rear_right;
    }
  private:
    int16_t front_left;
    int16_t front_right;
    int16_t rear_left;
    int16_t rear_right;
};

uint64_t combineMotorRpms(MotorRpms rpms) {
  uint64_t rpm_data = 0;

  // 各モーターの RPM 値をシフトして結合（16ビットに制限）
  rpm_data |= (uint64_t)rpms.frontLeft() << 48;
  rpm_data |= (uint64_t)rpms.frontRight() << 32;
  rpm_data |= (uint64_t)rpms.rearLeft() << 16;
  rpm_data |= (uint64_t)rpms.rearRight();

  return rpm_data;
}

MotorRpms calculateWheelRpms(int8_t x, int8_t y, int8_t rotation) { // メカナムの計算
  int16_t front_left = -x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t front_right = x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t rear_left = -x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t rear_right = x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  return MotorRpms(front_left, front_right, rear_left, rear_right);
}

void setup() {
  launchingServo.attach(LAUNCHING_SERVO_PIN);
  launchingServo.write(0);
  launch_flag = false; // サーボのロック状態

  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
  }
  PS4.begin("08:B6:1F:ED:5E:34");
}

void loop() {

  if (!PS4.isConnected()) {
    Serial.println("ERROR:Cant PS4Connect!!");
    return;
  }

  if (PS4.Circle()) {
    if (!circle_pressed && millis() - circle_debounce_time > DEBOUNCE_DELAY) {
      if (!launch_flag) {
        launchingServo.write(LAUNCH_DEGREE);
      } else {
        launchingServo.write(SET_DEGREE);
      }
      launch_flag = !launch_flag;
      circle_debounce_time = millis();
    }
    circle_pressed = true;
  } else {
    circle_pressed = false;
  }

  int8_t left_x = PS4.LStickX();
  int8_t left_y = PS4.LStickY();
  int8_t right_x = PS4.RStickX();
  // int8_t left_y = PS4.LStickY();
  
  if (abs(left_x) < DEAD_ZONE) {
    left_x = 0;
  }
  if (abs(left_y) < DEAD_ZONE) {
    left_y = 0;
  }
  if (abs(right_x) < DEAD_ZONE) {
    right_x = 0;
  }
  
  MotorRpms rpms = calculateWheelRpms(left_x, left_y, right_x);
  uint64_t data = combineMotorRpms(rpms);

  Serial.printf("data: 0x%016llX\r\n", data);
  Serial.printf("FL::%x\r\n", uint16_t(rpms.frontLeft()));
  Serial.printf("FR::%X\r\n", uint16_t(rpms.frontRight()));
  Serial.printf("RL::%X\r\n", uint16_t(rpms.rearLeft()));
  Serial.printf("RR::%X\r\n", uint16_t(rpms.rearRight()));

  for (int i = 0; i < 8; i++) {
    can_data[i] = (data >> (56 - i * 8)) & 0xFF;
  }

  CAN.beginPacket(ID);
  CAN.write(can_data, sizeof(can_data));
  CAN.endPacket();
}