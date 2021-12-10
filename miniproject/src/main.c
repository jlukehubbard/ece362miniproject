/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/
#include "stm32f0xx.h"
#include "midi.h"
#include "display.h"
#include "dac.h"
#include <math.h>
#include "interrupts.h"

void initUsart1(void){

    //CONFIGURE PA9 AND PA10
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER9|GPIO_MODER_MODER10);
    GPIOA->MODER |= (GPIO_MODER_MODER9_1|GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFR9|GPIO_AFRH_AFR10);
    GPIOA->AFR[1] |= (0x1<<4|0x1<<8);

    //CONFIGURE USART1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->CR1 &= ~(USART_CR1_UE);
    USART1->CR1 &= ~(USART_CR1_M|USART_CR1_M<<8|USART_CR1_PCE|USART_CR1_OVER8);
    USART1->CR1 |= USART_CR1_RE; //RECIEVING
    USART1->CR1 |= USART_CR1_TE; //TRANSMITTING
    USART1->CR2 &= ~(USART_CR2_STOP);

    //TESTING
    USART1->CR3 |= USART_CR3_ONEBIT;
    //INTERRUPT ENABLE
    USART1->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0] |= 1 << USART1_IRQn;
    //PRIORITY
    NVIC_SetPriority(USART1_IRQn,0);


    USART1->BRR = 0x600;

    USART1->CR1 |= USART_CR1_UE;

    while((USART1->ISR & USART_ISR_TEACK) == 0); //TRANSMITTING
    while((USART1->ISR & USART_ISR_REACK) == 0); //RECIEVING
}

void USART1_IRQHandler(void) {
    if (((USART1 -> ISR) & USART_ISR_NE) || ((USART1 -> ISR) & USART_ISR_FE) || ((USART1 -> ISR) & USART_ISR_ORE)) {
        initUsartBuffer();
        initNoteList();
        initMidiMsg();
        USART1->ICR |= USART_ICR_ORECF|USART_ICR_FECF|USART_ICR_FECF;
    } else {
        midiStateMachine(USART1->RDR);
    }
}
int main(void)
{
    pedal = 0;
    wavenum = 0;

    initNoteList();
    initUsartBuffer();
    initUsart1();
    setupDmaOled();
    setupSpiOled();
    spiInitOled();
    enableDmaOled();

    initWavetable();
    setupDac();
    init_tim7();

    enable_ports();
    init_interrupts();
    displayProgram();

    for(;;) {
        if(usartHead != usartTail){
            midiEventHandler();
        }
        if(dacScale > 10000) {  //Every Half Second
            dacScale = 0;
            adjustVel();
        }
    }
}
