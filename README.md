# WeatherPocketStation
This is the Github page for University of Oklahoma Capstone Class ECE 3773.
We are improving design and userable for a protype given to us by our Capstone Sponsor.


## Materials

Compoments | Usage | 
------------ | -------------
Arduino Mega 2650 | Center Unit
DHT 22 | Temperature and Humidity
RTC | Clock
WindSpeed & Direction | Windspeed and Direction
LCD Display Shield | Digital Display
UV sensor | Sensor
SD card | data storage
Thermistor | Temperature, but use for radation Math is incomplete  

The main source of the code is ran off from the main.ino. Thia main file contains all of the working coding for our project. It has the following code features: RTC, windspeed, SD card, DHT 22, LCD display, and UV sensor.

##File Header used
- Wire.h
- LiquidCrystal.h
- DHT.h
- SD.h
- RTClib.h
- math.h

### RTC
The RTC module is DS3221. It uses rtclib.h file. This is calbrated to MM/DD/YYYY with HH:MM:SS. 

#### WindSpeed and Direction
Dave's anemometer is our windspeed and direction sensor. In our main file we have the math and conversion for each cardinal direction.

### LCD Display Shield
The LCD display is used as a menu and display. The Menu has opitons of Temperature, Humidity, UV, and Windspeed and Dirction.
Each of the push buttons correlates to scroll up, down, left, right. Specifically the righty push button select which menu. 
All of the given sensors data are displayed under each sub menu.

### UV Sensor
This sensors calcuates the UV index by using the voltage in a specifc formula. You can modify the voltage for 5V or 3.3V to get an accurate reading. 

### SD Card
To store our data.

### Thermistor
Suppose to be used as a radiation measurement tool, but with the lack of math and guidance we can only find the temperature.

## Credits 
- RTC, SD card, DHT Adafruit/Arduino CC Create example codes.
- LCD display with keybutton - https://www.instructables.com/Arduino-Uno-Menu-Template/
- Windspeed and Direction - http://cactus.io/hookups/weather/anemometer/davis/hookup-arduino-to-davis-anemometer
- UV sensor - http://arduinolearning.com/code/arduino-guva-s12sd-uv-sensor.php
- RTClib - https://github.com/adafruit/RTClib

