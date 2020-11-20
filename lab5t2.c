/*
 * Task3.c
 *
 *  Created on: Oct 9, 2020
 *      Author: You Hu
 */
#include "init.h"
#include <stdint.h>
#include<stdlib.h>


char user_m[1];
char user_s[1];
UART_HandleTypeDef uart_init1;
SPI_HandleTypeDef spi_init;
DMA_HandleTypeDef handlerx;
DMA_HandleTypeDef handletx;

void DMA_init(){

	__HAL_RCC_DMA1_CLK_ENABLE();

	handlerx.Instance=DMA1_Stream3;
	handlerx.Init.Channel=DMA_CHANNEL_0;
	handlerx.Init.Direction=DMA_PERIPH_TO_MEMORY;
	handlerx.Init.PeriphInc=DMA_PINC_DISABLE;
	handlerx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
	handlerx.Init.MemInc=DMA_MINC_ENABLE;
	handlerx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
	handlerx.Init.Mode=DMA_NORMAL;
	handlerx.Init.Priority=DMA_PRIORITY_MEDIUM;
	__HAL_LINKDMA(&spi_init,hdmarx,handlerx);
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);


	HAL_DMA_DeInit(&handlerx);
	HAL_DMA_Init(&handlerx);




	handletx.Instance=DMA1_Stream4;
	handletx.Init.Channel=DMA_CHANNEL_0;
	handletx.Init.Direction=DMA_MEMORY_TO_PERIPH;
	handletx.Init.PeriphInc=DMA_PINC_DISABLE;
	handletx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
	handletx.Init.MemInc=DMA_MINC_ENABLE;
	handletx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
	handletx.Init.Mode=DMA_NORMAL;
	handletx.Init.Priority=DMA_PRIORITY_MEDIUM;
	__HAL_LINKDMA(&spi_init,hdmatx,handletx);
	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);


	HAL_DMA_DeInit(&handletx);
	HAL_DMA_Init(&handletx);

}

void DMA1_Stream3_IRQHandler(){
	HAL_DMA_IRQHandler(&handlerx);
	printf("\033[13;H");
	fflush(stdout);
	printf("From SPI:\r\n");
	printf("%c\r\n",user_m[0]);
	user_m[0] =0;
}

void DMA1_Stream4_IRQHandler(){
	HAL_DMA_IRQHandler(&handletx);
	user_s[0] =0;
}

void GPIO_set_SPI();
//void read_input();
//void GPIO_set_UART();



int main(){
	Sys_Init();
	HAL_Init();
    SystemClock_Config();

	GPIO_set_SPI();
	DMA_init();

	while(1){
		user_s[0] = getchar();
		printf("\033[2J\033[;H");
		fflush(stdout);
		printf("From computer:\r\n%c",user_s[0]);
		fflush(stdout);

		HAL_SPI_TransmitReceive_DMA(&spi_init, (uint8_t *)user_s,(uint8_t *)user_m,1);

		//}
	}
}


void GPIO_set_SPI(){
	__SPI2_CLK_ENABLE();

	spi_init.Instance = SPI2;
	spi_init.Init.Mode = SPI_MODE_MASTER;
	spi_init.Init.Direction = SPI_DIRECTION_2LINES;
	spi_init.Init.DataSize = SPI_DATASIZE_8BIT;
	spi_init.Init.CLKPolarity = SPI_POLARITY_HIGH;
	spi_init.Init.CLKPhase = SPI_PHASE_2EDGE;
	spi_init.Init.NSS = SPI_NSS_HARD_OUTPUT;
	spi_init.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	spi_init.Init.FirstBit = SPI_FIRSTBIT_LSB;
	spi_init.Init.TIMode = SPI_TIMODE_DISABLE;
	spi_init.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	HAL_SPI_Init(&spi_init);

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpio_init;
	gpio_init.Pin = GPIO_PIN_14 | GPIO_PIN_15;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_HIGH;
    gpio_init.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    gpio_init.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOA, &gpio_init);
}

//void read_input(){
//
//}
