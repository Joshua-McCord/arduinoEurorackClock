// Roland TB303 Step Sequencer engine clone.
// No interface here, just the engine as example.
// author: midilab contact@midilab.co
// Under MIT license
#include "Arduino.h"
#include "uClock.h"

// Sequencer config
#define STEP_MAX_SIZE      16
#define NOTE_LENGTH        4 // min: 1 max: 5 DO NOT EDIT BEYOND!!!
#define NOTE_VELOCITY      90
#define ACCENT_VELOCITY    127

// MIDI config
#define MIDI_CHANNEL      0 // 0 = channel 1

// do not edit below!
#define NOTE_STACK_SIZE    3

// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC
#define NOTE_ON    0x90
#define NOTE_OFF   0x80

// Sequencer data
typedef struct
{
  uint8_t note;
  bool accent;
  bool glide;
  bool rest;
} SEQUENCER_STEP_DATA;

typedef struct
{
  uint8_t note;
  int8_t length;
} STACK_NOTE_DATA;

// main sequencer data
SEQUENCER_STEP_DATA _sequencer[STEP_MAX_SIZE];
STACK_NOTE_DATA _note_stack[NOTE_STACK_SIZE];
uint16_t _step_length = STEP_MAX_SIZE;

// make sure all above sequencer data are modified atomicly only
// eg. ATOMIC(_sequencer[0].accent = true); ATOMIC(_step_length = 7);
uint8_t _tmpSREG;
#define ATOMIC(X) _tmpSREG = SREG; cli(); X; SREG = _tmpSREG;

// shared data to be used for user interface feedback
bool _playing = false;
uint16_t _step = 0;

void sendMidiMessage(uint8_t command, uint8_t byte1, uint8_t byte2)
{ 
  
  // send midi message
  //command = command | (uint8_t)MIDI_CHANNEL;
  //Serial.write(command);
  //Serial.write(byte1);
  //Serial.write(byte2);
  if(command == NOTE_ON){
     digitalWrite(22, HIGH);
     tone(8, 1000, 1000/32);
  }
  else if(command == NOTE_OFF) {
    digitalWrite(22, LOW);
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void ClockOut16PPQN(uint32_t * tick) 
{
  uint16_t step;
  uint16_t length = NOTE_LENGTH;
  
  // get actual step.
  _step = *tick % _step_length;
  
  // send note on only if this step are not in rest mode
  if ( _sequencer[_step].rest == false ) {

    // check for glide event ahead of _step
    step = _step;
    for ( uint16_t i = 1; i < _step_length; i++  ) {
      ++step;
      step = step % _step_length;
      if ( _sequencer[step].glide == true && _sequencer[step].rest == false ) {
        length = NOTE_LENGTH + (i * 6);
        break;
      } else if ( _sequencer[step].rest == false ) {
        break;
      }
    }

    // find a free note stack to fit in
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      if ( _note_stack[i].length == -1 ) {
        _note_stack[i].note = _sequencer[_step].note;
        _note_stack[i].length = length;
        // send note on
        sendMidiMessage(NOTE_ON, _sequencer[_step].note, _sequencer[_step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY);    
        return;
      }
    }
  }  
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t * tick) 
{
  // Send MIDI_CLOCK to external hardware
  Serial.write(MIDI_CLOCK);

  // handle note on stack
  for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
    if ( _note_stack[i].length != -1 ) {
      --_note_stack[i].length;
      if ( _note_stack[i].length == 0 ) {
        sendMidiMessage(NOTE_OFF, _note_stack[i].note, 0);
        _note_stack[i].length = -1;
      }
    }  
  }
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart() 
{
  Serial.write(MIDI_START);
  _playing = true;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  Serial.write(MIDI_STOP);
  // send all note off on sequencer stop
  for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
    sendMidiMessage(NOTE_OFF, _note_stack[i].note, 0);
    _note_stack[i].length = -1;
  }
  _playing = false;
}

void setup() 
{
  // Initialize serial communication
  // the default MIDI serial speed communication at 31250 bits per second
  Serial.begin(31250); 

  // Inits the clock
  uClock.init();
  
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  
  // Set the callback function for the step sequencer on 16ppqn
  uClock.setClock16PPQNOutput(ClockOut16PPQN);  
  
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  
  // Set the clock BPM to 126 BPM
  uClock.setTempo(80);

  // initing sequencer data
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    _sequencer[i].note = 48;
    _sequencer[i].accent = false;
    _sequencer[i].glide = false;
    _sequencer[i].rest = false;
  }

  // initing note stack data
  for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
    _note_stack[i].note = 0;
    _note_stack[i].length = -1;
  }

  // pins, buttons, leds and pots config
  //configureYourUserInterface();

  pinMode (22,OUTPUT);
  
  // start sequencer
  uClock.start();

}

// User interaction goes here
void loop() 
{
  // Controls your 303 engine interacting with user here...
  // you can change data by using _sequencer[] and _step_length only! do not mess with _note_stack[]!
  // IMPORTANT!!! Sequencer main data are used inside a interrupt enabled by uClock for BPM clock timing. Make sure all sequencer data are modified atomicly using this macro ATOMIC();
  // eg. ATOMIC(_sequencer[0].accent = true); ATOMIC(_step_length = 7);
  //processYourButtons();
  //processYourLeds();
  //processYourPots();
}
