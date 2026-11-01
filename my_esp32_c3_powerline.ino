/*
float readZMCT103C(int sensorPin) ;
HardwareSerial mySerial(1);
// ---------- Pins ----------
#define SENSOR_PIN 0
#define RELAY_PIN 4

// ---------- Settings ----------
#define DIFFERENCE_LIMIT 0.20

void setup() {

  Serial.begin(115200);
   mySerial.begin(115200,SERIAL_8N1,20,21);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);

  Serial.println("Waiting for SLAVE...");
}


void loop() {


  int masterValue = readZMCT103C(SENSOR_PIN);
  


  // --------------------------------
  // Send sensor value to slave
  // --------------------------------

  mySerial.println("DATA");
  // --------------------------------
  // Wait for SLAVE response
  // --------------------------------

  unsigned long startTime = millis();

  String response = "";

  while (millis() - startTime < 2000) {

    if (mySerial.available()) {

      response = mySerial.readStringUntil('\n');
      response.trim();
     
      break;
    }

    delay(5);
  }

  
  // --------------------------------
  // Check response
  // --------------------------------
  Serial.println(response);
float slaveValue = response.toFloat();
 
if (slaveValue){
int difference = abs(masterValue - slaveValue);


      // --------------------------------
      // Compare
      // --------------------------------
      
      if (difference <= DIFFERENCE_LIMIT) {
        digitalWrite(RELAY_PIN, LOW);
        //Serial.println("relay off");
        delay(1000);
        mySerial.println("normal");
        return;
      }
      else {
        digitalWrite(RELAY_PIN, HIGH);
         //Serial.println("relay on");
         delay(1000);
        mySerial.println("Shift");
        return;
      }
}
     

  delay(200);
}
*/
//slave code


float readZMCT103C(int sensorPin) ;
HardwareSerial mySerial(1);
// ---------- Pins ----------
#define SENSOR_PIN 0
#define RELAY_PIN 4
void setup() {
    Serial.begin(115200);
  mySerial.begin(115200,SERIAL_8N1,20,21);

  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);


}
void loop() {

  if (mySerial.available()) {

    String message = mySerial.readStringUntil('\n');
     Serial.println(message);
    message.trim();
    
    // --------------------------------
    // Check DATA command
    // --------------------------------

    if (message == "DATA") {

      float slaveValue = readZMCT103C(SENSOR_PIN);

       mySerial.println(slaveValue);  
      
    }
    if (message == "normal") {
       digitalWrite(RELAY_PIN, LOW);
       return;
    }
     if (message == "Shift"){
      digitalWrite(RELAY_PIN, LOW);
      return;
     }

        }
}
