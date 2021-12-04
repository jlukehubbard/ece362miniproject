#include <stdint.h>
#include <string.h>
#include "../CMSIS/device/stm32f0xx.h"

#define NOTELISTLENGTH 10

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

#define arrBuf
#ifdef arrBuf
uint8_t noteListIndex = 0;
uint16_t noteList[NOTELISTLENGTH];

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

		while (!(byte & 0x80))	{
			byte = getchar();
		}

		uint8_t type = byte & 0xf0;

		switch(type) {
		    case 0x80: { uint8_t note = getchar(); uint8_t velo = getchar;
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
		                 channel_pressure(); break;
		               }
		    case 0xe0: { uint8_t l = getchar(); uint8_t h = getchar();
		                 pitch_wheel_change((h<<7)+l); break;
		               }
		  }
	}
}
