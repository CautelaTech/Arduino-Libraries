#include <seeed_pwm.h>
#include <stepper_4wd.h>
#include <BetterStepper.h>
#include <Arduino.h>

#include "pvfm_motor_ctrl.h"

#define STEP_LOCA_UP            1
#define STEP_LOCA_DOWN          2
#define STEP_LOCA_MID           0

#define MAX_STEP                5000

const int pinUp   = 3;                  // pin connect
const int pinDown = 2;

const int stepsPerRevolution = 200;
const int stepPerHole        = 5;       // = 9deg/1.8deg, 9deg = 360deg/40teeth

// initialize the stepper library on pins 8 through 11:
stepper_4wd stepperCut(stepsPerRevolution);

void move(int __step)
{
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    stepperCut.step(__step);

    PWM.setPwm(9, 1, FREQPWM);
    PWM.setPwm(10, 1, FREQPWM);
}

void pvfm_motor_ctrl::init()
{
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    
    pinMode(pinUp, INPUT);                      // output and pull up
    pinMode(pinDown, INPUT);
    digitalWrite(pinUp, HIGH);
    digitalWrite(pinDown, HIGH);
    
    // pwm set
    PWM.init(); //FREQPWM
    PWM.setPwm(9, 1, FREQPWM);
    PWM.setPwm(10, 1, FREQPWM);
    
    stepperCut.setSpeed(150, 200);
}

int pvfm_motor_ctrl::getLocation()
{
    if(!digitalRead(pinUp))
    {
        //cout << "UP" << endl;
        return STEP_LOCA_UP;
    }
    else if(!digitalRead(pinDown))
    {
        //cout << "DOWN" << endl;
        return STEP_LOCA_DOWN;
    }
    else return STEP_LOCA_MID;
}

int isUp()
{
    return (digitalRead(pinUp) != HIGH);
}

int isDown()
{
    return (digitalRead(pinDown) != HIGH);
}

void pvfm_motor_ctrl::moveUp()
{
    if(isUp())return;

    int __step = 15000;
    
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    
    stepperCut.step(__step, isUp);
    
    PWM.setPwm(9, 1, FREQPWM);
    PWM.setPwm(10, 1, FREQPWM);
}

void pvfm_motor_ctrl::moveDown()
{
    if(isDown())return;

    int __step = -15000;
    
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    
    stepperCut.step(__step, isDown);
    
    PWM.setPwm(9, 1, FREQPWM);
    PWM.setPwm(10, 1, FREQPWM);
}

pvfm_motor_ctrl motor;