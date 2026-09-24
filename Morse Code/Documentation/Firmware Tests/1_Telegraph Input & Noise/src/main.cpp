// ============================================================================
// Notes:
//   - Pin 2 receives the incoming SW_data signal from the OPPOSING station's
//     telegraph key (active-low, pulled up by onboard R6).
//   - On each received press, the press duration is classified as SHORT or
//     LONG using SHORT_LONG_THRESHOLD_MS.
//   - The classification triggers a FIXED-duration output, either 'SHORT' or
//     'LONG' (not a translation of the actual input duration):
//       * A sine tone on the PT8211 (via Teensy Audio Library / I2S)
//       * A PWM fade on the LED dome (Pin 3), shaped to emulate an
//         incandescent bulb: fast warm-up, slower lingering cool-down.
//
// Hardware pin map:
//   Pin 14  -> incoming SW_data input (active-low, Bounce2 debounced)
//   Pin 3  -> LED dome PWM (drives onboard N-channel MOSFET, Q5)
//   Pin 21 -> PT8211 BCK
//   Pin 20 -> PT8211 WS
//   Pin 7  -> PT8211 DIN
// ============================================================================

#include <Bounce2.h>
#include <Audio.h>

constexpr uint8_t PIN_SIGNAL_IN = 14;   // signal from opposing station
constexpr uint8_t PIN_LED       = 3;   // LED dome PWM output 

constexpr uint16_t DEBOUNCE_INTERVAL_MS = 5;    // Bounce2 debounce interval


// Constraints for reading short/long inputs
constexpr uint16_t SHORT_LONG_THRESHOLD_MS = 200;
constexpr uint16_t OUTPUT_SHORT_MS = 150;   
constexpr uint16_t OUTPUT_LONG_MS  = 450;   

// Sound
constexpr float    TONE_FREQ_HZ  = 600.0;   

// LED dome
constexpr uint8_t  LED_PWM_RESOLUTION_BITS = 12;                      
constexpr uint32_t LED_PWM_FREQUENCY_HZ    = 30000;                    
constexpr uint16_t LED_MAX_DUTY = (1 << LED_PWM_RESOLUTION_BITS) - 1;

// LED dome transition fractions
constexpr float LED_FADE_IN_FRACTION  = 0.15f;
constexpr float LED_FADE_OUT_FRACTION = 0.55f;
constexpr uint16_t LED_MIN_FADE_MS    = 12; 
constexpr uint16_t LED_UPDATE_PERIOD_MS = 2;   

// PT8211 DAC object
AudioSynthWaveformSine sine1;
AudioOutputPT8211      dac;
AudioConnection        patchCord1(sine1, 0, dac, 0);
AudioConnection        patchCord2(sine1, 0, dac, 1);

Bounce2::Button button = Bounce2::Button();

// State machines
enum class PulseState { IDLE, ACTIVE };
enum class ButtonHoldState { IDLE, PRESSING, LONG_TRIGGERED };

// Pulse state tracking (audio + LED)
PulseState pulseState   = PulseState::IDLE;
uint16_t   pulseDurationMs = 0;
elapsedMillis pulseElapsed;

// Button hold tracking
ButtonHoldState buttonHoldState = ButtonHoldState::IDLE;
elapsedMillis pressStart = 0;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n=== Telegraph Receiver Initialized ===");
  
  button.attach(PIN_SIGNAL_IN, INPUT_PULLUP);
  button.interval(DEBOUNCE_INTERVAL_MS);
  button.setPressedState(LOW);
  
  analogWriteResolution(LED_PWM_RESOLUTION_BITS);
  analogWriteFrequency(PIN_LED, LED_PWM_FREQUENCY_HZ);
  analogWrite(PIN_LED, 0);
 
  AudioMemory(12);
  sine1.frequency(TONE_FREQ_HZ);
  sine1.amplitude(0.0); // do not play at setup
}

void startPulse(uint16_t durationMs) {
  pulseDurationMs = durationMs;
  pulseElapsed = 0;
  pulseState = PulseState::ACTIVE;
}



void updateInput() {
  button.update();

  if (buttonHoldState == ButtonHoldState::IDLE) {
    // Looking for a press
    if (button.isPressed()) {
      buttonHoldState = ButtonHoldState::PRESSING;
      pressStart = 0;
    }
  } 
  else if (buttonHoldState == ButtonHoldState::PRESSING) {
    if (!button.isPressed()) {
      // Released before hitting long threshold
      uint16_t heldMs = pressStart;
      if (heldMs < SHORT_LONG_THRESHOLD_MS) {
        Serial.print("Press detected: ");
        Serial.print(heldMs);
        Serial.print(" ms → SHORT (output ");
        Serial.print(OUTPUT_SHORT_MS);
        Serial.println(" ms)");
        startPulse(OUTPUT_SHORT_MS);
      }
      buttonHoldState = ButtonHoldState::IDLE;
    } 
    else if (pressStart >= SHORT_LONG_THRESHOLD_MS) {
      // Crossed the threshold - fire immediately
      buttonHoldState = ButtonHoldState::LONG_TRIGGERED;
      Serial.print("Press detected: LONG at ");
      Serial.print(pressStart);
      Serial.print(" ms (output ");
      Serial.print(OUTPUT_LONG_MS);
      Serial.println(" ms)");
      startPulse(OUTPUT_LONG_MS);
    }
  }
  else if (buttonHoldState == ButtonHoldState::LONG_TRIGGERED) {
    if (!button.isPressed()) {
      // Release after long trigger
      buttonHoldState = ButtonHoldState::IDLE;
    }
  }
}

void updateAudio() {
  static PulseState lastPulseState = PulseState::IDLE;
  
  // Only update amplitude when state actually changes
  if (pulseState != lastPulseState) {
    if (pulseState == PulseState::ACTIVE) {
      sine1.amplitude(0.5);  // Turn on
    } else {
      sine1.amplitude(0.0);  // Turn off
    }
    lastPulseState = pulseState;
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

  // Optional: Monitor button state (uncomment to debug)
  // static elapsedMillis lastStatePrint;
  // if (lastStatePrint > 500) {
  //   lastStatePrint = 0;
  //   Serial.print("Button pressed: ");
  //   Serial.println(button.isPressed());
  // }
}