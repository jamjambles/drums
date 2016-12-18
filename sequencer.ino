//sequence specs
#define NUMBER_OF_STEPS_PER_BEAT 4
#define NUMBER_OF_BEATS 16
#define NUMBER_OF_DRUMS 7
#define NUMBER_OF_STEPS (NUMBER_OF_STEPS_PER_BEAT*NUMBER_OF_BEATS)

//sequence order
#define SNARE_OFFSET  0
#define KICK_OFFSET   1
#define HAT_OFFSET    2
#define CRASH_OFFSET  3
#define TOM1_OFFSET   4
#define RIDE_OFFSET   5
#define FTOM_OFFSET   6

//hit strengths
#define WELL    255
#define MED     180
#define RARE    200
#define NO_HIT  0

//interupt pins
const byte PULSE_IN = 19;

//drum pins
#define SNARE 2
#define KICK  3
#define HAT   5
#define CRASH 6
#define TOM1  7
#define RIDE  8
#define FTOM  9

//timer
#define CLOCK_SPEED   16*10^6 //16 MHz
#define PRESCALER     1024
#define TIMER_RES     (1 / ( CLOCK_SPEED / PRESCALER))
#define TIMER_TIME    5 //ms
#define TIMER_COUNTS  ((TIMER_TIME*10^-3 / TIMER_RES) -1)

//drum times
#define KICK_TIME   130 //ms
#define SNARE_TIME  40  //ms  
#define HAT_TIME    40  //ms
#define CRASH_TIME  80  //ms
#define TOM1_TIME   40
#define RIDE_TIME   100
#define FTOM_TIME   50

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

volatile int seq_count;
/*
volatile int sequence[NUMBER_OF_STEPS*NUMBER_OF_DRUMS] = 
{
//snare, kick, hat, crash, tom1, ride, ftom
  NO_HIT, WELL, WELL, WELL, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 1
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 4
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 1
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 4
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 1
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 4
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4
  
  WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,     //step 1  `//beat 1
  WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 2
  WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,    //step 3
  WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT,  //step 4

  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,     //step 1  `//beat 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,  //step 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT,  //step 4

  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL,     //step 1  `//beat 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL,  //step 2
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL,    //step 3
  NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL,  //step 4
  
};
*/

volatile int sequence[NUMBER_OF_STEPS*NUMBER_OF_DRUMS] = 
{
//snare, kick, hat, crash, tom1, ride, ftom
//bar 1, beat 1
NO_HIT, WELL, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 2
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 3
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 4
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//bar 2 beat 1
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, WELL, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, WELL, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 2
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 3
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 4
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

//bar 3 beat 1
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 2
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 3
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 4
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

//bar 4 beat 1
NO_HIT, WELL, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 2
WELL, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 

WELL, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, 


//beat 3
WELL, NO_HIT, WELL, NO_HIT, WELL, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, 


//beat 4
NO_HIT, NO_HIT, WELL, NO_HIT, NO_HIT, NO_HIT, WELL, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, 

NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, NO_HIT, WELL, 
};
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
  
  attachInterrupt(digitalPinToInterrupt(PULSE_IN),write_drums_high,RISING); //Whenever pin 20 goes from low to high write drums

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

  set_pwm_2_3_5_6_7_8_9_10(); //setting pwm frequency to 31KHz
}

void loop() {

}

void write_drums_high()
{

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
    analogWrite(TOM1,sequence[seq_count*NUMBER_OF_DRUMS + TOM1_OFFSET]);  //crash
    tom1_active = true;
    is_hit = true;
  }

  if(ride_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET] != NO_HIT) 
  {
    analogWrite(RIDE,sequence[seq_count*NUMBER_OF_DRUMS + RIDE_OFFSET]);  //crash
    ride_active = true;
    is_hit = true;
  }

  if(ftom_active==false&&sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET] != NO_HIT) 
  {
    analogWrite(FTOM,sequence[seq_count*NUMBER_OF_DRUMS + FTOM_OFFSET]);  //crash
    ftom_active = true;
    is_hit = true;
  }
        
  if (is_hit)
    begin_5_timer();
       
  seq_count++;
  seq_count = seq_count % NUMBER_OF_STEPS;
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
 
/*OLD CODE
  TCCR3A = 0; //use this one for kick
  TCCR3B = 0;
void begin_150_timer()
{
  OCR3A = 3*780; //will count to 150ms
  TCCR3B |= (1 << WGM12); //compare mode
  TCCR3B |= (1 << CS30);// 1024 prescaler
  TCCR3B |= (1 << CS32);
  TIMSK3 |= (1 << OCIE3A);//enable compare interrupt
}

ISR(TIMER3_COMPA_vect)
{
  analogWrite(KICK,0); //after 150ms write kick to low
  TCCR3A = 0;
  TCCR3B = 0;//stop timer
}
*/




