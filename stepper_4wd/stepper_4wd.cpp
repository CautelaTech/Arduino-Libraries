#include <Arduino.h>
#include <seeed_pwm.h>
#include "stepper_4wd_dfs.h"
#include "stepper_4wd.h"


/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */

stepper_4wd::stepper_4wd(int number_of_steps)
{
  this->step_number = 0;      // which step the motor is on
  this->direction = 0;      // motor direction
  this->number_of_steps = number_of_steps;    // total number of steps for this motor
  
  // Arduino pins for the motor control connection:
  this->motor_pin_a_plus = PINM1F;
  this->motor_pin_a_minus = PINM1R;
  this->motor_pin_b_plus = PINM2F;
  this->motor_pin_b_minus = PINM2R;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_a_plus, OUTPUT);
  pinMode(this->motor_pin_a_minus, OUTPUT);
  pinMode(this->motor_pin_b_plus, OUTPUT);
  pinMode(this->motor_pin_b_minus, OUTPUT);
  
  digitalWrite(this->motor_pin_a_plus, 0);
  digitalWrite(this->motor_pin_a_minus, 0);
  digitalWrite(this->motor_pin_b_plus, 0);
  digitalWrite(this->motor_pin_b_minus, 0);

  // pin_count is used by the stepMotor() method:  
  this->pin_count = 4;  
  
  ALLMOSON;
}

/*
  Sets the speed in revs per minute

*/
void stepper_4wd::setSpeed(long rpm_start, long rpm_max)
{
  this->delay_max_speed = 60L * 1000L *1000L / this->number_of_steps / rpm_max;
  this->delay_start_speed = 60L * 1000L * 1000L/ this->number_of_steps / rpm_start;
}

/*
  Moves the motor steps_to_move steps.  If the number is negative, 
   the motor moves in the reverse direction.
 */
#define SPD_BUFF_STEPS        100

void stepper_4wd::step(int steps_to_move)
{  
  int steps_left = abs(steps_to_move);  // how many steps to take
  int steps_orig = steps_left;
  int steps_buffer = SPD_BUFF_STEPS;
  
  if (steps_orig < steps_buffer*2)
    steps_buffer = steps_left/2;
  
  float delays = this->delay_start_speed;
  float delay_minus = (delays - this->delay_max_speed)/SPD_BUFF_STEPS;
  
  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) {this->direction = 1;}
  if (steps_to_move < 0) {this->direction = 0;}
    
  // decrement the number of steps, moving one step each time:
  while(steps_left > 0) {
    delayMicroseconds(delays);
    if (this->direction == 1) {
      this->step_number++;
      if (this->step_number == this->number_of_steps) {
        this->step_number = 0;
      }
    } 
    else { 
      if (this->step_number == 0) {
        this->step_number = this->number_of_steps;
      }
      this->step_number--;
    }
    // decrement the steps left:
    steps_left--;
    if ((steps_orig - steps_left) <= steps_buffer)
      delays -= delay_minus;
    else if (steps_left <= steps_buffer)
        delays += delay_minus;
    
    // step the motor to step number 0, 1, 2, or 3:
    stepMotor(this->step_number % 4);
  }
  
}



void stepper_4wd::step(int steps_to_move,  int (*fun)())
{
  int steps_left = abs(steps_to_move);  // how many steps to take
  int steps_orig = steps_left;
  int steps_buffer = SPD_BUFF_STEPS;
  
  if (steps_orig < steps_buffer*2)
    steps_buffer = steps_left/2;
  
  float delays = this->delay_start_speed;
  float delay_minus = (delays - this->delay_max_speed)/SPD_BUFF_STEPS;
  
  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) {this->direction = 1;}
  if (steps_to_move < 0) {this->direction = 0;}
    
  // decrement the number of steps, moving one step each time:
  while(steps_left > 0) 
  {
  
    if(fun())return;
    
    delayMicroseconds(delays);
    if (this->direction == 1) {
      this->step_number++;
      if (this->step_number == this->number_of_steps) {
        this->step_number = 0;
      }
    } 
    else { 
      if (this->step_number == 0) {
        this->step_number = this->number_of_steps;
      }
      this->step_number--;
    }
    // decrement the steps left:
    steps_left--;
    if ((steps_orig - steps_left) <= steps_buffer)
      delays -= delay_minus;
    else if (steps_left <= steps_buffer)
        delays += delay_minus;
    
    // step the motor to step number 0, 1, 2, or 3:
    stepMotor(this->step_number % 4);
  }
  
}
/*
 * Moves the motor forward or backwards.
 */
void stepper_4wd::stepMotor(int thisStep)
{
  if (this->pin_count == 4) {
    switch (thisStep) {
      case 0:    // 1010
      digitalWrite(motor_pin_a_plus, HIGH);
      digitalWrite(motor_pin_a_minus, LOW);
      digitalWrite(motor_pin_b_plus, HIGH);
      digitalWrite(motor_pin_b_minus, LOW);
      break;
      case 1:    // 0110
      digitalWrite(motor_pin_a_plus, LOW);
      digitalWrite(motor_pin_a_minus, HIGH);
      digitalWrite(motor_pin_b_plus, HIGH);
      digitalWrite(motor_pin_b_minus, LOW);
      break;
      case 2:    //0101
      digitalWrite(motor_pin_a_plus, LOW);
      digitalWrite(motor_pin_a_minus, HIGH);
      digitalWrite(motor_pin_b_plus, LOW);
      digitalWrite(motor_pin_b_minus, HIGH);
      break;
      case 3:    //1001
      digitalWrite(motor_pin_a_plus, HIGH);
      digitalWrite(motor_pin_a_minus, LOW);
      digitalWrite(motor_pin_b_plus, LOW);
      digitalWrite(motor_pin_b_minus, HIGH);
      break;
      default:
      digitalWrite(motor_pin_a_plus, 0);
      digitalWrite(motor_pin_a_minus, 0);
      digitalWrite(motor_pin_b_plus, 0);
      digitalWrite(motor_pin_b_minus, 0);
      break;
    } 
  }
}
