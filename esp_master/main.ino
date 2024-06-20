#include <WiFi.h>
#include "PubSubClient.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include <Ticker.h>
#include "wifi_setup.h"
#include "mqtt_setup.h"
#include "sensor_manager.h"
#include "sync_signal.h"

// Déclaration des sémaphores pour la gestion des accès concurrents
SemaphoreHandle_t bufferMutex;
SemaphoreHandle_t i2cMutex;

// Déclaration des bus I2C
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);

// Déclaration du timer matériel et des indicateurs de statut
hw_timer_t* timer = NULL;
volatile bool sampleFlag = false;
volatile bool startAcquisition = false;  // Indicateur pour démarrer l'acquisition

const int BOOT_PIN = 0;  // GPIO0 est généralement le bouton BOOT

// Fonction d'interruption du timer
void IRAM_ATTR onTimer() {
  sampleFlag = true;  // Définir le flag d'échantillon lorsque le timer déclenche une interruption
}

// Fonction d'initialisation
void setup() {
  Serial.begin(115200);  // Initialisation de la communication série
  setupWiFi();  // Configuration du WiFi
  setupMQTT();  // Configuration du client MQTT
  setupSync();  // Configuration pour recevoir le signal de synchronisation

  Serial.println("Master ready.");
  I2Cone.begin(32, 33, 400000);  // Initialisation du bus I2C1 avec des pins spécifiques et une vitesse de 400kHz
  I2Ctwo.begin(21, 22, 400000);  // Initialisation du bus I2C2 avec des pins spécifiques et une vitesse de 400kHz

  // Création des mutex pour les accès concurrents
  bufferMutex = xSemaphoreCreateMutex();
  i2cMutex = xSemaphoreCreateMutex();

  // Création et démarrage de la tâche MQTT épinglée sur le cœur 0
  xTaskCreatePinnedToCore(TaskMQTT, "MQTT Task", 10000, NULL, 1, NULL, 0);

  startTime = millis();  // Initialisation du temps de démarrage

  // Initialisation et configuration du timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);  // Activation de l'alarme du timer

  pinMode(BOOT_PIN, INPUT_PULLUP);  // Configuration du bouton BOOT en entrée avec résistance pull-up
}

// Boucle principale
void loop() {
  // Si un échantillon est prêt et que l'acquisition a commencé
  if (sampleFlag && startAcquisition) {
    sampleFlag = false;  // Réinitialiser le flag d'échantillon
    readSensors();  // Lire les capteurs
  }

  // Vérifie si le bouton BOOT est pressé
  if (digitalRead(BOOT_PIN) == LOW) {  // Niveau bas actif
    sendSyncSignal();  // Envoyer un signal de synchronisation
    startAcquisition = true;  // Définir le flag pour démarrer l'acquisition
    delay(500);  // Délai pour éviter les rebonds du bouton
  }
}
