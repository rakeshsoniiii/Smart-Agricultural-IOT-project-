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

// ================= Functions =================
void sendSensorData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilAnalog = analogRead(SOIL_AO);
  int soilDigital = digitalRead(SOIL_DO);

  // Avoid NaN values
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Convert soil analog value to %
  int soilPercent = map(soilAnalog, 4095, 0, 0, 100);

  // Print to Serial Monitor
  Serial.print("Temp: "); Serial.print(t); Serial.print(" °C | ");
  Serial.print("Humidity: "); Serial.print(h); Serial.print(" % | ");
  Serial.print("Soil: "); Serial.print(soilPercent); Serial.println(" %");

  // Send to Blynk
  Blynk.virtualWrite(V1, t);           // Temperature
  Blynk.virtualWrite(V2, h);           // Humidity
  Blynk.virtualWrite(V3, soilPercent); // Soil Moisture

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t); lcd.print("C ");
  lcd.print("H:"); lcd.print(h); lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Soil: "); lcd.print(soilPercent); lcd.print("%");
}

// Manual Relay Control from Blynk (Button Widget on V4)
BLYNK_WRITE(V4)
{
  int relayState = param.asInt(); // 1 = ON, 0 = OFF
  digitalWrite(RELAY_PIN, relayState);
  if (relayState) {
    Serial.println("Relay ON (Manual from Blynk)");
  } else {
    Serial.println("Relay OFF (Manual from Blynk)");
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
  digitalWrite(RELAY_PIN, LOW); // Relay OFF initially

  // Init WiFi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer to send sensor data every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}

// ================= Loop =================
void loop()
{
  Blynk.run();
  timer.run();
}
