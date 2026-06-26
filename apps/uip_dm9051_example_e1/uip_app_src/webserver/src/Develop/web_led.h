#ifndef __BOARD_H__
#define __BOARD_H__

#include "stdint.h"

void Set_LED_mode(char lkkcode);
void Get_Pin_state(void);
void Get_Sensor_val(void);

extern int CurrTmp;
extern int CurrHum;
extern char sCurrLED[4]; // "on"/"off"
extern char sCurrP05[6]; // "high"/"low"

//extern
uint8_t SenseGetTemp(void);
void SenseGetPin_inbuf(char *bufLED, char *BufP05);
void LedSetOperate(char lkkcode);

#endif /* __BOARD_H__ */

