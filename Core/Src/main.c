/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "subroutine.h"
#include"stdbool.h"
#include <stdlib.h>
#include "DS18B20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE_OF_MSG 60 //NEW PROG:[5];S0T12t66;T33t18,
//S1T55;000000000000000000000000000000000000000000000000000000
//S0T55H10;000000000000000000000000000000000000000000000000000
//SET PROG:S1;T50t85;T35t70;T70t30;T0t0;T0t0;P10;0000000000000
//SET PROG:S0H10;T20t40;T50t80;T50t30;T0t0;T0t85;P20;000000000
//P1;000000000000000000000000000000000000000000000000000000000
//SET PROG:S0H10;T23t1;T0t0;T0t0;T0t0;T0t0;P1;0000000000000000
//STOP PROG;00000000000000000000000000000000000000000000000000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//definicja listy
List *head;
Subroutine kociol;
bool isHeating = false; //załączenie grzania
bool regType; //true dla PID, false dla bang-bang
double measuredTemperature; //zmierzona wartość temperatury
double hist; //wartość histerezy
uint8_t temperature; //nastawiona wartość temperatury
uint8_t timeToEndHeating; //czas trwania aktualnego cyklu grzania
uint8_t pageID; // numer aktualnie wyświetlanej strony
uint8_t progID; // numer aktywnego programu
uint8_t odebranaWiadomosc[110]; //string przychodzący z płytki
uint32_t startCounterTime = 0;
uint32_t CounterPump = 0;
uint32_t CounterHeating = 0;
uint8_t CounterMeasure = 0;
extern volatile bool startPIDReg;
extern volatile bool startBangBang;
extern volatile bool startPumping;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void initOneWayList();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_USART2_UART_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart2, odebranaWiadomosc, SIZE_OF_MSG);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim1);
	subroutine_Init(&kociol);
	//initOneWayList();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (startMeasure) {
			DS18B20_Full(&measuredTemperature);
			startMeasure = 0;
			if (measuredTemperature > 1.0) {
				uint8_t msg[100];
				sprintf(msg,
						(const char*) "Wartosc temperatury wynosi %.1lf\n\r",
						measuredTemperature);
				HAL_UART_Transmit_DMA(&huart2, msg, strlen(msg));
			}
		}
		if (startBangBang) {
			static uint8_t numberOfCycle = 0;
			grzanieRegDwustawna(kociol.heatingCycle[numberOfCycle][0],
					kociol.heatingCycle[numberOfCycle][1], kociol.hist,
					&numberOfCycle);
		} else if (startPIDReg) {

		} else if (startPumping) {
			pumping(kociol.pumpingTime);
		} else if (startPumpingWithoutTime) {

		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM1) {

		//przerwanie co 1 sekundę
		startCounterTime++;
		CounterPump++;
		CounterHeating++;
		CounterMeasure++;
		//pomiar co 3s
		if (CounterMeasure >= 3) {
			CounterMeasure = 0;
			startMeasure = 1;
		}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		HAL_UART_Receive_DMA(&huart2, odebranaWiadomosc, SIZE_OF_MSG);
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		setProgram(odebranaWiadomosc, &kociol);
		//convertToStucture(odebranaWiadomosc, &head);

	}
}
void initOneWayList() {
	head = (List*) malloc(sizeof(List));
	head = NULL;

}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
