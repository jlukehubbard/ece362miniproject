#include <stdint.h>
#include <string.h>
#include "../CMSIS/device/stm32f0xx.h"

void note_off(int key, int velo)
{
}

void note_on(int key, int velo)
{
}

void configure_USART_GPIO(void) {
	RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA -> MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
	GPIOA -> MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;

	GPIOA -> AFR[1] &= ~(GPIO_AFRH_AFR9 | GPIO_AFRH_AFR10);
	GPIOA -> AFR[1] |= GPIO_AFRH_AFR9_0 | GPIO_AFRH_AFR10_0;


}

void init_USART(void) {

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

}
