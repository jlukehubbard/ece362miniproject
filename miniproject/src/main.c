
//=============================================================================

#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI

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
    wavenum = (++wavenum % 4);
}

void EXTI2_3_IRQHandler(void){
    EXTI->PR |= EXTI_PR_PR2;
    if (volume < VOLUME){
        volume += 128;
    }
}

void EXTI4_15_IRQHandler(void){
    EXTI->PR |= EXTI_PR_PR4;
    if (volume > 0){
        volume -= 128;
    }
}

//===========================================================================
// 2.5 Output Sine Wave
//===========================================================================

void init_wavetable(void) {
    for(int i=0; i < WAVENUM; i++){
        for(int ii=0; ii < N; ii++){
            wavetable[i][ii] = 32767 * sin(2 * (i+1) * M_PI * ii / N);
        }
    }
    //for(int ii=0; ii < N; ii++){
    //    wavetable[WAVENUM - 1][ii] = -32767 + ((65534/20000) * (ii % 20000));
    //}
}

void set_freq(float f) {
    step = f * N / RATE * (1<<16);
}

void set_volume(int vol) {
	volume = vol;
}

void setup_dac() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER4;
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_EN1;
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
}

void TIM7_IRQHandler(void) {
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
    TIM7->SR &= ~TIM_SR_UIF;
    offset += step;
    if (offset>>16 > N){
        offset -= N<<16;
    }
    int sample = wavetable[wavenum][offset>>16];
    sample *= volume;
    sample = sample>>16;
    sample += 2048;
    DAC->DHR12R1 = sample;
}


//===========================================================================
// Main and supporting functions
//===========================================================================
int main(void)
{
    init_interrupts();
    init_wavetable();
    set_freq(150);
    setup_dac();
    init_tim7();

}

const char font[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, // 32: space
    0x86, // 33: exclamation
    0x22, // 34: double quote
    0x76, // 35: octothorpe
    0x00, // dollar
    0x00, // percent
    0x00, // ampersand
    0x20, // 39: single quote
    0x39, // 40: open paren
    0x0f, // 41: close paren
    0x49, // 42: asterisk
    0x00, // plus
    0x10, // 44: comma
    0x40, // 45: minus
    0x80, // 46: period
    0x00, // slash
    // digits
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67,
    // seven unknown
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    // Uppercase
    0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x6f, 0x76, 0x30, 0x1e, 0x00, 0x38, 0x00,
    0x37, 0x3f, 0x73, 0x7b, 0x31, 0x6d, 0x78, 0x3e, 0x00, 0x00, 0x00, 0x6e, 0x00,
    0x39, // 91: open square bracket
    0x00, // backslash
    0x0f, // 93: close square bracket
    0x00, // circumflex
    0x08, // 95: underscore
    0x20, // 96: backquote
    // Lowercase
    0x5f, 0x7c, 0x58, 0x5e, 0x79, 0x71, 0x6f, 0x74, 0x10, 0x0e, 0x00, 0x30, 0x00,
    0x54, 0x5c, 0x73, 0x7b, 0x50, 0x6d, 0x78, 0x1c, 0x00, 0x00, 0x00, 0x6e, 0x00
};
