#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Wire.h>
#include <SPI.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTTYPE DHT22
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

//DHT Pin
const int DHTPin = 2;
const long readDelay = 3000;
float Temperature = 999, Humidity = 999;
String TempText = "Temperature";
String HumText = "Humidity";
String moistureSen = "Moisture Sensor: ";
int soilReadings[2] = {0,0};
const int selectPins[3] = {13,14};   // S0 = 13 S1 = 14
int muxChannel[2][3]={
  {0,0}, //channel 0
  {1,0}, //channel 1
  // {0,1,0}, //channel 2
  // {1,1,0}, //channel 3
  // {0,0,1}, //channel 4
  // {1,0,1}, //channel 5
  // {0,1,1}, //channel 6
  // {1,1,1}, //channel 7
};
const int commonInput = 17;  

DHT dht(DHTPin, DHTTYPE);  // Initialize DHT sensor.
         
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Initialize OLED
void setup() 
{
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
  Serial.println("SSD1306 allocation failed");
  for(;;); // Don't proceed, loop forever
}
  delay(3000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //Set channel select pins as output
  for (int i=0; i<3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
  }
  //Set DHT22 pin as INPUT
  pinMode(DHTPin, INPUT);
  //set common input pin as input
  pinMode(commonInput, INPUT_PULLUP);
  dht.begin(); 
  Serial.println("---Soil Moniting System 1.0---");
  Serial.println("System Booted");
}

void loop() 
{
  Serial.println("Reading Values");
  Temperature = dht.readTemperature(true); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  
  for (byte pin=0; pin<=1; pin++) //Read soil sensor and place in array
  {
    soilReadings[pin] = readMux(pin);
  }
  Serial.println(TempText + ": " + Temperature);
  Serial.println(HumText + ": " + Humidity);
  for (int i = 0; i <=1; i++)
  {
    int sensor = i + 1;
    Serial.println(moistureSen + " # " + sensor + ": " + soilReadings[i]);
  }
  int seconds = readDelay / 1000;
  Serial.print("End Reading. Waiting ");
  Serial.print(seconds);
  Serial.println(" seconds.");
  Serial.println("-------------");
  
  display.clearDisplay();
  display.setCursor(10,10);
  display.println(TempText + ": " + Temperature);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(10,10);
  display.println(HumText + ": " + Humidity);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(10,10);
  for (int i = 0; i <=1; i++)
  {
    int sensor = i + 1;
    Serial.println(moistureSen + " # " + sensor + ": " + soilReadings[i]);
    
    display.println(moistureSen + ":" + sensor + "\n" + soilReadings[i]);
    display.display();
    display.setCursor(10,10);
    display.clearDisplay();
    delay(2000);
  }
  display.clearDisplay();
  display.setCursor(10,10);
  display.println("Waiting for next sensor reading...");
  display.display();
  delay(readDelay);

}

int readMux(int channel)
{

  //loop through the 3 sig
  for(int i = 0; i < 2; i ++){
    digitalWrite(selectPins[i], muxChannel[channel][i]);
  }
  //read the value at the Z pin
  int val = analogRead(commonInput);
  return val;
}
