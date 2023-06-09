//###############################################################################################################################################
// VINAZ-AQI (ESP32)
//###############################################################################################################################################
// EPS + MQ7 + MQ135 + DHT11
//###############################################################################################################################################

// All Sensors Test

#include <MQUnifiedsensor.h>
#include <DHT.h>

//###############################################################################################################################################

// WiFi Library
#include <WiFi.h>
#include <HTTPClient.h>

//###############################################################################################################################################

#define   Board                   "ESP32"
#define   Type2                   "MQ-7"
#define   Type3                   "MQ-135"
#define   mq7_pin                 34
#define   mq135_pin               35
#define   Voltage_Resolution      5
#define   ADC_Bit_Resolution      12   // For arduino UNO/MEGA/NANO
#define   RatioMQ7CleanAir        27.5 //RS / R0 = 27.5 ppm 
#define   RatioMQ135CleanAir      3.6  //RS / R0 = 3.6 ppm

#define   PWMPin                  5    // Pin connected to mosfet

//###############################################################################################################################################

#define DHTPIN 32
#define DHTTYPE DHT11

//###############################################################################################################################################

//https://script.google.com/macros/s/AKfycbxGG3vksmgDmlTZHGnnmt658tuevY36Y-e4rZevzkfLTOXbpgVdTzHLY8S7GxzsIX5K/exec

const char* ssid = "VINAZ-AQS";
const char* password = "vinazaqs99$";

String scriptUrl = "https://script.google.com/macros/s/AKfycbxGG3vksmgDmlTZHGnnmt658tuevY36Y-e4rZevzkfLTOXbpgVdTzHLY8S7GxzsIX5K/exec";

//###############################################################################################################################################

MQUnifiedsensor MQ7  (Board, Voltage_Resolution, ADC_Bit_Resolution, mq7_pin, Type2);
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, mq135_pin, Type3);

// DHT11 //------------------------------------------------------

DHT dht(DHTPIN, DHTTYPE);

// MQ-7 //-------------------------------------------------------

float mq7_a3 = 99.042, mq7_b3 = -1.518;               //CO

// MQ-135 //-----------------------------------------------------

float mq135_a = 110.47, mq135_b = -2.862;             //CO2
float mq135_a1 = 102.2, mq135_b1 = -2.473;            //NH4

//###############################################################################################################################################

void setup() 
{
  Serial.begin(115200);

//--------------------------------------------------------------
// WiFi Settings

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(">>> Connected: ");
  Serial.println(WiFi.localIP());

//--------------------------------------------------------------

  dht.begin();

  pinMode(PWMPin, OUTPUT);

  MQ7.setRegressionMethod(1);   //_PPM =  a*ratio^b
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  float calcR0_2 = 0;
  float calcR0_3 = 0;

  for(int i = 1; i<=10; i ++)
  {
    MQ7.update();
    MQ135.update();

    calcR0_2 += MQ7.calibrate(RatioMQ7CleanAir);
    calcR0_3 += MQ135.calibrate(RatioMQ135CleanAir);

    Serial.print(".");
  }

  MQ7.setR0(calcR0_2/10);
  MQ135.setR0(calcR0_3/10);
  
  if(isinf(calcR0_2) || isinf(calcR0_3))
  {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); 
    while(1);
  }
  if(calcR0_2 == 0 || calcR0_3 == 0)
  {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); 
    while(1);
  }

  Serial.print("\n");
}

//###############################################################################################################################################

void loop()
{
  Serial.println();

//--------------------------------------------------------------
// DHT11

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Temperature: "); 
  Serial.print(temperature); Serial.print("\xC2\xB0 C");
  Serial.print(" | Humidity: ");
  Serial.print(humidity); Serial.print(" %");

  String val1 = "?temp=" + String(temperature); // Column C | rowData[2] 
  String val2 = "&humid=" + String(humidity);   // Column D | rowData[3] 

//--------------------------------------------------------------

  mq7_gas(mq7_a3, mq7_b3);
  Serial.print(" | CO2: ");
  String val3 = "&co2=" + String(MQ7.serialDebug()); // Column E | rowData[4] 
  Serial.print(" ppm");

//--------------------------------------------------------------

  mq135_gas(mq135_a, mq135_b);
  Serial.print(" | CO: ");
  String val4 = "&co=" + String(MQ135.serialDebug()); // Column F | rowData[5] 
  Serial.print("ppm");
  
//--------------------------------------------------------------

  mq135_gas(mq135_a1, mq135_b1);
  Serial.print(" | NH4: ");
  String val5 = "&nh4=" + String(MQ135.serialDebug()); // Column G | rowData[6] 
  Serial.print("ppm");
  
//--------------------------------------------------------------

  Serial.print("\n");

//--------------------------------------------------------------
// HTTP Client Communication:

  HTTPClient http;

//https://script.google.com/macros/s/AKfycbxGG3vksmgDmlTZHGnnmt658tuevY36Y-e4rZevzkfLTOXbpgVdTzHLY8S7GxzsIX5K/exec
  String url = scriptUrl + val1 + val2 + val3 + val4 + val5;
  Serial.println(url);
  
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

//--------------------------------------------------------------

  //delay(300000); // 5min
  delay(10000);    // 10sec
}

//###############################################################################################################################################

// Functions:
//--------------------------------------------------------------
// Temperature and Humidity Sensor (C %)
void tempSensor()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Temperature: "); 
  Serial.print(temperature); Serial.print("\xC2\xB0 C");
  Serial.print(" | Humidity: ");
  Serial.print(humidity); Serial.println(" %");

  String val4 = "&temp=" + String(temperature);
  String val5 = "&humid=" + String(humidity);
}

//--------------------------------------------------------------
// MQ-7 Gas Sensor (ppm)
void mq7_gas(float a, float b)
{
  MQ7.setA(a); MQ7.setB(b);
  MQ7.init();
  analogWrite(5, 255); // 255 is DC 5V output
  MQ7.update();
  MQ7.readSensor();
  MQ7.serialDebug(true);
}

//--------------------------------------------------------------
// MQ-135 Gas Sensor (ppm)
void mq135_gas(float a, float b)
{
  MQ135.setA(a); MQ135.setB(b);
  MQ135.init();
  MQ135.update();
  MQ135.readSensor();
  MQ135.serialDebug(true);  
}

//###############################################################################################################################################
















