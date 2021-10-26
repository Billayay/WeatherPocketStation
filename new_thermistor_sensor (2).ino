#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           

#include <math.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
#include "DHT.h"

// which analog pin to connect for Thermistor
#define THERMISTORPIN A0         
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// the value of the 'other' resistor
#define SERIESRESISTOR 10000

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE); //Temperature and humidity sensor

#define analogPinForRV    1   // change to pins you the analog pins are using for the wind sensor
#define analogPinForTMP   2   // for the wind sensor

const int chipSelect = 10; //Declare Pin 10 for SD Card Module

File myFile; //Declare filename for SD Card

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Init the DS3231 using the hardware interface for the clock
DS3231  rtc(SDA, SCL);

/*
I/O define
*/
const int iled = 7;                                            //drive the led of sensor D7
const int vout = 0;                                            //analog input A0

/*
variable
*/
float density, voltage;
int   adcvalue;

/*
private function
*/
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

// to calibrate your sensor, put a glass over it, but the sensor should not be
// touching the desktop surface however.
// adjust the zeroWindAdjustment until your sensor reads about zero with the glass over it. 

const float zeroWindAdjustment =  -0.01; // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor 
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;

double Thermistor(int RawADC) {
 double Temp;
 Temp = log(10000.0*((1024.0/RawADC-1)));
  //Temp       =log(10000.0/(1024.0/RawADC-1)); // for pull-up configuration
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;            // Convert Kelvin to Celcius
 Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
 return Temp;
}

void setup() 
{
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);                                     //iled default closed
  
  Serial.begin(115200);                                         //send and receive at 115200 baud
 Serial.print("Initializing SD card...");
 // For Mega boards with an Ethernet shield, make sure the Wiznet
 // chip is not selected:
 pinMode(10, OUTPUT);
 digitalWrite(10, HIGH); // davekw7x: If it's low, the Wiznet chip corrupts the SPI bus
 
 // see if the card is present and can be initialized:
 if (!SD.begin(chipSelect)) {
   Serial.println("Card failed, or not present");
   // don't do anything more:
   return;
 }
 Serial.println("card initialized.");

  //Serial.print("*********************************** WaveShare ***********************************\n");
  //Serial.println("Time,Temp,PM2.5,RH");


   //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);        // GND pin      
  pinMode(A3, INPUT);        // VCC pin
  digitalWrite(A3, LOW);     // turn off pullups

  lcd.begin();  //initialize the lcd
  lcd.backlight();  //open the backlight 

  // Initialize the rtc object
  rtc.begin();

//  // The following lines can be uncommented to set the date and time
//  rtc.setDOW(TUESDAY);     // Set Day-of-Week to SUNDAY
//  rtc.setTime(18, 25, 30);     // Set the time to 12:00:00 (24hr format)
//  rtc.setDate(20, 06, 2017);   // Set the date to January 1st, 2014

      Serial.println("DHTxx test!");

  dht.begin();

}

void loop() {
 Serial.println(int(Thermistor(analogRead(0))));  // display Fahrenheit
 delay(100);

  /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

   TMP_Therm_ADunits = analogRead(analogPinForTMP);
    RV_Wind_ADunits = analogRead(analogPinForRV);
    RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);

    // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
    // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
    TempCtimes100 = (0.005 *((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;  

    zeroWind_ADunits = -0.0006*((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  

    // This from a regression from data in the form of 
    // Vraw = V0 + b * WindSpeed ^ c
    // V0 is zero wind at a particular temperature
    // The constants b and c were determined by some Excel wrangling with the solver.
    
   WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) /.2300) , 2.7265);   

  /*
  display the result
  */
  Serial.print(rtc.getDateStr());
  Serial.print(",");
  Serial.print(rtc.getTimeStr());
  Serial.print(",");
//  Serial.print("The current dust concentration is: ");
  Serial.print(density);
//  Serial.print(" ug/m3\n"); 
  Serial.print(",");
  Serial.print(h);
  Serial.print(",");
  //Serial.print(" %\t");
  //Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(",");
  //Serial.print(" *C ");
//  Serial.print(f);
//  Serial.print(" *F\t");
  //Serial.print("Heat index: ");
  Serial.println(hic);
  //Serial.print(" *C ");
//  Serial.print(hif);
//  Serial.println(" *F");

    Serial.print((float)WindSpeed_MPH);
    Serial.print(" mph ");
    

    Serial.print(", ");Serial.print((float)WindSpeed_MPH*1609/3600);
    Serial.println(" m/s ");


//Set up space for your variables Temp, RH, PM, Thermistor Temperature
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PM="); 
      lcd.setCursor(4, 0);
      lcd.print(density); 
      lcd.setCursor(0, 1);
      lcd.print("T="); lcd.print(t,1); 
//      lcd.setCursor(8, 1);
//      lcd.print("RH="); lcd.print(h,1);
      lcd.setCursor(7,1);
      lcd.print("TMRT=");lcd.print(int(Thermistor(analogRead(0))));

//      lcd.setCursor(0, 0);
//      lcd.print(T,2);//print the temperature on lcd1602
//      lcd.setCursor(6, 0);      
//      lcd.print(P,2);//print the temperature on lcd1602
//      lcd.setCursor(0, 1);
//      lcd.print((float)WindSpeed_MPH*1609/3600,2);//print the temperature on lcd1602
//      lcd.setCursor(7, 1);
//      lcd.print(rtc.getTimeStr());//print the temperature on lcd1602
//

 
  myFile = SD.open("AAD6.txt", FILE_WRITE);
  if (myFile) {    
    myFile.print(rtc.getDateStr());
    myFile.print(",");
    myFile.print(rtc.getTimeStr());
    myFile.print(",");
    myFile.print(density);
    myFile.print(",");
    myFile.print(h);
    myFile.print(",");
    myFile.print(t);
    myFile.print(",");
    myFile.println(hic);
    myFile.close(); // close the file
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening AAD6.txt");
  }   
    
  // 2-s resolution :)
   delay(5000);
}

