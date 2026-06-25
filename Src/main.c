/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "eth.h"
#include "sdmmc.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "app_x-cube-ai.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "har_app.h"
#include "har_io_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* EARLY RX TEST: init USART1 + echo in tight loop, before ANY HAL      */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
  GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
  GPIOA->AFR[1] &= ~(0xFF << 4);
  GPIOA->AFR[1] |= (7 << 4) | (7 << 8);
  USART1->BRR = 16000000U / 115200U;
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
  const char *p = "EARLY ECHO\r\n";
  while (*p) { while (!(USART1->ISR & USART_ISR_TXE)); USART1->TDR = *p++; }
  /* Echo loop: run for a while, echo any RX byte back                   */
  for (volatile uint32_t t=0; t<10000000; t++) {
      if (USART1->ISR & USART_ISR_RXNE) {
          char c = USART1->RDR;
          while (!(USART1->ISR & USART_ISR_TXE));
          USART1->TDR = c;
      }
  }
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();
  { const char *m="MPU OK\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  { const char *m="HAL OK\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  { const char *m="CLK OK\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* Init USART1 RIGHT AFTER GPIO — MX_GPIO_Init set PA9 to USB_VBUS,
     we need it back as USART1_TX (CH340G is on PA9/PA10) */
  HAR_IO_UART_Init();
  HAR_OutputSerial("BOOT: UART OK\r\n");
  MX_USART3_UART_Init();
  MX_ETH_Init();
  { const char *m="ETH OK\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }
  /* MX_SDMMC1_SD_Init() SKIPPED — no SD card, hangs without one */
  { const char *m="SD SKIP\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }
  /* MX_USB_DEVICE_Init() SKIPPED — PA9 used for UART, USB VBUS unavailable */
  { const char *m="USB SKIP\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }
  MX_X_CUBE_AI_Init();
  { const char *m="AI OK\r\n"; while(*m){while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=*m++;} }
  /* Dump USART1 registers for debug */
  { uint32_t v; char h[]="0123456789ABCDEF";
    #define P(m,n) v=*(volatile uint32_t*)(m); while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=n;
    #define X while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>28)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>24)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>20)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>16)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>12)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>8)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[(v>>4)&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR=h[v&0xF]; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR='\r'; while(!(USART1->ISR&USART_ISR_TXE));USART1->TDR='\n';
    P(0x40011000,'C');P(0x40011000,'R');P(0x40011000,'1');P(0x40011000,'=');X;
    P(0x40011004,'C');P(0x40011004,'R');P(0x40011004,'2');P(0x40011004,'=');X;
    P(0x40011008,'C');P(0x40011008,'R');P(0x40011008,'3');P(0x40011008,'=');X;
    P(0x4001100C,'B');P(0x4001100C,'R');P(0x4001100C,'R');P(0x4001100C,'=');X;
    P(0x4001101C,'I');P(0x4001101C,'S');P(0x4001101C,'R');P(0x4001101C,'=');X;
    P(0x40020000,'M');P(0x40020000,'O');P(0x40020000,'D');P(0x40020000,'=');X;
    P(0x4002000C,'P');P(0x4002000C,'U');P(0x4002000C,'P');P(0x4002000C,'=');X;
    P(0x40020024,'A');P(0x40020024,'F');P(0x40020024,'H');P(0x40020024,'=');X;
    P(0x40023844,'A');P(0x40023844,'P');P(0x40023844,'2');P(0x40023844,'=');X;
    #undef P
    #undef X
  }
  /* USER CODE BEGIN 2 */
  HAR_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

  MX_X_CUBE_AI_Process();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* SKIP PLL — use HSI 16MHz directly (PLL config was crashing)            */
  /* After reset: HSI=16MHz, SYSCLK=16MHz, HCLK=16MHz,                     */
  /*               APB1=16MHz, APB2=16MHz, FLASH_LATENCY=0                  */
  /* This is plenty for HAR inference and avoids all clock issues.          */
  HAL_SYSTICK_Config(16000000U / 1000U);  /* 1ms SysTick @ 16MHz            */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
