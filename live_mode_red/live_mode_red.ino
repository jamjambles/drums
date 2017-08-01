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
  
  // Default to drums muted
  mute_flag_b = true;
  mute_flag_s = true;
  
  //setting pwm frequency
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
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    
    mute_flag_s = false;
    
    for(int i = 0; i<7; i++){
      if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i) != NO_HIT){
        pd_active[i] = true;  
      }
    }
  }
}

void beat_interrupt_s(){
  if(mute_flag_s == false){ 
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    
    for(int i = 0; i<7; i++){
      if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i) != NO_HIT){
        pd_active[i] = true;  
      }
    }
  }
}

ISR(TIMER1_COMPA_vect){
  // This ISR is called every 5ms
  
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




