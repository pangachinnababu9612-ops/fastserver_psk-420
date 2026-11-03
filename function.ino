void key(){
  int state = 8;
   bool touched = 0;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("order:");
   lcd.setCursor(0,1);
  do{
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (i<=3){
     touched = (digitalRead(channelPins[i]) == HIGH); }
     else {
     
      touched =  (analogRead(channelPins[i]) > 600);
     }
       
    // Trigger only once per touch (on the rising edge)
    if (touched && !wasTouched[i]) {
     state = handleChannelPress(i);
     //Serial.println(touched);
    }

    wasTouched[i] = touched;
  }

  delay(50);
  }
  while(!(state == 100)); 
} 
void storage(String order,char c){

    if (c == 'R'){
     // Serial.println("ok");
     for(int i = 0;i<16;i++){
            if (items[i] == "0"){
                items[i] = order;
                qty[i]++;
                break;
            }
            else if (items[i] == order){
                qty[i]++;
                break;
            }
        }
        
    }
    else if (c == 'S' && order == "send"){
     // Serial.println("send");
     
      String orderMessage = "";
        for(int i = 0;i<16;i++){
       
        if(items[i] == "0"){
        break;
        }
         
  if (i != 0) {
    orderMessage += ",";
  }

  orderMessage += TABLE;
  orderMessage += ":";
  orderMessage += items[i];
  orderMessage += "/";
  orderMessage += String(qty[i]);

  // Clear after adding to message
  items[i] = "0";
  qty[i] = 0;
 }

 // Send everything ONCE
 Serial.println(orderMessage);
      
    }
}





void barcode(){
  TABLE = "none";  
  int b1 = (digitalRead(ir1) == LOW);
  int b2 = (digitalRead(ir2) == LOW);
  int b3 = (digitalRead(ir3) == LOW);
  int b4 = (digitalRead(ir4) == LOW);

  // Convert 4-bit binary to decimal
  int decimal = (b1 * 8) +
                     (b2 * 4) +
                     (b3 * 2) +
                     (b4 * 1);
  switch (decimal) {
    case 1:
      TABLE = "table1";
      Serial.println("Table_1"); 
      break;

    case 2:
      TABLE = "table2";
      Serial.println("Table_2");
      break;

    case 3:
      TABLE = "table3";
      Serial.println("Table_3");
      break;

    case 4:
      TABLE = "table4";
      Serial.println("Table_4");
      break;

    case 5:
      TABLE = "table5";
      Serial.println("Table_5");

      break;
    case 6:
     TABLE = "table6";
      Serial.println("Table_6");

      break;
    case 7:
    TABLE = "table7";
      Serial.println("Table_7");

      break;
    case 8:
      TABLE = "table8";
      Serial.println("Table_8");

      break;
    case 9:
      TABLE = "table9";
      Serial.println("Table_9");

      break;
    case 10:
     TABLE = "table10";
      Serial.println("Table_10");

      break;
    case 11:
      TABLE = "table11";
      Serial.println("Table_11");

      break;
    case 12:
      TABLE = "table12";
      Serial.println("Table_12");

      break;
    case 13:
      TABLE = "table13";
      Serial.println("Table_13");
      break;
    case 14:
      TABLE = "table14";
      Serial.println("Table_14");

      break;
    default:
         Serial.println("no table");
         break;
    delay(200);
   
    
}
// Serial.println(TABLE);
}

int handleChannelPress(int channelIndex) {
  int val = channelValue[channelIndex];

  if (val == -1) {
    // This channel is OK
    if (inputBuffer.length() > 0) {
      int orderNumber = inputBuffer.toInt();
      processOrder(orderNumber);
      inputBuffer = "";
      
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("order:");
   lcd.setCursor(0,1);
      delay(200);
      return 133;
    }}
  if (val == -2) {
    // This channel is finish
    storage("send",'S');
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ThankYou..");
  lcd.setCursor(0,1);
   lcd.print("Order Again");
      delay(2000);
     lcd.clear();
     //Serial.println("stop");
      return 100;
    }
        
     if (val == -3) {
       lcd.clear();
      Serial.println("cancel");
      return 100;
    }     
 else {
    // This channel is a digit

    inputBuffer += String(val);
    lcd.print(val);
    
    return 122;
  }
}

void showReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome.......");
  lcd.setCursor(0, 1);
  lcd.print("Ready to order");
  delay(3000);
}

void processOrder(int orderNumber) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("your Order :");
  lcd.print(orderNumber);
  lcd.setCursor(0, 1);

  switch (orderNumber) {
    case 132:
      lcd.print("IDLI");
      storage("IDLI",'R');
      break;

    case 431:
       lcd.print("VADA");
      storage("VADA",'R');
      break;

    case 231:
      lcd.print("MASALA DOSA");
      storage("MASALA DOSA",'R');
      break;

    case 221:
      lcd.print("PLAIN DOSA");
      storage("PLAIN DOSA",'R');
      break;

    case 133:
      lcd.print("UPMA");
      storage("UPMA",'R');

      break;
    case 442:
     lcd.print("VEG FRIED RICE");
      storage("VEG FRIED RICE",'R');

      break;
    case 123:
    lcd.print("LEMON JUICE");
    storage("LEMON JUICE",'R');

      break;
    case 421:
      lcd.print("VEG NOODLEAS");
      storage("VEG NOODLEAS",'R');

      break;
    case 143:
       lcd.print("TEA");
      storage("TEA",'R');

      break;
    case 234:
      lcd.print("COFFEE");
      storage("COFFEE",'R');

      break;
    
    default:
         Serial.println("ERROR TO ORDER");
         break;
   
    }


 delay(2000);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("enter next order");
 delay(1000);
}



void printEyes(bool open) {
  lcd.clear();

  if (open) {
    lcd.setCursor(4, 0);
    lcd.write(byte(0));

    lcd.setCursor(11, 0);
    lcd.write(byte(0));

     lcd.setCursor(6, 1);
     lcd.print("----");
  } 
  else {
    lcd.setCursor(4, 0);
    lcd.write(byte(1));

    lcd.setCursor(11, 0);
    lcd.write(byte(1));

     lcd.setCursor(6, 1);
     lcd.print("----");
  }
}
