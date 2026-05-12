#include "Background.h"
#include "LCD.h"

void Background_Draw(void)
{
    // sky is already drawn by LCD_Fill_Buffer(7) before PongEngine_Draw().
    // adds grass

    LCD_Draw_Rect(0, 190, 240, 50, 2, 1);     // grass

    //adds some bush shapes near the sniper (could be placed wherever to be fair)
    LCD_Draw_Circle(170, 200, 13, 2, 1);
    LCD_Draw_Circle(190, 198, 17, 2, 1);
    LCD_Draw_Circle(215, 200, 12, 2, 1);
}

void Background_Draw_Night(void) {
    //same as daytime just darker greenery
    LCD_Draw_Rect(0, 190, 240, 50, 6, 1); //grass

    LCD_Draw_Circle(170, 200, 13, 6, 1); //three circles are
    LCD_Draw_Circle(190, 198, 17, 6, 1); //bushes
    LCD_Draw_Circle(215, 200, 12, 6, 1);
}