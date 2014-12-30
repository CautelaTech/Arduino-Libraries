#include <Wire.h>
#include <pvfm_i2c_motor.h>
#include <Streaming.h>
#include <EEPROM.h>
#include <MsTimer2.h>

void setup()
{
    Serial.begin(115200);
    cout << "hello world" << endl;
    
    Wire.begin();
    
    delay(1000);
    SuctionValve.init();
}

void loop()
{
    cout << "run\r\n";
    SuctionValve.run();
    delay(2000);
    
    cout << "stop\r\n";
    SuctionValve.stop();
    delay(2000);
}
