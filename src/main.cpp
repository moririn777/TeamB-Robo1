#include <Arduino.h>
#include <ESP32Servo.h>
#include <Motor.h>
#include <PS4Controller.h>

const int32_t WHEELBASE_X = 1;
const int32_t WHEELBASE_Y = 1;
const int32_t DEAD_ZONE = 20;

/*足周り*/
int16_t front_left = 0;
int16_t front_right = 0;
int16_t rear_left = 0;
int16_t rear_right = 0;

Motor frontLeftMotor(32, 22, 4);
Motor frontRightMotor(33, 21, 5);
Motor rearLeftMotor(25, 19, 6);
Motor rearRightMotor(26, 18, 7);

const uint8_t LAUNCH_SERVO_PIN = 4;

Servo launchingServo;

bool launch_flag = false;

const uint8_t SET_DEGREE = 0;      // セット角度
const uint8_t LAUNCH_DEGREE = 180; // 発射角度
const int32_t DEBOUNCE_DELAY = 50;

bool circle_pressed = false;
uint32_t circle_debounce_time = 0;

void calculateWheelRPMs(int x, int y,
                        int rotation) { // メカナムホイールの各RPMを代入
  if (abs(x) < DEAD_ZONE)
    x = 0;
  if (abs(y) < DEAD_ZONE)
    y = 0;
  if (abs(rotation) < DEAD_ZONE)
    rotation = 0;
  front_left = -x - y - (WHEELBASE_X + WHEELBASE_Y) * rotation;
  front_right = -x + y - (WHEELBASE_X + WHEELBASE_Y) * rotation;
  rear_left = x - y - (WHEELBASE_X + WHEELBASE_Y) * rotation;
  rear_right = x + y - (WHEELBASE_X + WHEELBASE_Y) * rotation;
}

void runMotor(int16_t motorValue, Motor &motor, bool reverseDir) {
  motorValue = abs(motorValue) > 20 ? motorValue : 0;
  motorValue = abs(motorValue) > 255 ? 255 : motorValue;
  bool direction = motorValue < 0 ? reverseDir : !reverseDir;
  Serial.println(motorValue);
  motor.run(abs(motorValue), direction);
}

void setup() {
  Serial.begin(115200);
  PS4.begin("08:B6:1F:ED:5E:34");

  launchingServo.attach(LAUNCH_SERVO_PIN);
  launchingServo.write(0);
  launch_flag = false; // サーボのロック状態
}

void loop() {
  if (!PS4.isConnected()) {
    frontLeftMotor.run(0, 0);
    frontRightMotor.run(0, 0);
    rearLeftMotor.run(0, 0);
    rearRightMotor.run(0, 0);
    return;
  }

  calculateWheelRPMs(PS4.LStickX() * 2, PS4.LStickY() * 2, PS4.RStickX() / 2);

  runMotor(front_left, frontLeftMotor, 0);   // Front left motor
  runMotor(front_right, frontRightMotor, 0); // Front right motor
  runMotor(rear_left, rearLeftMotor, 0);     // Rear left motor
  runMotor(rear_right, rearRightMotor, 0);   // Rear right motor

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
}
