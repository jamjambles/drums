/*
 * These are the accent strengths
 */
#define HARD    255
#define MED     255
#define SOFT    255
#define NO_HIT  0

#define NUMBER_OF_DRUMS 7

// sequence order
#define SNARE_OFFSET  0
#define KICK_OFFSET   1
#define HAT_OFFSET    2
#define CRASH_OFFSET  3
#define TOM1_OFFSET   4
#define RIDE_OFFSET   5
#define FTOM_OFFSET   6

// interupt pins
const byte BASE_BPM_IN = 19;
const byte SUB_BPM_IN = 18;
const byte MUTE_IN = 20;// Will mute/unmute the drums

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
#define KICK_TIME   100  
#define SNARE_TIME  40   
#define HAT_TIME    40  
#define CRASH_TIME  100  
#define TOM1_TIME   50
#define RIDE_TIME   30
#define FTOM_TIME   30

// Beat pre delay: how early the PI writes out a beat. This could be set to the drum which takes the longest to strike i.e kick?
#define BEAT_PRE_DELAY 100

//Strike time: how long the mechanical strike is for each drum
#define KICK_STRIKE_TIME   55 
#define SNARE_STRIKE_TIME  30   
#define HAT_STRIKE_TIME    30  
#define CRASH_STRIKE_TIME  35  
#define TOM1_STRIKE_TIME   35
#define RIDE_STRIKE_TIME   30
#define FTOM_STRIKE_TIME   30

/*
 * Predelay: how much time to delay drum before writing it high
 * DRUM_PREDELAY = BEAT_PRE_DELAY - DRUM_STRIKE_TIME
 */
#define KICK_PREDELAY   (BEAT_PRE_DELAY-KICK_STRIKE_TIME)
#define SNARE_PREDELAY  (BEAT_PRE_DELAY-SNARE_STRIKE_TIME)   
#define HAT_PREDELAY    (BEAT_PRE_DELAY-HAT_STRIKE_TIME)  
#define CRASH_PREDELAY  (BEAT_PRE_DELAY-CRASH_STRIKE_TIME)  
#define TOM1_PREDELAY   (BEAT_PRE_DELAY-TOM1_STRIKE_TIME)
#define RIDE_PREDELAY   (BEAT_PRE_DELAY-RIDE_STRIKE_TIME)
#define FTOM_PREDELAY   (BEAT_PRE_DELAY-FTOM_STRIKE_TIME)

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

// Whether drums are in predelay phase or not
volatile bool kick_pd_active;
volatile bool snare_pd_active;
volatile bool hat_pd_active;
volatile bool crash_pd_active;
volatile bool tom1_pd_active;
volatile bool ride_pd_active;
volatile bool ftom_pd_active;

// Same concept as above but 'pd' refering to predelay counts 
volatile int s_pd_multiple_of_5;
volatile int k_pd_multiple_of_5;
volatile int h_pd_multiple_of_5;
volatile int c_pd_multiple_of_5;
volatile int t1_pd_multiple_of_5;
volatile int r_pd_multiple_of_5;
volatile int ft_pd_multiple_of_5;

// index to sequence array
// This is an int between 0 and 15
// 0, 4, 8 and 12 are the 1st, 2nd, 3rd and 4th beats in a bar respectively.
// The rest are the sub beats at semi-quaver resolution.
volatile int seq_count;

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
volatile bool mute_flag_b;
volatile bool mute_flag_s;


// Debounce stuff
int lastbuttonstate;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 

/*
volatile bool pd_active[NUMBER_OF_DRUMS];
volatile bool strike_active[NUMBER_OF_DRUMS];
volatile unsigned int pd_5ms_count[NUMBER_OF_DRUMS];
volatile unsigned int strike_5ms_count[NUMBER_OF_DRUMS];
volatile unsigned int predelays[NUMBER_OF_DRUMS];
volatile unsigned int drum_pins[NUMBER_OF_DRUMS];
volatile unsigned int strike_times[NUMBER_OF_DRUMS];
*/
