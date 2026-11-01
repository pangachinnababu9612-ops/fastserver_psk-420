
// ═══════════════════════════════════════════════════════════════════════════
//  PIN DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════════
#define NUM_SENSORS  8

// Sensor pins: A0 = S1 (leftmost), A7 = S8 (rightmost)
const uint8_t SENSOR_PINS[NUM_SENSORS] = { A0, A1, A2, A3, A4, A5, A6, A7 };

// TB6612FNG — PWMA on Timer0 (D5), PWMB on Timer1 (D9)
#define PWMA   5    // Left  motor speed  (PWM)
#define AIN1   4    // Left  motor direction 1
#define AIN2   7    // Left  motor direction 2
#define PWMB   6    // Right motor speed  (PWM)
#define BIN1   8    // Right motor direction 1
#define BIN2   9    // Right motor direction 2
#define STBY   10    // TB6612 standby: HIGH = enabled, LOW = sleep

// Buttons — wired active-LOW (press → pin to GND; INPUT_PULLUP inside)
#define BTN_START   2   // D2: Start / Stop robot
#define BTN_CALIB   3   // D3: Run calibration (only works when stopped)

// Status LED
#define LED_PIN    13

// ═══════════════════════════════════════════════════════════════════════════
//  SPEED PARAMETERS  (0–255 PWM)
// ═══════════════════════════════════════════════════════════════════════════
const int BASE_SPEED      = 100;  // Straight-line cruise speed
const int GHOST_SPEED     =  50;  // Speed used during ghost-mode recovery
const int MAX_SPEED       = 200;  // Absolute motor ceiling
const int MIN_TURN_SPEED  =  50;  // Iner motor spee3d during hard pivot

// ═══════════════════════════════════════════════════════════════════════════
//  PID GAINS  — tune on your actual track
// ═══════════════════════════════════════════════════════════════════════════
float Kp = 0.042f;
float Ki = 0.000;
float Kd = 0.45f;

// ═══════════════════════════════════════════════════════════════════════════
//  SENSOR CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

// Normalised value above this → sensor is on the black line (0–1000 scale)
const int LINE_THRESHOLD = 500;

// FIXED: Set to false so HIGH raw ADC value = on black line
#define SENSOR_INVERTED false

// Calibration sweep duration (ms)
const unsigned long CALIB_DURATION = 4000UL;

// ═══════════════════════════════════════════════════════════════════════════
//  SENSOR POSITION WEIGHTS
// ═══════════════════════════════════════════════════════════════════════════
const int WEIGHTS[NUM_SENSORS] = {
  -3500, -2500, -1500, -500,   // S1–S4: left of centre
    500,  1500,  2500,  3500   // S5–S8: right of centre
};

// ═══════════════════════════════════════════════════════════════════════════
//  LOST-LINE TIMING
// ═══════════════════════════════════════════════════════════════════════════
const unsigned long LOST_TIMEOUT_MS  =  300UL;   // Grace period before "lost"
const unsigned long LOST_RECOVERY_MS = 2000UL;   // Ghost-mode max before hard stop

// ═══════════════════════════════════════════════════════════════════════════
//  CALIBRATION DATA
// ═══════════════════════════════════════════════════════════════════════════
int  minVal[NUM_SENSORS];        // Minimum raw ADC seen per sensor
int  maxVal[NUM_SENSORS];        // Maximum raw ADC seen per sensor
int  rawVal[NUM_SENSORS];        // Current raw ADC readings
int  calVal[NUM_SENSORS];        // Normalised readings (0–1000)
bool calibrated = false;

// ═══════════════════════════════════════════════════════════════════════════
//  GHOST-MODE SNAPSHOT
// ═══════════════════════════════════════════════════════════════════════════
int  lastValidCalVal[NUM_SENSORS]; // Last sensor state that saw the line
long lastValidPosition = 0L;       // Position computed from that snapshot

// ═══════════════════════════════════════════════════════════════════════════
//  PID STATE
// ═══════════════════════════════════════════════════════════════════════════
float lastError   = 0.0f;
float integral    = 0.0f;
long  lastPosition = 0L;    // Last valid position (for fallback in lost-line)
int   lastDir      = 0;     // –1 = left,  0 = centre,  +1 = right

// ═══════════════════════════════════════════════════════════════════════════
//  LOST-LINE STATE
// ═══════════════════════════════════════════════════════════════════════════
bool          lineLost      = false;
unsigned long lastLineTime  = 0;   // millis() when line was last seen
unsigned long lostStartTime = 0;   // millis() when lineLost first became true

// ═══════════════════════════════════════════════════════════════════════════
//  ROBOT / UI STATE
// ═══════════════════════════════════════════════════════════════════════════
bool robotRunning = false;

// Edge-detect debounce: store previous button states
bool startWasLow  = false;
bool calibWasLow  = false;
unsigned long lastBtnTime = 0;
const unsigned long DEBOUNCE_MS = 50UL;   // Hardware debounce window (ms)

// Non-blocking LED blink
unsigned long prevLedMillis = 0;
bool          ledState      = false;

// ═══════════════════════════════════════════════════════════════════════════
//  FUNCTION PROTOTYPES
// ═══════════════════════════════════════════════════════════════════════════
void readSensors();
void calibrateSensors();
void getCalibratedValues();
long calculatePosition();
int  countOnLine();
bool sharpLeftDetected();
bool sharpRightDetected();
void setMotors(int leftSpeed, int rightSpeed);
void stopMotors();
void handleButtons();
void updateLED();
void lostLineHandler();
void blinkLED(uint8_t times, uint16_t ms);

// ═══════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════
void setup() {
  // Motor driver output pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, LOW);   // TB6612 disabled until robot is started

  // Buttons — active LOW; no external resistor needed
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_CALIB, INPUT_PULLUP);

  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialise calibration arrays
  for (int i = 0; i < NUM_SENSORS; i++) {
    minVal[i]          = 1023;
    maxVal[i]          = 0;
    lastValidCalVal[i] = 0;
  }

  Serial.begin(9600);
  Serial.println(F("========================================="));
  Serial.println(F("  Line Follower Robot v3  —  Ready"));
  Serial.println(F("  BTN1 (D2) = Start / Stop"));
  Serial.println(F("  BTN2 (D3) = Calibrate  (when stopped)"));
  Serial.println(F("========================================="));
}

// ═══════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════
void loop() {

 

 
  // ── Step 1: Read sensors ─────────────────────────────────────────────────
  readSensors();

 
    getCalibratedValues();
  

  // ── Step 2: Compute weighted position ────────────────────────────────────
  long position = calculatePosition();   // –3500..+3500 or –9999 (no line)
  int  onLine   = countOnLine();

  // ── Step 3: Lost-line routing ─────────────────────────────────────────────
  if (position == -9999L) {

    if (!lineLost) {
      if (millis() - lastLineTime <= LOST_TIMEOUT_MS) return;

      lineLost     = true;
      lostStartTime = millis();
      Serial.println(F(">> Line LOST — recovering..."));
    }

    lostLineHandler();
    return;   // Skip normal PID this cycle
  }

  // ── Step 4: Line visible — clear lost-line state ─────────────────────────
  if (lineLost) {
    lineLost  = false;
    integral  = 0.0f;   // Flush integral to prevent post-recovery lurch
    Serial.println(F(">> Line FOUND"));
  }
  lastLineTime  = millis();
  lostStartTime = 0;

  // Save fresh ghost-mode snapshot
  for (int i = 0; i < NUM_SENSORS; i++) lastValidCalVal[i] = calVal[i];
  lastValidPosition = position;
  lastPosition      = position;
  lastDir = (position < -300L) ? -1 : (position > 300L) ? 1 : 0;

  // ── Step 5: Intersection — ≥6 sensors lit → T-junction, go straight ──────
  if (onLine >= 6) {
    integral = 0.0f;
    setMotors(BASE_SPEED, BASE_SPEED);
    return;
  }

  // ── Step 6: Sharp turn — hard pivot for extreme positions ─────────────────
  if (sharpLeftDetected() && position < -2500L) {
    integral = 0.0f;
    setMotors(MIN_TURN_SPEED, MAX_SPEED);   // Left pivot
    return;
  }
  if (sharpRightDetected() && position > 2500L) {
    integral = 0.0f;
    setMotors(MAX_SPEED, MIN_TURN_SPEED);   // Right pivot
    return;
  }

  // ── Step 7: Normal PID with adaptive speed ────────────────────────────────
  float err    = (float)position;
  float absErr = fabsf(err);

  if (absErr < 1500.0f) {
    integral += err;
    integral  = constrain(integral, -8000.0f, 8000.0f);   // Anti-windup clamp
  } else {
    integral *= 0.85f;   // Decay — prevent accumulated error from big turns
  }

  float derivative = err - lastError;
  lastError        = err;

  float correction = (Kp * err) + (Ki * integral) + (Kd * derivative);
  correction = constrain(correction, -(float)MAX_SPEED, (float)MAX_SPEED);

  int dynamicBase;
  if      (absErr < 300.0f)   dynamicBase = BASE_SPEED;
  else if (absErr < 800.0f)   dynamicBase = BASE_SPEED - 15;
  else if (absErr < 1500.0f)  dynamicBase = BASE_SPEED - 30;
  else if (absErr < 2500.0f)  dynamicBase = BASE_SPEED - 50;
  else                         dynamicBase = BASE_SPEED - 65;

  int leftSpeed  = dynamicBase + (int)correction;
  int rightSpeed = dynamicBase - (int)correction;

  leftSpeed  = constrain(leftSpeed,  -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  setMotors(leftSpeed, rightSpeed);
}
