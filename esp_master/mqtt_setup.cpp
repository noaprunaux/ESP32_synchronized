#include <WiFi.h>
#include "PubSubClient.h"
#include "mqtt_setup.h"
#include "wifi_setup.h"

const char* mqtt_server = "192.168.0.170";
int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setupMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setKeepAlive(60);  // Set Keep-Alive interval to 60 seconds
}

void TaskMQTT(void* parameter) {
  while (true) {
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop();
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Augmenter le délai à 1 seconde pour réduire la charge
  }
}

void reconnectMQTT() {
  const int maxRetries = 5;
  int retries = 0;
  int delayBetweenRetries = 5000;  // Initial delay of 5 seconds

  while (!client.connected() && retries < maxRetries) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      retries++;
      vTaskDelay(delayBetweenRetries / portTICK_PERIOD_MS);
      delayBetweenRetries *= 2;  // Exponential backoff
    }
  }

  if (retries >= maxRetries) {
    Serial.println("Maximum reconnection attempts reached. Waiting before retrying...");
    vTaskDelay(30000 / portTICK_PERIOD_MS);  // Wait 30 seconds before retrying
  }
}
