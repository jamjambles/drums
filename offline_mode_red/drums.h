#define NUMBER_OF_DRUMS 7

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
#define FTOM_TIME   50

// Beat pre delay: how early the PI writes out a beat. This could be set to the drum which takes the longest to strike i.e kick?
#define BEAT_PRE_DELAY 60

//Strike time: how long the mechanical strike is for each drum
#define KICK_STRIKE_TIME   50 
#define SNARE_STRIKE_TIME  30   
#define HAT_STRIKE_TIME    30  
#define CRASH_STRIKE_TIME  35  
#define TOM1_STRIKE_TIME   35
#define RIDE_STRIKE_TIME   30
#define FTOM_STRIKE_TIME   35

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

#define DEBOUNCE_DELAY (unsigned long)50 
/*
 * There is a timer interrupt which is set for 5ms. This is used to (uniquely) control the duration which each drum is held down for.
 * When a drum is turned on, via a write_drums_high() call, the timer will begin.
 * the x_multiple_of_5 variable keeps track of how long the drum has been held on for (multiple of 5ms). 
 * when this variable equals the predefined drum duration (i.e SNARE_TIME) the drum will be switched off 
 */

#define HARD    255
#define MED     230
#define SOFT    200
#define NO_HIT  0
 
void(* resetFunc) (void) = 0;
bool reading_drum_coord;
volatile bool usr_ctrl = false;
int curr_bar;
char sequence[SEQUENCE_LENGTH] = {0};

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

// These could be multidimenional arrays?
volatile bool pd_active[NUMBER_OF_DRUMS]; // I think this should hold the accent rather than being a boolean
volatile bool strike_active[NUMBER_OF_DRUMS]; 
volatile unsigned int pd_5ms_count[NUMBER_OF_DRUMS]; // These could all be chars ???
volatile unsigned int strike_5ms_count[NUMBER_OF_DRUMS];
volatile unsigned int predelays[NUMBER_OF_DRUMS];
volatile unsigned int drum_pins[NUMBER_OF_DRUMS];
volatile unsigned int strike_times[NUMBER_OF_DRUMS];
volatile unsigned int accents[NUMBER_OF_DRUMS];

