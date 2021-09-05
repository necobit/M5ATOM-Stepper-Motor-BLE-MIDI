#include <Arduino.h>
#include <BLEMidi.h>

#include <M5Atom.h>

#define DIR_PIN 23
#define STP_PIN 19
#define ENL_PIN 22
#define VOL_PIN 25
#define DSW_PIN 21

#define VOL_MAX (4095.0)

#define STP_MIN (30)
#define STP_MAX (1000)

bool isRuning = false;

hw_timer_t *timer = NULL; //timer 初期化

// long now;
// long old;
long stop[4] = {10000, 20000, 30000, 40000};
int dir;

volatile int count = 0;

volatile int sp = 100;
volatile boolean HL = true;
volatile boolean CW = false;
volatile int CWold = 0;
volatile boolean MV = false;

volatile int NoteLowest = 46;
volatile int NoteHighest = 98;
volatile long FullRange = 46000;
volatile int now = FullRange;
volatile byte note = (NoteLowest + NoteHighest) / 2;
volatile int target = FullRange;


void connected();

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  Serial.printf("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  if (note == 60) {
    CW = true;
    target = 10000;
    MV = true;
  }
  else if (note == 62) {
    CW = false;
    target = 20000;
    MV = true;
  }

}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  Serial.printf("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  if (note == 60 && CW == true) {
    MV = false;
  }
  else if (note == 62 && CW == false) {
    MV = false;
  }
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
  Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
  sp = 50 + (1270 - (value * 10));
}

void connected()
{
  Serial.println("Connected");
}

void setup() {
  Serial.begin(115200);
  BLEMidiServer.begin("MIDI device");
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback([]() {    // To show how to make a callback with a lambda function
    Serial.println("Disconnected");
  });
  BLEMidiServer.setNoteOnCallback(onNoteOn);
  BLEMidiServer.setNoteOffCallback(onNoteOff);
  BLEMidiServer.setControlChangeCallback(onControlChange);
  //BLEMidiServer.enableDebugging();
  M5.begin(true, false, true);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STP_PIN, OUTPUT);
  pinMode(ENL_PIN, OUTPUT);
  pinMode(VOL_PIN, INPUT);
  pinMode(DSW_PIN, INPUT_PULLUP);

  delay(50);

  timer = timerBegin(0, 80, true); //timer=1us
  timerAttachInterrupt(timer, &rotate, true);
  timerAlarmWrite(timer, 10, true); // 500ms
  timerAlarmEnable(timer);

  M5.dis.clear();
  delay(100);
  target = 0;
}

void rotate()
{
  if (MV == true)
  {
    digitalWrite(DIR_PIN, CW);
    digitalWrite(ENL_PIN, LOW);
    if (count * 10 >= sp)
    {
      count = 0;
      digitalWrite(STP_PIN, HL);
      HL = !HL;
    }
  }
  else digitalWrite(ENL_PIN, HIGH);
  count++;
}

void loop() {
  delay(10);
}
