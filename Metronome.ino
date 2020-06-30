#include <LiquidCrystal.h>
#include "pitches.h"
#include "uClock.h"

//////////////////////////////////////////////
// Rotary Encoder
//////////////////////////////////////////////
 // Rotary Encoder Inputs
 #define inputCLK 5
 #define inputDT 4
 
 // LED Outputs
 #define ledCW 10
 #define ledCCW 9
 
 int counter = 0; 
 int currentStateCLK;
 int previousStateCLK; 

 String encdir ="";

//////////////////////////////////////////////
// BPM
//////////////////////////////////////////////
//PINS
const int SPEAKER_PIN = 8;
const int MASTER_CLOCK_PIN = 22;
const int MULT_CLOCK_ONE_PIN = 23;
const int knockSensor = A0;

const int threshold = 200;
const boolean ACCENT = true;

int BPM = 1;
int multOneBPM = 2;

unsigned long masterLast;
unsigned long masterTDelay;

unsigned long multClockOneLast;
unsigned long multClockOneTDelay;

int signiture = 4;


int noteDurationMaster;
int noteDurationMultOne;

int masterBeat;
int multClockOneBeat;


//////////////////////////////////////////////
// State Machine
//////////////////////////////////////////////
const int MASTER_STATE = 0;

const int MULT_CLOCK_ONE_STATE = 1;
const int MULT_CLOCK_TWO_STATE = 2;
const int MULT_CLOCK_THREE_STATE = 3;

const int DIV_CLOCK_ONE_STATE = 1;
const int DIV_CLOCK_TWO_STATE = 2;
const int DIV_CLOCK_THREE_STATE = 3;



void setup() {

  //////////////////////////////////////////////
  // Rotary Encoder Stuff
  //////////////////////////////////////////////
   // Set encoder pins as inputs  
   pinMode (inputCLK,INPUT);
   pinMode (inputDT,INPUT);
   
   // Set LED pins as outputs
   pinMode (ledCW,OUTPUT);
   pinMode (ledCCW,OUTPUT);
   
   // Setup Serial Monitor
   Serial.begin (9600);
   
   // Read the initial state of inputCLK
   // Assign to previousStateCLK variable
   previousStateCLK = digitalRead(inputCLK);

  
  Serial.begin(9600);

  //////////////////////////////////////////////
  // BPM Stuff
  //////////////////////////////////////////////
  pinMode(MASTER_CLOCK_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(MULT_CLOCK_ONE_PIN, OUTPUT);
  //pinMode(BUTTON_PIN, INPUT);
  
  
  masterBeat = 0;
  multClockOneBeat = 0;
  
  BPM = 120;
  //calculate seconds per beat
  
  masterTDelay = 60000/BPM;
  multClockOneTDelay = 60000/BPM;
  
  masterLast = millis();
  multClockOneLast = millis();
  
  // to calculate the note duration, take one second
  // divided by the note type.
  //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  noteDurationMaster = 1000 / 0.5;
  noteDurationMultOne = 1000 / 8;
}

void loop() {

   //////////////////////////////////////////////
   // Rotary Encoder Stuff
   //////////////////////////////////////////////
   // Read the current state of inputCLK
   currentStateCLK = digitalRead(inputCLK);
  
   // If the previous and the current state of the inputCLK are different then a pulse has occured
   if (currentStateCLK != previousStateCLK){ 
       
     // If the inputDT state is different than the inputCLK state then 
     // the encoder is rotating counterclockwise
     if ((digitalRead(inputDT)) != currentStateCLK) { 
       counter --;
       encdir ="CCW";
       digitalWrite(ledCW, LOW);
       digitalWrite(ledCCW, HIGH);
       if(BPM >= 30){
        BPM -= 5;
       }
       Serial.println("BPM: " + String(BPM));
       Serial.println("MultOne BPM: " + String(multOneBPM));
       
     } else {
       // Encoder is rotating clockwise
       counter ++;
       encdir ="CW";
       digitalWrite(ledCW, HIGH);
       digitalWrite(ledCCW, LOW);
       if(BPM <= 300){
        BPM += 5;
       }
       Serial.println("BPM: " + String(BPM));
       Serial.println("MultOne BPM: " + String(multOneBPM));
       
     }
     Serial.print("Direction: ");
     Serial.print(encdir);
     Serial.print(" -- Value: ");
     Serial.println(counter);
   } 
   // Update previousStateCLK with the current state
   previousStateCLK = currentStateCLK; 

  //////////////////////////////////////////////
  // BPM
  //////////////////////////////////////////////
  masterClock();
  multClockOne();
}
void masterClock() {
  int elapsed = millis() - masterLast;
  if(BPM == 0) { 
    return;
  }
  if(elapsed > noteDurationMultOne) { 
    digitalWrite(MASTER_CLOCK_PIN, LOW);
  }
  masterTDelay = 60000/BPM;
  if(elapsed < masterTDelay) { 
    return;
  }
  int play_note = NOTE_C4;
  masterBeat = masterBeat % signiture;
  if(ACCENT && masterBeat == 0) {
    play_note = NOTE_C6;
  }
  tone(SPEAKER_PIN, play_note, noteDurationMaster);
  digitalWrite(MASTER_CLOCK_PIN, HIGH);
  masterLast = millis();
  masterBeat++;
}

void multClockOne() {
  int elapsed = (millis() - multClockOneLast);
  if((BPM) == 0) { 
    return;
  }
  if(elapsed > noteDurationMaster) { 
    digitalWrite(MULT_CLOCK_ONE_PIN, LOW);
  }
  multClockOneTDelay = (60000/BPM);
  if(elapsed < multClockOneTDelay) { 
    return;
  }
  digitalWrite(MULT_CLOCK_ONE_PIN, HIGH);
  multClockOneLast = millis();
  masterBeat++;
}
