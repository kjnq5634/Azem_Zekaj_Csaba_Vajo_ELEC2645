#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>
#include "PWM.h"
#include "Buzzer.h"
#include "Boss.h"



void startmusic(void);
void phase2music(void);
//Phase Based music function
PHASE MusicPlayer(Boss*b);
#endif