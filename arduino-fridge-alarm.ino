#include "pitches.h"
#include "protothreads.h"

// if enabled, the melody will print to the serial port
const int silendMode = 0;

// Digital pins
const int buzzer = 3; 
const int sensor = 4;
const int statusLed = 9;
const int openLed = 10;

// Frozen - Let if go refrain
int melody[] = {
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_G4, NOTE_G4, NOTE_D5, 0,            // A B ^C G G ^D (7)
  NOTE_C5, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_C5, 0,   // ^C A A A A-B-^C (8)
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_G4, NOTE_E5, NOTE_E5, NOTE_D5, 0,    // A B ^C G ^E ^E-^D (8)
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5, 0 // ^C ^D-^E ^E ^F-^E ^D-^C ^D-^C
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 4, 1, 4, 4, 1, 2,
  2, 4, 2 ,2, 4, 4, 1, 2,
  4, 4, 1, 4, 4, 2, 1, 2,
  4, 4, 4, 2, 2, 2, 4, 1, 2
};

int noteCount = 35;
int currentNoteIndex = 0;
int pauseBetweenNotes = 0;

int ledBrightness = 0;
int fadeSteps = 5;

// Door state: 0 - closed / 1 - open
int doorState;

// 0 - alarm disabled / 1 - alarm enabled
int doorAlarmEnabled = 0;
// Special flag to kill the alarm thread properly.
int disableDoorAlarm = 0;

int doorCheckDelay = 250;
int ellapsedTime = 0;
int melodyStartTime = 30000; // 30 seconds

// Thread pointer
pt ptLedFade;
pt ptMelody;
pt ptDoorCheck;

int controlLeds(struct pt* pt) {
  PT_BEGIN(pt);

  if(doorState == 1) {
    analogWrite(openLed, 255);
  }
  else {
    analogWrite(openLed, 0);
  }

  analogWrite(statusLed, ledBrightness);
  ledBrightness = ledBrightness + fadeSteps;
  if (ledBrightness <= 0 || ledBrightness >= 250) 
  {
    fadeSteps = -fadeSteps;
  }
  
  PT_SLEEP(pt, 35);
  PT_END(pt);
}

int playMelodyThread(struct pt* pt) {
  PT_BEGIN(pt);

  Serial.println("\nStart playing melody...");
  
  // Use global note index instead of loop variables, because threads
  // will not save local variables when switching context
  currentNoteIndex = 0;
  disableDoorAlarm = 0;
  
  // iterate over the notes of the melody:
  for (;currentNoteIndex < noteCount; currentNoteIndex++) {

    // Disable melody when the "disableDoorAlarm" flag was set
    if(disableDoorAlarm == 1) {
      Serial.println("Shutdown Melody...");
      disableDoorAlarm = 0;
      doorAlarmEnabled = 0; 
      break;
    }

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[currentNoteIndex];

    if(silendMode == 0) {
      tone(buzzer, melody[currentNoteIndex], noteDuration);
    }
    else {
      Serial.print("~ ~ ~ : ");
      Serial.println(melody[currentNoteIndex]);
    }

    // to distinguish the notes, set a minimum time between them.
    pauseBetweenNotes = noteDuration * 1.10;
    PT_SLEEP(pt, pauseBetweenNotes);

    // stop the tone playing:
    noTone(buzzer);
  }

  noTone(buzzer);
  
  PT_END(pt);
}

int doorCheckThread(struct pt* pt) {
  PT_BEGIN(pt);
  
  int doorStateValue = digitalRead(sensor);

  if(doorState != doorStateValue) {
    Serial.print("\nDoor state changed: ");
    if(doorStateValue == HIGH) {
      Serial.println("CLOSED -> OPEN: Beginn Countdoun...");
    }
    else {
      Serial.println("OPEN -> CLOSED: Reset...");
    }
  }

  doorState = doorStateValue;

  if (doorState == HIGH && doorAlarmEnabled == 0){
    ellapsedTime = ellapsedTime + doorCheckDelay;
  }
  else if(doorState == LOW){
    // Set the flag so that the melody thread kills itself
    disableDoorAlarm = 1;
    ellapsedTime = 0;
  }

  if(doorState == HIGH && doorAlarmEnabled == 0) {
    Serial.print("Countdown: ");
    Serial.println(melodyStartTime - ellapsedTime);
  }
  
  if(ellapsedTime >= melodyStartTime) {
    doorAlarmEnabled = 1;
    ellapsedTime = 0;
  }
  
  PT_SLEEP(pt, doorCheckDelay);
  
  PT_END(pt);
}

void setup() {
  PT_INIT(&ptLedFade);
  PT_INIT(&ptMelody);
  PT_INIT(&ptDoorCheck);
  
  Serial.begin(9600);
  pinMode(sensor, INPUT_PULLUP);
}

void loop() {
  // Fade LED
  PT_SCHEDULE(controlLeds(&ptLedFade));

  // Check door state
  PT_SCHEDULE(doorCheckThread(&ptDoorCheck));

  // Play melody if door alarm is enabled
  // Do not use the "doorAlarmEnabled" flag to kill the melody thread!
  // First, set the "disableDoorAlarm" flag, so that the melody thread ends itself. Then, in the
  // melody thread, reset the "doorAlarmEnabled" and the "disableDoorAlarm" flag.
  if(doorAlarmEnabled == 1) {
    PT_SCHEDULE(playMelodyThread(&ptMelody));; 
  }
}
