/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main.c - main routines of wclock24h
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 *  System Clocks configured on STM32F103 BluePill board:
 *
 *    External 8MHz crystal
 *    SYSCLK:               72MHz ((HSE_VALUE / HSE_Prediv) * PLL_Mul) = ((8MHz / 1) * 9)  = 72MHz
 *    AHB clock:            72MHz (AHB  Prescaler = 1)
 *    APB1 clock:           36MHz (APB1 Prescaler = 2)
 *    APB2 clock:           72MHz (APB2 Prescaler = 1)
 *    Timer clock:          72MHz (APB1 Prescaler = 2, Timer Multiplier 2)
 *
 *  System Clocks configured on STM32F401 Nucleo Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           8MHz, external 8MHz crystal
 *      PLL M               4
 *      PLL N               84
 *      PLL P               2
 *      AHB prescaler       1
 *      APB1 prescaler      2
 *      APB2 prescaler      1
 *
 *      Main clock:         84MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 4) * 84) / 2 = 84MHz
 *      AHB clock:          84MHz (AHB  Prescaler = 1)
 *      APB1 clock:         42MHz (APB1 Prescaler = 2)
 *      APB2 clock:         84MHz (APB2 Prescaler = 1)
 *      Timer clock:        84MHz
 *
 *  System Clocks configured on STM32F401 BlackPill Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           25MHz, external 25MHz crystal
 *      PLL M               25
 *      PLL N               336
 *      PLL P               4
 *      AHB prescaler       1
 *      APB1 prescaler      2
 *      APB2 prescaler      1
 *
 *      Main clock:         84MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((25MHz / 25) * 336) / 4 = 84MHz
 *      AHB clock:          84MHz (AHB  Prescaler = 1)
 *      APB1 clock:         42MHz (APB1 Prescaler = 2)
 *      APB2 clock:         84MHz (APB2 Prescaler = 1)
 *      Timer clock:        84MHz
 *
 *  System Clocks configured on STM32F411 Nucleo Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           8MHz, external 8MHz crystal
 *      PLL M               4
 *      PLL N               100
 *      PLL P               2
 *      AHB prescaler       1
 *      APB1 prescaler      2
 *      APB2 prescaler      1
 *
 *      Main clock:         100MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 4) * 100) / 2 = 100MHz
 *      AHB clock:          100MHz (AHB  Prescaler = 1)
 *      APB1 clock:          50MHz (APB1 Prescaler = 2)
 *      APB2 clock:         100MHz (APB2 Prescaler = 1)
 *      Timer clock:        100MHz
 *
 *  System Clocks configured on STM32F411 BlackPill Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           25MHz, external 25MHz crystal
 *      PLL M               25
 *      PLL N               200
 *      PLL P               2
 *      AHB prescaler       1
 *      APB1 prescaler      2
 *      APB2 prescaler      1
 *
 *      Main clock:         100MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 4) * 100) / 2 = 100MHz
 *      AHB clock:          100MHz (AHB  Prescaler = 1)
 *      APB1 clock:          50MHz (APB1 Prescaler = 2)
 *      APB2 clock:         100MHz (APB2 Prescaler = 1)
 *      Timer clock:        100MHz
 *
 *  System Clocks configured on STM32F446 Nucleo Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           8MHz, external 8MHz crystal
 *      PLL M               4
 *      PLL N               180
 *      PLL P               2
 *      AHB prescaler       1
 *      APB1 prescaler      4
 *      APB2 prescaler      2
 *
 *      Main clock:         180 MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 4) * 180) / 2 = 180MHz
 *      AHB clock:          180 MHz (AHB  Prescaler = 1)
 *      APB1 clock:          45 MHz (APB1 Prescaler = 4)
 *      APB2 clock:          90 MHz (APB2 Prescaler = 2)
 *      Timer clock:        180 MHz
 *
 *  System Clocks configured on STM32F407VE Black Board Board (see changes in system_stm32f4xx.c):
 *
 *      HSE_VALUE           8MHz, external 8MHz crystal
 *      PLL M               4
 *      PLL N               168
 *      PLL P               2
 *      AHB prescaler       1
 *      APB1 prescaler      4
 *      APB2 prescaler      2
 *
 *      Main clock:         168 MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 4) * 168) / 2 = 168MHz
 *      AHB clock:          168 MHz (AHB  Prescaler = 1)
 *      APB1 clock:          42 MHz (APB1 Prescaler = 4)
 *      APB2 clock:          84 MHz (APB2 Prescaler = 2)
 *      Timer clock:        168 MHz
 *
 *
 *  On STM32F4xx Nucleo Board, make sure that
 *
 *      - SB54 and SB55 are OFF
 *      - SB16 and SB50 are OFF
 *      - R35 and R37 are soldered (0R or simple wire)
 *      - C33 and C34 are soldered with 22pF capacitors
 *      - X3 is soldered with 8MHz crystal
 *
 * Devices Nucleo, BlackPill, and BluePill:
 *
 *    +-----------------------+----------------------------------+----------------------------------+----------------------------------+-------------------------------+
 *    | Device                | STM32F4x1 Nucleo                 | STM32F4x1 BlackPill              | STM32F103C8T6 BluePill           | Remarks                       |
 *    +-----------------------+----------------------------------+----------------------------------+----------------------------------+-------------------------------+
 *    | User button           | GPIO:     PC13 (on board)        | GPIO:     PA0 (on board)         | GPIO:     PA6                    | on board                      |
 *    | Board LED             | GPIO:     PA5                    | GPIO:     PC13                   | GPIO:     PC13                   | on board                      |
 *    +-----------------------+----------------------------------+----------------------------------+----------------------------------+-------------------------------+
 *    | TSOP31238 (IRMP)      | GPIO      PC10                   | GPIO      PB4                    | GPIO      PB3                    |                               |
 *    | DS18xx (OneWire)      | GPIO      PD2                    | GPIO      PB5                    | GPIO      PB5                    |                               |
 *    | Logger (Nucleo: USB)  | USART2    TX=PA2  RX=PA3         | USART1    TX=PA9  RX=PA10        | USART1    TX=PA9  RX=PA10        |                               |
 *    | WPS button            | GPIO      PC5                    | GPIO      PA7                    | GPIO      PA7                    |                               |
 *    | ESP8266 USART         | USART6    TX=PA11 RX=PA12        | USART2    TX=PA2  RX=PA3         | USART2    TX=PA2  RX=PA3         |                               |
 *    | ESP8266 RST/CH_PD     | GPIO      RST=PA7 CH_PD=PA6      | GPIO      RST=PB10 CH_PD=PB3     | GPIO      RST=PA0 CH_PD=PA1      |                               |
 *    | ESP8266 GPIO0         | GPIO      FLASH=PA4              | GPIO      FLASH=PA4              | GPIO      FLASH=PA4              |                               |
 *    | ESP8266 GPIO13/GPIO15 | USART1    GPIO13=PA9 GPIO15=PA10 | USART1    GPIO13=PA9 GPIO15=PA10 | USART1    GPIO13=PA9 GPIO15=PA10 | only in STM32 bootloader mode |
 *    | ESP8266 GPIO14        | GPIO      GPIO14=RESET           | GPIO      GPIO14=RESET           | GPIO      GPIO14=RESET           |                               |
 *    | ESP8266 GPIO4         | GPIO      GPIO4=BOOT0            | GPIO      GPIO4=BOOT0            | GPIO      GPIO4=BOOT0            |                               |
 *    | DCF77                 | GPIO      DATA=PC11 PON=PC12     | GPIO      DATA=PB8  PON=PB9      | GPIO      DATA=PB8  PON=PB9      |                               |
 *    | I2C DS3231 & EEPROM   | I2C3      SCL=PA8 SDA=PC9        | I2C1      SCL=PB6 SDA=PB7        | I2C1      SCL=PB6 SDA=PB7        |                               |
 *    | LDR                   | ADC       ADC1_IN14=PC4          | ADC       ADC1_IN5=PA5           | ADC       ADC1_IN5=PA5           |                               |
 *    | WS2812 / SK6812       | TIM3/DMA1 PC6                    | TIM3/DMA1 PB1                    | TIM1/DMA1 PA8                    |                               |
 *    | APA102                | SPI2/DMA1 SCK=PB13 MOSI=PB15     | SPI2/DMA1 SCK=PB13 MOSI=PB15     | SPI2/DMA1 SCK=PB13 MOSI=PB15     |                               |
 *    | Power switch          | GPIO      PC8                    | GPIO      PB0                    | GPIO      PB0                    |                               |
 *    | DFPlayer              | USART1    TX=PB6  RX=PB7         | USART6    TX=PA11  RX=PA12       | USART3    TX=PB10  RX=PB11       |                               |
 *    +-----------------------+----------------------------------+----------------------------------+----------------------------------+-------------------------------+
 *    | Free Port A           | PA0 PA1 PA15                     | PA1  PA6  PA8  PA15              | PA11 PA12 PA15                   |                               |
 *    | Free Port B           | PB0 PB1 PB2 PB4  PB5 PB12 PB14   | PB2 (BOOT0!) PB12 PB14           | PB1  PB4  PB12 PB14              |                               |
 *    | Free Port C           | PC3 PC7 PC14 PC15                | PC14 PC15                        | PC14 PC15                        |                               |
 *    +-----------------------+----------------------------------+----------------------------------+----------------------------------+-------------------------------+
 *
 *
 *  Devices STM32F407VE BlackBoard with TFT - sorted by function:
 *
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | Device                | STM32F407VE BlackBoard                                                    | Remarks                       |
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | User button   "K0"    | GPIO           PE4                                                        | on board                      |
 *    | Board LED     D2      | GPIO           PA6                                                        | on board                      |
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | TSOP31238     (IRMP)  | GPIO           PC1                                                        |                               |
 *    | DS18xx                | GPIO           PD3                                                        |                               |
 *    | Logger        USART   | USART2 ALT0    TX=PA2  RX=PA3                                             |                               |
 *    | WPS button    "K1"    | GPIO           Button "K1" PE3                                            |                               |
 *    | ESP8266       RX      | USART3 ALT0    TX=PB10                                                    |                               |
 *    | ESP8266       TX      | USART3 ALT0    RX=PB11                                                    |                               |
 *    | ESP8266       RST     | GPIO           RST=PA4                                                    |                               |
 *    | ESP8266       CH_PD   | GPIO           CH_PD=PA5                                                  |                               |
 *    | ESP8266       GPIO0   | FLASH          PA8                                                        |                               |
 *    | ESP8266       GPIO13  | USART1         GPIO13=PA9                                                 | only in STM32 bootloader mode |
 *    | ESP8266       GPIO15  | USART1         GPIO15=PA10                                                | only in STM32 bootloader mode |
 *    | ESP8266       GPIO14  | RESET          GPIO14=RESET                                               |                               |
 *    | ESP8266       GPIO4   | BOOT0          GPIO4=BOOT0                                                |                               |
 *    | Flash W25Q16          | F_CS           PB0                                                        | fix on BlackBoard             |
 *    | Flash W25Q16          | SPI1 CLK       PB3                                                        | fix on BlackBoard             |
 *    | Flash W25Q16          | SPI1 MISO      PB4                                                        | fix on BlackBoard             |
 *    | Flash W25Q16          | SPI1 MOSI      PB5                                                        | fix on BlackBoard             |
 *    | DCF77                 | GPIO           DATA=PC2 PON=PC3                                           |                               |
 *    | I2C DS3231 & EEPROM   | I2C1:          SCL=PB8 SDA=PB9 (here: EEPROM not used, but FLASH on SPI1) |                               |
 *    | LDR                   | ADC            ADC1_IN14=PC4                                              |                               |
 *    | WS2812 / SK6812       | TIM3/DMA1      PC6                                                        |                               |
 *    | APA102                |                Not supported here                                         | Nucleo: SCK=PB13 MOSI=PB15    |
 *    | Power switch          | GPIO           PC0                                                        |                               |
 *    | DFPlayer      RX      | USART1 (ALT1): TX=PB6                                                     |                               |
 *    | DFPlayer      TX      | USART1 (ALT1): RX=PB7                                                     |                               |
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | FSMC          TFT     |                                                                           |                               |
 *    | FSMC RST      RS      | FSMC RST       STM32 RESET                                                | fix on BlackBoard             |
 *    | FSMC D15      DB15    | FSMC D15       PD10                                                       | fix on BlackBoard             |
 *    | FSMC D14      DB14    | FSMC D14       PD9                                                        | fix on BlackBoard             |
 *    | FSMC D13      DB13    | FSMC D13       PD8                                                        | fix on BlackBoard             |
 *    | FSMC D12      DB12    | FSMC D12       PE15                                                       | fix on BlackBoard             |
 *    | FSMC D11      DB11    | FSMC D11       PE14                                                       | fix on BlackBoard             |
 *    | FSMC D10      DB10    | FSMC D10       PE13                                                       | fix on BlackBoard             |
 *    | FSMC D9       DB9     | FSMC D9        PE12                                                       | fix on BlackBoard             |
 *    | FSMC D8       DB8     | FSMC D8        PE11                                                       | fix on BlackBoard             |
 *    | FSMC D7       DB7     | FSMC D7        PE10                                                       | fix on BlackBoard             |
 *    | FSMC D6       DB6     | FSMC D6        PE9                                                        | fix on BlackBoard             |
 *    | FSMC D5       DB5     | FSMC D5        PE8                                                        | fix on BlackBoard             |
 *    | FSMC D4       DB4     | FSMC D4        PE7                                                        | fix on BlackBoard             |
 *    | FSMC D3       DB3     | FSMC D3        PD1                                                        | fix on BlackBoard             |
 *    | FSMC D2       DB2     | FSMC D2        PD0                                                        | fix on BlackBoard             |
 *    | FSMC D1       DB1     | FSMC D1        PD15                                                       | fix on BlackBoard             |
 *    | FSMC D0       DB0     | FSMC D0        PD14                                                       | fix on BlackBoard             |
 *    | FSMC NOE      RD      | FSMC NOE       PD4                                                        | fix on BlackBoard             |
 *    | FSMC NWE      WR      | FSMC NWE       PD5                                                        | fix on BlackBoard             |
 *    | FSMC A18      RS      | FSMC A18       PD13                                                       | fix on BlackBoard             |
 *    | FSMC NE1      CS      | FSMC NE1       PD7                                                        | fix on BlackBoard             |
 *    | LCD BL                | LCD            PB1 (not used)                                             | fix on BlackBoard             |
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | TOUCH         TFT     |                                                                           |                               |
 *    | TOUCH         T_CS    | T_CS           PB12                                                       | fix on BlackBoard             |
 *    | TOUCH         T_SCK   | T_SCK          PB13                                                       | fix on BlackBoard             |
 *    | TOUCH         T_MISO  | T_MISO         PB14                                                       | fix on BlackBoard             |
 *    | TOUCH         T_MOSI  | T_MOSI         PB15                                                       | fix on BlackBoard             |
 *    | TOUCH         T_PEN   | T_PEN          PC5                                                        | fix on BlackBoard             |
 *    +-----------------------+---------------------------------------------------------------------------+-------------------------------+
 *
 * List of STM32F407VE BlackBoard pins - sorted by Port/Pin:
 *
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | Reserved                | WK_UP button   PA0                                                        | fix on BlackBoard             |
 *    | Free                    |                PA1                                                        |                               |
 *    | Logger          RX      | USART2 TX      PA2                                                        |                               |
 *    | Logger          TX      | USART2 RX      PA3                                                        |                               |
 *    | ESP8266         RST     | GPIO           PA4                                                        |                               |
 *    | ESP8266         CH_PD   | GPIO           PA5                                                        |                               |
 *    | Board LED       D2      | GPIO           PA6                                                        |                               |
 *    | Board LED       D3      | Board LED D3   PA7 (not used)                                             | fix on BlackBoard             |
 *    | ESP8266         GPIO0   | FLASH          PA8                                                        |                               |
 *    | ESP8266         GPIO13  | USART1 TX      PA9                                                        | only in STM32 bootloader mode |
 *    | ESP8266         GPIO15  | USART1 RX      PA10                                                       | only in STM32 bootloader mode |
 *    | Reserved        USB DM  | USB DM -       PA11                                                       | fix on BlackBoard             |
 *    | Reserved        USB DP  | USB DP +       PA12                                                       | fix on BlackBoard             |
 *    | Reserved        JTAG    | JTAG TMS       PA13                                                       | fix on BlackBoard             |
 *    | Reserved        JTAG    | JTAG TCK       PA14                                                       | fix on BlackBoard             |
 *    | Reserved        JTAG    | JTAG TDI       PA15                                                       | fix on BlackBoard             |
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | Flash W25Q16            | F_CS           PB0                                                        | fix on BlackBoard             |
 *    | LCD             BL      | LCD BL         PB1 (not used)                                             | fix on BlackBoard             |
 *    | Reserved                | BOOT1          PB2                                                        | fix on BlackBoard             |
 *    | Flash W25Q16            | SPI1 CLK       PB3                                                        | fix on BlackBoard             |
 *    | Flash W25Q16            | SPI1 MISO      PB4                                                        | fix on BlackBoard             |
 *    | Flash W25Q16            | SPI1 MOSI      PB5                                                        | fix on BlackBoard             |
 *    | DFPlayer TX             | USART1 TX      PB6                                                        |                               |
 *    | DFPlayer RX             | USART1 RX      PB7                                                        |                               |
 *    | I2C DS3231 & EEPROM     | I2C1 SCL       PB8 (here: EEPROM not used, but FLASH on SPI1)             |                               |
 *    | I2C DS3231 & EEPROM     | I2C1 SDA       PB9 (here: EEPROM not used, but FLASH on SPI1)             |                               |
 *    | ESP8266 USART   TX      | USART3 TX      PB10                                                       |                               |
 *    | ESP8266 USART   RX      | USART3 RX      PB11                                                       |                               |
 *    | TOUCH           T_CS    | T_CS           PB12                                                       | touch (yet not used)          |
 *    | TOUCH           T_SCK   | T_SCK          PB13                                                       | touch (yet not used)          |
 *    | TOUCH           T_MISO  | T_MISO         PB14                                                       | touch (yet not used)          |
 *    | TOUCH           T_MOSI  | T_MOSI         PB15                                                       | touch (yet not used)          |
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | LED strip power switch  | GPIO           PC0                                                        |                               |
 *    | TSOP31238 (IRMP)        | GPIO           PC1                                                        |                               |
 *    | DCF77 Data              | GPIO           PC2                                                        |                               |
 *    | DCF77 PON               | GPIO           PC3                                                        |                               |
 *    | LDR                     | ADC_IN14       PC4                                                        |                               |
 *    | TOUCHH          T_PEN   | T_PEN          PC5                                                        | fix on BlackBoard             |
 *    | WS2812 / SK6812         | TIM3/DMA1      PC6                                                        |                               |
 *    | Free                    |                PC7                                                        |                               |
 *    | Reserved                | SDIO D0        PC8                                                        | fix on BlackBoard             |
 *    | Reserved                | SDIO D1        PC9                                                        | fix on BlackBoard             |
 *    | Reserved                | SDIO D2        PC10                                                       | fix on BlackBoard             |
 *    | Reserved                | SDIO D3        PC11                                                       | fix on BlackBoard             |
 *    | Reserved                | SDIO SCK       PC12                                                       | fix on BlackBoard             |
 *    | Free                    |                PC13                                                       |                               |
 *    | Reserved                | XTAL           PC14                                                       | fix on BlackBoard             |
 *    | Reserved                | XTAL           PC15                                                       | fix on BlackBoard             |
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | FSMC D2         DB2     | FSMC D2        PD0                                                        | fix on BlackBoard             |
 *    | FSMC D3         DB3     | FSMC D3        PD1                                                        | fix on BlackBoard             |
 *    | Reserved                | SDIO CMD       PD2                                                        | fix on BlackBoard             |
 *    | DS18xx (OneWire)        | GPIO           PD3                                                        |                               |
 *    | FSMC NOE        RD      | FSMC NOE       PD4                                                        | fix on BlackBoard             |
 *    | FSMC NWE        WR      | FSMC NWE       PD5                                                        | fix on BlackBoard             |
 *    | FSMC NWAIT              | FSMC NWAIT     PD6                                                        | fix on BlackBoard, not used   |
 *    | FSMC NE1        CS      | FSMC NE1       PD7                                                        | fix on BlackBoard             |
 *    | FSMC D13        DB13    | FSMC D13       PD8                                                        | fix on BlackBoard             |
 *    | FSMC D14        DB14    | FSMC D14       PD9                                                        | fix on BlackBoard             |
 *    | FSMC D15        DB15    | FSMC D15       PD10                                                       | fix on BlackBoard             |
 *    | Free                    |                PD11                                                       |                               |
 *    | Free                    |                PD12                                                       |                               |
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *    | Free                    |                PE0                                                        |                               |
 *    | Free                    |                PE1                                                        |                               |
 *    | Free                    |                PE2                                                        |                               |
 *    | WPS button      "K1"    | GPIO           PE3                                                        |                               |
 *    | User button     "K0"    | GPIO           PE4                                                        |                               |
 *    | Free                    |                PE5                                                        |                               |
 *    | Free                    |                PE6                                                        | fix on BlackBoard             |
 *    | FSMC D4         DB4     | FSMC D4        PE7                                                        | fix on BlackBoard             |
 *    | FSMC D5         DB5     | FSMC D5        PE8                                                        | fix on BlackBoard             |
 *    | FSMC D6         DB6     | FSMC D6        PE9                                                        | fix on BlackBoard             |
 *    | FSMC D7         DB7     | FSMC D7        PE10                                                       | fix on BlackBoard             |
 *    | FSMC D8         DB8     | FSMC D8        PE11                                                       | fix on BlackBoard             |
 *    | FSMC D9         DB9     | FSMC D9        PE12                                                       | fix on BlackBoard             |
 *    | FSMC D10        DB10    | FSMC D10       PE13                                                       | fix on BlackBoard             |
 *    | FSMC D11        DB11    | FSMC D11       PE14                                                       | fix on BlackBoard             |
 *    | FSMC D12        DB12    | FSMC D12       PE15                                                       | fix on BlackBoard             |
 *    +-------------------------+---------------------------------------------------------------------------+-------------------------------+
 *
 * Timers:
 *
 *    +-------------------------+---------------------------------------+-----------------------------------+-------------------------------+
 *    | Device                  | STM32F4x1 Nucleo                      | STM32F103C8T6                     | Remarks                       |
 *    +-------------------------+---------------------------------------+-----------------------------------+-------------------------------+
 *    | General (IRMP etc.)     | TIM2                                  | TIM2                              |                               |
 *    | WS2812                  | TIM3                                  | TIM1                              |                               |
 *    | Beeper                  | TIM4                                  | TIM4                              |                               |
 *    | DS18xx (OneWire)        | Systick (see delay.c)                 | Systick (see delay.c)             |                               |
 *    +-------------------------+---------------------------------------+-----------------------------------+-------------------------------+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wclock24h-config.h"
#include "base.h"
#include "display.h"
#include "overlay.h"
#include "dcf77.h"
#include "timeserver.h"
#include "esp8266.h"
#include "esp-spiffs.h"
#include "board-led.h"
#include "power.h"
#include "irmp.h"
#include "remote-ir.h"
#include "button.h"
#include "wpsbutton.h"
#include "eep.h"
#include "eeprom-data.h"
#include "tempsensor.h"
#include "ds18xx.h"
#include "rtc.h"
#include "ldr.h"
#include "delay.h"
#include "night.h"
#include "alarm.h"
#include "weather.h"
#include "vars.h"
#include "dfplayer.h"
#include "tables.h"
#include "tetris.h"
#include "snake.h"
#include "log.h"
#include "ssd1963.h"
#include "touch.h"
#include "main.h"

#undef  UART_PREFIX                                                     // log.h hat UART_PREFIX zuletzt auf "log" gesetzt
#define UART_PREFIX                 esp8266                             // macht esp8266_uart_rxmax/-rxdrops/-rxore fuer die Diagnosezeile sichtbar
#include "uart.h"                                                       // absichtlich ohne Include-Guard, je Praefix neu einbindbar

#define DEFAULT_UPDATE_HOST         "uclock.de"
#define DEFAULT_UPDATE_PATH         "update"
#define DEFAULT_OBSERVE_SUMMERTIME  1

#define STATUS_LED_FLASH_TIME       50                                  // status LED: time of flash
#define MAX_DATE_TICKER_LEN         32                                  // date ticker overlay: buffer len

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * DIAGNOSE, siehe specs/beobachtbarkeit: Takt und Rueckfall-Schwelle der Diagnosezeile am Kopf des Hauptloops
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define DIAG_INTERVAL_SEC           10                                  // Takt der Diagnosezeile in Sekunden
#define DIAG_INTERVAL_TICKS         (DIAG_INTERVAL_SEC * F_INTERRUPTS)  // Makro: wird erst im Hauptloop expandiert, F_INTERRUPTS kommt aus irmpconfig.h
#define DIAG_LOOP_BUDGET_START      1000000UL                           // Rueckfall-Schwelle, solange noch keine regulaere Zeile kalibriert hat
#define DIAG_LOOP_BUDGET_MIN        1000UL                              // Untergrenze der selbstkalibrierten Schwelle. Seit A27 / L182 nur noch
                                                                        // ueber mehrere Fenster mit echtem Rueckgang erreichbar, nicht mehr in
                                                                        // einem Schritt nach einem Stillstand -- siehe Kalibrierung im Hauptloop

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global public variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
MAIN_GLOBALS                    gmain;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global private variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t             esp8266_is_online           = 0;
static uint32_t                 eep_version                 = 0xFFFFFFFF;
static char                     current_reset_cause[MAX_RESET_CAUSE_LEN + 1] = "";

#define WATCHDOG_TIMEOUT_MS      20000UL

static void                     watchdog_init (void);
void                            watchdog_reload (void);         // nicht static: display_test() bedient ihn selbst
static void                     fault_reset (const char *);
static void                     append_reset_cause (const char *);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log reset flags captured by RCC_CSR
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
append_reset_cause (const char * cause)
{
    size_t len;

    if (! cause || ! *cause)
    {
        return;
    }

    len = strlen (current_reset_cause);

    if (len > 0 && len < MAX_RESET_CAUSE_LEN - 2)
    {
        strncat (current_reset_cause, ", ", MAX_RESET_CAUSE_LEN - len);
        len = strlen (current_reset_cause);
    }

    if (len < MAX_RESET_CAUSE_LEN)
    {
        strncat (current_reset_cause, cause, MAX_RESET_CAUSE_LEN - len);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log reset flags captured by RCC_CSR
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
log_reset_flags (void)
{
    uint_fast8_t    have_flag = 0;

    current_reset_cause[0] = '\0';

    log_message ("Reset flags:");

    if (RCC_GetFlagStatus (RCC_FLAG_LPWRRST) != RESET)
    {
        log_message ("  LPWRRST");
        have_flag = 1;
        append_reset_cause ("Low-power reset");
    }
    if (RCC_GetFlagStatus (RCC_FLAG_WWDGRST) != RESET)
    {
        log_message ("  WWDGRST");
        have_flag = 1;
        append_reset_cause ("WWDG reset");
    }
    if (RCC_GetFlagStatus (RCC_FLAG_IWDGRST) != RESET)
    {
        log_message ("  IWDGRST");
        have_flag = 1;
        append_reset_cause ("Watchdog reset");
    }
    if (RCC_GetFlagStatus (RCC_FLAG_SFTRST) != RESET)
    {
        log_message ("  SFTRST");
        have_flag = 1;
        append_reset_cause ("Software reset");
    }
    if (RCC_GetFlagStatus (RCC_FLAG_PORRST) != RESET)
    {
        log_message ("  PORRST");
        have_flag = 1;
    }
    if (RCC_GetFlagStatus (RCC_FLAG_PINRST) != RESET)
    {
        log_message ("  PINRST");
        have_flag = 1;
    }
#ifdef RCC_FLAG_BORRST
    if (RCC_GetFlagStatus (RCC_FLAG_BORRST) != RESET)
    {
        log_message ("  BORRST");
        have_flag = 1;
        append_reset_cause ("Brownout reset");
    }
#endif

    if (! have_flag)
    {
        log_message ("  none");
    }

    RCC_ClearFlag ();
    log_flush ();
}

const char *
main_get_reset_cause (void)
{
    return current_reset_cause;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize independent watchdog with a moderate timeout to recover from hangs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
watchdog_init (void)
{
    uint32_t timeout = 1000000UL;

    /* Der IWDG laeuft am LSI. Die Flags PVU und RVU werden erst geloescht, wenn ein
     * LSI-Takt anliegt -- und der LSI startet von selbst erst mit IWDG_Enable(), also
     * NACH den Warteschleifen weiter unten. Ohne expliziten Start liefen sie deshalb
     * immer in den Timeout, die Funktion kehrte mit return zurueck und IWDG_Enable()
     * wurde nie erreicht. Am Geraet belegt: "IWDG init timeout, watchdog disabled" bei
     * jedem Start, und damit war der Watchdog wirkungslos -- ein Haenger am 30.09.2026
     * dauerte 18 Minuten, statt nach 20 s aufgeloest zu werden.
     */
    RCC_LSICmd (ENABLE);

    while (RCC_GetFlagStatus (RCC_FLAG_LSIRDY) == RESET && timeout > 0)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        log_message ("LSI startet nicht, watchdog disabled");
        log_flush ();
        return;
    }

    IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler (IWDG_Prescaler_256);

#if defined (STM32F103)
    IWDG_SetReload ((WATCHDOG_TIMEOUT_MS * 40UL) / 256UL);             // LSI approx. 40kHz on STM32F1
#else
    IWDG_SetReload ((WATCHDOG_TIMEOUT_MS * 32UL) / 256UL);             // LSI approx. 32kHz on STM32F4
#endif

    IWDG_ReloadCounter ();
    IWDG_Enable ();                                             // ab hier laeuft der Watchdog

    /* Erst JETZT auf PVU/RVU warten, nicht vorher. Der IWDG-Block bekommt seinen Takt
     * mit dem Enable; davor kann die Uebernahme der Werte gar nicht abgeschlossen
     * werden und die Schleife lief zwangslaeufig in den Timeout. Genau daran scheiterte
     * die Initialisierung bisher -- mit dem frueheren "return" blieb der Watchdog aus.
     *
     * IWDG_ReloadCounter() in der Schleife ist Pflicht: Bis die Werte uebernommen sind,
     * laeuft der Watchdog noch mit seinen Vorgabewerten von rund einer halben Sekunde.
     * Ohne Nachtriggern koennte er hier zuschlagen und das Geraet in eine Resetschleife
     * schicken.
     */
    timeout = 1000000UL;

    while ((IWDG_GetFlagStatus (IWDG_FLAG_PVU) != RESET || IWDG_GetFlagStatus (IWDG_FLAG_RVU) != RESET) && timeout > 0)
    {
        IWDG_ReloadCounter ();
        timeout--;
    }

    if (timeout == 0)
    {
        log_printf ("IWDG laeuft, Werte aber nicht bestaetigt (timeout=%lums angefordert)\r\n", WATCHDOG_TIMEOUT_MS);
    }
    else
    {
        log_printf ("IWDG enabled: timeout=%lums\r\n", WATCHDOG_TIMEOUT_MS);
    }

    log_flush ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * reload independent watchdog
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
watchdog_reload (void)
{
    IWDG_ReloadCounter ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log fault details and force a clean system reset instead of hanging forever
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
fault_reset (const char * reason)
{
    __disable_irq ();

    log_message ("");
    log_message ("fatal fault detected");
    log_message (reason);
    log_printf ("  CFSR=0x%08lX HFSR=0x%08lX ICSR=0x%08lX\r\n", SCB->CFSR, SCB->HFSR, SCB->ICSR);
    log_printf ("  MMFAR=0x%08lX BFAR=0x%08lX\r\n", SCB->MMFAR, SCB->BFAR);
    log_flush ();

    for (volatile uint32_t idx = 0; idx < 2000000UL; idx++)
    {
        ;
    }

    NVIC_SystemReset ();

    while (1)
    {
        ;
    }
}

void
HardFault_Handler (void)
{
    fault_reset ("HardFault");
}

void
MemManage_Handler (void)
{
    fault_reset ("MemManage");
}

void
BusFault_Handler (void)
{
    fault_reset ("BusFault");
}

void
UsageFault_Handler (void)
{
    fault_reset ("UsageFault");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global private variables, modified by timer ISR
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static volatile uint_fast8_t    animation_flag              = 0;        // flag: animate LEDs
static volatile uint_fast8_t    seconds_flag                = 0;        // flag: seconds changed
static volatile uint_fast8_t    ds3231_flag                 = 0;        // flag: read date/time from RTC DS3231
static volatile uint_fast8_t    net_time_flag               = 0;        // flag: read date/time from time server
static volatile uint_fast16_t   net_time_countdown          = 3800;     // counter: if it counts to 0, then net_time_flag will be triggered
static volatile uint_fast8_t    ldr_conversion_flag         = 0;        // flag: read LDR value
static volatile uint_fast8_t    measure_temperature_flag    = 0;        // flag: measure temperature from DS18XX
static volatile uint_fast8_t    read_temperature_flag       = 0;        // flag: read temperature from RTC & DS18XX
static volatile uint_fast8_t    read_rtc_temperature_flag   = 0;        // flag: read temperature from RTC
static volatile uint_fast8_t    show_time_flag              = 0;        // flag: update time on display, set every full minute
static volatile uint_fast8_t    half_minute_flag            = 0;        // flag: it is hh:mm:30
volatile uint32_t               uptime                      = 0;        // uptime in seconds
static volatile uint32_t        diag_tick_cnt               = 0;        // DIAGNOSE: zaehlt JEDEN Zeitgeber-Interrupt, nicht die Sekunden
#if 0
static volatile uint_fast8_t    wday                        = 0;        // current weekday, 0=Sunday
static volatile uint_fast16_t   year                        = 0;        // current year;
static volatile uint_fast8_t    month                       = 0;        // current month;
static volatile uint_fast8_t    mday                        = 0;        // current day of month 1..31;
static volatile uint_fast8_t    hour                        = 0;        // current hour
static volatile uint_fast8_t    minute                      = 0;        // current minute
static volatile uint_fast8_t    second                      = 0;        // current second
#endif
static volatile uint_fast8_t    dcf77_enabled               = 0;        // flag: DCF77 enabled (disabled if power is on)
static volatile uint32_t        ambilight_clock_wait_cycles = 0;        // number of wait cycles before changing ambilight LED in clock/clock2
static volatile uint32_t        ambilight_clock_wait_cnt    = 0;        // wait counter: counts from 0 to ambilight_clock_wait_cycles
static volatile uint32_t        ambilight_clock_led_idx     = 0;        // ambilight led index for clock/clock2
static volatile uint_fast8_t    ambilight_clock_tick        = 0;        // flag: call 20 times per ambilight LED in clock/clock2 mode

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timer definitions:
 *
 *      F_INTERRUPTS    = TIM_CLK / (TIM_PRESCALER + 1) / (TIM_PERIOD + 1)
 * <==> TIM_PRESCALER   = TIM_CLK / F_INTERRUPTS / (TIM_PERIOD + 1) - 1
 *
 * STM32F401:
 *      TIM_PERIOD      =   8 - 1 =   7
 *      TIM_PRESCALER   = 700 - 1 = 699
 *      F_INTERRUPTS    = 84000000 / 700 / 8 = 15000 (0.00% error)
 * STM32F411:
 *      TIM_PERIOD      = 101 - 1 = 100
 *      TIM_PRESCALER   =  66 - 1 =  65
 *      F_INTERRUPTS    = 100000000 / 66 / 101 = 15015 (0.01% error)
 * STM32F446:
 *      TIM_PERIOD      =    6 - 1 =   5
 *      TIM_PRESCALER   = 1000 - 1 = 999
 *      F_INTERRUPTS    = 90000000 / 1000 / 6 = 15000 (0.00% error)
 * STM32F407VE:
 *      TIM_PERIOD      =    8 - 1 =    7
 *      TIM_PRESCALER   =  700 - 1 =  699
 *      F_INTERRUPTS    = 84000000 / 700 / 8 = 15000 (0.00% error)
 * STM32F103:
 *      TIM_PERIOD      =   6 - 1 =   5
 *      TIM_PRESCALER   = 800 - 1 = 799
 *      F_INTERRUPTS    = 72000000 / 800 / 6 = 15000 (0.00% error)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if defined (NUCLEO_BOARD)
#  if defined (STM32F401RE)                                             // STM32F401 Nucleo Board @84MHz
#    define TIM_CLK                 84000000L                           // 84 MHz
#    define TIM_PERIOD              7
#  elif defined (STM32F411RE)                                           // STM32F411 Nucleo Board @100MHz
#    define TIM_CLK                 100000000L                          // 100 MHz
#    define TIM_PERIOD              100
#  elif defined (STM32F446RE)                                           // STM32F446 Nucleo Board @180MHz
#    define TIM_CLK                 90000000L                           // APB2 clock: 90 MHz
#    define TIM_PERIOD              5
#  else
#    error STM32 unknown
#  endif

#elif defined (BLACK_BOARD)
#  if defined (STM32F407VE)                                             // STM32F407VE Black Board @168MHz
#    define TIM_CLK                 84000000L                           // APB2 clock: 84 MHz
#    define TIM_PERIOD              7
#  else
#    error STM32 unknown
#  endif

#elif defined (BLACKPILL_BOARD)
#  if defined (STM32F401CC)                                             // STM32F401 BlackPill Board @84MHz
#    define TIM_CLK                 84000000L                           // 84 MHz
#    define TIM_PERIOD              7
#  elif defined (STM32F411CE)                                           // STM32F411 BlackPill Board @100MHz
#    define TIM_CLK                 100000000L                          // 100 MHz
#    define TIM_PERIOD              100
#  else
#    error STM32 unknown
#  endif

#elif defined (BLUEPILL_BOARD)
#  if defined (STM32F103)                                               // STM32F103 BLuePill Board @72MHz
#    define TIM_CLK                 72000000L                           // APB2 clock: 72MHz
#    define TIM_PERIOD              5
#  else
#    error STM32 unknown
#  endif

#else
#error STM32 unknown
#endif

#define TIM_PRESCALER               ((TIM_CLK / F_INTERRUPTS) / (TIM_PERIOD + 1) - 1)

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize timer2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
timer2_init (void)
{
    TIM_TimeBaseInitTypeDef     tim;
    NVIC_InitTypeDef            nvic;

    TIM_TimeBaseStructInit (&tim);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);

    tim.TIM_ClockDivision   = TIM_CKD_DIV1;
    tim.TIM_CounterMode     = TIM_CounterMode_Up;
    tim.TIM_Period          = TIM_PERIOD;
    tim.TIM_Prescaler       = TIM_PRESCALER;
    TIM_TimeBaseInit (TIM2, &tim);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    nvic.NVIC_IRQChannel                    = TIM2_IRQn;
    nvic.NVIC_IRQChannelCmd                 = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority  = 0x0F;
    nvic.NVIC_IRQChannelSubPriority         = 0x0F;
    NVIC_Init (&nvic);

    TIM_Cmd(TIM2, ENABLE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set number of wait cycles for ambilight modes clock & clock2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
main_set_ambilight_clock_wait_cycles (void)
{
    if (display.ambilight_leds)
    {
        ambilight_clock_wait_cycles = (60 * F_INTERRUPTS) / ((AMBILIGHT_CLOCK_TICK_COUNT_PER_LED * display.ambilight_leds));
        ambilight_clock_led_idx     = (gmain.second * display.ambilight_leds) / 60;
    }
    else
    {
        ambilight_clock_wait_cycles = 0;
        ambilight_clock_led_idx     = 0;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timer2 IRQ handler for IRMP, soft clock, dcf77, animations, several timeouts
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
extern void TIM2_IRQHandler (void);                                     // keep compiler happy

void
TIM2_IRQHandler (void)
{
    static uint_fast8_t     last_minute_of_ds3231_flag = 0xFF;
    static uint_fast16_t    ldr_cnt;
    static uint_fast16_t    animation_cnt;
    static uint_fast16_t    clk_cnt;
    static uint_fast16_t    dcf77_cnt;
    static uint_fast16_t    net_time_cnt;
    static uint_fast8_t     ambi_tick_cnt = 0;
#ifndef BLACK_BOARD                                                     // eeprom not used on STM32F407VE
    static uint_fast16_t    eeprom_cnt;
#endif

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    diag_tick_cnt++;                                                    // DIAGNOSE: nur Inkrement, kein Vergleich, kein Aufruf, kein Log

    (void) irmp_ISR ();                                                 // call irmp ISR

    if (uptime > 1)
    {
        ldr_cnt++;

        if (ldr_cnt == F_INTERRUPTS / 4)                                // start LDR conversion every 1/4 second
        {
            ldr_conversion_flag = 1;
            ldr_cnt = 0;
        }
    }

    animation_cnt++;

    if (animation_cnt == F_INTERRUPTS / 64)                             // set animation_flag every 1/64 of a second
    {
        animation_flag = 1;
        animation_cnt = 0;
    }

    if (dcf77_enabled)                                                  // disabled if power is on
    {
        dcf77_cnt++;

        if (dcf77_cnt == F_INTERRUPTS / 100)                            // call dcf77_tick every 1/100 of a second
        {
            dcf77_cnt = 0;
            dcf77_tick ();
        }
    }

    net_time_cnt++;

    if (net_time_cnt == F_INTERRUPTS / 100)                             // set esp8266_ten_ms_tick every 1/100 of a second
    {
        esp8266_ten_ms_tick = 1;
        net_time_cnt = 0;
    }

#ifndef BLACK_BOARD                                                     // eeprom not used on STM32F407VE
    eeprom_cnt++;

    if (eeprom_cnt == F_INTERRUPTS / 1000)                              // set eep_ms_tick every 1/1000 of a second
    {
        eeprom_ms_tick = 1;
        eeprom_cnt = 0;
    }
#endif

    if (ambilight_clock_wait_cycles)                                    // wait cycles for ambilight mode clock & clock2
    {
        ambilight_clock_wait_cnt++;

        if (ambilight_clock_wait_cnt >= ambilight_clock_wait_cycles)
        {
            ambilight_clock_wait_cnt = 0;

            ambi_tick_cnt++;

            if (ambi_tick_cnt == AMBILIGHT_CLOCK_TICK_COUNT_PER_LED)
            {
                ambilight_clock_led_idx++;
                ambi_tick_cnt = 0;
            }

            ambilight_clock_tick = 1;                                   // set AMBILIGHT_CLOCK_TICK_COUNT_PER_LED times per ambilight LED
        }
    }

    clk_cnt++;

    if (clk_cnt == F_INTERRUPTS)                                        // increment internal clock every second
    {
        clk_cnt         = 0;
        seconds_flag    = 1;

        uptime++;
        gmain.second++;

        if (gmain.second == 45)                                         // get rtc time at hh:mm:45, but not twice in same minute
        {
            if (last_minute_of_ds3231_flag != gmain.minute)
            {
                last_minute_of_ds3231_flag = gmain.minute;
                ds3231_flag = 1;
            }
        }
        else if (gmain.second == 49)
        {
            measure_temperature_flag = 1;                               // start ADC conversion of DS18xx
        }
        else if (gmain.second == 50)
        {
            read_temperature_flag = 1;                                  // read temperature data of DS18xx
        }
        else if (gmain.second == 51)
        {
            read_rtc_temperature_flag = 1;                              // read temperature data of RTC
        }
        else if (gmain.second == 60)
        {
            gmain.second             = 0;
            ambilight_clock_wait_cnt = 0;                               // fix rounding errors by resetting wait_cnt every minute
            ambilight_clock_led_idx  = 0;                               // reset led index, too
            ambi_tick_cnt            = 0;
            gmain.minute++;

            show_time_flag = 1;

            if (gmain.minute == 60)
            {
                gmain.minute = 0;
                gmain.hour++;

                if (gmain.hour == 24)
                {
                    gmain.hour = 0;
                    gmain.wday++;

                    if (gmain.wday == 7)
                    {
                        gmain.wday = 0;
                    }

                    gmain.mday++;

                    if (gmain.mday > days_of_month (gmain.month, gmain.year))
                    {
                        gmain.mday = 1;
                        gmain.month++;

                        if (gmain.month >= 13)
                        {
                            gmain.month = 1;
                            gmain.year++;
                        }
                    }
                }
            }
        }
        else if (gmain.second == 30)
        {
            half_minute_flag = 1;
        }

        if (net_time_countdown)
        {
            net_time_countdown--;

            if (net_time_countdown == 0)                                // trigger net time update
            {
                net_time_flag = 1;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read version from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_version_from_eep (void)
{
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        rtc = eep_read (EEPROM_DATA_OFFSET_VERSION, (uint8_t *) &eep_version, sizeof(uint32_t));
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write version to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_version_to_eep (void)
{
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        rtc = eep_write (EEPROM_DATA_OFFSET_VERSION, (uint8_t *) &eep_version, sizeof(uint32_t));
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read update host from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_update_host_from_eep (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         update_host_buf[EEPROM_DATA_SIZE_UPDATE_HOSTNAME];

    if (eep_is_up)
    {
        if (eep_read (EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, update_host_buf, EEPROM_DATA_SIZE_UPDATE_HOSTNAME))
        {
            memcpy (gmain.update_host, update_host_buf, EEPROM_DATA_SIZE_UPDATE_HOSTNAME);
            gmain.update_host[EEPROM_MAX_HOSTNAME_LEN - 1] = '\0';
            rtc = 1;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write update host to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_update_host_to_eep (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         update_host_buf[EEPROM_DATA_SIZE_UPDATE_HOSTNAME];

    if (eep_is_up)
    {
        memset (update_host_buf, 0, sizeof (update_host_buf));
        strncpy ((char *) update_host_buf, gmain.update_host, EEPROM_DATA_SIZE_UPDATE_HOSTNAME - 1);
        rtc = eep_write (EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, update_host_buf, EEPROM_DATA_SIZE_UPDATE_HOSTNAME);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read update path from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_update_path_from_eep (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         update_path_buf[EEPROM_DATA_SIZE_UPDATE_PATH];

    if (eep_is_up)
    {
        if (eep_read (EEPROM_DATA_OFFSET_UPDATE_PATH, update_path_buf, EEPROM_DATA_SIZE_UPDATE_PATH))
        {
            memcpy (gmain.update_path, update_path_buf, EEPROM_DATA_SIZE_UPDATE_PATH);
            gmain.update_path[EEPROM_MAX_UPDATE_PATH_LEN - 1] = '\0';
            rtc = 1;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write update path to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_update_path_to_eep (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         update_path_buf[EEPROM_DATA_SIZE_UPDATE_PATH];

    if (eep_is_up)
    {
        memset (update_path_buf, 0, sizeof (update_path_buf));
        strncpy ((char *) update_path_buf, gmain.update_path, EEPROM_DATA_SIZE_UPDATE_PATH - 1);
        rtc = eep_write (EEPROM_DATA_OFFSET_UPDATE_PATH, update_path_buf, EEPROM_DATA_SIZE_UPDATE_PATH);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read main parameters from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_main_parameters_from_eep (void)
{
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        if (eep_version >= EEPROM_VERSION_2_6)
        {
            if (read_update_host_from_eep () &&
                read_update_path_from_eep ())
            {
                if (gmain.update_host[0] != '\0' &&
                    gmain.update_path[0] != '\0' &&
                    (uint8_t) gmain.update_host[0] != 0xFF &&
                    (uint8_t) gmain.update_path[0] != 0xFF)
                {                                                                   // eeprom correctly initialized?
                    rtc = 1;
                }
            }
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write main parameters to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_main_parameters_to_eep (void)
{
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        if (write_update_host_to_eep () &&
            write_update_path_to_eep ())
        {
            rtc = 1;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log eep write error
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
log_eep_error (const char * str)
{
    log_printf ("Writing %s to EEPROM failed\r\n", str);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write version to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
read_configuration_from_eep (void)
{
    if (eep_is_up)
    {
        log_message ("eeprom/flash is online");
        display_set_status_or_minute_leds (0, 1, 0);                                        // show green status or minute LEDs
        read_version_from_eep ();
        //eep_version = 0x00000000;                                                         // hack: reset EEPROM to default values
        log_printf ("current eeprom/flash version: 0x%08x\r\n", eep_version);

        if ((eep_version & 0xFF0000FF) == 0x00000000 &&
            eep_version >= EEPROM_VERSION_1_5 && eep_version <= EEPROM_VERSION)
        {                                                                                   // Upper and Lower Byte must be 0x00
            gmain.eep_version[0] = ((eep_version >> 16) & 0xFF) + '0';
            gmain.eep_version[1] = '.';
            gmain.eep_version[2] = ((eep_version >>  8) & 0xFF) + '0';
            gmain.eep_version[4] = '\0';

            if (eep_version >= EEPROM_VERSION_1_5)
            {
                log_message ("reading ir codes");
                remote_ir_read_codes_from_eep ();

                debug_log_message ("reading display configuration");
                display_read_config_from_eep (eep_version);

                debug_log_message ("reading timeserver data");
                timeserver_read_data_from_eep ();
            }

            debug_log_message ("reading overlay configuration");
            overlay_read_config_from_eep (eep_version);

            debug_log_message ("reading night timers");
            night_read_data_from_eep (eep_version);

            debug_log_message ("reading alarm timers");
            alarm_read_data_from_eep (eep_version);

            if (eep_version >= EEPROM_VERSION_2_1)
            {
                debug_log_message ("reading temp configuration");
                temp_read_config_from_eep (eep_version);
                rtc_read_config_from_eep (eep_version);

                debug_log_message ("reading LDR configuration");
                ldr_read_config_from_eep (eep_version);
            }

            if (eep_version >= EEPROM_VERSION_2_2)
            {
                debug_log_message ("reading weather configuration");
                weather_read_config_from_eep ();
            }

            debug_log_message ("reading dfplayer configuration");
            dfplayer_read_config_from_eep (eep_version);

            debug_log_message ("reading update host/path configuration");
            if (! read_main_parameters_from_eep ())
            {
                strcpy (gmain.update_host, DEFAULT_UPDATE_HOST);
                strcpy (gmain.update_path, DEFAULT_UPDATE_PATH);
                write_main_parameters_to_eep ();
            }

#if DSP_USE_TFTLED_RGB == 1
            if (eep_version >= EEPROM_VERSION_2_9)
            {
                debug_log_message ("reading SSD1963 flags");
                ssd1963_read_flags_from_eep ();
            }
#endif
        }
        display_set_status_or_minute_leds (0, 0, 0);                                       // switch status or minute LEDs off
    }
    else
    {
        log_message ("eeprom/flash is offline");
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write version to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
upgrade_eep_version (void)
{
    if (eep_is_up)
    {
        if (eep_version != EEPROM_VERSION)
        {
            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs
            log_printf ("updating EEPROM to version 0x%08x... ", EEPROM_VERSION);

            eep_version = EEPROM_VERSION;

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! remote_ir_write_codes_to_eep ())
            {
                log_eep_error ("IR codes");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! display_write_config_to_eep ())
            {
                log_eep_error ("display configuration");
            }

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! overlay_write_config_to_eep ())
            {
                log_eep_error ("overlay configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! timeserver_write_data_to_eep ())
            {
                log_eep_error ("timeserver configuration");
            }

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! night_write_data_to_eep ())
            {
                log_eep_error ("night time configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! alarm_write_data_to_eep ())
            {
                log_eep_error ("alarm time configuration");
            }

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! temp_write_config_to_eep ())
            {
                log_eep_error ("temperature configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! rtc_write_config_to_eep ())
            {
                log_eep_error ("RTC configuration");
            }

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! ldr_write_config_to_eep ())
            {
                log_eep_error ("LDR configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! weather_write_config_to_eep ())
            {
                log_eep_error ("weather configuration");
            }

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! dfplayer_write_config_to_eep ())
            {
                log_eep_error ("dfplayer configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

#if DSP_USE_TFTLED_RGB == 1
            if (! ssd1963_write_flags_to_eep ())
            {
                log_eep_error ("SSD1963 configuration");
            }
#endif

            display_set_status_or_minute_leds (1, 1, 0);                                        // show yellow status or minute LEDs

            if (! write_main_parameters_to_eep ())
            {
                log_eep_error ("update main parameters configuration");
            }

            display_set_status_or_minute_leds (1, 0, 0);                                        // show red status or minute LEDs

            if (! write_version_to_eep ())                                                      // at least, write new eeprom version
            {
                log_eep_error ("version");
            }

            log_message ("done");
            eep_version = EEPROM_VERSION;

            display_set_status_or_minute_leds (0, 0, 0);                                        // switch off status or minute LEDs
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display power
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
set_display_power (uint_fast8_t new_power_is_on, uint_fast8_t do_sync_ambilight)
{
    uint_fast8_t    display_clock_flag;

    log_printf ("set_display_power: new=%d sync_ambi=%d old_display=%d old_ambi=%d\r\n",
                new_power_is_on,
                do_sync_ambilight,
                display.display_power_is_on,
                display.ambilight_power_is_on);

    display.display_power_is_on = new_power_is_on;

    if (do_sync_ambilight)
    {
        display_set_ambilight_power (display.display_power_is_on);
    }

    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;

    if (display.display_power_is_on)
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_ON;
    }
    else
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_OFF;
    }

    if (esp8266.is_online)
    {
        var_send_display_power ();
    }

    if (display.display_power_is_on || display.ambilight_power_is_on)
    {
        dcf77_enabled = FALSE;
    }
    else
    {
        dcf77_enabled = TRUE;
    }

    log_printf ("set_display_power: flags=0x%02x display=%d ambi=%d dcf77=%d\r\n",
                display_clock_flag,
                display.display_power_is_on,
                display.ambilight_power_is_on,
                dcf77_enabled);

    return display_clock_flag;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * speak clock time
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
speak (uint_fast8_t hh, uint_fast8_t mm)
{
    static uint8_t  words[WP_COUNT];
    uint_fast8_t    do_show_it_is = 0;
    uint_fast16_t   idx;

    if (tables.complete)
    {
#if WCLOCK24H == 0
        mm /= 5;
#endif
        if ((display.display_flags & DISPLAY_FLAGS_PERMANENT_IT_IS) || mm == 0 || mm == MINUTE_COUNT / 2)
        {
            do_show_it_is = 1;
        }

        if (tables_fill_words (words, hh, mm))
        {
            dfplayer_flush_queue ();

            for (idx = 0; idx < WP_COUNT; idx++)
            {
                if (words[idx])
                {
                    if (do_show_it_is || !(tables.illumination[idx].len & ILLUMINATION_FLAG_IT_IS))
                    {
                        dfplayer_enqueue (idx);
                    }
                }
            }

            dfplayer_start_queue ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_messages () - schedule messages of ESP8266
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t     show_temperature            = 0;
static uint_fast8_t     show_temperature_digits     = 0;
static uint_fast8_t     display_clock_flag          = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
static uint_fast8_t     show_date                   = 0;
static uint_fast8_t     last_ldr_value              = 0xFF;
static uint_fast8_t     show_overlay_idx            = MAX_OVERLAYS;
static uint_fast8_t     icon_duration               = 0;
/* Nachgezogener Display-Restore fuer JEDEN nicht blockierenden Ticker, nicht mehr nur fuer
 * den Wetterticker: Es laufen jetzt auch ticker_set, der IP-Ticker der Startsequenz, das
 * Ticker-Overlay und der Datumsticker mit do_wait = 0 (BEFUNDE.md L204, L211, L216). Der
 * Name bleibt unveraendert, weil CLAUDE.md und knowledge/quick-reference.md ihn woertlich
 * als Architektur-Invariante fuehren.
 */
static uint_fast8_t     pending_weather_ticker_restore = 0;
static uint32_t         show_icon_stop_time         = 0;
static uint32_t         local_uptime                = 0;
static uint_fast8_t     ir_export_idx               = N_REMOTE_IR_CMDS;     // Abzug der IR-Codes: naechster Index, N == nichts zu tun
static uint_fast8_t     var_sync_pending            = 0;                    // SYNCVARS gesehen: Vollabgleich im Hauptloop faellig

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set_overlay_idx () - used by external NIC function wc_display_overlay()
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
set_overlay_idx (uint_fast8_t idx)
{
    if (idx < MAX_OVERLAYS)
    {
        show_overlay_idx = idx;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * show_icon () - used by external NIC function wc_display_icon()
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
show_icon (const char * icon, uint_fast8_t duration)
{
    display_get_icon (icon, duration);
    icon_duration = duration;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get_date () - used by external NIC function wc_wordclock_date()
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
char *
get_date (void)
{
    static char date[16];

    sprintf (date, "%4d-%02d-%02d", gmain.year, gmain.month, gmain.mday);
    return date;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get_time () - used by external NIC function wc_wordclock_time()
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
char *
get_time (void)
{
    static char tim[16];

    sprintf (tim, "%02d:%02d:%02d", gmain.hour, gmain.minute, gmain.second);
    return tim;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_remote_procedure () - schedule remote procedure calls by ESP8266
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_remote_procedure (char * parameters)
{
    uint_fast8_t        var_idx;

    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (var_idx)
    {
        case LDR_MIN_VALUE_RPC_VAR:
        {
            debug_log_message ("rpc: set LDR value as minimum");
            ldr_set_min_value ();
            var_send_ldr_min_value ();
            break;
        }

        case LDR_MAX_VALUE_RPC_VAR:
        {
            debug_log_message ("rpc: set LDR value as maximum");
            ldr_set_max_value ();
            var_send_ldr_max_value ();
            break;
        }

        case LEARN_IR_RPC_VAR:
        {
            debug_log_message ("rpc: learn IR codes");

            if (remote_ir_learn ())
            {
                remote_ir_write_codes_to_eep ();
            }
            break;
        }

        case GET_NET_TIME_RPC_VAR:
        {
            net_time_flag = 1;
            debug_log_message ("rpc: start net time request");
            break;
        }

        case DISPLAY_TEMPERATURE_RPC_VAR:
        {
            show_temperature = 1;
            debug_log_message ("rpc: show temperature");
            break;
        }

        case TEST_DISPLAY_RPC_VAR:
        {
            debug_log_message ("rpc: start display test");
            display_test ();
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            break;
        }

        case GET_WEATHER_RPC_VAR:
        {
            weather_query (WEATHER_QUERY_ID_TEXT);
            debug_log_message ("rpc: get weather");
            break;
        }

        case GET_WEATHER_FC_RPC_VAR:
        {
            weather_query (WEATHER_QUERY_ID_TEXT_FC);
            debug_log_message ("rpc: get weather forecast");
            break;
        }

        case RESET_EEPROM_RPC_VAR:
        {
            eep_version = EEPROM_VERSION_0_0;
            write_version_to_eep ();
            debug_log_message ("rpc: reset EEPROM/FLASH");
            break;
        }

        case DISPLAY_DATE_RPC_VAR:
        {
            show_date = 1;
            debug_log_message ("rpc: show date");
            break;
        }

        case GET_IR_CODES_RPC_VAR:
        {
            /* Hier wird nur der Zaehler zurueckgesetzt, gesendet wird im Hauptloop.
             *
             * Eine for-Schleife ueber die 20 Kommandos stuende an dieser Stelle ohne jeden
             * watchdog_reload(): var_send_buf() wartet je Kommando bis zu
             * VAR_SEND_TIMEOUT_SEC (3 s) auf die Quittung des ESP, 20 davon sind bis zu 60 s
             * gegen 20 s Watchdog. Derselbe Mechanismus macht var_send_all_variables() zu
             * Befund L85, nur groesser.
             *
             * Ein Kommando je Hauptloop-Durchlauf loest das durch die Struktur statt durch
             * Sorgfalt: Zwischen zwei I-Kommandos liegt garantiert der regulaere
             * watchdog_reload() am Kopf des Loops, ohne eine einzige neue Aufrufstelle. Und
             * der Abzug friert die Uhr nicht ein, der Loop laeuft dazwischen weiter.
             */
            ir_export_idx = 0;
            debug_log_message ("rpc: send IR codes");
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_numeric_variable () - schedule ESP8266 numeric variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_numeric_variable (char * parameters)
{
    uint_fast8_t    var_idx;
    uint_fast8_t    lo;
    uint_fast8_t    hi;
    uint_fast16_t   val;

    var_idx = htoi (parameters, 2);
    parameters += 2;

    lo      = htoi (parameters, 2);
    parameters += 2;
    hi      = htoi (parameters, 2);
    parameters += 2;

    val     = (hi << 8) | lo;

    switch (var_idx)
    {
        case EEPROM_IS_UP_NUM_VAR:
        {
            debug_log_printf ("cmd: set eep_is_up = %d, but it's readonly\r\n", val);
            break;
        }

        case HARDWARE_CONFIGURATION_NUM_VAR:
        {
            debug_log_printf ("cmd: set hardware_configuration = %d, but it's readonly\r\n", val);
            break;
        }

        case RTC_IS_UP_NUM_VAR:
        {
            debug_log_printf ("cmd: set rtc_is_up = %d, but it's readonly\r\n", val);
            break;
        }

        case DISPLAY_POWER_NUM_VAR:
        {
            if (display.display_power_is_on != val)
            {
                display_clock_flag = set_display_power (val, TRUE);
                debug_log_printf ("cmd: set power_is_on = %d, display_clock_flag=0x%02x\r\n", val, display_clock_flag);
            }
            break;
        }

        case DISPLAY_AMBILIGHT_POWER_NUM_VAR:
        {
            if (display.ambilight_power_is_on != val)
            {
                display_set_ambilight_power (val);
                debug_log_printf ("cmd: set power_is_on = %d\r\n", val);
            }
            break;
        }

        case DISPLAY_MODE_NUM_VAR:
        {
            if (display.display_mode != val)
            {
                display_set_display_mode (val, FALSE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_printf ("cmd: set display_mode = %d\r\n", val);
            }
            break;
        }

        case SSD1963_FLAGS_NUM_VAR:
        {
#ifdef BLACK_BOARD                                                              // TFT & SSD1963 only for STM32F407
            if (ssd1963.flags != val)
            {
#if DSP_USE_TFTLED_RGB == 1
                ssd1963_set_flags (val, FALSE);
#endif
                debug_log_printf ("cmd: set ssd1963_flags = %d\r\n", val);
            }
#else
            debug_log_printf ("cmd: set ssd1963_flags = %d, but TFT is not active on this hardware\r\n", val);
#endif
            break;
        }

        case DISPLAY_BRIGHTNESS_NUM_VAR:
        {
            if (display.automatic_brightness)
            {
                last_ldr_value = 0xFF;
                display_set_automatic_brightness (0, FALSE);
            }
            display_set_display_brightness (val, FALSE, TRUE);
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
            debug_log_printf ("cmd: set display_brightness = %d, disable autmomatic brightness control per LDR\r\n", val);
            break;
        }

        case DISPLAY_FLAGS_NUM_VAR:
        {
            display_set_display_flags (val);
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            debug_log_printf ("cmd: set display_flags = 0x%02x\r\n", val);
            break;
        }

        case DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR:
        {
            last_ldr_value = 0xFF;

            if (val)
            {
                display_set_automatic_brightness (1, FALSE);
            }
            else
            {
                display_set_automatic_brightness (0, FALSE);
            }
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;

            debug_log_printf ("cmd: set automatic_brightness = %d\r\n", val);
            break;
        }

        case ANIMATION_MODE_NUM_VAR:
        {
            if (display.animation_mode != val)
            {
                display_set_animation_mode (val, FALSE);
                animation_flag = 0;
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }
            debug_log_printf ("cmd: set animation_mode = %d\r\n", val);
            break;
        }

        case AMBILIGHT_MODE_NUM_VAR:
        {
            if (display.ambilight_mode != val)
            {
                display_set_ambilight_mode (val, FALSE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }
            debug_log_printf ("cmd: set ambilight_mode = %d\r\n", val);
            break;
        }

        case AMBILIGHT_LEDS_NUM_VAR:
        {
            display_set_number_of_ambilight_leds (val);
            debug_log_printf ("cmd: set number of ambilight leds = %d\r\n", val);
            break;
        }

        case AMBILIGHT_OFFSET_NUM_VAR:
        {
            display_set_ambilight_led_offset (val);
            debug_log_printf ("cmd: set ambilight led offset = %d\r\n", val);
            break;
        }

        case AMBILIGHT_BRIGHTNESS_NUM_VAR:
        {
            display_set_ambilight_brightness (val, FALSE, TRUE);
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
            debug_log_printf ("cmd: set ambilight brightness = %d\r\n", val);
            break;
        }

        case COLOR_ANIMATION_MODE_NUM_VAR:
        {
            if (display.color_animation_mode != val)
            {
                display_set_color_animation_mode (val, FALSE);
            }
            debug_log_printf ("cmd: set color animation_mode = %d\r\n", display.color_animation_mode);
            break;
        }

        case LDR_RAW_VALUE_NUM_VAR:
        {
            debug_log_printf ("cmd: set ldr_raw_value = %d, but it's readonly\r\n", val);
            break;
        }

        case LDR_MIN_VALUE_NUM_VAR:
        {
            debug_log_printf ("cmd: set ldr_min_value = %d, but it's readonly\r\n", val);
            break;
        }

        case LDR_MAX_VALUE_NUM_VAR:
        {
            debug_log_printf ("cmd: set ldr_max_value = %d, but it's readonly\r\n", val);
            break;
        }

        case TIMEZONE_NUM_VAR:
        {
            uint_fast8_t    do_observe_summertime = 0;
            int_fast16_t    tz = val & 0xFF;

            if (val & 0x100)
            {
                tz = -tz;
            }

            if (val & 0x200)
            {
                do_observe_summertime = 1;
            }

            debug_log_printf ("cmd: set timezone = %d\r\n", tz);
            timeserver_set_timezone (tz, do_observe_summertime);
            break;
        }

        case DS18XX_IS_UP_NUM_VAR:
        {
            debug_log_printf ("cmd: set ds18xx_is_up = %d, but it's readonly\r\n", val);
            break;
        }

        case RTC_TEMP_INDEX_NUM_VAR:
        {
            debug_log_printf ("cmd: set rtc_temp_index = %d, but it's readonly\r\n", val);
            break;
        }

        case RTC_TEMP_CORRECTION_NUM_VAR:
        {
            // Das uebertragene Byte ist ein Zweierkomplement: der ESP schickt fuer -1 eine 0xFF.
            // Vorzeichenlos gelesen wurde daraus eine 255 und damit jede negative Korrektur Unsinn.
            int_fast8_t corr = (int8_t) (val & 0xFF);

            rtc_set_temp_correction (corr);
            read_rtc_temperature_flag = 1;
            debug_log_printf ("cmd: set rtc_temp_correction = %d\r\n", (int) corr);
            break;
        }

        case DS18XX_TEMP_INDEX_NUM_VAR:
        {
            debug_log_printf ("cmd: set ds18xx_temp_index = %d, but it's readonly\r\n", val);
            break;
        }

        case DS18XX_TEMP_CORRECTION_NUM_VAR:
        {
            int_fast8_t corr = (int8_t) (val & 0xFF);                               // Zweierkomplement, siehe RTC_TEMP_CORRECTION_NUM_VAR

            temp_set_temp_correction (corr);
            measure_temperature_flag = 1;                                           // measure & read
            read_temperature_flag = 1;
            debug_log_printf ("cmd: set ds18xx_temp_correction = %d\r\n", (int) corr);
            break;
        }

        case TICKER_DECELRATION_NUM_VAR:
        {
            display_set_ticker_deceleration (val);
            debug_log_printf ("cmd: set ticker_deceleration = %d\r\n", val);
            break;
        }
        case DFPLAYER_IS_UP_NUM_VAR:
        {
            debug_log_printf ("cmd: set TABLE_MODDES_COUNT = %d, but it's readonly\r\n", val);
            break;
        }
        case DFPLAYER_VOLUME_NUM_VAR:
        {
            dfplayer_set_new_volume (val);
            debug_log_printf ("cmd: set dfplayer volume = %d\r\n", val);
            break;
        }
        case DFPLAYER_SILENCE_START_NUM_VAR:
        {
            dfplayer_set_silence_start (val);
            debug_log_printf ("cmd: set dfplayer silence start = %d\r\n", val);
            break;
        }
        case DFPLAYER_SILENCE_STOP_NUM_VAR:
        {
            dfplayer_set_silence_stop (val);
            debug_log_printf ("cmd: set dfplayer silence stop = %d\r\n", val);
            break;
        }
        case DFPLAYER_MODE_NUM_VAR:
        {
            dfplayer_set_mode (val);
            debug_log_printf ("cmd: set dfplayer mode = %d\r\n", val);
            break;
        }
        case DFPLAYER_BELL_FLAGS_NUM_VAR:
        {
            dfplayer_set_bell_flags (val);
            debug_log_printf ("cmd: set dfplayer bell flags = %d\r\n", val);
            break;
        }
        case DFPLAYER_SPEAK_CYCLE_NUM_VAR:
        {
            dfplayer_set_speak_cycle (val);
            debug_log_printf ("cmd: set dfplayer speak cycle = %d\r\n", val);
            break;
        }
        case OBSOLETE_2_NUMVAR:
        {
            debug_log_printf ("cmd: set obsolete num var idx %d ignored\r\n", var_idx);
            break;
        }
        default:
        {
            log_printf ("cmd: set unknown num var idx %d: %d\r\n", var_idx, val);
            break;
        }
        case DFPLAYER_PLAY_FOLDER_TRACK_NUM_VAR:
        {
            uint_fast8_t    folder;
            uint_fast8_t    track;
            folder = (val >> 8) & 0xFF;
            track  = val & 0xFF;

            dfplayer_play_folder (folder, track);
            debug_log_printf ("cmd: play folder %d track %d\r\n", folder, track);
            break;
        }
        case DISPLAY_OVERLAY_NUM_VAR:
        {
            show_overlay_idx = val;
            debug_log_printf ("cmd: display overlay %d\r\n", val);
            break;
        }
        case OVERLAY_N_OVERLAYS_NUM_VAR:
        {
            overlay_set_n_overlays (val);
            debug_log_printf ("cmd: set number of overlays %d\r\n", val);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_numeric_array () - schedule ESP8266 numeric arrays
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_numeric_array (char * parameters)
{
    uint_fast8_t    var_idx;
    uint_fast8_t    n;
    uint_fast8_t    val;

    var_idx = htoi (parameters, 2);
    parameters += 2;

    n = htoi (parameters, 2);
    parameters += 2;

    val = htoi (parameters, 2);

    switch (var_idx)
    {
        case DISPLAY_DIMMED_DISPLAY_COLORS:
        {
            if (n < sizeof (display.dimmed_display_colors))
            {
                if (val <= MAX_BRIGHTNESS)
                {
                    display_set_dimmed_display_color (n, val);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                    debug_log_printf ("cmd: set dimmed_display_colors[%d] = %d\r\n", n, val);
                }
            }
            else
            {
                debug_log_printf ("cmd: set dimmed_display_colors[%d] = %d, but n exceeds array size %d\r\n",
                                  sizeof (display.dimmed_display_colors), n, val);
            }
            break;
        }
        case DISPLAY_DIMMED_AMBILIGHT_COLORS:
        {
            if (n < sizeof (display.dimmed_ambilight_colors))
            {
                if (val <= MAX_BRIGHTNESS)
                {
                    display_set_dimmed_ambilight_color (n, val);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                    debug_log_printf ("cmd: set dimmed_ambilight_colors[%d] = %d\r\n", n, val);
                }
            }
            else
            {
                debug_log_printf ("cmd: set dimmed_ambilight_colors[%d] = %d, but n exceeds array size %d\r\n",
                                  sizeof (display.dimmed_ambilight_colors), n, val);
            }
            break;
        }
    }
}

#define IR_CODE_PARAM_LEN       12                                      // iippaaaacccc, ohne das fuehrende 'I'

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_ir_code () - schedule a single IR code: I<idx:2><protocol:2><address:4><command:4>
 *
 * Feste Breiten in beiden Richtungen: gelesen wird mit htoi() in fester Breite, gesendet wird in
 * var_send_ir_code() mit maskierten Werten.
 *
 * Jede Abweisung meldet sich ueber log_printf, nicht ueber debug_log_printf: Letzteres ist ohne
 * -DDEBUG ein leeres Makro (log.h:23-31), und DEBUG wird im Build nirgends gesetzt -- die
 * Meldung gaebe es im ausgelieferten Fabrikat also gar nicht. Nur der Erfolgsfall bleibt
 * bedingt: zwanzig zusaetzliche blockierende Logzeilen in genau dem Zeitfenster, in dem der
 * Abzug ueber die Bruecke laeuft, waeren der falsche Preis.
 *
 * Warum der STM ueberhaupt prueft, obwohl der ESP es schon tut: Die ESP-Pruefung schuetzt gegen
 * falsche Eingaben, nicht gegen Verstuemmelung auf der Strecke. Der Empfangsring verwirft bei
 * Ueberlauf, aber nicht unbemerkt: esp8266_uart_rxdrops() zaehlt jeden Verwurf saettigend mit,
 * der Stand steht als d= in der diag-Zeile, und die Logwache meldet Spruenge darin (DIR-013).
 * Gezaehlt ist aber nicht verhindert: Das Zeichen bleibt weg, und die Uebernahme schneidet
 * mit strncpy() still ab (esp8266.c:516). htoi() faengt das Abschneiden seit L232 ab -- die
 * Schleife haelt am ersten Nullbyte --, aber nicht die Verstuemmelung: Jedes Nicht-Hex-Zeichen
 * wird weiterhin still zu 0, und ein zu kurzes Kommando liefert einen zu kleinen statt eines
 * aufgefuellten Werts. Beides erkennt nur die Pruefung hier.
 *
 * Aus einem unterwegs abgeschnittenen "I0502123400 01" wuerde so "I05" -- idx 5, protocol 0,
 * address 0, command 0. Und protocol 0 ist die Konvention "nie angelernt": Taste 5 waere
 * geloescht, persistent, im einzigen Datenbestand des Geraets ohne Rueckweg ausser erneutem
 * Anlernen mit der Fernbedienung in der Hand. Ein verlorenes Zeichen mitten im Kommando
 * verschoebe die Felder und traefe eine andere Taste mit einem falschen Code.
 *
 * Deshalb hier, vor jedem Schreibzugriff: genau 12 Hexziffern und danach Stringende; Index in
 * 0 .. N_REMOTE_IR_CMDS-1; protocol weder 0x00 noch 0xFF. Nichts wird zurechtgebogen -- das ist
 * die Lektion aus L70, wo ein fehlender Parameter still zum Vorgabeindex 0 wurde und die
 * falsche Taste traf.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_ir_code (char * parameters)
{
    char            shown[IR_CODE_PARAM_LEN + 1];
    uint_fast8_t    idx;
    uint_fast8_t    protocol;
    uint_fast16_t   address;
    uint_fast16_t   command;
    uint_fast8_t    i;
    uint_fast8_t    all_hex = 1;

    /* Bewusst kein strlen(): esp8266.u.cmd wird mit strncpy(..., ESP8266_MAX_CMD_LEN) gefuellt
     * (esp8266.c:386) und ist bei voller Laenge nicht nullterminiert. Gelesen werden hoechstens
     * 12 Zeichen plus das erwartete Nullbyte dahinter. shown[] wird dabei mitgefuellt und auf
     * druckbare Zeichen beschraenkt, damit eine verstuemmelte Zeile keine Steuerzeichen auf die
     * Logleitung legt.
     */
    for (i = 0; i < IR_CODE_PARAM_LEN; i++)
    {
        char c = parameters[i];

        if (! ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
        {
            all_hex = 0;
        }

        shown[i] = (c >= 0x20 && c <= 0x7E) ? c : '?';

        if (! c)
        {
            break;                                                  // Stringende: nicht darueber hinaus lesen
        }
    }

    shown[i] = '\0';

    if (! all_hex || parameters[IR_CODE_PARAM_LEN] != '\0')         // Reihenfolge wichtig: der zweite Zugriff
    {                                                               // ist nur sicher, wenn alle 12 Zeichen Hex sind
        log_printf ("cmd: ir_code rejected, expected 12 hex digits: \"%s\"\r\n", shown);
        return;
    }

    idx = htoi (parameters, 2);
    parameters += 2;

    protocol = htoi (parameters, 2);
    parameters += 2;

    address = htoi (parameters, 4);
    parameters += 4;

    command = htoi (parameters, 4);

    if (idx >= N_REMOTE_IR_CMDS)
    {
        log_printf ("cmd: ir_code[%d] rejected: index exceeds %d\r\n", (int) idx, (int) (N_REMOTE_IR_CMDS - 1));
        return;
    }

    /* protocol 0x00 und 0xFF sind die Konvention "nie angelernt". Der STM bietet kein Loeschen
     * einer Taste an, also fuehrt er auch keines aus -- zweite Verteidigungslinie neben der
     * Pruefung auf dem ESP. Folge, bewusst in Kauf genommen: Ein Restore kann eine Taste nie
     * mehr leeren. Die Spec bietet das ohnehin nicht an, und der Preis dafuer -- eine Taste
     * versehentlich unbrauchbar zu machen -- waere deutlich hoeher als der Nutzen.
     */
    if (protocol == 0x00 || protocol == 0xFF)
    {
        log_printf ("cmd: ir_code[%d] rejected: protocol %02x is the empty convention, no delete via command\r\n",
                    (int) idx, (unsigned int) protocol);
        return;
    }

    if (remote_ir_set_code (idx, protocol, address, command) == REMOTE_IR_SET_OK)
    {
        debug_log_printf ("cmd: set ir_code[%d] = %02x %04x %04x\r\n",
                          (int) idx, (unsigned int) protocol, (unsigned int) address, (unsigned int) command);
    }
    else
    {                                                               // Format, Index und protocol sind geprueft, bleibt nur das EEPROM
        log_printf ("cmd: ir_code[%d] failed: eeprom down or write error\r\n", (int) idx);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_string_variable () - schedule ESP8266 string variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_string_variable (char * parameters)
{
    uint_fast8_t    var_idx;

    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (var_idx)
    {
        case TICKER_TEXT_STR_VAR:
        {
            /* Nicht blockierend (BEFUNDE.md L204): Mit do_wait = 1 liess display_set_ticker()
             * den kompletten Tickertext hier im Hauptloop durchscrollen -- bei 32 Zeichen
             * 204 Iterationen a rund 93 ms, also 13 bis 20 s Stillstand je Aufruf. Zehn
             * schnelle ticker_set hintereinander legten den Loop rund 90 s still; der
             * Watchdog schwieg dabei, weil die Schleife ihn selbst bedient
             * (display.c, zwei watchdog_reload() im Rumpf). Mit do_wait = 0 scrollt der
             * Ticker ueber display_animation() im periodischen Zweig -- gleiche
             * Schrittweite, nur ohne Blockade. Vorbild ist der Wetterticker
             * (case ESP8266_WEATHER weiter unten).
             *
             * Hier steht bewusst KEIN display_clock_flag: Es wuerde den Ticker im naechsten
             * Durchlauf sofort zumalen. Den Restore zieht die Bedingung um
             * pending_weather_ticker_restore nach, sobald display_ticker_active() falsch ist.
             */
            display_set_ticker ((unsigned char *) parameters, 0);
            pending_weather_ticker_restore = 1;
            debug_log_printf ("cmd: print ticker: '%s'\r\n", parameters);
            break;
        }

        case VERSION_STR_VAR:
        {
            debug_log_printf ("cmd: set VERSION = '%s', but it's readonly\r\n", parameters);
            break;
        }

        case EEPROM_VERSION_STR_VAR:
        {
            debug_log_printf ("cmd: set eep_version = '%s', but it's readonly\r\n", parameters);
            break;
        }

        case ESP8266_VERSION_STR_VAR:
        {
            debug_log_printf ("cmd: set esp8266_version = '%s', but it's readonly\r\n", parameters);
            break;
        }

        case TIMESERVER_STR_VAR:
        {
            timeserver_set_timeserver (parameters);
            debug_log_printf ("cmd: set timeserver = '%s'\r\n", parameters);
            break;
        }

        case WEATHER_APPID_STR_VAR:
        {
            weather_set_appid (parameters);
            debug_log_printf ("cmd: set weather appid = '%s'\r\n", parameters);
            break;
        }

        case WEATHER_CITY_STR_VAR:
        {
            weather_set_city (parameters);
            debug_log_printf ("cmd: set weather city = '%s'\r\n", parameters);
            break;
        }

        case WEATHER_LON_STR_VAR:
        {
            weather_set_lon (parameters);
            debug_log_printf ("cmd: set weather lon = '%s'\r\n", parameters);
            break;
        }

        case WEATHER_LAT_STR_VAR:
        {
            weather_set_lat (parameters);
            debug_log_printf ("cmd: set weather lat = '%s'\r\n", parameters);
            break;
        }

        case UPDATE_HOST_VAR:
        {
            strncpy (gmain.update_host, parameters, EEPROM_MAX_HOSTNAME_LEN - 1);
            gmain.update_host[EEPROM_MAX_HOSTNAME_LEN - 1] = '\0';
            write_update_host_to_eep ();
            debug_log_printf ("cmd: set update host = '%s'\r\n", parameters);
            break;
        }

        case UPDATE_PATH_VAR:
        {
            strncpy (gmain.update_path, parameters, EEPROM_MAX_UPDATE_PATH_LEN - 1);
            gmain.update_path[EEPROM_MAX_UPDATE_PATH_LEN - 1] = '\0';
            write_update_path_to_eep ();
            debug_log_printf ("cmd: set update path = '%s'\r\n", parameters);
            break;
        }

        case DATE_TICKER_FORMAT_VAR:
        {
            display_set_date_ticker_format (parameters);
            debug_log_printf ("cmd: set date ticker format = '%s'\r\n", parameters);
            break;
        }

    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_time_variable () - schedule ESP8266 time tables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_time_variable (char * parameters)
{
    uint_fast8_t    var_idx;

    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (var_idx)
    {
        case CURRENT_TM_VAR:
        {
            // struct tm tmtemp;
            gmain.tm.tm_year = 1000 * (parameters[0]  - '0') +
                                100 * (parameters[1]  - '0') +
                                 10 * (parameters[2]  - '0') +
                                  1 * (parameters[3]  - '0');
            gmain.tm.tm_mon  =   10 * (parameters[4]  - '0') +
                                  1 * (parameters[5]  - '0');
            gmain.tm.tm_mday =   10 * (parameters[6]  - '0') +
                                  1 * (parameters[7]  - '0');
            gmain.tm.tm_hour =   10 * (parameters[8]  - '0') +
                                  1 * (parameters[9]  - '0');
            gmain.tm.tm_min  =   10 * (parameters[10] - '0') +
                                  1 * (parameters[11] - '0');
            gmain.tm.tm_sec =    10 * (parameters[12] - '0') +
                                  1 * (parameters[13] - '0');

            gmain.tm.tm_wday = dayofweek (gmain.tm.tm_mday, gmain.tm.tm_mon + 1, gmain.tm.tm_year + 1900);

            if (grtc.rtc_is_up)
            {
                rtc_set_date_time (&(gmain.tm));
            }

            if (gmain.hour != (uint_fast8_t) gmain.tm.tm_hour || gmain.minute != (uint_fast8_t) gmain.tm.tm_min)
            {
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }

            gmain.year    = gmain.tm.tm_year + 1900;
            gmain.month   = gmain.tm.tm_mon  + 1;
            gmain.mday    = gmain.tm.tm_mday;
            gmain.wday    = gmain.tm.tm_wday;
            gmain.hour    = gmain.tm.tm_hour;
            gmain.minute  = gmain.tm.tm_min;
            gmain.second  = gmain.tm.tm_sec;

            debug_log_printf ("cmd: set time = %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                                wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                                gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_display_variable () - schedule ESP8266 display variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_display_variable (char * parameters)
{
    uint_fast8_t        var_idx;
    uint_fast8_t        cmd_code;

    cmd_code = *parameters++;
    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (cmd_code)
    {
        case 'N':                                                       // DN: Display mode Name
        {
            debug_log_printf ("cmd: set display_mode_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
            break;
        }

        case 'C':                                                       // DC: Display Color
        {
            DSP_COLORS rgb;

            rgb.red = htoi (parameters, 2);
            parameters += 2;
            rgb.green = htoi (parameters, 2);
            parameters += 2;
            rgb.blue = htoi (parameters, 2);
            parameters += 2;
#if DSP_USE_SK6812_RGBW == 1
            rgb.white = htoi (parameters, 2);
            parameters += 2;
#endif

            if (var_idx == DISPLAY_DSP_COLOR_VAR)
            {
                display_set_display_colors (&(rgb));
#if DSP_USE_SK6812_RGBW == 1
                debug_log_printf ("cmd: set display_colors = %d %d %d %d\r\n", rgb.red, rgb.green, rgb.blue, rgb.white);
#else
                debug_log_printf ("cmd: set display_colors = %d %d %d\r\n", rgb.red, rgb.green, rgb.blue);
#endif
            }
            else if (var_idx == AMBILIGHT_DSP_COLOR_VAR)
            {
                display_set_ambilight_colors (&(rgb));
#if DSP_USE_SK6812_RGBW == 1
                debug_log_printf ("cmd: set ambilight_colors = %d %d %d %d\r\n", rgb.red, rgb.green, rgb.blue, rgb.white);
#else
                debug_log_printf ("cmd: set ambilight_colors = %d %d %d\r\n", rgb.red, rgb.green, rgb.blue);
#endif
            }
            else if (var_idx == AMBILIGHT_MARKER_DSP_COLOR_VAR)
            {
                display_set_ambilight_marker_colors (&(rgb));
#if DSP_USE_SK6812_RGBW == 1
                debug_log_printf ("cmd: set ambilight_marker_colors = %d %d %d %d\r\n", rgb.red, rgb.green, rgb.blue, rgb.white);
#else
                debug_log_printf ("cmd: set ambilight_marker_colors = %d %d %d\r\n", rgb.red, rgb.green, rgb.blue);
#endif
            }
            else
            {
                debug_log_printf ("DC: invalid var_idx: %d\r\n", var_idx);
            }
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_animation_variable () - schedule ESP8266 animation variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_animation_variable (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx;

    cmd_code = *parameters++;
    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (cmd_code)
    {
        case 'N':                                                       // AN: Animation mode Name
        {
            debug_log_printf ("cmd: set animation_mode_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
            break;
        }

        case 'D':                                                       // AD: Animation Deceleration
        {
            uint_fast8_t deceleration = htoi (parameters, 2);
            parameters += 2;
            display_set_animation_deceleration (var_idx, deceleration);
            debug_log_printf ("cmd: set animation_deceleration[%d] = %d\r\n", var_idx, deceleration);
            break;
        }

        case 'E':                                                       // AE: Animation default deceleration
        {
            debug_log_message ("cmd: set animation_default_deceleration, but it's readonly\r\n");
            break;
        }

        case 'F':                                                       // AF: Animation Flags
        {
            uint_fast8_t flags = htoi (parameters, 2);
            parameters += 2;
            display_set_animation_flags (var_idx, flags);
            debug_log_printf ("cmd: set animation_flags[%d] = 0x%02x\r\n", var_idx, flags);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_color_animation_variable () - schedule ESP8266 color animation variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_color_animation_variable (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx;

    cmd_code = *parameters++;
    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (cmd_code)
    {
        case 'N':                                                       // CNiis: Color animation Name
        {
            debug_log_printf ("cmd: set color_animation_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
            break;
        }

        case 'D':                                                       // CDiid: Color animation Deceleration
        {
            uint_fast8_t deceleration = htoi (parameters, 2);
            parameters += 2;
            display_set_color_animation_deceleration (var_idx, deceleration);
            debug_log_printf ("cmd: set color_animation_deceleration[%d] = %d\r\n", var_idx, deceleration);
            break;
        }

        case 'E':                                                       // CEiid: Color animation default deceleration
        {
            debug_log_message ("cmd: set color_animation_default_deceleration, but it's readonly\r\n");
            break;
        }

        case 'F':                                                       // CFiif: Color animation Flags
        {
            debug_log_message ("cmd: set color_animation_flags, but it's readonly");
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_ambilight_mode () - schedule ESP8266 ambilight modes
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_ambilight_mode (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx;

    cmd_code = *parameters++;
    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (cmd_code)
    {
        case 'N':                                                       // MN: Ambilight mode Name
        {
            debug_log_printf ("cmd: set ambilight_mode_name[%d] = '%s', but it's readonly", var_idx, parameters);
            break;
        }

        case 'D':                                                       // MD: Ambilight mode Deceleration
        {
            uint_fast8_t deceleration = htoi (parameters, 2);
            parameters += 2;
            display_set_ambilight_mode_deceleration (var_idx, deceleration);
            debug_log_printf ("cmd: set ambilight_mode_deceleration[%d] = %d\r\n", var_idx, deceleration);
            break;
        }

        case 'E':                                                       // ME: Ambilight mode default deceleration
        {
            debug_log_message ("cmd: set ambilight_mode_default_deceleration, but it's readonly\r\n");
            break;
        }

        case 'F':                                                       // MF: Ambilight mode Flags
        {
            uint_fast8_t flags = htoi (parameters, 2);
            parameters += 2;
            display_set_ambilight_mode_flags (var_idx, flags);
            debug_log_printf ("cmd: set ambilight_mode_flags[%d] = %d\r\n", var_idx, flags);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_overlay () - schedule ESP8266 overlays
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_overlay (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx;

    cmd_code = *parameters++;
    var_idx = htoi (parameters, 2);
    parameters += 2;

    switch (cmd_code)
    {
        case 'T':                                                       // OT: Overlay Type
        {
            uint_fast8_t type = htoi (parameters, 2);
            parameters += 2;
            overlay_set_type (var_idx, type);
            debug_log_printf ("cmd: set overlay type[%d] = %d\r\n", var_idx, type);
            break;
        }

        case 'I':                                                       // OI: Overlay Interval
        {
            uint_fast8_t interval = htoi (parameters, 2);
            parameters += 2;
            overlay_set_interval (var_idx, interval);
            debug_log_printf ("cmd: set overlay interval[%d] = %d\r\n", var_idx, interval);
            break;
        }

        case 'D':                                                       // OD: Overlay Duration
        {
            uint_fast8_t duration = htoi (parameters, 2);
            parameters += 2;
            overlay_set_duration (var_idx, duration);
            debug_log_printf ("cmd: set overlay duration[%d] = %d\r\n", var_idx, duration);
            break;
        }

        case 'C':                                                       // OD: Overlay Date Code
        {
            uint_fast8_t date_code = htoi (parameters, 2);
            parameters += 2;
            overlay_set_date_code (var_idx, date_code);
            debug_log_printf ("cmd: set overlay date code[%d] = %d\r\n", var_idx, date_code);
            break;
        }

        case 'S':                                                       // OS: Overlay date start
        {
            uint_fast16_t date_start = htoi (parameters, 4);
            overlay_set_date_start (var_idx, date_start);
            debug_log_printf ("cmd: set overlay date start[%d] = %02d-%02d\r\n", var_idx, date_start >> 8, date_start & 0xFF);
            break;
        }

        case 'Y':                                                       // OY: Overlay days
        {
            uint_fast8_t days = htoi (parameters, 2);
            overlay_set_days (var_idx, days);
            debug_log_printf ("cmd: set overlay days[%d] = %d\r\n", var_idx, days);

            break;
        }

        case 'N':                                                       // ON: Overlay name or text
        {
            overlay_set_text (var_idx, parameters);
            debug_log_printf ("cmd: set overlay text[%d] = %s\r\n", var_idx, parameters);
            break;
        }

        case 'F':                                                       // OF: Overlay flags
        {
            uint_fast8_t flags = htoi (parameters, 2);
            parameters += 2;
            overlay_set_flags (var_idx, flags);
            overlay_calc_dates (var_idx, gmain.year);
            debug_log_printf ("cmd: set overlay flags[%d] = %d\r\n", var_idx, flags);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_night_tables () - schedule ESP8266 night tables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_night_tables (char * parameters)
{
    uint_fast8_t        var_idx;
    uint_fast16_t       minutes;
    uint_fast8_t        flags;

    var_idx = htoi (parameters, 2);
    parameters += 2;
    minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
    parameters += 4;
    flags = htoi (parameters, 2);
    parameters += 2;

    night_time[var_idx].minutes = minutes;
    night_time[var_idx].flags = flags;
    night_write_data_to_eep ();
    debug_log_printf ("cmd: set night_time[%d]: minutes = %d, flags = 0x%02x\r\n", var_idx, minutes, flags);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_ambilight_night_tables () - schedule ESP8266 ambilight night tables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_ambilight_night_tables (char * parameters)
{
    uint_fast8_t        var_idx;
    uint_fast16_t       minutes;
    uint_fast8_t        flags;

    var_idx = htoi (parameters, 2);
    parameters += 2;
    minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
    parameters += 4;
    flags = htoi (parameters, 2);
    parameters += 2;

    ambilight_night_time[var_idx].minutes = minutes;
    ambilight_night_time[var_idx].flags = flags;
    night_write_data_to_eep ();
    debug_log_printf ("cmd: set ambilight night_time[%d]: minutes = %d, flags = 0x%02x\r\n", var_idx, minutes, flags);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_alarm_tables () - schedule ESP8266 alarm tables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_alarm_tables (char * parameters)
{
    uint_fast8_t        var_idx;
    uint_fast16_t       minutes;
    uint_fast8_t        flags;

    var_idx = htoi (parameters, 2);
    parameters += 2;
    minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
    parameters += 4;
    flags = htoi (parameters, 2);
    parameters += 2;

    alarm_time[var_idx].minutes = minutes;
    alarm_time[var_idx].flags = flags;
    alarm_write_data_to_eep ();
    debug_log_printf ("cmd: set alarm_time[%d]: minutes = %d, flags = 0x%02x\r\n", var_idx, minutes, flags);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_games () - schedule ESP8266 games
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_games (char * parameters)
{
    switch (*parameters++)
    {
        case 'T':
        {
            if (*parameters == 's')
            {
                tetris ();                                              // GTs: Start Tetris
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }
            break;
        }
        case 'S':
        {
            if (*parameters == 's')
            {
                snake ();                                               // GSs: Start Snake
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_cmd () - schedule ESP8266 commands
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
schedule_esp8266_cmd (void)
{
    char *              parameters;
    uint_fast8_t        cmd_code;

    parameters = esp8266.u.cmd;
    cmd_code = *parameters++;

    switch (cmd_code)
    {
        case 'R':                                                   // remote procedure call: Rii
        {
            schedule_esp8266_remote_procedure (parameters);
            break;
        }

        case 'N':                                                   // numeric variable: Niillhh
        {
            schedule_esp8266_numeric_variable (parameters);
            break;
        }

        case 'n':                                                   // numeric array: Niinnbb
        {
            schedule_esp8266_numeric_array (parameters);
            break;
        }

        case 'S':                                                   // string variable: Siissssssss...
        {
            schedule_esp8266_string_variable (parameters);
            break;
        }

        case 'T':
        {
            schedule_esp8266_time_variable (parameters);
            break;
        }

        case 'D':                                                   // display Dxii...
        {
            schedule_esp8266_display_variable (parameters);
            break;
        }

        case 'A':                                                   // animation
        {
            schedule_esp8266_animation_variable (parameters);
            break;
        }

        case 'C':                                                   // color animation
        {
            schedule_esp8266_color_animation_variable (parameters);
            break;
        }

        case 'M':                                                   // ambilight mode
        {
            schedule_esp8266_ambilight_mode (parameters);
            break;
        }

        case 'O':                                                   // Overlay
        {
            schedule_esp8266_overlay (parameters);
            break;
        }

        case 't':                                                   // tiimmff night table minutes + flags
        {
            schedule_esp8266_night_tables (parameters);
            break;
        }

        case 'a':                                                   // aiimmff night table minutes + flags
        {
            schedule_esp8266_ambilight_night_tables (parameters);
            break;
        }

        case 'l':                                                   // liimmff alarm table minutes + flags
        {
            schedule_esp8266_alarm_tables (parameters);
            break;
        }

        case 'G':                                                   // Gx Games
        {
            schedule_esp8266_games (parameters);
            break;
        }

        case 'I':                                                   // single IR code: Iiippaaaacccc
        {
            schedule_esp8266_ir_code (parameters);
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * schedule_esp8266_messages () - schedule messages of ESP8266
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
schedule_esp8266_messages (void)
{
    uint_fast8_t            msg_rtc;

    msg_rtc = esp8266_get_message ();

    switch (msg_rtc)
    {
        case ESP8266_CMD:
        {
            schedule_esp8266_cmd ();
            break;
        }
        case ESP8266_TIME:
        {
            char *          endptr;
            uint32_t        seconds_since_1900;

            seconds_since_1900  = strtoul (esp8266.u.time, &endptr, 10);
            timeserver_convert_time (&gmain.tm, seconds_since_1900);

            if (grtc.rtc_is_up)
            {
                rtc_set_date_time (&gmain.tm);
            }

            if (gmain.hour != (uint_fast8_t) gmain.tm.tm_hour || gmain.minute != (uint_fast8_t) gmain.tm.tm_min)
            {
                var_send_tm ();
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }

            gmain.year    = gmain.tm.tm_year + 1900;
            gmain.month   = gmain.tm.tm_mon  + 1;
            gmain.mday    = gmain.tm.tm_mday;
            gmain.wday    = gmain.tm.tm_wday;
            gmain.hour    = gmain.tm.tm_hour;
            gmain.minute  = gmain.tm.tm_min;
            gmain.second  = gmain.tm.tm_sec;

            debug_log_printf ("cmd: set time to %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                                wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                                gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
            net_time_countdown = 3600 + 180 + 40 - gmain.second;                // calc next timeserver call 1h3m later at hh:mm:40
            break;
        }
        case ESP8266_TABLES:                                                    // ESP8266 read layout tables
        {
            tables_init ();
            debug_log_message ("info: layout tables received");
            break;
        }
        case ESP8266_IPADDRESS:                                                 // ESP8266 got new ip address
        {
            unsigned char buf[32];

            sprintf ((char *) buf, "IP %s", esp8266.ipaddress);
            log_printf ("info: ip address = %s\r\n", esp8266.ipaddress);
            log_flush ();
            var_send_all_variables ();
            debug_log_message ("info: configuration sent");
            log_flush ();
            /* Nicht blockierend (BEFUNDE.md L211): Das ist der Pfad, den CLAUDE.md seit
             * Monaten als "teils Haenger exakt bei Anzeige von 'IP' in der Startsequenz"
             * fuehrt -- mit do_wait = 1 standen hier dieselben 13 bis 20 s Hauptloop-
             * Stillstand wie bei ticker_set (L204: 204 Iterationen a rund 93 ms bei
             * 32 Zeichen).
             *
             * Das Muster traegt an dieser Stelle, und zwar nachgesehen statt angenommen:
             * schedule_esp8266_messages() wird erst aus dem while (1) weiter unten gerufen,
             * timer2_init() laeuft lange davor, und der Zweig
             * "if (animation_flag) display_animation()" steckt im selben Loop -- der
             * periodische Zweig steht hier also bereits zur Verfuegung. Der Ticker wird
             * zudem erst NACH var_send_all_variables() gesetzt, dessen Warteschleife
             * display_animation() nicht ruft.
             *
             * buf ist lokal: display_set_ticker() kopiert den Text nach ticker_str, der
             * Zeiger darf nach der Rueckkehr ungueltig werden.
             *
             * Kein display_clock_flag hier, sonst malt display_clock() den Ticker sofort zu.
             */
            display_set_ticker (buf, 0);
            pending_weather_ticker_restore = 1;
            break;
        }
        case ESP8266_SYNCVARS:                                                  // ESP8266 bittet um den vollen Variablensatz
        {
            /* Nur vormerken, NICHT hier senden -- genau darin muss sich dieser Zweig vom
             * IPADDRESS-Zweig darueber unterscheiden.
             *
             * schedule_esp8266_messages() wird auch aus der Warteschleife von var_send_buf()
             * gerufen (vars.c). Ein direkter var_send_all_variables() liefe von dort
             * VERSCHACHTELT: Jedes der rund 194 Kommandos ginge auf die UART, ohne auf seine
             * Quittung zu warten -- var_send_buf() sendet VOR der Pruefung auf var_send_nested
             * und kehrt danach sofort zurueck. Die rund 194 zurueckkommenden Punkte quittierten
             * anschliessend der Reihe nach fremde Kommandos und ueberfuehren dabei den
             * 256-Byte-Empfangsring. Das ist der Schaden aus L102 (d=5148) -- also genau der,
             * gegen den dieser Zweig antritt.
             *
             * Der Fall ist nicht theoretisch: Der ESP wiederholt SYNCVARS bis zu dreimal im
             * Abstand von rund 10 s, und der zweite Versuch kann in die Warteschleife des
             * ersten fallen.
             *
             * Gesendet wird deshalb im Hauptloop, nach dem Muster des IR-Abzugs (siehe dort).
             * Kein Ticker und kein sprintf("IP %s") hier: Dass der IP-Lauftext NICHT ueber die
             * Uhr laeuft, ist der ganze Grund fuer Weg B (L231).
             */
            var_sync_pending = 1;
            break;
        }
        case ESP8266_ACCESSPOINT:
        {
            debug_log_message ("info: got ESP8266_ACCESSPOINT");
            break;
        }
        case ESP8266_MODE:
        {
            if (esp8266.mode == ESP8266_AP_MODE)
            {
                debug_log_message ("info: got ESP8266_AP_MODE");
            }
            else if (esp8266.mode == ESP8266_CLIENT_MODE)
            {
                debug_log_message ("info: got ESP8266_CLIENT_MODE");
            }
            else
            {
                debug_log_message ("info: got invalid ESP8266 mode");
            }
            break;
        }
        case ESP8266_FIRMWARE:
        {
            debug_log_message ("info: got ESP8266_FIRMWARE");
            break;
        }
        case ESP8266_WEATHER:
        {
            log_printf ("info: weather = %s\r\n", esp8266.u.weather);
            display_set_ticker ((unsigned char *) esp8266.u.weather, 0);
            pending_weather_ticker_restore = 1;
            break;
        }
        case ESP8266_WEATHER_FC:
        {
            log_printf ("info: weather forecast = %s\r\n", esp8266.u.weather);
            display_set_ticker ((unsigned char *) esp8266.u.weather, 0);
            pending_weather_ticker_restore = 1;
            break;
        }
        case ESP8266_WEATHER_ICON:
        {
            char tmpbuf[3];         // we must copy esp8266.u.weather, because union is used by icon transfer
            tmpbuf[0] = esp8266.u.weather[0];
            tmpbuf[1] = esp8266.u.weather[1];
            tmpbuf[2] = '\0';
            log_printf ("info: weather icon %s\r\n", tmpbuf);
            display_get_weather_icon (tmpbuf, icon_duration);
            break;
        }
        case ESP8266_WEATHER_FC_ICON:
        {
            char tmpbuf[3];         // we must copy esp8266.u.weather, because union is used by icon transfer
            tmpbuf[0] = esp8266.u.weather[0];
            tmpbuf[1] = esp8266.u.weather[1];
            tmpbuf[2] = '\0';
            log_printf ("info: weather forecast icon %s\r\n", tmpbuf);
            display_get_weather_icon (tmpbuf, icon_duration);
            break;
        }
        case ESP8266_ICONDATA:
        {
            if (display_read_icon () == 2)                                                          // icon read succefully
            {
                display.do_display_icon = 1;
                show_icon_stop_time = local_uptime + icon_duration;                                 // stop in x seconds
            }
            break;
        }
        case ESP8266_TABINFO:
        {
            tables_tabinfo (esp8266.u.tabinfo);
            break;
        }
        case ESP8266_TABILLU:
        {
            tables_tabillu (esp8266.u.tabillu);
            break;
        }
        case ESP8266_TABH:
        {
            tables_tabh (esp8266.u.tabh);
            break;
        }
#if WCLOCK24H == 1
        case ESP8266_TABT:
        {
            tables_tabt (esp8266.u.tabt);
            break;
        }
#endif
        case ESP8266_TABM:
        {
            tables_tabm (esp8266.u.tabm);

#if DSP_USE_TFTLED_RGB == 1
            if (tables.complete)
            {
                tftled_layout (0);                                                      // start transfer of led layout
            }
#endif
            break;
        }
#if DSP_USE_TFTLED_RGB == 1
        case ESP8266_DISP:
        {
            tftled_layout_get_line (esp8266.u.disp);
            break;
        }
#endif
        case ESP8266_OK:                                                        // nur noch der Punkt: Quittung eines var-Kommandos
        {
            var_send_busy = 0;
            break;
        }
        case ESP8266_NAK:                                                       // "!v": der ESP hat die Zeile abgelehnt
        {
            /* Die Pruefsumme der var-Zeile stimmte nicht, der Wert wurde NICHT gesetzt (A32,
             * Design 6.2/6.3). Ausgewertet wird das in var_send_buf(): Dort unterscheidet
             * got_answer (die Bruecke lebt) von applied (der Wert steht), und das Kommando wird
             * sofort zur Nachsendung vorgemerkt. Hier wird nur das Sendeflag aufgeloest -- wie
             * beim Punkt, denn das Kommando ist in beiden Faellen abgeschlossen.
             *
             * Bis Runde 2 dieser Spec ist dieser Zweig belegt wirkungslos: Kein ESP sendet "!v".
             */
            var_send_busy = 0;
            break;
        }
        case ESP8266_STATUS:                                                    // "OK ..." vom ESP: Statusmeldung, keine Quittung
        {
            /* Bewusst ohne Wirkung: var_send_busy bleibt stehen, damit eine der drei
             * unaufgeforderten OK-Zeilen des ESP (esp8266.c:299) nicht das Kommando
             * eines fremden Senders quittiert. Die Zeile ist bereits in esp8266.c:294-297
             * geloggt, es geht also nichts verloren.
             */
            break;
        }
#if 0 // yet not used
        case ESP8266_FILEOPEN:
        {
            break;
        }
        case ESP8266_FILEDATA:
        {
            break;
        }
        case ESP8266_FILECLOSE:
        {
            break;
        }
#endif
    }

    return msg_rtc;
}

static uint_fast8_t
do_play_dfplayer (uint_fast16_t cur_minute)
{
    uint_fast8_t    do_play = 0;

    if (dfplayer.silence_start < dfplayer.silence_stop)         // e.g. 08:00 - 11:00
    {
        if (cur_minute < dfplayer.silence_start || cur_minute > dfplayer.silence_stop)
        {
            do_play = 1;
        }
    }
    else if (dfplayer.silence_start > dfplayer.silence_stop)    // e.g. 22:00 - 06:00
    {
        if (cur_minute > dfplayer.silence_stop && cur_minute < dfplayer.silence_start)
        {
            do_play = 1;
        }
    }
    else                                                        // e.g. 00:00 - 00:00
    {
        do_play = 1;
    }

    return do_play;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Takt und Schwelle fuer das Senden des LDR-Rohwertes an den ESP (L69)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define LDR_RAW_SEND_DELTA          4                                   // Schwelle: darunter liegt das ADC-Rauschen, eine Handbewegung weit darueber
#define LDR_RAW_SEND_HOLD_POLLS     8                                   // 8 Pollzyklen a 250 ms = hoechstens alle 2 s ein Kommando ueber die Bruecke

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main function
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int
main (void)
{
    static uint_fast16_t    last_ldr_raw_value          = 0xFFFF;
    static uint_fast8_t     ldr_raw_send_hold           = 0;                // Sperrzaehler: Pollzyklen bis zum naechsten erlaubten Senden
    uint_fast8_t            esp8266_is_up               = 0;
    IRMP_DATA               irmp_data;
    uint32_t                stop_time;
    uint_fast8_t            cmd;
    uint_fast8_t            status_led_cnt              = 0;
    uint_fast8_t            time_changed                = 0;
    uint_fast16_t           ldr_raw_value;
    uint_fast8_t            ldr_value;
    uint32_t                ap_pressed                  = 0;
    uint32_t                wps_pressed                 = 0;                // timestamp when wps button has been pressed
    uint32_t                clockspeed                  = 100000;           // default clock speed for i2c bus
    uint_fast8_t            ds18xx_temperature_index    = 0xFF;
    uint_fast8_t            rtc_temperature_index       = 0xFF;

    SystemInit ();
    SystemCoreClockUpdate();                                                // needed for Nucleo board

#if defined (STM32F103)                                                     // disable JTAG to get back PB3, PB4, PA13, PA14, PA15
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);                    // turn on clock for the alternate function register
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);                // disable the JTAG, enable the SWJ interface
#endif

    gmain.hardware_configuration = 0;
    gmain.hardware_configuration |= HW_STM32_CONFIGURATION;
    gmain.hardware_configuration |= HW_CLOCK_CONFIGURATION;
    gmain.hardware_configuration |= HW_DISPLAY_CONFIGURATION;
    gmain.hardware_configuration |= HW_OSC_CONFIGURATION;

    strcpy (gmain.update_host, DEFAULT_UPDATE_HOST);
    strcpy (gmain.update_path, DEFAULT_UPDATE_PATH);

    log_init (115200);                                                      // initilize logger on uart

    log_message ("\r\nWelcome to WordClock Logger!");
    log_message ("----------------------------");
    log_reset_flags ();

    log_message ("irmp_init...");
    log_flush ();
    irmp_init ();                                                           // initialize IRMP
    log_message ("power_init...");
    log_flush ();
    power_init ();                                                          // initialize power port pin
    log_message ("power_on...");
    log_flush ();
    power_on ();                                                            // switch power on
    log_message ("delay_init...");
    log_flush ();
    delay_init (DELAY_RESOLUTION_5_US);                                     // initialize delay functions with granularity of 5 us
    log_message ("board_led_init...");
    log_flush ();
    board_led_init ();                                                      // initialize GPIO for board LED
    log_message ("button_init...");
    log_flush ();
    button_init ();                                                         // initialize GPIO for user button

#if DSP_USE_TFTLED_RGB
    log_message ("tftled_init...");
    tftled_init ();
#endif

    if (button_pressed ())                                                  // set ESP8266 into flash mode
    {
        board_led_on ();
        esp8266_flash ();
    }

    log_message ("timer2_init...");
    log_flush ();
    timer2_init ();                                                         // initialize timer2 for IRMP, DCF77, EEPROM etc.
    log_message ("wpsbutton_init...");
    log_flush ();
    wpsbutton_init ();                                                      // initialize GPIO for WPS button

    log_puts ("Version: ");
    log_flush ();
    log_message (VERSION);

#if defined (BLUEPILL_BOARD)
#  if defined (STM32F103C8)
    log_message ("Hardware: STM32F103 BluePill");
#  else
#    error unknown STM32 hardware
#  endif

#elif defined (NUCLEO_BOARD)
#  if defined (STM32F401RE)
    log_message ("Hardware: STM32F401RE Nucleo");
#  elif defined (STM32F411RE)
    log_message ("Hardware: STM32F411RE Nucleo");
#  elif defined (STM32F446RE)
    log_message ("Hardware: STM32F446RE Nucleo");
#  else
#    error unknown STM32 hardware
#  endif

#elif defined (BLACKPILL_BOARD)
#  if defined (STM32F401CC)
    log_message ("Hardware: STM32F401CC BlackPill");
#  elif defined (STM32F411CE)
    log_message ("Hardware: STM32F411CE BlackPill");
#  else
#    error unknown STM32 hardware
#  endif

#elif defined (BLACK_BOARD)
#  if defined (STM32F407VE)
    log_message ("Hardware: STM32F407VE Black");
#  else
#    error unknown STM32 hardware
#  endif

#else
#  error unknown STM32 hardware
#endif

    log_flush ();

#if WCLOCK24H == 1
    log_message ("Display: WC24h");
#else
    log_message ("Display: WC12h");
#endif
    log_flush ();

#if DSP_USE_WS2812_GRB == 1
    log_message ("LEDs: WS2812 GRB");
#elif DSP_USE_WS2812_RGB == 1
    log_message ("LEDs: WS2812 RGB");
#elif DSP_USE_APA102 == 1
    log_message ("LEDs: APA102");
#elif DSP_USE_SK6812_RGB == 1
    log_message ("LEDs: SK6812 RGB");
#elif DSP_USE_SK6812_RGBW == 1
    log_message ("LEDs: SK6812 RGBW");
#elif DSP_USE_TFTLED_RGB == 1
    log_message ("LEDs: TFTLED RGB");
#else
# error unknown LED type
#endif

    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    log_printf ("SYS:%lu H:%lu, P1:%lu, P2:%lu\r\n",
                      RCC_Clocks.SYSCLK_Frequency,
                      RCC_Clocks.HCLK_Frequency,   // AHB
                      RCC_Clocks.PCLK1_Frequency,  // APB1
                      RCC_Clocks.PCLK2_Frequency); // APB2
    log_flush ();

    rtc_init (clockspeed);                                                  // initialize I2C RTC
    eep_init (clockspeed);                                                  // initialize I2C EEPROM or SPI FLASH

    if (grtc.rtc_is_up)
    {
        log_message ("rtc is online");
    }
    else
    {
        log_message ("rtc is offline");
    }

    if (eep_is_up)
    {
        log_message ("eeprom/flash is online");
    }
    else
    {
        log_message ("eeprom/flash is offline");
    }

    if (! grtc.rtc_is_up || ! eep_is_up)
    {
        log_message ("trying to reduce I2C clock speed...");
        clockspeed /= 2;

        rtc_init (clockspeed);                                              // initialize I2C RTC again
        eep_init (clockspeed);                                              // initialize I2C EEPROM again

        if (grtc.rtc_is_up)
        {
            log_message ("rtc is now online");
        }
        else
        {
            log_message ("rtc remains offline");
        }

        if (eep_is_up)
        {
            log_message ("eeprom is now online");
        }
        else
        {
            log_message ("eeprom remains offine");
        }
    }

    if (grtc.rtc_is_up)
    {
        struct tm tm;

        if (rtc_get_date_time (& tm))
        {
            uint32_t z = (tm.tm_sec + 60 * tm.tm_min + 3600 * tm.tm_hour) & 0xFFFF;
            my_srand (z);
        }
    }

    display_init ();                                                        // initialize display
    overlay_init ();                                                        // initialize overlays
    night_init ();                                                          // initialize night time routines

    read_configuration_from_eep ();                                         // read configuration from EEPROM
    display_reset_led_states ();
    upgrade_eep_version ();                                                 // upgrade EEPROM to current version
    eep_flush (1);

#if DSP_USE_TFTLED_RGB == 1
    touch_init ();
#endif

    ldr_init ();                                                            // initialize LDR (ADC)
    dcf77_init ();                                                          // initialize DCF77
    alarm_init ();                                                          // initialize alarm routines
    temp_init ();                                                           // initialize DS18xx
    dfplayer_init ();                                                       // initialize DFPlayer

    ds3231_flag = 1;

    stop_time = uptime + 3;                                                 // wait 3 seconds for IR signal...
    display_set_status_or_minute_leds (1, 1, 1);                            // show white status or minute LEDs

    while (uptime < stop_time)
    {
        if (irmp_get_data (&irmp_data))                                     // got IR signal?
        {
            display_set_status_led (1, 0, 0);                               // yes, show red status LED
            delay_sec (1);                                                  // and wait 1 second
            (void) irmp_get_data (&irmp_data);                              // flush input of IRMP now
            display_set_status_led (0, 0, 0);                               // and switch status LED off

            debug_log_message ("calling IR learn function");
            if (remote_ir_learn ())                                         // learn IR commands
            {
                remote_ir_write_codes_to_eep ();                            // if successful, save them in EEPROM
            }
            break;                                                          // and break the loop
        }
    }

    display_set_status_or_minute_leds (0, 0, 0);                            // switch off status or minute LEDs

    esp8266_init ();

    if (ds18xx.is_up)
    {
        temp_read_temp_index ();
    }

    if (grtc.rtc_is_up)
    {
        rtc_get_temperature_index ();
    }

    if (grtc.rtc_is_up && rtc_get_date_time (&gmain.tm))
    {
        gmain.year    = gmain.tm.tm_year + 1900;
        gmain.month   = gmain.tm.tm_mon  + 1;
        gmain.mday    = gmain.tm.tm_mday;
        gmain.wday    = gmain.tm.tm_wday;
        gmain.hour    = gmain.tm.tm_hour;
        gmain.minute  = gmain.tm.tm_min;
        gmain.second  = gmain.tm.tm_sec;

        log_printf ("read rtc: %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                    wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                    gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
    }

    main_set_ambilight_clock_wait_cycles ();
    watchdog_init ();

    while (1)
    {
        watchdog_reload ();
        var_send_reload_budget_reset ();                                                // Nullpunkt des Reload-Budgets in var_send_buf(), siehe VAR_SEND_RELOAD_BUDGET_SEC

        /*---------------------------------------------------------------------------------------------------------------------------------
         * DIAGNOSE (specs/beobachtbarkeit): eine Zeile je Takt, am KOPF des Loops - nicht in einem flaggesteuerten Zweig,
         * denn genau dieser Zweig ist der Verdaechtige (haenger-2026-09-30.md).
         *
         *   l steigt, t steigt, u steigt  -> Zeitbasis lebt, der Ausfall liegt hinter den Flags
         *   l steigt, t steigt, u steht   -> die ISR laeuft, ihr Sekundenzweig nicht
         *   l steigt, t steht             -> die Zeitgeber-ISR selbst kommt nicht mehr dran
         *   keine Zeile                   -> Loop steht oder die Bruecke sendet nicht mehr; sichtbar an der Luecke in der Folgenummer
         *
         * Die drei Empfangsfelder messen DREI VERSCHIEDENE Dinge und duerfen nicht gegeneinander ausgespielt werden:
         *
         *   rx=<hoechststand>/<groesse>  der Grund, warum d=0 ueberhaupt etwas aussagt. Stuenden rx und d beide auf 0,
         *                                haette die Messung nicht stattgefunden. rx=250/256 d=0 ist dagegen eine echte Aussage.
         *   d=<verwuerfe>                Ueberlauf des SOFTWARE-Rings: die ISR lief, nur hat niemand rechtzeitig abgeholt.
         *                                d=0 heisst "der Ring hat nichts verworfen", NICHT "es ging kein Zeichen verloren".
         *   o=<overrun>                  Hardware-Ueberlauf des USART: die ISR selbst kam zu spaet, das Zeichen war schon im
         *                                Schieberegister ueberschrieben - genau der Zustand, den dieses Paket untersucht.
         *                                UNTERGRENZE: ein gesetztes ORE-Flag kann fuer mehrere verlorene Zeichen stehen.
         *
         * <groesse> liest der Treiber selbst aus (esp8266_uart_rxbuflen), es gibt hier keine zweite Kopie von UART_RXBUFLEN:
         * die ist je UART verschieden, dfplayer-uart.c setzt 32 statt 256.
         *
         * v=<timeouts>/<verschachtelt>  die beiden Verlustwege der Kommandobruecke, Stand seit dem Start. Beide Zaehler sind saettigend,
         *                               die Deutung ihres Standes steht bei ihrer Definition in vars.c (specs/bruecke, Design 3).
         *                               Im Ruhebetrieb bleibt das Feld auf v=0/0; tut es das nicht, kommt die Punkt-Quittung nicht an.
         *
         * Maximale Laenge der Zeile: 118 Zeichen, Grenze 119 (120 des ESP-Rings minus Kappungsmarke). Rest: 1 Zeichen - die Zeile ist voll.
         * Nachgerechnet, nicht uebernommen:
         *
         *   "diag " 5 + seq 10 + " l=" 3 + loop 10 + " t=" 3 + tick 10 + " u=" 3 + uptime 10            =  54
         *   + " r=" 3 + refresh 10 + " w=" 3 + dmawait 5 + " rx=" 4 + rxmax 4 + "/" 1 + rxbuflen 4      =  88
         *   + " d=" 3 + drops 5 + " o=" 3 + ore 5                                                       = 104
         *   + " v=" 3 + timeouts 5 + "/" 1 + verschachtelt 5                                            = 118
         *
         * Bis zum 03.10.2026 waren es 116: rx= trug damals zwei DREIstellige Werte, weil UART_RXBUFLEN 256 war. Seit der Ring auf 1024
         * steht (esp8266-uart.c), sind beide VIERstellig, und die Zeile ist bis auf ein Zeichen voll. KEIN Feld passt mehr dazu, und
         * auch keine weitere Stelle in einem bestehenden: Wer den Ring noch einmal vergroessert, muss zuerst Platz schaffen. Der Weg
         * dazu ist die Ringzeile des ESP von 120 auf 136 zu weiten (CMD_BUFFER_SIZE 128 -> 144) - das kostet 64 x 16 = 1024 Byte DRAM
         * von rund 12400 freien, einen ESP-Flash und einen zweiten Besitzer. Wer das Format anfasst, rechnet neu.
         *---------------------------------------------------------------------------------------------------------------------------------
         */
        static uint32_t     diag_seq            = 0;                                    // Folgenummer: eine Luecke im Ring ist sonst nicht von Ruhe zu unterscheiden
        static uint32_t     diag_loop_cnt       = 0;                                    // Hauptloop-Durchlaeufe seit dem Start
        static uint32_t     diag_last_tick      = 0;                                    // Stand von diag_tick_cnt bei der letzten REGULAEREN Zeile - nur die setzt ihn
        static uint32_t     diag_last_loop      = 0;                                    // Stand von diag_loop_cnt bei der letzten Zeile, Bezug des Rueckfalls
        static uint32_t     diag_regular_loop   = 0;                                    // Stand von diag_loop_cnt bei der letzten REGULAEREN Zeile, Bezug der Kalibrierung
        static uint32_t     diag_loop_budget    = DIAG_LOOP_BUDGET_START;               // Rueckfall-Schwelle, nach der ersten regulaeren Zeile selbstkalibriert
        static uint_fast8_t diag_armed          = 0;                                    // Nullpunkt gesetzt?
        uint32_t            diag_tick_now       = diag_tick_cnt;                        // volatile genau einmal lesen
        uint32_t            diag_tick_delta;
        uint32_t            diag_loop_delta;
        uint_fast8_t        diag_regular;

        diag_loop_cnt++;

        if (! diag_armed)                                                               // erste Runde: Nullpunkt setzen, sonst kalibrierte die erste Zeile auf einen einzigen Durchlauf
        {
            diag_armed         = 1;
            diag_last_tick     = diag_tick_now;
            diag_last_loop     = diag_loop_cnt;
            diag_regular_loop  = diag_loop_cnt;
        }

        diag_tick_delta = diag_tick_now - diag_last_tick;                               // vorzeichenlos: ein Umlauf von diag_tick_cnt stoert die Differenz nicht
        diag_loop_delta = diag_loop_cnt - diag_last_loop;
        diag_regular    = (diag_tick_delta >= DIAG_INTERVAL_TICKS) ? 1 : 0;             // regulaerer Takt ueber die Zeitgeber-ISR

        if (diag_regular || diag_loop_delta > diag_loop_budget)                         // zweiter Ausloeser: haengt weder an uptime noch an der ISR
        {
            if (diag_regular)                                                           // nur der regulaere Takt kalibriert - der Rueckfall zoege die Schwelle sonst nach unten
            {
                /*-------------------------------------------------------------------------------------------------------------
                 * WARUM Takt UND Kalibrierung ausschliesslich hier fortgeschrieben werden (A27 / BEFUNDE L182):
                 *
                 * Bis zum 04.10.2026 setzten BEIDE Zweige diag_last_tick und diag_last_loop. Damit verschob jede Rueckfallzeile
                 * auch den Bezugspunkt des regulaeren Takts: Feuerte der Rueckfall oefter als alle zehn Sekunden, erreichte
                 * diag_tick_delta nie mehr DIAG_INTERVAL_TICKS - es kam keine regulaere Zeile mehr, und damit kalibrierte nie
                 * wieder etwas. Das Budget blieb auf DIAG_LOOP_BUDGET_MIN stehen und die Flut hielt sich selbst am Leben.
                 * Am Geraet gemessen: morgens im Ruhebetrieb 60 Zeilen in 590 s (korrekter Zehn-Sekunden-Takt), abends nach
                 * einem Haenger 69,3 Zeilen JE SEKUNDE, dauerhaft, beendet erst durch einen STM-Reset. Der Ruhebetrieb kann
                 * diese Rueckkopplung nicht zeigen - sie braucht einen Stillstand als Ausloeser.
                 *
                 * Die Rueckfallzeile selbst bleibt unangetastet: Sie ist das einzige Signal dafuer, dass der zeitgesteuerte
                 * Zweig ausgesetzt hat (so wurde der Haenger aus L204 erkannt). Sie setzt nur noch diag_last_loop - ihren
                 * eigenen Bezug - und vergiftet die Kalibrierung nicht mehr.
                 *-------------------------------------------------------------------------------------------------------------
                 */
                if (diag_tick_delta < 2 * DIAG_INTERVAL_TICKS)                          // nur ein Fenster, das wirklich rund zehn Sekunden lang war, darf kalibrieren:
                {                                                                       // stand der Hauptloop dazwischen still, waere das Pensum zu klein gemessen
                    uint32_t diag_budget_new = 4 * (diag_loop_cnt - diag_regular_loop); // Vierfaches des Zehn-Sekunden-Pensums, seit der letzten REGULAEREN Zeile

                    if (diag_budget_new < diag_loop_budget / 2)                         // Daempfung: ein EINZELNES Fenster darf die Schwelle nicht einreissen. Nach
                    {                                                                   // einem Stillstand des Hauptloops (L204) enthaelt das Fenster den Stillstand,
                        diag_budget_new = diag_loop_budget / 2;                         // misst fast keine Durchlaeufe und riss sie sonst in EINEM Schritt bis auf die
                    }                                                                   // Untergrenze ein. Ein echter Rueckgang der Loopfrequenz setzt sich ueber
                                                                                        // mehrere Fenster trotzdem durch.
                    if (diag_budget_new < DIAG_LOOP_BUDGET_MIN)
                    {
                        diag_budget_new = DIAG_LOOP_BUDGET_MIN;
                    }

                    diag_loop_budget = diag_budget_new;
                }

                diag_last_tick    = diag_tick_now;                                      // NUR hier, sonst verhungert der regulaere Takt - siehe oben
                diag_regular_loop = diag_loop_cnt;
            }

            diag_last_loop = diag_loop_cnt;                                             // bei JEDER Zeile: sonst feuerte der Rueckfall in jedem weiteren Durchlauf
            diag_seq++;

            log_printf ("diag %lu l=%lu t=%lu u=%lu r=%lu w=%u rx=%u/%u d=%u o=%u v=%u/%u\r\n",
                        (unsigned long) diag_seq,
                        (unsigned long) diag_loop_cnt,
                        (unsigned long) diag_tick_now,
                        (unsigned long) uptime,
                        (unsigned long) led_get_refresh_cnt (),
                        (unsigned int)  led_get_dma_wait_cnt (),
                        (unsigned int)  esp8266_uart_rxmax (),
                        (unsigned int)  esp8266_uart_rxbuflen (),
                        (unsigned int)  esp8266_uart_rxdrops (),
                        (unsigned int)  esp8266_uart_rxore (),
                        (unsigned int)  var_send_timeout_count (),
                        (unsigned int)  var_send_nested_count ());
        }

        static uint_fast8_t icon_freeze_state = 0;                                      // DIAGNOSIS ONLY: do_display_icon freeze, see specs/bundle-guardrails-icon
        uint_fast8_t icon_freeze_now = (display.do_display_icon && ! display.display_power_is_on) ? 1 : 0;

        if (icon_freeze_now != icon_freeze_state)                                       // log on state change only, never per loop pass
        {
            icon_freeze_state = icon_freeze_now;
            log_printf ("icon_freeze: %s do_icon=%d power=%d astart=%d astop=%d ovl=%d\r\n",
                        icon_freeze_now ? "ENTER" : "LEAVE",
                        display.do_display_icon, display.display_power_is_on,
                        display.animation_start_flag, display.animation_stop_flag,
                        show_overlay_idx);
        }

        local_uptime = uptime;                                                          // cache volatile variable in local variable

        if (esp8266_is_up)                                                              // if user pressed user button, set ESP8266 to AP mode
        {
            if (local_uptime - ap_pressed > 5 && button_pressed ())
            {
                ap_pressed = local_uptime;                                              // debounce here
                log_message ("user button pressed: configuring esp8266 as access point");
                esp8266.is_online = 0;
                esp8266.ipaddress[0] = '\0';
                esp8266_accesspoint ("wordclock", "1234567890");
            }
            else if (local_uptime - wps_pressed > 5 && wpsbutton_pressed ())            // if user pressed wps button, send WPS command to ESP8266
            {
                wps_pressed = local_uptime;                                             // debounce here
                log_message ("wps button pressed: sending esp8266 wps command");
                esp8266.is_online = 0;
                esp8266.ipaddress[0] = '\0';
                esp8266_wps ();
            }
        }

        eep_flush (0);

#if DSP_USE_TFTLED_RGB == 1
        touch_check ();
#endif

        if (status_led_cnt)
        {
            status_led_cnt--;

            if (! status_led_cnt)
            {
                display_set_status_led (0, 0, 0);
            }
        }

        schedule_esp8266_messages ();

        /* Vollabgleich auf Anforderung des ESP (A6, Weg B; BEFUNDE.md L231, L255). Hier und nicht
         * im Zweig ESP8266_SYNCVARS, damit der Stoss von rund 194 Kommandos nie verschachtelt in
         * der Warteschleife von var_send_buf() liegt -- Begruendung dort.
         *
         * Das Flag wird VOR dem Senden geloescht: Trifft waehrenddessen ein weiteres SYNCVARS ein,
         * bekommt es seinen eigenen Durchlauf, statt verschluckt zu werden. Zwei Vollabgleiche
         * nacheinander sind harmlos, zwei ineinander nicht.
         *
         * Keine Pruefung auf esp8266.is_online: Das Flag hat der SYNCVARS-Zweig gerade selbst
         * gesetzt (esp8266.c), und genau dieses Setzen ist der Zweck der Uebung (L255).
         */
        if (var_sync_pending)
        {
            var_sync_pending = 0;
            var_send_all_variables ();
            log_message ("info: syncvars, configuration sent");
        }

        /* Abzug der IR-Codes: genau EIN Kommando je Hauptloop-Durchlauf. Der RPC
         * GET_IR_CODES_RPC_VAR setzt nur ir_export_idx auf 0, gesendet wird hier -- damit
         * zwischen zwei Kommandos garantiert der watchdog_reload() vom Kopf dieses Loops
         * liegt und keine neue Aufrufstelle noetig ist. Begruendung beim RPC-Zweig.
         */
        if (ir_export_idx < N_REMOTE_IR_CMDS)
        {
            if (esp8266.is_online)
            {
                uint_fast8_t    ir_idx = ir_export_idx;

                var_send_ir_code (ir_idx);

                /* var_send_buf() wartet auf die Quittung und ruft dabei
                 * schedule_esp8266_messages() selbst auf. Kam waehrenddessen ein neuer RPC,
                 * steht ir_export_idx bereits wieder auf 0 -- dann hier NICHT hochzaehlen,
                 * sonst fiele der erste Index des neuen Abzugs aus.
                 */
                if (ir_export_idx == ir_idx)
                {
                    ir_export_idx = ir_idx + 1;
                }
            }
            else
            {                                                       // Bruecke weg: Abzug abbrechen, statt den Zaehler haengen zu lassen
                ir_export_idx = N_REMOTE_IR_CMDS;
                log_message ("ir export: aborted, esp8266 offline");
            }
        }

        /* Nachsendung vorgemerkter var-Kommandos (A32, BEFUNDE.md L230): dasselbe Muster wie der
         * IR-Abzug darueber, und aus demselben Grund. var_send_buf() verwarf ein Kommando bisher
         * endgueltig, wenn die Quittung ausblieb -- genau so verschwinden im Nachsendestoss nach
         * einem ESP-Neustart einmalig angekuendigte Werte (HARDWARE_CONFIGURATION, Zeitzone,
         * Overlays; L42, L103, L205).
         *
         * Wiederholt wird NICHT in der Warteschleife, sondern hier: hoechstens ein Versuch je
         * Sekunde, damit zwischen zwei Versuchen garantiert der watchdog_reload() vom Kopf dieses
         * Loops liegt und keine neue Aufrufstelle noetig ist. Dreimal drei Sekunden in der
         * Warteschleife waeren neun Sekunden Stillstand und ein Rueckfall hinter A29/A31 gewesen;
         * so bleibt der laengste zusammenhaengende Stillstand bei den 3 s von heute.
         *
         * Nur bei lebender Bruecke: Ist der ESP offline, haette eine Nachsendung keine
         * Gegenstelle. Die Eintraege bleiben dann stehen und loesen sich von selbst auf, sobald
         * der ESP sich meldet -- der Vollabgleich raeumt jede Kennung beim Senden.
         */
        if (esp8266.is_online)
        {
            var_retry_drain ();
        }

        if (display.animation_stop_flag &&                                                                  // no animation running
            show_icon_stop_time == 0 &&                                                                     // no temperature display
            ! display.do_display_icon &&                                                                    // no icon display
            ldr_poll_brightness ())                                                                         // read LDR brightness
        {
            ldr_raw_value = ldr.ldr_raw_value;

            /* Der Rohwert geht unabhaengig von der Automatik raus. Frueher hing er mit am
             * display.automatic_brightness oben, dadurch zeigte die Kalibrierseite dauerhaft 0 --
             * also genau dann nichts, wenn der Nutzer Minimum und Maximum einstellt (L69).
             *
             * Gesendet wird hoechstens alle LDR_RAW_SEND_HOLD_POLLS Durchlaeufe, denn
             * var_send_buf() wartet auf die Quittung des ESP; anhaltende Bruecken- und
             * Requestlast ist der Verdacht aus L25. Der Takt ist kein neuer: er haengt am
             * vorhandenen Pollzyklus von 250 ms.
             */
            if (ldr_raw_send_hold > 0)                                                                      // Sendesperre laeuft noch?
            {
                ldr_raw_send_hold--;                                                                        // ja, nur herunterzaehlen
            }
            else if (esp8266.is_online &&
                     (ldr_raw_value + LDR_RAW_SEND_DELTA < last_ldr_raw_value ||
                      ldr_raw_value > last_ldr_raw_value + LDR_RAW_SEND_DELTA))                             // nennenswerte Aenderung?
            {
                debug_log_printf ("ldr: old raw brightnes: %d new raw brightness: %d\r\n", last_ldr_raw_value, ldr_raw_value);
                last_ldr_raw_value = ldr_raw_value;
                ldr_raw_send_hold  = LDR_RAW_SEND_HOLD_POLLS;                                               // fruehestens in 2 s wieder
                var_send_ldr_raw_value ();
            }

            if (display.automatic_brightness)                                                               // Helligkeit nur bei aktiver Automatik nachfuehren
            {
                ldr_value = ldr.ldr_value;                                                                  // ldr_value is 0...31

                if (ldr_value + 1 < last_ldr_value || ldr_value > last_ldr_value + 1)                       // difference greater than 2
                {
                    last_ldr_value = ldr_value;                                                             // store value 0...31
                    ldr_value /= 2;                                                                         // set ldr_value to 0...15
                    debug_log_printf ("ldr: old brightnes: %d new brightness: %d\r\n", last_ldr_value / 2, ldr_value);
                    display_set_display_brightness (ldr_value, FALSE, FALSE);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                }
            }
        }

        if (!esp8266_is_up)                                                 // esp8266 up yet?
        {
            if (esp8266.is_up)
            {
                esp8266_is_up = 1;
                log_message ("esp8266 now up");
            }
        }
        else
        {                                                                   // esp8266 is up...
            if (! esp8266_is_online)                                        // but not online yet...
            {
                if (esp8266.is_online)                                      // now online?
                {
                    esp8266_is_online = 1;
                    log_message ("esp8266 now online");
                    net_time_flag = 1;
                }
            }
        }

        if (dcf77_time(&gmain.tm))
        {
            display_set_status_led (1, 1, 0);                               // got DCF77 time, light yellow = green + red LED

            status_led_cnt = 50;

            if (grtc.rtc_is_up)
            {
                rtc_set_date_time (&gmain.tm);
            }

            if (gmain.hour != (uint_fast8_t) gmain.tm.tm_hour || gmain.minute != (uint_fast8_t) gmain.tm.tm_min)
            {
                if (esp8266.is_online)
                {
                    var_send_tm ();
                }
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }

            gmain.year    = gmain.tm.tm_year + 1900;
            gmain.month   = gmain.tm.tm_mon  + 1;
            gmain.mday    = gmain.tm.tm_mday;
            gmain.wday    = gmain.tm.tm_wday;
            gmain.hour    = gmain.tm.tm_hour;
            gmain.minute  = gmain.tm.tm_min;
            gmain.second  = gmain.tm.tm_sec;

            log_printf ("dcf77: %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                         wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                         gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
        }

        if (ds3231_flag)
        {
            if (grtc.rtc_is_up && rtc_get_date_time (&gmain.tm))
            {
                if (gmain.hour != (uint_fast8_t) gmain.tm.tm_hour || gmain.minute != (uint_fast8_t) gmain.tm.tm_min)
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                }

                gmain.year    = gmain.tm.tm_year + 1900;
                gmain.month   = gmain.tm.tm_mon  + 1;
                gmain.mday    = gmain.tm.tm_mday;
                gmain.wday    = gmain.tm.tm_wday;
                gmain.hour    = gmain.tm.tm_hour;
                gmain.minute  = gmain.tm.tm_min;
                gmain.second  = gmain.tm.tm_sec;

                log_printf ("read rtc: %4d-%02d-%02d %02d:%02d:%02d\r\n",
                            gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                            gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
            }

            ds3231_flag = 0;
        }

        if (display.animation_stop_flag && ! display.do_display_icon && ldr_conversion_flag)                // auch ohne Automatik messen, siehe L69
        {
            ldr_start_conversion ();
            ldr_conversion_flag = 0;
        }

        if (net_time_flag)
        {
            if (esp8266.is_online)
            {
                display_set_status_led (0, 0, 1);                           // light blue status LED
                status_led_cnt = STATUS_LED_FLASH_TIME;
                timeserver_start_timeserver_request ();                     // start a timeserver request, answer follows...
            }

            net_time_flag = 0;
            net_time_countdown = 3800;                                      // next net time after 3800 sec
        }

        if (show_time_flag)                                                 // set every full minute
        {
            show_time_flag = 0;

            gmain.tm.tm_year = gmain.year - 1900;
            gmain.tm.tm_mon  = gmain.month - 1;
            gmain.tm.tm_mday = gmain.mday;
            gmain.tm.tm_hour = gmain.hour;
            gmain.tm.tm_min  = gmain.minute;
            gmain.tm.tm_sec  = gmain.second;
            gmain.tm.tm_wday = gmain.wday;

            if (esp8266.is_online)
            {
                var_send_tm ();
            }

            // display_clock_flag = 0;                                      // don't reset flag

            if (night_check_night_times (0, display.display_power_is_on, gmain.wday, gmain.hour * 60 + gmain.minute))
            {
                if (display.display_power_is_on)                            // display currently on
                {                                                           // switch off display AND ambilight
                    display_clock_flag = set_display_power (FALSE, TRUE);
                }
                else                                                        // display currently off
                {                                                           // switch on display, but NOT ambilight
                    display_clock_flag = set_display_power (TRUE, FALSE);
                }

                log_printf ("Found Timer: %s at %02d:%02d\r\n", display.display_power_is_on ? "on" : "off", gmain.hour, gmain.minute);
            }

            if (night_check_night_times (1, display.ambilight_power_is_on, gmain.wday, gmain.hour * 60 + gmain.minute))
            {
                display_set_ambilight_power (! display.ambilight_power_is_on);
                log_printf ("Found Timer: ambilight %s at %02d:%02d\r\n", display.ambilight_power_is_on ? "on" : "off", gmain.hour, gmain.minute);
            }

            if (display_clock_flag == 0)                                    // no night time found
            {
#if WCLOCK24H == 1
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
#else
                if (gmain.minute % 5)
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_MINUTES; // only update minute LEDs
                }
                else
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                }
#endif
            }

            log_printf ("show_time: flags=0x%02x p=%d a=%d %02d:%02d\r\n",
                        display_clock_flag,
                        display.display_power_is_on,
                        display.ambilight_power_is_on,
                        gmain.hour,
                        gmain.minute);
        }

        if (half_minute_flag)
        {
            half_minute_flag = 0;

            if (display.display_power_is_on)
            {
                if (net_time_countdown > 3800 - 30)         // waiting for timeserver response?
                {                                           // no, else don't communicate with ESP8266 (icon vs. timeserver response)
                    log_printf ("net_time_countdown = %d, don't check overlays\r\n", net_time_countdown);
                }
                else
                {
                    static uint_fast16_t    last_year;
                    uint_fast16_t           mmdd;
                    uint_fast8_t            overlay_idx;
                    uint_fast8_t            overlay_max_interval;

                    mmdd                    = (gmain.month << 8) + gmain.mday;
                    overlay_max_interval    = 0;

                    if (last_year != gmain.year)
                    {
                        uint_fast8_t    i;

                        last_year = gmain.year;

                        for (i = 0; i < overlay.n_overlays; i++)
                        {
                            overlay_calc_dates (i, last_year);
                        }
                    }

                    // overlay step 1: find maximum overlay interval which matches current minute
                    // overlays with matching dates have higher priority
                    for (overlay_idx = 0; overlay_idx < overlay.n_overlays; overlay_idx++)
                    {
                        if (overlay.overlays[overlay_idx].flags & OVERLAY_FLAG_ACTIVE)
                        {
                            if ((gmain.minute % overlay.overlays[overlay_idx].interval) == 0)
                            {
                                if (overlay.overlays[overlay_idx].date_start == 0 ||
                                    (mmdd >= overlay.overlays[overlay_idx].date_start && mmdd <= overlay.overlays[overlay_idx].date_end))
                                {
                                    if (overlay.overlays[overlay_idx].interval > overlay_max_interval)
                                    {
                                        overlay_max_interval = overlay.overlays[overlay_idx].interval;
                                        show_overlay_idx = overlay_idx;
                                    }
                                    else if (overlay.overlays[overlay_idx].interval == overlay_max_interval)
                                    {
                                        if (overlay.overlays[overlay_idx].date_start != 0)
                                        {
                                            overlay_max_interval = overlay.overlays[overlay_idx].interval;
                                            show_overlay_idx = overlay_idx;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (measure_temperature_flag)
        {
            measure_temperature_flag = 0;

            if (ds18xx.is_up)
            {
                temp_start_conversion (read_temperature_flag);          // start conversion, wait if read_temperature_flag is also set
            }
        }

        if (read_temperature_flag)
        {
            read_temperature_flag = 0;

            if (ds18xx.is_up)
            {
                ds18xx_temperature_index = temp_read_temp_index ();
                log_printf ("DS18xxx temperature: %d%s\r\n", ds18xx_temperature_index / 2, (ds18xx_temperature_index % 2) ? ".5" : "");

                if (esp8266.is_online)
                {
                    var_send_ds18xx_temp_index ();
                }
            }
            else
            {
                ds18xx_temperature_index = 0xFF;
            }
        }

        if (read_rtc_temperature_flag)
        {
            read_rtc_temperature_flag = 0;

            if (grtc.rtc_is_up)
            {
                rtc_temperature_index = rtc_get_temperature_index ();
                log_printf ("RTC temperature: %d%s\r\n", rtc_temperature_index / 2, (rtc_temperature_index % 2) ? ".5" : "");

                if (esp8266.is_online)
                {
                    var_send_rtc_temp_index ();
                }
            }
            else
            {
                rtc_temperature_index = 0xFF;
            }
        }

        if (show_overlay_idx < MAX_OVERLAYS)                                                                // overlay to show?
        {
            switch (overlay.overlays[show_overlay_idx].type)
            {
                case OVERLAY_TYPE_ICON:                                                                     // icon
                {
                    log_printf ("overlay icon %s\r\n", overlay.overlays[show_overlay_idx].text);
                    display_get_icon (overlay.overlays[show_overlay_idx].text, overlay.overlays[show_overlay_idx].duration);
                    icon_duration = overlay.overlays[show_overlay_idx].duration;
                    break;
                }
                case OVERLAY_TYPE_DATE:                                                                     // date
                {
                    show_date = 1;
                    log_message ("overlay date");
                    break;
                }
                case OVERLAY_TYPE_TEMPERATURE:                                                              // temperature
                {
                    show_temperature = 1;
                    log_message ("overlay temperature");
                    break;
                }
                case OVERLAY_TYPE_TEMPERATURE_DIGITS:                                                       // temperature digits
                {
                    show_temperature_digits = 1;
                    log_message ("overlay temperature digits");
                    break;
                }
                case OVERLAY_TYPE_WEATHER_ICON:                                                             // weather icon
                {
                    weather_query (WEATHER_QUERY_ID_ICON);
                    icon_duration = overlay.overlays[show_overlay_idx].duration;
                    log_message ("overlay weather icon");
                    break;
                }
                case OVERLAY_TYPE_WEATHER_FC_ICON:                                                          // weather icon
                {
                    weather_query (WEATHER_QUERY_ID_ICON_FC);
                    icon_duration = overlay.overlays[show_overlay_idx].duration;
                    log_message ("overlay weather forecast icon");
                    break;
                }
                case OVERLAY_TYPE_WEATHER:                                                                  // weather
                {
                    weather_query (WEATHER_QUERY_ID_TEXT);
                    log_message ("overlay weather");
                    break;
                }
                case OVERLAY_TYPE_WEATHER_FC:                                                               // weather forecast
                {
                    weather_query (WEATHER_QUERY_ID_TEXT_FC);
                    log_message ("overlay weather forecast");
                    break;
                }
                case OVERLAY_TYPE_TICKER:                                                                   // ticker
                {
                    log_printf ("overlay ticker: '%s'\r\n", overlay.overlays[show_overlay_idx].text);
                    /* Nicht blockierend (BEFUNDE.md L211): Ein Ticker-Overlay feuert
                     * periodisch und ohne jede Benutzerhandlung; mit do_wait = 1 stand der
                     * Hauptloop dabei jedes Mal 13 bis 20 s still (L204: 204 Iterationen
                     * a rund 93 ms bei 32 Zeichen).
                     *
                     * Der Aufrufer ist auf den abgeschlossenen Ticker nicht angewiesen:
                     * nach dem switch wird nur show_overlay_idx zurueckgesetzt, und je
                     * Minutenwechsel feuert hoechstens ein Overlay -- ein noch laufender
                     * Ticker kann sich mit dem naechsten also nicht ueberschneiden.
                     *
                     * Kein display_clock_flag hier, sonst malt display_clock() den Ticker im
                     * naechsten Durchlauf sofort zu. Den Restore zieht die Bedingung um
                     * pending_weather_ticker_restore nach.
                     */
                    display_set_ticker ((unsigned char *) overlay.overlays[show_overlay_idx].text, 0);
                    pending_weather_ticker_restore = 1;
                    break;
                }
                case OVERLAY_TYPE_MP3:                                                                      // mp3
                {
                    char *  p;
                    int     folder;
                    int     track;

                    p = strchr (overlay.overlays[show_overlay_idx].text, '/');

                    if (p)
                    {
                        uint_fast16_t   cur_minute = 60 * gmain.hour + gmain.minute;

                        folder = atoi (overlay.overlays[show_overlay_idx].text);
                        track  = atoi (p + 1);

                        if (do_play_dfplayer (cur_minute))
                        {
                            log_printf ("overlay mp3: %02d/%03d\r\n", folder, track);
                            dfplayer_play_folder (folder, track);
                        }
                    }
                    else
                    {
                        log_printf ("overlay mp3: invalid format: %s\r\n", overlay.overlays[show_overlay_idx].text);
                    }
                    break;
                }
            }

            show_overlay_idx = MAX_OVERLAYS;
        }
        else if (show_icon_stop_time > 0)
        {
            if (show_icon_stop_time == 0xFFFFFFFF)                                      // waiting for end of animation?
            {
                 if (! display.animation_start_flag && display.animation_stop_flag)     // animation running?
                 {                                                                      // no, ...
                    show_icon_stop_time = local_uptime + 5;                             // show temperature 5 seconds long
                 }
            }
            else if (local_uptime >= show_icon_stop_time)                               // 5 seconds gone?
            {                                                                           // yes, ...
                show_icon_stop_time = 0;                                                // reset flag
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;                     // and force update of display
            }
        }

        if (show_temperature)
        {
            show_temperature = 0;

            if (ds18xx_temperature_index != 0xFF)
            {
                display_temperature (ds18xx_temperature_index);
                show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
            }
            else if (rtc_temperature_index != 0xFF)
            {
                display_temperature (rtc_temperature_index);
                show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
            }
        }
        else if (show_temperature_digits)
        {
            show_temperature_digits = 0;

            if (ds18xx_temperature_index != 0xFF)
            {
                display_temperature_digits (ds18xx_temperature_index);
                show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
            }
            else if (rtc_temperature_index != 0xFF)
            {
                display_temperature_digits (rtc_temperature_index);
                show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
            }
        }

        if (show_date)
        {
            char        datebuf[MAX_DATE_TICKER_LEN + 1];
            uint8_t *   p;
            int         datebuflen = 0;

            show_date = 0;

            for (p = display.date_ticker_format; *p && datebuflen < MAX_DATE_TICKER_LEN - 4; p++)
            {
                switch (*p)
                {
                    case 'd':
                        sprintf (datebuf + datebuflen, "%d", gmain.tm.tm_mday);
                        break;
                    case 'D':
                        sprintf (datebuf + datebuflen, "%02d", gmain.tm.tm_mday);
                        break;
                    case 'm':
                        sprintf (datebuf + datebuflen, "%d", gmain.tm.tm_mon + 1);
                        break;
                    case 'M':
                        sprintf (datebuf + datebuflen, "%02d", gmain.tm.tm_mon + 1);
                        break;
                    case 'y':
                        sprintf (datebuf + datebuflen, "%02d", gmain.tm.tm_year - 100);
                        break;
                    case 'Y':
                        sprintf (datebuf + datebuflen, "%d", gmain.tm.tm_year + 1900);
                        break;
                    default:
                        datebuf[datebuflen] = *p;
                        datebuf[datebuflen + 1] = '\0';
                        break;
                }
                datebuflen = strlen (datebuf);
            }

            /* Nicht blockierend (BEFUNDE.md L216): Auf dieser Uhr ist overlay[0] aktiv und
             * hat Typ 2 (DATUM). Der Pfad OVERLAY_TYPE_DATE -> show_date = 1 -> hierher lief
             * mit do_wait = 1 bei rund 10 Zeichen Datumstext auf (10 + 2) x 6 = 72
             * Iterationen a rund 93 ms hinaus: rund 7 s Hauptloop-Stillstand, periodisch und
             * ohne jede Benutzerhandlung. Das ist die Erklaerung fuer "haengt sporadisch von
             * selbst" -- und bei laengerem Datumsformat oder groesserem ticker_deceleration
             * kam es den 20 s des Watchdogs gefaehrlich nahe.
             *
             * datebuf ist lokal: display_set_ticker() kopiert den Text nach ticker_str, der
             * Zeiger darf nach der Rueckkehr ungueltig werden. Nach dieser Stelle folgt nur
             * noch die Restore-Bedingung -- auf den fertigen Ticker ist niemand angewiesen.
             *
             * Kein display_clock_flag hier, sonst malt display_clock() den Ticker im
             * naechsten Durchlauf sofort zu.
             */
            display_set_ticker ((unsigned char *) datebuf, 0);
            pending_weather_ticker_restore = 1;
        }

        if (pending_weather_ticker_restore &&
            ! display_ticker_active () &&
            ! display.do_display_icon &&
            show_icon_stop_time == 0 &&
            show_overlay_idx >= MAX_OVERLAYS)
        {
            if (! display_clock_flag)                                                   // only consume the flag if the restore can take effect
            {
                pending_weather_ticker_restore = 0;                                     // else keep it pending and retry next iteration
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }
        }

        if (display_clock_flag)                                                         // refresh display (time/mode changed)
        {
            debug_log_message ("update display");

#if WCLOCK24H == 1
            if (display.display_mode == tables.modes_count - 1)                         // temperature
            {
                uint_fast8_t temperature_index;

                if (ds18xx.is_up)
                {
                    temperature_index = temp_read_temp_index ();
                    log_printf ("got temperature from DS18xxx: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                }
                else if (grtc.rtc_is_up)
                {
                    temperature_index = rtc_get_temperature_index ();
                    log_printf ("got temperature from RTC: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                }
                else
                {
                    temperature_index = 0x00;
                    debug_log_message ("no temperature available");
                }

                if (temperature_index >= 20)
                {
                    display_clock (0, temperature_index - 20, display_clock_flag);      // show temperature
                }
            }
            else
            {
                /* Zwei Zeilen a rund 70 Byte um JEDEN display_clock()-Aufruf, und der Zweig laeuft
                 * nicht nur zum Minutenwechsel, sondern bei jedem gesetzten display_clock_flag --
                 * beim Speichern der Dimmkurve sind das 16 Aufrufe am Stueck. Jede Logzeile geht
                 * ueber log_vprintf() auch auf die ESP-Bruecke und verzoegert dort das Abholen
                 * (BEFUNDE.md, L144). Gekuerzt auf das, was gelesen wird; power und ambi stehen
                 * nur noch in der Rueckkehrzeile, weil sie sich allenfalls DORT geaendert haben.
                 *
                 * Beide Marker bleiben WORTGLEICH erhalten: tools/logger/README.md deutet die
                 * Luecke zwischen "main: call display_clock" und "main: display_clock returned"
                 * als ueberstandene Blockade. Wer sie kuerzt, nimmt dem Werkzeug seine Aussage.
                 */
                log_printf ("main: call display_clock flags=0x%02x\r\n", display_clock_flag);
                display_clock (gmain.hour, gmain.minute, display_clock_flag);           // show new time
                log_printf ("main: display_clock returned flags=0x%02x p=%d a=%d\r\n",
                            display_clock_flag,
                            display.display_power_is_on,
                            display.ambilight_power_is_on);
            }
#else
            log_printf ("main: call display_clock flags=0x%02x\r\n", display_clock_flag);
            display_clock (gmain.hour, gmain.minute, display_clock_flag);               // show new time
            log_printf ("main: display_clock returned flags=0x%02x p=%d a=%d\r\n",
                        display_clock_flag,
                        display.display_power_is_on,
                        display.ambilight_power_is_on);
#endif
            display_clock_flag = DISPLAY_CLOCK_FLAG_NONE;
        }

        if (animation_flag)
        {
            animation_flag = 0;
            display_animation ();
        }

        if (ambilight_clock_tick)                                                       // set 20 times per ambilight LED in clock mode
        {
            ambilight_clock_tick = 0;
            display_seconds (ambilight_clock_led_idx);
        }

        if (seconds_flag)                                                               // currently not used
        {
            seconds_flag = 0;
        }

        cmd = remote_ir_get_cmd ();                                                     // get IR command

        if (cmd != REMOTE_IR_CMD_INVALID)                                               // got IR command, light green LED
        {
            display_set_status_led (1, 0, 0);
            status_led_cnt = STATUS_LED_FLASH_TIME;
        }

        switch (cmd)
        {
            case REMOTE_IR_CMD_POWER:
            {
                display_clock_flag = set_display_power (! display.display_power_is_on, TRUE);
                debug_log_printf ("IRMP: POWER key, display_clock_flag=0x%02x\r\n", display_clock_flag);
                break;
            }

            case REMOTE_IR_CMD_OK:
            {
                debug_log_message ("IRMP: OK key, not used anymore");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_DISPLAY_MODE:                                  // decrement display mode
            {
                display_decrement_display_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: decrement display mode");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_DISPLAY_MODE:                                  // increment display mode
            {
                display_increment_display_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: increment display mode");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_ANIMATION_MODE:                                // decrement display mode
            {
                display_decrement_animation_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: decrement animation mode");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_ANIMATION_MODE:                                // increment display mode
            {
                display_increment_animation_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: increment animation mode");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_HOUR:                                          // decrement hour
            {
                if (gmain.hour > 0)
                {
                    gmain.hour--;
                }
                else
                {
                    gmain.hour = 23;
                }

                gmain.second        = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: decrement hour");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_HOUR:                                          // increment hour
            {
                if (gmain.hour < 23)
                {
                     gmain.hour++;
                }
                else
                {
                    gmain.hour =  0;
                }

                gmain.second        = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: increment hour");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_MINUTE:                                        // decrement minute
            {
                if (gmain.minute > 0)
                {
                    gmain.minute--;
                }
                else
                {
                    gmain.minute = 59;
                }

                gmain.second        = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: decrement minute");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_MINUTE:                                        // increment minute
            {
                if (gmain.minute < 59)
                {
                    gmain.minute++;
                }
                else
                {
                    gmain.minute = 0;
                }

                gmain.second        = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: increment minute");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_RED:                                // decrement red brightness
            {
                display_decrement_display_color_red (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement red brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_RED:                                // increment red brightness
            {
                display_increment_display_color_red (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment red brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_GREEN:                              // decrement green brightness
            {
                display_decrement_display_color_green (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement green brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_GREEN:                              // increment green brightness
            {
                display_increment_display_color_green (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment green brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_BLUE:                               // decrement blue brightness
            {
                display_decrement_display_color_blue (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement blue brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_BLUE:                               // increment blue brightness
            {
                display_increment_display_color_blue (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment blue brightness");
                break;
            }

            case REMOTE_IR_CMD_AUTO_BRIGHTNESS_CONTROL:                                 // toggle auto brightness
            {
                uint_fast8_t new_auto = display.automatic_brightness;
                new_auto = ! new_auto;
                last_ldr_value = 0xFF;
                display_set_automatic_brightness (new_auto, TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: toggle auto brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS:                                    // decrement brightness
            {
                if (display.automatic_brightness)
                {
                    last_ldr_value = 0xFF;
                    display_set_automatic_brightness (0, TRUE);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                }

                display_decrement_display_brightness (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS:                                    // increment brightness
            {
                if (display.automatic_brightness)
                {
                    last_ldr_value = 0xFF;
                    display_set_automatic_brightness (0, TRUE);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                }

                display_increment_display_brightness (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment brightness");
                break;
            }

            case REMOTE_IR_CMD_GET_TEMPERATURE:                                         // get temperature
            {
                show_temperature = 1;
                debug_log_message ("IRMP: get temperature");
                break;
            }

            default:
            {
                break;
            }
        }

        if (time_changed)
        {
            if (grtc.rtc_is_up)
            {
                gmain.tm.tm_year = gmain.year - 1900;
                gmain.tm.tm_mon  = gmain.month - 1;
                gmain.tm.tm_mday = gmain.mday;
                gmain.tm.tm_hour = gmain.hour;
                gmain.tm.tm_min  = gmain.minute;
                gmain.tm.tm_sec  = gmain.second;
                gmain.tm.tm_wday = gmain.wday;
                rtc_set_date_time (&gmain.tm);
            }

            time_changed = 0;
        }

        dfplayer_read_message ();

        if (dfplayer.is_up)
        {
            static uint_fast8_t last_dfplayer_is_up;
            static uint_fast8_t last_minute = 0xFF;

            if (! last_dfplayer_is_up)
            {
                last_dfplayer_is_up = 1;
                log_message ("dfplayer is online");
            }

            if (last_minute != gmain.minute)
            {
                uint_fast16_t   cur_minute = 60 * gmain.hour + gmain.minute;
                uint_fast8_t    alarm_idx;

                alarm_idx = alarm_check_alarm_times (gmain.wday, cur_minute);

                if (alarm_idx > 0)
                {
                    log_printf ("%d %02d:%02d alarm_idx = %d\r\n", gmain.wday, gmain.hour, gmain.minute, alarm_idx);
                    dfplayer_play_folder (ALARM_FOLDER, alarm_idx);
                }
                else if (dfplayer.mode != DFPLAYER_MODE_NONE)
                {
                    if (do_play_dfplayer (cur_minute))
                    {
                        if (dfplayer.mode == DFPLAYER_MODE_BELL)
                        {
                            uint_fast8_t track = 0xFF;

                            if (gmain.minute == 0)
                            {
                                if (gmain.hour >= 12)
                                {
                                    if (gmain.hour == 12)
                                    {
                                        track = DFPLAYER_TRACK_HOUR12;
                                    }
                                    else
                                    {
                                        track = gmain.hour - 12;
                                    }
                                }
                                else
                                {
                                    track = gmain.hour;
                                }
                            }
                            else if (gmain.minute == 15)
                            {
                                if (dfplayer.bell_flags & DFPLAYER_MODE_BELL_FLAG_15)
                                {
                                    track = DFPLAYER_TRACK_HOUR_15M;
                                }
                            }
                            else if (gmain.minute == 30)
                            {
                                if (dfplayer.bell_flags & DFPLAYER_MODE_BELL_FLAG_30)
                                {
                                    track = DFPLAYER_TRACK_HOUR_30M;
                                }
                            }
                            else if (gmain.minute == 45)
                            {
                                if (dfplayer.bell_flags & DFPLAYER_MODE_BELL_FLAG_45)
                                {
                                    track = DFPLAYER_TRACK_HOUR_45M;
                                }
                            }

                            if (track != 0xFF)
                            {
                                dfplayer_play_folder (BELL_FOLDER, track);
                            }
                        }
                        else if (dfplayer.mode == DFPLAYER_MODE_SPEAK)
                        {
                            if (dfplayer.speak_cycle > 0 && gmain.minute % dfplayer.speak_cycle == 0)
                            {
#if WCLOCK24H == 1
                                if (display.display_mode == tables.modes_count - 1)                     // temperature
                                {
                                    uint_fast8_t temperature_index;

                                    if (ds18xx.is_up)
                                    {
                                        temperature_index = temp_read_temp_index ();
                                        log_printf ("got temperature from DS18xxx: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                                    }
                                    else if (grtc.rtc_is_up)
                                    {
                                        temperature_index = rtc_get_temperature_index ();
                                        log_printf ("got temperature from RTC: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                                    }
                                    else
                                    {
                                        temperature_index = 0x00;
                                        debug_log_message ("no temperature available");
                                    }

                                    if (temperature_index >= 20)
                                    {
                                        speak (0, temperature_index - 20);                              // speak temperature
                                    }
                                }
                                else
                                {
                                    speak (gmain.hour, gmain.minute);
                                }
#else
                                speak (gmain.hour, gmain.minute);
#endif
                            }
                        }
                    }
                }

                last_minute = gmain.minute;
            }
        }
    }

    return 0;
}
