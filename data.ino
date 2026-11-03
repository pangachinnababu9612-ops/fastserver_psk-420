
#define STBY 10


String command = "";

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(9600);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  stopMotors();

  Serial.println("ROBOT READY");
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  receiveCommand();



  blackLineDetected();

  Serial.println("BLACK DETECTED");

  

    }
  
}

// =====================================
// SERIAL COMMUNICATION
// =====================================

void receiveCommand() {

  if (Serial.available()) {

    command = Serial.readStringUntil('\n');

    command.trim();

    if (command == "GO") {

      //STAND BY HIGH

      Serial.println("RET");
    }

    else if (command == "T90") {
      rotateRight();
      delay(TURN_90_TIME);
      stopMotors();

      Serial.println("Tok");
    }

    else if (command == "B90") {

      stopMotors();

      rotateLeft();

      delay(TURN_90_TIME);

      stopMotors();
      moveForward();

      Serial.println("bok");
    }

    else if (command == "R180") {

      stopMotors();

      rotateRight();

      delay(TURN_180_TIME);

      stopMotors();
      moveForward();

      Serial.println("Rok");
    }

    else if (command == "SKIP") {
       moveForward();


    }

    else {

      Serial.println("INVALID COMMAND");
    }
  }
}

// =====================================
// SENSOR DETECTION
// =====================================

bool blackLineDetected() {
    if (onLine == 8)
                  {
        stopMotors();
        delay(2000);
        return true;
      }
}
// =====================================
// MOTOR CONTROL
// =====================================

void moveForward() {

  digitalWrite(STBY, HIGH);

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, BASE_SPEED);
  analogWrite(PWMB, BASE_SPEED);
}

void reverseMotors() {

  digitalWrite(STBY, HIGH);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, REVERSE_SPEED);
  analogWrite(PWMB, REVERSE_SPEED);
}

void stopMotors() {

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

// =====================================
// ROTATION CONTROL
// =====================================

void rotateRight() {

  digitalWrite(STBY, HIGH);

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, TURN_SPEED);
  analogWrite(PWMB, TURN_SPEED);
}

void rotateLeft() {

  digitalWrite(STBY, HIGH);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, TURN_SPEED);
  analogWrite(PWMB, TURN_SPEED);
}
/*
#define ENCODER_PIN 2

volatile unsigned long encoderCount = 0;

void encoderISR() {
  encoderCount++;  // Count positive edge only
}

void setup() {

  Serial.begin(9600);

  pinMode(ENCODER_PIN, INPUT);

  // Trigger only on LOW to HIGH transition
  attachInterrupt(
    digitalPinToInterrupt(ENCODER_PIN),
    encoderISR,
    RISING
  );

  Serial.println("MAGNETIC ENCODER READY");
}

void loop() {

  static unsigned long previousTime = 0;

  if (millis() - previousTime >= 500) {

    previousTime = millis();

    unsigned long count;

    noInterrupts();
    count = encoderCount;
    interrupts();

    Serial.print("Encoder Count: ");
    Serial.println(count);
  }
}
*/