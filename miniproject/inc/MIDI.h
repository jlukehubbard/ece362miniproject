void note_off(int time, int chan, int key, int velo);
void note_on(int time, int chan, int key, int velo);
void key_pressure(int time, int chan, int key, int val);
void control_change(int time, int chan, int ctrl, int val);
void program_change(int time, int chan, int prog);
void channel_pressure(int time, int chan, int val);
void pitch_wheel_change(int time, int chan, int val);
void midi_event(int time, int event, const uint8_t **pp);
