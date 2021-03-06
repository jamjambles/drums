// Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560

// sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 1
#define NUMBER_OF_BEATS_PER_BAR 1
#define NUMBER_OF_BEATS (NUMBER_OF_BARS*NUMBER_OF_BEATS_PER_BAR)
#define NUMBER_OF_DRUMS 7
#define REST 0
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS+REST)
#define SEQUENCE_LENGTH (NUMBER_OF_STEPS*NUMBER_OF_DRUMS)
#define BEAT_LENGTH (SEQUENCE_LENGTH/4)
// Number of bars in our entire sequence.
#define NUM_BARS 4

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
#define HARD    (char)255
#define MED     (char)200
#define SOFT    (char)180
#define NO_HIT  (char)0

// interupt pins
const byte BASE_BPM_IN = 19;
const byte SUB_BPM_IN = 18;
const byte MUTE_IN = 20;// Will mute/unmute the drums
const byte STRIKE = A0;

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
#define RIDE_TIME   100
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

// debounce stuff
int lastbuttonstate;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 

long trigger_time;
volatile bool calib;

const PROGMEM char sequence[SEQUENCE_LENGTH]  =
{
//Order of drums: snare, kick, hat, crash, tom1, ride, floor tom//
//Accents: HARD, MED, SOFT, NO_HIT
//bar 1, beat 1
NO_HIT, NO_HIT, HARD, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 
NO_HIT, NO_HIT, MED, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 
NO_HIT, NO_HIT, SOFT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 
NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,

};

void setup() {
  //setting up the serials
  Serial.begin(9600); //just to print to serial monitor
  
  // now do the drums
  seq_count = 0;
  pinMode(KICK, OUTPUT);
  pinMode(SNARE, OUTPUT);
  pinMode(HAT, OUTPUT);
  pinMode(CRASH, OUTPUT);
  pinMode(TOM1, OUTPUT);
  pinMode(RIDE, OUTPUT);
  pinMode(FTOM, OUTPUT);
  pinMode(STRIKE, INPUT);
  TCCR1A = 0; //Timer 1 (used by servo lib)
  TCCR1B = 0;

  //Interrupt for beats
  attachInterrupt(digitalPinToInterrupt(BASE_BPM_IN),write_drums_high_b,RISING); //Whenever pin 19 goes from low to high write drums (base)
  attachInterrupt(digitalPinToInterrupt(SUB_BPM_IN),write_drums_high_s,RISING); //Whenever pin 18 goes from low to high write drums (sub)

  kick_active = false;
  snare_active = false;
  hat_active = false;
  crash_active = false;
  tom1_active = false;
  ride_active = false;
  ftom_active = false;

  //default to drums muted
  mute_flag_b = true;
  mute_flag_s = true;

  calib = false;
  
  s_multiple_of_5 = 0;
  k_multiple_of_5 = 0;
  h_multiple_of_5 = 0;
  c_multiple_of_5 = 0;
  t1_multiple_of_5 = 0;
  r_multiple_of_5 = 0;
  ft_multiple_of_5 = 0;

  //setting pwm frequency to 31KHz on pins 2,3,5,6,7,8,9,10
  set_pwm_2_3_5_6_7_8_9_10();

  begin_5_timer();
  /*
   * The timer fucks around for 4.5 seconds before it works.
   * If we don't wait here, the drum will be held down for up to 4.5 seconds,
   * This would probably fry the solenoid driving circuit.
   */
  delay(4500);

  lastbuttonstate = digitalRead(MUTE_IN);
  Serial.println('g');
  
}

void loop() {
  //debouncing the mute pin
  int mutestate = digitalRead(MUTE_IN); //read state of mutepin
  if (mutestate != lastbuttonstate) { //if the state has changed since the last reading set the lastDebounceTime
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) { //if the last time a bounce occured is greater than debounceDelay perform mute logic
    if(mutestate==LOW){
      if(mute_flag_b==true){
        Serial.println('a'); //write an 'a' to the pi to indicate drums are active
        seq_count = 0; 
        mute_flag_b = false;
      }
    }else{
      if (mute_flag_b == false){
        Serial.println('m'); //write
        mute_flag_b = true;
        mute_flag_s = true;
      }    
    }
    
  }
  lastbuttonstate = mutestate;

  //if(digitalRead(STRIKE)==HIGH)
  //  Serial.print('h');
  
  if(digitalRead(STRIKE) == HIGH && calib == true){
    long cur = millis();
    Serial.println(cur - trigger_time);
    calib = false;
    Serial.println("finished calib");
  }
}

void write_drums_high_b()
{  
  if(mute_flag_b == false)
  { 
    Serial.println("base");
    mute_flag_s = false;  
    bool is_hit = false;
    
    // Calibration stuff
    trigger_time = millis();
    calib = true;
    
    if(snare_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET) != NO_HIT)
    {     
      analogWrite(SNARE,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET));   //snare
      snare_active = true;
      is_hit = true;
    }
    
    
    if(kick_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET) != NO_HIT)
    {
      analogWrite(KICK,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET)); //kick
      kick_active = true;
      is_hit = true;
    }
    
    if(hat_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET) != NO_HIT)
    {
      analogWrite(HAT,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET));
      hat_active = true;
      is_hit = true;
    }
    
    if(crash_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET) != NO_HIT)
    {
      analogWrite(CRASH,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET));
      crash_active = true;
      is_hit = true;
    }
    
    if(tom1_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET) != NO_HIT)
    {
      analogWrite(TOM1,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET));
      tom1_active = true;
      is_hit = true;
    }

    if(ride_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET) != NO_HIT)
    {
      analogWrite(RIDE,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET));
      ride_active = true;
      is_hit = true;
    }
    
    if(ftom_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET) != NO_HIT)
    {
      analogWrite(FTOM,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET));
      ftom_active = true;
      is_hit = true;
    }
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    
    
  }
}
void write_drums_high_s()
{
  if(mute_flag_s == false)
  { 
    bool is_hit = false;
    
    if(snare_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET) != NO_HIT)
    {     
      analogWrite(SNARE,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET));   //snare
      snare_active = true;
      is_hit = true;
    }
    
    
    if(kick_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET) != NO_HIT)
    {
      analogWrite(KICK,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET)); //kick
      kick_active = true;
      is_hit = true;
    }
    
    if(hat_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET) != NO_HIT)
    {
      analogWrite(HAT,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET));
      hat_active = true;
      is_hit = true;
    }
    
    if(crash_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET) != NO_HIT)
    {
      analogWrite(CRASH,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET));
      crash_active = true;
      is_hit = true;
    }
    
    if(tom1_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET) != NO_HIT)
    {
      analogWrite(TOM1,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET));
      tom1_active = true;
      is_hit = true;
    }

    if(ride_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET) != NO_HIT)
    {
      analogWrite(RIDE,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET));
      ride_active = true;
      is_hit = true;
    }
    
    if(ftom_active==false&&pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET) != NO_HIT)
    {
      analogWrite(FTOM,pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET));
      ftom_active = true;
      is_hit = true;
    }
          
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
  }
}


// This catches the 5ms interrupt
// It steps all of the active drums -> counting down their strike duration.
ISR(TIMER1_COMPA_vect)
{

  if (snare_active == true) {
    s_multiple_of_5++; //another 5ms passed
  }

  if (kick_active == true)
    k_multiple_of_5++; //another 5ms passed

  if (hat_active == true)
    h_multiple_of_5++; //another 5ms passed

  if (crash_active == true)
    c_multiple_of_5++; //another 5ms passed

  if (tom1_active == true)
    t1_multiple_of_5++; //another 5ms passed

  if (ride_active == true)
    r_multiple_of_5++; //another 5ms passed

  if (ftom_active == true)
    ft_multiple_of_5++; //another 5ms passed


  if (snare_active == true && s_multiple_of_5 == (SNARE_TIME / TIMER_TIME)) {
    analogWrite(SNARE, 0);
    snare_active = false;
    s_multiple_of_5 = 0;
  }
  if (kick_active == true && k_multiple_of_5 == (KICK_TIME / TIMER_TIME)) {
    analogWrite(KICK, 0);
    kick_active = false;
    k_multiple_of_5 = 0;
  }
  if (hat_active == true && h_multiple_of_5 == (HAT_TIME / TIMER_TIME)) {
    analogWrite(HAT, 0);
    hat_active = false;
    h_multiple_of_5 = 0;
  }
  if (crash_active == true && c_multiple_of_5 == (CRASH_TIME / TIMER_TIME)) {
    analogWrite(CRASH, 0);
    crash_active = false;
    c_multiple_of_5 = 0;
  }

  if (tom1_active == true && t1_multiple_of_5 == (TOM1_TIME / TIMER_TIME)) {
    analogWrite(TOM1, 0);
    tom1_active = false;
    t1_multiple_of_5 = 0;
  }
  if (ride_active == true && r_multiple_of_5 == (RIDE_TIME / TIMER_TIME)) {
    analogWrite(RIDE, 0);
    ride_active = false;
    r_multiple_of_5 = 0;
  }

  if (ftom_active == true && ft_multiple_of_5 == (FTOM_TIME / TIMER_TIME)) {
    analogWrite(FTOM, 0);
    ftom_active = false;
    ft_multiple_of_5 = 0;
  }
  begin_5_timer();
}

// This ticks over the 5ms interrupt.
void begin_5_timer()
{
  TCCR1A = 0;
  TCCR1B = 0;//stop timer

  OCR1A = TIMER_COUNTS; //will count to 5ms
  TCCR1B |= (1 << WGM12); //compare mode
  TCCR1B |= (1 << CS10);// 1024 prescaler
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);//enable compare interrupt
}

// Sets up the high frequency pwm to 31kHz -> beyond audible frequency.
void set_pwm_2_3_5_6_7_8_9_10()
{
  //timer 3
  int prescaler = 1;
  int eraser = 7;//erases last 3 bits
  TCCR3B &= ~eraser; //Clear last 3 bits
  TCCR3B |= prescaler; //Change frequency to 31KHz
  //timer 4
  TCCR4B &= ~eraser; //Clear last 3 bits
  TCCR4B |= prescaler; //Change frequency to 31KHz
  //timer 2
  TCCR2B &= ~eraser; //Clear last 3 bits
  TCCR2B |= prescaler; //Change frequency to 31KHz
}




