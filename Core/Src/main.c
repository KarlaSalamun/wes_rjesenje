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
#include "i2c.h"
#include "i2s.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define BUFFER_SIZE 64

uint16_t buffer[BUFFER_SIZE];


#define KEY_TIME 3 // 3 seconds
#define TIMER_FREQ_SEC 1000000
#define KEYS_NUM 12
#define DECAY_CONSTANT 0.1
#define SAMPLE_RATE 48000.0

char rx_data;

struct key {
	double freq;
	uint32_t curr;
	uint32_t max_idx;
	char pressed;
	char tone;
	uint32_t timestamp;
	double samples[256];
};
struct key keys[KEYS_NUM];
uint16_t sample_num;
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void init_key(int idx, double freq, int curr, char pressed, char tone,
		uint32_t timestamp) {

	keys[idx].freq = freq;
	keys[idx].curr = curr;
	keys[idx].pressed = pressed;
	keys[idx].tone = tone;
	keys[idx].timestamp = timestamp;
	keys[idx].max_idx = SAMPLE_RATE / freq;

	for (int i = 0; i < keys[idx].max_idx; i++) {
		keys[idx].samples[i] = sin(2 * M_PI * freq * i
				/ SAMPLE_RATE);
	}
}

void key_timeout() {
	uint32_t now = get_time();
	for (int i = 0; i < KEYS_NUM; i++) {
		if (now - keys[i].timestamp < KEY_TIME * TIMER_FREQ_SEC) {
			keys[i].pressed = 0;
		}
	}
}

static inline uint16_t get_sample() {
	double result = 0;
	uint32_t press_cnt = 0;
	uint16_t retval;

	for (int i = 0; i < KEYS_NUM; i++) {
		if (keys[i].pressed) {
			press_cnt++;
			result += keys[i].samples[keys[i].curr++];
			if (keys[i].curr == keys[i].max_idx) {
				keys[i].curr = 0;
			}
		}
	}
	result /= press_cnt;
	retval = (10000 + result * 10000);

	return retval;
}

void init_keys() {
	init_key(0, 261.63, 0, 0, 'a', 0); // C4
	init_key(1, 277.18, 0, 0, 'w', 0);
	init_key(2, 293.66, 0, 0, 's', 0);
	init_key(3, 311.13, 0, 0, 'e', 0);
	init_key(4, 329.63, 0, 0, 'd', 0);
	init_key(5, 349.23, 0, 0, 'f', 0);
	init_key(6, 369.99, 0, 0, 't', 0);
	init_key(7, 392.00, 0, 0, 'g', 0);
	init_key(8, 415.30, 0, 0, 'z', 0);
	init_key(9, 440.00, 0, 0, 'h', 0);
	init_key(10, 466.16, 0, 0, 'u', 0);
	init_key(11, 493.88, 0, 0, 'j', 0); // H4
}

int main(void){
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	MX_I2S3_Init();
	MX_TIM2_Init();

	init_keys();
	configAudio();

	HAL_I2S_Transmit_DMA(&hi2s3, buffer, BUFFER_SIZE);

	while(1) {

	}
}

void on_key_pressed(char c) {
	for (int i = 0; i < KEYS_NUM; i++) {
		if (keys[i].tone == c) {
			keys[i].pressed = 1;
			break;
		}
	}
}

void on_key_released(char c) {
	for (int i = 0; i < KEYS_NUM; i++) {
		if (keys[i].tone == c) {
			keys[i].pressed = 0;
			break;
		}
	}
}

void USER_UART_IRQHandler(UART_HandleTypeDef *huart) {
	static uint8_t state = 0; // 0 -> released, 1 -> pressed
	if( huart ->Instance == USART2 ) {
		rx_data = __HAL_UART_FLUSH_DRREGISTER( huart );
		if( rx_data == 'r' ) {
			state = 0;
			return;
		}
		else if( rx_data == 'p' ) {
			state = 1;
			return;
		}
		if( state == 0 ) {
			on_key_released(rx_data);
		}
		else {
			on_key_pressed(rx_data);
		}

//		static char rx_head;
//		rx_head = RX_BUFFER_HEAD + 1;
//		if( rx_head == BUFSIZE ) {
//			rx_head = 0;
//		}
//		if( rx_head != RX_BUFFER_TAIL ) {
//			RX_BUFFER[RX_BUFFER_HEAD] = rx_data;
//			RX_BUFFER_HEAD = rx_head;
//		}
	}
}

static void fill_buffer(int start, int end) {
	uint16_t sample = 0;

	for (int i = start; i < end; i+=2) {
		sample = get_sample();
		buffer[i] = sample;
		buffer[i+1] = sample;
	}
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	fill_buffer(0, BUFFER_SIZE / 2);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	fill_buffer(BUFFER_SIZE / 2, BUFFER_SIZE);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  while (1){
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
