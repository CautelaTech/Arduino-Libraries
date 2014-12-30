#include <Streaming.h>
#include <EEPROM.h>

#include "pvfm_dta.h"

void setup()
{
    Serial.begin(115200);
    cout << "hello world" << endl;
    
    
    P_DTA.set_temps(300);
    P_DTA.set_tempn(296);
    
    P_DTA.set_time1(15);
    P_DTA.set_time2(20);

    cout << P_DTA.get_temps() << endl;
    cout << P_DTA.get_tempn() << endl;
    cout << P_DTA.get_time1() << endl;
    cout << P_DTA.get_time2() << endl;
    
}

void loop()
{
}