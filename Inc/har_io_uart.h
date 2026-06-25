#ifndef __HAR_IO_UART_H
#define __HAR_IO_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* USART1 on PA9/PA10 is used by the board USB-UART bridge for HAR samples. */
void HAR_IO_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAR_IO_UART_H */
