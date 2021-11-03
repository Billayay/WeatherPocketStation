
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Wire.h>
#include "Adafruit_VEML6075.h"



#define DHTPIN 4
#define DHTTYPE DHT22

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);






//setting up data types
volatile float hum;
volatile float tempC;
volatile float tempF;
volatile float hif;
volatile float hic;
volatile float UVI;
volatile float UVA;
volatile float UVB;
volatile int sampleTime;

//setting up objects
Adafruit_VEML6075 uv = Adafruit_VEML6075();


void setup() {
  // set up the LCD's number of columns and rows:
 lcd.begin(16, 4);
 
  //starts DHT11 or DHT 22
  Serial.begin(9600);
  dht.begin();
   if (! uv.begin()) {
    Serial.println("Failed to communicate with VEML6075 sensor.");
  }
  Serial.println("Found the VEML6075 sensor");
}
  


void throws_Execption_Temp() {

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
   throws_Execption_Temp();
  
}

//Functions reads the UV Index, Ultraviolet A, Ultraviolet B
void UVindex() {
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
  UVindex();
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 0);
 //lcd.print("Humidity: ");
 //lcd.print(hum);

 lcd.setCursor(0,1);
 lcd.print("Temp (F) : ");
 lcd.print(tempF);

 lcd.setCursor(0,0);
 lcd.print("UVB Index: ");
 lcd.print (UVB);




}
