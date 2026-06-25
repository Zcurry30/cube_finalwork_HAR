#include "har_app.h"
#include "har_io_uart.h"
#include "stm32f7xx_hal.h"

#include <stddef.h>

#define HAR_UART_BAUDRATE       115200U
#define HAR_UART_RX_BUFFER_SIZE 512U
#define HAR_UART_IDLE_TIMEOUT_MS 30U

static uint8_t har_uart_rx_buffer[HAR_UART_RX_BUFFER_SIZE];
static volatile uint16_t har_uart_rx_head;
static volatile uint16_t har_uart_rx_tail;
static char har_uart_line[HAR_LINE_MAX_LEN];
static uint16_t har_uart_line_len;
static uint32_t har_uart_last_rx_tick;

static uint32_t HAR_UART_BrrFromPclk(void)
{
  uint32_t pclk = HAL_RCC_GetPCLK2Freq();

  if (pclk == 0U)
  {
    pclk = SystemCoreClock;
  }

  if (pclk == 0U)
  {
    pclk = 16000000U;
  }

  return (pclk + (HAR_UART_BAUDRATE / 2U)) / HAR_UART_BAUDRATE;
}

static int HAR_UART_PopByte(uint8_t *byte)
{
  int has_data = 0;

  __disable_irq();
  if (har_uart_rx_tail != har_uart_rx_head)
  {
    *byte = har_uart_rx_buffer[har_uart_rx_tail];
    har_uart_rx_tail = (uint16_t)((har_uart_rx_tail + 1U) % HAR_UART_RX_BUFFER_SIZE);
    has_data = 1;
  }
  __enable_irq();

  return has_data;
}

static int HAR_UART_FlushLine(char *buffer, uint16_t buffer_len)
{
  if ((buffer == NULL) || (buffer_len == 0U) || (har_uart_line_len == 0U))
  {
    return 0;
  }

  if (har_uart_line_len >= buffer_len)
  {
    har_uart_line_len = (uint16_t)(buffer_len - 1U);
  }

  for (uint16_t i = 0U; i < har_uart_line_len; ++i)
  {
    buffer[i] = har_uart_line[i];
  }
  buffer[har_uart_line_len] = '\0';
  har_uart_line_len = 0U;

  HAR_OutputSerial("RX,");
  HAR_OutputSerial(buffer);
  HAR_OutputSerial("\r\n");

  return 1;
}

void HAR_IO_UART_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  USART1->CR1 = 0U;
  USART1->CR2 = 0U;
  USART1->CR3 = 0U;
  USART1->BRR = HAR_UART_BrrFromPclk();
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

  har_uart_rx_head = 0U;
  har_uart_rx_tail = 0U;
  har_uart_line_len = 0U;
  har_uart_last_rx_tick = HAL_GetTick();

  HAL_NVIC_SetPriority(USART1_IRQn, 5U, 0U);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

int HAR_UART_ReadLine(char *buffer, uint16_t buffer_len)
{
  uint8_t byte;
  int saw_byte = 0;

  if ((buffer == NULL) || (buffer_len == 0U))
  {
    return 0;
  }

  while (HAR_UART_PopByte(&byte) != 0)
  {
    saw_byte = 1;
    har_uart_last_rx_tick = HAL_GetTick();

    if ((byte == '\r') || (byte == '\n'))
    {
      if (har_uart_line_len == 0U)
      {
        continue;
      }

      return HAR_UART_FlushLine(buffer, buffer_len);
    }

    if (har_uart_line_len < (uint16_t)(HAR_LINE_MAX_LEN - 1U))
    {
      har_uart_line[har_uart_line_len++] = (char)byte;
    }
    else
    {
      har_uart_line_len = 0U;
      HAR_OutputSerial("ERR,line-too-long\r\n");
    }
  }

  if ((saw_byte == 0) &&
      (har_uart_line_len > 0U) &&
      ((HAL_GetTick() - har_uart_last_rx_tick) >= HAR_UART_IDLE_TIMEOUT_MS))
  {
    return HAR_UART_FlushLine(buffer, buffer_len);
  }

  return 0;
}

void HAR_OutputSerial(const char *message)
{
  if (message == NULL)
  {
    return;
  }

  while (*message != '\0')
  {
    while ((USART1->ISR & USART_ISR_TXE) == 0U)
    {
    }
    USART1->TDR = (uint8_t)(*message++);
  }
}

void USART1_IRQHandler(void)
{
  if ((USART1->ISR & USART_ISR_RXNE) != 0U)
  {
    uint8_t byte = (uint8_t)(USART1->RDR & 0xFFU);
    uint16_t next_head = (uint16_t)((har_uart_rx_head + 1U) % HAR_UART_RX_BUFFER_SIZE);

    if (next_head != har_uart_rx_tail)
    {
      har_uart_rx_buffer[har_uart_rx_head] = byte;
      har_uart_rx_head = next_head;
    }
  }

  if ((USART1->ISR & USART_ISR_ORE) != 0U)
  {
    (void)USART1->RDR;
  }
}
