#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

// ---------- WIFI ----------
const char* ssid = "ESPTEST";
const char* password = "12345678";

// ---------- THINGSPEAK ----------
unsigned long channelID = 3269673;
const char* writeAPIKey = "0V8H71TXF6FOCBHS";

WiFiClient client;

// ---------- SENSOR PINS ----------
#define DHTPIN 5
#define DHTTYPE DHT22
#define PH_PIN 34
#define TURB_PIN 35

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("System Starting...");

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && timeout < 30) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Failed!");
  }

  ThingSpeak.begin(client);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected!");
    delay(5000);
    return;
  }

  // ----- Read Sensors -----
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int phRaw = analogRead(PH_PIN);
  int turbRaw = analogRead(TURB_PIN);

  float phVoltage = phRaw * (3.3 / 4095.0);
  float pH = 7 + ((2.5 - phVoltage) / 0.18);

  float turbVoltage = turbRaw * (3.3 / 4095.0);
  float NTU = -1120.4 * turbVoltage * turbVoltage
              + 5742.3 * turbVoltage
              - 4352.9;

  if (NTU < 0) NTU = 0;

  Serial.println("------ Sensor Data ------");
  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("pH: "); Serial.println(pH);
  Serial.print("Turbidity (NTU): "); Serial.println(NTU);

  // ----- Upload to ThingSpeak -----
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pH);
  ThingSpeak.setField(4, NTU);

  int status = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (status == 200) {
    Serial.println("Upload Successful");
  } else {
    Serial.print("Upload Failed. Code: ");
    Serial.println(status);
  }

  Serial.println("-------------------------\n");

  delay(20000);  // ThingSpeak minimum 15 seconds
}