/*
  Michal Lesiowski
  Complete project details at https://RandomNerdTutorials.com/esp32-i2c-communication-arduino-ide/
  

*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define I2C_SDA 5
#define I2C_SCL 4

//SEA Level in PL - 218m
//Calculated by https://keisan.casio.com/exec/system/1224575267 , finally presussure data from windy.com
#define SEALEVELPRESSURE_HPA (1029)

TwoWire I2CBME = TwoWire(0);
Adafruit_BME280 bme;

unsigned long delayTime;

void setup() {
  Serial.begin(115200);
  Serial.println(F("BME280 test"));
  I2CBME.begin(I2C_SDA, I2C_SCL, 100000);

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76, &I2CBME);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
}

void loop() { 
  printValues();
  delay(delayTime);
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");*/
  
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
