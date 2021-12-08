#include "stm32f0xx.h"
#include "midi.h"
#include "display.h"

uint16_t oledDisp[34] =
{ 0x002, 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' ,
  0x0c0, 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' };
const char hexToChar[] =
{ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46 };

void nanoWait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void setupSpiOled() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER5|GPIO_MODER_MODER6|GPIO_MODER_MODER7|GPIO_MODER_MODER15);
    GPIOA->MODER |= (GPIO_MODER_MODER5_1|GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1|GPIO_MODER_MODER15_1);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFR5|GPIO_AFRL_AFR6|GPIO_AFRL_AFR7);
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFR15);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~(SPI_CR1_SPE);
    SPI1->CR1 |= SPI_CR1_BR|SPI_CR1_MSTR;
    SPI1->CR2 &= ~(SPI_CR2_DS);
    SPI1->CR2 = SPI_CR2_DS_3|SPI_CR2_DS_0|SPI_CR2_NSSP|SPI_CR2_SSOE|SPI_CR2_TXDMAEN;
    SPI1->CR1 |= SPI_CR1_SPE;
}
void setupDmaOled() {
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel3->CCR &= ~(DMA_CCR_EN|DMA_CCR_MSIZE|DMA_CCR_PSIZE);
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0|DMA_CCR_DIR|DMA_CCR_MINC;
    //DMA1_Channel3->CCR |= DMA_CCR_TCIE; //INTERRUPT
    DMA1_Channel3->CCR |= DMA_CCR_CIRC;
    DMA1_Channel3->CMAR = (uint32_t) &oledDisp;
    DMA1_Channel3->CPAR = (uint32_t) &SPI1->DR;
    DMA1_Channel3->CNDTR = 34;
    //NVIC->ISER[0] |= (1 << DMA1_Ch2_3_DMA2_Ch1_2_IRQn);
}
void spiOledCmd(unsigned int data) {
    while(!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR=data;
}
void spiInitOled() {
    screenState=KEYCODES;
    nanoWait(1000000);
    spiOledCmd(0x38);
    spiOledCmd(0x08);
    spiOledCmd(0x01);
    nanoWait(2000000);
    spiOledCmd(0x06);
    spiOledCmd(0x02);
    spiOledCmd(0x0c);
}
void enableDmaOled(void){
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void displayNoteList(){
    for(int i = 0; i < NOTE_LIST_MAX_SIZE; i++)
    {

        if(i < 5){
            if(noteList[i][0] == 0){
                oledDisp[3*i+2] = 0x200|' ';
                oledDisp[3*i+3] = 0x200|' ';
            }
            else{
            oledDisp[3*i+2] = 0x200|hexToChar[(noteList[i][0] & 0xf0)>>4];
            oledDisp[3*i+3] = 0x200|hexToChar[(noteList[i][0] & 0x0f)];
            }
        }else{
            if(noteList[i][0] == 0){
                oledDisp[3*(i-5)+19] = 0x200|' ';
                oledDisp[3*(i-5)+20] = 0x200|' ';
            }
            else{
                oledDisp[3*(i-5)+19] = 0x200|hexToChar[(noteList[i][0] & 0xf0)>>4];
                oledDisp[3*(i-5)+20] = 0x200|hexToChar[(noteList[i][0] & 0x0f)];
            }
        }
    }
}

void displayBuffer(){
    //FFFF FFFF
    int place = 2;
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[0] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[0] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[0] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[0] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';

    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[1] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[1] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[1] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[1] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';

    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[2] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[2] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[2] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[2] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';

    place = 19;

    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[3] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[3] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[3] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[3] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';

    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[4] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[4] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[4] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[4] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';

    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[5] & 0x000f)];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[5] & 0x00f0)>>4];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[5] & 0x0f00)>>8];
    oledDisp[place++] = 0x200|hexToChar[(usartBuffer[5] & 0xf000)>>12];
    oledDisp[place++] = 0x200|' ';
}

void displayOff(){
    for(int i=1;i<17;i++){
        oledDisp[i] = 0x200|' ';
    }
    for(int i=18;i<34;i++){
        oledDisp[i] = 0x200|' ';
    }
}
