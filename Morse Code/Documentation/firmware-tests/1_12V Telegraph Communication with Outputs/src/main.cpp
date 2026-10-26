// ============================================================================
// Telegraph Receiver Test
//
// Both boards run IDENTICAL firmware. Pressing the telegraph key on one station
// triggers the sound + LED on the OPPOSING station.
//
//   - Pin 14 receives the opposing station's key signal (via SW_IN -> 560 ohm
//     -> Pin 14, with 220 ohm to GND). REQUIRES THE SW_OUT HOTWIRE: SW_OUT is
//     taken from Q6's drain (not the key/gate net), so a press actively drives
//     12V down the line. At Pin 14 the signal is ACTIVE-HIGH: idle 0V (LOW,
//     held by the 220 ohm), pressed ~3.38V (HIGH).
//   - Each received press is classified SHORT or LONG by SHORT_LONG_THRESHOLD_MS.
//   - The classification triggers a FIXED-duration output (not a copy of the
//     actual press length):
//       * A sine tone on the PT8211 (Teensy Audio Library / I2S)
//       * A PWM glow on the LED dome (Pin 3): a thermal filament model gives a
//         soft warm-up and a lingering cool-down, like an incandescent bulb.
//         The glow keeps fading briefly after the pulse itself has ended.
//
// Boot guard: the input stays DISARMED until Pin 14 has read LOW (idle) for
// ARM_AFTER_IDLE_MS, so a key held during boot or a stuck-high line can't
// fire outputs. Note: with the active-high hotwire, a disconnected cable
// reads the same as idle, so a missing cable can no longer be detected.
//
// Hardware pin map:
//   Pin 14 -> incoming SW_DATA (active-high, Bounce2 debounced)
//   Pin 3  -> LED_DATA (PWM to onboard N-channel MOSFET, Q5)
//   Pin 21 -> PT8211 BCK
//   Pin 20 -> PT8211 WS
//   Pin 7  -> PT8211 DIN
//   Pin 15 -> NOT USED in this test. Keep it disconnected from the MISC lines.
// ============================================================================

#include <Bounce2.h>
#include <Audio.h>

constexpr uint8_t PIN_SIGNAL_IN = 14;   // signal from opposing station
constexpr uint8_t PIN_LED       = 3;    // LED_DATA

constexpr uint16_t DEBOUNCE_INTERVAL_MS = 5;
constexpr uint16_t ARM_AFTER_IDLE_MS    = 100;   // idle HIGH time before inputs count
constexpr uint16_t NOT_CONNECTED_WARN_MS = 2000;

// Short/long classification and fixed outputs
constexpr uint16_t SHORT_LONG_THRESHOLD_MS = 200;
constexpr uint16_t OUTPUT_SHORT_MS = 150;
constexpr uint16_t OUTPUT_LONG_MS  = 450;

// Sound
constexpr float TONE_FREQ_HZ   = 600.0;
constexpr float TONE_AMPLITUDE = 0.5;

// LED dome
// If the fade looks like a plain on/off, try a lower frequency (e.g. 1000-2000).
constexpr uint8_t  LED_PWM_RESOLUTION_BITS = 12;
constexpr uint32_t LED_PWM_FREQUENCY_HZ    = 2000;
constexpr uint16_t LED_MAX_DUTY = (1 << LED_PWM_RESOLUTION_BITS) - 1;

// Incandescent filament model: a "temperature" state heats/cools exponentially
// toward on/off, and light output rises steeply with temperature, so the glow
// swells up softly and lingers as a dim ember after the key is released.
constexpr float    LED_HEAT_TAU_MS      = 75.0f;    // warm-up time constant (larger = slower swell)
constexpr float    LED_COOL_TAU_MS      = 260.0f;   // cool-down time constant (larger = longer afterglow)
constexpr float    LED_OUTPUT_EXPONENT  = 2.0f;     // light ~ temp^N (higher = dimmer, redder tail)
constexpr uint16_t LED_UPDATE_PERIOD_MS = 2;

// PT8211 DAC
AudioSynthWaveformSine sine1;
AudioOutputPT8211      dac;
AudioConnection        patchCord1(sine1, 0, dac, 0);
AudioConnection        patchCord2(sine1, 0, dac, 1);

Bounce2::Button button = Bounce2::Button();

enum class PulseState { IDLE, ACTIVE };
enum class ButtonHoldState { IDLE, PRESSING, LONG_TRIGGERED };

PulseState    pulseState = PulseState::IDLE;
uint16_t      pulseDurationMs = 0;
elapsedMillis pulseElapsed;

ButtonHoldState buttonHoldState = ButtonHoldState::IDLE;
elapsedMillis   pressStart = 0;

bool armed = false;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n=== Telegraph Receiver Test ===");

  button.attach(PIN_SIGNAL_IN, INPUT);   // external 220 ohm to GND defines idle
  button.interval(DEBOUNCE_INTERVAL_MS);
  button.setPressedState(HIGH);

  analogWriteResolution(LED_PWM_RESOLUTION_BITS);
  analogWriteFrequency(PIN_LED, LED_PWM_FREQUENCY_HZ);
  analogWrite(PIN_LED, 0);

  AudioMemory(12);
  sine1.frequency(TONE_FREQ_HZ);
  sine1.amplitude(0.0);

  Serial.println("Setup complete. Waiting for Pin 14 to idle LOW...");
}

void startPulse(uint16_t durationMs) {
  pulseDurationMs = durationMs;
  pulseElapsed = 0;
  pulseState = PulseState::ACTIVE;
}

void updateInput() {
  button.update();

  // Stay disarmed until the line has idled LOW (far key released, no fault)
  if (!armed) {
    static elapsedMillis idleFor = 0;
    static bool warned = false;

    if (button.isPressed()) {
      idleFor = 0;
      if (!warned && millis() > NOT_CONNECTED_WARN_MS) {
        Serial.println("WARNING: Pin 14 is HIGH. Far key held, or line fault?");
        warned = true;
      }
    } else if (idleFor >= ARM_AFTER_IDLE_MS) {
      armed = true;
      Serial.println("Input armed (Pin 14 idles LOW).");
    }
    return;
  }

  if (buttonHoldState == ButtonHoldState::IDLE) {
    if (button.isPressed()) {
      buttonHoldState = ButtonHoldState::PRESSING;
      pressStart = 0;
    }
  }
  else if (buttonHoldState == ButtonHoldState::PRESSING) {
    if (!button.isPressed()) {
      // Released before hitting the long threshold
      uint16_t heldMs = pressStart;
      if (heldMs < SHORT_LONG_THRESHOLD_MS) {
        Serial.print("Press detected: ");
        Serial.print(heldMs);
        Serial.print(" ms -> SHORT (output ");
        Serial.print(OUTPUT_SHORT_MS);
        Serial.println(" ms)");
        startPulse(OUTPUT_SHORT_MS);
      }
      buttonHoldState = ButtonHoldState::IDLE;
    }
    else if (pressStart >= SHORT_LONG_THRESHOLD_MS) {
      // Crossed the threshold: fire immediately
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
      buttonHoldState = ButtonHoldState::IDLE;
    }
  }
}

void updateAudio() {
  static PulseState lastPulseState = PulseState::IDLE;

  if (pulseState != lastPulseState) {
    sine1.amplitude(pulseState == PulseState::ACTIVE ? TONE_AMPLITUDE : 0.0);
    lastPulseState = pulseState;
  }
}

void updateLED() {
  static elapsedMillis sinceLastUpdate;
  static float filament = 0.0f;   // normalized filament temperature, 0.0 - 1.0

  if (sinceLastUpdate < LED_UPDATE_PERIOD_MS) return;
  float dtMs = (float)sinceLastUpdate;
  sinceLastUpdate = 0;

  bool  heating = (pulseState == PulseState::ACTIVE);
  float target  = heating ? 1.0f : 0.0f;
  float tau     = heating ? LED_HEAT_TAU_MS : LED_COOL_TAU_MS;

  filament += (target - filament) * (1.0f - expf(-dtMs / tau));
  if (filament < 0.002f && !heating) filament = 0.0f;

  float brightness = powf(filament, LED_OUTPUT_EXPONENT);
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