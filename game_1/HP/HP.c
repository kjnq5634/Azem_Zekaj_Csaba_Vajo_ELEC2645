#include "HP.h"

//initialise entity health
void initEntity(Entity *e, int maxHP){
   e->maxHP = maxHP;
   e->hp = maxHP;
} 
// universal damage function ( can be set based on given attack)
void takeDamage(Entity *e, int damage){
    if (e == NULL) {
        return;
    } // 0 dmg return
    if (e->hp <= damage){
        e->hp = 0;
    } //health anchored to 0 ( death regardless)
    else {
        e->hp -= damage; // remove damage from entity hp
    }
}
// universal heal function ( can be set based on item type)
void heal(Entity *e, int HealAmount){
    e-> hp += HealAmount;// add health to player
    if (e->hp >= e->maxHP){
        e->hp = e->maxHP; // caps player at max health ( spamming heals won't allow over health)
    }
}
void drawHP(const Entity *e, HEALTHBAR *bar)
{
    int filled = (e->hp * bar->width) / e->maxHP;

    for (int i = 0; i < bar->width; i++)
    {
        if (i < filled)
        {
            LCD_Draw_Rect(bar->x_pos + i, bar->y_pos,
                         1, bar->height,
                         3,1);
        }
        else
        {
            LCD_Draw_Rect(bar->x_pos + i, bar->y_pos,
                         1, bar->height,
                         0,1); // empty color
        }
    }
} // HP drawing function 
/*(void UpdateHP(const Entity *e, HEALTHBAR *bar){

}*/
