#include <PubSubClient.h>
#include <WiFiNINA.h>
#include <EEPROM.h>
#include <DHT.h>
#define EEPROM_SIZE 96
#define SSID_ADDR 0
#define PASS_ADDR 32
#define SOIL_PIN A0
#define DHTPIN 2       // DHT11 DATA腳接在 D2
#define DHTTYPE DHT11  // 型號是 DHT11
DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;

PubSubClient client(wifiClient);
const char* mqtt_server = "192.168.0.114"; // 例如 "192.168.1.10"
const int   mqtt_port = 1883;
const char* mqtt_topic = "plantmate/sensor";
const char* sensor_id = "sensor_001"; 


char ssid[32] = "";
char pass[32] = "";

void readCredentials() {
  for (int i = 0; i < 32; i++) {
    ssid[i] = EEPROM.read(SSID_ADDR + i);
    pass[i] = EEPROM.read(PASS_ADDR + i);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT Connecting...");
    if (client.connect(sensor_id)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void writeCredentials(const char* ssid_in, const char* pass_in) {
  for (int i = 0; i < 32; i++) {
    EEPROM.write(SSID_ADDR + i, ssid_in[i]);
    EEPROM.write(PASS_ADDR + i, pass_in[i]);
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();           // 啟動 DHT11
  while (!Serial);

  Serial.println("Type S to set WiFi, or any key to use saved:");

  delay(2000);
  String input = "S";
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim(); // 去除空白和換行
  }
  if (input == "S" || input == "s") {
    Serial.println("Enter SSID:");
    while (!Serial.available());
    String ssid_str = Serial.readStringUntil('\n');
    ssid_str.trim();
    ssid_str.toCharArray(ssid, 32);

    Serial.println("Enter Password:");
    while (!Serial.available());
    String pass_str = Serial.readStringUntil('\n');
    pass_str.trim();
    pass_str.toCharArray(pass, 32);

    writeCredentials(ssid, pass);
    Serial.println("WiFi credentials saved!");
  } else {
    readCredentials();
    Serial.print("Loaded SSID: "); Serial.println(ssid);
  }

  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    counter++;
    if (counter > 10) {
      Serial.println("\nFailed to connect.");
      return;
    }
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 每2秒讀取一次感測數據
  delay(5000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilValue = analogRead(SOIL_PIN);  // 0~1023, 值越大代表越乾
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11!");
    return;
  }
  Serial.print("humidity: ");
  Serial.println(humidity);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.print("soil: ");
  Serial.println(soilValue);
  char tempBuffer[10];
  char humBuffer[10];

  dtostrf(temperature, 6, 2, tempBuffer);
  dtostrf(humidity, 6, 2, humBuffer);

  // 組成JSON payload
  char payload[128];
  snprintf(payload, sizeof(payload),
    "{\"sensor_id\":\"%s\",\"temperature\":%s,\"humidity\":%s,\"soil\":%d}",
    sensor_id, tempBuffer, humBuffer, soilValue);
  Serial.print("Sending MQTT payload: ");
  Serial.println(payload);

  // 發佈到MQTT
  client.publish(mqtt_topic, payload);
}
