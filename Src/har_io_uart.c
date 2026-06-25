/**
  ******************************************************************************
  * @file    har_io_uart.c
  * @brief   USART1 bare-metal I/O — NO HAL, just like the working scan code
  ******************************************************************************
  */

#include "har_app.h"
#include "har_io_uart.h"
#include "stm32f7xx_hal.h"
#include <string.h>

#define UART_RX_BUF_SIZE    512U

static volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t uart_rx_head;
static volatile uint16_t uart_rx_tail;

/* ======================================================================== */
/*  HAR hooks                                                              */
/* ======================================================================== */

int HAR_UART_ReadLine(char *buffer, size_t buffer_size)
{
    size_t count = 0U;
    /* Poll: read all available bytes from HW + ring buffer */
    for (;;) {
        if (USART1->ISR & USART_ISR_RXNE) {
            char c = (char)(USART1->RDR & 0xFF);
            buffer[count++] = c;
            if (c == '\n' || count >= buffer_size - 1) break;
            continue;
        }
        uint16_t tail;
        __disable_irq(); tail = uart_rx_tail; __enable_irq();
        if (uart_rx_head != tail) {
            char c = (char)uart_rx_buf[uart_rx_head];
            uart_rx_head = (uart_rx_head + 1U) % UART_RX_BUF_SIZE;
            buffer[count++] = c;
            if (c == '\n' || count >= buffer_size - 1) break;
            continue;
        }
        break; /* no more data */
    }
    if (count > 0U) { buffer[count] = '\0'; return 1; }
    return 0;
}

void HAR_OutputSerial(const char *text)
{
    while (*text) {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *text++;
    }
}

/* ======================================================================== */
/*  USART1 interrupt (for ring buffer overflow protection, optional)       */
/* ======================================================================== */

void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)(USART1->RDR & 0xFF);
        uint16_t next = (uart_rx_tail + 1U) % UART_RX_BUF_SIZE;
        if (next != uart_rx_head) {
            uart_rx_buf[uart_rx_tail] = byte;
            uart_rx_tail = next;
        }
    }
    /* Clear ORE if set */
    if (USART1->ISR & USART_ISR_ORE) {
        (void)USART1->RDR;
    }
}

/* ======================================================================== */
/*  Init — pure register writes, zero HAL, same as proven scan code       */
/* ======================================================================== */

void HAR_IO_UART_Init(void)
{
    /* --- Enable clocks --- */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* --- PA9=TX (AF7, push-pull, no pull) --- */
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
    GPIOA->OSPEEDR |= (3 << 18) | (3 << 20);   /* very high speed */
    GPIOA->PUPDR &= ~((3 << 18) | (3 << 20));  /* no pull */
    GPIOA->PUPDR |= (1 << 20);                  /* PA10 = pull-up */
    GPIOA->AFR[1] &= ~(0xFF << 4);
    GPIOA->AFR[1] |= (7 << 4) | (7 << 8);      /* AF7 = USART1 */

    /* --- USART1: 115200 8N1 --- */
    USART1->BRR = 16000000U / 115200U;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

    /* --- NVIC --- */
    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}
