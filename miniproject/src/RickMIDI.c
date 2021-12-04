#include <stdint.h>
#include <string.h>
#include "../inc/MIDI.h"

#define WEAK __attribute__((weak))

//====<Weak symbols for empty functions.  Override in other modules>=========

void WEAK note_off(int time, int chan, int key, int velo)
{
}

void WEAK note_on(int time, int chan, int key, int velo)
{
}

void WEAK key_pressure(int time, int chan, int key, int val)
{
}

void WEAK control_change(int time, int chan, int ctrl, int val)
{
}

void WEAK program_change(int time, int chan, int prog)
{
}

void WEAK channel_pressure(int time, int chan, int val)
{
}

void WEAK pitch_wheel_change(int time, int chan, int val)
{
}

//===========================================================================
// Parser support functions.
//===========================================================================
int intvar(const uint8_t **pp)
{
  int x = 0;
  do {
    x = (x << 7) | ((**pp) & 0x7f);
  } while( *((*pp)++) & 0x80);
  return x;
}
int int1(const uint8_t **pp)
{
  int x = 0;
  x = (x<<8) | *((*pp)++);
  return x;
}
int int2(const uint8_t **pp)
{
  int x = 0;
  x = (x<<8) | *((*pp)++);
  x = (x<<8) | *((*pp)++);
  return x;
}
int int4(const uint8_t **pp)
{
  int x = 0;
  x = (x<<8) | *((*pp)++);
  x = (x<<8) | *((*pp)++);
  x = (x<<8) | *((*pp)++);
  x = (x<<8) | *((*pp)++);
  return x;
}
int match(const uint8_t *p, const char *s)
{
  int len = strlen(s);
  if (strncmp(s, (char *)p, len) != 0)
    return 0;
  return 1;
}
int eat(const uint8_t **pp, const char *s)
{
  int len = strlen(s);
  if (strncmp(s, (const char *)*pp, len) != 0)
    return 0;
  *pp += len;
  return 1;
}


//===========================================================================
// Main interpretation functions.
//===========================================================================
void midi_event(int time, int event, const uint8_t **pp)
{
  const uint8_t *p = *pp;
  uint8_t type = event & 0xf0;
  uint8_t chan = event & 0x0f;

  switch(type) {
    case 0x80: { int note=int1(&p); int velo=int1(&p);
                 note_off(time, chan, note, velo); break;
               }
    case 0x90: { int note=int1(&p); int velo=int1(&p);
                 note_on(time, chan, note, velo); break;
               }
    case 0xa0: { int note=int1(&p); int value=int1(&p);
                 key_pressure(time, chan, note, value); break;
               }
    case 0xb0: { int ctrl=int1(&p); int value=int1(&p);
                 control_change(time, chan, ctrl, value); break;
               }
    case 0xc0: { int prog=int1(&p);
                 program_change(time, chan, prog); break;
               }
    case 0xd0: { int value=int1(&p);
                 channel_pressure(time, chan, value); break;
               }
    case 0xe0: { int l=int1(&p); int h=int1(&p);
                 pitch_wheel_change(time, chan, (h<<7)+l); break;
               }
  }

  *pp = p;
}


