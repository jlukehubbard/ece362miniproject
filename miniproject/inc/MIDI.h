#ifndef MIDI_H
#define MIDI_H

void note_off(uint8_t key, uint8_t velo);
void note_on(uint8_t key, uint8_t velo);
void key_pressure(uint8_t key, uint8_t val);
void control_change(uint8_t ctrl, uint8_t val);
void program_change(uint8_t prog);
void channel_pressure(uint8_t val);
void pitch_wheel_change(uint8_t val);

#endif
