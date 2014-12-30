#include <seeed_pwm.h>
#include <stepper_4wd.h>
#include <BetterStepper.h>
#include <Streaming.h>

#include "pvfm_motor_ctrl.h"


void setup()
{
    Serial.begin(115200);
    cout << "hello world" << endl;
    
    motor.init();
}

void loop()
{
    motor.moveUp();
    
    motor.moveDown();
}