/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - Lab 4: Cooperative Scheduler (HCMUT)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler.h"
#include <stdio.h>
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
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Redirect printf to USART1
  */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
  return ch;
}

/**
  * @brief Task 1: Period 0.5s (500 ms = 50 ticks)
  */
void Task_LED1_05s(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  printf("[Time: %6lu ms] Task 1 (0.5s): LED1 toggled\r\n", get_time());
}

/**
  * @brief Task 2: Period 1.0s (1000 ms = 100 ticks)
  */
void Task_LED2_10s(void)
{
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  printf("[Time: %6lu ms] Task 2 (1.0s): LED2 toggled\r\n", get_time());
}

/**
  * @brief Task 3: Period 1.5s (1500 ms = 150 ticks)
  */
void Task_LED3_15s(void)
{
  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  printf("[Time: %6lu ms] Task 3 (1.5s): LED3 toggled\r\n", get_time());
}

/**
  * @brief Task 4: Period 2.0s (2000 ms = 200 ticks)
  */
void Task_LED4_20s(void)
{
  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  printf("[Time: %6lu ms] Task 4 (2.0s): LED4 toggled\r\n", get_time());
}

/**
  * @brief Task 5: Period 2.5s (2500 ms = 250 ticks)
  */
void Task_LED5_25s(void)
{
  HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
  printf("[Time: %6lu ms] Task 5 (2.5s): LED5 toggled\r\n", get_time());
}

/**
  * @brief Demonstration One-Shot Task (fires after initial delay)
  */
//void Task_OneShot_Demo(void)
//{
//  printf("[Time: %6lu ms] >>> One-Shot Demo Task Expired! <<<\r\n", get_time());
//}

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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* Turn off all LEDs initially (assuming active-low or active-high) */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin | LED4_Pin | LED5_Pin, GPIO_PIN_SET);

  /* Initialize the Cooperative Scheduler */
  SCH_Init();

//  printf("\r\n");
//  printf("==================================================\r\n");
//  printf("     BK-HCMUT - MICROPROCESSOR / RTOS LAB 4       \r\n");
//  printf("     COOPERATIVE SCHEDULER (O(1) DELTA QUEUE)     \r\n");
//  printf("==================================================\r\n");
//  printf("1 Tick = 10 ms\r\n");
//  printf("Task 1: 0.5s (50 ticks)  -> LED1 (PA1)\r\n");
//  printf("Task 2: 1.0s (100 ticks) -> LED2 (PA2)\r\n");
//  printf("Task 3: 1.5s (150 ticks) -> LED3 (PA3)\r\n");
//  printf("Task 4: 2.0s (200 ticks) -> LED4 (PA4)\r\n");
//  printf("Task 5: 2.5s (250 ticks) -> LED5 (PA5)\r\n");
//  printf("==================================================\r\n\r\n");

  /* Section 4: Add 5 periodic tasks */
  SCH_Add_Task(Task_LED1_05s, 0, 50);   /* 0.5s = 500ms = 50 ticks */
  SCH_Add_Task(Task_LED2_10s, 0, 100);  /* 1.0s = 1000ms = 100 ticks */
  SCH_Add_Task(Task_LED3_15s, 0, 150);  /* 1.5s = 1500ms = 150 ticks */
  SCH_Add_Task(Task_LED4_20s, 0, 200);  /* 2.0s = 2000ms = 200 ticks */
  SCH_Add_Task(Task_LED5_25s, 0, 250);  /* 2.5s = 2500ms = 250 ticks */

//  /* Section 5: Demonstration one-shot task before main loop */
//  SCH_Add_Task(Task_OneShot_Demo, 100, 0); /* 100 ticks = 1000ms delay, one-shot */

  /* Start TIM2 interrupt */
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    SCH_Dispatch_Tasks();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin
                          |LED5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin LED4_Pin
                           LED5_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin
                          |LED5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
