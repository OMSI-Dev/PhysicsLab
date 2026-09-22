// Granular Effect Example - Pitch shift or freeze sound
//
// Adapted from the PJRC audio tutorial granular example for a
// bare Teensy 4.0 + PT8211 DAC, controlled over the serial
// monitor instead of buttons/knobs:
//   '1' - start Freeze effect
//   '2' - start Pitch Shift effect
//   '3' - stop the current effect
//   '+' / '-' - raise/lower pitch shift speed (0.5x to 2.0x)
//
// Teensy 4.0 -> PT8211 wiring:
//   Pin 7  -> DIN,  Pin 20 -> WS,  Pin 21 -> BCK
//   3.3V   -> VCC,  GND -> GND
//
// SD card on SPI (e.g. audio shield or SD breakout):
//   Pin 10 -> CS, Pin 11 -> MOSI, Pin 12 -> MISO, Pin 13 -> SCK
//
// Data files to put on your SD card can be downloaded here:
//   http://www.pjrc.com/teensy/td_libs_AudioDataFiles.html
//
// This example code is in the public domain.

#include <Audio.h>
#include <SPI.h>
#include <SD.h>

AudioPlaySdWav           playSdWav1;     //xy=163,135
AudioMixer4              mixer1;         //xy=332,167
AudioEffectGranular      granular1;      //xy=504,155
AudioOutputPT8211        dac;            //xy=664,185
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, granular1);
AudioConnection          patchCord4(granular1, 0, dac, 0);
AudioConnection          patchCord5(granular1, 0, dac, 1);

#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

// grain lengths formerly set by the A3 knob (mid position)
#define FREEZE_GRAIN_MSEC  150.0
#define SHIFT_GRAIN_MSEC   50.0

#define NUM_FILES  4
const char *filenames[NUM_FILES]={"SDTEST1.WAV", "SDTEST2.WAV", "SDTEST3.WAV", "SDTEST4.WAV"};
int nextfile=0;

float speedRatio = 1.0;

void setup() {
  Serial.begin(9600);
  AudioMemory(10);

  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  // the Granular effect requires memory to operate
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
  granular1.setSpeed(speedRatio);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  Serial.println("Granular effect ready:");
  Serial.println("  1 = freeze, 2 = pitch shift, 3 = stop, +/- = speed");
}

void loop() {
  if (playSdWav1.isPlaying() == false) {
    // start the next song playing
    playSdWav1.play(filenames[nextfile]);
    Serial.print("Playing: ");
    Serial.println(filenames[nextfile]);
    delay(5); // brief delay for the library read WAV info
    nextfile = nextfile + 1;
    if (nextfile >= NUM_FILES) {
      nextfile = 0;
    }
  }

  // read keypresses from the serial monitor
  while (Serial.available() > 0) {
    char key = Serial.read();

    if (key == '1') {
      granular1.beginFreeze(FREEZE_GRAIN_MSEC);
      Serial.print("Begin granular freeze using ");
      Serial.print(FREEZE_GRAIN_MSEC);
      Serial.println(" ms grains");
    }
    if (key == '2') {
      granular1.beginPitchShift(SHIFT_GRAIN_MSEC);
      Serial.print("Begin granular pitch shift using ");
      Serial.print(SHIFT_GRAIN_MSEC);
      Serial.println(" ms grains");
    }
    if (key == '3') {
      granular1.stop();
      Serial.println("Effect stopped");
    }

    // adjust pitch shift speed, one semitone per press
    if (key == '+' || key == '=' || key == '-') {
      if (key == '-') {
        speedRatio /= 1.05946;
      } else {
        speedRatio *= 1.05946;
      }
      if (speedRatio < 0.5) speedRatio = 0.5;
      if (speedRatio > 2.0) speedRatio = 2.0;
      granular1.setSpeed(speedRatio);
      Serial.print("Speed ratio: ");
      Serial.println(speedRatio);
    }
  }
}
