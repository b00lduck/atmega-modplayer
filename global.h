#define SYSCLK 16000000
#define SBI(port,bit) port |= (1<<bit)
#define CBI(port,bit) port &= ~(1<<bit)
#define NOP asm volatile ("nop");

#define SAMPLERATE 8000

#define REFERENCE_RATE 8287

// FIXED POINT ARITHMETICS

#define FPACBITS 16

//#define FPA_TO_NORM(a) (a >> FPACBITS)
//#define NORM_TO_FPA(a) ((uint32_t)(((uint32_t)a) << FPACBITS))
//#define FPA_MUL(a,b) ((a * b) >> FPACBITS)
//#define FPA_DIV(a,b) ((a << FPACBITS) / b)

#define FPS 50 // PAL
#define SAMPLES_PER_FRAME (SAMPLERATE/FPS)

#define OFFSET_SAMPLE_HEADER (20)
#define SAMPLE_HEADER_LENGTH (22+2+1+1+2+2)

#define OFFSET_SONG_HEADER (OFFSET_SAMPLE_HEADER + SAMPLE_HEADER_LENGTH * 31)
#define OFFSET_PATTERNS (OFFSET_SONG_HEADER + 1 + 1 + 128 + 4)


