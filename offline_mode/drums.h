// sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 1
#define NUMBER_OF_BEATS_PER_BAR 4
#define NUMBER_OF_BEATS (NUMBER_OF_BARS*NUMBER_OF_BEATS_PER_BAR)
#define NUMBER_OF_DRUMS 7
#define REST 0
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS+REST)
#define SEQUENCE_LENGTH (NUMBER_OF_STEPS*NUMBER_OF_DRUMS)
#define BEAT_LENGTH (SEQUENCE_LENGTH/4)
// Number of bars in our entire sequence.
#define NUM_BARS 4

// These are the indexes to the sequence array
#define BAR_1 0
#define BAR_2 1
#define BAR_3 2
#define BAR_4 3


// sequence order
#define SNARE_OFFSET  0
#define KICK_OFFSET   1
#define HAT_OFFSET    2
#define CRASH_OFFSET  3
#define TOM1_OFFSET   4
#define RIDE_OFFSET   5
#define FTOM_OFFSET   6

// hit strengths in pwm. 
// Provides ability to accent notes.
#define HARD    255
#define MED     230
#define SOFT    200
#define NO_HIT  0

// output drum pins
// why they are in this order I don't know.
#define SNARE 8
#define KICK  7
#define HAT   3
#define CRASH 2
#define TOM1  9
#define RIDE  5
#define FTOM  6

// timer
#define CLOCK_SPEED   16*10^6 //16 MHz
#define PRESCALER     1024
#define TIMER_RES     (1 / ( CLOCK_SPEED / PRESCALER))
#define TIMER_TIME    5 //ms
#define TIMER_COUNTS  ((TIMER_TIME*10^-3 / TIMER_RES) -1)

// drum times: how long each drum strike is
// longer strike means harder hit
// but a longer strike also limits how fast we can hit drums
#define KICK_TIME   100 //ms
#define SNARE_TIME  40  //ms  
#define HAT_TIME    40  //ms
#define CRASH_TIME  100  //ms
#define TOM1_TIME   50
#define RIDE_TIME   30
#define FTOM_TIME   50


/*
 * There is a timer interrupt which is set for 5ms. This is used to (uniquely) control the duration which each drum is held down for.
 * When a drum is turned on, via a write_drums_high() call, the timer will begin.
 * the x_multiple_of_5 variable keeps track of how long the drum has been held on for (multiple of 5ms). 
 * when this variable equals the predefined drum duration (i.e SNARE_TIME) the drum will be switched off 
 */
 
// Signals whether the drums are currently striking.
volatile bool kick_active;
volatile bool snare_active;
volatile bool hat_active;
volatile bool crash_active;
volatile bool tom1_active;
volatile bool ride_active;
volatile bool ftom_active;

// Used to count down drum strikes in multiples of 5.
volatile int s_multiple_of_5;
volatile int k_multiple_of_5;
volatile int h_multiple_of_5;
volatile int c_multiple_of_5;
volatile int t1_multiple_of_5;
volatile int r_multiple_of_5;
volatile int ft_multiple_of_5;

// Holds the drum sequence.
// Each entry is a PWM value for accents.
// Will need to use progmem for large songs.
char sequence[SEQUENCE_LENGTH] = {0};

int curr_bar;

// index to sequence array
// This is an int between 0 and 15
// 0, 4, 8 and 12 are the 1st, 2nd, 3rd and 4th beats in a bar respectively.
// The rest are the sub beats at semi-quaver resolution.
volatile int seq_count;

// This is set to true in the setup loop.
// It will be set false only when we are loading in entire sequence from the Pi/computer (for live mode).
volatile bool usr_ctrl = false;

/*
 * mute_flag_b: 'base' beat mute
 * mute_flag_s: 'sub' beat mute
 * These flags are used in live mode when a physical footswitch is required to cue the drums in at the beginning of the bar
 * base beats refer to the 'true' beats returned by the real time beat tracking algo (ibt)
 * sub beats refer to beats generated via linear interpolation of the base beats. This enables finer resolution for programming 
 * complex drum patters (16th notes as opposed to quarter notes)
 * it is unlikely that a human would be able to push the footswitch at the correct time between two subbeats so we distinguish between 
 * sub and base beats to make it possible. 
 * When the drums are initially switched on via the footswitch, only the mute_flag_b is set to false. This allows the write_drums_high_b() 
 * call to occur making the drums start playing at the start of the bar on beat 1. After this intial call the mute_flag_s is set to false 
 * allowing subsequent write_drums_high_s() calls to occur. 
 * When the footswitch is pressed again, both mute_flag_b and mute_flag_b are set to true immediately.
 */

void(* resetFunc) (void) = 0;
bool reading_drum_coord;
/*
 * Called whenever there is a beat interrupt.
 * seq_count tells us where we are in the bar
 * we check the drum sequence array to see if this beat (or sub-beat) has a drum hit, 
 *        grab the PWM value and write that out to the pin.
 * 
 * Then we increment and mod seq_count to the next beat.
 */
