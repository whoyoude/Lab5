/*
 * Task4.c
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
DMA_HandleTypeDef dma1;
TIM_HandleTypeDef htim6;
uint16_t x[2];//Buffer for ADC
uint16_t y[1];//Buffer for DAC
uint32_t x_now = 0;
uint32_t x_1 = 0;
uint32_t x_2=0;
uint32_t putout=0;
uint16_t flag = 0;//Indicate the conversion status: 1 means buffer is half-filled;0 means buffer is ready to re-fill.

void configureDAC();
void configureADC();
void Init_timer_HAL();

// Main Execution Loop
int main(void)
{
	//Initialize the system
	Sys_Init();
	configureDAC();
	Init_timer_HAL();
	configureADC();

	//Initialize the buffer
	x[0] = 0;
	x[1] = 0;

	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hDAC1, DAC_CHANNEL_1);
	while(1){
		HAL_ADC_Start_DMA(&hADC3,(uint32_t*)x, 2);
	}
}


void Init_timer_HAL(){
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	//Set the TIM to be TIM6 and the corresponding prescaler and period
	htim6.Instance = TIM6;
	htim6.Init.Prescaler =5;
	htim6.Init.Period = 5;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim6);

	//Enable clock
	__HAL_RCC_TIM6_CLK_ENABLE();

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim6, &sClockSourceConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}

void DMA2_Stream0_IRQHandler(){//ADC
	HAL_DMA_IRQHandler(&dma2);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
	if(flag == 0){//Ensure the second hlaf of the buffer is empty.
	    flag = 1;
	    x_2 = x_1;
	    x_1 = x_now;
	    x_now = x[0];
	    putout = 0.312500*x_now + 0.240385*x_1 + 0.312500*x_2 + 0.296875*putout;
	    y[0]= putout;
	    HAL_DAC_Start_DMA(&hDAC1, DAC_CHANNEL_1,(uint32_t*) y,1,DAC_ALIGN_12B_R);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if (flag == 1){//Make sure buffer is half-filled
	    flag = 0;
	    x_2 = x_1;
	    x_1 = x_now;
	    x_now = x[1];
	    putout = 0.312500*x_now + 0.240385*x_1 + 0.312500*x_2 + 0.296875*putout;
	    y[0]= putout;
	    HAL_DAC_Start_DMA(&hDAC1, DAC_CHANNEL_1,(uint32_t*) y,1,DAC_ALIGN_12B_R);
	}
}
//



void configureADC()
{  //A3 =>PF10 (ADC3_IN8)
	__HAL_RCC_ADC3_CLK_ENABLE();

	//Initialize the ADC handler.
	hADC3.Instance = ADC3;
	hADC3.Init.Resolution = ADC_RESOLUTION_12B;
	hADC3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hADC3.Init.ScanConvMode = DISABLE;
	hADC3.Init.NbrOfConversion = 2;//Make two conversion at a time
	hADC3.Init.ContinuousConvMode = DISABLE;
	hADC3.Init.DiscontinuousConvMode = DISABLE;
	hADC3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hADC3.Init.DMAContinuousRequests = ENABLE;
	hADC3.Init.EOCSelection = 0;
	hADC3.Init.ExternalTrigConv = ADC_EXTERNALTRIG0_T6_TRGO;
	hADC3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	HAL_ADC_Init(&hADC3);

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
       	//Set the DMA which connect to ADC in circular mode
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

	//Connect ADC1 and its associated DMA.
	__HAL_LINKDMA(hadc, DMA_Handle,dma2);

	//Enable the interrupt of DMA.
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    }
}

void configureDAC(){
	__HAL_RCC_DAC_CLK_ENABLE();

	//Initialize the DAC handler.
	hDAC1.Instance = DAC;
	HAL_DAC_Init(&hDAC1);

	DAC_ChannelConfTypeDef dacchan;
	dacchan.DAC_Trigger = DAC_TRIGGER_T6_TRGO;//THe DAC event triggered by TIM6
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

    	//Set the DMA connect to DAC
    	 __HAL_RCC_DMA1_CLK_ENABLE();
    	 dma1.Instance = DMA1_Stream5;
    	 dma1.Init.Channel = DMA_CHANNEL_7;
    	 dma1.Init.Direction = DMA_MEMORY_TO_PERIPH;
         dma1.Init.PeriphInc = DMA_PINC_DISABLE;
    	 dma1.Init.MemInc = DMA_MINC_ENABLE;
    	 dma1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    	 dma1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    	 dma1.Init.Mode = DMA_CIRCULAR;
    	 dma2.Init.Priority = DMA_PRIORITY_LOW;
    	 dma2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    	 HAL_DMA_Init(&dma1);

    	 __HAL_LINKDMA(hdac, DMA_Handle1,dma1);
    }
  }
