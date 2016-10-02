#include "../sram.h"

#define READ_SONGDATA_BYTE(addr) sram_read(addr)

//#define SAMPLERATE 8000

//#define REFERENCE_RATE 8287

// FIXED POINT ARITHMETICS

//#define FPACBITS 16

//#define FPA_TO_NORM(a) (a >> FPACBITS)
//#define NORM_TO_FPA(a) ((uint32_t)(((uint32_t)a) << FPACBITS))
//#define FPA_MUL(a,b) ((a * b) >> FPACBITS)
//#define FPA_DIV(a,b) ((a << FPACBITS) / b)

//#define FPS 50 // PAL
//#define SAMPLES_PER_FRAME (SAMPLERATE/FPS)

#define OFFSET_SAMPLE_HEADER (20)
#define SAMPLE_HEADER_LENGTH (22+2+1+1+2+2)
#define OFFSET_MK (1080) // M.K.

//#define SET_BIT(PORT, BITNUM)    ((PORT) |=  (1<<(BITNUM)))
//#define CLEAR_BIT(PORT, BITNUM)  ((PORT) &= ~(1<<(BITNUM)))
//#define TOGGLE_BIT(PORT, BITNUM) ((PORT) ^=  (1<<(BITNUM)))

//#define TESTCHAN 0

#define HALF_FREQ_THRESH 200
