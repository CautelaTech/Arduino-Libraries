#ifndef Stepper_4wd_h
#define Stepper_4wd_h

#define FREQPWM         10000       // 10khZ

// library interface description
class stepper_4wd {

  public:
    // constructors:
    stepper_4wd(int number_of_steps);

    // speed setter method:
    void setSpeed(long rpm_start, long rpm_max);

    // mover method:
    void step(int number_of_steps);
    void step(int steps_to_move,  int (*fun)());

  private:
    void stepMotor(int this_step);
    
    int direction;        // Direction of rotation
    unsigned long delay_max_speed;    // delay between steps, in us, based on speed
    unsigned long delay_start_speed;    // delay at startup,in us
    int number_of_steps;      // total number of steps this motor can take
    int pin_count;        // whether you're driving the motor with 2 or 4 pins
    int step_number;        // which step the motor is on
    
    // motor pin numbers:
    int motor_pin_a_plus;
    int motor_pin_a_minus;
    int motor_pin_b_plus;
    int motor_pin_b_minus;
};

#endif