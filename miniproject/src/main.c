#include "../CMSIS/device/stm32f0xx.h"
#include "../inc/MIDI.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI

#define WAVENUM 1
#define VOLUME 2048
#define N 1000
#define RATE 20000
int step = 0;
int offset = 0;
int volume = 2048;
short int wavetable[WAVENUM][N];
uint8_t wavenum = 0;
#define NOTELISTLENGTH 10
uint8_t noteListIndex = 0;
uint16_t noteList[NOTELISTLENGTH];

//OLED CHARACTER MAP
uint16_t oledDisp[34] =
{ 0x002, 0x200|'K' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' ,
  0x0c0, 0x200|'V' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' , 0x200|' ' };
const char hexToChar[] =
{ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46 };

//===========================================================================
// Configuring GPIO
//===========================================================================
void enable_ports(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(GPIO_MODER_MODER0|GPIO_MODER_MODER1|GPIO_MODER_MODER2);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR0|GPIO_PUPDR_PUPDR1|GPIO_PUPDR_PUPDR2);
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1|GPIO_PUPDR_PUPDR1_1|GPIO_PUPDR_PUPDR2_1;
}



//===========================================================================
// Button Interrupt
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
uint8_t getchar(void) {
     while (!(USART1->ISR & USART_ISR_RXNE)) { }
     int c = USART1->RDR;
     return c;
}

//#define arrBuf
#ifdef arrBuf

void init_noteList(){
    for(int i=0;i<NOTELISTLENGTH;i++){
        noteList[i] = 0x0000;
    }
}
void add_note(uint8_t keyCode, uint8_t velocity){
    if(noteListIndex >= NOTELISTLENGTH){
        return;
    }
    noteList[noteListIndex] = keyCode|(velocity<<8);
    noteListIndex++;
}
void remove_note(uint8_t hexCode){
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

#define llbuf
#ifdef llbuf

Node *notelist = newNode(NULL, NULL, NULL);

void note_off(uint8_t key, uint8_t velo) {
	notelist = listDelete(notelist, key);
}

void note_on(uint8_t key, uint8_t velo) {
	notelist = listInsert(notelist, key, velo);
}


#endif

#ifdef skeletonFunc
void note_off(int key, int velo)
{
}

void note_on(int key, int velo)
{
}

void key_pressure(int key, int val)
{
}

void control_change(int ctrl, int val)
{
}

void program_change(int prog)
{
}

void channel_pressure(int val)
{
}

void pitch_wheel_change(int val)
{
}
#endif


/*
Idle:
   while nothing has happened yet, do nothing (stay Idle).
   If a command is received, move on (go to Process Command).

Process Command:
   If command is not a note-on or note-off, return to Idle.
   Else:
      If command is a note-on, read in the key and velocity and go to
      Process Note.
      Else:
         Command is a note-off; read in the key and velocity, and if
         the key is the same as the currently playing key, go to End
         Note.
         If the key does not match, continue doing what was being done
         before.

Process Note:
   Based on the Key, find the note's directory entry in memory.
   Look up the Memory Addresses of the note.
   Play the attack once.
   Loop on Sustain until note-off or another note-on comes
   If next thing is a note-off,
      play the release once.
      go to End Note.
   Else (the next thing is a note-on)
      restart Process Note for the key associated with this new
      note-on

End Note:
   wait for release to finish playing.
   return to Idle.

   https://www.ocf.berkeley.edu/~jonc/project/150rept/150r2-1.html
 */

// MIDI State Machine
void start_MIDI_RX() {
	for (;;) {
		uint8_t byte = 0;

		do	{
			byte = getchar();
		} while (!(byte & 0x80) || (byte == 0xfe));

		uint8_t type = byte & 0xf0;

		switch(type) {
		    case 0x80: { uint8_t note = getchar(); uint8_t velo = getchar();
		                 note_off(note, velo); break;
		               }
		    case 0x90: { uint8_t note = getchar(); uint8_t velo = getchar();
		                 note_on(note, velo); break;
		               }
		    case 0xa0: { uint8_t note = getchar(); uint8_t value = getchar();
		                 key_pressure(note, value); break;
		               }
		    case 0xb0: { uint8_t ctrl = getchar(); uint8_t value = getchar();
		                 control_change(ctrl, value); break;
		               }
		    case 0xc0: { uint8_t prog = getchar();
		                 program_change(prog); break;
		               }
		    case 0xd0: { uint8_t value = getchar();
		                 channel_pressure(value); break;
		               }
		    case 0xe0: { uint8_t l = getchar(); uint8_t h = getchar();
		                 pitch_wheel_change((h<<7)+l); break;
		               }
		  }
	}
}
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

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

void check_on_off(void){
    uint8_t usartByte;
    uint8_t command;
    for(;;) {
        usartByte = getchar();
        if(usartByte == 0x90){
            command = usartByte;
            on_command(0x00);
        }
        else if(usartByte == 0x80){
            command = usartByte;
            off_command(0x00);
        }
        else{
            if(command == 0x90){on_command(usartByte);}
            else if(command == 0x80){off_command(usartByte);}
        }
        display_note_list();
    }
}
void on_command(uint8_t keyCode){
    if(keyCode == 0x00){keyCode = getchar();}
    uint8_t velocity = getchar();
    if(velocity == 0){remove_note(keyCode);} //IF RUNNING STATUS
    else{add_note(keyCode,velocity);}
}
void off_command(uint8_t keyCode){
    if(keyCode == 0x00){keyCode = getchar();}
    getchar();
    remove_note(keyCode);
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
// Main and supporting functions
//===========================================================================

int main(void)
{
	init_interrupts();
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
