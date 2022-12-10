#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

// Pin definitions
#define CAPACITANCE_SENSOR_PIN A0
#define PHOTORESISTOR_PIN A1
#define DAYNIGHT_INDICATOR_PIN 2
#define DHT11_PIN 7
// 10, 11, 12, 13 are default SPI, used for SD card interaction
// SCL and SDA are default I2C, used for BMP interaction

Adafruit_BMP280 bmp;
DHT myDHT(DHT11_PIN, DHT11);
File file;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(100);

  // BMP280 Module setup
  bmp.begin(0x76);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);

  // Photoresistor setup
  Wire.begin();
  pinMode(DAYNIGHT_INDICATOR_PIN, OUTPUT);

  myDHT.begin();

  while (!SD.begin(10)) delay(100);
  file = CreateFile();

  // Flash LED to indicate successful initialization
  digitalWrite(DAYNIGHT_INDICATOR_PIN, HIGH);
  delay(500);
  digitalWrite(DAYNIGHT_INDICATOR_PIN, LOW);
  delay(500);
  digitalWrite(DAYNIGHT_INDICATOR_PIN, HIGH);
  delay(500);
  digitalWrite(DAYNIGHT_INDICATOR_PIN, LOW);
}

void loop() {
  // DHT output
  float temperature = myDHT.readTemperature(true); // Fahrenheit
  float humidity = myDHT.readHumidity();
  // BMP output
  float pressure = bmp.readPressure()/101325.0; // Atmospheres
  // Photoresitor output
  int light = analogRead(PHOTORESISTOR_PIN);
  int capacitance = analogRead(CAPACITANCE_SENSOR_PIN);
  float rainFall = 4.0E-5*capacitance*capacitance-0.0332*capacitance+6.89; // Inches
  if (rainFall < 0.0)
    rainFall = 0.0;

  // Write to serial in CSV format
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.print(light);
  Serial.print(",");
  Serial.print(rainFall);
  Serial.println();

  // Write to SD card in CSV format
  file.print(temperature);
  file.print(",");
  file.print(humidity);
  file.print(",");
  file.print(pressure);
  file.print(",");
  file.print(light);
  file.print(",");
  file.print(rainFall);
  file.println();
  file.flush();

  // Turn on LED if it is dark out
  digitalWrite(DAYNIGHT_INDICATOR_PIN, light>75?LOW:HIGH);
  
  // Wait fifteen seconds
  delay(1000*30);
}

File CreateFile() {
  unsigned int num = 1;
  String newName = "/log" + String(num) + ".csv";
  while(SD.exists(newName)) {
    newName = "/log" + String(num) + ".csv";
    num += 1;
  }
  
  return SD.open(newName, FILE_WRITE);
}
