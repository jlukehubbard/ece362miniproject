#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI

//===========================================================================
// VARIABLES
//===========================================================================
#define NOTELISTLENGTH 10
#define WAVENUM 1
#define VOLUME 2048
#define N 1000
#define RATE 20000
int step = 0;
int offset = 0;
int volume = 2048;
short int wavetable[WAVENUM][N];
uint8_t wavenum = 0;
//OLED CHARACTER MAP
uint16_t oledDisp[34] =
{ 0x002, 0x200|'K' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' ,
  0x0c0, 0x200|'V' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' };
const char hexToChar[] =
{ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46 };

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

//===========================================================================
// USART MIDI FUNCTIONS
//===========================================================================
void init_usart1(void){

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

    USART1->BRR = 0x600;

    USART1->CR1 |= USART_CR1_UE;

    while((USART1->ISR & USART_ISR_TEACK) == 0); //TRANSMITTING
    while((USART1->ISR & USART_ISR_REACK) == 0); //RECIEVING
}
int getchar(void) {
     while (!(USART1->ISR & USART_ISR_RXNE)) { }
     int c = USART1->RDR;
     return c;
}

//#define circBuf
#ifdef circBuf
uint8_t noteListFront = 0;
uint8_t noteListRear = 1;
uint8_t noteList[NOTELISTLENGTH] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

void add_note(uint8_t hexCode){
    if (noteListFront == noteListRear) {
        noteListFront = (noteListFront + 1) % NOTELISTLENGTH;
    }
    noteList[noteListRear] = hexCode;
    noteListRear = (noteListRear + 1) % NOTELISTLENGTH;
}
void remove_note(uint8_t hexCode){
    for (int i = 0; i < NOTELISTLENGTH; i++) {
        if (noteList[(i + noteListFront) % NOTELISTLENGTH] == hexCode) {
            for (int j = (i + noteListFront) % NOTELISTLENGTH; i < )
        }
    }
}

#endif

#define arrBuf
#ifdef arrBuf
uint8_t noteListIndex = 0;
uint16_t noteList[NOTELISTLENGTH];

void init_noteList(){
    for(int i=0;i<NOTELISTLENGTH;i++){
        noteList[i] = 0x0000;
    }
}
void add_note(uint16_t hexCode){
    if(noteListIndex >= NOTELISTLENGTH){
        return;
    }
    noteList[noteListIndex] = hexCode;
    noteListIndex++;
}
void remove_note(uint16_t hexCode){
    for(int i=0;i<NOTELISTLENGTH;i++){
        if((noteList[i]&0xff) == hexCode){
            uint16_t tmp = 0x0000;
            noteList[i] = tmp;
            for(int ii=i;ii<NOTELISTLENGTH-1;ii++){
                tmp = noteList[ii + 1];
                noteList[ii + 1] = noteList[ii];
                noteList[ii] = tmp;
            }
            noteListIndex--;
            return;
        }
    }
}
#endif
void display_note_list(){
    for(int i=0;i<5;i++){
        //SHOW KEYCODE
        oledDisp[3*i + 2] = 0x200|' ';
        oledDisp[3*i + 3] = 0x200|hexToChar[(noteList[i] & 0xf0)>>4];
        oledDisp[3*i + 4] = 0x200|hexToChar[(noteList[i] & 0x0f)];

        //SHOW VELOCITY
        oledDisp[3*i + 19] = 0x200|' ';
        oledDisp[3*i + 20] = 0x200|hexToChar[((noteList[i]>>8) & 0xf0)>>4];;
        oledDisp[3*i + 21] = 0x200|hexToChar[((noteList[i]>>8) & 0x0f)];
    }
}
void check_on_off(){
    for(;;) {
        int c = getchar();
        if(c == 0x90){
            c = getchar(); //KEY
            c |= (getchar()<<8); //VELOCITY
            add_note(c);
        }
        if(c == 0x80){
            c = getchar();
            remove_note(c); //KEY
        }
        display_note_list();
    }
}

//===========================================================================
// SPI OLED FUNCTIONS
//===========================================================================
void setup_spi_oled() {
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
void setup_dma_oled() {
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel3->CCR &= ~(DMA_CCR_EN|DMA_CCR_MSIZE|DMA_CCR_PSIZE);
    DMA1_Channel3->CCR |= DMA_CCR_CIRC|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0|DMA_CCR_DIR|DMA_CCR_MINC;
    DMA1_Channel3->CMAR = (uint32_t) &oledDisp;
    DMA1_Channel3->CPAR = (uint32_t) &SPI1->DR;
    DMA1_Channel3->CNDTR = 34;
}
void spi_oled_cmd(unsigned int data) {
    while(!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR=data;
}
void spi_init_oled() {
    nano_wait(1000000);
    spi_oled_cmd(0x38);
    spi_oled_cmd(0x08);
    spi_oled_cmd(0x01);
    nano_wait(2000000);
    spi_oled_cmd(0x06);
    spi_oled_cmd(0x02);
    spi_oled_cmd(0x0c);
}
void enable_dma_oled() {
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

//===========================================================================
// DAC FUNCTIONS
//===========================================================================

void init_wavetable(void) {
    for(int i=0; i < WAVENUM; i++){
        for(int ii=0; ii < N; ii++){
            wavetable[i][ii] = 32767 * sin(2 * M_PI * ii / N);
        }
    }
}

void set_freq(float f) {
    step = f * N / RATE * (1<<16);
}

int note_to_step(uint16_t hexCode){
    float freq = (2<<(((hexCode&0xff) - 69)/12)) * 440;
    return((freq * N)/(RATE * (1<<16)));
}

int note_to_volume(uint16_t hexCode){
    return((hexCode>>8) * 16);
}

void set_volume(int vol) {
    volume = vol;
}

void setup_dac() {
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
}

void TIM7_IRQHandler(void) {
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
    TIM7->SR &= ~TIM_SR_UIF;
    //step = note_to_step(noteList[0]);
    volume = note_to_volume(noteList[0]);
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

int main(void)
{
    // SPI OLED
    setup_dma_oled();
    setup_spi_oled();
    spi_init_oled();
    enable_dma_oled();

    // DAC AND TIMER
    init_wavetable();
    setup_dac();

    // USART MIDI
    init_noteList();
    init_usart1();

    init_tim7();

    check_on_off();
}
