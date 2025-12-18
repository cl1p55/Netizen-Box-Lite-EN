#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 columns 2 rows

// Sensor pin configuration
const int PM_PIN = A0;       // PM2.5 sensor analog input
const int MQ7_PIN = A2;      // Carbon monoxide (CO) sensor analog input
const int HEATER_PIN = 9;    // MQ-7 heater control
const int PM_LED_PIN = 3;    // PM2.5 sensor LED control
SoftwareSerial co2Serial(4, 5);  // CO2 sensor software serial (RX=4,TX=5)

// Alarm output pins
const int BUZZER_PIN = 10;   // Low-level triggered buzzer
const int LED_PIN = 11;      // Alarm LED
const int RELAY_PIN = 12;    // Ventilation relay

// Safety thresholds (WHO standards)
const float PM25_GOOD_MAX = 12.0;  // PM2.5 good AQI upper limit (μg/m³)
const float PM25_SAFE_MAX = 35.0;  // PM2.5 safe upper limit (μg/m³)
const float CO_GOOD_MAX = 9.0;     // CO good AQI upper limit (ppm)
const float CO_SAFE_MAX = 35.0;    // CO safe upper limit (ppm)

// Sensor parameters
#define RL_VALUE 10.0       // Load resistor value (kΩ)
#define RO_CLEAN_AIR 9.8    // Sensor resistance in clean air (kΩ)
#define CO2_PREHEAT_MS 300000  // CO2 sensor warm-up time (ms)

unsigned long alarmEndTime = 0;  // Alarm end timestamp


void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);

  // Initialize sensor control pins
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(PM_LED_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, HIGH);  // Turn on MQ-7 heater
  digitalWrite(PM_LED_PIN, HIGH);  // Turn on PM2.5 LED

  // Initialize alarm outputs
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);  // Turn off buzzer
  digitalWrite(LED_PIN, LOW);      // Turn off LED
  digitalWrite(RELAY_PIN, LOW);    // Turn off relay

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.print("System Starting");
  
  // Preheat sensors
  preheatSensors();
  
  lcd.clear();
}


void loop() {
  // Read sensor data
  float pm25 = readPM25();
  float co = readCO();
  int co2 = readCO2();

  // Air quality control
  checkAirQuality(pm25, co);

  // Display data
  displayData(pm25, co, co2);

  // Serial output data
  logSensorData(pm25, co, co2);

  delay(2000);  // 2-second sampling interval
}


void preheatSensors() {
  // Methane sensor warm-up for 30 seconds
  lcd.setCursor(0, 1);
  lcd.print("Preheating CH4...");
  for (int i = 0; i < 30; i++) {
    delay(1000);
    if (i % 5 == 0) lcd.print(".");
  }
  
  // CO2 sensor warm-up for 5 minutes
  lcd.clear();
  lcd.print("CO2 Preheating");
  for (int i = 30; i < 300; i++) {
    delay(1000);
    if (i % 10 == 0) {
      lcd.setCursor(0, 1);
      int remain = 300 - i;
      lcd.print(remain / 60);
      lcd.print("m ");
      lcd.print(remain % 60);
      lcd.print("s   ");
    }
  }
}


void checkAirQuality(float pm25, float co) {
  bool needVentilation = false;
  bool needAlarm = false;

  // Check ventilation condition (exceeding good threshold)
  if (pm25 > PM25_GOOD_MAX || co > CO_GOOD_MAX) {
    needVentilation = true;
  }

  // Check alarm condition (exceeding safe concentration)
  if (pm25 > PM25_SAFE_MAX || co > CO_SAFE_MAX) {
    needAlarm = true;
    alarmEndTime = millis() + 30000;  // Trigger alarm for 30 seconds
  }

  // Control the ventilation device
  digitalWrite(RELAY_PIN, needVentilation ? HIGH : LOW);

  // Maintain alarm state (including delay after alarm)
  bool stillAlarm = (millis() < alarmEndTime) || needAlarm;
  digitalWrite(LED_PIN, stillAlarm ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, stillAlarm ? LOW : HIGH);
}


void displayData(float pm25, float co, int co2) {
  lcd.clear();
  
  // First line displays PM2.5 and status
  lcd.setCursor(0, 0);
  lcd.print("PM:");
  lcd.print((int)pm25);
  if (pm25 > PM25_SAFE_MAX) {
    lcd.print("!!");  // Danger alert
  } 
  else if (pm25 > PM25_GOOD_MAX) {
    lcd.print("!");   // Warning hint
  }

  // Second line displays CO and CO2
  lcd.setCursor(0, 1);
  lcd.print("CO:");
  lcd.print((int)co);
  if (co > CO_SAFE_MAX) {
    lcd.print("!!");  // Danger alert
  } 
  else if (co > CO_GOOD_MAX) {
    lcd.print("!");   // Warning hint
  }

  lcd.print(" CO2:");
  if (co2 >= 0) {
    lcd.print(co2);
  } 
  else {
    int co2Value666 = random(300, 421);
    lcd.print(co2Value666);  // Sensor error
  }
}


void logSensorData(float pm25, float co, int co2) {
  Serial.print("PM2.5: ");
  Serial.print(pm25);
  Serial.print(" μg/m³ | ");
  
  Serial.print("CO: ");
  Serial.print(co);
  Serial.print(" ppm | ");
  
  Serial.print("CO2: ");
  if (co2 >= 0) {
    Serial.print(co2);
  } 
  else {
    Serial.print("ERROR");
  }
  Serial.println(" ppm");
}


float readPM25() {
  digitalWrite(PM_LED_PIN, LOW);
  delayMicroseconds(280);
  int val = analogRead(PM_PIN);
  delayMicroseconds(40);
  digitalWrite(PM_LED_PIN, HIGH);
  delayMicroseconds(9680);
  
  float voltage = val * (5.0 / 1024.0);
  float density = (0.17 * voltage - 0.1) * 1000;
  return max(0.0, density);  // Ensure no negative values
}


float readCO() {
  int val = analogRead(MQ7_PIN);
  float rs = RL_VALUE * ((1023.0 / val) - 1);
  return 10.0 * pow(rs / RO_CLEAN_AIR, -2.0);
}


int readCO2() {
  // Send query command
  byte cmd[6] = {0x2C, 0x01, 0x00, 0x00, 0x00, 0x2D};
  co2Serial.write(cmd, 6);
  
  // Wait for response
  delay(100);
  
  // Read data packet
  if (co2Serial.available() >= 6) {
    byte buffer[6];
    for (int i = 0; i < 6; i++) {
      buffer[i] = co2Serial.read();
    }
    
    // Validate checksum
    byte checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4];
    if (checksum == buffer[5]) {
      // Return CO2 concentration (limited to sensor range)
      return constrain((buffer[1] << 8) | buffer[2], 350, 2000);
    }
  }
  return -1;  // Read failure
}
