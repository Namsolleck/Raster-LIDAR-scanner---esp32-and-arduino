#include "Adafruit_VL53L0X.h"

// Tworzymy obiekt czujnika
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Definicja pinów UART dla Arduino Mega
#define ESP_RX_PIN 17 
#define ESP_TX_PIN 18

// Piny I2C
#define I2C_SDA 8
#define I2C_SCL 9

void setup() {
  Serial.begin(115200);
  
  // Komunikacja z Arduino
  Serial1.begin(9600, SERIAL_8N1, ESP_RX_PIN, ESP_TX_PIN);

  // Inicjalizacja I2C na konkretnych pinach S3
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.println("Test czujnika Adafruit VL53L0X...");
  if (!lox.begin()) {
    Serial.println(F("Błąd! Nie znaleziono czujnika VL53L0X"));
    while (1);
  }
  Serial.println(F("Czujnik gotowy!"));
}

void loop() {
  // 1. Rozkaz dla Arduino Mega
  Serial1.print('G'); 

  // 2. Czekaj na 'R' (Ready)
  while (!Serial1.available() || Serial1.read() != 'R') {
    delay(1);
  }

  // 3. Pomiar (Wersja Adafruit używa struktury VL53L0X_RangingMeasurementData_t)
  VL53L0X_RangingMeasurementData_t measure;
  
  lox.rangingTest(&measure, false); // 'false' oznacza, że nie chcemy debugowania w Serialu

  if (measure.RangeStatus != 4) {  // Status 4 to błąd (out of range)
    Serial.print("Dystans: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println(" Poza zasięgiem ");
  }
}
