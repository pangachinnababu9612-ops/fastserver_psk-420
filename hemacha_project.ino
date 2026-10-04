#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// -------- PIN DEFINITIONS --------
#define ACS712_PIN A0
#define MQ2_PIN    A1
#define FLAME_PIN  2
#define TEMP_PIN   3

// -------- TEMPERATURE SENSOR --------
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);
// -------- ACS712-20A --------
// ACS712-20A sensitivity = 100 mV/A
float sensitivity = 0.100;
String fam = "none";
// Arduino reference voltage
float Vref = 5.0;

// -------- SAFETY LIMITS --------
float MAX_CURRENT = 5.0;      // A - demonstration limit
float MAX_TEMP = 50.0;        // °C
int GAS_LIMIT = 250;          // MQ2 analog limit

int left1 = 25;
  int right1 = 100;
  int left2 = 50;
  int right2 = 200;
void setup()
{
  Serial.begin(9600);

  pinMode(FLAME_PIN, INPUT);

  sensors.begin();

  Serial.println("================================");
  Serial.println("   EV BATTERY SAFETY MONITOR");
  Serial.println("================================");

  lcd.init();
  lcd.backlight();

  delay(2000);
}

void loop()
{
  bool batterySafe = true;

  // =================================================
  // 1. CHECK BATTERY CURRENT FIRST
  // =================================================

  int currentRaw = analogRead(ACS712_PIN);

  float voltage = (currentRaw * Vref) / 1023.0;

  // ACS712 zero-current output is approximately 2.5V
  float current = (voltage - 2.5) / sensitivity;

  // Make negative current positive for monitoring
  if (current < 0)
  {
    current = -current;
  }

  Serial.println();
  Serial.println("===== BATTERY CHECK =====");

  Serial.print("Battery Current: ");
  Serial.print(current);
  Serial.print(" A");

  if (current > MAX_CURRENT)
  {
    Serial.println(" -> CURRENT ERROR!");
    batterySafe = false;
  }
  else
  {
    Serial.println(" -> OK");
  }

  // =================================================
  // 2. CHECK MQ-2 GAS SENSOR
  // =================================================

  int gasValue = analogRead(MQ2_PIN);

  Serial.print("MQ-2 Gas Value: ");
  Serial.print(gasValue);

  if (gasValue > GAS_LIMIT)
  {
    Serial.println(" -> GAS ERROR!");
    batterySafe = false;
  }
  else
  {
    Serial.println(" -> OK");
  }

  // =================================================
  // 3. CHECK FLAME SENSOR
  // =================================================

  int flameState = digitalRead(FLAME_PIN);

  Serial.print("Flame Sensor: ");

  // Most flame modules give LOW when flame is detected
  if (flameState == LOW)
  {
    Serial.println("FIRE DETECTED -> FIRE ERROR!");
    batterySafe = false;
  }
  else
  {
    Serial.println("NO FIRE -> OK");
  }

  // =================================================
  // 4. CHECK BATTERY TEMPERATURE
  // =================================================

  sensors.requestTemperatures();

  float temperature = sensors.getTempCByIndex(0);

  Serial.print("Battery Temperature: ");

  // DS18S20 communication error
  if (temperature == DEVICE_DISCONNECTED_C)
  {
    Serial.println("SENSOR ERROR!");
    
    batterySafe = false;
  }
  else
  {
    Serial.print(temperature);
    Serial.print(" °C");

    if (temperature > MAX_TEMP)
    {
      Serial.println(" -> TEMPERATURE ERROR!");
      batterySafe = false;
      fam = "fire alert";
    }
    else
    {
      Serial.println(" -> OK");
      fam = "no fire";
    }
  }

  // =================================================
  // FINAL BATTERY STATUS
  // =================================================

  Serial.println("--------------------------------");

  if (batterySafe)
  {
    Serial.println("BATTERY STATUS: SAFE");
  }
  else
  {
    Serial.println("BATTERY STATUS: DANGER!");
  }

  Serial.println("--------------------------------");
// ================= PAGE 1 =================
lcd.clear();

lcd.setCursor(0, 0);
lcd.print("CUR:");
lcd.print(current);

lcd.setCursor(9, 0);
lcd.print("GAS:");
lcd.print(gasValue);

lcd.setCursor(0, 1);
lcd.print("FAM:");
lcd.print(fam);

lcd.setCursor(9, 1);
lcd.print("T:");
lcd.print(temperature);

delay(3000);


// ================= PAGE 2 =================
lcd.clear();

lcd.setCursor(0, 0);
if (batterySafe == true) {
  lcd.print("BATTERY STATUS:");
  lcd.setCursor(0, 1);
  lcd.print("  SAFE  ");
} else {
  lcd.print("BATTERY STATUS:");
  lcd.setCursor(0, 1);
  lcd.print("   DANGER!  ");
}

delay(3000);
  
}
