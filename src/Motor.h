#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor{
    public:
     Motor(int pwm_pin, int dir_pin, int channel);
     void run(int axis, bool direction);

    private:
     int pwm_pin;
     int dir_pin;
     int channel;

     int frequency;
     int resolution;
};

#endif