// ================= this is my Blynk Template Info =================
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
#define DHTPIN 4        // DHT11 data pin
#define DHTTYPE DHT11   // DHT11
#define SOIL_AO 34      // Soil moisture AO (analog)
#define SOIL_DO 25      // Soil moisture DO (digital)
#define RELAY_PIN 26    // Relay control pin

// ================= Objects =================
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD (0x27 or 0x3F)
BlynkTimer timer;

// ================= WiFi =================
char ssid[] = "Redmi Note 12 Pro 5G";   // WiFi SSID
char pass[] = "Rupam892";             // WiFi Password

// ================= Virtual Pins =================
// V0: Button for Relay Control
// V1: Temperature Display
// V2: Humidity Display
// V3: Digital Soil Moisture Status

// ================= Function Declarations =================
void sendSensorData();

// ================= Blynk Functions =================
// This function is called when the Blynk button for the relay (V0) is pressed.
BLYNK_WRITE(V0) {
  int relayState = param.asInt();

  // Control the relay based on the received value
  // Note: Most relays are "active low," meaning LOW turns them on.
  if (relayState == 1) {
    digitalWrite(RELAY_PIN, LOW); // Turn the relay ON
    Serial.println("Relay ON");
    lcd.setCursor(0, 1);
    lcd.print("Relay: ON ");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Turn the relay OFF
    Serial.println("Relay OFF");
    lcd.setCursor(0, 1);
    lcd.print("Relay: OFF");
  }
}

// This function is called every time the device connects to the Blynk server.
// It synchronizes the state of all virtual pins.
BLYNK_CONNECTED() {
  Blynk.syncAll();
}

// ================= Setup and Loop =================
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Set pin modes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SOIL_DO, INPUT); // Digital soil sensor is an input

  // Start with the relay off
  digitalWrite(RELAY_PIN, HIGH);

  // Initialize the DHT11 sensor
  dht.begin();

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Connecting...");

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Setup a timer to send sensor data to Blynk every 5 seconds
  timer.setInterval(5000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}

// ================= Custom Functions =================
// This function reads sensor data and sends it to Blynk
void sendSensorData() {
  // Read Temperature and Humidity from DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Read digital soil moisture status
  int soilMoistureStatus = digitalRead(SOIL_DO); // 0 = wet, 1 = dry

  // Check if any reads failed and exit early (to avoid sending wrong data)
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print to Serial for debugging
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Soil Moisture (DO): ");
  Serial.println(soilMoistureStatus == LOW ? "WET" : "DRY");

  // Send the values to Blynk
  Blynk.virtualWrite(V1, t); // Temperature
  Blynk.virtualWrite(V2, h); // Humidity
  Blynk.virtualWrite(V3, soilMoistureStatus); // Digital Soil Moisture (0 or 1)

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.write(223); // Degree symbol
  lcd.print("C H:");
  lcd.print(h);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  lcd.print(soilMoistureStatus == LOW ? "WET" : "DRY");
}
