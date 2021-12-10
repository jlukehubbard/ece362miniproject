#include "stm32f0xx.h"
#include "dac.h"
#include <math.h>
#include "midi.h"

uint16_t drawbars[8] = {8192, 2048, 4096, 2048, 4096, 4096, 4096, 4096};

void initWavetable(void) {
    volume = 4096;
    for(int program = 0; program < WAVENUM; program++){
        for(int ii = 0; ii < N; ii++){
        	switch(program) {
        	case 0: //Sine
        		wavetable[program][ii] = 32767 * sin(2 * M_PI * ii / N);
        		break;
        	case 1: //Triangle
        		wavetable[program][ii] = 32767 * (2 / M_PI) * asin(sin(2 * M_PI * ii / N));
        		break;
        	case 2: //Sawtooth
        		wavetable[program][ii] = 32767 * (-2 / M_PI) * atan( cos(2 * M_PI * ii / N) / sin(2 * M_PI * ii / N) );
        		break;
        	case 3: //Combination
        		wavetable[program][ii] = 8192 * (2 / M_PI) * asin(sin(2 * M_PI * ii / N)) + 24576 * (-2 / M_PI) * atan( cos(2 * M_PI * ii / N) / sin(2 * M_PI * ii / N) );
        		break;
        	case 4:
        	    wavetable[program][ii] =
        	              (drawbars[0] * sin(2 * M_PI * ii / N))
        	            + (drawbars[1] * sin(4 * M_PI * ii / N))
        	            + (drawbars[2] * sin(6 * M_PI * ii / N))
        	            + (drawbars[3] * sin(12 * M_PI * ii / N))
        	            + (drawbars[4] * sin(16 * M_PI * ii / N))
        	            + (drawbars[5] * sin(20 * M_PI * ii / N))
        	            + (drawbars[6] * sin(24 * M_PI * ii / N))
        	            + (drawbars[7] * sin(32 * M_PI * ii / N));
        	}
        }
    }
}
void setupDac() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER4;
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~(DAC_CR_EN1|DAC_CR_EN2);
    DAC->CR &= ~DAC_CR_BOFF1;
    DAC->CR |= DAC_CR_TEN1;
    DAC->CR |= DAC_CR_TSEL1;
    DAC->CR |= DAC_CR_EN1;
}
void init_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 47;
    int countup = (1000000 / RATE) - 1;
    TIM7->ARR = countup;
    TIM7->CR1 |= TIM_CR1_CEN;
    TIM7->DIER |= TIM_DIER_UIE;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM7_IRQn;
    //PRIORITY
    //NVIC_SetPriority(TIM7_IRQn,0);
    dacScale=0;
}
void TIM7_IRQHandler(void) {
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
    TIM7->SR &= ~TIM_SR_UIF;
    uint32_t tmpsmp = 0;
    uint32_t sample = 0;
    dacScale++;

    for (int i = 0; i < NOTE_LIST_MAX_SIZE; i++) {
        if(noteList[i][0]==0) {continue;}
        offsetList[i] = offsetList[i] + (stepList[i] + (pitch * (stepList[i] >> 16)));
        if (offsetList[i] > (NSHIFT16)) {offsetList[i] -= NSHIFT16;}
        tmpsmp = wavetable[wavenum][offsetList[i]>>16];
        tmpsmp = (tmpsmp*(noteList[i][1]))>>6;
        sample += tmpsmp;
    }
    sample = sample>>3;
    sample *= volume;
    sample = sample>>16;
    sample += 2048;
    dacScale++;
    DAC->DHR12R1 = sample;
}
