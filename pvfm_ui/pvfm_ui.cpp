#include <Streaming.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include <pvfm_dta.h>
#include <SD.h>
#include <Arduino.h>

#include "pvfm_ui.h"
#include "pvfm_ui_dfs.h"

TouchScreen ts = TouchScreen(XP, YP, XM, YM);



const int PIN_SD_CS = 4;                        // pin of sd card

const long __Gnbmp_height = 320;                 // bmp hight
const long __Gnbmp_width  = 240;                 // bmp width

long __Gnbmp_image_offset  = 0;;

int __Gnfile_num = 0;                           // num of file

File bmpFile;

#define BUFFPIXEL       60                          // must be a divisor of 240 
#define BUFFPIXEL_X3    180                         // BUFFPIXELx3


#define UP_DOWN     1
#define DOWN_UP     0

// dir - 1: up to down
// dir - 2: down to up


/*********************************************/
// These read data from the SD card file and convert them to big endian
// (the data is stored in little endian format!)

// LITTLE ENDIAN!



uint16_t read16(File f)
{
    uint16_t d;
    uint8_t b;
    b = f.read();
    d = f.read();
    d <<= 8;
    d |= b;
    return d;
}

// LITTLE ENDIAN!
uint32_t read32(File f)
{
    uint32_t d;
    uint16_t b;

    b = read16(f);
    d = read16(f);
    d <<= 16;
    d |= b;
    return d;
}


void bmpdraw(File f, int x, int y, int dir)
{

    if(bmpFile.seek(__Gnbmp_image_offset))
    {
        // Serial.print("pos = ");
        // Serial.println(bmpFile.position());
    }
    
    uint32_t time = millis();

    uint8_t sdbuffer[BUFFPIXEL_X3];                                         // 3 * pixels to buffer

    for (int i=0; i< __Gnbmp_height; i++)
    {
        if(dir)
        {
            bmpFile.seek(__Gnbmp_image_offset+(__Gnbmp_height-1-i)*240*3);
        }

        for(int j=0; j<(240/BUFFPIXEL); j++)
        {
        
            bmpFile.read(sdbuffer, BUFFPIXEL_X3);
            uint8_t buffidx = 0;
            int offset_x = j*BUFFPIXEL;
            
            unsigned int __color[BUFFPIXEL];
            
            for(int k=0; k<BUFFPIXEL; k++)
            {
                __color[k] = sdbuffer[buffidx+2]>>3;                        // read
                __color[k] = __color[k]<<6 | (sdbuffer[buffidx+1]>>2);      // green
                __color[k] = __color[k]<<5 | (sdbuffer[buffidx+0]>>3);      // blue
                
                buffidx += 3;
            }

            Tft.setCol(offset_x, offset_x+BUFFPIXEL);

            if(dir)
            {
                Tft.setPage(i, i);
            }
            else
            {
                Tft.setPage(__Gnbmp_height-i-1, __Gnbmp_height-i-1);
            }

            Tft.sendCMD(0x2c);                                                  
            
            TFT_DC_HIGH;
            TFT_CS_LOW;

            for(int m=0; m < BUFFPIXEL; m++)
            {
                SPI.transfer(__color[m]>>8);
                SPI.transfer(__color[m]);
            }

            TFT_CS_HIGH;
        }
        
    }
    
    //Serial.print(millis() - time, DEC);
    //Serial.println(" ms");
}

bool bmpReadHeader(File f) 
{
    // read header
    uint32_t tmp;
    uint8_t bmpDepth;
    
    if (read16(f) != 0x4D42) {
        // magic bytes missing
        return false;
    }

    // read file size
    tmp = read32(f);
    //Serial.print("size 0x");
    //Serial.println(tmp, HEX);

    // read and ignore creator bytes
    read32(f);

    __Gnbmp_image_offset = read32(f);
    //Serial.print("offset ");
    //Serial.println(__Gnbmp_image_offset, DEC);

    // read DIB header
    tmp = read32(f);
    Serial.print("header size ");
    Serial.println(tmp, DEC);
    
    int bmp_width = read32(f);
    int bmp_height = read32(f);
    
    if(bmp_width != __Gnbmp_width || bmp_height != __Gnbmp_height)      // if image is not 320x240, return false
    {
        return false;
    }

    if (read16(f) != 1)
    return false;

    bmpDepth = read16(f);
    //Serial.print("bitdepth ");
    //Serial.println(bmpDepth, DEC);

    if (read32(f) != 0) {
        // compression not supported!
        return false;
    }

    //Serial.print("compression ");
    //Serial.println(tmp, DEC);

    return true;
}


void sd_init()
{
    pinMode(PIN_SD_CS,OUTPUT);
    digitalWrite(PIN_SD_CS,HIGH);
    
    
    Sd2Card card;
    card.init(SPI_FULL_SPEED, PIN_SD_CS); 
    
    if(!SD.begin(PIN_SD_CS))              
    { 
        Serial.println("failed!");
        while(1);                               // init fail, die here
    }
    

}


void disp_confirm()
{
    bmpFile = SD.open("pfvm_1.bmp");
    
        if (! bmpFile)
        {
            Serial.println("didnt find image");
            while (1);
        }

        if(! bmpReadHeader(bmpFile)) 
        {
            Serial.println("bad bmp");
            return;
        }

        bmpdraw(bmpFile, 0, 0, 1);
        bmpFile.close();
}

void disp_modify()
{
    bmpFile = SD.open("pfvm_2.bmp");
    
        if (! bmpFile)
        {
            Serial.println("didnt find image");
            while (1);
        }

        if(! bmpReadHeader(bmpFile)) 
        {
            Serial.println("bad bmp");
            return;
        }

        bmpdraw(bmpFile, 0, 0, 1);
        bmpFile.close();
}


void pvfm_ui::begin()
{
    TFT_BL_ON;          // turn on the background light
    Tft.TFTinit();      // init TFT library

    num_set = 0;

    value[0] = P_DTA.get_temps();                    // temperature set
    value[1] = P_DTA.get_tempn();                    // temperature now
    value[2] = P_DTA.get_time1();                    // timer1
    value[3] = P_DTA.get_time2();                    // timer2

    temp_now_buf = value[1];
    
    
    sd_init();

}


void pvfm_ui::updateValue()
{
    int yoffset = 20;

    Tft.drawNumber(value[0], 120, 64*0+yoffset, 3, make_color(0, 0, 0));
    Tft.drawNumber(value[2], 120, 64*2+yoffset, 3, make_color(0, 0, 0));
    Tft.drawNumber(value[3], 120, 64*3+yoffset, 3, make_color(0, 0, 0));

    int clr = make_color(COLOR_TEMP_NOW_R, COLOR_TEMP_NOW_G, COLOR_TEMP_NOW_B);
    dispNum(value[1], -1, 120, 64+20, 3, make_color(0, 0, 0), clr);
}

unsigned char pvfm_ui::updateTemp()                                             // refresh temperature
{

    int clr = make_color(103, 0, 219);
    
    dispNum(value[1], temp_now_buf, 130, 30, 4, make_color(255, 255, 255), clr);
}

void pvfm_ui::setTempNow(int tpn)                                               // set temprature now
{
    temp_now_buf = value[1];
    value[1] = tpn;
}

void pvfm_ui::setTempNow()
{
    temp_now_buf = value[1];
    value[1] = P_DTA.get_tempn();
}

void pvfm_ui::dispAddMinus(int x, int y, unsigned int color)
{

    int h_t  = 12;
    int d_t  = 10;

    // unsigned int color_up   = make_color(255, 0, 0);

    for(int i=0; i<h_t; i++)
    {
        Tft.drawHorizontalLine(x-i, y+i, 2*i+1, color);                      // up

        Tft.drawHorizontalLine(x-i+55, y+h_t-i, 2*i+1, color);                      // up
    }
}

/*
void pvfm_ui::updateValue()
{
    
}*/

void pvfm_ui::upDateTempS()
{
    unsigned int color_white = make_color(255, 255, 255);
    unsigned int color[3] = {make_color(103, 0, 219), make_color(77, 79, 218), make_color(83, 162, 218)};
    
    dispNum(P_DTA.get_temps(), P_DTA.get_temps_buf(), 20, 43, 3, color_white, color[0]);
}

void pvfm_ui::upDateTempS2()
{
    unsigned int color_white = make_color(255, 255, 255);
    unsigned int color[3] = {make_color(103, 0, 219), make_color(77, 79, 218), make_color(83, 162, 218)};
    
    dispNum(P_DTA.get_temps(), P_DTA.get_temps()+111, 20, 43, 3, color_white, color[0]);
}



void pvfm_ui::upDateTempN()
{
    unsigned int color_white = make_color(255, 255, 255);
    unsigned int color[3] = {make_color(103, 0, 219), make_color(77, 79, 218), make_color(83, 162, 218)};
    
    dispNum(P_DTA.get_tempn(), P_DTA.get_tempn_buf(), 20, 43, 3, color_white, color[0]);
}

void pvfm_ui::upDateTempN2()
{
    unsigned int color_white = make_color(255, 255, 255);
    unsigned int color[3] = {make_color(103, 0, 219), make_color(77, 79, 218), make_color(83, 162, 218)};
    
    dispNum(P_DTA.get_tempn(), (P_DTA.get_tempn()+111), 20, 43, 3, color_white, color[0]);
}


void pvfm_ui::normalPage()
{

    disp_modify();
   // unsigned int color = make_color(255, 255, 255);
    
   // Tft.drawNumber(P_DTA.get_temps(), 20, 43, 3, color);
  //  Tft.drawNumber(P_DTA.get_time1(), 20, 43+80, 3, color);
  //  Tft.drawNumber(P_DTA.get_time2(), 20, 43+160, 3, color);
    
    
    
    unsigned int color_white = make_color(255, 255, 255);
    unsigned int color[3] = {make_color(103, 0, 219), make_color(77, 79, 218), make_color(83, 162, 218)};
    
    
    upDateTempS2();
    //upDateTempN2();
    
    setTempNow(value[0]+111);
    
    
    //dispNum(P_DTA.get_tempn(), P_DTA.get_tempn()+111, 130, 30, 4, make_color(255, 255, 255), clr);
    

    
/*
    unsigned int color_x[3][9] = 
    {
        {
            make_color(132, 59, 253), make_color(116, 51, 213), make_color(145, 79, 253),
            make_color(101, 10, 253), make_color(81, 8, 202), make_color(117, 35, 253), 
            make_color(81, 8, 202), make_color(65, 6, 162), make_color(94, 28, 202), 
        },
        {
            make_color(93, 127, 253), make_color(110, 140, 253), make_color(85, 112, 213),
            make_color(53, 95, 252), make_color(75, 111, 252), make_color(42, 76, 202), 
            make_color(43, 76, 202), make_color(59, 89, 202), make_color(34, 61, 162), 
        },
        {
            make_color(51, 189, 254), make_color(51, 161, 213), make_color(92, 202, 254),
            make_color(0, 173, 254), make_color(0, 138, 203), make_color(51, 189, 254), 
            make_color(0, 138, 203), make_color(0, 110, 162), make_color(41, 151, 203), 
        },
    };
    
    int x = 0;
    int y = 0;
    
    unsigned char startX[] = {1, 118, 120, 178, 180, 238};
    unsigned int startY[]  = {1, 6, 74, 79};
    unsigned char startY_Y[] = {0, 80, 160, 240};
    
    
    
    
    for(int i=0; i<3; i++)
    {
        drawItem(startX[0], startY_Y[i], color_x[i][0], color_x[i][3], color_x[i][6]);
    }
    
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<2; j++)
        {
            drawButton(startX[2*j+2], startY_Y[i], color_x[i][1+j], color_x[i][4+j], color_x[i][7+j], 0);
            
           // delay(1000);
        }
    }
    
    
    drawStateMunu(0);
    
  /*  
    drawButton(0, 0, color_x[0][1], color_x[0][4], color_x[0][7], 0);
    
    int __st        = 0;
    int __stbuf     = 0;
    
    int cnt = 0;
    while(1)
    {
    
        if(isTouch())
        {
            cout << "touch" << endl;
            drawButton(0, 0, color_x[0][1], color_x[0][4], color_x[0][7], 1);
            cnt = 0;
            
            long timer1 = millis();
            
            for(;;)
            {
                if(isTouch()) timer1 = millis();
                
                if(millis()-timer1 > 100)break;
            }

            drawButton(0, 0, color_x[0][1], color_x[0][4], color_x[0][7], 0);
        }
    }
    */
}

void pvfm_ui::drawAllBtn()
{
    unsigned int color_x[3][9] = 
    {
        {
            make_color(132, 59, 253), make_color(116, 51, 213), make_color(145, 79, 253),
            make_color(101, 10, 253), make_color(81, 8, 202), make_color(117, 35, 253), 
            make_color(81, 8, 202), make_color(65, 6, 162), make_color(94, 28, 202), 
        },
        {
            make_color(93, 127, 253), make_color(110, 140, 253), make_color(85, 112, 213),
            make_color(53, 95, 252), make_color(75, 111, 252), make_color(42, 76, 202), 
            make_color(43, 76, 202), make_color(59, 89, 202), make_color(34, 61, 162), 
        },
        {
            make_color(51, 189, 254), make_color(51, 161, 213), make_color(92, 202, 254),
            make_color(0, 173, 254), make_color(0, 138, 203), make_color(51, 189, 254), 
            make_color(0, 138, 203), make_color(0, 110, 162), make_color(41, 151, 203), 
        },
    };
    
    unsigned char startX[] = {1, 118, 120, 178, 180, 238};
    unsigned int startY[]  = {1, 6, 74, 79};
    unsigned char startY_Y[] = {0, 80, 160, 240};
    
    
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<2; j++)
        {
            drawButton(startX[2*j+2], startY_Y[i], color_x[i][1+j], color_x[i][4+j], color_x[i][7+j], 0);
            
           // delay(1000);
        }
    }
}


int pvfm_ui::isGotoModify()
{
    return getTouchRect(0, 240, 239, 319);
}


void pvfm_ui::modeModify()
{
    disp_confirm();

    cout << "goto modify mode" << endl;
   // drawAllBtn();
    
    upDateTempS2();
    
    
    while(1)
    {
        if(isGotoModify())
        {
            normalPage();
            break;
        }
        
        if(getTouchRect(120, 0, 180, 80))                   // tempn -
        {
        
            cout << "--" << endl;
            int temps = P_DTA.get_temps();
            temps -= 5;
            
            P_DTA.set_temps(temps);
            
            upDateTempS();
            
            long timer1 = millis();
            while(1)
            {
                if(getTouchRect(120, 0, 180, 80))
                {
                    timer1 = millis();
                }
                
                if(millis() - timer1 > 100)break;
                
                delay(1);              
            }
        }
        
        else if(getTouchRect(180, 0, 239, 80))              // tempn +
        {
        
            cout << "++" << endl;
            int temps = P_DTA.get_temps();
            temps += 5;
            
            
            P_DTA.set_temps(temps);
            
            upDateTempS();
            
            long timer1 = millis();
            while(1)
            {
                if(getTouchRect(180, 0, 239, 80))
                {
                    timer1 = millis();
                }
                
                if(millis() - timer1 > 100)break;
                
                delay(1);              
            }
        }
        
        delay(1);
    }
    
    
    
    
}

// state: 0 unpressed, 1 pressed
void pvfm_ui::drawButton(int x, int y, int color1, int color2, int color3, int state)
{

    unsigned char startX[] = {0, 58};
    unsigned char startY[] = {0, 5, 73, 78};
    unsigned int color[3] = {color1, color2, color3};    

    unsigned int __color;
    
    for(int i=0; i<3; i++)
    {
        __color = state ? color[2] : color[i];
        Tft.fillScreen(x+startX[0], x+startX[1]-1, y+startY[i], y+startY[i+1]-1, __color);
    }

}

void pvfm_ui::drawItem(int x, int y, int color1, int color2, int color3)
{

    unsigned char startX[] = {0, 117};
    unsigned char startY[] = {0, 5, 73, 78};
    unsigned int color[3] = {color1, color2, color3};
    
    for(int i=0; i<3; i++)
    {
        Tft.fillScreen(x+startX[0], x+startX[1]-1, y+startY[i], y+startY[i+1]-1, color[i]);
    }
    
    Tft.drawString("Target Temp",10, 5, 1, make_color(255, 255, 255));
    
}

// state = 0 - modify
// state = 1 - confirm
void pvfm_ui::drawStateMunu(int state)
{
    int color[3] = {make_color(132, 221, 207), make_color(100, 208, 204), make_color(78, 162, 159)};
    
    int x = 1;
    int y = 240;
    
    unsigned char startX[] = {0, 237};
    unsigned char startY[] = {0, 5, 73, 78};
    
    for(int i=0; i<3; i++)
    {
        Tft.fillScreen(x+startX[0], x+startX[1]-1, y+startY[i], y+startY[i+1]-1, color[i]);
    }
    
}




int pvfm_ui::getVal(int wh_val)
{
    if(wh_val > 3 || wh_val < 0)return -1;

    return value[wh_val];
}

void pvfm_ui::setValue(int val, int which_val)                                   // set value
{
    if(which_val > 3 || which_val < 0)return;

    value[which_val] = val;

    if(which_val == 0)P_DTA.set_temps(val);
    else if(which_val == 2)P_DTA.set_time1(val);
    else if(which_val == 3)P_DTA.set_time2(val);

}

unsigned int pvfm_ui::make_color(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int color_ = r;

    color_ = r>>3;                              // red
    color_ = color_<<6 | (g>>2);                // green
    color_ = color_<<5 | (b>>3);                // blue

    return color_;
}

bool pvfm_ui::isTouch()
{
    Point p = ts.getPoint();
    if(p.z > __PRESURE)return 1;
    else return 0;
}

bool pvfm_ui::getTouchRect(int XL, int YU, int XR, int YD)
{
    if(!isTouch())return 0;
    Point p = ts.getPoint();

    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

    if(p.z < __PRESURE)return 0;

    if(p.x > XL && p.x < XR && p.y > YU && p.y < YD)return 1;
    return 0;
}

unsigned char pvfm_ui::getTouchItem()
{
    if(!isTouch())return 0;

    Point p = ts.getPoint();

    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

    if(p.z < __PRESURE)return 0;


    if(p.y < LOCA_TEMP_NOW_Y)
    {
        return 1;
    }
    else if(p.y < LOCA_TIME1_Y)
    {
        return 0;
    }
    else if(p.y < LOCA_TIME2_Y)
    {
        return 3;
    }
    else if(p.y < LOCA_STATUS_Y)
    {
        return 4;
    }
    else return 5;

}

unsigned char pvfm_ui::getTouchItem2()
{
    if(!isTouch())return 0;

    Point p = ts.getPoint();

    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

    if(p.z < __PRESURE)return 0;

    //cout << p.x << '\t' << p.y << endl;

    unsigned char startX[4] = {0, 130, 185, 240};
    unsigned int startY[6] = {0, 64, 128, 192, 256, 320};

    if(p.x<startX[1])return 0;

    for(int i=0; i<4; i++)
    {
        if(p.y<startY[i+1])
        {
            if(p.x<startX[2])
            {
                return (1+2*i);
            }
            return (2+2*i);
        }
    }

}

void pvfm_ui::dispSetMode()
{
    int x_t  = 50;
    int y_t  = 120;

    int h_t  = 40;

    int len_r = 35;
    int wid_r = 40;

    unsigned int color_up   = make_color(213, 43, 43);
    unsigned int color_down = make_color(87, 170, 85);

    unsigned int color_bk   = make_color(162, 134, 94);

    Tft.fillRectangle(0, 0, 239, 319, color_bk);

    for(int i=0; i<h_t; i++)
    {
        Tft.drawHorizontalLine(x_t-i, y_t+i, 2*i+1, color_up);                      // up

        Tft.drawHorizontalLine(240-x_t-i, y_t+h_t+wid_r-i, 2*i+1, color_down);      // down
    }

    Tft.fillRectangle(x_t-(len_r/2), y_t+h_t, len_r, wid_r, color_up);              // up
    Tft.fillRectangle(240-x_t-len_r/2, y_t, len_r, wid_r, color_down);              // down

    int wid_ok  = 60;
    int fsize   = 4;

    Tft.fillRectangle(0, 320-wid_ok, 240, wid_ok, make_color(192, 192, 105));        // ok

    Tft.drawChar('O', 65, 320-wid_ok+(8*fsize/2),fsize, make_color(208, 47, 112));
    Tft.drawChar('K', 145, 320-wid_ok+(8*fsize/2),fsize, make_color(208, 47, 112));

    Tft.fillRectangle(0, 0, 240, 60, make_color(255, 255, 255));        // display

}

const unsigned char steps_set_num[4] = {5, 1, 1, 1};

unsigned int pvfm_ui::setNum(int item, int num_input, int _min, int _max)
{


    if(num_input < 0)return -1;

    int step = steps_set_num[item];

    dispSetMode();

    unsigned long timer_tmp = millis();
    int num_buf = num_input+111;

    Tft.fillRectangle(0, 0, 240, 60, make_color(255, 255, 255));            // display
    dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));

    for(;;)
    {
        long timer_1 = millis();
        long timer_2 = timer_1;
        int speed_cnt = 100;
        timer_tmp = millis();

        if(getTouchRect(10, 120, 90, 200))            // up
        {

            num_buf = num_input;
            num_input = num_input<_max ? num_input+step : num_input;
            dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));

            for(;;)
            {
                if(getTouchRect(10, 120, 90, 200))
                {
                    timer_tmp = millis();
                    delay(1);
                }

                if(millis()-timer_tmp > 100)break;                          // release for 100ms , that is released
                else
                {
                    if(millis()-timer_1 > 500 && (millis() - timer_2 > speed_cnt))
                    {
                        timer_2 = millis();
                        num_buf = num_input;
                        num_input = num_input < _max ? num_input+step : num_input;
                        dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));

                        //speed_cnt = speed_cnt > 20 ? (speed_cnt>90 ? speed_cnt-1 : (speed_cnt > 50 ? speed_cnt-3 : speed_cnt - 5)) : speed_cnt;

                        if(speed_cnt > 100-8)speed_cnt -= 1;                   // speed faster
                        else if(speed_cnt > 100-16) speed_cnt -= 2;
                        else if(speed_cnt > 100-32) speed_cnt -= 4;
                        else speed_cnt -= 8;

                        speed_cnt = speed_cnt<10 ? 10 : speed_cnt;
                    }
                }
            }
        }
        else if(getTouchRect(150, 120, 230, 200))           // down
        {
            num_buf = num_input;
            num_input = num_input>_min ? num_input-step : num_input;
            dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));

            for(;;)
            {
                if(getTouchRect(150, 120, 230, 200))
                {
                    timer_tmp = millis();
                    delay(1);
                }

                if(millis()-timer_tmp > 100)break;                          // release for 100ms , that is released
                else
                {
                    if(millis()-timer_1 > 500 && (millis() - timer_2 > speed_cnt))
                    {
                        timer_2 = millis();
                        num_buf = num_input;
                        num_input = num_input > _min ? num_input-step : num_input;
                        dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));


                        if(speed_cnt > 100-8)speed_cnt -= 1;                   // speed faster
                        else if(speed_cnt > 100-16) speed_cnt -= 2;
                        else if(speed_cnt > 100-32) speed_cnt -= 4;
                        else speed_cnt -= 8;

                        speed_cnt = speed_cnt<10 ? 10 : speed_cnt;
                    }
                }
            }
        }
        else if(getTouchRect(0, 320-60, 239, 320))
        {
            timer_tmp = millis();

            for(;;)
            {
                if(getTouchRect(0, 320-60, 239, 320))
                {
                    timer_tmp = millis();
                    delay(1);
                }

                if(millis()-timer_tmp > 100)break;
            }

            return num_input;
        }
    }
}


int test_input[4] = {1000, 1000, 1000, 1000};

void pvfm_ui::setNum(int item)
{

    int num_buf     = 0;
    //int num_input   = test_input[(item-1)/2];
    
    
    int num_input = getVal((item-1)/2);
    
    cout << "num_input = " << num_input << endl;
    
    int step = steps_set_num[(item-1)/2];
    
    if(item%2 == 0)
    {
        step *= -1;
    }
    
    if(item == getTouchItem2())            // up
    {

        num_buf = num_input;
        num_input += step;
        
        cout << "num[" << (item-1)/2 << "] = " << num_input << endl;
        
        // dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, make_color(30, 49, 254), make_color(255, 255, 255));
        
        long timer_1 = millis();
        long timer_2 = timer_1;
        int speed_cnt = 100;
        long timer_tmp = millis();
        
        
        for(;;)
        {
            if(item == getTouchItem2())
            {
                timer_tmp = millis();
                delay(1);
            }

            if(millis()-timer_tmp > 100)break;                          // release for 100ms , that is released
            else
            {
                if(millis()-timer_1 > 500 && (millis() - timer_2 > speed_cnt))
                {
                    timer_2 = millis();
                    num_buf = num_input;
                    // num_input = num_input < _max ? num_input+step : num_input;
                    
                    num_input += step;
                    
                    
                    cout << "num[" << (item-1)/2 << "] = " << num_input << endl;
                    
                    // dispNum(num_input, num_buf, SET_MODE_NUM_X, SET_MODE_NUM_Y, 4, RED, UI.make_color(255, 255, 255));

                    if(speed_cnt > 100-8)speed_cnt -= 1;                   // speed faster
                    else if(speed_cnt > 100-16) speed_cnt -= 2;
                    else if(speed_cnt > 100-32) speed_cnt -= 4;
                    else speed_cnt -= 8;

                    speed_cnt = speed_cnt<10 ? 10 : speed_cnt;
                }
            }
        }
        
        setValue(num_input, (item-1)/2);
        
        cout << "num[" << (item-1)/2 << "] = " << num_input << endl;
    }
}

void pvfm_ui::dispNum(int num, int num_buf, int x, int y, int size, int color, int color_bk)
{
    if(num > 1000)return;

    if(num_buf == -1)       // no buf
    {
        num_buf = num+111;
    }

    char tmp[5];

    int pow__[3] = {1, 10, 100};

    for(int i=0; i<3; i++)
    {
        if((num/pow__[2-i])%10 != (num_buf/pow__[2-i])%10)
        {
            Tft.drawNumber((num_buf/pow__[2-i])%10, x+i*size*FONT_SPACE, y, size, color_bk);
            Tft.drawNumber((num/pow__[2-i])%10, x+i*size*FONT_SPACE, y, size, color);
        }
    }
}


/*--  文字:  好  --*/
/*--  宋体26;  此字体下对应的点阵为：宽x高=35x35   --*/
/*--  宽度不是8的倍数，现调整为：宽度x高度=40x35  --*/
const unsigned char font_test_1[] PROGMEM = {
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x01,0xC0,0x00,0x00,0x00,
    0x01,0xF0,0x00,0x00,0x00,
    0x01,0xE0,0x00,0x1C,0x00,
    0x01,0xC1,0xFF,0xFE,0x00,
    0x01,0xC0,0x00,0x1F,0x00,
    0x01,0xC0,0x00,0x3C,0x00,
    0x03,0xC0,0x00,0x70,0x00,
    0x03,0xCE,0x00,0x60,0x00,
    0x7F,0xFF,0x03,0xE0,0x00,
    0x03,0x8E,0x03,0xC0,0x00,
    0x03,0x8E,0x03,0xC0,0x00,
    0x07,0x8E,0x03,0xC0,0x00,
    0x07,0x0E,0x03,0xC0,0x00,
    0x07,0x1E,0x03,0xC3,0x00,
    0x07,0x1E,0x03,0xC7,0x80,
    0x07,0x1F,0xFF,0xFF,0xC0,
    0x0E,0x1C,0x03,0xC0,0x00,
    0x0E,0x3C,0x03,0xC0,0x00,
    0x0E,0x3C,0x03,0xC0,0x00,
    0x1E,0x38,0x03,0xC0,0x00,
    0x1C,0x38,0x03,0xC0,0x00,
    0x1F,0x78,0x03,0xC0,0x00,
    0x03,0xF0,0x03,0xC0,0x00,
    0x00,0xF8,0x03,0xC0,0x00,
    0x00,0xFE,0x03,0xC0,0x00,
    0x01,0xFF,0x03,0xC0,0x00,
    0x03,0xC7,0x83,0xC0,0x00,
    0x07,0x87,0x03,0xC0,0x00,
    0x0E,0x00,0xF7,0x80,0x00,
    0x1C,0x00,0x3F,0x80,0x00,
    0x70,0x00,0x0F,0x80,0x00,
    0x60,0x00,0x06,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
};

void pvfm_ui::dispSpecialBuff_test()
{
    dispSpecialBuff(font_test_1);
}

void pvfm_ui::dispSpecialBuff(const unsigned char * dta)
{

    int lenght = 40;
    int width  = 35;

    int pox  = 0;
    int poy  = 0;

    unsigned int color = make_color(255, 0, 0);

    unsigned char temp = 0;

    unsigned int __color[40];


    for(int i=0; i<width; i++)
    {
        for(int j=0; j<lenght/8; j++)
        {
            //temp = font_test_1[5*i+j];

            temp = pgm_read_byte(&dta[5*i+j]);

            for(int k=0; k<8; k++)
            {
                if((temp>>(7-k)) & 0x01)
                {
                    __color[j*8+k] = color;
                }
                else
                {
                    __color[j*8+k] = make_color(0, 0, 0);
                }
            }
        }

        Tft.setCol(pox, pox+40);
        Tft.setPage(poy+i, poy+i);

        Tft.sendCMD(0x2c);

        TFT_DC_HIGH;
        TFT_CS_LOW;

        for(int m=0; m < 40; m++)
        {
            SPI.transfer(__color[m]>>8);
            SPI.transfer(__color[m]);
        }
    }
}







pvfm_ui UI;