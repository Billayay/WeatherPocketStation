#include <config.h>

#include <LiquidCrystal.h>
#include <DHT.h>
#include <Wire.h>
#include <SD.h>
#include "RTClib.h"
#include "Adafruit_VEML6075.h"

#define DHTPIN 4
#define DHTTYPE DHT22

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;

//setting up data types
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
volatile float hum;
volatile float tempC;
volatile float tempF;
volatile float hif;
volatile float hic;
volatile float UVI;
volatile float UVA;
volatile float UVB;
volatile int sampleTime;
const int contrast = 75;

//setting up objects
Adafruit_VEML6075 uv = Adafruit_VEML6075();


void setup() {
  Serial.begin(115200);
    Serial.begin(9600);
    Wire.begin();
     
  // set up the LCD's number of columns and rows:
  analogWrite(6,contrast);
 lcd.begin(16, 4);
  
  //starts DHT11 or DHT 22

  dht.begin();
   if (! uv.begin()) {
    Serial.println("Failed to communicate with VEML6075 sensor.");
  }
  Serial.println("Found the VEML6075 sensor");



while (!Serial) {
//SETTING UP RTC MODULE
  
  
; // wait for serial port to connect. Needed for native USB port only
}

  

}
  


void throws_Execption_DHT() {

   // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(tempC) || isnan(tempF)) {
    lcd.println(F("Failed to read from DHT sensor!"));
    return;
  }
}

//Reads temperature and humidity can be adjusted
void DHT22Reader() {
  
  delay(2000);
  //humidity
   hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
   tempC = dht.readTemperature(true);
  // Read temperature as Fahrenheit (isFahrenheit = true)
   tempF = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
   hif = dht.computeHeatIndex(tempF, hum);
  // Compute heat index in Celsius (isFahreheit = false)
   hic = dht.computeHeatIndex(tempC, hum, false);
   throws_Execption_DHT();
  
}


void UV_index() {
   //reads UV Index
   UVI = uv.readUVI();
   //reads UV A 
   UVA = uv.readUVA();
   //reads UV B
   UVB = uv.readUVB();
    delay(1000);
  
}

void loop() {
  DHT22Reader();
  UV_index();
 lcd.setCursor(0,1);
 lcd.print("Temp (F) : ");
 lcd.print(tempF);
 lcd.setCursor(0,0);
 lcd.print("UVA  Index: ");
 lcd.print (UVA);

  DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    delay(3000);
}
