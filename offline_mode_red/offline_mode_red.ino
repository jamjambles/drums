// Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560
#include "drums.h"

void setup() {
  //setting up the serials
  Serial3.begin(9600); //jreceivee beats from jerry. MUST BE SERIAL 3!!
  Serial2.begin(9600); //communicate to the trellis arduino

  // now do the drums
  seq_count = 0;

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
  /*  
  pinMode(KICK, OUTPUT);
  pinMode(SNARE, OUTPUT);
  pinMode(HAT, OUTPUT);
  pinMode(CRASH, OUTPUT);
  pinMode(TOM1, OUTPUT);
  pinMode(RIDE, OUTPUT);
  pinMode(FTOM, OUTPUT);
*/
  TCCR1A = 0; //Timer 1 (used by servo lib)
  TCCR1B = 0;
/*
  kick_active = false;
  snare_active = false;
  hat_active = false;
  crash_active = false;
  tom1_active = false;
  ride_active = false;
  ftom_active = false;

  s_multiple_of_5 = 0;
  k_multiple_of_5 = 0;
  h_multiple_of_5 = 0;
  c_multiple_of_5 = 0;
  t1_multiple_of_5 = 0;
  r_multiple_of_5 = 0;
  ft_multiple_of_5 = 0;
*/
  //setting pwm frequency to 31KHz on pins 2,3,5,6,7,8,9,10
  set_pwm_2_3_5_6_7_8_9_10();

  begin_5_timer();
  //Serial.println("timer...");


  /*
   * The timer fucks around for 4.5 seconds before it works.
   * If we don't wait here, the drum will be held down for up to 4.5 seconds,
   * This would probably fry the solenoid driving circuit.
   */
  delay(4500);
  //.println("go");
  //Serial.println("Set up complete: Drums are good to go!");

  usr_ctrl = true;
  curr_bar = BAR_1;
  
  // this will clear the drum sequence on the trellis.
  Serial2.println('c');
  Serial2.flush();
  // line up the sequence counter on the trellis.
  Serial2.println('1');
  Serial2.flush();

  Serial.println('g');
  reading_drum_coord = false;
}

void loop() {
  
  /* 
   * This is coming from the other mega updating the drum sequence.
   * We only update the drum sequence when in user control mode.
   * there are 7 drums and 16 time slots: so numbers should be from 0 to 111.
   */

  if (Serial2.available()) {
    //read char, is it c
    char in = (char)Serial2.read();
    int coord = (int)in;
    //Serial.println(coord);
       
      // read the coordinate
      if (coord >= 0 && coord < 112 && usr_ctrl) {
      int beat_num = (int)coord / (int)7;
      int accent_num = beat_num % 4;
      
     /*
      * The beats of the bar have HARD accents
      *      Occurs on beats: 0, 4, 8 and 12.
      * The 2,3,4 sub beats are automatically set to MED
      *      Occurs on the rest (ie. all of the sub beats).
      * This not only sounds much nicer and more human but allows the drums to hit much faster.
      *      ie. at semi-quaver time instead of just quaver time.
      */
      switch (accent_num) {
        case 0:
          if (sequence[coord] != 0) {
            sequence[coord] = 0;
          } else {
            sequence[coord] = HARD;
          }
          break;

        // These are the sub beats
        case 1:
        case 2:
        case 3:
          if (sequence[coord] != 0) {
            sequence[coord] = 0;
          } else {
            sequence[coord] = MED;
          }
          break;
        }
      } else {

        /*
         * This is where the special command and control panel buttons are handled.
         * 
         * -16: clear drum sequence to zero.
         * 
         * TODO:
         *    - what to do when we click clear on a long, uploaded sequence?
         *    - any other trellis button that needs to change the drum sequence here?
         */

        switch (coord) {
        case -16: // clear drum sequence.
          //Serial.println("restart");
          for (int j = 0; j < SEQUENCE_LENGTH; j++) {
            sequence[j] = NO_HIT;
          }
          break;
        }
      }
    } 

  /*
   * This is coming from Jerry's computer via the FTDI
   * Gives us beat times.
   * 
   * We've just realised that this doesn't need to be over FTDI
   * It can just be usb over regualr Serial instead of Serial3.
   */
  while (Serial3.available() > 0) {
    //Receive the serial via ftdi
    char temp = (char)Serial3.read();
    //Send the serial to the trellis arduino
    Serial2.println(temp);
    Serial2.flush();
   
    switch (temp) {
      case '1':
        seq_count = 0;
        //write_drums_high();
        read_sequence();
        break;
      case '2':
        seq_count = 4;
        //write_drums_high();
        read_sequence();
        break;
      case '3':
        seq_count = 8;
//        write_drums_high();
        read_sequence();
        break;
      case '4':
        seq_count = 12;
//        write_drums_high();
        read_sequence();
        break;
      case 's':
//        write_drums_high();
        read_sequence();
        break;
      case 'c':
        resetFunc();
        break;
      default:
        // shit is fucked. Should something happen here?

        break;
    }

  }

}

void read_sequence(){
  // Reads sequence at current seq_count position
  for(int i = 0; i < NUMBER_OF_DRUMS; i++){
    // Check if there is a drum hit due
    if(sequence[seq_count*NUMBER_OF_DRUMS + i] != NO_HIT){
      // Store the strength of the hit
      accents[i] = sequence[seq_count*NUMBER_OF_DRUMS + i];
      // Start the predelay phase
      pd_active[i] = true; 
      // Increment seq count
    }
  }
  seq_count++;
  seq_count = seq_count % NUMBER_OF_STEPS; 
}
/*
void write_drums_high()
{  

//  char cur_seq = (char((seq_count/4) + 1) + '0');
  
  if(snare_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET] != NO_HIT)
  {
    analogWrite(SNARE,sequence[seq_count*NUMBER_OF_DRUMS + SNARE_OFFSET]);   //snare
    snare_active = true;

  }
  
  if(kick_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + KICK_OFFSET] != NO_HIT)
  {
    analogWrite(KICK,sequence[seq_count*NUMBER_OF_DRUMS + KICK_OFFSET]);  //kick
    kick_active = true;

  }
  
  if(hat_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + HAT_OFFSET] != NO_HIT) 
  {
    analogWrite(HAT,sequence[seq_count*NUMBER_OF_DRUMS + HAT_OFFSET]);  //hat
    hat_active = true;

  }
  
  if(crash_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET] != NO_HIT) 
  {
    analogWrite(CRASH,sequence[seq_count*NUMBER_OF_DRUMS + CRASH_OFFSET]);  //crash
    crash_active = true;

  }
  
  if(tom1_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET] != NO_HIT) 
  {
    analogWrite(TOM1,sequence[seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET]);  //tom1
    tom1_active = true;

  }
  
  if(ride_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET] != NO_HIT) 
  {
    analogWrite(RIDE,sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET]);  //ride
    ride_active = true;

  }
  
  if(ftom_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET] != NO_HIT) 
  {
    analogWrite(FTOM,sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET]);  //ftom
    ftom_active = true;

  }
       
  seq_count++;
  seq_count = seq_count % NUMBER_OF_STEPS;
}
*/
/*
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
*/
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
