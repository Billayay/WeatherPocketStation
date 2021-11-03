
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Wire.h>
#include <SD.h>
#include "Adafruit_VEML6075.h"




#define DHTPIN 4
#define DHTTYPE DHT22

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);
File my_File;

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
const int contrast = 75;

//setting up objects
Adafruit_VEML6075 uv = Adafruit_VEML6075();


void setup() {
  // set up the LCD's number of columns and rows:
  analogWrite(6,contrast);
 lcd.begin(16, 4);
 
  //starts DHT11 or DHT 22
  Serial.begin(9600);
  dht.begin();
   if (! uv.begin()) {
    Serial.println("Failed to communicate with VEML6075 sensor.");
  }
  Serial.println("Found the VEML6075 sensor");


while (!Serial) {
; // wait for serial port to connect. Needed for native USB port only
}
Serial.print("Initializing SD card...");
if (!SD.begin(10)) {
Serial.println("initialization failed!");
while (1);
}
Serial.println("initialization done.");
// open the file. note that only one file can be open at a time,
// so you have to close this one before opening another.
my_File = SD.open("test.txt", FILE_WRITE);
// if the file opened okay, write to it:
if (my_File) {
Serial.print("Writing to test.txt...");
my_File.println("This is a test file :)");
my_File.println("testing 1, 2, 3.");
for (int i = 0; i < 20; i++) {
my_File.println(i);
}
// close the file:
my_File.close();
Serial.println("done.");
} else {
// if the file didn't open, print an error:
Serial.println("error opening test.txt");
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




}
