#include "stm32f0xx.h"
#include "midi.h"
#include "display.h"
#include "dac.h"
#include <math.h>
#include "interrupts.h"

uint8_t prog = 0;

void initUsartBuffer(void){
    for(int i=0;i<BUFFER_MAX_SIZE;i++){
        usartBuffer[i] = 0x00;
    }
    usartHead = 0;
    usartTail = 0;
}
void initMidiMsg(void){
    midiMsg=0;
    midiState=STATUS;
}

uint16_t fifoRead(void) {
   usartTail = (usartTail + 1) % BUFFER_MAX_SIZE;
   return usartBuffer[usartTail];
}
uint16_t fifoWrite(uint16_t val) {
   if (usartHead + 1 == usartTail) return ERROR_FULL;
   usartHead = (usartHead + 1) % BUFFER_MAX_SIZE;
   usartBuffer[usartHead] = val;
   return usartBuffer[usartHead];
}

void initNoteList(void){
    for(int i;i<NOTE_LIST_MAX_SIZE;i++){
        noteList[i][0]=0;
        noteList[i][1]=0;
        noteList[i][2]=0;
        noteList[i][3]=0;
    }
    noteIndex = 0;
}
void midiStateMachine(uint8_t byte){
    switch(midiState){
    case STATUS:    if(byte == 0x90) {midiState=WAITKEY; midiMsg |= (1<<7);}
                    else if(byte == 0x80) {midiState=WAITKEY; midiMsg &= ~(1<<7);}
                    break;
    case WAITKEY:   midiMsg|=byte<<8; midiState=WAITVEL; break;
    case WAITVEL:   midiMsg|=byte&0x7F; midiState=ADDNOTE;
    case ADDNOTE:   fifoWrite(midiMsg);
                    initMidiMsg();
                    midiState = STATUS; break;
    default:        initUsartBuffer();
    }
}

void midiEventHandler() {
    uint16_t msg = fifoRead();
    if(msg & (1<<7)) NoteOn(msg>>8,(msg & 0x7F));
    else if(pedal == 1) NoteOff(msg>>8);
    //switch(screenState){
    //    MIDIUSART: displayMidiMsg(); break;
    //    KEYCODES: displayNoteList(); break;
    //    default: displayOff(); break;
    //}
    //displayMidiMsg();
    //displayBuffer();
    //displayNoteList();
    midiState = STATUS;
}

void NoteOn(uint8_t key, uint8_t vel) {
    NoteOff(key);
    if(vel==0) return;
    if(noteIndex >= NOTE_LIST_MAX_SIZE){
        noteIndex = noteIndex - NOTE_LIST_MAX_SIZE;
    }
    noteList[noteIndex][0] = key;
    noteList[noteIndex][1] = vel;
    stepList[noteIndex] = (pow(1.059,(key-69)) * 440) * NSHIFT16 / RATE;
    offsetList[noteIndex] = 0;
    noteIndex++;
}

void NoteOff(uint8_t key) {
    for(int i=0;i<NOTE_LIST_MAX_SIZE;i++){
        if(key==noteList[i][0]){
            noteList[i][0] = 0x00;
            noteList[i][1] = 0x00;
        }
    }
}

void adjustVel() {
    for(int i=0; i<NOTE_LIST_MAX_SIZE;i++){
        noteList[i][1] >>= 1;
        if(noteList[i][1] == 0) {
            //NoteOff
            noteList[i][0] = 0x00;
            noteList[i][1] = 0x00;
        }
    }
}
