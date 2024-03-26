//Temperature

#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

bool temp_on = false;
bool temp_off = false;
bool mot_on = false;
bool mot_off = false;

HardwareSerial SerialESP3(2);

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASS";
const char* mqtt_server = "115.187.22.64";
const int mqtt_port = 1883;
const char* mqtt_user = "USER";
const char* mqtt_password = "PASS";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  SerialESP3.begin(115200);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Broker", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("/public/amir/temperature");
      client.subscribe("/public/amir/humidity");
      client.subscribe("/public/amir/motion");
      //client.subscribe("/public/iqram/light");
      //client.subscribe("/public/iqram/fan");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void processAndPublishValues(String data) {
  char temperatureStr[10];
  char humidityStr[10];

  if (sscanf(data.c_str(), "Temperature: %s °C, Humidity: %s%%", temperatureStr, humidityStr) == 2) {
    float temperatureValue = atof(temperatureStr);
    float humidityValue = atof(humidityStr);

    if (client.connected()) {
      client.publish("/public/amir/temperature", String(temperatureValue).c_str());
      client.publish("/public/amir/humidity", String(humidityValue).c_str());
    }
  } else {
    Serial.println("Failed to display data");
  }
}

void setup() {
  Serial.begin(115200);
  SerialESP3.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (SerialESP3.available()) {
    String receivedData = SerialESP3.readStringUntil('\n');
    Serial.println(receivedData);
    processAndPublishValues(receivedData);
  }
}