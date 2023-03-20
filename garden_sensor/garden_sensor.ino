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

//Define Pins
const int selectPins[3] = {13,14};   // S0 = 13 S1 = 14
const int DHTPin = 0; //DHT Pin
const int commonInput = 17; //Analog input from MUX
const int calibratePin = 2;

//Set Options
const long readDelay = 500; //Delay between sensor readings
bool calibrate = false; //Set calibration mode of soil sensors

//Define Strings
String TempText = "Temperature";
String HumText = "Humidity";
String moistureSen = "Moisture Sensor: ";

//Define other variables
int soilReadings[4] = {0,0}; //Array that holds soil sensor readings before printing.

float Temperature = 999, Humidity = 999;
int numberOfSensors = 4;
int muxChannel[4][2]={ //Setups MUX channel pins 4 channels only need two pins
  {0,0}, //channel 0
  {1,0}, //channel 1
  {0,1}, //channel 2
  {1,1}, //channel 3
  // {0,0,1}, //channel 4
  // {1,0,1}, //channel 5
  // {0,1,1}, //channel 6
  // {1,1,1}, //channel 7
};
 
DHT dht(DHTPin, DHTTYPE);  // Initialize DHT sensor.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Initialize OLED

void setup() 
{
  numberOfSensors = numberOfSensors - 1;
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
  Serial.println("SSD1306 allocation failed");
  for(;;); // Don't proceed, loop forever
}
  //Set channel select pins as output
  for (int i=0; i<3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
  }
  pinMode(DHTPin, INPUT);   //Set DHT22 pin as INPUT
  pinMode(commonInput, INPUT_PULLUP);   //set common input pin as input
  pinMode(calibratePin, INPUT);
  delay(3000); //Inital delay for everything to boot up.
  
  //Initial display settings
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  

  dht.begin(); //Start taking temps and humidity readings
  Serial.println("---Soil Moniting System 1.0---");
  Serial.println("System Booted");
}

void loop()
{
  int calibrateCheck = digitalRead(calibratePin);
  Serial.println("Calibrate Check: " + String(calibrateCheck));
  if (calibrateCheck == LOW)
  {
      calibrateSensor(1);
  }
  else 
  {
    Serial.println("Reading Values");
    Temperature = getTemperature();

    Humidity = dht.readHumidity(); // Gets the values of the humidity

    for (byte pin=0; pin<=numberOfSensors; pin++) //Read soil sensors and place in array
    {
      soilReadings[pin] = readMux(pin);
    }
    Serial.println(TempText + ": " + Temperature);
    Serial.println(HumText + ": " + Humidity);
    for (int i = 0; i <=3; i++)
    {
      int sensor = i + 1;
      Serial.println(moistureSen + " # " + sensor + ": " + soilReadings[i]);
    }
    Serial.println("End Reading. Waiting ");
    
    
    updateDisplay("Reading Sensors");
    //display.clearDisplay();
    //display.setCursor(10,10);
    for (int i = 0; i <=3; i++)
    {
      int sensor = i + 1;
      Serial.println("Soil " + String(sensor) + ": " + soilReadings[i]);
      updateDisplay("Soil " + String(sensor) + ": " + soilReadings[i]);
      // display.println(moistureSen + ":" + sensor + "\n" + soilReadings[i]);
      // display.display();
      // display.setCursor(10,10);
      // display.clearDisplay();
      delay(3000);
    }
    updateDisplay("Waiting for next sensor reading.");
    // display.clearDisplay();
    // display.setCursor(10,10);
    // display.println("Waiting for next sensor reading...");
    //display.display();
    delay(readDelay);
    }
}

void updateDisplay(String message)
{
  display.clearDisplay();
  display.setCursor(1,1);
  display.println(String(Temperature) + "F - " + String(Humidity) + "%");
  display.setCursor(10,12);
  display.println("S1-" + String(soilReadings[0]) + " " + "S2-" + String(soilReadings[1]) + "\n" + "S3-" + String(soilReadings[2]) + " " + "S4-" + String(soilReadings[3]));
  display.display();
}

int readMux(int channel)
{
  int i;
  //loop through the 3 sig
  for(i = 0; i < 2; i ++){
    digitalWrite(selectPins[i], muxChannel[channel][i]);
  }
  //read the value at the Z pin

  int val = measureSensor(channel);
  return val;
}

float getTemperature()
{
  float temperature;
  float tempRead;
  for (int i = 0; i <=2; i++)
  {
    float tempRead = dht.readTemperature(true); // Gets the values of the temperature
    temperature = temperature + tempRead;
  }
  float average = temperature / 3;
  return average;
}

int measureSensor(int sensor){
  int total = 0;
  int average;
  delay(1000); //Allow for measure to stabilize
  Serial.println("Reading Sensor #: " + String(sensor + 1));
  for (int i=0;i<=9;i++)
  {

    int val = analogRead(commonInput);
    Serial.println("run " + String(i) + "  " + String(val));
    total = total + val;
    delay(300);
  }
  average = total / 10;
  return average;
  Serial.println("average: " + String(average));
}

void calibrateSensor(int channel){
  for(int i = 0; i < 2; i ++)
  {
    digitalWrite(selectPins[i], muxChannel[channel][i]);
  }
  int total = 0;
  int average;
  for (int i=0;i<=15;i++)
  {
    int val = analogRead(commonInput);
    Serial.println("run " + String(i) + "  " + String(val));
    total = total + val;
    delay(200);
  }
  average = total / 16;
  display.clearDisplay();
  display.setCursor(10,10);
  display.println("AVERAGE: " + String(average));
  display.display();
  Serial.println("average: " + String(average));
}
