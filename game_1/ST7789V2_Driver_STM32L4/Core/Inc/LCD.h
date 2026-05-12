/*
Functions for using LCD screen with ST7789V2 controller.
Drawing pixels, shapes, text, sprites, etc.
All colours are in RGB565 format (16-bit).

Written by Aeron Jarvis and James Avery
*/

#ifndef LCD_h
#define LCD_h

#include "ST7789V2_Driver.h"
#include <stdlib.h>

// ========== Colour definitions ==========

// Each pixel on the LCD is represented by a 16-bit value in RGB565 format
// https://github.com/newdigate/rgb565_colours or https://rgbcolourpicker.com/565
// for more colours. You can also use an online converter like http://www.rinkydinkelectronics.com/calc_rgb565.php  
//
// However before use here they need to be byte-swapped for ST7789V2 little-endian format.
// i.e. Red is 0xF800 (big endian) but byte swapped to 0x00F8 (little endian) here.

// Basic colours - either chosen for ease of use or high contrast
// some based on https://sashamaps.net/docs/resources/20-colors/ 
#define RGB565_BLACK      0x0000 
#define RGB565_WHITE      0xFFFF 
#define RGB565_RED        0x00F8 
#define RGB565_GREEN      0xE007 
#define RGB565_BLUE       0x1F00 
#define RGB565_YELLOW     0xE0FF 
#define RGB565_CYAN       0xFF07 
#define RGB565_MAGENTA    0x1FF8
#define RGB565_GREY       0x1084
#define RGB565_LIGHT_GREY 0x18C6
#define RGB565_DARK_GREY  0x0842
#define RGB565_ORANGE     0x20FD
#define RGB565_BROWN      0x45A1
#define RGB565_PINK       0x18FC
#define RGB565_PURPLE     0x0F78
#define RGB565_TEAL       0x3804
#define RGB565_NAVY       0x0F00
#define RGB565_MAROON     0x0080
#define RGB565_OLIVE      0x0084
#define RGB565_SKY_BLUE   0x7D86
#define RGB565_GOLD       0xA0FE
#define RGB565_VIOLET     0x5C91
#define RGB565_RED_BRIGHT    0xC9E0  
#define RGB565_GREEN_BRIGHT  0xA93D  
#define RGB565_BLUE_BRIGHT   0x0B42  
#define RGB565_APRICOT       0xB6F5  
#define RGB565_LAVENDER      0x5FD8  
#define RGB565_MINT          0xF8A7  
#define RGB565_BEIGE         0xD9FD  
#define RGB565_LIME_BRIGHT   0xE8B9  
#define RGB565_MAGENTA_BRIGHT 0xBCF1 
#define RGB565_CYAN_BRIGHT   0xBE44  
#define RGB565_ORANGE_BRIGHT 0x06F4  
#define RGB565_PURPLE_BRIGHT 0xDE90  
#define RGB565_PINK_BRIGHT   0x5AF8  
#define RGB565_TEAL_BRIGHT   0xD244  
#define RGB565_GREY_STANDARD 0xB5A8 

// These colours are used in the default colour palette
// Selected from the above for high contrast and variety
#define LCD_COLOUR_0  RGB565_BLACK
#define LCD_COLOUR_1  RGB565_WHITE
#define LCD_COLOUR_2  RGB565_RED
#define LCD_COLOUR_3  RGB565_GREEN
#define LCD_COLOUR_4  RGB565_BLUE
#define LCD_COLOUR_5  RGB565_ORANGE
#define LCD_COLOUR_6  RGB565_YELLOW
#define LCD_COLOUR_7  RGB565_PINK
#define LCD_COLOUR_8  RGB565_PURPLE
#define LCD_COLOUR_9  RGB565_NAVY
#define LCD_COLOUR_10 RGB565_GOLD
#define LCD_COLOUR_11 RGB565_VIOLET
#define LCD_COLOUR_12 RGB565_BROWN
#define LCD_COLOUR_13 RGB565_GREY
#define LCD_COLOUR_14 RGB565_CYAN
#define LCD_COLOUR_15 RGB565_MAGENTA

// Vintage palette - 16 colours based on vintage consoles and computers
// adapted from https://androidarts.com/palette/16pal.htm
#define RGB565_VINTAGE_0   0x0000  // Black 0x0000
#define RGB565_VINTAGE_1   0xF39C  // Grey 0x9CF3
#define RGB565_VINTAGE_2   0xFFFF  // White 0xFFFF
#define RGB565_VINTAGE_3   0x26B9  // Red 0xB926
#define RGB565_VINTAGE_4   0x71E3  // Magenta/Pink  0xE371
#define RGB565_VINTAGE_5   0xE549  // Brown 0x49E5
#define RGB565_VINTAGE_6   0x24A3  // Orange/Brown 0xA324
#define RGB565_VINTAGE_7   0x46EC  // Orange 0xEC46
#define RGB565_VINTAGE_8   0x0DF7  // Yellow 0xF70D
#define RGB565_VINTAGE_9   0x492A  // Dark Teal 0x2A49
#define RGB565_VINTAGE_10  0x4344  // Green 0x4443
#define RGB565_VINTAGE_11  0x64A6  // Yellow-Green 0xA664
#define RGB565_VINTAGE_12  0x2619  // Dark Blue 0x1926
#define RGB565_VINTAGE_13  0xB002  // Blue 	0x02B0
#define RGB565_VINTAGE_14  0x1E35  // Light Blue 0x351E
#define RGB565_VINTAGE_15  0xFDB6  // Cyan/Light Blue 0xB6FD

// Greyscale palette - 16 levels from black to white (byte-swapped RGB565)
#define RGB565_GREY_0   0x0000  // #000000
#define RGB565_GREY_1   0xC318  // #181818
#define RGB565_GREY_2   0x4529  // #282828
#define RGB565_GREY_3   0xC739  // #383838
#define RGB565_GREY_4   0x2842  // #474747
#define RGB565_GREY_5   0xAA52  // #565656
#define RGB565_GREY_6   0x2C63  // #646464
#define RGB565_GREY_7   0x8E73  // #717171
#define RGB565_GREY_8   0xEF7B  // #7e7e7e
#define RGB565_GREY_9   0x718C  // #8c8c8c
#define RGB565_GREY_10  0xD39C  // #9b9b9b
#define RGB565_GREY_11  0x55AD  // #ababab
#define RGB565_GREY_12  0xF7BD  // #bdbdbd
#define RGB565_GREY_13  0x9AD6  // #d1d1d1
#define RGB565_GREY_14  0x3CE7  // #e7e7e7
#define RGB565_GREY_15  0xFFFF  // #ffffff



// ========== Buffer Configuration ==========
#define BUFFER_LENGTH ST7789V2_HEIGHT*ST7789V2_WIDTH/2  // 4 pixels per byte (2 bits per pixel)

// ========== Function Prototypes ==========

/* Palette Selection 
*   See palette definitions in LCD.c for examples of how to create custom palettes.
*   Each palette is an array of 16 RGB565 colour values that map to colour indices 0-15. */
typedef enum {
    PALETTE_DEFAULT = 0,
    PALETTE_GREYSCALE = 1,
    PALETTE_VINTAGE = 2,
    PALETTE_CUSTOM = 3
} LCD_Palette;

/* Set Colour Palette
*   Switches the active colour palette. Buffer content remains the same (0-15), only the RBG565 colours change
*   when LCD_Refresh() is called.
*   @param palette - The palette to activate (PALETTE_DEFAULT, PALETTE_GREYSCALE, PALETTE_VINTAGE, PALETTE_CUSTOM)*/
void LCD_Set_Palette(LCD_Palette palette);

/* Initialise display
*   Powers up the display and turns on backlight.
*   Sets the display up in horizontal addressing mode and with normal video mode.*/
void LCD_init(ST7789V2_cfg_t* cfg);

/* Turn off
*   Powers down the display and turns off the backlight.*/
void LCD_turnOff(ST7789V2_cfg_t* cfg);

/* Turn on
*   Powers up the display and turns on the backlight.*/
void LCD_turnOn(ST7789V2_cfg_t* cfg);

/* Clear
*   Clears the screen buffer.*/
void LCD_clear();

/* Normal mode
*   Turn on normal video mode (default).*/
void LCD_normalMode(ST7789V2_cfg_t* cfg);

/* Inverse mode
*   Turn on inverse video mode */
void LCD_inverseMode(ST7789V2_cfg_t* cfg);

/* Print String
*   Prints a string of characters to the screen buffer. String is cut-off after the 83rd pixel.
*   @param  x - the x position (top-left)
*   @param  y - the y position (top-left)
*   @param  colour - Value from 0-15 referring to the colour map colour
*   @param  font_size - Value to scale up font by, 1 = 1x scale, 2 = 2x scale, 3 = 3x scale, etc...*/
void LCD_printString(char const *str, const uint16_t x, const uint16_t y, uint8_t colour, uint8_t font_size);

/* Print Character
*   Sends a character to the screen buffer.  Printed at the specified location. Character is cut-off after the 83rd pixel.
*   @param  c - the character to print. Can print ASCII as so printChar('C').
*   @param  x - the x position (top-left)
*   @param  y - the y position (top-left)
*   @param  colour - Value from 0-15 referring to the colour map colour*/
void LCD_printChar(char const c, const uint16_t x, const uint16_t y, uint8_t colour);

/* Set a Pixel
* @param x      The x co-ordinate of the pixel (0 to 239)
* @param y      The y co-ordinate of the pixel (0 to 279)
* @param colour The colour of the pixel
* @details This function sets the colour of a pixel in the screen buffer.*/
void LCD_Set_Pixel(const uint16_t x, const uint16_t y, uint8_t colour);

/* Get a Pixel
*   This function gets the status of a pixel in the screen buffer.
*   @param  x - the x co-ordinate of the pixel (0 to 83)
*   @param  y - the y co-ordinate of the pixel (0 to 47)
*   @returns - colour of pixel*/
uint8_t LCD_Get_Pixel(const uint16_t x, const uint16_t y);

/* Refresh display
*   This functions sends the screen buffer to the display.*/
void LCD_Refresh(ST7789V2_cfg_t* cfg);

/* Randomise buffer
*   This function fills the buffer with random data.  Can be used to test the display.
*   A call to refresh() must be made to update the display to reflect the change in pixels.
*   The seed is not set and so the generated pattern will probably be the same each time.
*   TODO: Randomise the seed - maybe using the noise on the AnalogIn pins.*/
void LCD_randomiseBuffer();

/* Plot Array
*   This function plots a one-dimensional array in the buffer.
*   @param array[] - y values of the plot. Values should be normalised in the range 0.0 to 1.0. First 84 plotted.
*   @param colour  - Value from 0-15 referring to the colour map colour.*/
void LCD_plotArray(float const array[], const uint8_t colour);

/* Draw Circle
*   This function draws a circle at the specified origin with specified radius in the screen buffer
*   Uses the midpoint circle algorithm.
*   @see http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
*   @param  x0     - x-coordinate of centre
*   @param  y0     - y-coordinate of centre
*   @param  radius - radius of circle in pixels
*   @param  colour - Value from 0-15 referring to the colour map colour
*   @param  fill   - fill-type for the shape*/
void LCD_Draw_Circle(const uint16_t x0, const uint16_t y0, const uint16_t radius, const uint8_t colour, const uint8_t fill);

/* Draw Line
*   This function draws a line between the specified points using linear interpolation.
*   @param  x0 - x-coordinate of first point
*   @param  y0 - y-coordinate of first point
*   @param  x1 - x-coordinate of last point
*   @param  y1 - y-coordinate of last point
*   @param  colour - 4-bit colour*/
void LCD_Draw_Line(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint8_t colour);

/* Draw Rectangle
*   This function draws a rectangle.
*   @param  x0 - x-coordinate of origin (top-left)
*   @param  y0 - y-coordinate of origin (top-left)
*   @param  width - width of rectangle
*   @param  height - height of rectangle
*   @param  colour - Value from 0-15 referring to the colour map colour
*   @param  fill   - fill-type for the shape*/
void LCD_Draw_Rect(const uint16_t x0, const uint16_t y0, const uint16_t width, const uint16_t height, const uint8_t colour, const uint8_t fill);

/* Draw Sprite
*   This function draws a sprite as defined in a 2D array. Sprite values directly map to colour indices.
*   @param  x0 - x-coordinate of origin (top-left)
*   @param  y0 - y-coordinate of origin (top-left)
*   @param  nrows - number of rows in sprite
*   @param  ncols - number of columns in sprite
*   @param  sprite - 2D array (255=transparent, 0=LCD_COLOUR_0/black, 1=LCD_COLOUR_1/white, etc.)*/
void LCD_Draw_Sprite(const uint16_t x0, const uint16_t y0, const uint16_t nrows, const uint16_t ncols, const uint8_t *sprite);

/* Draw Sprite with Colour Override
*   This function draws a sprite with all non-transparent pixels set to the specified colour. 255 is transparent.
*   @param  x0 - x-coordinate of origin (top-left)
*   @param  y0 - y-coordinate of origin (top-left)
*   @param  nrows - number of rows in sprite
*   @param  ncols - number of columns in sprite
*   @param  sprite - 2D array (255=transparent, any other value=draw with colour)
*   @param  colour - Value from 0-15 referring to the colour map colour (replaces all non-transparent values)*/
void LCD_Draw_Sprite_Colour(const uint16_t x0, const uint16_t y0, const uint16_t nrows, const uint16_t ncols, const uint8_t *sprite, const uint8_t colour);

/* Draw Sprite Scaled
*   This function draws a sprite scaled by an integer factor. Sprite values directly map to colour indices.
*   @param  x0 - x-coordinate of origin (top-left)
*   @param  y0 - y-coordinate of origin (top-left)
*   @param  nrows - number of rows in sprite
*   @param  ncols - number of columns in sprite
*   @param  sprite - 2D array (255=transparent, 0=LCD_COLOUR_0/black, 1=LCD_COLOUR_1/white, etc.)
*   @param  scale - integer scale factor (1=original size, 2=double size, 3=triple, etc.)*/
void LCD_Draw_Sprite_Scaled(const uint16_t x0, const uint16_t y0, const uint16_t nrows, const uint16_t ncols, const uint8_t *sprite, const uint8_t scale);

/* Draw Sprite with Colour Override and Scaling
*   This function draws a sprite scaled by an integer factor with all non-transparent pixels set to the specified colour.
*   @param  x0 - x-coordinate of origin (top-left)
*   @param  y0 - y-coordinate of origin (top-left)
*   @param  nrows - number of rows in sprite
*   @param  ncols - number of columns in sprite
*   @param  sprite - 2D array (255=transparent, any other value=draw with colour)
*   @param  colour - Value from 0-15 referring to the colour map colour (replaces all non-transparent values)
*   @param  scale - integer scale factor (1=original size, 2=double size, 3=triple, etc.)*/
void LCD_Draw_Sprite_Colour_Scaled(const uint16_t x0, const uint16_t y0, const uint16_t nrows, const uint16_t ncols, const uint8_t *sprite, const uint8_t colour, const uint8_t scale);

/* Fill Buffer
*   This function fills the image buffer with the desired colour
*   @param  colour - Value from 0-15 referring to the colour map colour*/
void LCD_Fill_Buffer(const uint8_t colour);

/* Fill Screen
*   This function directly writes to the LCD filling in a rectangle with a solid colour
*   x0 must be < x1 and y0 must be < y1, function does no parameter checking
*   @param  cfg - LCD Config struct
*   @param  x0 - Start x-coordinate (top-left)
*   @param  y0 - Start y-coordinate (top-left)
*   @param  x1 - End x-coordinate (bottom-right)
*   @param  y1 - End y-coordinate (bottom-right)
*   @param  colour - Value from 0-15 referring to the colour map colour*/
void LCD_Fill(ST7789V2_cfg_t* cfg, const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint16_t colour);

extern const unsigned char font5x7_[480];// = {
//     0x00, 0x00, 0x00, 0x00, 0x00,// (space)
//     0x00, 0x00, 0x5F, 0x00, 0x00,// !
//     0x00, 0x07, 0x00, 0x07, 0x00,// "
//     0x14, 0x7F, 0x14, 0x7F, 0x14,// #
//     0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
//     0x23, 0x13, 0x08, 0x64, 0x62,// %
//     0x36, 0x49, 0x55, 0x22, 0x50,// &
//     0x00, 0x05, 0x03, 0x00, 0x00,// '
//     0x00, 0x1C, 0x22, 0x41, 0x00,// (
//     0x00, 0x41, 0x22, 0x1C, 0x00,// )
//     0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
//     0x08, 0x08, 0x3E, 0x08, 0x08,// +
//     0x00, 0x50, 0x30, 0x00, 0x00,// ,
//     0x08, 0x08, 0x08, 0x08, 0x08,// -
//     0x00, 0x60, 0x60, 0x00, 0x00,// .
//     0x20, 0x10, 0x08, 0x04, 0x02,// /
//     0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
//     0x00, 0x42, 0x7F, 0x40, 0x00,// 1
//     0x42, 0x61, 0x51, 0x49, 0x46,// 2
//     0x21, 0x41, 0x45, 0x4B, 0x31,// 3
//     0x18, 0x14, 0x12, 0x7F, 0x10,// 4
//     0x27, 0x45, 0x45, 0x45, 0x39,// 5
//     0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
//     0x01, 0x71, 0x09, 0x05, 0x03,// 7
//     0x36, 0x49, 0x49, 0x49, 0x36,// 8
//     0x06, 0x49, 0x49, 0x29, 0x1E,// 9
//     0x00, 0x36, 0x36, 0x00, 0x00,// :
//     0x00, 0x56, 0x36, 0x00, 0x00,// ;
//     0x00, 0x08, 0x14, 0x22, 0x41,// <
//     0x14, 0x14, 0x14, 0x14, 0x14,// =
//     0x41, 0x22, 0x14, 0x08, 0x00,// >
//     0x02, 0x01, 0x51, 0x09, 0x06,// ?
//     0x32, 0x49, 0x79, 0x41, 0x3E,// @
//     0x7E, 0x11, 0x11, 0x11, 0x7E,// A
//     0x7F, 0x49, 0x49, 0x49, 0x36,// B
//     0x3E, 0x41, 0x41, 0x41, 0x22,// C
//     0x7F, 0x41, 0x41, 0x22, 0x1C,// D
//     0x7F, 0x49, 0x49, 0x49, 0x41,// E
//     0x7F, 0x09, 0x09, 0x01, 0x01,// F
//     0x3E, 0x41, 0x41, 0x51, 0x32,// G
//     0x7F, 0x08, 0x08, 0x08, 0x7F,// H
//     0x00, 0x41, 0x7F, 0x41, 0x00,// I
//     0x20, 0x40, 0x41, 0x3F, 0x01,// J
//     0x7F, 0x08, 0x14, 0x22, 0x41,// K
//     0x7F, 0x40, 0x40, 0x40, 0x40,// L
//     0x7F, 0x02, 0x04, 0x02, 0x7F,// M
//     0x7F, 0x04, 0x08, 0x10, 0x7F,// N
//     0x3E, 0x41, 0x41, 0x41, 0x3E,// O
//     0x7F, 0x09, 0x09, 0x09, 0x06,// P
//     0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
//     0x7F, 0x09, 0x19, 0x29, 0x46,// R
//     0x46, 0x49, 0x49, 0x49, 0x31,// S
//     0x01, 0x01, 0x7F, 0x01, 0x01,// T
//     0x3F, 0x40, 0x40, 0x40, 0x3F,// U
//     0x1F, 0x20, 0x40, 0x20, 0x1F,// V
//     0x7F, 0x20, 0x18, 0x20, 0x7F,// W
//     0x63, 0x14, 0x08, 0x14, 0x63,// X
//     0x03, 0x04, 0x78, 0x04, 0x03,// Y
//     0x61, 0x51, 0x49, 0x45, 0x43,// Z
//     0x00, 0x00, 0x7F, 0x41, 0x41,// [
//     0x02, 0x04, 0x08, 0x10, 0x20,// "\"
//     0x41, 0x41, 0x7F, 0x00, 0x00,// ]
//     0x04, 0x02, 0x01, 0x02, 0x04,// ^
//     0x40, 0x40, 0x40, 0x40, 0x40,// _
//     0x00, 0x01, 0x02, 0x04, 0x00,// `
//     0x20, 0x54, 0x54, 0x54, 0x78,// a
//     0x7F, 0x48, 0x44, 0x44, 0x38,// b
//     0x38, 0x44, 0x44, 0x44, 0x20,// c
//     0x38, 0x44, 0x44, 0x48, 0x7F,// d
//     0x38, 0x54, 0x54, 0x54, 0x18,// e
//     0x08, 0x7E, 0x09, 0x01, 0x02,// f
//     0x08, 0x14, 0x54, 0x54, 0x3C,// g
//     0x7F, 0x08, 0x04, 0x04, 0x78,// h
//     0x00, 0x44, 0x7D, 0x40, 0x00,// i
//     0x20, 0x40, 0x44, 0x3D, 0x00,// j
//     0x00, 0x7F, 0x10, 0x28, 0x44,// k
//     0x00, 0x41, 0x7F, 0x40, 0x00,// l
//     0x7C, 0x04, 0x18, 0x04, 0x78,// m
//     0x7C, 0x08, 0x04, 0x04, 0x78,// n
//     0x38, 0x44, 0x44, 0x44, 0x38,// o
//     0x7C, 0x14, 0x14, 0x14, 0x08,// p
//     0x08, 0x14, 0x14, 0x18, 0x7C,// q
//     0x7C, 0x08, 0x04, 0x04, 0x08,// r
//     0x48, 0x54, 0x54, 0x54, 0x20,// s
//     0x04, 0x3F, 0x44, 0x40, 0x20,// t
//     0x3C, 0x40, 0x40, 0x20, 0x7C,// u
//     0x1C, 0x20, 0x40, 0x20, 0x1C,// v
//     0x3C, 0x40, 0x30, 0x40, 0x3C,// w
//     0x44, 0x28, 0x10, 0x28, 0x44,// x
//     0x0C, 0x50, 0x50, 0x50, 0x3C,// y
//     0x44, 0x64, 0x54, 0x4C, 0x44,// z
//     0x00, 0x08, 0x36, 0x41, 0x00,// {
//     0x00, 0x00, 0x7F, 0x00, 0x00,// |
//     0x00, 0x41, 0x36, 0x08, 0x00,// }
//     0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
//     0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
// };

#endif