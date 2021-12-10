#include <string.h>

enum screenStates {
    MIDIUSART,
    KEYCODES,
    WAVES,
    OFF
};
uint8_t screenState;

void setupDmaOled(void);
void setupSpiOled(void);
void spiInitOled(void);
void enableDmaOled(void);
void displayOff(void);
void displayProgram();
