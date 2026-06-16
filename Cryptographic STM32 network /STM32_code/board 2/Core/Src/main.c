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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmox_gcm.h"
#include "cmox_crypto.h"
#include "cmox_cipher.h"
#include "lcd_i2c.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_LEN        16
#define IV_LEN         12
#define TAG_LEN        16
#define CIPHERTEXT_LEN 32
#define TOTAL_LEN      60

#define THINGSPEAK_API_KEY  "N3CKKQMFNT7S68MS"
#define THINGSPEAK_HOST     "api.thingspeak.com"
#define THINGSPEAK_PORT     80


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* Definitions for StartDefaultTas */
osThreadId_t StartDefaultTasHandle;
const osThreadAttr_t StartDefaultTas_attributes = {
  .name = "StartDefaultTas",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for uartRxTask */
osThreadId_t uartRxTaskHandle;
const osThreadAttr_t uartRxTask_attributes = {
  .name = "uartRxTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal4,
};
/* Definitions for wifiConnTask */
osThreadId_t wifiConnTaskHandle;
const osThreadAttr_t wifiConnTask_attributes = {
  .name = "wifiConnTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for decryptTask */
osThreadId_t decryptTaskHandle;
const osThreadAttr_t decryptTask_attributes = {
  .name = "decryptTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal3,
};
/* Definitions for lcdTask */
osThreadId_t lcdTaskHandle;
const osThreadAttr_t lcdTask_attributes = {
  .name = "lcdTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for wifiTxTask */
osThreadId_t wifiTxTaskHandle;
const osThreadAttr_t wifiTxTask_attributes = {
  .name = "wifiTxTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for rx_data */
osMessageQueueId_t rx_dataHandle;
const osMessageQueueAttr_t rx_data_attributes = {
  .name = "rx_data"
};
/* Definitions for data_is_decrypted_Lcd */
osSemaphoreId_t data_is_decrypted_LcdHandle;
const osSemaphoreAttr_t data_is_decrypted_Lcd_attributes = {
  .name = "data_is_decrypted_Lcd"
};
/* Definitions for data_is_decrypted_Wifi */
osSemaphoreId_t data_is_decrypted_WifiHandle;
const osSemaphoreAttr_t data_is_decrypted_Wifi_attributes = {
  .name = "data_is_decrypted_Wifi"
};
/* Definitions for lcd_ack */
osSemaphoreId_t lcd_ackHandle;
const osSemaphoreAttr_t lcd_ack_attributes = {
  .name = "lcd_ack"
};
/* Definitions for wifi_ack */
osSemaphoreId_t wifi_ackHandle;
const osSemaphoreAttr_t wifi_ack_attributes = {
  .name = "wifi_ack"
};
/* USER CODE BEGIN PV */

uint8_t rxbuffer[TOTAL_LEN] = {0};
volatile uint8_t rx_flag = 0, lcd_flag = 0;
uint32_t fault_check = CMOX_CIPHER_AUTH_FAIL;
cmox_cipher_retval_t retval;
char response[100] = {0};
volatile uint8_t wifi_ready = 0;

//QueueHandle_t rx_data;
//SemaphoreHandle_t data_is_decrypted;
volatile uint32_t lcd_count = 95, wifi_count = 99;


uint8_t key[KEY_LEN]        __attribute__((aligned(4))) = { 0x46, 0x3b, 0x41, 0x29, 0x11, 0x76, 0x7d, 0x57, 0xa0, 0xb3, 0x39, 0x69, 0xe6, 0x74, 0xff, 0xe7 };
uint8_t iv[IV_LEN]          __attribute__((aligned(4))) = { 0 };
uint8_t tag[TAG_LEN]        __attribute__((aligned(4)));
uint8_t aad[] __attribute__((aligned(4))) = { 0x0a, 0x68, 0x2f, 0xbc, 0x61, 0x92, 0xe1, 0xb4, 0x7a, 0x5e, 0x08, 0x68, 0x78, 0x7f, 0xfd, 0xaf,0xe5, 0xa5, 0x0c, 0xea, 0xd3, 0x57, 0x58, 0x49, 0x99, 0x0c, 0xdd, 0x2e, 0xa9, 0xb3, 0x59, 0x77,0x49, 0x40, 0x3e, 0xfb, 0x4a, 0x56, 0x68, 0x4f, 0x0c, 0x6b, 0xde, 0x35, 0x2d, 0x4a, 0xee, 0xc5 };
uint8_t ciphertext[CIPHERTEXT_LEN] __attribute__((aligned(4)));
uint8_t plaintext[CIPHERTEXT_LEN]  __attribute__((aligned(4))) =  { 0 };
cmox_gcm_handle_t gcm_ctx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void *argument);
void StartUartRxTask(void *argument);
void StartWifiConnTask(void *argument);
void StartDecryptTask(void *argument);
void StartLcdTask(void *argument);
void StartWifiTxTask(void *argument);

/* USER CODE BEGIN PFP */

void toggle_led()
{
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
    HAL_Delay(500);
}
void led_on()
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_Delay(500);
}
void led_off()
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_Delay(500);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
    rx_flag = 1;
    osThreadFlagsSet(uartRxTaskHandle, 0x01);
    /*BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(rx_data, &rxbuffer, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);*/
    }
}

void ESP_Send_AT_Command(char *cmd)
{

    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    HAL_UART_Receive(&huart3, (uint8_t *)response, sizeof(response), 1000);
    //printf("ESP Response: %s\r\n", response);
}
void wifi_cnx()
{
	  ESP_Send_AT_Command("AT");
	  ESP_Send_AT_Command("AT+CWMODE=1");
	  ESP_Send_AT_Command("AT+CWJAP=\"HUAWEI Y6 Prime 2019\",\"750tasss\""); //"AT+CWJAP=\"CSF.5G\",\"SuPjYerK\""
	  HAL_Delay(8000);
}
void wifi_reset()
{
	  ESP_Send_AT_Command("AT+RST");
	  HAL_Delay(3000);
}
void wifi_send()
{
    char at_cmd[128];
    char http_request[256];
    int data_length;

    // 1. Start TCP connection to ThingSpeak
    sprintf(at_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", THINGSPEAK_HOST, THINGSPEAK_PORT);
    ESP_Send_AT_Command(at_cmd);
    HAL_Delay(2000);

    // 2. Construct HTTP GET request
    sprintf(http_request,"GET /update?api_key=%s&field1=%s\r\nHost: %s\r\nConnection: close\r\n\r\n",THINGSPEAK_API_KEY, (char*)plaintext, THINGSPEAK_HOST);

    data_length = strlen(http_request);

    // 3. Tell ESP you will send `data_length` bytes
    sprintf(at_cmd, "AT+CIPSEND=%d\r\n", data_length);
    ESP_Send_AT_Command(at_cmd);
    HAL_Delay(500);

    // 4. Send actual HTTP GET request A49r2_k2G_Xqs.L
    ESP_Send_AT_Command(http_request);

}
void wifi_cnx_test()
{
	ESP_Send_AT_Command("AT+CWJAP?");
    HAL_Delay(8000);
    wifi_ready = 1;
    //ESP_Send_AT_Command("AT+CIFSR");
}


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

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);  // priority must match config
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
lcd_clear();

  /*if (HAL_UART_Receive_IT(&huart2, rxbuffer, TOTAL_LEN) != HAL_OK)
  {
	  Error_Handler();
  }*/
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of data_is_decrypted_Lcd */
  data_is_decrypted_LcdHandle = osSemaphoreNew(1, 0, &data_is_decrypted_Lcd_attributes);

  /* creation of data_is_decrypted_Wifi */
  data_is_decrypted_WifiHandle = osSemaphoreNew(1, 0, &data_is_decrypted_Wifi_attributes);

  /* creation of lcd_ack */
  lcd_ackHandle = osSemaphoreNew(1, 0, &lcd_ack_attributes);

  /* creation of wifi_ack */
  wifi_ackHandle = osSemaphoreNew(1, 0, &wifi_ack_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  if (data_is_decrypted_LcdHandle == NULL || data_is_decrypted_WifiHandle == NULL) {
      Error_Handler(); // Semaphore creation failed
  }
  if (lcd_ackHandle == NULL || data_is_decrypted_WifiHandle == NULL) {
      Error_Handler(); // Semaphore creation failed
  }
 // data_is_decrypted = xSemaphoreCreateBinary();
 // if (data_is_decrypted == NULL) {
  //    Error_Handler();}
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of rx_data */
  rx_dataHandle = osMessageQueueNew (1, 60, &rx_data_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
 // rx_data = xQueueCreate(5, TOTAL_LEN);
  if (rx_dataHandle == NULL) Error_Handler();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of StartDefaultTas */
  StartDefaultTasHandle = osThreadNew(StartDefaultTask, NULL, &StartDefaultTas_attributes);

  /* creation of uartRxTask */
  uartRxTaskHandle = osThreadNew(StartUartRxTask, NULL, &uartRxTask_attributes);

  /* creation of wifiConnTask */
  wifiConnTaskHandle = osThreadNew(StartWifiConnTask, NULL, &wifiConnTask_attributes);

  /* creation of decryptTask */
  decryptTaskHandle = osThreadNew(StartDecryptTask, NULL, &decryptTask_attributes);

  /* creation of lcdTask */
  lcdTaskHandle = osThreadNew(StartLcdTask, NULL, &lcdTask_attributes);

  /* creation of wifiTxTask */
  wifiTxTaskHandle = osThreadNew(StartWifiTxTask, NULL, &wifiTxTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PD13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the StartDefaultTas thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartUartRxTask */
/**
* @brief Function implementing the uartRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartRxTask */
void StartUartRxTask(void *argument)
{
  /* USER CODE BEGIN StartUartRxTask */
	  memset(rxbuffer, 0, sizeof(rxbuffer));
	  memset(ciphertext, 0, sizeof(ciphertext));
	  memset(iv, 0, sizeof(iv));
	  memset(tag, 0, sizeof(tag));
  /* Infinite loop */
  for(;;)
  {
		 led_on();

		  if (HAL_UART_Receive_IT(&huart2, rxbuffer, TOTAL_LEN) != HAL_OK)
		   {

		 	  //Error_Handler();
		   }
		  else
		  {
			  if (rxbuffer[0] != 97  || rxbuffer[1] != 28 || rxbuffer[2] != 230 || rxbuffer[3] != 249)
			  {
				  lcd_flag = 0; // it is the real data
			  }
			  else
			  {
				  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
				  osMessageQueuePut(rx_dataHandle, rxbuffer, 0, 0);
			            /*if (xQueueSend(rx_data, rxbuffer, portMAX_DELAY) != pdPASS)
			            {
			                // Queue full or failed
			                Error_Handler();
			            }*/
			      memset(rxbuffer, 0, sizeof(rxbuffer));
			      //osThreadFlagsSet(decryptTaskHandle, 0x01);
				  //lcd_count = osSemaphoreGetCount(lcd_ackHandle);
				  //wifi_count = osSemaphoreGetCount(wifi_ackHandle);
			      led_off();
			  }
		  }
		     //HAL_Delay(1000);
       osDelay(1000);
  }
  /* USER CODE END StartUartRxTask */
}

/* USER CODE BEGIN Header_StartWifiConnTask */
/**
* @brief Function implementing the wifiConnTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWifiConnTask */
void StartWifiConnTask(void *argument)
{
  /* USER CODE BEGIN StartWifiConnTask */
	  wifi_cnx();
	  wifi_cnx_test();
	  //wifi_reset();
	  osThreadExit();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartWifiConnTask */
}

/* USER CODE BEGIN Header_StartDecryptTask */
/**
* @brief Function implementing the decryptTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDecryptTask */
void StartDecryptTask(void *argument)
{
  /* USER CODE BEGIN StartDecryptTask */
	 if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
	  {
		   Error_Handler();
	  }
  /* Infinite loop */
  for(;;)
  {
	  //osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
	  //uint8_t local_buffer[TOTAL_LEN];  try this
	  if (osMessageQueueGet(rx_dataHandle, rxbuffer, NULL, osWaitForever) == osOK)
	  {
      memcpy(iv, rxbuffer, IV_LEN);
      memcpy(ciphertext, rxbuffer + IV_LEN, CIPHERTEXT_LEN);
      memcpy(tag, rxbuffer + IV_LEN + CIPHERTEXT_LEN, TAG_LEN);
      HAL_Delay(100);

	  cmox_gcm_construct(&gcm_ctx, CMOX_AESFAST_GCMFAST_DEC);
	  cmox_cipher_setKey((cmox_cipher_handle_t*)&gcm_ctx, key, KEY_LEN);
	  cmox_cipher_setIV((cmox_cipher_handle_t*)&gcm_ctx, iv, IV_LEN);
	  cmox_cipher_setTagLen((cmox_cipher_handle_t*)&gcm_ctx, TAG_LEN);
	  cmox_cipher_appendAD((cmox_cipher_handle_t*)&gcm_ctx, aad, 0); // or same AAD if used

	  size_t out_len;
	  if (cmox_cipher_append((cmox_cipher_handle_t*)&gcm_ctx, ciphertext, CIPHERTEXT_LEN, plaintext, &out_len) != CMOX_CIPHER_SUCCESS)
	  	   Error_Handler();

/*	  if (cmox_cipher_verifyTag((cmox_cipher_handle_t*)&gcm_ctx, tag, &fault_check) != CMOX_CIPHER_AUTH_SUCCESS)
	  {
	  	  Error_Handler();
	  }*/
	  retval = cmox_cipher_verifyTag((cmox_cipher_handle_t*)&gcm_ctx,tag, &fault_check);

	  /* Verify API returned value */
	  if (retval != CMOX_CIPHER_AUTH_SUCCESS)
	  {
	    //Error_Handler();
	  }
	  /* Verify Fault check variable value */
	  if (fault_check != CMOX_CIPHER_AUTH_SUCCESS)
	  {
	    //Error_Handler();
	  }
	  cmox_cipher_cleanup((cmox_cipher_handle_t*)&gcm_ctx);
	 /* if (rxbuffer[0] == 97  && rxbuffer[1] == 28 && rxbuffer[2] == 230 && rxbuffer[3] == 249)
	  {
		  // it is the real data
		  lcd_flag = 1;
	  }*/
	  //osThreadFlagsSet(lcdTaskHandle, 0x02);
	  //osThreadFlagsSet(wifiTxTaskHandle, 0x03);
	  osSemaphoreRelease(data_is_decrypted_LcdHandle);
	  osSemaphoreRelease(data_is_decrypted_WifiHandle);

	  osSemaphoreAcquire(lcd_ackHandle, osWaitForever);
	  osSemaphoreAcquire(wifi_ackHandle, osWaitForever);
	  }
      osDelay(1000);
  }
  /* USER CODE END StartDecryptTask */
}

/* USER CODE BEGIN Header_StartLcdTask */
/**
* @brief Function implementing the lcdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLcdTask */
void StartLcdTask(void *argument)
{
  /* USER CODE BEGIN StartLcdTask */
	  lcd_init();
	  HAL_Delay(100);
	  lcd_put_cursor(0, 5);
	  lcd_send_string((char*)"LCD_ON");
	  lcd_put_cursor(1, 0);
	  lcd_send_string((char*)"wait tx...");

	  for (int x = 0; x <= 5 ; x++)
	  {
		  lcd_put_cursor(1, 9);
		  lcd_send_int(x);
		  HAL_Delay(500);
	  }
	  lcd_clear();
  /* Infinite loop */
  for(;;)
  {

	  if (osSemaphoreAcquire(data_is_decrypted_LcdHandle, osWaitForever) == osOK)
	  {
		  //osThreadFlagsWait(0x02, osFlagsWaitAny, osWaitForever);
		  lcd_clear();
	      lcd_put_cursor(0, 0);
	      lcd_send_string((char*)"Temperature=");
	      lcd_put_cursor(1, 0);
	      lcd_send_string((char*)plaintext);


	   }
	  else
	  {
		  lcd_clear();
		  lcd_put_cursor(0, 0);
          lcd_send_string((char*)"No_Data!");
	  }
	  osSemaphoreRelease(lcd_ackHandle);
	  osDelay(1000);
  }
  /* USER CODE END StartLcdTask */
}

/* USER CODE BEGIN Header_StartWifiTxTask */
/**
* @brief Function implementing the wifiTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWifiTxTask */
void StartWifiTxTask(void *argument)
{
  /* USER CODE BEGIN StartWifiTxTask */
  /* Infinite loop */
  for(;;)
  {
	if (wifi_ready && osSemaphoreAcquire(data_is_decrypted_WifiHandle, osWaitForever) == osOK)
	{
	  //osThreadFlagsWait(0x03, osFlagsWaitAny, osWaitForever);
	  wifi_send();
	}
	osSemaphoreRelease(wifi_ackHandle);
    osDelay(1000);
  }
  /* USER CODE END StartWifiTxTask */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
