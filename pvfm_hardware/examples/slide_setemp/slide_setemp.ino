// test of display
#include <pvfm_slide.h>
#include <Streaming.h>

PVFM_Slide slide;

int val         = 0;
int val_buf     = 0;

void setup()
{
    Serial.begin(115200);
    cout << "hello world" << endl;
    
}

void loop()
{

    val = slide.getTempSet();
    
    //val = map(val, 0, 1023, 0, 35);
    if(val != val_buf)
    {
        val_buf = val;
        cout << val << endl;
    }
    
    delay(10);
}