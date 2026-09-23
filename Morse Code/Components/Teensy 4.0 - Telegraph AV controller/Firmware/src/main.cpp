// ============================================================================
// Notes:
//   - Pin 2 receives the incoming SW_data signal from the OPPOSING station's
//     telegraph key (active-low, pulled up by onboard R6). 
//   - On each received press, the press duration is classified as SHORT or
//     LONG using SHORT_LONG_THRESHOLD_MS.
//   - The classification triggers a FIXED-duration output, either 'SHORT' or 'LONG' (not a translation
//     of the actual input duration):
//       * A sine tone on the PT8211 (via Teensy Audio Library / I2S)
//       * A PWM fade on the LED dome (Pin 3), shaped to emulate an
//         incandescent bulb: fast warm-up, slower lingering cool-down.
//
// Hardware pin map:
//   Pin 2  -> incoming SW_data input (active-low, Bounce2 debounced)
//   Pin 3  -> LED dome PWM (drives onboard N-channel MOSFET, Q5)
//   Pin 21 -> PT8211 BCK
//   Pin 20 -> PT8211 WS
//   Pin 7  -> PT8211 DIN
// ============================================================================

#include <Bounce2.h>
#include <Audio.h>

constexpr uint8_t PIN_SIGNAL_IN = 2;   // signal from opposing station
constexpr uint8_t PIN_LED       = 3;   // LED dome PWM output 

constexpr uint16_t DEBOUNCE_INTERVAL_MS = 5;    // (ASSUMED) Bounce2 default-ish

// Short/long classification
constexpr uint16_t SHORT_LONG_THRESHOLD_MS = 200;

//  Fixed OUTPUT constraints 
constexpr uint16_t OUTPUT_SHORT_MS = 150;   
constexpr uint16_t OUTPUT_LONG_MS  = 450;   

// Sound
constexpr float    TONE_FREQ_HZ  = 600.0f;  
constexpr uint16_t TONE_FADE_MS  = 8;       
constexpr float    TONE_AMPLITUDE = 0.8f;   

// LED dome
constexpr uint8_t  LED_PWM_RESOLUTION_BITS = 12;                      
constexpr uint32_t LED_PWM_FREQUENCY_HZ    = 30000;                    
constexpr uint16_t LED_MAX_DUTY = (1 << LED_PWM_RESOLUTION_BITS) - 1;

// LED dome transition fractions
constexpr float LED_FADE_IN_FRACTION  = 0.15f;
constexpr float LED_FADE_OUT_FRACTION = 0.55f;
constexpr uint16_t LED_MIN_FADE_MS    = 12; 
constexpr uint16_t LED_UPDATE_PERIOD_MS = 2;   // how often the fade curve is re-evaluated

// Audio library object
AudioSynthWaveformSine sine1;
AudioOutputI2S         i2s1;
AudioConnection        patchCord1(sine1, 0, i2s1, 0);
AudioConnection        patchCord2(sine1, 0, i2s1, 1);

Bounce2::Button button = Bounce2::Button();

// Shared timing source for LED + audio pulses
enum class PulseState { IDLE, ACTIVE };

PulseState pulseState   = PulseState::IDLE;
uint16_t   pulseDurationMs = 0;   // OUTPUT_SHORT_MS or OUTPUT_LONG_MS for this pulse
elapsedMillis pulseElapsed;       

void setup() {
  button.attach(PIN_SIGNAL_IN, INPUT_PULLUP);
  button.interval(DEBOUNCE_INTERVAL_MS);
  button.setPressedState(LOW);

  analogWriteResolution(LED_PWM_RESOLUTION_BITS);
  analogWriteFrequency(PIN_LED, LED_PWM_FREQUENCY_HZ);
  analogWrite(PIN_LED, 0);

  AudioMemory(8);
  sine1.frequency(TONE_FREQ_HZ);
  sine1.amplitude(0.0f);
}

void startPulse(uint16_t durationMs) {
  pulseDurationMs = durationMs;
  pulseElapsed = 0;
  pulseState = PulseState::ACTIVE;
}

void updateInput() {
  button.update();

  if (button.released()) {
    uint32_t heldMs = button.previousDuration();
    bool isLong = (heldMs >= SHORT_LONG_THRESHOLD_MS);
    startPulse(isLong ? OUTPUT_LONG_MS : OUTPUT_SHORT_MS);
  }
}

void updateAudio() {
  if (pulseState != PulseState::ACTIVE) {
    sine1.amplitude(0.0f);
    return;
  }

  uint16_t t = pulseElapsed;

  if (t < TONE_FADE_MS) {
    float p = (float)t / (float)TONE_FADE_MS;
    sine1.amplitude(TONE_AMPLITUDE * p);
  } else if (t < (pulseDurationMs - TONE_FADE_MS)) {
    sine1.amplitude(TONE_AMPLITUDE);
  } else if (t < pulseDurationMs) {
    float p = (float)(pulseDurationMs - t) / (float)TONE_FADE_MS;
    sine1.amplitude(TONE_AMPLITUDE * p);
  } else {
    sine1.amplitude(0.0f);
  }
}

void updateLED() {
  static elapsedMillis sinceLastUpdate;
  if (sinceLastUpdate < LED_UPDATE_PERIOD_MS) return;
  sinceLastUpdate = 0;

  if (pulseState != PulseState::ACTIVE) {
    analogWrite(PIN_LED, 0);
    return;
  }

  uint16_t t = pulseElapsed;

  uint16_t fadeInMs  = max((uint16_t)(pulseDurationMs * LED_FADE_IN_FRACTION), LED_MIN_FADE_MS);
  uint16_t fadeOutMs = max((uint16_t)(pulseDurationMs * LED_FADE_OUT_FRACTION), LED_MIN_FADE_MS);

  if (fadeInMs + fadeOutMs > pulseDurationMs) {
    float scale = (float)pulseDurationMs / (float)(fadeInMs + fadeOutMs);
    fadeInMs  = (uint16_t)(fadeInMs * scale);
    fadeOutMs = (uint16_t)(fadeOutMs * scale);
  }
  uint16_t fadeOutStart = pulseDurationMs - fadeOutMs;

  float brightness; // 0.0 - 1.0

  if (t >= pulseDurationMs) {
    brightness = 0.0f;
  } else if (t < fadeInMs) {
    float p = (float)t / (float)fadeInMs;
    brightness = 1.0f - (1.0f - p) * (1.0f - p); // ease-out quad: fast rise
  } else if (t < fadeOutStart) {
    brightness = 1.0f;
  } else {
    float p = (float)(t - fadeOutStart) / (float)fadeOutMs;
    float remain = 1.0f - p;
    brightness = remain * remain * remain; // ease-in cubic: slow lingering decay
  }

  analogWrite(PIN_LED, (uint16_t)(brightness * LED_MAX_DUTY));
}

void loop() {
  updateInput();
  updateAudio();
  updateLED();

  if (pulseState == PulseState::ACTIVE && pulseElapsed >= pulseDurationMs) {
    pulseState = PulseState::IDLE;
  }
}
