void sendData(String folderPath)
 {

   if (!SD.exists(folderPath)){

    if (SD.mkdir(folderPath)) {
      Serial.println("Created folder: " + folderPath);
    } else {
      Serial.println("Failed to create folder: " + folderPath);
    }
   }
    
    File root = SD.open(folderPath);

    if (!root) {
        Serial.println("Failed to open SD root");
        return;
    }

    if (!root.isDirectory()) {
        Serial.println("Root is not a directory");
        root.close();
        return;
    }

     bool com = false;
     String json = "{";
    File file = root.openNextFile();
    
     while (file) {

        String fileName = file.name();
        fileName = fileName.substring(0, fileName.indexOf(".txt"));
        // Check whether the filename starts with /table
        if (fileName.startsWith("/table")) {

            Serial.print("Table file found: ");
            Serial.println(fileName);
            file = root.openNextFile();
            
        }
        else {
          if (com) json += ",";
              com = true;
          json += "\"" + fileName + "\":";
          
          json += "{";
          bool first = true;

          while (file.available()) {
          String data1 = file.readStringUntil('\n');
           data1.trim();

          int pos = data1.indexOf(':');
            if (pos == -1) continue;

            if (!first) json += ",";
              first = false;

            String key = data1.substring(0, pos);
            String value = data1.substring(pos + 1);
    
            json += "\"" + key + "\":" + value;
          }

           json += "}";
           file.close();
        }
        file = root.openNextFile();
     }
        
    json += "}";



   server.sendHeader("Access-Control-Allow-Origin", "*");
   server.send(200, "application/json", json);
    
}
void saveItemCost(String table, String item, float cost) {
  //String folderPath = "/box10";
  String fileName = folderPath + "/" + table + ".txt";

  // Make sure the folder exists first — SD_MMC won't auto-create it
  if (!SD.exists(folderPath)) {
    if (SD.mkdir(folderPath)) {
      Serial.println("Created folder: " + folderPath);
    } else {
      Serial.println("Failed to create folder: " + folderPath);
      return;
    }
  }

  //bool exists = SD_MMC.exists(fileName);

  // FILE_APPEND creates the file if it doesn't exist, and appends if it does
  File file = SD.open(fileName, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open " + fileName);
    return;
  }

  //Serial.println(exists ? "Opened existing: " + fileName : "Created new: " + fileName);

  String cleanItem = stripQuotes(item);
  file.printf("%s:%.1f\n", cleanItem.c_str(), cost);

  file.close();
}
String stripQuotes(String s)
 {
  s.trim();
  while (s.length() > 0 && (s.charAt(0) == '"' || s.charAt(0) == '\'')) {
    s.remove(0, 1);
  }
  while (s.length() > 0 &&
         (s.charAt(s.length() - 1) == '"' || s.charAt(s.length() - 1) == '\'')) {
    s.remove(s.length() - 1, 1);
  }
  return s;
}
float readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT);
  int distance = duration * 0.0343 / 2.0;
  if (distance > 50) return 0; // no echo / out of range
  else return distance;
  // speed of sound = 343 m/s
}

void update() {
  //if (!wsConnected) {Serial.println("server"); return;}

  float distLeft  = readDistanceCM(TRIG_LEFT, ECHO_LEFT);
  float distRight = 0;
  int stateul = (distLeft > 0 || distRight > 0) ? 100 : 0;

  // clear old data
  int count = 0;
  for (int k = 0; k < n; k++) { keys[k] = ""; value[k] = 0; }

  // flush stale input, then request
  while (Serial.available()) Serial.read();
  Serial.println("flash");

  unsigned long start = millis();
  bool done = false;

  while (!done && millis() - start < 500) {
    server.handleClient();
    webSocket.loop();
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      
      if (line == "END") { done = true; break; }

      int sep = line.indexOf(':');
      if (sep != -1 && count < n) {
        keys[count] = line.substring(0, sep);
        value[count] = line.substring(sep + 1).toInt();
        count++;
      }
    }
  }

  // match by parameter name
  for (int i = 0; i < n; i++) {
    newValue[i] = 0;
    for (int j = 0; j <= count; j++) {
      if (para[i] == keys[j]) { newValue[i] = value[j]; break; }
    }
  }
  newValue[4] = stateul;   // make sure arrays and n are >= 6

  String data = "[";
  for (int i = 0; i < n+1; i++) {
    data += newValue[i];
    if (i <= n - 1) data += ",";
  }
  

  data += "]";
  Serial.println(data);
  //data = "[1,2,3,4,5]";
  server.handleClient();
  webSocket.loop();
  webSocket.broadcastTXT(data);
  server.handleClient();
  webSocket.loop();
  
  done = false;
}
String filterCommand(String data) {

 

  int start = 0;

  // Find the first printable ASCII character
  while (start < data.length()) {

    char c = data[start];

    if (c >= 32 && c <= 126) {
      break;
    }

    start++;
  }

  return data.substring(start);
}




void readOrders(String data) {
 
  int colon = data.indexOf(':');
  String table = data.substring(0, colon);

  // Get item and quantity
  int comma = data.indexOf('/');
  String item = data.substring(colon + 1, comma);
  int qty = data.substring(comma + 1).toInt();
      
  saveItemCost(table,item,qty); 
}
String waitForLine(Stream &s, unsigned long timeoutMs,bool EN) {
  String buf = "";
  unsigned long start = millis();

  while (true) {
    if (s.available()) {
      char c = s.read();
      if (c == '\n' || c == '\r') {
        if (buf.length() > 0) return buf;   // got a complete line
        // else: skip stray CR/LF before real content, keep waiting
      } else {
        buf += c;
      }
    } else {
      server.handleClient();
      webSocket.loop();
      
    }
    
    if (EN && timeoutMs != 0 && (millis() - start >= timeoutMs)) {
      return "fail";  // timed out
    }
    //chenge();
  }
}

void processOrder(String data)
 {
    int start = 0;

    while (start < data.length())
    {
        int comma = data.indexOf(',', start);

        String item;

        if (comma == -1)
        {
            // Last item
            item = data.substring(start);
        }
        else
        {
            // Get item before comma
            item = data.substring(start, comma);
        }

       //item ready
       readOrders(item);

        if (comma == -1)
            break;

        start = comma + 1;
    }
}

void webSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t *payload,
  size_t length
 ) {

  if (type == WStype_CONNECTED) {

    Serial.println("Webpage connected!");
    wsConnected  = true;
    // Send initial response
    webSocket.sendTXT(
      num,
      "{\"message\":\"Connected to ESP32\"}"
    );
  }

  else if (type == WStype_DISCONNECTED) {

    Serial.println("Webpage disconnected!");
    wsConnected = false;
  }

  else if (type == WStype_TEXT) {

    Serial.print("Received: ");
    Serial.println((char*)payload);

    String message = String((char*)payload);

    // Simple button detection
    if(message == "M_ENABLE"){
      //Serial.println("motor_enable");
      M_enable = true;
      Serial.println("M_EN");
    }
    else if(message == "LED_ENABLE"){
      digitalWrite(2,1);

    }
    else if(message == "M_DISABLE"){
     // Serial.println("motor_disable");
       M_enable = true;
       Serial.println("M_dis");
    }
    else if(message == "LED_DISABLE"){
      digitalWrite(2,0);

    }
    else if (message.startsWith("table")) {

      String parts = String((char*)payload);

      int separator = parts.indexOf('_');

      String table = parts.substring(0, separator);
      String state = parts.substring(separator + 1);
      Serial.println(table);
      Serial.println(",");
      Serial.println(state);
      if (state == "shift") {
       String source = folderPath +"/" + table + ".txt";
       
       moveFile(source.c_str(),payment.c_str());
      }
      else if (state == "delete") {
          String filePath = payment + "/" + table + ".txt"; //folderPath + table +".txt";

          if (SD.exists(filePath.c_str())) {
          SD.remove(filePath.c_str());
         }
      }
    }
    else if(message == "SD_RESET"){
      //sd_f = true;
    
     clearFolder(payment.c_str());
     clearFolder(folderPath.c_str());
    }
    else if (message.startsWith("P1:")) {
      Serial.println("Control:");
      Serial.println(message);
    }
    else if (message == "MODE1"){
      mode1 = true;
      if(mode2){
        mode2 = false;
      }
    }
    else if (message.startsWith("MODE2")) {

      datam = message.substring(6);  // remove "MODE2"

      if (datam.length() > 0) {
        mode2 = true;
        if(mode1){
        mode1 = false;
        }
      }
     
    }
    else {
       webSocket.sendTXT(
      num,
      "{\"message\":\"Check\"}"
    );
    }
    
  }
}

/*void webSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t *payload,
  size_t length
) {

  switch (type) {

    // -------------------------
    // CONNECT
    // -------------------------
    case WStype_CONNECTED:

      wsConnected = true;

      Serial.print("Webpage connected: ");
      Serial.println(num);

      webSocket.sendTXT(
        num,
        "{\"message\":\"Connected to ESP32\"}"
      );

      break;


    // -------------------------
    // DISCONNECT
    // -------------------------
    case WStype_DISCONNECTED:

      Serial.print("Webpage disconnected: ");
      Serial.println(num);

      wsConnected = false;

      break;


    // -------------------------
    // TEXT RECEIVED
    // -------------------------
    case WStype_TEXT: {

      String message = "";

      for (size_t i = 0; i < length; i++) {
        message += (char)payload[i];
      }

      message.trim();

      Serial.print("Received: ");
      Serial.println(message);


      // -------------------------
      // MOTOR ENABLE
      // -------------------------
      if (message == "M_ENABLE") {

        M_enable = true;

        Serial.println("Motor ENABLE");
      }


      // -------------------------
      // MOTOR DISABLE
      // -------------------------
      else if (message == "M_DISABLE") {

        M_enable = false;

        Serial.println("Motor DISABLE");
      }


      // -------------------------
      // LED ON
      // -------------------------
      else if (message == "LED_ENABLE") {

        digitalWrite(2, HIGH);
      }


      // -------------------------
      // LED OFF
      // -------------------------
      else if (message == "LED_DISABLE") {

        digitalWrite(2, LOW);
      }


      // -------------------------
      // TABLE
      // Example:
      // table1_open
      // table1_close
      // -------------------------
      else if (message.startsWith("table")) {

        int separator = message.indexOf('_');

        if (separator > 0) {

          String table = message.substring(0, separator);
          String state = message.substring(separator + 1);

          if (state == "open") {

            String source = folderPath + "/" + table + ".txt";
            String destination = payment;

            Serial.println("Opening table:");
            Serial.println(source);

            moveFile(source.c_str(), destination.c_str());
          }

          else if (state == "close") {

            String filePath = payment + "/" + table + ".txt";

            Serial.println("Closing table:");
            Serial.println(filePath);

            if (SD.exists(filePath.c_str())) {
              SD.remove(filePath.c_str());
            }
          }
        }
      }


      // -------------------------
      // SD RESET
      // -------------------------
      else if (message == "SD_RESET") {

        clearFolder(payment.c_str());

        Serial.println("SD reset");
      }


      // -------------------------
      // P1 COMMAND
      // -------------------------
      else if (message.startsWith("P1:")) {

        Serial.println("Control:");
        Serial.println(message);
      }


      // -------------------------
      // MODE 1
      // -------------------------
      else if (message == "MODE1") {

        mode1 = true;
        mode2 = false;

        Serial.println("MODE1 ENABLED");
      }


      // -------------------------
      // MODE 2
      // Example:
      // MODE2startnow
      // MODE2problem
      // MODE2fun
      // -------------------------
      else if (message.startsWith("MODE2")) {

        datam = message.substring(5);
        datam.trim();

        mode2 = true;
        mode1 = false;

        Serial.print("MODE2 DATA: ");
        Serial.println(datam);
      }


      // -------------------------
      // UNKNOWN COMMAND
      // -------------------------
      else {

        webSocket.sendTXT(
          num,
          "{\"message\":\"Check\"}"
        );
      }

      break;
    }


    default:
      break;
  }
}*/
void setupI2S(uint32_t sampleRate) {

  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pins = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
}

void playWAV(const char *filename) {
    String fileName = sound + "/" + filename;
     if (!SD.exists(folderPath)) {
    if (SD.mkdir(folderPath)) {
      Serial.println("Created folder: " + folderPath);
    }
  }

  File file = SD.open(fileName);

  if (!file) {
   
    return;
  }

  Serial.println("Playing...");

  // WAV header
  uint8_t header[44];

  if (file.read(header, 44) != 44) {
    
    file.close();
    return;
  }

  // Read WAV parameters
  uint16_t channels =
      header[22] | (header[23] << 8);

  uint32_t sampleRate =
      header[24] |
      (header[25] << 8) |
      (header[26] << 16) |
      (header[27] << 24);

  uint16_t bitsPerSample =
      header[34] | (header[35] << 8);

 

  // Only support 16-bit WAV
  if (bitsPerSample != 16) {
   
    file.close();
    return;
  }

  // Reconfigure I2S according to WAV sample rate
  i2s_driver_uninstall(I2S_PORT);

  setupI2S(sampleRate);

  uint8_t buffer[1024];

  while (file.available()) {

    int bytesRead = file.read(buffer, sizeof(buffer));

    if (bytesRead <= 0)
      break;

    size_t bytesWritten;

    i2s_write(
      I2S_PORT,
      buffer,
      bytesRead,
      &bytesWritten,
      portMAX_DELAY
    );
  }

  file.close();

  i2s_zero_dma_buffer(I2S_PORT);

  
}

bool moveFile(const char* src,const char* dstFolder) {
  // Extract filename from src path (everything after the last '/')
  const char* filename = strrchr(src, '/');
  if (filename) {
    filename++;  // skip the '/' itself
  } else {
    filename = src;  // no '/' found, whole thing is the filename
  }

  // Build full destination path: dstFolder + "/" + filename
  char dst[128];
  snprintf(dst, sizeof(dst), "%s/%s", dstFolder, filename);

  // Make sure destination folder exists
  if (!SD.exists(dstFolder)) {
    SD.mkdir(dstFolder);
  }

  File in = SD.open(src, FILE_READ);
  if (!in) return false;

  File out = SD.open(dst, FILE_WRITE);
  if (!out) { in.close(); return false; }

  uint8_t buf[512];
  int n;
  while ((n = in.read(buf, sizeof(buf))) > 0) {
    out.write(buf, n);
  }
  in.close();
  out.close();

  return SD.remove(src);
}
void clearFolder(const char* path) {
  File dir = SD.open(path);
  if (!dir || !dir.isDirectory()) {
    Serial.println("Not a valid directory");
    return;
  }

  File entry = dir.openNextFile();
  while (entry) {
    String entryPath = String(path) + "/" + entry.name();
    bool isDir = entry.isDirectory();
    entry.close();  // must close before removing

    if (isDir) {
      // recurse into subfolder first (optional)
      clearFolder(entryPath.c_str());
      SD.rmdir(entryPath.c_str());
    } else {
      SD.remove(entryPath.c_str());
    }

    entry = dir.openNextFile();
  }
  dir.close();
}