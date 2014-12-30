// 

#include <Streaming.h>
#include "pvfm_dta.h"


void pvfm_dta::write_word_eeprom(int addr, unsigned int dta)
{
    EEPROM.write(addr, dta>>8);
    EEPROM.write(addr+1, dta);
}


pvfm_dta P_DTA;