// ============================================================================
// PLEASE NOTE: This is low-level testing for firmware for the telegraph boards.
// The wiring SKIPS SW_IN header signals and routes dedicated (1x each) MISC connections on the ethernet,
// directly to Pin 14 for each Teensy 4.0 board. 

// Board 1 MISC (4) routed to Pin 15 (output)
// Board 1 MISC (3) routed to Pin 14 (input)

// Board 2 MISC (4) routed to pin 14 (input)
// Board 2 MISC (3) routed to Pin 15 (output)

// This test works as of 2026-10-21
// ============================================================================

#include <Audio.h>

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
constexpr bool IS_LEADER = false;   // true on ONE board, false on the other

constexpr uint8_t PIN_SIGNAL_IN  = 14;   // token from the other board
constexpr uint8_t PIN_SIGNAL_OUT = 15;   // token to the other board
constexpr uint8_t PIN_LED        = 3;    // LED_DATA

constexpr uint16_t NUM_CYCLES      = 5;     // turns per board, then stop
constexpr uint16_t ACTIVATION_MS   = 300;   // between SHORT (150) and LONG (450)
constexpr uint16_t GAP_MS          = ACTIVATION_MS / 2;   // 150 ms
constexpr uint32_t LEADER_START_DELAY_MS = 3000;  // lets the other board boot

// Handshake
constexpr uint16_t TOKEN_PULSE_MS    = 30;   // how long the line is held HIGH
constexpr uint16_t TOKEN_DEBOUNCE_MS = 10;   // HIGH must persist this long to count

// Sound (same as example)
constexpr float TONE_FREQ_HZ = 600.0;
constexpr float TONE_AMPLITUDE = 0.5;

// LED dome (same eased shape as example)
constexpr uint8_t  LED_PWM_RESOLUTION_BITS = 12;
constexpr uint32_t LED_PWM_FREQUENCY_HZ    = 30000;
constexpr uint16_t LED_MAX_DUTY = (1 << LED_PWM_RESOLUTION_BITS) - 1;
constexpr float    LED_FADE_IN_FRACTION  = 0.15f;   // fast warm-up
constexpr float    LED_FADE_OUT_FRACTION = 0.55f;   // slow lingering cool-down
constexpr uint16_t LED_UPDATE_PERIOD_MS  = 2;

// ---------------------------------------------------------------------------
// Audio objects
// ---------------------------------------------------------------------------
AudioSynthWaveformSine sine1;
AudioOutputPT8211      dac;
AudioConnection        patchCord1(sine1, 0, dac, 0);
AudioConnection        patchCord2(sine1, 0, dac, 1);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
const char* roleName() { return IS_LEADER ? "LEADER" : "FOLLOWER"; }

// Eased brightness (0.0 - 1.0) for time t within an activation of length durMs
float ledBrightness(uint16_t t, uint16_t durMs) {
  uint16_t fadeInMs  = (uint16_t)(durMs * LED_FADE_IN_FRACTION);
  uint16_t fadeOutMs = (uint16_t)(durMs * LED_FADE_OUT_FRACTION);
  uint16_t fadeOutStart = durMs - fadeOutMs;

  if (t >= durMs) return 0.0f;
  if (t < fadeInMs) {
    float p = (float)t / (float)fadeInMs;
    return 1.0f - (1.0f - p) * (1.0f - p);        // ease-out quad: fast rise
  }
  if (t < fadeOutStart) return 1.0f;
  float p = (float)(t - fadeOutStart) / (float)fadeOutMs;
  float remain = 1.0f - p;
  return remain * remain * remain;                 // ease-in cubic: slow decay
}

// One full turn: sound + LED together for ACTIVATION_MS. Blocking.
void runActivation() {
  sine1.amplitude(TONE_AMPLITUDE);
  elapsedMillis t = 0;
  elapsedMillis sinceUpdate = 0;

  while (t < ACTIVATION_MS) {
    if (sinceUpdate >= LED_UPDATE_PERIOD_MS) {
      sinceUpdate = 0;
      analogWrite(PIN_LED, (uint16_t)(ledBrightness(t, ACTIVATION_MS) * LED_MAX_DUTY));
    }
  }

  sine1.amplitude(0.0);
  analogWrite(PIN_LED, 0);
}

// Send token: pulse Pin 15 HIGH briefly, then return it to idle LOW.
void sendToken() {
  digitalWrite(PIN_SIGNAL_OUT, HIGH);
  delay(TOKEN_PULSE_MS);
  digitalWrite(PIN_SIGNAL_OUT, LOW);
}

// Block until the other board's token arrives on Pin 14 (HIGH, then released).
void waitForToken() {
  while (true) {
    if (digitalRead(PIN_SIGNAL_IN) == HIGH) {
      elapsedMillis high = 0;
      bool stable = true;
      while (high < TOKEN_DEBOUNCE_MS) {
        if (digitalRead(PIN_SIGNAL_IN) == LOW) { stable = false; break; }
      }
      if (stable) {
        while (digitalRead(PIN_SIGNAL_IN) == HIGH) { }   // wait for release
        return;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.print("\n=== Telegraph Board Test: ");
  Serial.print(roleName());
  Serial.println(" ===");

  pinMode(PIN_SIGNAL_IN, INPUT);        // board's 220 ohm to GND holds it LOW
  pinMode(PIN_SIGNAL_OUT, OUTPUT);
  digitalWrite(PIN_SIGNAL_OUT, LOW);    // idle level

  analogWriteResolution(LED_PWM_RESOLUTION_BITS);
  analogWriteFrequency(PIN_LED, LED_PWM_FREQUENCY_HZ);
  analogWrite(PIN_LED, 0);

  AudioMemory(12);
  sine1.frequency(TONE_FREQ_HZ);
  sine1.amplitude(0.0);

  Serial.print("Cycles: "); Serial.print(NUM_CYCLES);
  Serial.print("  Activation: "); Serial.print(ACTIVATION_MS);
  Serial.print(" ms  Gap: "); Serial.print(GAP_MS);
  Serial.println(" ms");

  delay(50);
  if (digitalRead(PIN_SIGNAL_IN) == HIGH) {
    Serial.println("WARNING: Pin 14 reads HIGH at idle (expected LOW). Check wiring.");
  }

  if (IS_LEADER) {
    Serial.println("Leader waiting for follower to boot...");
    delay(LEADER_START_DELAY_MS);
  }
}

void loop() {
  static bool done = false;
  if (done) return;

  for (uint16_t cycle = 1; cycle <= NUM_CYCLES; cycle++) {
    bool isLastTurn = (cycle == NUM_CYCLES);

    if (!IS_LEADER) {
      Serial.print("Follower: waiting for token (cycle ");
      Serial.print(cycle); Serial.println(")");
      waitForToken();
    }

    Serial.print(roleName());
    Serial.print(": ACTIVATE (cycle ");
    Serial.print(cycle); Serial.print("/"); Serial.print(NUM_CYCLES);
    Serial.println(")");
    runActivation();

    // The follower's last turn ends the test; no token needed after it.
    if (!IS_LEADER && isLastTurn) break;

    delay(GAP_MS);

    Serial.print(roleName());
    Serial.println(": sending token");
    sendToken();

    if (IS_LEADER && !isLastTurn) {
      // Leader waits for the follower's token before its next turn
      Serial.println("Leader: waiting for token");
      waitForToken();
    }
  }

  Serial.print(roleName());
  Serial.println(": test complete.");
  done = true;
}