//Drum sequencer by James Wagner and Angus Keatinge. For use with arduino mega2560


#include <Wire.h>
#include "Adafruit_Trellis.h"

// How to hook up the hardware.
// Connect Trellis Vin to 5V and Ground to ground.
// Connect I2C SDA pin to your Arduino SDA line
// Connect I2C SCL pin to your Arduino SCL line

// Initialise each individual 4x4 board on the trellis
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_Trellis matrix2 = Adafruit_Trellis();
Adafruit_Trellis matrix3 = Adafruit_Trellis();
Adafruit_Trellis matrix4 = Adafruit_Trellis();
Adafruit_Trellis matrix5 = Adafruit_Trellis();
Adafruit_Trellis matrix6 = Adafruit_Trellis();
Adafruit_Trellis matrix7 = Adafruit_Trellis();

// define our trellis board as a set of small trellis boards.
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet( &matrix0, &matrix1, &matrix2, &matrix3,
                                                    &matrix4, &matrix5, &matrix6, &matrix7);

#define NUMTRELLIS 8
#define numKeys (NUMTRELLIS * 16)

// sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BARS 1
#define NUMBER_OF_BEATS_PER_BAR 4
#define NUMBER_OF_BEATS (NUMBER_OF_BARS*NUMBER_OF_BEATS_PER_BAR)
#define NUMBER_OF_DRUMS 7
#define REST 0
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS+REST)
#define SEQUENCE_LENGTH (NUMBER_OF_STEPS*NUMBER_OF_DRUMS)

// hit strength codes, this code is used for sending stuff over serial.
// Provides ability to accent notes.
#define HARD    '3'
#define MED     '2'
#define SOFT    '1'
#define NO_HIT  '0'

// interupt pins
#define PULSE_IN 18 // The beats from the pi


////////////////////////////
// control button indexes //
////////////////////////////
// We can switch between these 4 bars, the state will be saved when changing bars.
// When you change bars, the trellis will change state right away but the drums will keep with the current sequence till next bar.
// or the trellis will only change at the end of the bar.
// Only one of these four can be on at a time, pressing one turns off the others.
// We will have to write the bar serially to the drum sequencer mega.
#define FIRST_BAR   0
#define SECOND_BAR  1
#define THIRD_BAR   2
#define FOURTH_BAR  3

#define CLR_DRUM 4 // while this is on, any drum buttons pressed will have their lines cleared.
/*
 * Clears the current bar right away.
 * This should call flash_trellis(), doesn't need to do anything else really...
 */
#define CLR_CURR_BAR 5

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
// Only have the formula to map one way so I need to keep both...
char drum_sequence[4][SEQUENCE_LENGTH];
bool light_sequence[4][numKeys]; // this does no light up the control buttons, there is redundancy for ease of indexing.

//index to sequence array
volatile int seq_count;

volatile bool usr_ctrl = false;

void(* resetFunc) (void) = 0;

/*
 * This function steps along the visual beat on the top row of the tellis.
 * checks if any of the top row buttons should be on and leaves them on.
 * nb: doesn't call writeDisplay();
 */
void new_beat()
{
  Serial.print("old beat: ");
  Serial.println(seq_count);
  trellis.setLED(map_seq_count_to_untz_index(seq_count));
  
  if (!control_button_active[seq_count-1]) {
    trellis.clrLED(map_seq_count_to_untz_index(seq_count-1));
  }
  
  seq_count++;
  seq_count = seq_count % NUMBER_OF_STEPS;

  // catches the case where the last light gets left on.
  if (seq_count-1 == 0) {
    trellis.clrLED(map_seq_count_to_untz_index(15));
  }

  fix_ctrl_btn_display();
  Serial.print("new beat: ");
  Serial.println(seq_count);

}

/*
 * This whole thing is to map the buttons nicely to the drums, without changing how the drums are addressed.
 * This function returns a negative number for the control row.
 * The first crtl button here is -1 to -16
 */
int map_untz_index_to_drum_index(int i) {
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
    
  return result;
}

int map_seq_count_to_untz_index(int seq_count) {
  int result;
  int block_num = seq_count / 4;
  int block_index = seq_count % 4;

  result = block_num*16 + block_index;
  return result;
}

/*
 * We flash every light on, then we clear the whole board and send the clear msg to the drum controller
 */
void flash_trellis(void) {
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
  }
  trellis.writeDisplay();
  delay(200);
  
  clear_trellis();
}

/*
 * Clears the trellis display and sends a message to the other duino, telling it to clear it's drum sequence.
 */
void clear_trellis(void) {
  // clear the lights
  trellis.clear();
  
  // send a message to the other mega to clear the drum sequence.
  Serial2.print((char)-16);
  Serial2.flush();

}

// I need to loop through the control buttons, and if any are meant to be on, I need set them to be on...
void fix_ctrl_btn_display(void) {
  for (int i = 0; i < 16; ++i) {
    if (control_button_active[i] == true) {
      Serial.print("Control button: ");
      Serial.print(i);
      Serial.println(" is active");
      // set LED???
    }
  }
}

void setup() {
  /*
   * Serial is used to print development messages out to the computer.
   * I think the trellis is also hooked up to this line because this is the onlt UART line on the mega.
   */
  Serial.begin(9600);
  Serial.println("Trellis Demo");

  /*
   * Serial2 is the line used to communicate with the other arduino.
   * It recieves 2 things from the drum sequencer duino
   *      1) beats: 1, 2, 3, 4 beats of the bar and s for sub-beats.
   *      2) 'clear all' instruction (for when the other duino restarts)
   */
  Serial2.begin(9600); // comms with the drum sequencer arduino

  // Set up trellis
  trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);

  // This sets the drums to the initial beat.
  seq_count = 0;

  // Set all of these values to false.
  for (int i = 0; i < 16; ++i) {
    control_button_active[i] = false;
  }
  

  // Whenever pin 19 goes from low to high write drums
  // The signal that the raspberry writes out is a very fast pulse, 
  //    so whether this is rising or falling doesn't _really_ matter.
  attachInterrupt(digitalPinToInterrupt(PULSE_IN),new_beat,RISING);

  // Trellis animation to show that it has started and is good to go.
  flash_trellis();
  
  // Only allow users to press buttons once the setup is complete.
  usr_ctrl = true;

  // Set the 0th bar as being active.
  control_button_active[0] = true;
  trellis.setLED(0);

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 112; ++j) {
      drum_sequence[i][j] = '0';
    }
  }

  
  Serial.println("Setup finished");
  
}

// cannot reset this every loop!!!
int curr_bar = 0;
int active_bar = 0;

// Everything breaks if there is more than one trellis.writeDisplay() per 30ms
void loop() {
  
  delay(30); // 30ms delay is required, dont remove me!
  int drum_index = 0;
  int ctrl_button_num = 0;

  // This is by default true
  // it is only not true with we are playing a song with a predefined drum sequence which is loaded in from th raspberry Pi.
  if(usr_ctrl){
    
    // This function call communicates via serial with the trellis board and updates all of the trellis state.
    //      ie. isKeyPressed, wasKeyPressed, etc.
    if (trellis.readSwitches()) {
      Serial.println("Button has been pressed");
      
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        
        if (trellis.justPressed(i)) {
          drum_index = map_untz_index_to_drum_index(i);

          // Is a control button
          if (drum_index <= -1) {
            Serial.println("was a control button.");
            Serial.print("Drum index: ");
            Serial.println(drum_index);
            
            ctrl_button_num = -1-drum_index; // maps the button code to the array index.
            Serial.print("ctrl_button_num: ");
            Serial.println(ctrl_button_num);
            
            // Which control button is it?
            switch (ctrl_button_num) {
              case 0:
              case 1:
              case 2:
              case 3:
                curr_bar = ctrl_button_num;
                break;
              case 4:
                control_button_active[4] = !control_button_active[4];
                break;
              case 15:
                resetFunc();
              default:
                break;
            }


            // if it's a control button I update the light sequence for all bars
            // but I leave the drum sequence alone
            // I handle the bars elsewhere.
            if (ctrl_button_num > 3) {
              for (int j = 0; j < 4; ++j) {
                if (trellis.isLED(i)) {
                  light_sequence[j][i] = true;
                } else {
                  light_sequence[j][i] = false;
                }
              }

            } else {
              for (int j = 0; j < 4; ++j) {
                if (trellis.isLED(i)) {
                  light_sequence[j][i] = true;
                } else {
                  light_sequence[j][i] = false;
                }
              }
            }
            
          // Not a control button
          // Now I update both the light sequence and the drum sequence.
          // I only sent this to the drum sequencer if it is a drum hit.
          } else {
            
            if ( trellis.isLED(i) ) {
              if (curr_bar == active_bar) {
                Serial2.print((char)drum_index);
              }
              drum_sequence[curr_bar][drum_index] = NO_HIT;
              light_sequence[curr_bar][i] = false;
            } else if ( !trellis.isLED(i) ) {
              if (curr_bar == active_bar) {
                Serial2.print((char)drum_index);
              }
              drum_sequence[curr_bar][drum_index] = HARD;
              light_sequence[curr_bar][i] = true;
            }
          }
          
          /*
           * A key was pressed.
           * update trellis board.
           * I do this no matter what key was pressed.
           * 
           * I don't send the info if we're on the wrong bar though
           */
          if (trellis.isLED(i)) {
            trellis.clrLED(i);
            light_sequence[curr_bar][i] = false;
          } else {
            trellis.setLED(i);
            light_sequence[curr_bar][i] = true;
          }
        }
      }
      
    }
  }
  
  // Listen for beats and update the 
  // Serial2 is the drum driving arduino
  // 
  while(Serial2.available() > 0){
    char temp = (char)Serial2.read();
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
      case 'b':
      case 's':
        new_beat();
        break;
      case 'c':
        // The other trellis has reset.
        resetFunc();
        break;
      default:
        // shit is fucked.
        break;
    }
  }

  // The active bar has been changed.
  // There is a discrepancy between the control button array and the current bar.
  if (control_button_active[curr_bar] == false) {

    // update the control_button_active[] array.
    for (int i = 0; i < 4; ++i) {
      if ( i == curr_bar ) {
        control_button_active[i] = true;
      } else {
        control_button_active[i] = false;
      }
    }

    // set the light sequence for the bar buttons
    for (int i = 0; i < 4; ++i) {
      // 
      for (int j = 0; j < 4; ++j) {
        if ( j == curr_bar ) {
          light_sequence[i][j] = true;
        } else {
          light_sequence[i][j] = false;
        }
      }
    }



    /* If I want to change the lights straight away I need to:
     *      - update the lights right away -> done
     *      - change which light_sequence[] is being updated -> done
     *      - change which drum_sequence[] is being updated -> done
     *      - stop sending info to the drum_sequencer 
     *      - at the end of the bar, send all this info to the drum_sequencer
     *      
     * It will break if:
     *      - a control button is pressed at the same time as a drum button.
     */

    // This loop updates the whole board
    // Light sequence needs to be up to date.
    for (int j = 0; j < numKeys; ++j) {
      if (light_sequence[curr_bar][j] == true ) {
        trellis.setLED( j );
      } else {
        trellis.clrLED( j );
      }
    }

  }


  // check if we need to change the lights to a new bar and update brother duino
  if ( seq_count == 0 && control_button_active[4] == true) { // then we need to find which one it is

    active_bar = curr_bar;
    // turn #5 control button off.
    control_button_active[4] = false;
    trellis.clrLED( map_seq_count_to_untz_index(4) );
    light_sequence[0][32] = false;
    light_sequence[1][32] = false;
    light_sequence[2][32] = false;
    light_sequence[3][32] = false;

    // update brother duino at the end of the bar.
    // use the same protocol we used with the pi.
    // 'z' starts reading in
    // 0, 1, 2 and 3 for NO_HIT, SOFT, MED, and HARD respectively.
    // 'f' to end the sequence.
    // This should work, just need to set it up on brother duino and check it out
    Serial.println("About to send new sequence to brother duino.");

    Serial2.print('z');
    for (int j = 0; j < SEQUENCE_LENGTH; ++j) {
      Serial2.print( drum_sequence[curr_bar][j] );
    }
    Serial2.print('f');

    Serial.println("New sequence written to other duino!");

  }
  
  // this will update only the beat line that taps along with the beat.
  trellis.writeDisplay();
}

