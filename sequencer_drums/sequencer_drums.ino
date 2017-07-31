// Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560

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

// hit strengths in pwm. 
// Provides ability to accent notes.
#define HARD    255
#define MED     255
#define SOFT    255
#define NO_HIT  0

// interupt pins
  // The beats from the pi, there is no beat / sub-beat distinction here yet.
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
#define KICK_TIME   100 //ms
#define SNARE_TIME  40  //ms  
#define HAT_TIME    40  //ms
#define CRASH_TIME  100  //ms
#define TOM1_TIME   50
#define RIDE_TIME   30
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

// Holds the drum sequence.
// Each entry is a PWM value for accents.
// Will need to use progmem for large songs.
char sequence[SEQUENCE_LENGTH] = {0};

int curr_bar;

// index to sequence array
// This is an int between 0 and 15
// 0, 4, 8 and 12 are the 1st, 2nd, 3rd and 4th beats in a bar respectively.
// The rest are the sub beats at semi-quaver resolution.
volatile int seq_count;

// This is set to true in the setup loop.
// It will be set false only when we are loading in entire sequence from the Pi/computer (for live mode).
volatile bool usr_ctrl = false;

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

void(* resetFunc) (void) = 0;

int lastbuttonstate;
/*
 * Called whenever there is a beat interrupt.
 * seq_count tells us where we are in the bar
 * we check the drum sequence array to see if this beat (or sub-beat) has a drum hit, 
 *        grab the PWM value and write that out to the pin.
 * 
 * Then we increment and mod seq_count to the next beat.
 */
void write_drums_high_b()
{  
  if(mute_flag_b == false)
  { 
//    Serial.println('b');
//    Serial.println(seq_count);
    char cur_seq = (char((seq_count/4) + 1) + '0');
    //Serial.println(cur_seq);
    Serial2.println(cur_seq);
    Serial2.flush();
    mute_flag_s = false;  
    bool is_hit = false;
    
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
    //if (is_hit)
    //  begin_5_timer();
         
    seq_count++;
    seq_count = seq_count % NUMBER_OF_STEPS;
    
    
  }
}
void write_drums_high_s()
{
  if(mute_flag_s == false)
  { 
    //Serial.print('s');
    //Serial.print(seq_count);
    //Serial.println('s');
    Serial2.println('s');
    Serial2.flush();
    bool is_hit = false;
    
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
    
//    if (is_hit)
//      begin_5_timer();
         
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


void setup() {
  //setting up the serials
  Serial.begin(9600); //just to print to serial monitor
  Serial2.begin(9600); //communicate to the trellis arduino
  Serial3.begin(9600); //receive beats

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

  lastbuttonstate = digitalRead(MUTE_IN);
  Serial.println('g');
  
}

// These three guys are for reading the drum sequence in from the pi.
bool read_drum_sequence = false;
bool read_beat_sequence = false;
int sequence_position = 0; // initialised to 0

// These three guys are for reading the drum sequence in from the other diuno
bool reading_drum_sequence_duino = false;
int sequence_position_duino = 0; // initialised to 0

bool reading_drum_coord = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 

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
  /* 
   * This is coming from the other mega updating the drum sequence.
   * We only update the drum sequence when in user control mode.
   * there are 7 drums and 16 time slots: so numbers should be from 0 to 111.
   */

  
  if (Serial2.available() && !reading_drum_sequence_duino) {
    //read char, is it c
    char in = (char)Serial2.read();
    int coord = (int)in;
    
    if (in == 'z') {
//      Serial.println("starting protocol");
      reading_drum_sequence_duino = true;
      sequence_position_duino = 0;
    } else {


      
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

  }

  if (Serial2.available() && reading_drum_sequence_duino) {
    char sequence_char = (char)Serial2.read();
        
        // check if we have overflowed our buffer.
    if (sequence_position_duino >= SEQUENCE_LENGTH && sequence_char != 'f') {
      //Serial.println("input sequence from arduino is too long. wtf mate");
    }

    /*
     * Same protocol as reading in a sequence from the Pi
     * If there are any Serial.prints in here, 
     *          it will introduce a lag that makes it sound shit around the change of bar.
     * TODO:
     *    - check the sequence length is corrent ocne we have finished reading
     *    - Fucking progmem shit for big sequences.
     */

    switch ((char)sequence_char) {
      case '0':
        sequence[sequence_position_duino] = NO_HIT;
        sequence_position_duino++;
        break;
      case '1':
        sequence[sequence_position_duino] = SOFT;
        sequence_position_duino++;
        break;
      case '2':
        sequence[sequence_position_duino] = MED;
        sequence_position_duino++;
        break;
      case '3':
        sequence[sequence_position_duino] = HARD;
        sequence_position_duino++;
        break;
      case 'z':
        sequence_position_duino = 0;
        
      case 'f':
        reading_drum_sequence_duino = false;
        sequence_position_duino = 0;        
        break;

        // What should we do here???
      default:
        //Serial.print("dud character, was expecting '0', '1', '2' or '3':    ");
        //Serial.println((int)sequence_char);
        sequence_position_duino = 0;
        reading_drum_sequence_duino = false;   
    }
  }


  /*
   * This is coming from the Raspberry Pi
   * The raspberry pi writes the live mode song sequence out over usb Serial
   * We read it in here and save it to our sequence.
   * For drum sequence structure, refer to blah...
   * 
   * 1) Look for a 'z' which signals the start of the sequence
   * 2) Read in values (0, 1, 2, 3)
   * 3) 'f' signals the end of the sequence.
   * 
   * 
   * TODO:
   *    - raspberry pi will send usr_ctrl flag?
   *    - Pi send out the sequence length so we can allocate memory.
   *    - check that the sequence length is factors into 7
   */
  char sequence_char;
  if (Serial.available() > 0) {
    
    sequence_char = (char)Serial.read();
    if (read_drum_sequence || read_beat_sequence) {
      //Serial.println(sequence_position);
      // check if we have overflowed our buffer.
//      if (read_drum_sequence && sequence_position > SEQUENCE_LENGTH) {
//        read_drum_sequence = false;
//        break;
//        //Serial.println("input sequence is too long.");
//      }
//      if(read_beat_sequence && sequence_position > SEQUENCE_LENGTH){
//        //Serial.println("input sequence is too long.");
//        read_beat_sequence = false;
//        break;
//
//      }
      // Serial.println(sequence_position);
      // Serial.flush();

      /*
       * 0 = no hit
       * 1 = soft hit
       * 2 = med hit
       * 3 = hard hit
       * f = end of sequence
       * 
       * TODO:
       *    - check the sequence length is corrent ocne we have finished reading
       *    - Fucking progmem shit for big sequences.
       */
      sequence_position = sequence_position % SEQUENCE_LENGTH;
      switch ((char)sequence_char) {
        
        case '0':
          sequence[sequence_position] = NO_HIT;
          sequence_position++;
//          Serial2.println('n'); 
          break;
        case '1':
          sequence[sequence_position] = SOFT;
          sequence_position++;
//          Serial2.println('o'); 
          break;
        case '2':
          sequence[sequence_position] = MED;
          sequence_position++;
//          Serial2.println('m'); 
          break;
        case '3':
          sequence[sequence_position] = HARD;
          sequence_position++;
//          Serial2.println('h'); 
          break;
        case 'f':
          //resetFunc();
          Serial.println('f'); // indicate end of beat/bar sequennce to trellis
          Serial.println(sequence_position); // indicate end of beat/bar sequennce to trellis
          if (read_drum_sequence) {
            sequence_position = 0;
          } else if (read_beat_sequence) {
            
          }
          read_drum_sequence = false;
          read_beat_sequence = false;
          break;
        default:
          // Serial.println("dud character, was expecting '0', '1', '2' or '3'");
          // I don't know if I should ignore it or if I should exit...
          // read_drum_sequence = false;
          // read_beat_sequence = false;
          break;
      }
    }

    // 'z' signals the start of the sequence.
    // The sequence will start to be read in on the next iteration of this loop.
    if ((char)sequence_char == 'z') {
      // need to print to Serial2 here too....
      Serial.println('z'); // indicate a start of bar sequence to trellis
      read_drum_sequence = true;
      read_beat_sequence = false;
      sequence_position = 0;
      
    } else if ((char)sequence_char == 'y') { //y indicates start of beat sequence
      Serial.println('y'); // indicate a start of beat sequence to trellis
      Serial.println(sequence_position);
      read_beat_sequence = true;
      read_drum_sequence = false;
      //sequence_position = ((seq_count+11)%16)*7;

    
    } else if ((char)sequence_char == 'r') { // reset
      resetFunc();
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
    char temp = (char)Serial3.read();
    Serial3.flush();
    //Serial2.println(temp);
    //Serial2.flush();

    switch (temp) {
      case '1':
        seq_count = 0;
//        write_drums_high();
         write_drums_high_b();
        break;
      case '2':
        seq_count = 4;
//        write_drums_high();
        write_drums_high_b();
        break;
      case '3':
        seq_count = 8;
//        write_drums_high();
        write_drums_high_b();
        break;
      case '4':
        seq_count = 12;
//        write_drums_high();
        write_drums_high_b();
        break;
      case 's':
//        write_drums_high();
        write_drums_high_b();
        break;
      default:
        // shit is fucked.
        //      write_drums_high();
        break;
    }

  }

}

