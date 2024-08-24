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
  int16_t frontLeft;
  int16_t frontRight;
  int16_t rearLeft;
  int16_t rearRight;
};

unsigned long long combineMotorRPMs(Motor_RPMs RPMs) {
    unsigned long long  RPMdata = 0;
    
    // 各モーターの RPM 値をシフトして結合（16ビットに制限）
    RPMdata |= ((unsigned long long)(RPMs.frontLeft & 0xFFFF) << 48);
    RPMdata |= ((unsigned long long)(RPMs.frontRight & 0xFFFF) << 32);
    RPMdata |= ((unsigned long long)(RPMs.rearLeft & 0xFFFF) << 16);
    RPMdata |= (unsigned long long)(RPMs.rearRight & 0xFFFF);
    
    return RPMdata;
}

Motor_RPMs calculateWheelRPMs(int x,int y, int rotation){//メカナムの計算
  Motor_RPMs RPMs;

  if(abs(x) < dead_zone && abs(y) < dead_zone && abs(rotation)){
    RPMs.frontLeft = 00000000;
    RPMs.frontRight = 00000000;
    RPMs.rearLeft = 00000000;
    RPMs.rearRight = 00000000;
  }else{
    RPMs.frontLeft = -x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
    RPMs.frontRight = x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
    RPMs.rearLeft = -x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
    RPMs.rearRight = x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
  }
  return RPMs;
}

void setup() {
  Serial.begin(115200);
  if (!CAN.begin(1000E3)) {
    Serial.println("ERROR:Starting CAN failed!");
    while (1);
  }
  PS4.begin("00:00:00:00:00:00");
}

void loop() {
  
  if(!PS4.isConnected()){
    Serial.println("ERROR:Cant Connect!!");
    return;
  }

  int right_x = PS4.RStickX();
  int right_y = PS4.RStickY();
  int left_x = PS4.LStickX();
  //int left_y = PS4.LStickY();

  Motor_RPMs RPMs = calculateWheelRPMs(right_x,right_y,left_x);
  //Motor_RPMs RPMs = calculateWheelRPMs(0,0,0);
  data = combineMotorRPMs(RPMs);

  Serial.printf("FL::%d\r\n",int(RPMs.frontLeft));
  Serial.printf("FR::%d\r\n",int(RPMs.frontRight));
  Serial.printf("RL::%d\r\n",int(RPMs.rearLeft));
  Serial.printf("RR::%d\r\n",int(RPMs.rearRight));
  delay(1);
  
  CAN.beginPacket(ID);
  CAN.write(canData, sizeof(canData));  
  CAN.endPacket();  
  /*
  for (int i = 0; i < 8; i++) {
    canData[i] = (data >> (56 - i * 8)) & 0xFF;
  }
  for (int i = 0; i < 8; i +=2) {
    Serial.printf("%02X%02X ", canData[i],canData[i+1]);
  }
    Serial.println();
  */

}



