/*#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6500_WE.h>

#define MPU6500_ADDR 0x68

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

// =================================================
// WIFI
// =================================================

const char* AP_SSID = "BalanceRobot";
const char* AP_PASSWORD = "12345678";

WebServer server(80);


// =================================================
// MOTOR PINS - TB6612FNG
// =================================================

#define LEFT_PWM   25
#define LEFT_IN1   27
#define LEFT_IN2   26

#define RIGHT_PWM  17
#define RIGHT_IN1  18
#define RIGHT_IN2  19

// If your TB6612FNG STBY is connected to ESP32,
// define its pin here.
// If STBY is permanently connected to 3.3V,
// comment these two lines.
//
// #define STBY_PIN  23


// =================================================
// PID
// =================================================

// INITIAL VALUES

float Kp = 2.00;
float Ki = 0.00;
float Kd = 0.00;

float targetAngle = 0.00;

float error = 0.00;
float previousError = 0.00;
float integral = 0.00;

unsigned long previousTime;


// =================================================
// ANGLE
// =================================================

float angle = 0.00;


// =================================================
// MOTOR OUTPUT
// =================================================

float motorOutput = 0.00;


// =================================================
// MOVEMENT
// =================================================

// Current movement target angle

float movementAngle = 0.00;

// Maximum movement angle

float movementLimit = 30.00;


// =================================================
// WEB PAGE
// =================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>Balance Robot</title>

<style>

body {

    font-family: Arial;

    background: #111;

    color: white;

    text-align: center;

    margin: 0;

    padding: 15px;

}


h1 {

    margin-bottom: 20px;

}


.box {

    max-width: 500px;

    margin: auto;

    background: #222;

    padding: 20px;

    border-radius: 15px;

}


.item {

    margin: 22px 0;

}


label {

    display: block;

    font-size: 20px;

    margin-bottom: 10px;

}


.value {

    font-size: 22px;

    font-weight: bold;

}


input[type=range] {

    width: 100%;

}


button {

    border: none;

    border-radius: 8px;

    padding: 15px 10px;

    margin: 5px;

    font-size: 16px;

    font-weight: bold;

}


.forward {

    background: green;

    color: white;

}


.backward {

    background: red;

    color: white;

}


.balance {

    background: orange;

    color: black;

}


.data {

    margin-top: 20px;

    padding: 15px;

    background: #333;

    border-radius: 10px;

}


.data p {

    font-size: 18px;

}


</style>

</head>


<body>


<h1>🤖 Balance Robot</h1>


<div class="box">


<!-- ================================================= -->
<!-- KP -->
<!-- ================================================= -->

<div class="item">

<label>

Kp:

<span class="value" id="kpValue">2.00</span>

</label>

<input

type="range"

id="kp"

min="0"

max="10"

step="0.01"

value="2.00"

oninput="updatePID()">

</div>


<!-- ================================================= -->
<!-- KI -->
<!-- ================================================= -->

<div class="item">

<label>

Ki:

<span class="value" id="kiValue">0.00</span>

</label>

<input

type="range"

id="ki"

min="0"

max="5"

step="0.01"

value="0.00"

oninput="updatePID()">

</div>


<!-- ================================================= -->
<!-- KD -->
<!-- ================================================= -->

<div class="item">

<label>

Kd:

<span class="value" id="kdValue">0.00</span>

</label>

<input

type="range"

id="kd"

min="0"

max="5"

step="0.01"

value="0.00"

oninput="updatePID()">

</div>


<!-- ================================================= -->
<!-- TARGET ANGLE -->
<!-- ================================================= -->

<div class="item">

<label>

Target Angle:

<span class="value" id="targetValue">0.00</span>°

</label>

<input

type="range"

id="target"

min="-10"

max="10"

step="0.01"

value="0.00"

oninput="updatePID()">

</div>


<!-- ================================================= -->
<!-- MOVEMENT ANGLE -->
<!-- ================================================= -->

<div class="item">

<label>

Movement Angle:

<span class="value"
id="movementValue">0.00</span>°

</label>

<input

type="range"

id="movement"

min="0"

max="30"

step="0.1"

value="0"

oninput="updateMovement()">

</div>


<!-- ================================================= -->
<!-- MOVEMENT BUTTONS -->
<!-- ================================================= -->

<div class="item">


<button
class="forward"
onclick="moveForward()">

FORWARD

</button>


<button
class="balance"
onclick="balanceRobot()">

BALANCE

</button>


<button
class="backward"
onclick="moveBackward()">

BACKWARD

</button>


</div>


<!-- ================================================= -->
<!-- LIVE DATA -->
<!-- ================================================= -->

<div class="data">


<p>

Angle:

<span id="angle">0.00</span>°

</p>


<p>

Target:

<span id="targetLive">0.00</span>°

</p>


<p>

Error:

<span id="error">0.00</span>

</p>


<p>

Motor:

<span id="motor">0.00</span>

</p>


</div>


</div>


<script>


// =================================================
// PID
// =================================================

function updatePID() {


    let kp =
        document.getElementById("kp").value;


    let ki =
        document.getElementById("ki").value;


    let kd =
        document.getElementById("kd").value;


    let target =
        document.getElementById("target").value;


    document.getElementById("kpValue").innerHTML =
        parseFloat(kp).toFixed(2);


    document.getElementById("kiValue").innerHTML =
        parseFloat(ki).toFixed(2);


    document.getElementById("kdValue").innerHTML =
        parseFloat(kd).toFixed(2);


    document.getElementById("targetValue").innerHTML =
        parseFloat(target).toFixed(2);


    fetch(

        "/pid?kp=" + kp +
        "&ki=" + ki +
        "&kd=" + kd +
        "&target=" + target

    );

}


// =================================================
// MOVEMENT ANGLE
// =================================================

function updateMovement() {


    let movement =
        document.getElementById("movement").value;


    document.getElementById("movementValue").innerHTML =
        parseFloat(movement).toFixed(2);


    fetch(

        "/movement?angle=" + movement

    );

}


// =================================================
// FORWARD
// =================================================

function moveForward() {

    fetch("/move?direction=forward");

}


// =================================================
// BACKWARD
// =================================================

function moveBackward() {

    fetch("/move?direction=backward");

}


// =================================================
// BALANCE
// =================================================

function balanceRobot() {

    fetch("/move?direction=balance");

}


// =================================================
// LIVE DATA
// =================================================

function updateData() {


    fetch("/data")

    .then(response => response.json())

    .then(data => {


        document.getElementById("angle").innerHTML =
            data.angle.toFixed(2);


        document.getElementById("targetLive").innerHTML =
            data.target.toFixed(2);


        document.getElementById("error").innerHTML =
            data.error.toFixed(2);


        document.getElementById("motor").innerHTML =
            data.motor.toFixed(2);

    });

}


setInterval(updateData, 100);


</script>


</body>

</html>

)rawliteral";


// =================================================
// SETUP
// =================================================

void setup() {

    Serial.begin(115200);


    // =================================================
    // I2C
    // =================================================

    Wire.begin(21, 22);

    Wire.setClock(400000);

    delay(100);


    // =================================================
    // MPU6500
    // =================================================

    if (!myMPU6500.init()) {

        Serial.println(
            "MPU6500 connection failed!"
        );

        while (1);

    }


    Serial.println(
        "MPU6500 connected!"
    );


    myMPU6500.setAccRange(
        MPU6500_ACC_RANGE_2G
    );


    myMPU6500.setGyrRange(
        MPU6500_GYRO_RANGE_250
    );


    // =================================================
    // MPU CALIBRATION
    // =================================================

    Serial.println(
        "Keep MPU6500 still..."
    );

    delay(1000);

    myMPU6500.autoOffsets();

    Serial.println(
        "Calibration complete!"
    );


    // =================================================
    // MOTOR PINS
    // =================================================

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


    stopMotors();


    // =================================================
    // STANDBY
    // =================================================

    /*
    If STBY is connected to an ESP32 GPIO,
    uncomment and configure:

    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(STBY_PIN, HIGH);

    */

/*
    // =================================================
    // TIME
    // =================================================

    previousTime = micros();


    // =================================================
    // WIFI ACCESS POINT
    // =================================================

    WiFi.mode(WIFI_AP);


    WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );


    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        "BALANCE ROBOT"
    );

    Serial.println(
        "=============================="
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "Password: "
    );

    Serial.println(
        AP_PASSWORD
    );


    Serial.print(
        "IP Address: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    Serial.println(
        "=============================="
    );


    // =================================================
    // HOME PAGE
    // =================================================

    server.on(
        "/",
        HTTP_GET,
        []() {

            server.send_P(
                200,
                "text/html",
                webpage
            );

        }
    );


    // =================================================
    // PID UPDATE
    // =================================================

    server.on(
        "/pid",
        HTTP_GET,
        []() {


            if (server.hasArg("kp")) {

                Kp =
                    server.arg("kp").toFloat();

            }


            if (server.hasArg("ki")) {

                Ki =
                    server.arg("ki").toFloat();

            }


            if (server.hasArg("kd")) {

                Kd =
                    server.arg("kd").toFloat();

            }


            if (server.hasArg("target")) {

                targetAngle =
                    server.arg("target").toFloat();

            }


            // Reset integral
            integral = 0;


            server.send(
                200,
                "text/plain",
                "OK"
            );

        }
    );


    // =================================================
    // MOVEMENT ANGLE
    // =================================================

    server.on(
        "/movement",
        HTTP_GET,
        []() {


            if (server.hasArg("angle")) {

                movementAngle =
                    server.arg("angle").toFloat();

            }


            movementAngle =
                constrain(
                    movementAngle,
                    0.0,
                    movementLimit
                );


            server.send(
                200,
                "text/plain",
                "OK"
            );

        }
    );


    // =================================================
    // MOVEMENT CONTROL
    // =================================================

    server.on(
        "/move",
        HTTP_GET,
        []() {


            if (!server.hasArg("direction")) {

                server.send(
                    400,
                    "text/plain",
                    "Missing direction"
                );

                return;

            }


            String direction =
                server.arg("direction");


            // -----------------------------------------
            // FORWARD
            // -----------------------------------------

            if (
                direction == "forward"
            ) {

                targetAngle =
                    -movementAngle;

            }


            // -----------------------------------------
            // BACKWARD
            // -----------------------------------------

            else if (
                direction == "backward"
            ) {

                targetAngle =
                    movementAngle;

            }


            // -----------------------------------------
            // BALANCE
            // -----------------------------------------

            else if (
                direction == "balance"
            ) {

                targetAngle =
                    0.00;

            }


            // Reset integral

            integral = 0;


            server.send(
                200,
                "text/plain",
                "OK"
            );

        }
    );


    // =================================================
    // LIVE DATA
    // =================================================

    server.on(
        "/data",
        HTTP_GET,
        []() {


            String json = "{";


            json += "\"angle\":";
            json += String(
                angle,
                2
            );


            json += ",\"target\":";
            json += String(
                targetAngle,
                2
            );


            json += ",\"error\":";
            json += String(
                error,
                2
            );


            json += ",\"motor\":";
            json += String(
                motorOutput,
                2
            );


            json += ",\"kp\":";
            json += String(
                Kp,
                2
            );


            json += ",\"ki\":";
            json += String(
                Ki,
                2
            );


            json += ",\"kd\":";
            json += String(
                Kd,
                2
            );


            json += "}";


            server.send(
                200,
                "application/json",
                json
            );

        }
    );


    // =================================================
    // START SERVER
    // =================================================

    server.begin();


    Serial.println(
        "Web server started!"
    );


    Serial.println(
        "Balance system ready"
    );

}


// =================================================
// MAIN LOOP
// =================================================

void loop() {


    // =================================================
    // WEB SERVER
    // =================================================

    server.handleClient();


    // =================================================
    // READ MPU6500
    // =================================================

    xyzFloat acc =
        myMPU6500.getGValues();


    xyzFloat gyr =
        myMPU6500.getGyrValues();


    // =================================================
    // ACCELEROMETER ANGLE
    // =================================================

    float accelAngle =
        atan2(
            acc.x,
            acc.z
        )
        * 180.0 / PI;


    // =================================================
    // TIME
    // =================================================

    unsigned long currentTime =
        micros();


    float dt =
        (currentTime - previousTime)
        / 1000000.0;


    previousTime =
        currentTime;


    if (
        dt <= 0 ||
        dt > 0.05
    ) {

        dt = 0.005;

    }


    // =================================================
    // COMPLEMENTARY FILTER
    // =================================================

    angle =
        0.98 *
        (
            angle +
            gyr.y * dt
        )
        +
        0.02 *
        accelAngle;


    // =================================================
    // PID ERROR
    // =================================================

    error =
        targetAngle -
        angle;


    // =================================================
    // INTEGRAL
    // =================================================

    integral +=
        error * dt;


    integral =
        constrain(
            integral,
            -50.0,
            50.0
        );


    // =================================================
    // DERIVATIVE
    // =================================================

    float derivative =
        (
            error -
            previousError
        )
        / dt;


    previousError =
        error;


    // =================================================
    // PID OUTPUT
    // =================================================

    motorOutput =

        (Kp * error)

        +

        (Ki * integral)

        +

        (Kd * derivative);


    // =================================================
    // OUTPUT LIMIT
    // =================================================

    motorOutput =
        constrain(
            motorOutput,
            -255.0,
            255.0
        );


    // =================================================
    // FALL PROTECTION
    // =================================================

    if (
        abs(angle) > 45.0
    ) {


        stopMotors();


        integral = 0;


    }

    else {


        // =================================================
        // NEAR ZERO
        // =================================================

        if (
            abs(error) < 1.0
        ) {


            stopMotors();


        }

        else {


            setMotors(
                motorOutput
            );


        }

    }

}


// =================================================
// MOTOR CONTROL
// =================================================

void setMotors(
    float output
) {


    // Convert negative
    // output to positive PWM

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


    // =================================================
    // POSITIVE OUTPUT
    // =================================================

    if (
        output > 0
    ) {


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


    // =================================================
    // NEGATIVE OUTPUT
    // =================================================

    else if (
        output < 0
    ) {


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


    // =================================================
    // ZERO
    // =================================================

    else {


        stopMotors();

        return;

    }


    // =================================================
    // PWM
    // =================================================

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

void stopMotors() {


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

}*/