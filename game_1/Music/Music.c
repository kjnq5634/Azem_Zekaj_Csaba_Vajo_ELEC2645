#include "Music.h"
#include "PWM.h"
#include "Buzzer.h"

extern Buzzer_cfg_t buzzer_cfg;
extern Boss b; //for Boss phases
// melodies and durations based on the Buzzer.h files provided by the module
uint16_t startmelody[] = {
        NOTE_D4, NOTE_D4, NOTE_D4, 
        NOTE_DS4, 
        NOTE_E4,                
        NOTE_D4, NOTE_D4, NOTE_D4,                 
        NOTE_CS4,
        NOTE_C4,
        NOTE_D4, NOTE_D4, NOTE_D4, 
        NOTE_DS4, 
        NOTE_E4, 
    };

    // Note durations: 4 = quarter, 8 = eighth, 1 = whole (note duration is base_duration / noteDurations[i])
    uint8_t startnoteDurations[] = {
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1
    };
uint16_t phase2melody[]= {
    // Initial Crescendo
    NOTE_E5,
    NOTE_F5,
    NOTE_FS5,
    NOTE_DS5,NOTE_DS5,
    NOTE_D5,
    //main theme
    NOTE_B4,
    NOTE_AS4,NOTE_AS4,NOTE_AS4,NOTE_AS4,NOTE_AS4,NOTE_AS4,
    NOTE_D5,
    NOTE_GS4,
    NOTE_D4,
    NOTE_AS4,
    NOTE_CS5,
    NOTE_A4,
    NOTE_G4,
    NOTE_A4,
    NOTE_C5,
    NOTE_G4    
    
};
uint8_t phase2noteDurations[]={
    8,
    8,
    8,
    8,
    1,
    1,
    8,
    1,
    1,1,1,1,1,1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1
};
uint16_t base_duration = 1000;
void startmusic(void)
{
    static uint16_t i = 0;
    static uint32_t nextTick = 0;

    uint32_t now = HAL_GetTick();

    if (now < nextTick)
        return; // slight delay on the music allows action to occur every tick
 
    uint16_t note = startmelody[i];
    uint16_t duration = base_duration/ startnoteDurations[i];

    if (note != 0)
        buzzer_note(&buzzer_cfg, note, duration);
    else
        buzzer_off(&buzzer_cfg);

    nextTick = now + duration + 50;

    i++;

    if (i >= sizeof(startmelody)/sizeof(startmelody[0]))
     i = 5;  //creates an infinite loop within the start menu so the music constantly plays
}
//Phase 2 music player ( works the same as startmusic just with differnt notes)
void phase2music(void)
{
    static uint16_t i = 0;
    static uint32_t nextTick = 0;

    uint32_t now = HAL_GetTick();

    if (now < nextTick)
        return; // slight delay on the music , allows action to occur every tick.
 
    uint16_t note = phase2melody[i];
    uint16_t duration = base_duration/ phase2noteDurations[i];

    if (note != 0)
        buzzer_note(&buzzer_cfg, note, duration);
    else
        buzzer_off(&buzzer_cfg);

    nextTick = now + duration + 50;}
//Music Controller
//shifts based on healthpoints and phases 
PHASE MusicPlayer(Boss*b){
    PHASE current_phase = PHASE1;
    int health = b->hp/2;
    if (health>50){
        current_phase = PHASE1;
    }else if( health<50 && health>0){
        current_phase = PHASE2;
    }
    switch(current_phase){
        case PHASE1 : 
        startmusic();
        break;
        case PHASE2: 
        phase2music();
        break;

    }
}