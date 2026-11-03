#include <Wire.h>
#include <LiquidCrystal_I2C.h>


void key();
void storage(String ,char );
void barcode();
int handleChannelPress(int);
void showReady();
void processOrder(int);
void printEyes(bool open);

String items[16],TABLE;
int qty[16];
const int NUM_CHANNELS = 7;
const int channelPins[NUM_CHANNELS] = {7, 11, 10, 8,A3,A6,A7};

const int channelValue[NUM_CHANNELS] = {1, 2, 3, 4,-1,-2,-3}; 

bool wasTouched[NUM_CHANNELS] = {false, false, false, false,false, false, false};

String inputBuffer = "";

int ir1 = 3;
int ir2 = 4;
int ir3 = 5;
int ir4 = 6;

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte eyeOpen[] = {
  B00000,
  B01110,
  B11111,
  B11011,
  B11111,
  B01110,
  B00000,
  B00000
};

byte eyeClosed[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};
void setup() {
  Serial.begin(9600);
   for(int i = 0;i<16;i++){
        items[i] = "0";
        qty[i] = 0;
    }
    
  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(channelPins[i], INPUT);
  }
  pinMode(ir1, INPUT);
 pinMode(ir2, INPUT);
  pinMode(ir3, INPUT);
  pinMode(ir4, INPUT);

    
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, eyeOpen);
  lcd.createChar(1, eyeClosed);

} 

void loop() {
   if(Serial.available()){
        String value = Serial.readStringUntil('\n');
        value.trim();
     if (value == "scan"){
         barcode();
     }
    else if (value == "order place"){
        showReady();
        key();
    }
    }
    printEyes(true);
  delay(2000);

  printEyes(false);
  delay(200);
}

  

