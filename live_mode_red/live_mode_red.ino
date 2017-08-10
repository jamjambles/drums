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
//#include "island_in_the_sun.h"

void setup() {
  //setting up the serials
  Serial.begin(9600); 
  
  seq_count = -1;

  TCCR1A = 0; //Timer 1 (used by servo lib)
  TCCR1B = 0;

  // Interrupt for beats
  attachInterrupt(digitalPinToInterrupt(BASE_BPM_IN), beat_interrupt_b,RISING); //Whenever pin 19 goes from low to high write drums (base) 
  attachInterrupt(digitalPinToInterrupt(SUB_BPM_IN), beat_interrupt_s,RISING); //Whenever pin 18 goes from low to high write drums (sub)
  
  // Predelay durations
  predelays[SNARE_OFFSET] = SNARE_PREDELAY;
  predelays[KICK_OFFSET] = KICK_PREDELAY;
  predelays[HAT_OFFSET] = HAT_PREDELAY;
  predelays[CRASH_OFFSET] = CRASH_PREDELAY;
  predelays[TOM1_OFFSET] = TOM1_PREDELAY;
  predelays[RIDE_OFFSET] = RIDE_PREDELAY;
  predelays[FTOM_OFFSET] = FTOM_PREDELAY;
  
  // Strike times
  strike_times[SNARE_OFFSET] = SNARE_TIME;
  strike_times[KICK_OFFSET] = KICK_TIME;
  strike_times[HAT_OFFSET] = HAT_TIME;
  strike_times[CRASH_OFFSET] = CRASH_TIME;
  strike_times[TOM1_OFFSET] = TOM1_TIME;
  strike_times[RIDE_OFFSET] = RIDE_TIME;
  strike_times[FTOM_OFFSET] = FTOM_TIME;

  // Drum pins
  drum_pins[SNARE_OFFSET] = SNARE;
  drum_pins[KICK_OFFSET] = KICK;
  drum_pins[HAT_OFFSET] = HAT;
  drum_pins[CRASH_OFFSET] = CRASH;
  drum_pins[TOM1_OFFSET] = TOM1;
  drum_pins[RIDE_OFFSET] = RIDE;
  drum_pins[FTOM_OFFSET] = FTOM;
  
  for(int i = 0; i < NUMBER_OF_DRUMS; i++){
    // Initially turn predelay and strikes false
    pd_active[i] = false;
    strike_active[i] = false;
    // Set 5ms counters
    pd_5ms_count[i] = 0;
    strike_5ms_count[i] = 0;
    // Set accents
    accents[i] = 0;
    // Set all drum pins to be outputs
    pinMode(drum_pins[i], OUTPUT);
  }

  
  
  // Drums initially muted (OFF)
  mute_flag_b = true;
  mute_flag_s = true;
  
  // Setting PWM frequency
  set_pwm_2_3_5_6_7_8_9_10();

  // Start the 5ms timer now
  begin_5_timer();
  /*
   * The timer fucks around for 4.5 seconds before it works.
   * If we don't wait here, the drum will be held down for up to 4.5 seconds,
   * This would probably fry the solenoid driving circuit.
   */
  delay(4500);

  lastpinstate = digitalRead(MUTE_IN);
  Serial.println('g'); //good2gobro
}

void loop() {
  // Polling and debouncing the mute pin
  debounce(MUTE_IN, 50);
}

void beat_interrupt_b(){
  if(mute_flag_b == false){
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    mute_flag_s = false;
    read_sequence();  
  }
}

void beat_interrupt_s(){
  if(mute_flag_s == false){ 
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    read_sequence(); 
  }
}

void read_sequence(){
  // Reads sequence at current seq_count position
  for(int i = 0; i < NUMBER_OF_DRUMS; i++){
    // Check if there is a drum hit due
    if(pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i) != NO_HIT){
      // Store the strength of the hit
      accents[i] = pgm_read_word_near(sequence+seq_count*NUMBER_OF_DRUMS + i);
      // Start the predelay phase
      pd_active[i] = true;  
    }
  }
}

ISR(TIMER1_COMPA_vect){
  // This ISR is called every 5ms
  
  for(int i = 0; i < NUMBER_OF_DRUMS; i++){
      if(pd_active[i] == true){
        // Increment predelay counter
        pd_5ms_count[i]++;  
      }
      if(strike_active[i] == true){
        // Increment strike duration counter
        strike_5ms_count[i]++;  
      }
      // TODO: test accents
      if(pd_active[i] == true && pd_5ms_count[i] == predelays[i] / TIMER_TIME){
        // Write the accent of the array to the drum
        analogWrite(drum_pins[i], accents[i]);
        // Turn the predelay flag off
        pd_active[i] = false;
        // Reset the predelay counter
        pd_5ms_count[i] = 0;
        // Set the strike flag on
        strike_active[i] = true;  
      }
      if(strike_active[i] == true && strike_5ms_count[i] == strike_times[i] / TIMER_TIME){
        // Turn the drum off
        analogWrite(drum_pins[i],0);
        // Turn the strike flag off
        strike_active[i] = false;
        // Reset the strike duration counter
        strike_5ms_count[i] = 0;
      }
  }
  // Reset the timer
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

void debounce(const byte PIN, unsigned long debounce_delay){
  int pinstate = digitalRead(PIN); //read state of mutepin
  if (pinstate != lastpinstate) { //if the state has changed since the last reading set the lastDebounceTime
    lastdebounce = millis();
  }
  if ((millis() - lastdebounce) > debounce_delay) { //if the last time a bounce occured is greater than debounceDelay perform mute logic
    mute(pinstate);
  }
  lastpinstate = pinstate;
}

void mute(int mutestate){
  if(mutestate == LOW){
    if(mute_flag_b == true){
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

