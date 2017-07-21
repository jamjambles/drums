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

// hit strengths. Provides ability to accent notes
#define HARD    255
#define MED     230
#define SOFT    200
#define NO_HIT  0

// interupt pins
const byte PULSE_IN = 18; // The beats from the pi

////////////////////////////
// control button indexes //
////////////////////////////
// We can switch between these 4 bars, the state will be saved when changing bars.
// When you change bars, the trellis will change state right away but the drums will keep with the current sequence till next bar.
// or the trellis will only change at the end of the bar.
// Only one of these four can be on at a time, pressing one turns off the others.
#define FIRST_BAR   0
#define SECOND_BAR  1
#define THIRD_BAR   2
#define FOURTH_BAR  3


#define CLR_DRUM 4 // while this is on, any drum buttons pressed will have their lines cleared.
#define CLR_CURR_BAR 5 // only flashes while you hold it and clears the current bar right away

// these fill in the notes every single, second or fourth note starting from the drum pressed.
//      TODO: should it wrap around? (no, I don't think so, what do the people expect?)
// these are toggle buttons.
//      ie. they will turn everything on if anything is off, and they will turn everything off if everything is on.
#define DRUM_FILL_CROTCHET 6 // need to hold down?
#define DRUM_FILL_QUAVER 7 // need to hold down?
#define DRUM_FILL_SEMIQUAVER 8 // need to hold down?

// Will need to communicate with the drum controller and the Raspberry Pi.
// This is a stretch goal...
#define REALIGN_DOWNBEAT_PHASE 9

// 
// Only one of these three can be on at a time, pressing one turns off the others.
#define SOFT_ACCENT 10 // need to hold down?
#define MED_ACCENT 11 // need to hold down?
#define HARD_ACCENT 12 // need to hold down?

// These are just fillers really, but might be pretty cool to use.
// So far the idea is that they fill bar 1/2/3/4 with a preset pattern, clearing what was already there.
#define PRESET_1 13
#define PRESET_2 14

// Clears all 4 bars, basically the reset everything button.
// Not sure if this is necessary. In fact it might be a very dangerous button to accidentally press.
#define CLR_ALL 15

////////////////////////////


// Not all of these are toggle buttons
volatile bool control_button_active[16];

// you will be able to program 4 bars independantly of each other.
int sequence[4][16*7];

//index to sequence array
volatile int seq_count;

volatile bool usr_ctrl = false;

/*
 * What are we going to do here?
 */
void new_beat()
{
  // if the control button is on, leave it.
  if (control_button_active[seq_count]) {
    // What do I do here???
  } else {
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

void flash_trellis(void) {
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
  }
  trellis.writeDisplay();
  delay(200);
  trellis.clear();  
}

void clear_trellis(void) {
  // clear the lights
  trellis.clear();
  
  // send a message to the other mega to clear the drum sequence.
  Serial2.print((char)-16);
  Serial2.flush();

}

void setup() {
  Serial.begin(9600);
  Serial.println("Trellis Demo");
  Serial2.begin(9600);
  Serial3.begin(9600);
  // INT pin requires a pullup
  pinMode(INTPIN, INPUT);
  digitalWrite(INTPIN, HIGH);

  // Set up trellis
  trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);

  // This sets the drums to the initial beat.
  seq_count = 0;

  // This might be used for live mode.
//  attachInterrupt(digitalPinToInterrupt(MUTE_IN),mute,HIGH); //Mute/unmute drums
//  attachInterrupt(digitalPinToInterrupt(MUTE_IN),unmute,LOW);

  // Whenever pin 19 goes from low to high write drums
  attachInterrupt(digitalPinToInterrupt(PULSE_IN),new_beat,RISING);

  // Trellis animation to show that it has started and is good to go.
  flash_trellis();
  clear_trellis();
  
  // Only allow users to press buttons once 
  usr_ctrl = true;
  Serial.println("Setup finished");
  
}


void loop() {
  
  delay(30); // 30ms delay is required, dont remove me!
  int drum_index = 0;
  int ctrl_button_num = 0;
  
  if(usr_ctrl){
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {  
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        
        if (trellis.justPressed(i)) {
          drum_index = get_drum_index(i);
          
          if (drum_index <= -1) { // then it was a control button.
            
            ctrl_button_num = 1-drum_index; // toggle the button.
            control_button_active[ctrl_button_num] = !control_button_active[ctrl_button_num];
            //I need to update the top row here
            // fix drum index to 
          } 
          
          if (trellis.isLED(i)) {
            Serial2.print((char)drum_index);
            
            sequence[0][drum_index] = NO_HIT; // have an internal index that we 
            trellis.clrLED(i);
          } else {
            Serial2.print((char)drum_index);
            
            sequence[0][drum_index] = HARD;
            trellis.setLED(i); // Need a solution for the 
          }
        }
      }
      
    }
  }
  trellis.writeDisplay();

  // Listen for beats and update the 
  while(Serial2.available() > 0){
    char temp = (char)Serial2.read();
//    Serial2.flush();
    Serial.println(temp);
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
        flash_trellis();
        clear_trellis();
        
        break;
      default:
        // shit is fucked.
        break;
    }

  }
  
}

