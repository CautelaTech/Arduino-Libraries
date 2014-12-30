//
#include <MsTimer2.h>
#include <Streaming.h>
#include <EEPROM.h>
#include "pvfm_temp.h"
#include "pvfm_dta.h"


void setup()
{
    Serial.begin(115200);
    
    cout << "hello world" << endl;
    
    
    //ptp.makeArray();
    
    ptp.begin();
    ptp.setTemp(100);
    
}


long timer1 = 0;
void loop()
{
    //ptp.pushDta();
    delay(1);
    if((millis() - timer1) > 200)
    {
        timer1 = millis();
        cout << ptp.get_kt(0) << '\t' << ptp.get_kt(1) << '\t' << ptp.get_kt(2) << endl;
    }
}