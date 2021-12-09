uint16_t midiMsg;
#define READY_TO_READ_FLAG 1
#define ALREADY_READ_FLAG 0

#define NOTE_LIST_MAX_SIZE 10
uint8_t noteList[NOTE_LIST_MAX_SIZE][2]; //0 NOTE, 1 VELOCITY
uint32_t stepList[NOTE_LIST_MAX_SIZE];
uint32_t offsetList[NOTE_LIST_MAX_SIZE];
uint8_t noteIndex;
enum midiStates {
    STATUS=0,
    WAITKEY,
    WAITVEL,
    ADDNOTE,
	PROGRAMCHANGE
};
uint8_t midiState;
#define BUFFER_MAX_SIZE 16
uint16_t usartBuffer[BUFFER_MAX_SIZE];
int usartHead;
int usartTail;
#define ERROR_EMPTY 0x00
#define ERROR_FULL 0x00

void NoteOn(uint8_t,uint8_t);
void NoteOff(uint8_t);
void adjustVel(void);
void displayNoteList(void);
void initMidiMsg(void);
void midiStateMachine(uint8_t);
void midiEventHandler(void);
