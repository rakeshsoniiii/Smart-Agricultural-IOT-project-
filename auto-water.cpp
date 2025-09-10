// ================= Blynk Template Info =================
#define BLYNK_TEMPLATE_ID "TMPL3dj1JLlKk"
#define BLYNK_TEMPLATE_NAME "Plant Watering System 3"
#define BLYNK_AUTH_TOKEN "2qoK_f7NvIbIJorFSBEwRgddeXJJUVHg"

// ================= Libraries =================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ================= Pin Definitions =================
#define DHTPIN 4          // DHT11 data pin
#define DHTTYPE DHT11     // DHT11
#define SOIL_AO 34        // Soil moisture AO (analog)
#define SOIL_DO 25        // Soil moisture DO (digital)
#define RELAY_PIN 26      // Relay control pin

// ================= Objects =================
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C LCD
BlynkTimer timer;

// ================= WiFi =================
char ssid[] = "Redmi Note 12 Pro 5G";   // WiFi SSID
char pass[] = "Rupam892";               // WiFi Password

// ================= Global Variables =================
bool manualMode = false;  // If true, manual relay control
int soilThreshold = 40;   // % threshold for auto watering

// ================= Functions =================
void sendSensorData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilAnalog = analogRead(SOIL_AO);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Soil % (0 = wet, 100 = dry)
  int soilPercent = map(soilAnalog, 4095, 0, 0, 100);

  // Print to Serial
  Serial.print("Temp: "); Serial.print(t);
  Serial.print(" °C | Humidity: "); Serial.print(h);
  Serial.print(" % | Soil: "); Serial.print(soilPercent); Serial.println(" %");

  // Send to Blynk
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, soilPercent);

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t); lcd.print("C ");
  lcd.print("H:"); lcd.print(h); lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Soil: "); lcd.print(soilPercent); lcd.print("%");

  // Auto-watering only if manual mode is OFF
  if (!manualMode) {
    if (soilPercent < soilThreshold) {
      digitalWrite(RELAY_PIN, HIGH); // Pump ON
      Blynk.virtualWrite(V4, 1);     // Update app button
      Serial.println("Pump ON (Auto - Soil dry)");
    } else {
      digitalWrite(RELAY_PIN, LOW);  // Pump OFF
      Blynk.virtualWrite(V4, 0);     // Update app button
      Serial.println("Pump OFF (Auto - Soil OK)");
    }
  }
}

// Manual Relay Control from Blynk (Button Widget on V4)
BLYNK_WRITE(V4)
{
  int relayState = param.asInt(); // 1 = ON, 0 = OFF

  if (manualMode) {  // Only if manual mode is enabled
    digitalWrite(RELAY_PIN, relayState);
    if (relayState) {
      Serial.println("Relay ON (Manual from Blynk)");
    } else {
      Serial.println("Relay OFF (Manual from Blynk)");
    }
  }
}

// Switch between Auto / Manual mode (V5 button in app)
BLYNK_WRITE(V5)
{
  manualMode = param.asInt(); // 1 = Manual, 0 = Auto
  if (manualMode) {
    Serial.println("Mode: MANUAL");
  } else {
    Serial.println("Mode: AUTO");
  }
}

// ================= Setup =================
void setup()
{
  Serial.begin(115200);

  // Init sensors & LCD
  dht.begin();
  lcd.init();
  lcd.backlight();

  // Init relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer for sensor data
  timer.setInterval(2000L, sendSensorData);
}

// ================= Loop =================
void loop()
{
  Blynk.run();
  timer.run();
}
