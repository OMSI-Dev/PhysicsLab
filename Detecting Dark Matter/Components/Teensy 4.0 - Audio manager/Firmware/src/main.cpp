#include <Arduino.h>

// ---- Pins ------------------------------------------------------------------
constexpr uint8_t PIN_SWITCH_1 = 14;  // X3
constexpr uint8_t PIN_SWITCH_2 = 15;  // X2
constexpr uint8_t PIN_LED = LED_BUILTIN;
// WAV Trigger on Serial1: RX1 = pin 0, TX1 = pin 1

// ---- Switch polarity (flip if bench test shows the opposite) ---------------
constexpr bool SWITCH_ACTIVE_LOW = true;  // true: INPUT_PULLUP, LOW = triggered

// ---- Timing ----------------------------------------------------------------
constexpr uint32_t DEBOUNCE_MS = 20;
constexpr uint32_t RESET_DELAY_MS = 1000;  // after all switches released
constexpr uint32_t LED_FLASH_MS = 100;

// ---- WAV Trigger -----------------------------------------------------------
constexpr uint32_t WAVTRIGGER_BAUD = 57600;
constexpr uint16_t TRACK_FIRST_PLACE = 1;   // SD card file 001_*.wav
constexpr uint16_t TRACK_SECOND_PLACE = 2;  // SD card file 002_*.wav

constexpr uint8_t NUM_SWITCHES = 2;
constexpr uint8_t SWITCH_PINS[NUM_SWITCHES] = {PIN_SWITCH_1, PIN_SWITCH_2};

struct Switch {
  bool triggered;         // debounced state
  bool lastRaw;           // last raw reading
  uint32_t lastChangeMs;  // when raw reading last changed
  bool lockedOut;         // has already played a sound this round
};

Switch switches[NUM_SWITCHES];
int8_t winner = -1;               // index of first switch triggered this round, -1 if none
uint32_t allReleasedSinceMs = 0;  // when every switch became released
uint32_t ledOffAtMs = 0;

bool readTriggered(uint8_t pin) {
  return digitalRead(pin) == (SWITCH_ACTIVE_LOW ? LOW : HIGH);
}

// Poly play so the second-place sound can overlap the first-place sound.
void wavTriggerPlayPoly(uint16_t track) {
  const uint8_t msg[] = {0xF0, 0xAA, 0x08, 0x03, 0x01,
                         static_cast<uint8_t>(track & 0xFF),
                         static_cast<uint8_t>(track >> 8), 0x55};
  Serial1.write(msg, sizeof(msg));
}

void onTriggered(uint8_t i) {
  if (switches[i].lockedOut) {
    return;
  }
  switches[i].lockedOut = true;

  uint16_t track;
  if (winner < 0) {
    winner = i;
    track = TRACK_FIRST_PLACE;
  } else {
    track = TRACK_SECOND_PLACE;
  }

  wavTriggerPlayPoly(track);
  digitalWrite(PIN_LED, HIGH);
  ledOffAtMs = millis() + LED_FLASH_MS;

  Serial.printf("Switch %u triggered -> track %u (%s)\n", i + 1, track,
                track == TRACK_FIRST_PLACE ? "first place" : "second place");
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(WAVTRIGGER_BAUD);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    pinMode(SWITCH_PINS[i], SWITCH_ACTIVE_LOW ? INPUT_PULLUP : INPUT);
    const bool state = readTriggered(SWITCH_PINS[i]);
    switches[i] = {state, state, millis(), state};  // a switch held at boot stays locked out
  }
  allReleasedSinceMs = millis();

  Serial.println("Detecting Dark Matter audio manager ready");
}

void loop() {
  const uint32_t now = millis();

  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    Switch &s = switches[i];
    const bool raw = readTriggered(SWITCH_PINS[i]);

    if (raw != s.lastRaw) {
      s.lastRaw = raw;
      s.lastChangeMs = now;
    }

    if (raw != s.triggered && now - s.lastChangeMs >= DEBOUNCE_MS) {
      s.triggered = raw;
      if (s.triggered) {
        onTriggered(i);
      }
    }
  }

  // Round resets once every switch has been released for RESET_DELAY_MS.
  bool anyTriggered = false;
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    anyTriggered |= switches[i].triggered;
  }

  if (anyTriggered) {
    allReleasedSinceMs = now;
  } else if (now - allReleasedSinceMs >= RESET_DELAY_MS) {
    bool needsReset = (winner >= 0);
    for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
      needsReset |= switches[i].lockedOut;
      switches[i].lockedOut = false;
    }
    winner = -1;
    if (needsReset) {
      Serial.println("Lock-out reset");
    }
  }

  if (ledOffAtMs != 0 && static_cast<int32_t>(now - ledOffAtMs) >= 0) {
    digitalWrite(PIN_LED, LOW);
    ledOffAtMs = 0;
  }
}
