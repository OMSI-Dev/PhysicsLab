/*
 * Detecting Dark Matter - Audio Manager
 * Teensy 4.0 + SparkFun WAV Trigger
 *
 * Each of the two switches plays one sound when pressed.
 * After 25 rounds in which both switches were pressed, the next round
 * plays a bonus sound for each switch, then the count starts over.
 *
 * SD card files:
 *   001_*.wav  switch 1        003_*.wav  switch 1 bonus
 *   002_*.wav  switch 2        004_*.wav  switch 2 bonus
 */

#include <Arduino.h>

// Pins (the WAV Trigger connects to Serial1: pin 0 = RX1, pin 1 = TX1)
constexpr uint8_t NUM_SWITCHES = 2;
constexpr uint8_t SWITCH_PINS[NUM_SWITCHES] = {14, 15};
constexpr uint8_t LED_PIN = LED_BUILTIN;

// true: switch pulls the pin LOW when pressed
constexpr bool SWITCH_ACTIVE_LOW = true;

// Timing (milliseconds)
constexpr uint32_t DEBOUNCE_MS = 20;
constexpr uint32_t RESET_DELAY_MS = 1000;  // wait after all switches are released
constexpr uint32_t LED_FLASH_MS = 100;

// WAV Trigger
constexpr uint32_t WAVTRIGGER_BAUD = 57600;
constexpr uint16_t NORMAL_TRACKS[NUM_SWITCHES] = {1, 2};
constexpr uint16_t BONUS_TRACKS[NUM_SWITCHES] = {3, 4};
constexpr uint8_t ROUNDS_BEFORE_BONUS = 25;

struct Switch {
  bool pressed;           // debounced state
  bool lastReading;       // most recent raw reading
  uint32_t lastChangeMs;  // when the raw reading last changed
  bool played;            // already played its sound this round
};

Switch switches[NUM_SWITCHES];
uint32_t releasedSinceMs = 0;
uint32_t ledOffAtMs = 0;
uint8_t bothPressedRounds = 0;

bool isPressed(uint8_t pin) {
  return digitalRead(pin) == (SWITCH_ACTIVE_LOW ? LOW : HIGH);
}

// Starts a track without stopping others, so both sounds can overlap.
void playTrack(uint16_t track) {
  const uint8_t msg[] = {0xF0, 0xAA, 0x08, 0x03, 0x01,
                         static_cast<uint8_t>(track & 0xFF),
                         static_cast<uint8_t>(track >> 8), 0x55};
  Serial1.write(msg, sizeof(msg));
}

void onPressed(uint8_t i) {
  if (switches[i].played) {
    return;
  }
  switches[i].played = true;

  const bool bonus = bothPressedRounds >= ROUNDS_BEFORE_BONUS;
  const uint16_t track = bonus ? BONUS_TRACKS[i] : NORMAL_TRACKS[i];
  playTrack(track);

  digitalWrite(LED_PIN, HIGH);
  ledOffAtMs = millis() + LED_FLASH_MS;

  Serial.printf("Switch %u -> track %u\n", i + 1, track);
}

// Ends the round and updates the bonus counter.
void endRound() {
  bool anyPlayed = false;
  bool allPlayed = true;
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    anyPlayed |= switches[i].played;
    allPlayed &= switches[i].played;
    switches[i].played = false;
  }
  if (!anyPlayed) {
    return;
  }

  if (bothPressedRounds >= ROUNDS_BEFORE_BONUS) {
    bothPressedRounds = 0;
  } else if (allPlayed) {
    bothPressedRounds++;
  }
  Serial.printf("Round over (%u/%u)\n", bothPressedRounds, ROUNDS_BEFORE_BONUS);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(WAVTRIGGER_BAUD);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    pinMode(SWITCH_PINS[i], SWITCH_ACTIVE_LOW ? INPUT_PULLUP : INPUT);
    const bool state = isPressed(SWITCH_PINS[i]);
    // A switch held down at power-up stays silent until released.
    switches[i] = {state, state, millis(), state};
  }
  releasedSinceMs = millis();

  Serial.println("Audio manager ready");
}

void loop() {
  const uint32_t now = millis();

  // Read and debounce each switch.
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    Switch &s = switches[i];
    const bool reading = isPressed(SWITCH_PINS[i]);

    if (reading != s.lastReading) {
      s.lastReading = reading;
      s.lastChangeMs = now;
    }

    if (reading != s.pressed && now - s.lastChangeMs >= DEBOUNCE_MS) {
      s.pressed = reading;
      if (s.pressed) {
        onPressed(i);
      }
    }
  }

  // Once all switches have been released for a while, start a new round.
  bool anyPressed = false;
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    anyPressed |= switches[i].pressed;
  }

  if (anyPressed) {
    releasedSinceMs = now;
  } else if (now - releasedSinceMs >= RESET_DELAY_MS) {
    endRound();
  }

  if (ledOffAtMs != 0 && static_cast<int32_t>(now - ledOffAtMs) >= 0) {
    digitalWrite(LED_PIN, LOW);
    ledOffAtMs = 0;
  }
}
