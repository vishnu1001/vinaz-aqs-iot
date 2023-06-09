//###############################################################################################################################################
// VINAZ-AQI (Nano)
//###############################################################################################################################################
// Nano + Nova PM Sensor + Optical Dust Sensor
//###############################################################################################################################################

#include <SDS011.h>

// OLED Display
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//###############################################################################################################################################

SDS011 nova;

////////////////////////////////////////////////////////////////////////////////
//--------------------
// Buzzer
int buzzer = 9;
char button_pin = 8;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Dust Sensor Define:

#define measurePin 0 // Analog Pin
#define ledPower 4   // Digital Pin

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

float p10, p25;
int error;

void setup(){

  Serial.begin(9600);
  nova.begin(3, 2); //RX, TX // TX, RX
  pinMode(buzzer, OUTPUT);

  pinMode(ledPower,OUTPUT);

  // OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

}

void loop(){

    

////////////////////////////////////////////////////////////////////////////////
// Nova PM Readings:

  error = nova.read(&p25, &p10);
	if (!error) {
		Serial.print(p25);
    Serial.print(",");
		Serial.print(p10);
	}

  Serial.print(",");



////////////////////////////////////////////////////////////////////////////////
// Optical Dust Sensor:

  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin); // read the dust value
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);
  dustDensity = 170 * calcVoltage - 0.1;
  Serial.println(dustDensity); // unit: ug/m3

  alertCode(dustDensity);


  // OLED Display

  // PM Sensor Display

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 3);
  display.println("PM2.5");

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35, 0);
  display.println(p25);

  display.setTextSize(1);
  display.setCursor(100, 3);
  display.println("ppm");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 23);
  display.println("PM10");

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35, 20);
  display.println(p10);

  display.setTextSize(1);
  display.setCursor(100, 23);
  display.println("ppm");

  // Dust Sensor Display
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 50);
  display.println(dustDensity);
  display.setTextSize(1);
  display.setCursor(90,55);
  display.println("ug/m3");

  display.display();

  display.clearDisplay();
  
  delay(5000); // Time Delay (ms)

}

void alertCode(float value)
{  

  if(value>0 && value<50)
  {
    Serial.println("> GOOD");
    //display.println("GOOD");
  }
  else if(value>51 && value<100)
  {
    Serial.println("> Moderate");
    //display.println("Moderate");
  }
  else if(value>101 && value<150)
  {
    //buzzerAlert();
    Serial.println("> Unhealthy");
    //display.println("Unhealthy");
  }
  else if(value>151 && value<200)
  {
    buzzerAlert();
    Serial.println("> Unhealthy");
    //display.println("Unhealthy");
  }
  else if(value>201 && value<300)
  {
    buzzerAlert();
    Serial.println("> Very Unhealthy");
    //display.println("Very Unhealthy");
  }
  else
  {
    //buzzerAlert();
    Serial.println("> Hazardous");
    //display.println("Hazardous");
  }

}


void buzzerAlert()
{
  // beep sound freq
  tone(buzzer, 392);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  noTone(buzzer);

  // 2 beep delay
  delay(30);

  // beep sound freq
  tone(buzzer, 392);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  noTone(buzzer);

  // delay after 2 beep
  delay(5000);
}

/*

https://www.airnow.gov/aqi/aqi-basics/

AQI Basics for Ozone and Particle Pollution

Good           | 0-50       | Green
Moderate       | 51-100     | Yellow
Unhealthy      | 101-150    | Orange
Unhealthy      | 151-200    | Red
Very Unhealthy | 201-300    | Purple
Hazardous      | 301-higher | Maroon

*/
