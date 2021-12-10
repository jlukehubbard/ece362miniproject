#define WAVENUM 5
#define VOLUME 2048
#define N 1000
#define NSHIFT16 (N << 16)
#define RATE 20000
int volume;
int dacScale;
uint8_t wavenum;
short int wavetable[WAVENUM][N];

void initWavetable(void);
void setupDac(void);
void init_tim7(void);
