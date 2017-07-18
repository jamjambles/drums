//Drum sequencer by James Wagner. For use with arduino mega2560


#include <Wire.h>
#include "Adafruit_Trellis.h"


Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_Trellis matrix2 = Adafruit_Trellis();
Adafruit_Trellis matrix3 = Adafruit_Trellis();
Adafruit_Trellis matrix4 = Adafruit_Trellis();
Adafruit_Trellis matrix5 = Adafruit_Trellis();
Adafruit_Trellis matrix6 = Adafruit_Trellis();
Adafruit_Trellis matrix7 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet( &matrix0, &matrix1, &matrix2, &matrix3,
                                                    &matrix4, &matrix5, &matrix6, &matrix7);

// set to however many you're working with here, up to 8
#define NUMTRELLIS 8

#define numKeys (NUMTRELLIS * 16)

// Connect Trellis Vin to 5V and Ground to ground.
// Connect the INT wire to pin #A2 (can change later!)
#define INTPIN A2
// Connect I2C SDA pin to your Arduino SDA line
// Connect I2C SCL pin to your Arduino SCL line
// All Trellises share the SDA, SCL and INT pin! 
// Even 8 tiles use only 3 wires max

//////// NOW THE DRUMS

//sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 1
#define NUMBER_OF_BEATS_PER_BAR 4
#define NUMBER_OF_BEATS (NUMBER_OF_BARS*NUMBER_OF_BEATS_PER_BAR)
#define NUMBER_OF_DRUMS 7
#define REST 0
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS+REST)
#define SEQUENCE_LENGTH (NUMBER_OF_STEPS*NUMBER_OF_DRUMS)

//sequence order
#define SNARE_OFFSET  0
#define KICK_OFFSET   1
#define HAT_OFFSET    2
#define CRASH_OFFSET  3
#define TOM1_OFFSET   4
#define RIDE_OFFSET   5
#define FTOM_OFFSET   6

//hit strengths. Provides ability to accent notes
#define HARD    255
#define MED     230
#define SOFT    200
#define NO_HIT  0

//interupt pins
const byte PULSE_IN = 18; // The beats from the pi
//const byte MUTE_IN = 19; // Will mute/unmute the drums


//output drum pins
#define SNARE 8
#define KICK  7
#define HAT   3
#define CRASH 2
#define TOM1  9
#define RIDE  5
#define FTOM  6

//timer
#define CLOCK_SPEED   16*10^6 //16 MHz
#define PRESCALER     1024
#define TIMER_RES     (1 / ( CLOCK_SPEED / PRESCALER))
#define TIMER_TIME    5 //ms

//#define TIMER_COUNTS  ((TIMER_TIME*10^-3 / TIMER_RES) -1)
#define TIMER_COUNTS 77

//drum times: how long each drum strike is
#define KICK_TIME   150 //ms
#define SNARE_TIME  80  //ms  
#define HAT_TIME    80  //ms
#define CRASH_TIME  150  //ms
#define TOM1_TIME   80
#define RIDE_TIME   80
#define FTOM_TIME   80

//#define KICK_TIME   0 //ms
//#define SNARE_TIME  10  //ms  
//#define HAT_TIME    0  //ms
//#define CRASH_TIME  0  //ms
//#define TOM1_TIME   0
//#define RIDE_TIME   0
//#define FTOM_TIME   0

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

int sequence[16*7];

//index to sequence array
volatile int seq_count;
volatile int untz_poll;

volatile bool control_buttons[16];

volatile bool mute_flag;

volatile bool usr_ctrl = false;
//pwm stuff
int eraser = 7;

int loop_count = 0;


void mute(){
  mute_flag = true;
}

void unmute(){
  seq_count = 0;
  
  mute_flag = false;
}

void phase_rst(){
  seq_count = 0;   
}


void new_beat()
{
  // if the control button is on, leave it.
  if (!control_buttons[seq_count]) {
    trellis.clrLED(map_seq_count_to_untz_index(seq_count-1));
  }
  trellis.setLED(map_seq_count_to_untz_index(seq_count));

  seq_count++;
  seq_count = seq_count % NUMBER_OF_STEPS;
  
  if (seq_count-1 == 0) {
    trellis.clrLED(map_seq_count_to_untz_index(15));
  }

}


// This whole thing is to map the buttons nicely to the drums,
// without changing how the drums are addressed.
// This function returns a negative number for the control row.
// The first crtl button here is -1 to -16
int get_drum_index(int i) {
  int result = 0;

  int block_num = i/16;
  int button_num = i%16;

  // flip the blocks.
  if (block_num < 4) {
    block_num = block_num * 2;
  } else {
    block_num = block_num * 2 - 7;
  }

  int row = button_num / 4 + 4*(block_num % 2);
  int col = button_num % 4 + 4*(block_num / 2);

  // this is to make the top drum start from zero
  // if the row is -1, then it is one of the control rows. (play, pause, shift phase, etc.)
  row--;
  
  result = col*7+row;

  if (row == -1) {
    result = -col-1;
  }
  
  // put it back together
//  Serial.print("Row number: "); Serial.println(row);
//  Serial.print("Col number: "); Serial.println(col);
//  Serial.print("Block number: "); Serial.println(block_num);
//  Serial.print("Button number: "); Serial.println(button_num);
//  Serial.print("Drum number: "); Serial.println(result);

  
  return result;
}

int map_seq_count_to_untz_index(int seq_count) {
  int result;
  int block_num = seq_count / 4;
  int block_index = seq_count % 4;

  result = block_num*16 + block_index;
  return result;
}

void clear_trellis(void) {
  trellis.clear();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Trellis Demo");
  Serial2.begin(9600);
  Serial3.begin(9600);
  // INT pin requires a pullup
  pinMode(INTPIN, INPUT);
  digitalWrite(INTPIN, HIGH);

  
  // begin() with the addresses of each panel in order
  // I find it easiest if the addresses are in order
//  trellis.begin(0x70);  // only one
   trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);  // or four!

//  // light up all the LEDs in order
//  for (uint8_t i=0; i<numKeys; i++) {
//    trellis.setLED(i);
//    trellis.writeDisplay();    
//    delay(2);
//  }
//  // then turn them off
//  for (uint8_t i=0; i<numKeys; i++) {
//    trellis.clrLED(i);
//    trellis.writeDisplay();    
//    delay(2);
//  }

  // now do the drums
  seq_count = 0;
  pinMode(KICK,OUTPUT);
  pinMode(SNARE,OUTPUT);
  pinMode(HAT,OUTPUT);
  pinMode(CRASH,OUTPUT);
  pinMode(TOM1,OUTPUT);
  pinMode(RIDE,OUTPUT);
  pinMode(FTOM,OUTPUT);
  
  TCCR1A = 0; //Timer 1 (used by servo lib)
  TCCR1B = 0;

//  attachInterrupt(digitalPinToInterrupt(MUTE_IN),mute,HIGH); //Mute/unmute drums
//  attachInterrupt(digitalPinToInterrupt(MUTE_IN),unmute,LOW);

  // Whenever pin 19 goes from low to high write drums
  attachInterrupt(digitalPinToInterrupt(PULSE_IN),new_beat,RISING); 

  //Serial stuff
  untz_poll = 0;

  //initialising sequence
  
//  sequence = {0};
  
  
  Serial.println("Setup finished");
  usr_ctrl = true;
}


void loop() {
  
  delay(30); // 30ms delay is required, dont remove me!
  int drum_index = 0;
  
  if(usr_ctrl){
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {  
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed...
        if (trellis.justPressed(i)) {
          
          // Alternate the LED
          // need to find the block number
//          drum_index = get_drum_index(i);
//          if (drum_index <= -1) {
//
//            // if the ctrl button just pressed was already on.
//            // toggle the bit at the end
//            if (control_buttons[1-drum_index]) {
//              control_buttons[1-drum_index] = false;
//            } else {
//              control_buttons[1-drum_index] = true;
//            }
//            //I need to update the top row here
//            // fix drum index to 
//          } 
          
          if (trellis.isLED(i)) {
            sequence[drum_index] = NO_HIT;
            Serial2.print((char)get_drum_index(i));
            trellis.clrLED(i);
          } else {
            sequence[drum_index] = HARD;
            Serial2.print((char)get_drum_index(i));
            trellis.setLED(i);
          }
        } 
      }
      // tell the trellis to set the LEDs we requested
      
    }
  }
  trellis.writeDisplay();

  while(Serial2.available() > 0){
    char temp = (char)Serial2.read();
//    Serial2.flush();
//    Serial.println(temp);
    switch (temp) {
      case '1':
        seq_count = 0;
        new_beat();
        break;
        case '2':
        seq_count = 4;
        new_beat();
        break;
        case '3':
        seq_count = 8;
        new_beat();
        break;
      case '4':
        seq_count = 12;
        new_beat();
        break;
      case 's':
        new_beat();
        break;
      case 'c':
        //flash, then clear
        for (uint8_t i=0; i<numKeys; i++) {
          trellis.setLED(i);
        }
//        trellis.setBrightness(10);
        trellis.writeDisplay();
        delay(200);
        clear_trellis();
        break;
      default:
        // shit is fucked.
        break;
    }

  }
  
}

