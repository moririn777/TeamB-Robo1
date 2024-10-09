#include <Arduino.h>
#include <CAN.h>
#include <string.h>

const int32_t WHEELBASE_X = 1;
const int32_t WHEELBASE_Y = 1;
const int32_t DEAD_ZONE = 30;

const uint32_t TX_ID = 0x100; // TX_ID

const uint32_t RX_ID1 = 0x111; 
const uint32_t RX_ID2 = 0x222;

uint8_t can_data[8];
uint8_t rpm[2];

unsigned long previous_millis = 0;

float front_left_rpm = 0;
float front_right_rpm = 0;
float rear_left_rpm = 0;
float rear_right_rpm = 0;

class MotorRpms {
public:
  MotorRpms(int16_t front_left, int16_t front_right, int16_t rear_left,
            int16_t rear_right) {
    this->front_left = front_left;
    this->front_right = front_right;
    this->rear_left = rear_left;
    this->rear_right = rear_right;
  }
  int16_t frontLeft() { return this->front_left; }
  int16_t frontRight() { return this->front_right; }
  int16_t rearLeft() { return this->rear_left; }
  int16_t rearRight() { return this->rear_right; }

private:
  int16_t front_left;
  int16_t front_right;
  int16_t rear_left;
  int16_t rear_right;
};

uint64_t combineMotorRpms(MotorRpms *rpms) {
  uint64_t rpm_data = 0;

  // 各モーターの RPM 値をシフトして結合（16ビットに制限）
  rpm_data |= (uint64_t)rpms->frontLeft() << 48;
  rpm_data |= (uint64_t)rpms->frontRight() << 32;
  rpm_data |= (uint64_t)rpms->rearLeft() << 16;
  rpm_data |= (uint64_t)rpms->rearRight();

  return rpm_data;
}

MotorRpms calculateWheelRpms(int8_t x, int8_t y,
                             int8_t rotation) { // メカナムの計算
  int16_t front_left = -x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t front_right = x + y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t rear_left = -x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  int16_t rear_right = x - y + (WHEELBASE_X + WHEELBASE_Y) * rotation;
  return MotorRpms(front_left, front_right, rear_left, rear_right);
}

void receiveFloatViaCan(uint8_t* rx_data, float* value) {
    memcpy(value, rx_data, sizeof(float));  // RxDataからfloat型へコピー
}

void setup() {
  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
  }
}

void loop() {
  MotorRpms rpms = calculateWheelRpms(0, 0, 127);
  uint64_t data = combineMotorRpms(&rpms);

  const long interval = 10; // 10ミリ秒
  unsigned long currentMillis = millis();
  
  /*
  Serial.printf("data: 0x%016llX\r\n", data);
  Serial.printf("FL::%d\r\n", uint16_t(rpms.frontLeft()));
  Serial.printf("FR::%d\r\n", uint16_t(rpms.frontRight()));
  Serial.printf("RL::%d\r\n", uint16_t(rpms.rearLeft()));
  Serial.printf("RR::%d\r\n", uint16_t(rpms.rearRight()));
  */

  for (int i = 0; i < 8; i++) {
    can_data[i] = (data >> (56 - i * 8)) & 0xFF;
  }

  if (currentMillis - previous_millis >= interval) {
    previous_millis = currentMillis;

    CAN.beginPacket(TX_ID);
    CAN.write(can_data, sizeof(can_data));
    CAN.endPacket();
  }

  char buf[256];
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // バッファに受信データを格納
    byte received_data[8]; // 最大8バイトのデータ格納用
    uint32_t receivedId = CAN.packetId(); // 受信したメッセージのIDを取得
    for (int i = 0; i < packetSize; i++) {
      received_data[i] = CAN.read(); // 受信したバイトを配列に格納
    }
    float received_rotations1, received_rotations2;

    receiveFloatViaCan(received_data, &received_rotations1);
    receiveFloatViaCan(received_data + 4, &received_rotations2);
    Serial.print("Data: \r\n");
    Serial.printf("rotations1: %f\r\n",received_rotations1);
    Serial.printf("rotations2: %f\r\n",received_rotations2);
    Serial.println();

    if(receivedId == RX_ID1){
      Serial.println("Processing data for ID1...");
      front_left_rpm = received_rotations1;
      front_right_rpm = received_rotations2;
    }
    if(receivedId == RX_ID2){
      Serial.println("Processing data for ID2...");
      rear_left_rpm = received_rotations1;
      rear_right_rpm = received_rotations2;
    }
  }
}
