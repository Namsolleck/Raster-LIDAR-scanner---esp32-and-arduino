#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define ESP_RX_PIN 17 
#define ESP_TX_PIN 18
#define I2C_SDA 8
#define I2C_SCL 9

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, ESP_RX_PIN, ESP_TX_PIN);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!lox.begin()) {
    Serial.println(F("Błąd! Nie znaleziono czujnika VL53L0X"));
    while (1);
  }
  
  delay(1000);
}

void loop() {
  
  Serial1.print('G'); 

  unsigned long startTime = millis();
  while (!Serial1.available()) {
    if (millis() - startTime > 1000) return;
      }


  String line = Serial1.readStringUntil('\n');
  line.trim();

  if (line.startsWith("R,")) {
    
    
    int firstComma = line.indexOf(',');
    int secondComma = line.lastIndexOf(',');
    
    if (firstComma > 0 && secondComma > firstComma) {
      String xStr = line.substring(firstComma + 1, secondComma);
      String yStr = line.substring(secondComma + 1);
      
      int currentX = xStr.toInt();
      int currentY = yStr.toInt();

      
      VL53L0X_RangingMeasurementData_t measure;
      lox.rangingTest(&measure, false); 

      int distance = -1;
      if (measure.RangeStatus != 4) { 
        distance = measure.RangeMilliMeter;
      }

 
      Serial.print(distance);
      Serial.print(",");
      Serial.print(currentX);
      Serial.print(",");
      Serial.println(currentY);
    }
  }
}
