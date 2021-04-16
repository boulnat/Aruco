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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

#include <AS7341.h>
#include <MCP9600.h>
#include <PCA9685.h>

//#include <gen_define.h>
//#include <co_canopen.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define CANOPEN_LED
//#ifdef CANOPEN_LED
//#define CANOPEN_LED_PORT GPIOA
//#define CANOPEN_LED_PIN GPIO_PIN_5
//#endif /* CANOPEN_LED */
//#define ERROR_LED
//#ifdef ERROR_LED
//# error "NO ERROR LED IMPLEMENTED"
//#endif /* ERROR_LED */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CAN_HandleTypeDef hcan1;
//CO_TIMER_T timer1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim15;

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

CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t TxData[8]={1,2,3,4,5,6,7,8};
uint8_t RxData[8];
uint32_t TxMailbox;

typedef struct {
  uint8_t ID;
  uint8_t TH;
  uint8_t TC;
  uint8_t TD;
  uint16_t CH1to3[3];
  uint16_t CH4to5[3];
  uint16_t CH6to8[3];
} canPacketStruct;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_CAN1_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM15_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C3_Init(void);
void StartDefaultTask(void *argument);
void StartReadTempTask(void *argument);
void StartReadLightTask(void *argument);

/* USER CODE BEGIN PFP */
/*
static RET_T nmtInd(BOOL_T	execute, CO_NMT_STATE_T newState);
static void hbState(UNSIGNED8	nodeId, CO_ERRCTRL_T state,
		CO_NMT_STATE_T	nmtState);
static RET_T sdoServerReadInd(BOOL_T execute, UNSIGNED8	sdoNr, UNSIGNED16 index,
		UNSIGNED8	subIndex);
static RET_T sdoServerCheckWriteInd(BOOL_T execute, UNSIGNED8 node,
		UNSIGNED16 index, UNSIGNED8	subIndex, const UNSIGNED8 *pData);
static RET_T sdoServerWriteInd(BOOL_T execute, UNSIGNED8 sdoNr,
		UNSIGNED16 index, UNSIGNED8	subIndex);
static void pdoInd(UNSIGNED16);
static void pdoRecEvent(UNSIGNED16);
static void canInd(CO_CAN_STATE_T);
static void commInd(CO_COMM_STATE_EVENT_T);
static void ledGreenInd(BOOL_T);
static void ledRedInd(BOOL_T);
static void errorHandler(int errorCode);
static void timerCallback(void* pVoid);
*/
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
	//UNSIGNED8	emcyData[5] = { 1, 2, 3, 4, 5 };

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  //uint8_t I2C_address = 0x80;
  //pca9685_init(&hi2c3, I2C_address);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_CAN1_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM15_Init();
  MX_USART2_UART_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */
	  PCA9685begin(hi2c1,0);
	  pca9685_init(0x80);
	  RxData[0]=8;
	  RxData[1]=0;
  /* In this evaluation example
  	 * the hardware configuration is generated by the CubeIDE.
  	 * Normally this is done by codrvHardwareInit() */
  	//codrvHardwareInit();
/*
  	if (codrvCanInit(250u) != RET_OK)  {
  		errorHandler(1);
  	}


  	if (codrvTimerSetup(CO_TIMER_INTERVAL) != RET_OK)  {
  		errorHandler(2);
  	}


  	if (coCanOpenStackInit(NULL) != RET_OK)  {
  		printf("error init library\n");
  		errorHandler(1);
  	}


  	if (coEventRegister_NMT(nmtInd) != RET_OK)  {
  		errorHandler(3);
  	}
  	if (coEventRegister_ERRCTRL(hbState) != RET_OK)  {
  		errorHandler(4);
  	}
  	if (coEventRegister_SDO_SERVER_READ(sdoServerReadInd) != RET_OK)  {
  		errorHandler(5);
  	}
  	if (coEventRegister_SDO_SERVER_CHECK_WRITE(sdoServerCheckWriteInd) != RET_OK)  {
  		errorHandler(6);
  	}
  	if (coEventRegister_SDO_SERVER_WRITE(sdoServerWriteInd) != RET_OK)  {
  		errorHandler(7);
  	}
  	if (coEventRegister_PDO(pdoInd) != RET_OK)  {
  		errorHandler(8);
  	}
  	if (coEventRegister_PDO_REC_EVENT(pdoRecEvent) != RET_OK)  {
  		errorHandler(9);
  	}
  	if (coEventRegister_LED_GREEN(ledGreenInd) != RET_OK)  {
  		errorHandler(10);
  	}
  	if (coEventRegister_LED_RED(ledRedInd) != RET_OK)  {
  		errorHandler(11);
  	}
  	if (coEventRegister_CAN_STATE(canInd) != RET_OK)  {
  		errorHandler(12);
  	}
  	if (coEventRegister_COMM_EVENT(commInd) != RET_OK)  {
  		errorHandler(13);
  	}


  	if (codrvCanEnable() != RET_OK)  {
  		errorHandler(14);
  	}


  	if (coEmcyWriteReq(0x1234, &emcyData[0]) != RET_OK)  {
  		errorHandler(15);
  	}


  	if (coTimerStart(&timer1, (1000ul * 1000ul), timerCallback, NULL, CO_TIMER_ATTR_ROUNDUP_CYCLIC) != RET_OK)  {
  		errorHandler(16);
  	}
*/

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x033<<5;
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

	  Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_OVERRUN) != HAL_OK)
  {

	  Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
  {

	  Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {

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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C3|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 5;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 14;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
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
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
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
  hi2c1.Init.Timing = 0x00000000;
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
  hi2c3.Init.Timing = 0x00000000;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 0;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 65535;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */
  HAL_TIM_MspPostInit(&htim15);

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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
/*********************************************************************/
/*
static void timerCallback(void* pVoid)
{
UNSIGNED8 val_2000_1;


	if (coOdGetObj_u8(0x2000, 1u, &val_2000_1) == RET_OK)  {

		val_2000_1++;


		if (coOdPutObj_u8(0x2000, 1u, val_2000_1) != RET_OK)  {
			printf("something went wrong\n");
		}
	}
}



static RET_T nmtInd(
		BOOL_T	execute,
		CO_NMT_STATE_T	newState
	)
{
	//printf("nmtInd: New Nmt state %d - execute %d\n", newState, execute);

	return(RET_OK);
}


static void pdoInd(
		UNSIGNED16	pdoNr
	)
{
	//printf("pdoInd: pdo %d received\n", pdoNr);
}



static void pdoRecEvent(
		UNSIGNED16	pdoNr
	)
{
	//printf("pdoRecEvent: pdo %d time out\n", pdoNr);
}



static void hbState(
		UNSIGNED8	nodeId,
		CO_ERRCTRL_T state,
		CO_NMT_STATE_T	nmtState
	)
{
	//printf("hbInd: HB Event %d node %d nmtState: %d\n", state, nodeId, nmtState);

    return;
}

static RET_T sdoServerReadInd(
		BOOL_T		execute,
		UNSIGNED8	sdoNr,
		UNSIGNED16	index,
		UNSIGNED8	subIndex
	)
{
	//printf("sdo server read ind: exec: %d, sdoNr %d, index %x:%d\n",
	//	execute, sdoNr, index, subIndex);

   // return(RET_INVALID_PARAMETER);
	return(RET_OK);
}


static RET_T sdoServerCheckWriteInd(
		BOOL_T		execute,
		UNSIGNED8	sdoNr,
		UNSIGNED16	index,
		UNSIGNED8	subIndex,
		const UNSIGNED8	*pData
	)
{
	//printf("sdo server check write ind: exec: %d, sdoNr %d, index %x:%d\n",
	//	execute, sdoNr, index, subIndex);

   // return(RET_INVALID_PARAMETER);
	return(RET_OK);
}


static RET_T sdoServerWriteInd(
		BOOL_T		execute,
		UNSIGNED8	sdoNr,
		UNSIGNED16	index,
		UNSIGNED8	subIndex
	)
{
	//printf("sdo server write ind: exec: %d, sdoNr %d, index %x:%d\n",
	//	execute, sdoNr, index, subIndex);

   // return(RET_INVALID_PARAMETER);
	return(RET_OK);
}


static void canInd(
	CO_CAN_STATE_T	canState
	)
{
	switch (canState)  {
		case CO_CAN_STATE_BUS_OFF:
			//printf("CAN: Bus Off\n");
			break;
		case CO_CAN_STATE_BUS_ON:
			//printf("CAN: Bus On\n");
			break;
		case CO_CAN_STATE_PASSIVE:
			//printf("CAN: Passive\n");
			break;
		default:
			break;
	}
}


static void commInd(
		CO_COMM_STATE_EVENT_T	commEvent
	)
{
	switch (commEvent)  {
		case CO_COMM_STATE_EVENT_CAN_OVERRUN:
			//printf("COMM-Event: CAN Overrun\n");
			break;
		case CO_COMM_STATE_EVENT_REC_QUEUE_FULL:
			//printf("COMM-Event: Rec Queue Full\n");
			break;
		case CO_COMM_STATE_EVENT_REC_QUEUE_OVERFLOW:
			//printf("COMM-Event: Rec Queue Overflow\n");
			break;
		case CO_COMM_STATE_EVENT_REC_QUEUE_EMPTY:
			//printf("COMM-Event: Rec Queue Empty\n");
			break;
		case CO_COMM_STATE_EVENT_TR_QUEUE_FULL:
			//printf("COMM-Event: Tr Queue Full\n");
			break;
		case CO_COMM_STATE_EVENT_TR_QUEUE_OVERFLOW:
			//printf("COMM-Event: Tr Queue Overflow\n");
			break;
		case CO_COMM_STATE_EVENT_TR_QUEUE_EMPTY:
			//printf("COMM-Event: Tr Queue Empty\n");
			break;
		default:
			//printf("COMM-Event: %d\n", commEvent);
			break;
	}

}


static void ledGreenInd(
		BOOL_T	on
	)
{
#ifdef CANOPEN_LED
	if (on == CO_TRUE)  {
		HAL_GPIO_WritePin(CANOPEN_LED_PORT, CANOPEN_LED_PIN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(CANOPEN_LED_PORT, CANOPEN_LED_PIN, GPIO_PIN_RESET);
	}
#endif
}


static void ledRedInd(
		BOOL_T	on
	)
{
#ifdef ERROR_LED
	if (on == CO_TRUE)  {
		HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_RESET);
	}
#endif
}

static void errorHandler(
		int errorCode
	)
{
	while(1) { (void)errorCode; };
}
*/
/*********************************************************************/
/**
  * @brief  Rx Fifo 0 message pending callback
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];

  /* Get RX message */
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {
    /* Reception Error */
    Error_Handler();
  }
//ID that it listen to for a specific action
  if ((RxHeader.StdId == 0x0008) && (RxHeader.IDE == CAN_ID_STD))
  {
		HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&RxHeader,RxData);
		sprintf(msg, "rxData %x\r\n", RxData[0]);
		HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
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

	  char msg[50];
	  uint8_t canPacketMSB[8];
	  uint8_t canPacketLSB[8]={1,2,3,4,5,6,7,8};
	  uint8_t canPacketLSB2[8]={9,10,11,12,13,14,15,16};
	  uint16_t canPacketLSB3[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	  for(;;){
		    for(int i=0; i<12; i++){
		        _channel_readings[i]=0;
		    }

		    //AS7341 initialize
		    AS7341begin(hi2c1);
		    AS7341init(0x01);

			//configure integration time
		    setATIME(100);
		    setASTEP(0xE7);
		    setGain(AS7341_GAIN_256X);

		    readAllChannels(_channel_readings);

		    //get all channel
		    int a = 0;
		    for(int i=0; i<8; i++){
		    	if(i%2==0){
		    		canPacketMSB[i]=getChannel(a)>>8;
		    	}
		    	else{
		    		canPacketMSB[i]=getChannel(a)&0x00FF;
		    		a++;
		    	}
		    }


		    for(int i=0; i<12; i++){
			    sprintf(msg, "getChannel %d = %x\r\n",i+1,  getChannel(i));
			    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
		    }


		    sprintf(msg, "getThermocoupleTemp = %d\r\n",getThermocoupleTemp(1));
		    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

		    enableFlickerDetection(1);
		    sprintf(msg, "getFlickerDetectStatus = %d\r\n",getFlickerDetectStatus());
		    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

		    //coCanOpenStackInit();
		    //codrvCanInit();
		    //codrvTimerSetup();
		    //codrvCanEnable();

			//if (coCommTask() == CO_TRUE)  {
						/* more CAN messages to process
						 * coCommTask() is called again */
			//}

		    //HAL_StatusTypeDef status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_OVERRUN);
		    //uint8_t messages = HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0);

		    //if(HAL_CAN_AddTxMessage(&hcan1,&TxHeader,getChannel(0),&TxMailbox)!= HAL_OK){
		    //	sprintf(msg,"HAL NOK\r\n");
		   // 	HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
		    //}

		    canPacketStruct b;
		    b.CH1to3[0]=0;
		    b.CH1to3[1]=getChannel(0);
		    b.CH1to3[2]=getChannel(1);
		    b.CH1to3[3]=getChannel(2);

		    b.CH4to5[0]=1;
		    b.CH4to5[1]=getChannel(3);
		    b.CH4to5[2]=getChannel(4);
		    b.CH4to5[3]=getChannel(5);

		    b.CH6to8[0]=2;
		    b.CH6to8[1]=getChannel(6);
		    b.CH6to8[2]=getChannel(7);
		    b.CH6to8[3]=getChannel(8);

		    uint16_t buff[12];
		    getAllChannels(buff);

		    HAL_CAN_AddTxMessage(&hcan1,&TxHeader,buff,&TxMailbox);
		    //HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH1to3,&TxMailbox);
		    //HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH4to5,&TxMailbox);
		    //HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH6to8,&TxMailbox);
		    while(HAL_CAN_IsTxMessagePending(&hcan1,TxMailbox)!=0);

		    //HAL_CAN_AddTxMessage(&hcan1,&TxHeader,canPacketLSB2,&TxMailbox);
		    //while(HAL_CAN_IsTxMessagePending(&hcan1,TxMailbox)!=0);

		    sprintf(msg, "b.CH1to3[0] %x\r\n", b.CH4to5[0]);
		    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
		    //osDelay(100);


		    uint16_t sharedvar=RxData[0];
		    uint16_t sharedchannel=RxData[1];
		    uint16_t shareddelay = 5;


		  	 uint8_t I2C_address = 0x80;

		  	 	 //all_led_off(I2C_address);


		  	 	 pca9685_mult_pwm(I2C_address, 0xffff, 0, 4095);

		  		 for(int i=0; i<4096/sharedvar; i++){
		  			pca9685_pwm(I2C_address, sharedchannel, 0, 4095-(sharedvar*i));

		  			if(i%80 == 0){
		  				readAllChannels(_channel_readings);
		  				b.CH1to3[0]=0;
		  				b.CH1to3[1]=getChannel(0);
		  				b.CH1to3[2]=getChannel(1);
		  				b.CH1to3[3]=getChannel(2);

		  				b.CH4to5[0]=1;
		  				b.CH4to5[1]=getChannel(3);
		  				b.CH4to5[2]=getChannel(4);
		  				b.CH4to5[3]=getChannel(5);

		  				b.CH6to8[0]=2;
		  				b.CH6to8[1]=getChannel(6);
		  				b.CH6to8[2]=getChannel(7);
		  				b.CH6to8[3]=getChannel(8);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH1to3,&TxMailbox);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH4to5,&TxMailbox);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH6to8,&TxMailbox);
		  			}
		  			else{
		  				//osDelay(50);
		  			}
		  			//pca9685_pwm(&hi2c1, I2C_address, 15, 0, 4095-(sharedvar*i));
		  			//HAL_Delay(shareddelay);
		  		 }

		  	 	 for(int i=0; i<4096/sharedvar; i++){
		  	 		pca9685_pwm(I2C_address, sharedchannel, 0, (sharedvar*i));
		  			if(i%80 == 0){
		  				readAllChannels(_channel_readings);
		  				b.CH1to3[0]=0;
		  				b.CH1to3[1]=getChannel(0);
		  				b.CH1to3[2]=getChannel(1);
		  				b.CH1to3[3]=getChannel(2);

		  				b.CH4to5[0]=1;
		  				b.CH4to5[1]=getChannel(3);
		  				b.CH4to5[2]=getChannel(4);
		  				b.CH4to5[3]=getChannel(5);

		  				b.CH6to8[0]=2;
		  				b.CH6to8[1]=getChannel(6);
		  				b.CH6to8[2]=getChannel(7);
		  				b.CH6to8[3]=getChannel(8);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH1to3,&TxMailbox);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH4to5,&TxMailbox);
		  				HAL_CAN_AddTxMessage(&hcan1,&TxHeader,b.CH6to8,&TxMailbox);
		  			}
		  	 		//pca9685_pwm(&hi2c1, I2C_address, 15 ,0, 4095-(sharedvar*i));
		  	 		//HAL_Delay(shareddelay);
		  	 	 }


	  }



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
/*
	  char msg[50];
	  uint8_t read[2]={0};
	  uint16_t readtest;
	  float readfloat=0.0;
	  uint8_t TH = 0x00;
	  uint8_t TV = 0x01;
	  uint8_t TC = 0x02;
	  uint8_t tempReg[] = {TH,TV,TC};

	  uint8_t data[1]={TH};
	  uint8_t MSB = 0;
	  uint8_t LSB = 0;
	  HAL_StatusTypeDef status;
	  */
	  /* Infinite loop */
	  for(;;)
	  {
	    //osDelay(10);

	    //C1 read CF
	    //C0 write CE
	/*
	    for(int i=0;i<255;i++){
	      if(HAL_I2C_IsDeviceReady(&hi2c1,i,10,200)==HAL_OK){
	    	  sprintf(msg,"id: %d\r\n",i);
	    	  HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
	      }
	    }
	*/
/*
	    uint8_t x = getThermocoupleTemp(&hi2c1,0);
	    __disable_irq();
	    sprintf(msg, "TH = %d\r\n", x);
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
	    __enable_irq();
*/
/*
	    uint8_t y = getAmbientTemp(&hi2c1,0);
	    sprintf(msg, "TC = %d\r\n", y);
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    uint8_t z = getTempDelta(&hi2c1,0);
	    sprintf(msg, "TD = %d\r\n", z);
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    sprintf(msg, "getAmbientResolution = %d\r\n", getAmbientResolution(&hi2c1));
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    sprintf(msg, "getAmbientResolution = %d\r\n", getThermocoupleResolution(&hi2c1));
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    sprintf(msg, "available = %d\r\n", available(&hi2c1));
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    sprintf(msg, "checkDeviceID = %d\r\n", checkDeviceID(&hi2c1));
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

	    sprintf(msg, "isInputRangeExceeded = %d\r\n", isInputRangeExceeded(&hi2c1));
	    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
	*/
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
  uint16_t sharedvar=32;
  uint16_t sharedchannel=0xFFFF;
  uint16_t shareddelay = 60;
  /* Infinite loop */

	 //turn off all LED
	 //all_led_off(&hi2c1, I2C_address);

	 //uint16_t channel = 0b1001001110010101;
	 //uint16_t channel = sharedchannel;

  	  //PCA9685begin(hi2c1,0);
  	  //pca9685_init(0x80);
/*
	 for(;;)

	 {

		 for(int i=0; i<4096/sharedvar; i++){
			pca9685_mult_pwm(0x80, sharedchannel, 0, 4095-(sharedvar*i));
			//pca9685_pwm(&hi2c1, I2C_address, 15, 0, 4095-(sharedvar*i));
			osDelay(shareddelay);

		 }


	 	 for(int i=0; i<4096/sharedvar; i++){
	 		pca9685_mult_pwm(0x80, sharedchannel, 0, (sharedvar*i));
	 		//pca9685_pwm(&hi2c1, I2C_address, 15 ,0, 4095-(sharedvar*i));
	 		osDelay(shareddelay);
	 	 }

	 }
*/
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
  //__disable_irq();
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
