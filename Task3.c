/*
 * Task3.c
 *
 *  Created on: Oct 29, 2020
 *      Author: You Hu
 */
#include "init.h"
#include <stdint.h>
#include <stdlib.h>

ADC_HandleTypeDef hADC3;
DAC_HandleTypeDef hDAC1;
DMA_HandleTypeDef dma2;
uint32_t x[1];
uint32_t x_now = 0;
uint32_t x_1 =0;
uint32_t x_2=0;
uint32_t putout=0;
uint16_t flag = 0;

void configureDAC();
void configureADC();
//uint16_t readvoltage();

// Main Execution Loop
int main(void)
{
	//Initialize the system
	Sys_Init();
	configureDAC();
	HAL_DAC_Start(&hDAC1, DAC_CHANNEL_1);
	configureADC();
	x[0] = 0;
	HAL_ADC_Start_DMA(&hADC3, (uint32_t*)x, 1);
	while(1){
		HAL_ADC_Start_DMA(&hADC3, (uint32_t*)x, 1);
//		while(!flag){
//		}
//		HAL_ADC_Stop_DMA(&hADC3);
//		flag = 0;
	}
}


void DMA2_Stream0_IRQHandler(){
	HAL_DMA_IRQHandler(&dma2);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	flag = 1;
	x_2 = x_1;
	x_1 = x_now;
	x_now = x[0];
	putout = 0.312500*x_now + 0.240385*x_1 + 0.312500*x_2 + 0.296875*putout;
	HAL_DAC_SetValue(&hDAC1, DAC_CHANNEL_1,DAC_ALIGN_12B_R, putout);
}
//




void configureADC()
{  //A3 =>PF10 (ADC3_IN8)
	__HAL_RCC_ADC3_CLK_ENABLE();

	hADC3.Instance = ADC3;
	hADC3.Init.Resolution = ADC_RESOLUTION_12B;
	hADC3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hADC3.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hADC3.Init.NbrOfConversion = 1;
	hADC3.Init.ContinuousConvMode = ENABLE;
	hADC3.Init.DiscontinuousConvMode = DISABLE;
	hADC3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hADC3.Init.DMAContinuousRequests = ENABLE;
	hADC3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hADC3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	HAL_ADC_Init(&hADC3);

	//ADC->CCR |= ADC_CCR_TSVREFE;
	ADC_ChannelConfTypeDef adcchan;
	adcchan.Channel = ADC_CHANNEL_8;
	adcchan.Rank = ADC_REGULAR_RANK_1;
	adcchan.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hADC3,&adcchan);

}


void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
    if(hadc->Instance == ADC3){
    	// Enable GPIO Clocks
       	__HAL_RCC_GPIOF_CLK_ENABLE();

    	// Initialize Pin
    	GPIO_InitStruct.Pin       = GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
    	GPIO_InitStruct.Pull      = GPIO_PULLUP;
    	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

       	__HAL_RCC_DMA2_CLK_ENABLE();
    	dma2.Instance = DMA2_Stream0;
    	dma2.Init.Channel = DMA_CHANNEL_2;
    	dma2.Init.Direction = DMA_PERIPH_TO_MEMORY;
		dma2.Init.PeriphInc = DMA_PINC_DISABLE;
		dma2.Init.MemInc = DMA_MINC_ENABLE;
		dma2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		dma2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		dma2.Init.Mode = DMA_CIRCULAR;
		dma2.Init.Priority = DMA_PRIORITY_LOW;
		dma2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&dma2);

		__HAL_LINKDMA(hadc, DMA_Handle,dma2);

		HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    }
}

void configureDAC(){
	__HAL_RCC_DAC_CLK_ENABLE();

	hDAC1.Instance = DAC;
	HAL_DAC_Init(&hDAC1);

	DAC_ChannelConfTypeDef dacchan;
	dacchan.DAC_Trigger = DAC_TRIGGER_NONE;
	dacchan.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&hDAC1, &dacchan, DAC_CHANNEL_1);
}

void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
    if(hdac->Instance == DAC){
    	// Enable GPIO Clocks
       	__HAL_RCC_GPIOA_CLK_ENABLE();

    	// Initialize Pin
    	GPIO_InitStruct.Pin       = GPIO_PIN_4;
        GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
    	GPIO_InitStruct.Pull      = GPIO_NOPULL;
    	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
  }



