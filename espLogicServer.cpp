#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// Dane WiFi
const char* ssid = "esp32";
const char* password = "12345678";

WebServer server(80);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Konfiguracja skanu
const int WIDTH = 91;  // X: 45-135
const int HEIGHT = 61; // Y: 60-120
uint16_t lidarMap[WIDTH][HEIGHT];

// Stan skanowania
bool isScanning = false;
int currentX = 0;
int currentY = 0;
int dirX = 1;

// Piny
#define I2C_SDA 8
#define I2C_SCL 9
#define RX_PIN 17
#define TX_PIN 18

// HTML i Skrypt wizualizacji
const char HTML_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 LIDAR Scanner</title>
    <style>
        body { font-family: sans-serif; text-align: center; background: #222; color: white; }
        canvas { border: 1px solid #555; margin-top: 20px; image-rendering: pixelated; width: 600px; }
        .controls { margin: 20px; }
        button { padding: 10px 20px; font-size: 16px; cursor: pointer; background: #e67e22; border: none; color: white; border-radius: 5px; }
        #status { color: #aaa; }
    </style>
</head>
<body>
    <h1>Micro-LIDAR 2D/3D</h1>
    <div class="controls">
        <button onclick="startScan()">ROZPOCZNIJ SKAN</button>
        <p id="status">Gotowy</p>
    </div>
    <canvas id="lidarCanvas"></canvas>
    <script>
        const canvas = document.getElementById('lidarCanvas');
        const ctx = canvas.getContext('2d');
        canvas.width = 91; canvas.height = 61;

        function startScan() {
            fetch('/start');
            document.getElementById('status').innerText = "Skanowanie...";
        }

        async function updateMap() {
            const response = await fetch('/data');
            const data = await response.json();
            
            ctx.clearRect(0,0,91,61);
            for(let y=0; y<61; y++) {
                for(let x=0; x<91; x++) {
                    let d = data[x][y];
                    if(d > 0) {
                        // Kolorowanie: 0mm = Czerwony (0), 1200mm = Niebieski (240)
                        let hue = Math.min(d / 5, 240); 
                        ctx.fillStyle = `hsl(${hue}, 100%, 50%)`;
                        ctx.fillRect(x, y, 1, 1);
                    }
                }
            }
        }
        setInterval(updateMap, 1000); // Odświeżaj podgląd co sekundę
    </script>
</body>
</html>
)=====";

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!lox.begin()) { Serial.println("Błąd VL53L0X"); while(1); }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nPołączono! IP: " + WiFi.localIP().toString());

  server.on("/", []() { server.send_P(200, "text/html", HTML_INDEX); });
  
  server.on("/start", []() {
    memset(lidarMap, 0, sizeof(lidarMap));
    currentX = 0; currentY = 0; dirX = 1;
    isScanning = true;
    server.send(200, "text/plain", "OK");
  });

server.on("/data", []() {
    String json = "[";
    for(int x = 0; x < WIDTH; x++) {
      json += "[";
      for(int y = 0; y < HEIGHT; y++) {
        json += String(lidarMap[x][y]);
        if (y < HEIGHT - 1) json += ",";
      }
      json += "]"; // Tutaj kończymy wewnętrzną tablicę
      if (x < WIDTH - 1) json += ","; // A tutaj dodajemy przecinek między rzędami
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient(); // Obsługa serwera (nieblokująca)

  if (isScanning) {
    // KROK SKANOWANIA
    Serial1.print('G'); 
    
    // Czekamy na odpowiedź z Arduino (zabezpieczenie przed zawieszeniem)
    unsigned long startWait = millis();
    while(!Serial1.available() || Serial1.read() != 'R') {
      server.handleClient(); // Pozwól serwerowi działać podczas czekania
      if(millis() - startWait > 500) break; 
    }

    // Pomiar
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    uint16_t d = (measure.RangeStatus != 4) ? measure.RangeMilliMeter : 0;
    
    lidarMap[currentX][currentY] = d;

    // Logika przesunięcia
    currentX += dirX;
    if (currentX >= WIDTH || currentX < 0) {
      dirX = -dirX;
      currentX += dirX;
      currentY++;
      if (currentY >= HEIGHT) {
        isScanning = false;
        Serial.println("Skan gotowy!");
      }
    }
  }
}
