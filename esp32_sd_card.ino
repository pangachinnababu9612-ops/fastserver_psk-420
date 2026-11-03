#include <Arduino.h>


#include <SPI.h>
#include <SD.h>
#include "driver/i2s.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

// Wi-Fi credentials
const char* ssid = "Airtel__sanathkumar";
const char* password = "Sanath@kumar#123";
const char* hostname = "sanath";

String keys[4] = {"0","0","0","0"};
int value[4] = {0,0,0,0};
String para[4] = {"speed1","speed2","line","angle"};

// Fixed IP configuration
IPAddress local_IP(192, 168, 1, 20);    // ESP32 IP
IPAddress gateway(192, 168, 1, 1);       // Router IP
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
// --------------------------------------------------
// variable
// --------------------------------------------------
#define SD_CS    5
#define SD_SCK   18
#define SD_MISO  19
#define SD_MOSI  23

// ---------- I2S ----------
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

#define I2S_PORT I2S_NUM_0
//---------------------------------------------------
String payment = "/pay";
String folderPath = "/list";
String sound = "/music";
int command = 0;
const unsigned long PULSE_TIMEOUT = 30000;
int n = 4; // number of items
bool wsConnected = false;

int newValue[5] = {0,0,0,0,0};

#define SLAVE_SERIAL   Serial   // ESP32 slave  (movement control)
#define NANO_SERIAL    Serial2   // Arduino Nano (table display / order)

#define SLAVE_BAUD     115200
#define NANO_BAUD      9600
const unsigned long SHORT_TIMEOUT_MS = 10000;  
const unsigned long ORDER_TIMEOUT_MS = 0;

bool mode1 = false;
bool mode2 = false;
bool sd_f = false;
bool M_enable = false;
bool led_enable = false;
bool pid = false;
String datam;
// --------------------------------------------------
// pins show
// --------------------------------------------------
#define TRIG_LEFT   33
#define ECHO_LEFT   35
#define TRIG_RIGHT  32
#define ECHO_RIGHT  34
// --------------------------------------------------
// Function declarations
// --------------------------------------------------
void processOrder(String data);
void update();
void sendData(String folderPath);
void saveItemCost(String, String, float );
String stripQuotes(String);
bool logData(String newEntry);
void printFullFile();
void handleRoot();
float readDistanceCM(int, int );
String waitForLine(Stream &s, unsigned long timeoutMs,bool EN);
void readOrders(String);
void playWAV(const char *filename);
void clearFolder(const char* path);
bool moveFileconst (char* src,const char* dstFolder);
void setupI2S(uint32_t sampleRate);
String filterCommand(String data);
void webSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t *payload,
  size_t length
 );

// --------------------------------------------------
// SETUP
// --------------------------------------------------
 unsigned long board = millis();
void handleImage() {
  File file = SD.open("/image.jpeg");
  if (!file) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }

  // streamFile sets Content-Type and Content-Length automatically
  server.streamFile(file, "image/jpeg");
  file.close();
}
void datalist(){
  sendData(folderPath);
}
void datapay(){
  sendData(payment);
}
void clearSerialBuffer() {
  while (Serial2.available()) {
    
    int rrr = Serial2.read();
  }
}
void setup() {

  Serial.begin(9600);
  //SLAVE_SERIAL.begin(SLAVE_BAUD);
  NANO_SERIAL.begin(9600, SERIAL_8N1, 16, 17);
  delay(2000);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(2,OUTPUT);
  delay(30);
  digitalWrite(TRIG_LEFT, LOW);
  digitalWrite(TRIG_RIGHT, LOW);

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("SD Card failed!");
    while (1);
  }


  Serial.println("SD Card initialized successfully.");

  // ------------------------------------------------
  // Check SD card type
  // ------------------------------------------------
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {

    Serial.println("No SD card detected!");

    while (true) {
      delay(1000);
    }
  }
 
  WiFi.mode(WIFI_AP);

  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("AP Config Failed");
  }

  WiFi.softAP("ESP32-Robot", "12345678");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());


 
    
    // Web server route
    server.on("/", handleRoot);
    server.on("/food.jpg", handleImage);
    server.on("/data11", datalist);
    server.on("/data22", datapay);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Web Server Started");

     // ---------- I2S ----------
  setupI2S(44100);

  // ---------- PLAY ----------
  playWAV("/welcome.wav");
}
// --------------------------------------------------
// LOOP
// --------------------------------------------------
void loop() {
 server.handleClient();
  webSocket.loop();
  if (millis() - board > 1000) {
  update();
  board = millis();
  }
 

 
if(mode1 || mode2){
  if(mode1){
   while (true) {

    // ---- Step 1: tell slave to go ----
    SLAVE_SERIAL.println("GO");
    String slaveResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,false);
    slaveResp.trim();
    slaveResp = filterCommand(slaveResp);
    
    if (slaveResp != "RET") {
      // Didn't get the expected handshake — restart the whole function.
      continue;
      //no/off
    }
   

    // ---- Step 2: ask Nano to check the table ----
    NANO_SERIAL.println("scan");
    String nanoResp = waitForLine(NANO_SERIAL, SHORT_TIMEOUT_MS,true);
   
    nanoResp.trim();
    Serial.println(nanoResp);
    nanoResp = filterCommand(nanoResp);
    if (!nanoResp.startsWith("Table")) {
      // Not a valid table response — restart the whole function from the top.
      SLAVE_SERIAL.println("SKIP");
      continue;
    }

    String tableStr = nanoResp;   // e.g. "table1", "table9"
     float dL  = readDistanceCM(TRIG_LEFT, ECHO_LEFT);
     float dR = dL-4;
     //float dR = readDistanceCM(TRIG_RIGHT, ECHO_RIGHT);
     Serial.println(dL);
     
     if(!(dL>0|| dR>0)){
          continue;
     }
    // ---- Step 3: turn 90 ----
    SLAVE_SERIAL.println("T90");
    String turnResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,true);
    turnResp.trim();
    turnResp = filterCommand(turnResp);
    if (turnResp != "Tok") {
      // Turn didn't confirm — restart the whole function from the top.
      continue;
    }

    // ---- Step 4: place order, wait (servicing updateFunction) for data ----
     clearSerialBuffer();
    NANO_SERIAL.println("order place");
    playWAV("/takeorder.wav");
   
    String orderData = waitForLine(NANO_SERIAL, ORDER_TIMEOUT_MS,false); // waits forever
    orderData.trim();
    Serial.println(orderData);
    orderData = filterCommand(orderData);
    processOrder(orderData);
    playWAV("/thankyou.wav");
    Serial.println("B90");
    turnResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,true);
    if (turnResp != "Bok"){
      continue;
    }
    // orderData now holds whatever the Nano sent for the order.
    // TODO: parse/use orderData as needed (e.g. forward to web dashboard).

    // ---- Step 5: check table number, decide next action ----
    int tableNum = tableStr.substring(6).toInt();  // "table9" -> 9

    if (tableNum == 9) {
       mode1 = false;
      SLAVE_SERIAL.println("LEAVE");
      break;        // mode1 cycle complete, exit
    }
  
   // else: loop back and run the whole function again for the next table
    }
   }
   else if(mode2){
     while (true) {
     
    // ---- Step 1: tell slave to go ----
    SLAVE_SERIAL.println("GO");
    String slaveResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,false);
    slaveResp.trim();
    
    if (slaveResp != "RET") {
      // Didn't get the expected handshake — restart the whole function.
      continue;
      //no/off
    }
  

    // ---- Step 2: ask Nano to check the table ----
    NANO_SERIAL.println("scan");
    String nanoResp = waitForLine(NANO_SERIAL, SHORT_TIMEOUT_MS,true);
    nanoResp.trim();

    if (!nanoResp.startsWith("ok")) {
      // Not a valid table response — restart the whole function from the top.
      SLAVE_SERIAL.println("SKIP");
      continue;
    }

    String tableStr = nanoResp;   // e.g. "table1", "table9"
     float dL  = readDistanceCM(TRIG_LEFT, ECHO_LEFT);
     float dR = dL - 5;//readDistanceCM(TRIG_RIGHT, ECHO_RIGHT);

     if(!(dL>0|| dR>0)){
          continue;
     }
    // ---- Step 3: turn 90 ----
    SLAVE_SERIAL.println("T90");
    String turnResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,true);
    turnResp.trim();

    if (turnResp != "Tok") {
      // Turn didn't confirm — restart the whole function from the top.
      continue;
    }
    
      if (datam == "startnow") {
         playWAV("/starter.wav");
      // your start action here
      }
      else if (datam == "problem") {
      playWAV("/problem.wav");
        // your problem handling here
      }
      else if (datam == "fun") {
        playWAV("/fun.wav");
        // your fun action here
      }
      else {
        Serial.println("Sorry, I did not understand");
      }

    Serial.println("B90");
   
    turnResp = waitForLine(SLAVE_SERIAL, SHORT_TIMEOUT_MS,true);
    if (turnResp != "Bok"){
      continue;
    }
    else{
      Serial.println("b90ok");
    }
    int tableNum = tableStr.substring(6).toInt();  // "table9" -> 9

    if (tableNum == 9) {
      SLAVE_SERIAL.println("LEAVE");
      mode2 = false;
      break;        // mode1 cycle complete, exit
    }
     mode2 = false;
   break;
  
    }
  }
}

}