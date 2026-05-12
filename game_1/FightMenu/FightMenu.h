#ifndef FIGHTMENU_H
#define FIGHTMENU_H

#include <stdint.h>


typedef enum {
    ATKMENU=0,
    FIGHT,
    ITEM,
    MERCY,
    ACT,
    BOSSATTACK
}FIGHTSTATE; // Main game state function
typedef struct {
    uint8_t selected;
}OptionSelection_t; //option selection struct
//Menu initialiser
void FightMenuInit(OptionSelection_t *op);
// Menu update based on main game state function
FIGHTSTATE FightMenuUpdate(OptionSelection_t*op);


#endif