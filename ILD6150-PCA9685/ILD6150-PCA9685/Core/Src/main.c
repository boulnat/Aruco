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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct
{
  volatile uint8_t ID_channel;

  volatile uint16_t Temp_C[10];                  /*!< Specifies the length of a time quantum.
                                            This parameter must be a number between Min_Data = 1 and Max_Data = 1024. */
  volatile uint16_t X[10];

  volatile uint16_t Y[10];

  volatile uint16_t Z[10];

  volatile uint16_t Power[10];

} LUT_TabTypeDef;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */
osThreadId_t comUsartTaskHandle;
const osThreadAttr_t comUsartTask_attributes = {
  .name = "comUsartTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

uint16_t sharedvar=16;
uint16_t sharedchannel=0x9395;
uint16_t shareddelay = 5;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void ComUsartTask(void *argument);

uint8_t PCA9685_read(I2C_HandleTypeDef *hi2c, uint8_t address, unsigned char reg);
void pca9685_init(I2C_HandleTypeDef *hi2c, uint8_t address);
void pca9685_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t num, uint16_t on, uint16_t off);
void pca9685_mult_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint16_t num, uint16_t on, uint16_t off);
HAL_StatusTypeDef pca9685_all_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint16_t on, uint16_t off);
void all_led_off(I2C_HandleTypeDef *hi2c, uint8_t address);
void scenario1();
void scenario2();
void scenario3();
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  comUsartTaskHandle = osThreadNew(ComUsartTask, NULL, &comUsartTask_attributes);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 16;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

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
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
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
  huart2.Init.StopBits = UART_STOPBITS_2;
  huart2.Init.Parity = UART_PARITY_EVEN;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
// Returns position of the only set bit in 'n'

uint8_t PCA9685_read(I2C_HandleTypeDef *hi2c, uint8_t address, unsigned char reg)
{
	uint8_t res={0};
	HAL_I2C_Master_Transmit(hi2c, address, 0x00, 1, 1);
	//HAL_I2C_Master_Receive(hi2c,address,res,1,1);
	return res;
}

void pca9685_init(I2C_HandleTypeDef *hi2c, uint8_t address)
{
 #define PCA9685_MODE1 0x00
 uint8_t initStruct[2];
 uint8_t prescale = 0x03; // hardcoded
 HAL_I2C_Master_Transmit(hi2c, address, PCA9685_MODE1, 1, 1);
 uint8_t oldmode = 0x00; // hardcoded
 //uint8_t oldmode = PCA9685_read(hi2c,address,PCA9685_MODE1);
 // HAL_I2C_Master_Receive(hi2c, address, &oldmode, 1, 1);
 uint8_t newmode = ((oldmode & 0x7F) | 0x10);
 initStruct[0] = PCA9685_MODE1;
 initStruct[1] = newmode;
 HAL_I2C_Master_Transmit(hi2c, address, initStruct, 2, 1);
 //initStruct[0] = 0xFE;
 initStruct[1] = prescale;
 HAL_I2C_Master_Transmit(hi2c, address, initStruct, 2, 1);
 //initStruct[0] = PCA9685_MODE1;
 initStruct[1] = oldmode;
 HAL_I2C_Master_Transmit(hi2c, address, initStruct, 2, 1);
 osDelay(5);
 initStruct[1] = (oldmode | 0xA1);
 HAL_I2C_Master_Transmit(hi2c, address, initStruct, 2, 1);
}

void pca9685_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t num, uint16_t on, uint16_t off)
{
	uint8_t outputBuffer[] = {0x06 + 4*num, on, (on >> 8), off, (off >> 8)};
	HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
}

void pca9685_mult_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint16_t num, uint16_t on, uint16_t off)
{
	int i, iter;

	for (i=1, iter=1; i<65535; i<<=1, iter++)
	{
		if (num & i)
		{
			uint8_t outputBuffer[] = {0x06 + 4*((iter)-1), on, (on >> 8), off, (off >> 8)};
			HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
		}
		else
		{
			uint8_t outputBuffer[] = {0x06 + 4*((iter)-1), 0, (0 >> 8), 4096, (4096 >> 8)};
			HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
		}
	}


	/*
	uint8_t channel = 0;
	if(num & 0b00000001)
	{
		uint8_t outputBuffer[] = {0x06 + 4*((num & 0b00000001)-1), on, (on >> 8), off, (off >> 8)};
		HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
	}
	if(num & 0b00000010)
	{
		uint8_t outputBuffer[] = {0x06 + 4*((num & 0b00000010)-1), on, (on >> 8), off, (off >> 8)};
		HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
	}
	*/
}

void all_led_off(I2C_HandleTypeDef *hi2c, uint8_t address){

	 uint8_t ALL_LED_OFF = 0xFC;
	 uint8_t outputBuffer[] = {ALL_LED_OFF, 0, (0 >> 8), 4096, (4096 >> 8)};
	 HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
}

HAL_StatusTypeDef pca9685_all_pwm(I2C_HandleTypeDef *hi2c, uint8_t address, uint16_t on, uint16_t off)
{
	uint8_t ALL_LED_ON = 0xFA;
	uint8_t outputBuffer[] = {ALL_LED_ON, on, (on >> 8), off, (off >> 8)};
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, address, outputBuffer, sizeof(outputBuffer), 1);
	return status;
}

void scenario1(){
	 uint8_t I2C_address = 0x80;
	 pca9685_init(&hi2c1, I2C_address);
	 //turn off all LED
	 all_led_off(&hi2c1, I2C_address);

	 /* Infinite loop */
	 for(;;)
	 {
		 for(int i=0; i<4096/sharedvar; i++){
			pca9685_all_pwm(&hi2c1, I2C_address, 0, 4095-(sharedvar*i));
			osDelay(5);
		 }

	 	 for(int i=0; i<4096/sharedvar; i++){
	 		 pca9685_all_pwm(&hi2c1, I2C_address, 0, (sharedvar*i));
	 		 osDelay(5);
	 	 }
	 }
}

void scenario2(){
	 uint8_t I2C_address = 0x80;
	 pca9685_init(&hi2c1, I2C_address);
	 //turn off all LED
	 //all_led_off(&hi2c1, I2C_address);

	 //uint16_t channel = 0b1001001110010101;
	 //uint16_t channel = sharedchannel;

	 /* Infinite loop */
	 for(;;)
	 {
		 for(int i=0; i<4096/sharedvar; i++){
			pca9685_mult_pwm(&hi2c1, I2C_address, sharedchannel, 0, 4095-(sharedvar*i));
			//pca9685_pwm(&hi2c1, I2C_address, 15, 0, 4095-(sharedvar*i));
			osDelay(shareddelay);
		 }

	 	 for(int i=0; i<4096/sharedvar; i++){
	 		pca9685_mult_pwm(&hi2c1, I2C_address, sharedchannel, 0, (sharedvar*i));
	 		//pca9685_pwm(&hi2c1, I2C_address, 15 ,0, 4095-(sharedvar*i));
	 		osDelay(shareddelay);
	 	 }
	 }
}

void scenario3(){
uint8_t I2C_address = 0x80;
	pca9685_init(&hi2c1, I2C_address);

	uint8_t size = 2;
	LUT_TabTypeDef pLUTChannel[size];

	for(int a=0; a<size; a++){
		pLUTChannel[a].ID_channel = 1 << (a);
		for(int b=0; b<100; b++){
			pLUTChannel[a].Temp_C[b]=b;
			pLUTChannel[a].X[b]=b+b;
			pLUTChannel[a].Y[b]=b+b+b;
			pLUTChannel[a].Z[b]=b+b+b+b;
			pLUTChannel[a].Power[b]=b+b+b+b+b;
		}
	}

	/*
	LUT_TabTypeDef pLUTChannel[] =	{
									{1 << 0, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99},{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99},{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99},{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99},{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99}}
									};

	pLUTChannel[0].Temp_C={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
	pLUTChannel[0].X={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
	pLUTChannel[0].Y={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
	pLUTChannel[0].Z={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
	pLUTChannel[0].Power={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
	 */
	for(;;){
		for(int i=0; i<4096/sharedvar; i++){
			pca9685_mult_pwm(&hi2c1, I2C_address, pLUTChannel[0].Power[9], 0, 4095-(sharedvar*i));
			//pca9685_mult_pwm(&hi2c1, I2C_address, pLUTChannel[2].ID_channel, 0, 4095-(sharedvar*i));
			//pca9685_mult_pwm(&hi2c1, I2C_address, pLUTChannel[4].ID_channel, 0, 4095-(sharedvar*i));
			//pca9685_mult_pwm(&hi2c1, I2C_address, pLUTChannel[7].ID_channel, 0, 4095-(sharedvar*i));
			//pca9685_pwm(&hi2c1, I2C_address, 0x07, 0, 4095-(sharedvar*i));
			osDelay(shareddelay);
		}
	}

}

void ComUsartTask(void *argument){
	uint8_t rxBuffer[8];
	for(;;)
	{
		while (HAL_UART_Receive_DMA(&huart2, rxBuffer, sizeof(rxBuffer)) != HAL_OK)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
			osDelay(5);
		}
		osDelay(5);
		if(rxBuffer[0]==0x4D){
			HAL_UART_Transmit(&huart2,&rxBuffer[1], 1,500);
			sharedvar = 16;
			sharedchannel = 0x9395;
			shareddelay = 5;
			//sharedvar = (uint16_t) atoi(rxBuffer[5]);
		}
		if(rxBuffer[0]==0x44){
			HAL_UART_Transmit(&huart2,&rxBuffer[1], 1,500);
			sharedvar = 16;
			sharedchannel = 0x0F05;
			shareddelay = 1;
			//sharedvar = (uint16_t) atoi(rxBuffer[5]);
		}
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
		//osDelay(500);
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	//scenario1();
	scenario3();
  /* USER CODE END 5 */
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
