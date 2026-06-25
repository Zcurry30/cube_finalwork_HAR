/**
  ******************************************************************************
  * @file    har_io_uart.h
  * @brief   USART1 I/O for Apollo STM32F767 board
  *
  *          Uses the onboard CH340G USB-UART bridge wired to PA9(TX)/PA10(RX).
  *          Connect the grey Mini-USB cable to the "USB UART" port on the
  *          bottom board — it appears as a COM port on the PC.
  *
  *          Call HAR_IO_UART_Init() once after MX_USART3_UART_Init() to
  *          enable serial input/output for the HAR application.
  ******************************************************************************
  */

#ifndef __HAR_IO_UART_H
#define __HAR_IO_UART_H

#ifdef __cplusplus
extern "C" {
#endif

void HAR_IO_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAR_IO_UART_H */
