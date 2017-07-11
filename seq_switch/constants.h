

//sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 48 // this is what you change
#define NUMBER_OF_BEATS_PER_BAR 4
#define NUMBER_OF_BEATS (NUMBER_OF_BARS*NUMBER_OF_BEATS_PER_BAR)
#define NUMBER_OF_DRUMS 7
#define REST 0
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS+REST)
#define SEQUENCE_LENGTH (NUMBER_OF_STEPS*NUMBER_OF_DRUMS)

//if sequence length is too long, store it in flash mem
//3000 is somewhat arbitrary
#if SEQUENCE_LENGTH > 3000
  #include <avr/pgmspace.h> 
  #define PROGMEM_SET
#endif

//sequence order
#define SNARE_OFFSET  0
#define KICK_OFFSET   1
#define HAT_OFFSET    2
#define CRASH_OFFSET  3
#define TOM1_OFFSET   4
#define RIDE_OFFSET   5
#define FTOM_OFFSET   6

//hit strengths. Provides ability to accent notes
#define WELL    255
#define MED     230
#define RARE    200
#define NO_HIT  0

//timer
#define CLOCK_SPEED   16*10^6 //16 MHz
#define PRESCALER     1024
#define TIMER_RES     (1 / ( CLOCK_SPEED / PRESCALER))
#define TIMER_TIME    5 //ms
#define TIMER_COUNTS  ((TIMER_TIME*10^-3 / TIMER_RES) -1)

//drum times: how long each drum strike is
#define KICK_TIME   90 //ms
#define SNARE_TIME  40  //ms  
#define HAT_TIME    40  //ms
#define CRASH_TIME  80  //ms
#define TOM1_TIME   40
#define RIDE_TIME   80
#define FTOM_TIME   40

