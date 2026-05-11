# ST7789V2_Driver_STM32L4

Driver for the ST7789V2 LCD Screen for the Nucleo L476RG, for the ELEC2645 module. 

Intended for use with the Pimoroni 1.54" SPI Colour Square LCD (240x240) Breakout PIM576
https://shop.pimoroni.com/products/1-54-spi-colour-square-lcd-240x240-breakout?variant=39351811702867 

Also tested with https://wiki.seeedstudio.com/1-69inch_lcd_spi_display/ 

In the ELEC2645 labs the project setup has already been done, so this setup information is only required if you want to create your own projects. 


## Requirements
 - stm32l476xx.h and all sub-dependencies

## Setup
If you have used STM32CubeMX to generate a CMake project, create a git repo for that project, and add this repo to the root directory using:
```
git submodule add https://github.com/AeronJJ/ST7789V2_Driver_STM32L4
```
Then add these lines in the following sections in ./CMakeLists.txt (Or use the project configuration in STM32Cube IDE software if so desired):

target_link_directories:
```
    ST7789V2_Driver_STM32L4/Core/Inc/Src/LCD.c
    ST7789V2_Driver_STM32L4/Core/Inc/Src/ST7789V2_Driver.c
```
target_sources:
```
    ${CMAKE_SOURCE_DIR}/ST7789V2_Driver_STM32L4/Core/Src/LCD.c
    ${CMAKE_SOURCE_DIR}/ST7789V2_Driver_STM32L4/Core/Src/ST7789V2_Driver.c
```
target_include_directories:
```
    ${CMAKE_SOURCE_DIR}/ST7789V2_Driver_STM32L4/Core/Inc/
```

You should then be good to go.

## Usage

Theoretically, this library supports any of the three SPI peripherals on the MCU potentially allowing multiple displays to be driven, however, the only one that has been tested is SPI2. Here is a table showing how the LCD should be connected for use of SPI2, and other recommended pins:

| LCD | MCU |
|-----|-----|
| VCC | VDD |
| GND | GND |
| DIN | B15 |
| CLK | B13 |
| CS  | B12 |
| DC  | B11 |
| RST | B2  |
| BL  | B1  |

To use the LCD library, a config struct must first be initialised, an example for the above connections looks like this:
```
  ST7789V2_cfg_t cfg0 = {
    .setup_done = 0,
    .spi = SPI2,
    .RST = {.port = GPIOB, .pin = GPIO_PIN_2},
    .BL = {.port = GPIOB, .pin = GPIO_PIN_1},
    .DC = {.port = GPIOB, .pin = GPIO_PIN_11},
    .CS = {.port = GPIOB, .pin = GPIO_PIN_12},
    .MOSI = {.port = GPIOB, .pin = GPIO_PIN_15},
    .SCLK = {.port = GPIOB, .pin = GPIO_PIN_13},
    .dma = {.instance = DMA1, .channel = DMA1_Channel5}
  };
```

This struct is then passed around whenever a LCD function is called. For example, to call the initialisation function, you use:
```
  LCD_init(&cfg0);
```

Of course, the LCD must be initialised before use, so it it recommended to call the init function immediately after creating the struct.

This library utilises a compact frame buffer that stores image data at 4 bits per pixel for a total of 16 colours. These colours can be changed by modifying the `#define LCD_COLOUR_n RGB565_c` lines in LCD.h with your desired colour palette. Functions that modify pixel data, such as `LCD_Set_Pixel()` or `LCD_Draw_Circle()`, write directly to the frame buffer, rather than to the LCD. To push these changes onto the LCD, you must call the `LCD_Refresh()` with the config struct of the desired LCD, such as `LCD_Refresh(&cfg0)`.

## Optimisations
There are a number of optimisations that have been utilised in order to achieve a reasonable refresh rate on the LCD. First of all is the use of DMA to transfer data over SPI, this allows the CPU to continue running the game while the LCD is being updated.

The next is the compactisation of the frame buffer. The LCD is expecting each pixel to be 16 bits, this would require a frame buffer of 134,400 bytes, which would require more RAM than exists on the STM32L4 MCU. By using 4 bits per pixel, we can reduce the memory size to 33,600 bytes, which is much more reasonable. However, an extra processing step is required in order to convert the 4 bits back to 16 for the LCD. This also means we can't just DMA the whole frame buffer over to SPI, as the memory will not be converted (Note that this is a perfect example usecase for the PIO present on the Raspberry Pi Pico series microcontrollers, which can offload this extra processing step whilst still utilising DMA). To solve this problem, we can convert and transfer the frame buffer to the LCD one row at a time. When the `LCD_Refresh()` function is called, a preallocated section of memory equal to one row of pixels is written to with pixel values from the current row of the frame buffer after conversion. This memory block is then transferred to the LCD using DMA. This method, despite using DMA, still has to wait until the row has finished transferring to avoid overwriting pixels with the next row of data. We can optimise this by introducing a second row buffer that is written to while the other row buffer is being transferred. Once the first row has finished transferring, the DMA process for the next row can be started immediately. This means that there shouldn't be any point that the CPU is waiting around for a transfer to finish.

The final major optimisation applied is to track which rows of the frame buffer have been changed since the last refresh, and only write the rows which have changed to the LCD. To properly utilise this optimisation will require the User to create their program in a way that minimises writes to every row in the display, such as avoiding frequent use of the `LCD_fill()`. This can be unavoidable, but will likely cause major slowdowns to your application if used unwisely.