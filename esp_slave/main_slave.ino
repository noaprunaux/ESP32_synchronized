#include <WiFi.h>
#include "PubSubClient.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include "wifi_setup.h"
#include "mqtt_setup.h"
#include "sensor_manager.h"
#include "sync_signal.h"
#include "global_variables.h"

// Définition des variables globales
volatile bool sampleFlag = false; // Flag volatile pour indiquer qu'un échantillon est prêt
volatile bool startAcquisition = false; // Flag volatile pour indiquer le début de l'acquisition de données
hw_timer_t* timer = NULL; // Pointeur vers un objet timer matériel

// Sémaphores pour la gestion des accès concurrentiels
SemaphoreHandle_t bufferMutex;
SemaphoreHandle_t i2cMutex;

// Définition des bus I2C
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);

// Fonction de gestion de l'interruption du timer
void IRAM_ATTR onTimer() {
  sampleFlag = true; // Définir le flag d'échantillon lorsque le timer déclenche une interruption
}

// Fonction d'initialisation
void setup() {
  Serial.begin(115200); // Initialisation de la communication série à 115200 bauds
  setupWiFi(); // Configuration du WiFi
  setupMQTT(); // Configuration du client MQTT
  setupSync();  // Configuration pour recevoir le signal de synchronisation

  Serial.println("Slave ready.");
  I2Cone.begin(32, 33, 400000); // Initialisation du bus I2C1 avec des pins spécifiques et une vitesse de 400kHz
  I2Ctwo.begin(21, 22, 400000); // Initialisation du bus I2C2 avec des pins spécifiques et une vitesse de 400kHz

  // Création des mutex pour les accès concurrents
  bufferMutex = xSemaphoreCreateMutex();
  i2cMutex = xSemaphoreCreateMutex();

  // Création de la tâche MQTT épinglée sur le cœur 0
  xTaskCreatePinnedToCore(TaskMQTT, "MQTT Task", 10000, NULL, 1, NULL, 0);

  // Initialisation et configuration du timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer); // Activation de l'alarme du timer
}

// Boucle principale
void loop() {
  // Si un échantillon est prêt et que l'acquisition a commencé
  if (sampleFlag && startAcquisition) {
    sampleFlag = false; // Réinitialiser le flag d'échantillon
    readSensors(); // Lire les capteurs
  }
}
