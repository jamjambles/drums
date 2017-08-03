//Drum sequencer by James Wagner. For use with arduino mega2560

#include "constants.h"
#include "imagine.h"
//interupt pins
const byte BASE_BPM_IN = 19; // The beats from the pi
const byte SUB_BPM_IN =  18;  
const byte MUTE_IN = 20; // Will mute/unmute the drums
const byte PHASE_RST_IN = 21; // Correct any phase issues during performance. Not implemented

//output drum pins
#define SNARE 2
#define KICK  3
#define HAT   5
#define CRASH 6
#define TOM1  7
#define RIDE  8
#define FTOM  9


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

//index to sequence array
volatile int seq_count;
//mute drums
volatile bool mute_flag_b;
volatile bool mute_flag_s;







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

  //attachInterrupt(digitalPinToInterrupt(MUTE_IN),mute,HIGH); //Mute/unmute drums
  //attachInterrupt(digitalPinToInterrupt(MUTE_IN),unmute,LOW);
  
  attachInterrupt(digitalPinToInterrupt(BASE_BPM_IN),write_drums_high_b,RISING); //Whenever pin 19 goes from low to high write drums
  attachInterrupt(digitalPinToInterrupt(SUB_BPM_IN),write_drums_high_s,RISING); //Whenever pin 18 goes from low to high write drums
  
  //attachInterrupt(digitalPinToInterrupt(PHASE_RST_IN),phase_rst,RISING);
  
  kick_active = false;
  snare_active = false;
  hat_active = false;
  crash_active = false;
  tom1_active = false;
  ride_active = false;
  ftom_active = false;
  
  //default to drums off
  mute_flag_b = true;
  mute_flag_s = true;
  
  s_multiple_of_5 = 0;
  k_multiple_of_5 = 0;
  h_multiple_of_5 = 0;
  c_multiple_of_5 = 0;
  t1_multiple_of_5 = 0;
  r_multiple_of_5 = 0;
  ft_multiple_of_5 = 0;
  
  //setting pwm frequency to 31KHz on pins 2,3,5,6,7,8,9,10
  set_pwm_2_3_5_6_7_8_9_10(); 

}

void loop() {
  if(digitalRead(MUTE_IN)==HIGH)
  {
    if(mute_flag_b==true){
      seq_count = 0; 
      mute_flag_b = false;
    }
  }else{
    mute_flag_b = true;
    mute_flag_s = true;
  }
}

void mute(){
  mute_flag_b = true;
  mute_flag_s = true;
}

void unmute(){
  seq_count = 0; 
  mute_flag_b = false;

  //dont set mute_flag_s here
}

void phase_rst(){
  seq_count = 0;   
}


void write_drums_high_b()
{
  
  if(mute_flag_b == false)
  { 
    Serial.print("in base\n");
    mute_flag_s = false;  
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

}




void write_drums_high_s()
{
  //Serial.print("in sub \n");
  if(mute_flag_s == false)
  { 
    Serial.print("in sub and not mute \n");
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


