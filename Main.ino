//setting up data types
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Menu Names
String menuItems[] = {"Temperature", "Humdity & UVI", "WindSpeed&Direction", "Time", "Credits", "About"};

// Navigation button variables
int readKey;

// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;

// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};

//DHT22 values
volatile float hum;
volatile float tempC;
volatile float tempF;
volatile float hif;
volatile float hic;
//UV
float sensorVoltage; 
float sensorValue;
int VaneValue;      // raw analog value from wind vane 
int Direction;      // translated 0 - 360 direction 
int CalDirection;   // converted value with offset applied 
int LastValue;      // last direction value
String dir;

//Thermos values
int ThermistorPin = 0;
int Vo;
float R1 = 100000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;


volatile unsigned long Rotations; // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in interrupt routine
float WindSpeed; // speed miles per hour

#include <Wire.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include "RTClib.h"
#include <math.h>

#define WindSensorPin (2) // The pin location of the anemometer sensor
#define Offset 0; 
#define DHTPIN 33
#define DHTTYPE DHT22

// Setting the LCD shields pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
RTC_DS3231 rtc;
DHT dht(DHTPIN, DHTTYPE);


void setup() {

  // Initializes serial communication
  Serial.begin(9600);
  Serial.begin(57600);
  Serial.println("Vane Value\tDirection\tHeading");
  LastValue = 0;
  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);
  Serial.println("Davis Wind Speed Test");
  Serial.println("Rotations\tMPH");
  
  // Initializes and clears the LCD screen
  lcd.begin(16, 2);
  lcd.clear();

  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  dht.begin();

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  
}

//Reads temperature and humidity can be adjusted
void DHT22Reader() {
  
  delay(2000);
  //humidity
   hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
   tempC = dht.readTemperature(false);
  // Read temperature as Fahrenheit (isFahrenheit = true)
   tempF = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
   hif = dht.computeHeatIndex(tempF, hum);
  // Compute heat index in Celsius (isFahreheit = false)
   hic = dht.computeHeatIndex(tempC, hum, false);
   // throws_Execption_DHT();
  
}

void speedRunner() {
  delay(1000);                                    // wait for a second
  VaneValue = analogRead(A14);                     // Read the analog value from the A/D converter
  Direction = map(VaneValue, 0, 1023, 0, 359);    // Map the A/D value into the compass range
  CalDirection = Direction + Offset;              // Add an offset if the anemometer/windvane arm is not pointing North

  if(CalDirection > 360)                          // Calculate for a calibrated anemometer/windvane arm 
    CalDirection = CalDirection - 360; 
  
  if(CalDirection < 0) 
    CalDirection = CalDirection + 360; 
  
  // Only update the display if change greater than 2 degrees. Otherwise skip until a change is detected.
  if(abs(CalDirection - LastValue) > 2) 
  { 
    Serial.print(VaneValue); Serial.print("\t\t"); 
    Serial.print(CalDirection); Serial.print("\t\t"); 
    getHeading(CalDirection);                     // Get the compass heading to display
    LastValue = CalDirection;                     // Set the LastValue variable to dtermine of direct changed > 2 degrees 
  }
Rotations = 0; // Set Rotations count to 0 ready for calculations

sei(); // Enables interrupts
delay (3000); // Wait 3 seconds to average
cli(); // Disable interrupts

// convert to mp/h using the formula V=P(2.25/T)
// V = P(2.25/3) = P * 0.75

WindSpeed = Rotations * 0.75;

Serial.print(Rotations); Serial.print("\t\t");
Serial.println(WindSpeed);
}

void UVvalues() {
 
  sensorValue = analogRead(A8);
  sensorVoltage = sensorValue/1024*3.3;
  Serial.print("sensor reading = ");
  Serial.print(sensorValue);
  Serial.println("");
  Serial.print("sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  delay(1000);
}

// Converts compass direction to heading 
void getHeading(int direction) { 
  if(direction > 337 and direction <= 22) {
    Serial.println("N"); 
    dir ="N";
  }
  else if (direction > 22 and direction <= 67) {
    Serial.println("NE");
    dir ="NE"; 
  }
  else if (direction > 67 and direction <= 112) {
    Serial.println("E"); 
    dir = "E";
  }
  else if (direction > 112 and direction <= 157) {
    Serial.println("SE"); 
    dir = "SE";
  }
  else if (direction > 157 and direction <= 202) {
     Serial.println("S");
    dir = "S"; 
  }
  else if (direction > 202 and direction <= 247) {
    Serial.println("SW"); 
    dir = "SW";
  }
    
  else if (direction > 247 and direction <= 292) {
     Serial.println("W"); 
    dir = "W";
  }
  else if (direction > 292 and direction <= 337) {
     Serial.println("NW");
    dir = "NW"; 
  }
  else{
     Serial.println("N");
    dir = "N";
  }
   
}

// This is the function that the interrupt calls to increment the rotation count
void isr_rotation () {

if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
Rotations++;
ContactBounceTime = millis();
}

}

void thermo() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc = T - 273.15;
  Tf = (Tc * 9.0)/ 5.0 + 32.0; 

  Serial.print("Temperature: "); 
  Serial.print(Tf);
  Serial.print(" F; ");
  Serial.print(Tc);
  Serial.println(" C");   

  delay(500);
}

void loop() {
  UVvalues();
  mainMenuDraw();
  drawCursor();
  operateMainMenu();

}

// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
          case 4:
            menuItem5();
            break;
          case 5:
            menuItem6();
            break;
          case 6:
            menuItem7();
            break;
          case 7:
            menuItem8();
            break;
          case 8:
            menuItem9();
            break;
          case 9:
            menuItem10();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 380) {
    result = 2; // up
  } else if (x < 555) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  } else if (x < 1000) {
    result = 5; //select
  }
  return result;
}

// If there are common usage instructions on more than 1 of your menu items you can call this function from the sub
// menus to make things a little more simplified. If you don't have common instructions or verbage on multiple menus
// I would just delete this void. You must also delete the drawInstructions()function calls from your sub menu functions.
void drawInstructions() {
  lcd.setCursor(0, 1); // Set cursor to the bottom line
  lcd.print("Use ");
  lcd.print(byte(1)); // Up arrow
  lcd.print("/");
  lcd.print(byte(2)); // Down arrow
  lcd.print(" buttons");
}

void menuItem1() { // Function executes when you select the 1st item from main menu
  int activeButton = 0;
  lcd.clear();  
  while (activeButton == 0) {
    int button;
    DHT22Reader();
    
   lcd.setCursor(0,0);
  lcd.print("Temp C: ");
  lcd.print(tempC);
  lcd.setCursor(0,1);
  lcd.print("Temp F: ");
  lcd.print(tempF);
    
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem2() { // Function executes when you select the 2nd item from main menu
  int activeButton = 0;

  lcd.clear();
  
  while (activeButton == 0) {
    int button;
     DHT22Reader();
    lcd.setCursor(0, 0);
    lcd.print("Humdity: ");
    lcd.print(hum);
    lcd.setCursor(0, 1);
    lcd.print("UVI: ");
    lcd.print(sensorValue);
   
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem3() { // Function executes when you select the 3rd item from main menu
  int activeButton = 0;

  lcd.clear();
 
  
  while (activeButton == 0) {
    int button;
    speedRunner();
     lcd.setCursor(0, 0);
  lcd.print("WindSpeed: ");
  lcd.print(WindSpeed);
  lcd.setCursor(0,1);
  lcd.print("Direction: ");
  lcd.print(dir);
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem4() { // Function executes when you select the 4th item from main menu
  int activeButton = 0;
  
  lcd.clear();

  while (activeButton == 0) {
    int button;
      DateTime now = rtc.now();
      lcd.setCursor(0, 0);
      lcd.print("Time: ");
      lcd.print(now.hour(), DEC);
      lcd.print(':');
      lcd.print(now.minute(), DEC);
      lcd.print(':');
      lcd.print(now.second(), DEC);
      lcd.setCursor(0, 1);
      lcd.print("Date: ");
      lcd.print(now.month(), DEC);
      lcd.print('/');
      lcd.print(now.day(), DEC);
      lcd.print('/');
      lcd.print(now.year(), DEC);
    
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem5() { // Function executes when you select the 5th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("TF, TA");
  lcd.setCursor(2,1);
  lcd.print("NBL, AE, AW");
  

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem6() { // Function executes when you select the 6th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sooner Weather");
  lcd.setCursor(0, 1);
   lcd.print("Pocket Station");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 1000) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem7() { // Function executes when you select the 7th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 7");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem8() { // Function executes when you select the 8th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 8");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem9() { // Function executes when you select the 9th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 9");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem10() { // Function executes when you select the 10th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 10");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
