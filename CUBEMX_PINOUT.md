# STM32F767IGT6 CubeMX Pinout

This project does not use a touch screen. Recognition results are sent through
UART.

## Enabled Interfaces

| Interface | Function | Pins |
| --- | --- | --- |
| USART3 | UART input/output | PD8 TX, PD9 RX |
| SDMMC1 | SD card 4-bit bus | PC8 D0, PC9 D1, PC10 D2, PC11 D3, PC12 CK, PD2 CMD |
| USB OTG FS | USB CDC device | PA9 VBUS, PA11 DM, PA12 DP |
| ETH | RMII Ethernet | PA1 REF_CLK, PA2 MDIO, PA7 CRS_DV, PC1 MDC, PC4 RXD0, PC5 RXD1, PB11 TX_EN, PB12 TXD0, PB13 TXD1 |
| SWD | Debug | PA13 SWDIO, PA14 SWCLK |
| RCC | HSE | PH0 OSC_IN, PH1 OSC_OUT |

## CubeMX Middleware

The `.ioc` is prepared for:

- FATFS on SD card
- USB_DEVICE CDC FS
- LwIP with DHCP
- X-CUBE-AI

Open `finalwork.ioc` in STM32CubeMX and run code generation after installing
the matching STM32CubeF7 firmware package and X-CUBE-AI pack.

## Board Notes

The pinout is a generic STM32F767IGT6 LQFP176 assignment. Check the final
schematic before manufacturing or wiring a custom board:

- Ethernet RMII requires an external PHY and a valid 50 MHz reference clock.
- SD card lines need pull-ups according to the SD card socket design.
- USB FS needs the correct connector, ESD protection, VBUS sensing, and power
  design.
- USART3 can be connected to a USB-to-UART adapter or ST-Link VCP if the board
  routes PD8/PD9 that way.
