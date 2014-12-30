
#ifndef __PVFM_UI_DFS_H__
#define __PVFM_UI_DFS_H__

// display temperature set
#define LOCA_TEMP_SET_X     0
#define LOCA_TEMP_SET_Y     0

#define COLOR_TEMP_SET_R    115
#define COLOR_TEMP_SET_G    106
#define COLOR_TEMP_SET_B    149


// display temperature now
#define LOCA_TEMP_NOW_X     0
#define LOCA_TEMP_NOW_Y     64

#define COLOR_TEMP_NOW_R    162
#define COLOR_TEMP_NOW_G    94
#define COLOR_TEMP_NOW_B    155


// time 1
#define LOCA_TIME1_X        0
#define LOCA_TIME1_Y        128

#define COLOR_TIME1_R       170
#define COLOR_TIME1_G       102
#define COLOR_TIME1_B       85


// time2
#define LOCA_TIME2_X        0
#define LOCA_TIME2_Y        192

#define COLOR_TIME2_R       128
#define COLOR_TIME2_G       128
#define COLOR_TIME2_B       64


// STATUE
#define LOCA_STATUS_X       0
#define LOCA_STATUS_Y       256

#define COLOR_STATUS_R      77
#define COLOR_STATUS_G      179
#define COLOR_STATUS_B      179


//touch screen

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // mega
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM A0   // can be a digital pin, this is A0
#define XP A3   // can be a digital pin, this is A3 

#elif defined(__AVR_ATmega32U4__) // leonardo
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3 

#else //168, 328, something else
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3 
#endif

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2


// setting mode, number
#define SET_MODE_NUM_X  80
#define SET_MODE_NUM_Y  18

#endif