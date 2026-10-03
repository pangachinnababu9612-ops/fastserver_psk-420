/*#include <WiFi.h>
float readZMCT103C(int sensorPin) ;

// ---------- Wi-Fi ----------
const char* ssid = "POWER_LINE";
const char* password = "12345678";

WiFiServer server(5000);
WiFiClient client;

// ---------- Pins ----------
#define SENSOR_PIN 0
#define RELAY_PIN 4

// ---------- Settings ----------
#define DIFFERENCE_LIMIT 0.20

bool connected = false;

void setup() {

  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);

  // Create SoftAP
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("MASTER STARTED");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.begin();

  Serial.println("Waiting for SLAVE...");
}


void loop() {

  // --------------------------------
  // Check slave connection
  // --------------------------------

  if (!client || !client.connected()) {

    client = server.available();

    if (client) {
      connected = true;

      Serial.println();
      Serial.println("SLAVE CONNECTED");
    }

    return;
  }


  // --------------------------------
  // Read MASTER sensor
  // --------------------------------

  int masterValue = readZMCT103C(SENSOR_PIN);
  


  // --------------------------------
  // Send sensor value to slave
  // --------------------------------

  client.print("DATA");
  //client.println(masterValue);

 // Serial.println("Sensor value sent");


  // --------------------------------
  // Wait for SLAVE response
  // --------------------------------

  unsigned long startTime = millis();

  String response = "";

  while (millis() - startTime < 2000) {

    if (client.available()) {

      response = client.readStringUntil('\n');
      response.trim();
     
      break;
    }

    delay(5);
  }

  
  // --------------------------------
  // Check response
  // --------------------------------
int slaveValue = response.toInt();
if (slaveValue){
int difference = abs(masterValue - slaveValue);


      // --------------------------------
      // Compare
      // --------------------------------
      
      if (difference <= DIFFERENCE_LIMIT) {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("relay off");
        delay(100);
        client.print("normal");
        return;
      }
      else {
        digitalWrite(RELAY_PIN, HIGH);
         Serial.println("relay on");
         delay(100);
        client.println("Shift");
        return;
      }
}
     

  delay(200);
}*/

//slave code

#include <WiFi.h>
float readZMCT103C(int sensorPin) ;
// ---------- Wi-Fi ----------
const char* ssid = "POWER_LINE";
const char* password = "12345678";

WiFiClient client;

// Master SoftAP IP
IPAddress masterIP(192, 168, 4, 1);

const int masterPort = 5000;


// ---------- Pins ----------
#define SENSOR_PIN 0
#define RELAY_PIN 4


// ---------- Settings ----------
#define DIFFERENCE_LIMIT 50

void connectToMaster() {

  Serial.println("Connecting to MASTER...");

  while (!client.connect(masterIP, masterPort)) {

    Serial.println("Connection failed");

    delay(1000);
  }

  Serial.println("MASTER CONNECTED");
}

void setup() {

  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);


  // Connect to MASTER Wi-Fi

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println("SLAVE STARTING");

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println("Wi-Fi connected");

  Serial.print("Slave IP: ");
  Serial.println(WiFi.localIP());


  // Connect TCP

  connectToMaster();
}





void loop() {

  // --------------------------------
  // Check Wi-Fi
  // --------------------------------

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected");

    //digitalWrite(RELAY_PIN, LOW);

    WiFi.disconnect();

    WiFi.begin(ssid, password);

    delay(1000);

    return;
  }


  // --------------------------------
  // Check TCP
  // --------------------------------

  if (!client.connected()) {

    Serial.println("MASTER disconnected");

    digitalWrite(RELAY_PIN, LOW);

    connectToMaster();

    return;
  }


  // --------------------------------
  // Receive MASTER sensor
  // --------------------------------

  if (client.available()) {

    String message = client.readStringUntil('\n');

    message.trim();
    // --------------------------------
    // Check DATA command
    // --------------------------------

    if (message == "DATA") {

      int slaveValue = readZMCT103C(SENSOR_PIN);

       client.println(slaveValue);  
      
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