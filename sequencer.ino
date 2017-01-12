//runs on the atmega2560

//sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 1
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

//hit strengths. Provide ability to accent notes
#define WELL    255
#define MED     230
#define RARE    200
#define NO_HIT  0

//interupt pins
const byte PULSE_IN = 19; // The beats from the algo
const byte MUTE_IN = 20; // Will mute/unmute the drums
const byte PHASE_RST_IN = 21; // Correct any phase issues during performance

//count in
#define NUMBER_OF_COUNT_IN_BARS 1

//drum pins
#define SNARE 2
#define KICK  3
#define HAT   5
#define CRASH 6
#define TOM1  7
#define RIDE  8
#define FTOM  9

//timer
#define CLOCK_SPEED   16*10^6 //16 MHz
#define PRESCALER     1024
#define TIMER_RES     (1 / ( CLOCK_SPEED / PRESCALER))
#define TIMER_TIME    5 //ms
#define TIMER_COUNTS  ((TIMER_TIME*10^-3 / TIMER_RES) -1)

//drum times
#define KICK_TIME   90 //ms
#define SNARE_TIME  40  //ms  
#define HAT_TIME    40  //ms
#define CRASH_TIME  80  //ms
#define TOM1_TIME   40
#define RIDE_TIME   40
#define FTOM_TIME   40

//interupt stuff
volatile bool kick_active;
volatile bool snare_active;
volatile bool hat_active;
volatile bool crash_active;
volatile bool tom1_active;
volatile bool ride_active;
volatile bool ftom_active;

volatile int s_multiple_of_5;
volatile int k_multiple_of_5;
volatile int h_multiple_of_5;
volatile int c_multiple_of_5;
volatile int t1_multiple_of_5;
volatile int r_multiple_of_5;
volatile int ft_multiple_of_5;

volatile int seq_count;
volatile int count_in;
volatile bool mute_flag;
volatile int global_count;

#ifdef PROGMEM_SET
const PROGMEM unsigned int sequence[NUMBER_OF_STEPS*NUMBER_OF_DRUMS]  = 
#else
const unsigned int sequence[(NUMBER_OF_STEPS)*NUMBER_OF_DRUMS]  = 
#endif
{
//Order of drums: snare, kick, hat, crash, tom1, ride, floor tom//
NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,

NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,

NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,

NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,WELL,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,
NO_HIT,NO_HIT,WELL,NO_HIT,NO_HIT,NO_HIT,NO_HIT,


};
//pwm stuff
int eraser = 7;

void setup() {
  Serial.begin(9600);
  seq_count = 0;
  pinMode(KICK,OUTPUT);
  pinMode(SNARE,OUTPUT);
  pinMode(HAT,OUTPUT);
  pinMode(CRASH,OUTPUT);
  pinMode(TOM1,OUTPUT);
  pinMode(RIDE,OUTPUT);
  pinMode(FTOM,OUTPUT);
  
  noInterrupts();
  TCCR1A = 0; //Timer 1 (used by servo lib)
  TCCR1B = 0;
  interrupts();            

  attachInterrupt(digitalPinToInterrupt(MUTE_IN),mute,HIGH);
  attachInterrupt(digitalPinToInterrupt(MUTE_IN),unmute,LOW);
  attachInterrupt(digitalPinToInterrupt(PULSE_IN),write_drums_high,RISING); //Whenever pin 19 goes from low to high write drums
 
  //attachInterrupt(digitalPinToInterrupt(PHASE_RST_IN),phase_rst,RISING);
  
  kick_active = false;
  snare_active = false;
  hat_active = false;
  crash_active = false;
  tom1_active = false;
  ride_active = false;
  ftom_active = false;

  count_in = NUMBER_OF_COUNT_IN_BARS*NUMBER_OF_BEATS_PER_BAR*NUMBER_OF_STEPS_PER_BEAT;
  mute_flag = false;
  
  s_multiple_of_5 = 0;
  k_multiple_of_5 = 0;
  h_multiple_of_5 = 0;
  c_multiple_of_5 = 0;
  t1_multiple_of_5 = 0;
  r_multiple_of_5 = 0;
  ft_multiple_of_5 = 0;
  
  //setting pwm frequency to 31KHz on pins 2,3,5,6,7,8,9,10
  set_pwm_2_3_5_6_7_8_9_10(); 

  global_count = 0;
}

void loop() {

}

void mute(){
  mute_flag = true;
}

void unmute(){
  seq_count = global_count%4;
  
  mute_flag = false;
}

void phase_rst(){
  seq_count = 0; 
  
}


void write_drums_high()
{
  global_count++;
  //if(mute_flag == false && count_in == 0)
  if(mute_flag == false)
  { 
    bool is_hit = false;
    
#ifdef PROGMEM_SET
   
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
#else
    if(snare_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET] != NO_HIT)
    {
      analogWrite(SNARE,sequence[seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET]);   //snare
      snare_active = true;
      is_hit = true;
    }
    
    if(kick_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + KICK_OFFSET] != NO_HIT)
    {
      analogWrite(KICK,sequence[seq_count*NUMBER_OF_DRUMS + KICK_OFFSET]);  //kick
      kick_active = true;
      is_hit = true;
    }
    
    if(hat_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + HAT_OFFSET] != NO_HIT) 
    {
      analogWrite(HAT,sequence[seq_count*NUMBER_OF_DRUMS + HAT_OFFSET]);  //hat
      hat_active = true;
      is_hit = true;
    }
    
    if(crash_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET] != NO_HIT) 
    {
      analogWrite(CRASH,sequence[seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET]);  //crash
      crash_active = true;
      is_hit = true;
    }
    
    if(tom1_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET] != NO_HIT) 
    {
      analogWrite(TOM1,sequence[seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET]);  //tom1
      tom1_active = true;
      is_hit = true;
    }
    
    if(ride_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET] != NO_HIT) 
    {
      analogWrite(RIDE,sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET]);  //ride
      ride_active = true;
      is_hit = true;
    }
    
    if(ftom_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET] != NO_HIT) 
    {
      analogWrite(FTOM,sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET]);  //ftom
      ftom_active = true;
      is_hit = true;
    }
#endif
    if (is_hit)
      begin_5_timer();
         
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
  }
  else
  {
    
  }
  
  //else if(mute_flag == false && count_in > 0)
  //{
  //  count_in--;
  //}
}

ISR(TIMER1_COMPA_vect)
{
  if(snare_active == true)
    s_multiple_of_5++; //another 5ms passed

  if(kick_active == true)
    k_multiple_of_5++; //another 5ms passed

  if(hat_active == true)
    h_multiple_of_5++; //another 5ms passed

  if(crash_active == true)
    c_multiple_of_5++; //another 5ms passed

  if(tom1_active == true)
    t1_multiple_of_5++; //another 5ms passed

  if(ride_active == true)
     r_multiple_of_5++; //another 5ms passed

  if(ftom_active == true)
     ft_multiple_of_5++; //another 5ms passed

  
  if(snare_active==true&&s_multiple_of_5==(SNARE_TIME/TIMER_TIME)){
    analogWrite(SNARE,0); 
    snare_active=false;
    s_multiple_of_5 = 0;
  }
  if(kick_active==true&&k_multiple_of_5==(KICK_TIME/TIMER_TIME)){
    analogWrite(KICK,0); 
    kick_active=false;
    k_multiple_of_5 = 0;
  }
  if(hat_active==true&&h_multiple_of_5==(HAT_TIME/TIMER_TIME)){
    analogWrite(HAT,0);
    hat_active=false;
    h_multiple_of_5 = 0;
  }
  if(crash_active==true&&c_multiple_of_5==(CRASH_TIME/TIMER_TIME)){
    analogWrite(CRASH,0); 
    crash_active=false;
    c_multiple_of_5 = 0;
  }
  
  if(tom1_active==true&&t1_multiple_of_5==(TOM1_TIME/TIMER_TIME)){
    analogWrite(TOM1,0); 
    tom1_active=false;
    t1_multiple_of_5 = 0;
  }
  if(ride_active==true&&r_multiple_of_5==(RIDE_TIME/TIMER_TIME)){
    analogWrite(RIDE,0); 
    ride_active=false;
    r_multiple_of_5 = 0;
  }
  
  if(ftom_active==true&&ft_multiple_of_5==(FTOM_TIME/TIMER_TIME)){
    analogWrite(FTOM,0); 
    ftom_active=false;
    ft_multiple_of_5 = 0;
  }
  
  begin_5_timer();
  
}

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

void set_pwm_2_3_5_6_7_8_9_10()
{
  //timer 3
  int prescaler = 1;
  TCCR3B &= ~eraser; //Clear last 3 bits
  TCCR3B |=prescaler; //Change frequency to 31KHz
  //timer 4
  TCCR4B &= ~eraser; //Clear last 3 bits
  TCCR4B |=prescaler; //Change frequency to 31KHz
  //timer 2
  TCCR2B &= ~eraser; //Clear last 3 bits
  TCCR2B |=prescaler; //Change frequency to 31KHz
}


