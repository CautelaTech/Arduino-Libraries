#include <Streaming.h>
#include <TFTv2.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SeeedTouchScreen.h>
#include <SD.h>
#include <pvfm_dta.h>

#include "pvfm_ui.h"
#include "pvfm_ui_dfs.h"

void setup()
{
    Serial.begin(115200);
    cout << "hello world" << endl;
    
    UI.begin();
    
    UI.normalPage();

   // UI.dispSpecialBuff_test();
 
}


void loop()
{

    int item = UI.getTouchItem2();
    
    if(item)
    {
        UI.setNum(item);
       // cout << item << endl;
       
       
    }
}