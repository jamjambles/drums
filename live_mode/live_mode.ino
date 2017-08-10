// Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560

#include "drums.h"
/*
 * Contains the sequence specs and the sequence itself 
 * imagine.h : Imagine by John Lennon 
 * otherside.h: Otherside by RHCP
 * TODO: add preprocessor macros to choose which header to include
 */
#include "imagine.h"
//#include "otherside.h"
//#include "simple.h"

void setup() {
  //setting up the serials
  Serial.begin(9600); //just to print to serial monitor
  
  seq_count = -1;
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
  //attachInterrupt(digitalPinToInterrupt(BASE_BPM_IN),write_drums_high_b,RISING); //Whenever pin 19 goes from low to high write drums (base)
  //attachInterrupt(digitalPinToInterrupt(SUB_BPM_IN),write_drums_high_s,RISING); //Whenever pin 18 goes from low to high write drums (sub)

  attachInterrupt(digitalPinToInterrupt(BASE_BPM_IN), beat_interrupt_b,RISING); //Whenever pin 19 goes from low to high write drums (base) 
  attachInterrupt(digitalPinToInterrupt(SUB_BPM_IN), beat_interrupt_s,RISING); //Whenever pin 18 goes from low to high write drums (sub)

  // All drums are initially off
  
  kick_active = false;
  snare_active = false;
  hat_active = false;
  crash_active = false;
  tom1_active = false;
  ride_active = false;
  ftom_active = false;
  
  /*
  for(int i = 0; i < 7; i++){
    pd_active[i] = false;
    strike_active[i] = false;
    pd_5ms_count[i] = 0;
    strike_5ms_count[i] = 0;
  }
  
  predelays[0] = SNARE_PREDELAY;
  predelays[1] = KICK_PREDELAY;
  predelays[2] = HAT_PREDELAY;
  predelays[3] = CRASH_PREDELAY;
  predelays[4] = TOM1_PREDELAY;
  predelays[5] = RIDE_PREDELAY;
  predelays[6] = FTOM_PREDELAY;

  drum_pins[0] = SNARE;
  drum_pins[1] = KICK;
  drum_pins[2] = HAT;
  drum_pins[3] = CRASH;
  drum_pins[4] = TOM1;
  drum_pins[5] = RIDE;
  drum_pins[6] = FTOM;

  strike_times[0] = SNARE_TIME;
  strike_times[1] = KICK_TIME;
  strike_times[2] = HAT_TIME;
  strike_times[3] = CRASH_TIME;
  strike_times[4] = TOM1_TIME;
  strike_times[5] = RIDE_TIME;
  strike_times[6] = FTOM_TIME;
  */
  
  // No predelays at the moment
  kick_pd_active = false;
  snare_pd_active = false;
  hat_pd_active = false;
  crash_pd_active = false;
  tom1_pd_active = false;
  ride_pd_active = false;
  ftom_pd_active = false;
  
  // Default to drums muted
  mute_flag_b = true;
  mute_flag_s = true;
  
  s_multiple_of_5 = 0;
  k_multiple_of_5 = 0;
  h_multiple_of_5 = 0;
  c_multiple_of_5 = 0;
  t1_multiple_of_5 = 0;
  r_multiple_of_5 = 0;
  ft_multiple_of_5 = 0;

  s_pd_multiple_of_5 = 0;
  k_pd_multiple_of_5 = 0;
  h_pd_multiple_of_5 = 0;
  c_pd_multiple_of_5 = 0;
  t1_pd_multiple_of_5 = 0;
  r_pd_multiple_of_5 = 0;
  ft_pd_multiple_of_5 = 0;
  
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
  Serial.println('g'); //goos2gobro
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
        Serial.println('m'); //write an 'm' to the pi to indicate drums are muted
        mute_flag_b = true;
        mute_flag_s = true;
      }    
    }
    
  }
  lastbuttonstate = mutestate;
}
void beat_interrupt_b(){
  
  if(mute_flag_b == false){
    Serial.println("base");
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    
    mute_flag_s = false;
    /*
    for(int i = 0; i<7; i++){
      if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i) != NO_HIT){
        pd_active[i] = true;  
      }
    }
    */

    // Check the next drums which will be playing and begin predelay timer
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET) != NO_HIT){
      snare_pd_active = true; 
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET) != NO_HIT){
      kick_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET) != NO_HIT){
      hat_pd_active = true; 
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET) != NO_HIT){
      crash_pd_active = true;
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET) != NO_HIT){
      tom1_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET) != NO_HIT){
      ride_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET) != NO_HIT){
      ftom_pd_active = true; 
    }
    
  }
}

void beat_interrupt_s(){
  
  if(mute_flag_s == false){ 
    Serial.println("sub");
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
  /*  
    for(int i = 0; i<7; i++){
      if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i) != NO_HIT){
        pd_active[i] = true;  
      }
    }
    */
    
    // Check the next drums which will be playing and begin predelay timer
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET) != NO_HIT){
      snare_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + KICK_OFFSET) != NO_HIT){
      kick_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + HAT_OFFSET) != NO_HIT){
      hat_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET) != NO_HIT){
      crash_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET) != NO_HIT){
      tom1_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET) != NO_HIT){
      ride_pd_active = true;  
    }
  
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET) != NO_HIT){
      ftom_pd_active = true;  
    }
    
  }
}

ISR(TIMER1_COMPA_vect){
  // This ISR is called every 5ms
  /*
  for(int i = 0; i<7; i++){
      if(pd_active[i] == true){
        pd_5ms_count[i]++;  
      }
      if(strike_active[i] == true){
        strike_5ms_count[i]++;  
      }
      if(pd_active[i] == true && pd_5ms_count[i] == predelays[i] / TIMER_TIME){
        pd_active[i] = false;
        pd_5ms_count[i] = 0;
        analogWrite(drum_pins[i],255);
        strike_active[i] = true;  
      }
      if(strike_active[i] == true && strike_5ms_count[i] == strike_times[i] / TIMER_TIME){
        analogWrite(drum_pins[i],0);
        strike_active[i] = false;
        strike_5ms_count[i] = 0;
      }
  }
  */
  
  // Incrementing the strike duration
  if (snare_active) {
    s_multiple_of_5++; 
  }
  if (kick_active){
    k_multiple_of_5++; 
  }
  if (hat_active){
    h_multiple_of_5++; 
  }
  if (crash_active){
    c_multiple_of_5++; 
  }
  if (tom1_active){
    t1_multiple_of_5++;
  }
  if (ride_active){
    r_multiple_of_5++; 
  }
  if (ftom_active){
    ft_multiple_of_5++; 
  }
  
  
  // Incrementing the predelay
  if (snare_pd_active){
    s_pd_multiple_of_5++; 
  }
  if (kick_pd_active){
    k_pd_multiple_of_5++; 
  }
  if (hat_pd_active){
    h_pd_multiple_of_5++;
  }
  if (crash_pd_active){
    c_pd_multiple_of_5++; 
  }    
  if (tom1_pd_active){
    t1_pd_multiple_of_5++;
  }
  if (ride_pd_active){
    r_pd_multiple_of_5++; 
  }
  if (ftom_pd_active){
    ft_pd_multiple_of_5++; 
  }  
  
  // Writing the drums high after their predelay time
  // TODO: implement accents
  if (snare_pd_active == true && s_pd_multiple_of_5 == (SNARE_PREDELAY / TIMER_TIME)) {
    snare_pd_active = false;
    s_pd_multiple_of_5 = 0;
    analogWrite(SNARE,255);
    snare_active = true;
  }
  if (kick_pd_active == true && k_pd_multiple_of_5 == (KICK_PREDELAY / TIMER_TIME)) {
    kick_pd_active = false;
    k_pd_multiple_of_5 = 0;
    analogWrite(KICK,255);
    kick_active = true;
  }
  if (hat_pd_active == true && h_pd_multiple_of_5 == (HAT_PREDELAY / TIMER_TIME)) {
    hat_pd_active = false;
    h_pd_multiple_of_5 = 0;
    analogWrite(HAT,255);
    hat_active = true;
  }
  if (crash_pd_active == true && c_pd_multiple_of_5 == (CRASH_PREDELAY / TIMER_TIME)) {
    crash_pd_active = false;
    c_pd_multiple_of_5 = 0;
    analogWrite(CRASH,255);
    crash_active = true;
  }

  if (tom1_pd_active == true && t1_pd_multiple_of_5 == (TOM1_PREDELAY / TIMER_TIME)) {
    tom1_pd_active = false;
    t1_pd_multiple_of_5 = 0;
    analogWrite(TOM1,255);
    tom1_active = true;
  }
  if (ride_pd_active == true && r_pd_multiple_of_5 == (RIDE_PREDELAY / TIMER_TIME)) {
    ride_pd_active = false;
    r_pd_multiple_of_5 = 0;
    analogWrite(RIDE,255);
    ride_active = true;
  }

  if (ftom_pd_active == true && ft_pd_multiple_of_5 == (FTOM_PREDELAY / TIMER_TIME)) {
    ftom_pd_active = false;
    ft_pd_multiple_of_5 = 0;
    analogWrite(FTOM,255);
    ftom_active = true;
  }
  
  // Writing the drums low after their defined on time
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
  
  // Keep resetting the timer
  begin_5_timer();
}

void begin_5_timer(){
  // Setting up 5ms timer interrupt routine
  TCCR1A = 0;
  TCCR1B = 0;//stop timer

  OCR1A = TIMER_COUNTS; //will count to 5ms
  TCCR1B |= (1 << WGM12); //compare mode
  TCCR1B |= (1 << CS10);// 1024 prescaler
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);//enable compare interrupt
}

void set_pwm_2_3_5_6_7_8_9_10(){ 
  // Sets up the PWM on pins:2,3,5,6,7,8,9,10 to 31kHz
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




