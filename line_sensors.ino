
// ═══════════════════════════════════════════════════════════════════════════
//  SENSOR: READ RAW
// ═══════════════════════════════════════════════════════════════════════════
void readSensors() {

  // Sensors 3, 4, 5 and 6 (Analog)
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (i == 0 || i == 7){
      calVal[i] = digitalRead(PINS[i]);
    }
    else 
    {
      calVal[i] =  (analogRead(PINS[i]) > LINE_THRESHOLD) ? HIGH : LOW;
    }
    
  }
   /*for (int i = 0; i < NUM_SENSORS; i++) {
    if (i == 7) {Serial.println(calVal[i]);}
    else {Serial.print(calVal[i]);}
   }*/
}

// ═══════════════════════════════════════════════════════════════════════════
//  POSITION: WEIGHTED AVERAGE
// ═══════════════════════════════════════════════════════════════════════════
long calculatePosition() {
  long weightedSum = 0L;
  long totalWeight = 0L;
  bool anyOnLine   = false;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (calVal[i] == HIGH) {
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
    if (calVal[i] == HIGH) count++;
  }
  return count;
}

bool sharpLeftDetected() {
  return (calVal[0] || calVal[1] ) && !(calVal[6]  && calVal[7] );
}

bool sharpRightDetected() {
  return (calVal[6] || calVal[7] ) && !(calVal[0] && calVal[1] );
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

  int leftSpeed  = constrain(GHOST_SPEED + (int)correction, -MAX_SPEED, MAX_SPEED);
  int rightSpeed = constrain(GHOST_SPEED - (int)correction, -MAX_SPEED, MAX_SPEED);

  setMotors(leftSpeed, rightSpeed);
}

// ═══════════════════════════════════════════════════════════════════════════
//  MOTOR CONTROL
// ═══════════════════════════════════════════════════════════════════════════
void setMotors(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  // ── Left motor (Channel A) ────────────────────────────────────────────
  if      (leftSpeed > 0) { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);  }
  else if (leftSpeed < 0) { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); leftSpeed  = -leftSpeed; }
  else                    { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);  }
  analogWrite(PWMA,(uint8_t)leftSpeed);

  // ── Right motor (Channel B) ───────────────────────────────────────────
  if      (rightSpeed > 0) { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);  }
  else if (rightSpeed < 0) { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); rightSpeed = -rightSpeed; }
  else                     { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, LOW);  }
  analogWrite(PWMB, (uint8_t)rightSpeed - 60);
 
  Serial.print(F("the left  :"));
  Serial.println(leftSpeed);
  Serial.print(F("the right  :"));
  Serial.println(rightSpeed );
  
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}
