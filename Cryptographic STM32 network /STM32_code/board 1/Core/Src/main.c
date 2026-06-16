/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmox_gcm.h"
#include "cmox_crypto.h"
#include "cmox_cipher.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_LEN       16
#define IV_LEN        12
#define TAG_LEN       16
#define PLAINTEXT_LEN 32
#define TOTAL_LEN 60
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile int tx_flag = 0;
volatile int tx_enable = 0;
uint8_t txbuffer[TOTAL_LEN] = { 0 };
uint8_t adcVal = 0;
float temp = 0, temprt = 0;
/*__ALIGN_BEGIN uint8_t key[KEY_LEN] __ALIGN_END;
__ALIGN_BEGIN uint8_t iv[IV_LEN] __ALIGN_END;
__ALIGN_BEGIN uint8_t tag[TAG_LEN] __ALIGN_END;
__ALIGN_BEGIN uint8_t aad[] __ALIGN_END = { 0 };
__ALIGN_BEGIN uint8_t plaintext[PLAINTEXT_LEN] __ALIGN_END = "STM32 GCM Encrypt Test";
__ALIGN_BEGIN uint8_t ciphertext[PLAINTEXT_LEN] __ALIGN_END;*/

cmox_cipher_handle_t cipher_ctx;
cmox_gcm_handle_t gcm_ctx;

uint8_t key[KEY_LEN]             __attribute__((aligned(4))) = { 0x46, 0x3b, 0x41, 0x29, 0x11, 0x76, 0x7d, 0x57, 0xa0, 0xb3, 0x39, 0x69, 0xe6, 0x74, 0xff, 0xe7 };
uint8_t iv[IV_LEN]               __attribute__((aligned(4))) = { 0x61, 0x1c, 0xe6, 0xf9, 0xa6, 0x88, 0x07, 0x50, 0xde, 0x7d, 0xa6, 0xcb };
uint8_t tag[TAG_LEN]             __attribute__((aligned(4)));
uint8_t plaintext[PLAINTEXT_LEN] __attribute__((aligned(4))) = { 0 };
uint8_t aad[] __attribute__((aligned(4))) = { 0x0a, 0x68, 0x2f, 0xbc, 0x61, 0x92, 0xe1, 0xb4, 0x7a, 0x5e, 0x08, 0x68, 0x78, 0x7f, 0xfd, 0xaf,0xe5, 0xa5, 0x0c, 0xea, 0xd3, 0x57, 0x58, 0x49, 0x99, 0x0c, 0xdd, 0x2e, 0xa9, 0xb3, 0x59, 0x77,0x49, 0x40, 0x3e, 0xfb, 0x4a, 0x56, 0x68, 0x4f, 0x0c, 0x6b, 0xde, 0x35, 0x2d, 0x4a, 0xee, 0xc5 };
uint8_t ciphertext[PLAINTEXT_LEN]__attribute__((aligned(4)));

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void toggle_led()
{
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    HAL_Delay(500);
}

float readVoltage()
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adcVal = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return (adcVal / 4095.0f) * 3.3f;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0)
    {
        tx_enable = !tx_enable;
        if(tx_enable)
            tx_flag = 1;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        tx_flag = 1;
    }
}

void led_on()
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_Delay(500);
}
void led_off()
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_Delay(500);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
   if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
   {
	   Error_Handler();
   }



   if( HAL_UART_Transmit_IT(&huart2, (uint8_t*)txbuffer, sizeof(txbuffer)) != HAL_OK)
   {
  	Error_Handler();
   }



  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
 while (1)
  {
     temp = readVoltage();
     temprt = temp * 100.0f;
     sprintf((char*)plaintext, "%.2f", temprt);


     cmox_gcm_construct(&gcm_ctx, CMOX_AESFAST_GCMFAST_ENC);
     //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);  // Green LED

     cmox_cipher_setKey((cmox_cipher_handle_t*)&gcm_ctx, key, KEY_LEN);
     cmox_cipher_setIV((cmox_cipher_handle_t*)&gcm_ctx, iv, IV_LEN);
     cmox_cipher_setTagLen((cmox_cipher_handle_t*)&gcm_ctx, TAG_LEN);
     cmox_cipher_appendAD((cmox_cipher_handle_t*)&gcm_ctx, aad, 0);

     size_t out_len = 0;
     if (cmox_cipher_append((cmox_cipher_handle_t*)&gcm_ctx, plaintext, PLAINTEXT_LEN, ciphertext, &out_len) != CMOX_CIPHER_SUCCESS)
       Error_Handler();

     size_t tag_len = TAG_LEN;
     if (cmox_cipher_generateTag((cmox_cipher_handle_t*)&gcm_ctx, tag, &tag_len) != CMOX_CIPHER_SUCCESS)
       Error_Handler();

     if (tx_enable && tx_flag)
     {
   	  tx_flag = 0;
   	  led_on();
   	  memcpy(txbuffer, iv, IV_LEN);
   	  memcpy(txbuffer + IV_LEN, ciphertext, PLAINTEXT_LEN);
   	  memcpy(txbuffer + IV_LEN + PLAINTEXT_LEN, tag, TAG_LEN);
   	  //HAL_UART_Transmit_IT(&huart2, iv, IV_LEN);            // Send IV
   	 // HAL_UART_Transmit_IT(&huart2, ciphertext, PLAINTEXT_LEN);  // Send ciphertext
   	  HAL_UART_Transmit_IT(&huart2, txbuffer, TOTAL_LEN);
   	  led_off();
     }
     HAL_Delay(1500);
     cmox_cipher_cleanup((cmox_cipher_handle_t*)&gcm_ctx);
    /* USER CODE END WHILE */

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
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
