#include <Arduino.h>
#include <Motor.h>

/* MOTOR SETTINGS */
Motor::Motor(int pwm_pin, int dir_pin, int channel)
    : pwm_pin(pwm_pin), dir_pin(dir_pin), channel(channel), frequency(5000),
      resolution(8) {
  pinMode(pwm_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(pwm_pin, channel);
}

/* MOVE MOTOR */
void Motor::run(int speed, bool direction) {
 // Serial.printf("MOTOR's SPEED : %d  \n", speed);
  digitalWrite(dir_pin, direction);
  ledcWrite(channel, speed);
}