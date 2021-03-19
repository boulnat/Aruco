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

//typedef struct
//{
//  volatile uint8_t ID_channel;
//  volatile uint16_t Temp_C[20];
//  volatile uint16_t X[20];
//  volatile uint16_t Y[20];
//  volatile uint16_t Z[20];
//  volatile uint16_t Power[20];
//} LUT_TabTypeDef;

typedef struct  {
    uint16_t temp[100];
    union {
        struct {
            uint8_t id;
            float x[100];
            float y[100];
            float z[100];
            float power[100];
        }channel;
    }item;
}LUT_TabTypeDef;

typedef enum _themotype {
  MCP9600_TYPE_K,
  MCP9600_TYPE_J,
  MCP9600_TYPE_T,
  MCP9600_TYPE_N,
  MCP9600_TYPE_S,
  MCP9600_TYPE_E,
  MCP9600_TYPE_B,
  MCP9600_TYPE_R,
} MCP9600_ThemocoupleType;

typedef enum _resolution {
  MCP9600_ADCRESOLUTION_18,
  MCP9600_ADCRESOLUTION_16,
  MCP9600_ADCRESOLUTION_14,
  MCP9600_ADCRESOLUTION_12,
} MCP9600_ADCResolution;
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
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readTempTask */
osThreadId_t readTempTaskHandle;
const osThreadAttr_t readTempTask_attributes = {
  .name = "readTempTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for readLightTask */
osThreadId_t readLightTaskHandle;
const osThreadAttr_t readLightTask_attributes = {
  .name = "readLightTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */
osThreadId_t comUsartTaskHandle;
const osThreadAttr_t comUsartTask_attributes = {
  .name = "comUsartTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 254
};

uint16_t sharedvar=16;
uint16_t sharedchannel=0x9395;
uint16_t shareddelay = 5;

CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t TxData[8]={1,2,3,4,5,6,7,8};
uint8_t RxData[8];
uint32_t TxMailbox;
char msg[50];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C3_Init(void);
void StartDefaultTask(void *argument);
void StartReadTempTask(void *argument);
void StartReadLightTask(void *argument);

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
void scenario4();
void scenario5();
void Receive_CAN_Message_Polling(CAN_RxHeaderTypeDef RxMessage_, uint8_t *RxData_);
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
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x245<<5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
  {
	  Error_Handler();
  }

  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
	  /* Start Error */
	  Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_OVERRUN) != HAL_OK)
  {

	  Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
  {
	  /* Notification Error */
	  Error_Handler();
  }
  TxHeader.StdId = 0x07; //07
  //TxHeader.ExtId = 0x01; //delete
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.DLC = 8;
  TxHeader.TransmitGlobalTime = DISABLE;


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

  /* creation of readTempTask */
  readTempTaskHandle = osThreadNew(StartReadTempTask, NULL, &readTempTask_attributes);

  /* creation of readLightTask */
  readLightTaskHandle = osThreadNew(StartReadLightTask, NULL, &readLightTask_attributes);

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C3;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
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
  hcan1.Init.Prescaler = 1;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_6TQ;
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
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00000E14;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

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
	//uint8_t I2C_address = 0x80;
	//pca9685_init(&hi2c1, I2C_address);

	int n = 10;
	int m = 1;
    uint16_t temp[n*m];
    uint8_t id[n];
    float x[n*m];
    float y[n*m];
    float z[n*m];
    float power[n*m];

	uint8_t size = 1;
	//LUT_TabTypeDef pLUTChannel[size];

	for(;;){
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
		osDelay(100);
	}
}

void scenario4(){

	sprintf(msg,"Init\r\n");
	HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	//for(;;){
		if(HAL_CAN_AddTxMessage(&hcan1,&TxHeader,TxData,&TxMailbox)!= HAL_OK){
			sprintf(msg,"HAL NOK\r\n");
			HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
		}

		while(HAL_CAN_IsTxMessagePending(&hcan1,TxMailbox));
		TxData[0]+=1;

		sprintf(msg,"Transmitted\r\n");
		HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
		osDelay(1000);
	//}

}

void scenario5(){
	char msg[50];
	HAL_StatusTypeDef status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_OVERRUN);
	for(;;){
		uint8_t messages = HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0);

		if(messages > 0){
			if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK){

				sprintf(msg,"Received\r\n");
				HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);


			}
			scenario4();
		}

	}
	/*
}
	char msg[] = {'A','B','C'};
	CAN_RxHeaderTypeDef pHeader;
	uint8_t aData[8]={0,0,0,0,0,0,0,0};


	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_FULL);

	for(;;){
		if(HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&pHeader,aData)==HAL_OK){
			uint8_t val[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
			scenario4(val);
		}
*/
}



// here is the Polling Function
/*
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{

}
*/
/*
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];
	HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);

	sprintf(msg,"Received\r\n");
	HAL_UART_Transmit(&huart2, RxData,strlen(RxData),HAL_MAX_DELAY);

}
*/
	/*
	uint8_t TYPE_K = 0b000;
	uint8_t MCP9600_I2CADDR_DEFAULT = 0x67;
	uint8_t MCP9600_STATUS = 0x04;
	uint8_t MCP9600_DEVICEID = 0x20;
	uint8_t MCP9600_HOTJUNCTION = 0x00;
	uint8_t MCP9600_RAWDATAADC = 0x03;

	uint8_t outputBuffer[]={MCP9600_RAWDATAADC};
	uint8_t  inputBuffer[4];

	for(;;){


	HAL_I2C_Master_Transmit(&hi2c1, MCP9600_I2CADDR_DEFAULT, outputBuffer, sizeof(inputBuffer), 1);

	HAL_I2C_Master_Receive(&hi2c1, MCP9600_I2CADDR_DEFAULT, inputBuffer, sizeof(inputBuffer),1);


	osDelay(100);
	}
	*/


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
	scenario5();
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartReadTempTask */
/**
* @brief Function implementing the readTempTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadTempTask */
void StartReadTempTask(void *argument)
{
  /* USER CODE BEGIN StartReadTempTask */
  /* Infinite loop */
  for(;;)
  {

    osDelay(1);
  }
  /* USER CODE END StartReadTempTask */
}

/* USER CODE BEGIN Header_StartReadLightTask */
/**
* @brief Function implementing the readLightTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadLightTask */
void StartReadLightTask(void *argument)
{
  /* USER CODE BEGIN StartReadLightTask */
  /* Infinite loop */
  for(;;)
  {
	//scenario2();
    //osDelay(1);
  }
  /* USER CODE END StartReadLightTask */
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
	  sprintf(msg,"Error\r\n");
	  HAL_UART_Transmit(&huart2, (uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
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
