/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include<stdarg.h>
#include<stdio.h>
#include<string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define BL_DEBUG_MSG_EN
#define FLASH_SECTOR2_BASE_ADDRESS      0X8008000U
#define HSI_CLK					     	16000000U
#define SYSTICK_TIM_CLK          	 	HSI_CLK
#define TICK_HZ 					 	1000
#define CRC32_LEN						4
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
extern int counter;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_USART3_UART_Init(void);
static void jump_to_bootloader_cmdMode(void);
static void jump_to_user_application(void);
static void printmsg(char *, ...);
static void init_Systick_Timer(uint32_t );
static void DelayMs(uint32_t);
uint8_t bl_verify_crc(uint8_t *pdata, uint32_t len, uint32_t host_crc);
void send_ack(void);
void send_nack(void);
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
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_CRC_Init();
  MX_USART3_UART_Init();
  init_Systick_Timer(TICK_HZ);

  //HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
  printmsg("In Bootloader\n\r");
  if(HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin) == GPIO_PIN_RESET){

	  printmsg("BL_DEBUG_MSG: Button is pressed, going to BL CMD mode\n\r");

	  jump_to_bootloader_cmdMode();
  }
  else{

	  printmsg("BL_DEBUG_MSG: Button is not pressed, going to USER mode\n\r");

	  jump_to_user_application();
  }



}


void jump_to_bootloader_cmdMode(void){

	uint8_t rx_buffer[20];
	HAL_UART_Receive(&huart3, (uint8_t *)rx_buffer,1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, (uint8_t *)&rx_buffer[1],rx_buffer[0], HAL_MAX_DELAY);

	uint32_t host_crc = *((uint32_t *)(rx_buffer + rx_buffer[0] + 1 - CRC32_LEN));
	uint8_t cmd_len = rx_buffer[0] - CRC32_LEN - 1;

	if(bl_verify_crc(rx_buffer, cmd_len, host_crc)){
		send_ack();
	}
	else{
		send_nack();
	}

}

uint8_t bl_verify_crc(uint8_t *pdata, uint32_t len, uint32_t host_crc){
	uint32_t target_crc;

	for(int i = 0; i<len; i++){
		uint32_t data = pdata[i];
		target_crc = HAL_CRC_Accumulate(&hcrc, &data, host_crc);
	}
	if(target_crc == host_crc){
		return 1;
	}
	else{
		return 0;
	}
}

void send_ack(void){
	uint8_t ack = 0xAA;
	HAL_UART_Transmit(&huart3, &ack, 1, HAL_MAX_DELAY);
}

void send_nack(void){
	uint8_t nack = 0x55;
	HAL_UART_Transmit(&huart3, &nack, 1, HAL_MAX_DELAY);
}

void DelayMs(uint32_t ms){

	counter =0;
	while(counter < ms);
}

void init_Systick_Timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t *)0xE000E014; // Systick reload value register
	uint32_t *pSCSR = (uint32_t *)0xE000E010; // Systick control and status register
	uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz) - 1; // 1ms

	//clear value of SVR
	*pSRVR &= ~(0x00FFFFFF);

	//load the value into SVR
	*pSRVR |= count_value;

	//configuration
	*pSCSR |= (1 << 1);// systick exception request enable
	*pSCSR |= (1 << 2);// clock source, 0->AHB/8, 1->uP clock (AHB)

	//enable systick
	*pSCSR |= (1 << 0); // enable counter
}
void jump_to_user_application(void){

	void (*app_reset_handler)(void);
	printmsg("BL_DEBUG_MSG: Jump_to_user_application\n\r");

	uint32_t msp_value = *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;
	printmsg("BL_DEBUG_MSG: Msp value : %#x\n\r", msp_value);

	__set_MSP(msp_value); // __ASM volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );

	uint32_t reset_handler_addr = *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4);

	app_reset_handler = (void *)reset_handler_addr;

	printmsg("BL_DEBUG_MSG: app reset handler addr : %#x\n\r", app_reset_handler);
	app_reset_handler();
}

void printmsg(char *format, ...){
	#ifdef BL_DEBUG_MSG_EN
		char str[80];
		va_list args;
		va_start(args, format);
		vsprintf(str,format,args);
		HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		va_end(args);
	#endif
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
