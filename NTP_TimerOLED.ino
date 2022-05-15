#include <WiFi.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduino_secrets.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ThingerESP32.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define NTP_OFFSET  3600 // In seconds 
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
const int relay = 16;

// Variables to save date and time
String formattedDate;
String formattedTime;
String dayStamp;
String timeStamp;
String currentSec;
int countUp = 0;

int COUNTER;
int flag = 0;

//Board = ESP32 Wemos Lolin

unsigned long prevRelayTime = 0;
unsigned long delayRelay = 1200000; //20 minutes

//Alarms
#define ALARM1_ON "02:01:00"
#define ALARM2_ON  "04:14:00"

//BME280 I2C data
#define I2C_SDA 5
#define I2C_SCL 4
#define SEALEVELPRESSURE_HPA (1029)

//BME280 - Temperature, Presseru, Level
TwoWire I2CBME = TwoWire(0);
Adafruit_BME280 bme;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Thinger
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);


void setup()
{

  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  //Connect to Thinger.io
  thing.add_wifi(SSID, SSID_PASSWORD);
  thing["GPIO_16"] << digitalPin(relay);
  thing["Temp"] >> outputValue(bme.readTemperature());
  thing["Humidity"] >> outputValue(bme.readHumidity());
  thing["Pressure"] >> outputValue(bme.readPressure() / 100.0F);
  thing["Relay"] >> outputValue(digitalRead(relay));
  thing["Altitiude"] >> outputValue(bme.readAltitude(SEALEVELPRESSURE_HPA));
  

  //Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
  Wire.begin(5, 4);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  display.clearDisplay();
  display.setTextColor(WHITE);
  //display.startscrollright(0x00, 0x0F);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("  Internet ");
  display.println("  Clock ");
  display.display();

  //BME280
  Serial.println(F("BME280 connecting..."));
  I2CBME.begin(I2C_SDA, I2C_SCL, 100000);
  bool status;

  status = bme.begin(0x76, &I2CBME);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  delay(700);

  }

void loop()
{
  unsigned long timeNow = millis();
  
  thing.handle();

  timeClient.update();
  formattedTime = timeClient.getFormattedTime();
  formattedDate = timeClient.getFormattedDate();

  //Print Data
  //printData();
  //printBMEValues();
  printLCD();


  //Alarm at night
  if (formattedTime == ALARM1_ON || formattedTime == ALARM2_ON) {     
      prevRelayTime = timeNow;      
       Serial.println("-- Start NOW ----");
       digitalWrite(relay, HIGH); 
       flag = 1;      
  }

  if (timeNow > prevRelayTime + delayRelay && flag == 1) {
         Serial.println("--> End time <--"); 
         digitalWrite(relay, LOW);        
         flag = 0;
       }


  delay(100);

}

void printLCD() {

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(formattedTime);
  display.setTextSize(1);
  display.println();
  display.print("Temp:  ");
  display.print(bme.readTemperature());
  display.println();
  display.print("Humidity:  ");
  display.print(bme.readHumidity());
  display.println();
  display.print("Pressure:  ");
  display.print(bme.readPressure() / 100.0F);
  display.println();
  display.print("Altitiude:  ");
  display.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  display.println();
  display.print("Relay:  ");
  display.print(digitalRead(relay));
  display.display();   // write the buffer to the display

  
}

void printData() {

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  currentSec = timeStamp.substring(6);
  Serial.println(currentSec);
  
}

void printBMEValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
    
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}
