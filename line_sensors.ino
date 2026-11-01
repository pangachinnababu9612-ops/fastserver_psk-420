
// ═══════════════════════════════════════════════════════════════════════════
//  SENSOR: READ RAW
// ═══════════════════════════════════════════════════════════════════════════
void readSensors() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    rawVal[i] = analogRead(SENSOR_PINS[i]);
  }
}


// ═══════════════════════════════════════════════════════════════════════════
//  POSITION: WEIGHTED AVERAGE
// ═══════════════════════════════════════════════════════════════════════════
long calculatePosition() {
  long weightedSum = 0L;
  long totalWeight = 0L;
  bool anyOnLine   = false;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (calVal[i] > LINE_THRESHOLD) {
      anyOnLine    = true;
      weightedSum += (long)calVal[i] * (long)WEIGHTS[i];
      totalWeight += (long)calVal[i];
    }
  }

  if (!anyOnLine || totalWeight == 0L) return -9999L;
  return weightedSum / totalWeight;
}

int countOnLine() {
  int count = 0;
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (calVal[i] > LINE_THRESHOLD) count++;
  }
  return count;
}

bool sharpLeftDetected() {
  return (calVal[0] > LINE_THRESHOLD || calVal[1] > LINE_THRESHOLD) &&
         (calVal[6] < LINE_THRESHOLD && calVal[7] < LINE_THRESHOLD);
}

bool sharpRightDetected() {
  return (calVal[6] > LINE_THRESHOLD || calVal[7] > LINE_THRESHOLD) &&
         (calVal[0] < LINE_THRESHOLD && calVal[1] < LINE_THRESHOLD);
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOST-LINE HANDLER
// ═══════════════════════════════════════════════════════════════════════════
void lostLineHandler() {
  unsigned long ghostDuration = millis() - lostStartTime;

  if (ghostDuration > LOST_RECOVERY_MS) {
    stopMotors();
    return;
  }

  if (labs(lastValidPosition) < 800L) {
    setMotors(BASE_SPEED, BASE_SPEED);
    return;
  }

  for (int i = 0; i < NUM_SENSORS; i++) calVal[i] = lastValidCalVal[i];

  float err        = (float)lastValidPosition;
  float derivative = err - lastError;
  lastError        = err;

  float correction = (Kp * err) + (Kd * derivative);
  correction = constrain(correction, -(float)MAX_SPEED, (float)MAX_SPEED);

  int leftSpeed  = constrain(GHOST_SPEED + (int)correction, 0, MAX_SPEED);
  int rightSpeed = constrain(GHOST_SPEED - (int)correction, 0, MAX_SPEED);

  setMotors(leftSpeed, rightSpeed);
}

// ═══════════════════════════════════════════════════════════════════════════
//  MOTOR CONTROL
// ═══════════════════════════════════════════════════════════════════════════
void setMotors(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed,  0, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, 0, MAX_SPEED);

  // ── Left motor (Channel A) ────────────────────────────────────────────
  if      (leftSpeed > 0) { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);  }
  else if (leftSpeed < 0) { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); leftSpeed  = -leftSpeed; }
  else                    { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);  }
  analogWrite(PWMA, (uint8_t)leftSpeed);

  // ── Right motor (Channel B) ───────────────────────────────────────────
  if      (rightSpeed > 0) { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);  }
  else if (rightSpeed < 0) { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); rightSpeed = -rightSpeed; }
  else                     { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, LOW);  }
  analogWrite(PWMB, (uint8_t)rightSpeed - 65);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}
