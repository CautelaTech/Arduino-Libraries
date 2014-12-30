#ifndef __PVFM_I2C_MOTOR_H__
#define __PVFM_I2C_MOTOR_H__

#include <Wire.h>
#include <Arduino.h>


#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01
#define EnableStepper             0x1a
#define UnenableStepper           0x1b
#define Stepernu                  0x1c
#define I2CMotorDriverAdd         0x0f   // Set the address of the I2CMotorDriver

#define __SPEED                   100

class pvfm_i2c_motor{

private:

    void MotorSpeedSetAB(unsigned char MotorSpeedA , unsigned char MotorSpeedB)  
    {
        MotorSpeedA=map(MotorSpeedA,0,100,0,255);
        MotorSpeedB=map(MotorSpeedB,0,100,0,255);
        Wire.beginTransmission(I2CMotorDriverAdd);          // transmit to device I2CMotorDriverAdd
        Wire.write(MotorSpeedSet);                          // set pwm header
        Wire.write(MotorSpeedA);                            // send pwma
        Wire.write(MotorSpeedB);                            // send pwmb
        Wire.endTransmission();                             // stop transmitting
    }
    
    void MotorDirectionSet(unsigned char Direction)  {      //  Adjust the direction of the motors 0b0000 I4 I3 I2 I1
        Wire.beginTransmission(I2CMotorDriverAdd);          // transmit to device I2CMotorDriverAdd
        Wire.write(DirectionSet);                           // Direction control header
        Wire.write(Direction);                              // send direction control information
        Wire.write(Nothing);                                // need to send this byte as the third byte(no meaning)
        Wire.endTransmission();                             // stop transmitting
    }

    // you can adjust the driection and speed together
    void MotorDriectionAndSpeedSet(unsigned char Direction,unsigned char MotorSpeedA,unsigned char MotorSpeedB)  
    { 
        MotorDirectionSet(Direction);
        MotorSpeedSetAB(MotorSpeedA,MotorSpeedB);
    }

public:

    void init()
    {
        MotorSpeedSetAB(0,0);//defines the speed of motor 1 and motor 2;
        MotorDirectionSet(0b1010);  //"0b1010" defines the output polarity, "10" means the M+ is "positive" while the M- is "negtive"
    }
    
    void run()
    {
        //MotorDirectionSet(0b1010);
        MotorSpeedSetAB(100, 100);
        delay(10);
        MotorDirectionSet(0b1010);
    }
    
    void stop()
    {
       // MotorDirectionSet(0b1010);
        MotorSpeedSetAB(0, 0);
        delay(10);
        MotorDirectionSet(0b1010);
    }
};


pvfm_i2c_motor  SuctionValve;

#endif