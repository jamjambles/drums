// Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560

#include "drums.h"
/*
 * Contains the sequence specs and the sequence itself 
 * imagine.h : Imagine by John Lennon 
 * otherside.h: Otherside by RHCP
 * TODO: add preprocessor macros to choose which header to include
 */
//#include "imagine.h"
#include "otherside.h"


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
}

void write_drums_high_b()
{  
  if(mute_flag_b == false)
  { 

    mute_flag_s = false;  
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




