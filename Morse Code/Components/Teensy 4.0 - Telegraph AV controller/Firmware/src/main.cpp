// Telegraph AV Controller
//
// Plays a tone and lights an LED dome when the telegraph key at the other
// station is pressed. Both stations run the same firmware.
//
// Input
//   Pin 14 reads the far key through a 560 ohm / 220 ohm divider.
//   Idle is LOW, pressed is HIGH (about 3.4 V).
//   The v1 board needs SW_OUT wired from the drain of Q6.
//
// Output
//   A press shorter than 200 ms is SHORT, anything longer is LONG.
//   SHORT plays a 150 ms pulse, LONG plays a 450 ms pulse.
//   Each pulse plays a 600 Hz tone and glows the LED like an incandescent bulb.
//
// Startup
//   Input stays disabled until Pin 14 has been LOW for 100 ms. A key held
//   during power-up, or a stuck line, is ignored.
//
// Pins
//   14  Signal in
//   3   LED dome (PWM)
//   21  PT8211 BCK
//   20  PT8211 WS
//   7   PT8211 DIN

#include <Bounce2.h>
#include <Audio.h>

constexpr uint8_t PIN_SIGNAL_IN = 14;
constexpr uint8_t PIN_LED       = 3;

constexpr uint16_t DEBOUNCE_INTERVAL_MS  = 5;
constexpr uint16_t ARM_AFTER_IDLE_MS     = 100;   // LOW time required before input is enabled
constexpr uint16_t NOT_CONNECTED_WARN_MS = 2000;

constexpr uint16_t SHORT_LONG_THRESHOLD_MS = 200;
constexpr uint16_t OUTPUT_SHORT_MS = 150;
constexpr uint16_t OUTPUT_LONG_MS  = 450;

constexpr float TONE_FREQ_HZ   = 600.0;
constexpr float TONE_AMPLITUDE = 0.5;

constexpr uint8_t  LED_PWM_RESOLUTION_BITS = 12;
constexpr uint32_t LED_PWM_FREQUENCY_HZ    = 2000;
constexpr uint16_t LED_MAX_DUTY = (1 << LED_PWM_RESOLUTION_BITS) - 1;

// LED glow model: a filament "temperature" heats and cools exponentially,
// and brightness is temperature raised to LED_OUTPUT_EXPONENT.
constexpr float    LED_HEAT_TAU_MS      = 75.0f;    // warm-up time constant
constexpr float    LED_COOL_TAU_MS      = 260.0f;   // cool-down time constant
constexpr float    LED_OUTPUT_EXPONENT  = 2.0f;
constexpr uint16_t LED_UPDATE_PERIOD_MS = 2;

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
  Serial.println("\nTelegraph AV Controller");

  button.attach(PIN_SIGNAL_IN, INPUT);   // idle level set by the external 220 ohm to GND
  button.interval(DEBOUNCE_INTERVAL_MS);
  button.setPressedState(HIGH);

  analogWriteResolution(LED_PWM_RESOLUTION_BITS);
  analogWriteFrequency(PIN_LED, LED_PWM_FREQUENCY_HZ);
  analogWrite(PIN_LED, 0);

  AudioMemory(12);
  sine1.frequency(TONE_FREQ_HZ);
  sine1.amplitude(0.0);

  Serial.println("Waiting for Pin 14 to idle LOW");
}

void startPulse(uint16_t durationMs) {
  pulseDurationMs = durationMs;
  pulseElapsed = 0;
  pulseState = PulseState::ACTIVE;
}

void updateInput() {
  button.update();

  if (!armed) {
    static elapsedMillis idleFor = 0;
    static bool warned = false;

    if (button.isPressed()) {
      idleFor = 0;
      if (!warned && millis() > NOT_CONNECTED_WARN_MS) {
        Serial.println("Warning: Pin 14 is HIGH. Key held or line fault?");
        warned = true;
      }
    } else if (idleFor >= ARM_AFTER_IDLE_MS) {
      armed = true;
      Serial.println("Input armed");
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
      uint16_t heldMs = pressStart;
      if (heldMs < SHORT_LONG_THRESHOLD_MS) {
        Serial.print("SHORT press, ");
        Serial.print(heldMs);
        Serial.println(" ms");
        startPulse(OUTPUT_SHORT_MS);
      }
      buttonHoldState = ButtonHoldState::IDLE;
    }
    else if (pressStart >= SHORT_LONG_THRESHOLD_MS) {
      // Fire as soon as the press crosses the threshold
      buttonHoldState = ButtonHoldState::LONG_TRIGGERED;
      Serial.println("LONG press");
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
  static float filament = 0.0f;   // 0.0 (cold) to 1.0 (hot)

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
