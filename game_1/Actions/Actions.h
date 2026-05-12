#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdint.h>
#include "Actions.h"
#include "FightMenu.h"
#include "Bulb.h"

// structs for item states and selection
typedef enum{
    SELECTITEM = 0,
    BATTERY ,
    CAPACITOR
}ITEMSTATES;
typedef struct {
    uint8_t itemsel;
}ItemSelected_t;



//void AttackInit(ActionSelection *atk);
//FIGHTSTATE AttackRun(ActionSelection*atk);
FIGHTSTATE ITEMRUN(void);
void ItemInit(ItemSelected_t *item);
//Item based functions
ITEMSTATES ItemSelection(ItemSelected_t *item);
ITEMSTATES BatteryFunc(Bulbparams_t *BulbParam);
ITEMSTATES CapacitorFunc(Bulbparams_t *BulbParam);
// Fighstates to allow for each function to become the main state displayed (button based interruct FSM)
FIGHTSTATE MERCYMESSAGE(void);
FIGHTSTATE ACTMESSAGE(void);
FIGHTSTATE AttackFunction (void);

#endif
