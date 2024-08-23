#include <Arduino.h>
#include <PS4Controller.h>

const int WHEELBASE_X = 1; 
const int WHEELBASE_Y = 1;
const int dead_zone = 30;

struct Motor_RPMs{
  double frontLeft;
  double frontRight;
  double rearLeft;
  double rearRight;
};

Motor_RPMs calculateWheelRPMs(int x,int y, int rotation){//メカナムの計算
  Motor_RPMs RPMs;
  if(abs(x) < dead_zone && abs(y) < dead_zone)
  RPMs.frontLeft = -x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
  RPMs.frontRight = x + y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
  RPMs.rearLeft = -x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation;
  RPMs.rearRight = x - y + (WHEELBASE_X + WHEELBASE_Y)*rotation;

  return RPMs;
}

void setup() {
  Serial.begin(115200);
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
  Serial.printf("%f\r\n",RPMs.frontLeft);
  Serial.printf("%f\r\n",RPMs.frontRight);
  Serial.printf("%f\r\n",RPMs.rearLeft);
  Serial.printf("%f\r\n",RPMs.rearRight);
  delay(1000);
}



