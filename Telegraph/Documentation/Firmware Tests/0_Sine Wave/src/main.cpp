#include <Arduino.h>
#include <Audio.h>

// Sine wave test tone -> PT8211 DAC over I2S.
//
// Teensy 4.0 -> PT8211
//   Pin 7  -> DIN  (data)
//   Pin 20 -> WS   (left/right word select)
//   Pin 21 -> BCK  (bit clock)
//   3.3V   -> VCC
//   GND    -> GND
// Verified shield fits Teensy 4.0 [2026-09-22 by autumn]

AudioSynthWaveformSine sine1;
AudioOutputPT8211      dac;

AudioConnection patchCord1(sine1, 0, dac, 0);
AudioConnection patchCord2(sine1, 0, dac, 1);

void setup() {
  AudioMemory(12);

  sine1.frequency(200.0);  
  sine1.amplitude(0.5);    
  delay(500);  
  sine1.frequency(150.0);  
  sine1.amplitude(0.5);    
 delay(500);  
 
  sine1.frequency(200.0);  
  sine1.amplitude(0.5);    
  delay(500);  
  sine1.frequency(100.0);  
  sine1.amplitude(0.5);   
}

void loop() {
}
