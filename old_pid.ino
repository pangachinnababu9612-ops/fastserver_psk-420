#include <Wire.h>
#include <MPU6500_WE.h>

// =================================================
// MPU6500
// =================================================

#define MPU6500_ADDR 0x68

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);


// =================================================
// TB6612FNG MOTOR PINS
// =================================================

#define LEFT_PWM   25
#define LEFT_IN1   27
#define LEFT_IN2   26

#define RIGHT_PWM  17
#define RIGHT_IN1  18
#define RIGHT_IN2  19


// =================================================
// PID
// =================================================

float Kp = 3.14;
float Ki = 0.77;
float Kd = 0.23;

float targetAngle = 0.00;

float error = 0.00;
float previousError = 0.00;
float integral = 0.00;

float motorOutput = 0.00;


// =================================================
// ANGLE
// =================================================

float angle = 0.00;


// =================================================
// TIME
// =================================================

unsigned long previousTime;


// =================================================
// SETUP
// =================================================

void setup()
{
    Serial.begin(115200);

    // -------------------------------------------------
    // I2C
    // -------------------------------------------------

    Wire.begin(21, 22);
    Wire.setClock(400000);

    delay(100);


    // -------------------------------------------------
    // MPU6500
    // -------------------------------------------------

    if (!myMPU6500.init())
    {
        Serial.println("MPU6500 connection failed!");

        while (1)
        {
            delay(100);
        }
    }

    Serial.println("MPU6500 connected!");


    // -------------------------------------------------
    // MPU RANGE
    // -------------------------------------------------

    myMPU6500.setAccRange(
        MPU6500_ACC_RANGE_2G
    );

    myMPU6500.setGyrRange(
        MPU6500_GYRO_RANGE_250
    );


    // -------------------------------------------------
    // MPU CALIBRATION
    // -------------------------------------------------

    Serial.println();
    Serial.println("Keep robot completely still...");
    Serial.println("Calibrating MPU6500...");

    delay(1000);

    myMPU6500.autoOffsets();

    Serial.println("Calibration complete!");


    // -------------------------------------------------
    // MOTOR PINS
    // -------------------------------------------------

    pinMode(
        LEFT_PWM,
        OUTPUT
    );

    pinMode(
        LEFT_IN1,
        OUTPUT
    );

    pinMode(
        LEFT_IN2,
        OUTPUT
    );


    pinMode(
        RIGHT_PWM,
        OUTPUT
    );

    pinMode(
        RIGHT_IN1,
        OUTPUT
    );

    pinMode(
        RIGHT_IN2,
        OUTPUT
    );


    // -------------------------------------------------
    // STOP MOTORS
    // -------------------------------------------------

    stopMotors();


    // -------------------------------------------------
    // INITIAL TIME
    // -------------------------------------------------

    previousTime = micros();


    // -------------------------------------------------
    // START MESSAGE
    // -------------------------------------------------

    Serial.println();
    Serial.println("==============================");
    Serial.println("SELF BALANCING ROBOT");
    Serial.println("==============================");

    Serial.print("Kp = ");
    Serial.println(Kp, 2);

    Serial.print("Ki = ");
    Serial.println(Ki, 2);

    Serial.print("Kd = ");
    Serial.println(Kd, 2);

    Serial.println("Target = 0.00 degree");
    Serial.println("Fall cutoff = +/-60 degree");

    Serial.println("==============================");
}


// =================================================
// MAIN LOOP
// =================================================

void loop()
{
    // -------------------------------------------------
    // READ MPU6500
    // -------------------------------------------------

    xyzFloat acc =
        myMPU6500.getGValues();

    xyzFloat gyr =
        myMPU6500.getGyrValues();


    // -------------------------------------------------
    // ACCELEROMETER ANGLE
    // -------------------------------------------------

    float accelAngle =
        atan2(
            acc.x,
            acc.z
        )
        * 180.0 / PI;


    // -------------------------------------------------
    // CALCULATE DT
    // -------------------------------------------------

    unsigned long currentTime =
        micros();

    float dt =
        (currentTime - previousTime)
        / 1000000.0;

    previousTime =
        currentTime;


    // -------------------------------------------------
    // PROTECT AGAINST BAD DT
    // -------------------------------------------------

    if (dt <= 0 || dt > 0.05)
    {
        dt = 0.005;
    }


    // -------------------------------------------------
    // COMPLEMENTARY FILTER
    // -------------------------------------------------

    angle =
        0.98 *
        (
            angle +
            gyr.y * dt
        )
        +
        0.02 *
        accelAngle;


    // -------------------------------------------------
    // PID ERROR
    // -------------------------------------------------

    error =
        targetAngle -
        angle;


    // -------------------------------------------------
    // INTEGRAL
    // -------------------------------------------------

    integral +=
        error * dt;


    // Anti-windup
    integral =
        constrain(
            integral,
            -50.0,
            50.0
        );


    // -------------------------------------------------
    // DERIVATIVE
    // -------------------------------------------------

    float derivative =
        (
            error -
            previousError
        )
        / dt;

    previousError =
        error;


    // -------------------------------------------------
    // PID OUTPUT
    // -------------------------------------------------

    motorOutput =
        (Kp * error)
        +
        (Ki * integral)
        +
        (Kd * derivative);


    // -------------------------------------------------
    // LIMIT MOTOR OUTPUT
    // -------------------------------------------------

    motorOutput =
        constrain(
            motorOutput,
            -255.0,
            255.0
        );


    // -------------------------------------------------
    // FALL PROTECTION
    // -------------------------------------------------

    if (abs(angle) > 60.0)
    {
        stopMotors();

        integral = 0.0;
        previousError = 0.0;
        motorOutput = 0.0;

        Serial.println("FALL PROTECTION!");

        delay(50);

        return;
    }


    // -------------------------------------------------
    // MOTOR CONTROL
    // -------------------------------------------------

    if (abs(error) < 1.5)
    {
        stopMotors();
    }
    else
    {
        setMotors(
            motorOutput
        );
    }


    // -------------------------------------------------
    // SERIAL MONITOR
    // -------------------------------------------------

    static unsigned long lastPrint = 0;

    if (
        millis() - lastPrint >= 100
    )
    {
        lastPrint = millis();

        Serial.print("Angle: ");
        Serial.print(angle, 2);

        Serial.print(" | Error: ");
        Serial.print(error, 2);

        Serial.print(" | PID: ");
        Serial.println(motorOutput, 2);
    }
}


// =================================================
// MOTOR CONTROL
// =================================================

void setMotors(
    float output
)
{
    // -------------------------------------------------
    // CONVERT OUTPUT TO PWM
    // -------------------------------------------------

    int pwm =
        abs(
            (int)output
        );

    pwm =
        constrain(
            pwm,
            0,
            255
        );


    // -------------------------------------------------
    // POSITIVE OUTPUT
    // -------------------------------------------------

    if (output > 0)
    {
        digitalWrite(
            LEFT_IN1,
            HIGH
        );

        digitalWrite(
            LEFT_IN2,
            LOW
        );


        digitalWrite(
            RIGHT_IN1,
            HIGH
        );

        digitalWrite(
            RIGHT_IN2,
            LOW
        );
    }


    // -------------------------------------------------
    // NEGATIVE OUTPUT
    // -------------------------------------------------

    else if (output < 0)
    {
        digitalWrite(
            LEFT_IN1,
            LOW
        );

        digitalWrite(
            LEFT_IN2,
            HIGH
        );


        digitalWrite(
            RIGHT_IN1,
            LOW
        );

        digitalWrite(
            RIGHT_IN2,
            HIGH
        );
    }


    // -------------------------------------------------
    // ZERO OUTPUT
    // -------------------------------------------------

    else
    {
        stopMotors();

        return;
    }


    // -------------------------------------------------
    // APPLY PWM
    // -------------------------------------------------

    analogWrite(
        LEFT_PWM,
        pwm
    );

    analogWrite(
        RIGHT_PWM,
        pwm
    );
}


// =================================================
// STOP MOTORS
// =================================================

void stopMotors()
{
    analogWrite(
        LEFT_PWM,
        0
    );

    analogWrite(
        RIGHT_PWM,
        0
    );


    digitalWrite(
        LEFT_IN1,
        LOW
    );

    digitalWrite(
        LEFT_IN2,
        LOW
    );


    digitalWrite(
        RIGHT_IN1,
        LOW
    );

    digitalWrite(
        RIGHT_IN2,
        LOW
    );
}