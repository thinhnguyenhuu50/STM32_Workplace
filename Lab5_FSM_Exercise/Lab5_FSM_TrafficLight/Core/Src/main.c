/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "led_7seg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// graphic position
#define CX_GREEN1 	160
#define CY_GREEN1 	94
#define CX_RED1 	160
#define CY_RED1 	40
#define CX_YELLOW1 	160
#define CY_YELLOW1 	67
#define CX_GREEN2 	30
#define CY_GREEN2 	264
#define CX_RED2 	30
#define CY_RED2 	210
#define CX_YELLOW2 	30
#define CY_YELLOW2 	237
#define RADIUS 		12

// State of Traffic light
#define INIT 				0
#define PHASE1_GREEN        1
#define PHASE1_YELLOW       2
#define PHASE2_GREEN        3
#define PHASE2_YELLOW       4


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
int timeOfGreenPhase1 = 25;
int timeOfYellowPhase1 = 3;
int timeOfGreenPhase2 = 20;
int timeOfYellowPhase2 = 3;
int timeOfLight = 0;
int cntOfLight = 0;
int statusOfLight = INIT;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();

void AppTrafficLight();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//int count1_s = 0;
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
	MX_TIM2_Init();
	MX_SPI1_Init();
	MX_FSMC_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */
	system_init();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		while (!timer2_flag);
		timer2_flag = 0;

		button_scan();
		AppTrafficLight();
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
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void system_init() {
	HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);

	led_7seg_init();
	button_init();
	lcd_init();

	timer2_init();
	timer2_set(50);
	timer4_init();
	timer4_set(1);

	HAL_Delay(1000);
}

void phase1Green() {
	lcd_draw_circle(CX_GREEN1, CY_GREEN1, GREEN, RADIUS, 1);
	lcd_draw_circle(CX_RED1, CY_RED1, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW1, CY_YELLOW1, 0x6351, RADIUS, 1);

	lcd_draw_circle(CX_GREEN2, CY_GREEN2, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED2, CY_RED2, RED, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW2, CY_YELLOW2, 0x6351, RADIUS, 1);
}

void phase1Yellow() {
	lcd_draw_circle(CX_GREEN1, CY_GREEN1, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED1, CY_RED1, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW1, CY_YELLOW1, YELLOW, RADIUS, 1);

	lcd_draw_circle(CX_GREEN2, CY_GREEN2, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED2, CY_RED2, RED, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW2, CY_YELLOW2, 0x6351, RADIUS, 1);
}

void phase2Green() {
	lcd_draw_circle(CX_GREEN2, CY_GREEN2, GREEN, RADIUS, 1);
	lcd_draw_circle(CX_RED2, CY_RED2, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW2, CY_YELLOW2, 0x6351, RADIUS, 1);

	lcd_draw_circle(CX_GREEN1, CY_GREEN1, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED1, CY_RED1, RED, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW1, CY_YELLOW1, 0x6351, RADIUS, 1);
}

void phase2Yellow() {
	lcd_draw_circle(CX_GREEN2, CY_GREEN2, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED2, CY_RED2, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW2, CY_YELLOW2, YELLOW, RADIUS, 1);

	lcd_draw_circle(CX_GREEN1, CY_GREEN1, 0x6351, RADIUS, 1);
	lcd_draw_circle(CX_RED1, CY_RED1, RED, RADIUS, 1);
	lcd_draw_circle(CX_YELLOW1, CY_YELLOW1, 0x6351, RADIUS, 1);
}

void AppTrafficLight()
{
    cntOfLight = (cntOfLight + 1)%20;
    if (cntOfLight == 0){
    	//every 1s
    	timeOfLight --;
    }

    switch (statusOfLight)
    {
		case INIT:
			lcd_clear(BLACK);
			lcd_fill(0, 0, 240, 20, BLUE);
			lcd_show_string_center(0, 0, "Traffic light", WHITE, BLUE, 16, 0);
			lcd_fill(0, 300, 240, 320, BLUE);
			lcd_show_picture(0, 20, 240, 280, gImage_traffic);

			timeOfLight = timeOfGreenPhase1;
			statusOfLight = PHASE1_GREEN;
			break;
        case PHASE1_GREEN:
        	//set light
        	phase1Green();
            //print status
            lcd_show_string_center(0, 302, "PHASE1 GREEN ", WHITE, BLUE, 16, 0);
            //print counter phase1
    		lcd_show_int_num(75, 50, timeOfLight / 10, 1, RED, 0x47e8, 32);
    		lcd_show_int_num(90, 50, timeOfLight % 10, 1, RED, 0x47e8, 32);
    		//print counter phase2
    		lcd_show_int_num(15, 140, (timeOfLight + timeOfYellowPhase1) / 10, 1, RED,
    				0x47e8, 32);
    		lcd_show_int_num(30, 140, (timeOfLight + timeOfYellowPhase1) % 10, 1, RED,
    				0x47e8, 32);

            if (timeOfLight == 0)
            {
            	//timeout
                statusOfLight = PHASE1_YELLOW;
                timeOfLight = timeOfYellowPhase1;
            }
            break;
        case PHASE1_YELLOW:
        	//set light
        	phase1Yellow();
            //print status
            lcd_show_string_center(0, 302, "PHASE1 YELLOW", WHITE, BLUE, 16, 0);
            //print counter phase1
    		lcd_show_int_num(75, 50, timeOfLight / 10, 1, RED, 0x47e8, 32);
    		lcd_show_int_num(90, 50, timeOfLight % 10, 1, RED, 0x47e8, 32);
    		//print counter phase1
    		lcd_show_int_num(15, 140, timeOfLight / 10, 1, RED,
    				0x47e8, 32);
    		lcd_show_int_num(30, 140, timeOfLight % 10, 1, RED,
    				0x47e8, 32);

            if (timeOfLight == 0)
            {
            	//timeout
                statusOfLight = PHASE2_GREEN;
                timeOfLight = timeOfGreenPhase2;
            }            break;
        case PHASE2_GREEN:
        	//set light
        	phase2Green();
            //print status
            lcd_show_string_center(0, 302, "PHASE2 GREEN ", WHITE, BLUE, 16, 0);
            //print counter phase1
    		lcd_show_int_num(75, 50, (timeOfLight + timeOfYellowPhase2) / 10, 1, RED, 0x47e8, 32);
    		lcd_show_int_num(90, 50, (timeOfLight + timeOfYellowPhase2) % 10, 1, RED, 0x47e8, 32);
    		//print counter phase2
    		lcd_show_int_num(15, 140, timeOfLight / 10, 1, RED,
    				0x47e8, 32);
    		lcd_show_int_num(30, 140, timeOfLight % 10, 1, RED,
    				0x47e8, 32);

            if (timeOfLight == 0)
            {
            	//timeout
                statusOfLight = PHASE2_YELLOW;
                timeOfLight = timeOfYellowPhase2;
            }
            break;
        case PHASE2_YELLOW:
        	//set light
        	phase2Yellow();
            //print status
            lcd_show_string_center(0, 302, "PHASE2 YELLOW", WHITE, BLUE, 16, 0);
            //print counter phase1
    		lcd_show_int_num(75, 50, timeOfLight / 10, 1, RED, 0x47e8, 32);
    		lcd_show_int_num(90, 50, timeOfLight % 10, 1, RED, 0x47e8, 32);
    		//print counter phase2
    		lcd_show_int_num(15, 140, (timeOfLight) / 10, 1, RED,
    				0x47e8, 32);
    		lcd_show_int_num(30, 140, (timeOfLight) % 10, 1, RED,
    				0x47e8, 32);

            if (timeOfLight == 0)
            {
            	//timeout
                statusOfLight = PHASE1_GREEN;
                timeOfLight = timeOfGreenPhase1;
            }
            break;
        default:
            statusOfLight = PHASE1_GREEN;
            break;
    }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
