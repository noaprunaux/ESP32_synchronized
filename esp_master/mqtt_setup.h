#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

#include <WiFi.h>
#include "PubSubClient.h"

extern PubSubClient client;

void setupMQTT();
void TaskMQTT(void* parameter);
void reconnectMQTT();

#endif // MQTT_SETUP_H
