
//=============================================================================

#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "interrupts.h"

#define WAVENUM 4
#define VOLUME 2048
#define N 1000
#define RATE 20000
int step = 0;
int offset = 0;
int volume = 2048;
short int wavetable[WAVENUM][N];
uint8_t wavenum = 0;

//===========================================================================
// 2.1 Configuring GPIO
//===========================================================================
void enable_ports(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(GPIO_MODER_MODER0|GPIO_MODER_MODER1|GPIO_MODER_MODER2);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR0|GPIO_PUPDR_PUPDR1|GPIO_PUPDR_PUPDR2);
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1|GPIO_PUPDR_PUPDR1_1|GPIO_PUPDR_PUPDR2_1;
}
extern const char font[];

//===========================================================================
// 2.3 Button Interrupt
//===========================================================================
void init_interrupts(void){
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    //Configure GPIO ports for interrupt on C
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0|SYSCFG_EXTICR1_EXTI2);
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI4);
    SYSCFG->EXTICR[0] |= (0x2|0x2<<8);
    SYSCFG->EXTICR[1] |= (0X2);
    //Rising edge trigger
    EXTI->RTSR |= (EXTI_RTSR_TR0|EXTI_RTSR_TR2|EXTI_RTSR_TR4);
    //Disable falling edge trigger
    EXTI->FTSR &= ~(EXTI_FTSR_TR0|EXTI_FTSR_TR2|EXTI_FTSR_TR4);
    //Unmask Interrupt
    EXTI->IMR |= (EXTI_IMR_MR0|EXTI_IMR_MR2|EXTI_IMR_MR4);

    //Enable Interrupts
    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
    NVIC_EnableIRQ(EXTI4_15_IRQn);

    NVIC_SetPriority(EXTI0_1_IRQn,1); //Change priority based on sound generation
    NVIC_SetPriority(EXTI2_3_IRQn,1);
    NVIC_SetPriority(EXTI4_15_IRQn,1);
}

void EXTI0_1_IRQHandler(void){
    EXTI->PR |= EXTI_PR_PR0;
    if(pedal == 1){
        pedal = 0;
        return;
    }
    pedal = 1;
}
