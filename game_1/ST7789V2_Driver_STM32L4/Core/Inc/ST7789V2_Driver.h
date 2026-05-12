/*
Driver for the ST7789V2 LCD display controller, for ELEC2645 module.
Intended for use with STM32L4 series microcontrollers, specifically the NUCLEO0-L476RG board.
Tested with the Pimoroni 1.54" SPI Colour Square LCD (240x240) Breakout
https://shop.pimoroni.com/products/1-54-spi-colour-square-lcd-240x240-breakout?variant=39351811702867

Written by Aeron Jarvis with extremely minor modifications by James Avery
*/

#ifndef ST7789V2_Driver_h
#define ST7789V2_Driver_h

#include <stm32l476xx.h>
#include <stdio.h>

/* Control Registers and constant codes */
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36
/**
 * Memory Data Access Control Register (0x36H)
 * MAP:     D7  D6  D5  D4  D3  D2  D1  D0
 * param:   MY  MX  MV  ML  RGB MH  -   -
 *
 */

/* Page Address Order ('0': Top to Bottom, '1': the opposite) */
#define ST7789_MADCTL_MY  0x80
/* Column Address Order ('0': Left to Right, '1': the opposite) */
#define ST7789_MADCTL_MX  0x40
/* Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode) */
#define ST7789_MADCTL_MV  0x20
/* Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite) */
#define ST7789_MADCTL_ML  0x10
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
#define ST7789_MADCTL_RGB 0x00

#define ST7789_RDID1   0xDA
#define ST7789_RDID2   0xDB
#define ST7789_RDID3   0xDC
#define ST7789_RDID4   0xDD

/* Advanced options */
#define ST7789_COLOR_MODE_16bit 0x55    //  RGB565 (16bit)
#define ST7789_COLOR_MODE_18bit 0x66    //  RGB666 (18bit)

#define ST7789_ROTATION 2	

#define ST7789V2_WIDTH 240

#define ST7789V2_HEIGHT 240

#define GPIO_SET_LSB 0

#define GPIO_RESET_LSB 16

// #ifndef GPIO_PIN_RESET
// #define GPIO_PIN_RESET 0
// #endif

// #ifndef GPIO_PIN_SET
// #define GPIO_PIN_SET 1
// #endif

void delay_ms_approx(uint16_t ms);

typedef struct GPIO_Pin_struct {
   GPIO_TypeDef* port;
   uint16_t pin;
} GPIO_Pin_t;

void gpio_write(GPIO_Pin_t gpio, uint8_t val);

typedef struct DMA_Channel_Struct {
   DMA_TypeDef *instance;
   DMA_Channel_TypeDef *channel;
} DMA_Channel_t;

typedef struct ST7789V2_cfg_struct {
   uint8_t setup_done;
   SPI_TypeDef *spi;
   GPIO_Pin_t RST, BL, DC, CS, MOSI, SCLK;
   DMA_Channel_t dma;
} ST7789V2_cfg_t;

void ST7789V2_Init(ST7789V2_cfg_t* cfg);

void ST7789V2_Reset(ST7789V2_cfg_t* cfg);

void ST7789V2_Send_Command(ST7789V2_cfg_t* cfg, uint8_t command);

void ST7789V2_Send_Data(ST7789V2_cfg_t* cfg, uint8_t data);

void ST7789V2_Send_Data_Block(ST7789V2_cfg_t* cfg, uint8_t* data, uint32_t length);

void ST7789V2_Set_Address_Window(ST7789V2_cfg_t* cfg, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void ST7789V2_BL_On(ST7789V2_cfg_t* cfg);

void ST7789V2_BL_Off(ST7789V2_cfg_t* cfg);

void ST7789V2_Fill(ST7789V2_cfg_t* cfg, uint16_t* colour, uint32_t len);


void gpio_init(ST7789V2_cfg_t* cfg);
void spi_init(ST7789V2_cfg_t* cfg);
void dma_init(ST7789V2_cfg_t* cfg);
void spi_transmit_byte(ST7789V2_cfg_t* cfg, uint8_t data);
void spi_transmit_dma_8bit(ST7789V2_cfg_t* cfg, uint8_t* data, uint16_t len);
void spi_transmit_dma_16bit(ST7789V2_cfg_t* cfg, uint16_t* data, uint16_t len);
void spi_transmit_dma_16bit_noinc(ST7789V2_cfg_t* cfg, uint16_t* data, uint16_t len);

#endif