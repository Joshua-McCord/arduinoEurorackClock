#include "Arduino.h"
#include "uClock.h"
#include <LiquidCrystal.h>

//State Machine
const int MASTER_STATE = 0;
const int CHANNEL_ONE_STATE = 1;
const int CHANNEL_TWO_STATE = 2;
const int CHANNEL_THREE_STATE = 3;
const int CHANNEL_FOUR_STATE = 4;
const int CHANNEL_FIVE_STATE = 5;
const int CHANNEL_SIX_STATE = 6;

int currentState;

//State Change Detections
const int CHANNEL_ONE_BUTTON_PIN = 33;
int channelOneButtonState = 0;
int lastChannelOneButtonState = 0;


//LCD Display
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Rotary Encoder
#define outputA 9
#define outputB 10
int counter = 0; 
int aState;
int aLastState;  

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

#define PROPER_BPM_RES  24


//Tempo and Clock Mults/Divs
int masterTempo = 80;
int clockOneCalculation = 1;

int ppqnCount;





// The callback function wich will be called by Clock each Pulse of 96PPQN clock resolution.
//PPQN is Pulse Per Quarter Note
//Resolution of 96 per quarter note means when I divide
//by 4, I can get an accurate representation of BPM 
void ClockOut96PPQN(uint32_t * tick) 
{
  // Send MIDI_CLOCK to external gears
  //Serial.write(MIDI_CLOCK);
  //PPQN is Pulse Per Quarter Note
  //Resolution of 96 per quarter note means when I divide
  //by 4, I can get an accurate representation of BPM 
  if(ppqnCount % PROPER_BPM_RES == 0) {
    tone(8, 1000, 250);
    digitalWrite(22, HIGH);
  } else {
    digitalWrite(22, LOW);
  }
  if(ppqnCount % (PROPER_BPM_RES*clockOneCalculation) == 0) {
      digitalWrite(23, HIGH);
  } else {
    digitalWrite(23, LOW);
  }
  ppqnCount++;
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart() 
{
  //Serial.write(MIDI_START);
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  //Serial.write(MIDI_STOP);
}

void setup() 
{

  //LCD DIsplay
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Tempo: " + String(masterTempo));

  //Rotary Encoder
  pinMode (outputA,INPUT);
  pinMode (outputB,INPUT);
  aLastState = digitalRead(outputA);  
  
  ppqnCount=0;
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);

  //State Machine
  currentState = MASTER_STATE;

  pinMode(CHANNEL_ONE_BUTTON_PIN, INPUT);
  
  // Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
  Serial.begin(9600);

  // Inits the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  // Set the clock BPM to 126 BPM
  uClock.setTempo(masterTempo);
  
  // Starts the clock, tick-tac-tick-tac...
  uClock.start();

}

// Do it whatever to interface with Clock.stop(), Clock.start(), Clock.setTempo() and integrate your environment...
void loop() 
{

  //ChannelOneButtonState
  channelOneButtonState = digitalRead(CHANNEL_ONE_BUTTON_PIN);
  if(lastChannelOneButtonState == HIGH && channelOneButtonState == LOW) {
    if(currentState == MASTER_STATE) {
      currentState = CHANNEL_ONE_STATE;
      lcd.clear();
    } else {
      currentState = MASTER_STATE;
      lcd.clear();
    }
    delay(50);
  }
  lastChannelOneButtonState = channelOneButtonState;

  
  //Rotary Encoder
  aState = digitalRead(outputA);
  if (aState != aLastState){     
     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
     if (digitalRead(outputB) != aState) { 
      switch(currentState) {
        case MASTER_STATE :
          if(masterTempo <= 150){
            masterTempo ++;
          }
          break;
         case CHANNEL_ONE_STATE : 
          if(clockOneCalculation <= 16) {
            clockOneCalculation++;
          }
          break;
      }
     } else {
       switch(currentState) {
        case MASTER_STATE :
          if(masterTempo >= 20){
            masterTempo--;
          }
          break;
         case CHANNEL_ONE_STATE : 
          if(clockOneCalculation >= 1) {
            clockOneCalculation--;
          }
          break;
      }
     }
     lcd.clear();
   } 
   aLastState = aState; // Updates the previous state of the outputA with the current state
  uClock.setTempo(masterTempo);

  updateLCD();
}

void updateLCD() {
  //LCD Display
  // Turn on the display:
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  // print the number of seconds since reset:
  if(currentState == MASTER_STATE) {
    lcd.print("Tempo: " + String(masterTempo));
  } else if(currentState == CHANNEL_ONE_STATE) {
    lcd.print("ClockOneMult: " + String(clockOneCalculation));
  }
  
}
