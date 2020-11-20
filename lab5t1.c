#include "init.h" // Always need init.h, otherwise nothing will work.
#include<stdint.h>
#include<stdlib.h>

DMA_HandleTypeDef handle;

uint8_t source_buffer[1000];
uint8_t dest_buffer[1000];
int DMA_l;

int equal(uint8_t *a, uint8_t *b, int size){
	for(int i=0; i<size;i++){
		if(a[i]!=b[i]) return 0;
	}
	return 1;
}

void DMA_init(){
	__HAL_RCC_DMA2_CLK_ENABLE();

	handle.Instance=DMA2_Stream0;
	handle.Init.Channel=DMA_CHANNEL_0;
	handle.Init.Direction=DMA_MEMORY_TO_MEMORY;
	handle.Init.PeriphInc=DMA_PINC_ENABLE;
	handle.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
	handle.Init.MemInc=DMA_MINC_ENABLE;
	handle.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
	handle.Init.Mode=DMA_NORMAL;
	handle.Init.Priority=DMA_PRIORITY_MEDIUM;

	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);


	HAL_DMA_DeInit(&handle);
	HAL_DMA_Init(&handle);

}

void DMA2_Stream0_IRQHandler(){
	uint32_t cycles = DWT->CYCCNT;
	HAL_DMA_IRQHandler(&handle);
	if(equal(source_buffer,dest_buffer,DMA_l+1)){
		printf("memory length:\t%d\tcycle:\t%u\n\r",(DMA_l+1)*100,cycles);
	}
	DMA_l++;
	if(DMA_l<10){
		DWT->CYCCNT = 0;
		HAL_DMA_Start_IT(&handle, source_buffer, dest_buffer, (DMA_l+1)*100);
	}
}

int main(void)
{
	Sys_Init();
	DMA_init();
	// Enable the DWT_CYCCNT register
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	printf("\033[2J\033[;H");
	fflush(stdout);

	for(int i=0;i<1000;i++){//make src a 1-10 array
		source_buffer[i]=i;
	}

	//generate software transfer for buffer length=100, 200, ..., 1000
	printf("software:\n\r");
	for(int i=0;i<10;i++){
		DWT->CYCCNT = 0;
		for(int j=0;j<(i+1)*100;j++){
			dest_buffer[j]=source_buffer[j];
		}
		uint32_t cycles = DWT->CYCCNT;
		if(equal(source_buffer,dest_buffer,(i+1)*100)){
			printf("memory length:\t%d\tcycle:\t%u\n\r",(i+1)*100,cycles);
		}
	}


	//generate DMA transfer for buffer length=100, 200, ..., 1000
	printf("\nDMA:\n\r");
	DMA_l=0;
	DWT->CYCCNT = 0;
	HAL_DMA_Start_IT(&handle, source_buffer, dest_buffer, (DMA_l+1)*100);
	//continue in interrupt

	while(1);
}
