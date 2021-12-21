/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
char RX_BUFFER[BUFSIZE];
int RX_BUFFER_HEAD, RX_BUFFER_TAIL;
uint8_t rx_data;

static uint8_t USART_DMA_tx_complete = 1;
/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
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
  __HAL_UART_ENABLE_IT (&huart2, UART_IT_RXNE);

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */
  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


int USART2_Dequeue(char* c){
	int  ret;
	ret = 0;
	*c = 0;

	HAL_NVIC_DisableIRQ(USART2_IRQn);

	if (RX_BUFFER_HEAD  !=  RX_BUFFER_TAIL){
		*c = RX_BUFFER[RX_BUFFER_TAIL ];
		RX_BUFFER_TAIL++;

		if (RX_BUFFER_TAIL  ==  BUFSIZE) {
			RX_BUFFER_TAIL = 0;

		}
		ret = 1;

	}

	HAL_NVIC_EnableIRQ(USART2_IRQn);
	return  ret;

}

uint8_t printf_eig(const char * text){
	uint16_t i;

	if ( USART_DMA_tx_complete== 0 ) return 0;

	for ( i=0; i<=255; i++){
		if ( *(text+i)=='\0' ) break;

	}

	if (i==0) return 1;

	HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);
	USART_DMA_tx_complete = 0;
	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

	HAL_UART_Transmit_DMA(&huart2 ,(uint8_t *) text, i);

	return 1;
}

char *gets_eig(char *s){
	char c;
	uint8_t i=0;
	while(1){
		if (USART2_Dequeue(&c) != 0){
			if ( c=='\r'){
				if ( i==0 ) {
					return NULL;
				} else {
					s[i]='\0';
					return s;
				}
			} else {
				s[i] = c;
				i++;
				if (i==255) {
					s[i] = '\0';
					break;
				}
			}
		}
	}
	return s;
}

void uart_start_reception(void){
	HAL_UART_Receive_DMA(&huart2 ,&rx_data, 1);
	return;
}


/* USER CODE END 1 */
