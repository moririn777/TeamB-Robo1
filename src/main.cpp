#include <Arduino.h>
#include <PS4Controller.h>
#include <CAN.h>

const int WHEELBASE_X = 1; 
const int WHEELBASE_Y = 1;
const int dead_zone = 30;

unsigned int ID = 0x555; //ID
unsigned long long data;

uint8_t canData[8];

struct Motor_RPMs{
  uint16_t frontLeft;
  uint16_t frontRight;
  uint16_t rearLeft;
  uint16_t rearRight;
};

unsigned long long combineMotorRPMs(Motor_RPMs RPMs) {
    unsigned long long  RPMdata = 0;
    
    // 各モーターの RPM 値をシフトして結合（16ビットに制限）
    RPMdata |= ((unsigned long long)((uint16_t)RPMs.frontLeft) << 48);
    RPMdata |= ((unsigned long long)((uint16_t)RPMs.frontRight) << 32);
    RPMdata |= ((unsigned long long)((uint16_t)RPMs.rearLeft) << 16);
    RPMdata |= (unsigned long long)((uint16_t)RPMs.rearRight);

    return RPMdata;
}

Motor_RPMs calculateWheelRPMs(int x,int y, int rotation){//メカナムの計算
  Motor_RPMs RPMs;
  
  RPMs.frontLeft = (int16_t)(-x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation);
  RPMs.frontRight = (int16_t)(x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation);
  RPMs.rearLeft = (int16_t)(-x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation);
  RPMs.rearRight = (int16_t)(x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation);
  return RPMs;
}

void setup() {
  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
    while (1);
  }
  
  PS4.begin("48:e7:29:a3:c5:0c");
}

void loop() {
  
  if(!PS4.isConnected()){
    Serial.println("ERROR:Cant PS4Connect!!");
    return;
  }

  int left_x = PS4.LStickX();
  int left_y = PS4.LStickY();
  int right_x = PS4.RStickX();
  //int left_y = PS4.LStickY();
  
  Motor_RPMs RPMs;

  if(abs(left_x) < dead_zone && abs(left_y) < dead_zone && abs(right_x)<dead_zone){
    data = 0;
  } else{
    RPMs = calculateWheelRPMs(left_x,left_y,right_x);
    data = combineMotorRPMs(RPMs);
  }

  Serial.printf("data: 0x%016llX\n", data);
  Serial.printf("FL::%x\r\n",uint16_t(RPMs.frontLeft));
  Serial.printf("FR::%X\r\n",uint16_t(RPMs.frontRight));
  Serial.printf("RL::%X\r\n",uint16_t(RPMs.rearLeft));
  Serial.printf("RR::%X\r\n",uint16_t(RPMs.rearRight));
  
  for (int i = 0; i < 8; i++) {
    canData[i] = (data >> (56 - i * 8)) & 0xFF;
  }

  CAN.beginPacket(ID);
  CAN.write(canData, sizeof(canData));  
  CAN.endPacket();  
  
  delay(10);
}