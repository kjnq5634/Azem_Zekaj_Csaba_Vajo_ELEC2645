#include "ST7789V2_Driver.h"

void delay_ms_approx(uint16_t ms) {
  // Crude ms delay function, use hal for more accurate timing functions
  for (int i = 0; i < 5714*ms; i++) {
    __asm("nop"); // do nothing in a way that stops the compiler optimising out the loop
  }
}

void gpio_write(GPIO_Pin_t gpio, uint8_t val) {
  gpio.port->BSRR = gpio.pin << (val ? GPIO_SET_LSB : GPIO_RESET_LSB);
}

void ST7789V2_Init(ST7789V2_cfg_t* cfg) {
  gpio_init(cfg);
  spi_init(cfg);
  dma_init(cfg);

  cfg->setup_done = 1;
  ST7789V2_Reset(cfg);

  ST7789V2_BL_On(cfg);

  ST7789V2_Send_Command(cfg, ST7789_SLPOUT);

  // Wait for sleep out to propagate
  delay_ms_approx(50);

  ST7789V2_Send_Command(cfg, ST7789_COLMOD);
  ST7789V2_Send_Data(cfg, 0x55);

  delay_ms_approx(10);

  ST7789V2_Send_Command(cfg, ST7789_MADCTL);
  ST7789V2_Send_Data(cfg, 0x00);

  ST7789V2_Send_Command(cfg, ST7789_INVON);
  delay_ms_approx(10);

  ST7789V2_Send_Command(cfg, ST7789_NORON);
  delay_ms_approx(10);

  ST7789V2_Set_Address_Window(cfg, 0, 20, 239, 299); 

  ST7789V2_Send_Command(cfg, 0x29);
  delay_ms_approx(10);
}

void ST7789V2_Reset(ST7789V2_cfg_t* cfg) {
  if (cfg->setup_done) {    
    // Set reset low and wait
    gpio_write(cfg->RST, 0);
    delay_ms_approx(50);

    // Set reset high
    gpio_write(cfg->RST, 1);

    // Software reset
    ST7789V2_Send_Command(cfg, ST7789_SWRESET);

    // Wait 120ms after resetting before sleep out
    delay_ms_approx(150);  
  }
}

void ST7789V2_Send_Command(ST7789V2_cfg_t* cfg, uint8_t command) {
  if (cfg->setup_done) {    
    // Deassert CS
    gpio_write(cfg->CS, 1);

    // Set DC 0
    gpio_write(cfg->DC, 0);

    // Send command
    spi_transmit_byte(cfg, command);
  }
}

void ST7789V2_Send_Data(ST7789V2_cfg_t* cfg, uint8_t data) {
  if (cfg->setup_done) {
    // Set DC 1
    gpio_write(cfg->DC, 1);

    // Send command
    spi_transmit_byte(cfg, data);
  }
}

void ST7789V2_Send_Data_Block(ST7789V2_cfg_t* cfg, uint8_t* data, uint32_t length) {
  if (cfg->setup_done) {
    // CS control is done in dma transmit function

    // Set DC 1
    gpio_write(cfg->DC, 1);

    // Wait for any previous transmissions to finish
    while(cfg->spi->SR & SPI_SR_BSY) {
      ;
    }

    // Send data
    spi_transmit_dma_8bit(cfg, data, length);
  }
}

void ST7789V2_Set_Address_Window(ST7789V2_cfg_t* cfg, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  while (cfg->spi->SR & SPI_SR_BSY);
  ST7789V2_Send_Command(cfg, ST7789_CASET);
  ST7789V2_Send_Data(cfg, x0 >> 8);
  ST7789V2_Send_Data(cfg, x0 & 0xFF);
  ST7789V2_Send_Data(cfg, x1 >> 8);
  ST7789V2_Send_Data(cfg, x1 & 0xFF);

  ST7789V2_Send_Command(cfg, ST7789_RASET);
  ST7789V2_Send_Data(cfg, y0 >> 8);
  ST7789V2_Send_Data(cfg, y0 & 0xFF);
  ST7789V2_Send_Data(cfg, y1 >> 8);
  ST7789V2_Send_Data(cfg, y1 & 0xFF);
}

void ST7789V2_Clear_RAM(ST7789V2_cfg_t* cfg);

void ST7789V2_BL_On(ST7789V2_cfg_t* cfg) {
  gpio_write(cfg->BL, 1);
}

void ST7789V2_BL_Off(ST7789V2_cfg_t* cfg) {
  gpio_write(cfg->BL, 0);
}

void ST7789V2_Fill(ST7789V2_cfg_t* cfg, uint16_t* colour, uint32_t len) {
  ST7789V2_Send_Command(cfg, ST7789_RAMWR);
  if (len & 0xFFFF0000) {
    spi_transmit_dma_16bit_noinc(cfg, colour, 65535);
    while (cfg->spi->SR & SPI_SR_BSY);
    spi_transmit_dma_16bit_noinc(cfg, colour, len - 65535);
  }
  else {
    spi_transmit_dma_16bit_noinc(cfg, colour, len);
  }
}

void gpio_init(ST7789V2_cfg_t* cfg) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; // GPIOB for SPI2 pins

  GPIOB->MODER = 0xB97FFFD7;
  GPIOB->OTYPER = 0x00000000;
  GPIOB->OSPEEDR = 0xCF000000;
  GPIOB->PUPDR = 0x00000010;
  GPIOB->AFR[1] = 0x50500000;
}

void spi_init(ST7789V2_cfg_t* cfg) {
  // Enable SPI clock
  RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;

  // Disable SPI
  cfg->spi->CR1 &= ~SPI_CR1_SPE;

  // Set CR1
  cfg->spi->CR1 = SPI_CR1_BIDIMODE |
                  SPI_CR1_BIDIOE   |
                  SPI_CR1_SSM      |
                  SPI_CR1_SSI      |
                  SPI_CR1_MSTR;
                  // | SPI_CR1_BR_1;

  // Set CR2
  cfg->spi->CR2 = SPI_CR2_FRXTH    |
                  SPI_CR2_DS_0     |
                  SPI_CR2_DS_1     |
                  SPI_CR2_DS_2     |
                  SPI_CR2_NSSP;

  // Enable SPI
  cfg->spi->CR1 |= SPI_CR1_SPE;
}

void dma_init(ST7789V2_cfg_t* cfg) {
  // Enable DMA1 clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  // Set DMA CCR
  cfg->dma.channel->CCR = DMA_CCR_PL_0 |
                          DMA_CCR_PL_1 |
                          DMA_CCR_MINC |
                          DMA_CCR_DIR;

  // Set DMA CSELR
  if (cfg->dma.channel == DMA1_Channel3) {
    DMA1_CSELR->CSELR |= 0x1 << DMA_CSELR_C3S_Pos;
  }
  else if (cfg->dma.channel == DMA1_Channel5) {
    DMA1_CSELR->CSELR |= 0x1 << DMA_CSELR_C5S_Pos;
  }
  else if (cfg->dma.channel == DMA2_Channel2) {
    DMA2_CSELR->CSELR |= 0x3 << DMA_CSELR_C2S_Pos;
  }
}

void spi_transmit_byte(ST7789V2_cfg_t* cfg, uint8_t data) {
  SPI_TypeDef* spi_inst = cfg->spi;

  // Wait for not busy
  while (spi_inst->SR & SPI_SR_BSY);
  
  // Check for 16 bit data or DMA enabled
  if (spi_inst->CR2 & (SPI_CR2_DS_3 | SPI_CR2_TXDMAEN)) {
    // Disable SPI and clear DS and DMA en
    spi_inst->CR1 &= ~SPI_CR1_SPE;
    spi_inst->CR2 &= ~(SPI_CR2_DS_Msk | SPI_CR2_TXDMAEN);

    // Set 8-bit mode
    spi_inst->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2;

    // Enable SPI
    spi_inst->CR1 |= SPI_CR1_SPE;
  }

  // Assert CS
  gpio_write(cfg->CS, 0);

  // Write data
  *((__IO uint8_t*)&spi_inst->DR) = data;

  // Wait for not busy
  while (spi_inst->SR & SPI_SR_BSY);

  // Deassert CS
  gpio_write(cfg->CS, 1);
}

void spi_transmit_dma_8bit(ST7789V2_cfg_t* cfg, uint8_t* data, uint16_t len) {  
  // Deassert CS
  gpio_write(cfg->CS, 1);
  
  // Set DC
  gpio_write(cfg->DC, 1);

  // Clear interrupts
  uint32_t isr = DMA1->ISR;
  DMA1->IFCR = isr;

  SPI_TypeDef* spi_inst = cfg->spi;

  // Disable SPI and clear data size
  spi_inst->CR1 &= ~SPI_CR1_SPE;  
  spi_inst->CR2 &= ~SPI_CR2_DS_Msk;

  // Set 8 bit data and enable DMA
  spi_inst->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; // 8-bit data 
  spi_inst->CR2 |= SPI_CR2_TXDMAEN;

  // Clear DMA CCR
  cfg->dma.channel->CCR = 0;

  // Peripheral = SPI2 data register
  cfg->dma.channel->CPAR = (uint32_t)&SPI2->DR; 
  // Memory = pixel buffer
  cfg->dma.channel->CMAR = (uint32_t)data; 
  // Size of data
  cfg->dma.channel->CNDTR = len; 

  // Set CCR register
  cfg->dma.channel->CCR = DMA_CCR_PL_0 |
                          DMA_CCR_PL_1 |
                          DMA_CCR_MINC |
                          DMA_CCR_DIR;
  
  // Enable SPI
  spi_inst->CR1 |= SPI_CR1_SPE;

  // Assert CS
  gpio_write(cfg->CS, 0);

  // Enable DMA channel (starts transfer)
  cfg->dma.channel->CCR |= DMA_CCR_EN;
}

void spi_transmit_dma_16bit(ST7789V2_cfg_t* cfg, uint16_t* data, uint16_t len) {
  // Deassert CS
  gpio_write(cfg->CS, 1);
  
  // Set DC
  gpio_write(cfg->DC, 1);

  // Clear interrupts
  uint32_t isr = DMA1->ISR;
  DMA1->IFCR = isr;

  SPI_TypeDef* spi_inst = cfg->spi;

  // Disable SPI and clear data size
  spi_inst->CR1 &= ~SPI_CR1_SPE;  
  spi_inst->CR2 &= ~SPI_CR2_DS_Msk;

  // Set 8 bit data and enable DMA
  spi_inst->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_DS_3; // 16-bit data 
  spi_inst->CR2 |= SPI_CR2_TXDMAEN;

  // Clear DMA CCR
  cfg->dma.channel->CCR = 0;

  // Peripheral = SPI2 data register
  cfg->dma.channel->CPAR = (uint32_t)&SPI2->DR; 
  // Memory = pixel buffer
  cfg->dma.channel->CMAR = (uint32_t)data; 
  // Size of data
  cfg->dma.channel->CNDTR = len; 

  // Set CCR register
  cfg->dma.channel->CCR = DMA_CCR_PL_0    |
                          DMA_CCR_PL_1    |
                          DMA_CCR_MSIZE_0 |
                          DMA_CCR_PSIZE_0 |
                          DMA_CCR_MINC    |
                          DMA_CCR_DIR;
  
  // Enable SPI
  spi_inst->CR1 |= SPI_CR1_SPE;

  // Assert CS
  gpio_write(cfg->CS, 0);

  // Enable DMA channel (starts transfer)
  cfg->dma.channel->CCR |= DMA_CCR_EN;
}

void spi_transmit_dma_16bit_noinc(ST7789V2_cfg_t* cfg, uint16_t* data, uint16_t len) {
  // Deassert CS
  gpio_write(cfg->CS, 1);
  
  // Set DC
  gpio_write(cfg->DC, 1);

  // Clear interrupts
  uint32_t isr = DMA1->ISR;
  DMA1->IFCR = isr;

  SPI_TypeDef* spi_inst = cfg->spi;

  // Disable SPI and clear data size
  spi_inst->CR1 &= ~SPI_CR1_SPE;  
  spi_inst->CR2 &= ~SPI_CR2_DS_Msk;

  // Set 8 bit data and enable DMA
  spi_inst->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_DS_3; // 16-bit data 
  spi_inst->CR2 |= SPI_CR2_TXDMAEN;

  // Clear DMA CCR
  cfg->dma.channel->CCR = 0;

  // Peripheral = SPI2 data register
  cfg->dma.channel->CPAR = (uint32_t)&SPI2->DR; 
  // Memory = pixel buffer
  cfg->dma.channel->CMAR = (uint32_t)data; 
  // Size of data
  cfg->dma.channel->CNDTR = len; 

  // Set CCR register
  cfg->dma.channel->CCR = DMA_CCR_PL_0    |
                          DMA_CCR_PL_1    |
                          DMA_CCR_MSIZE_0 |
                          DMA_CCR_PSIZE_0 |
                          DMA_CCR_DIR;
  
  // Enable SPI
  spi_inst->CR1 |= SPI_CR1_SPE;

  // Assert CS
  gpio_write(cfg->CS, 0);

  // Enable DMA channel (starts transfer)
  cfg->dma.channel->CCR |= DMA_CCR_EN;
}