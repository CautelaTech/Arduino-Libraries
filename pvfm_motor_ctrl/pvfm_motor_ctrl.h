#ifndef __PVFM_MOTOR_CTRL_H__
#define __PVFM_MOTOR_CTRL_H__


class pvfm_motor_ctrl{


private:

    int loca;
    int speed;
    int max_step;

private:


public:

    void init();
    int getLocation();
    
    void moveUp();
    void moveDown();

};

extern pvfm_motor_ctrl motor;
#endif