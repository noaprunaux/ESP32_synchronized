#include "sensor_manager.h"
#include "PubSubClient.h"
#include "mqtt_setup.h"

// Tableau pour stocker les données des capteurs
int16_t sensorData[NUM_SENSORS];

// Buffers pour stocker les données avant envoi
char buffer1[1500];
char buffer2[1500];
int bufferIndex1 = 0;
int bufferIndex2 = 0;
const int BUFFER_SIZE = 1500;

unsigned long startTime; // Variable pour mesurer le temps écoulé

// Fonction pour lire les capteurs
void readSensors() {
  unsigned long startMicros = micros();
  // Création de deux tâches pour lire les capteurs en parallèle
  xTaskCreatePinnedToCore(readSensorsTask, "Read Sensors Task 1", 10000, (void*)0, 1, NULL, 0);
  xTaskCreatePinnedToCore(readSensorsTask, "Read Sensors Task 2", 10000, (void*)1, 1, NULL, 1);
  vTaskDelay(0.3 / portTICK_PERIOD_MS);
  storeSensorData();
  unsigned long endMicros = micros();
  Serial.print("Temps d'exécution de readSensors: ");
  Serial.println(endMicros - startMicros);
}

// Tâche pour lire les données des capteurs
void readSensorsTask(void* parameter) {
  int taskNum = (int)parameter;
  if (taskNum == 0) {
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
      // Lire les capteurs sur le premier bus I2C
      for (int channel = 0; channel < 4; channel++) {
        selectMultiplexerChannel(&I2Cone, MULTIPLEXER1_ADDR, channel);
        readSensorData(&I2Cone, channel);
      }
      xSemaphoreGive(i2cMutex);
    }
  } else {
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
      // Lire les capteurs sur le second bus I2C
      for (int channel = 0; channel < 4; channel++) {
        selectMultiplexerChannel(&I2Ctwo, MULTIPLEXER2_ADDR, channel);
        readSensorData(&I2Ctwo, channel + 4);
      }
      xSemaphoreGive(i2cMutex);
    }
  }
  vTaskDelete(NULL); // Supprimer la tâche après exécution
}

// Fonction pour sélectionner un canal sur le multiplexeur
void selectMultiplexerChannel(TwoWire* bus, uint8_t multiplexerAddress, uint8_t channel) {
  bus->beginTransmission(multiplexerAddress);
  bus->write(1 << channel);
  bus->endTransmission();
}

// Fonction pour lire les données d'un capteur
void readSensorData(TwoWire* bus, int sensorIndex) {
  bus->beginTransmission(SENSOR_ADDR);
  bus->write(SENSOR_MEMORY_ADDR);
  int transmissionResult = bus->endTransmission();
  if (transmissionResult == 0) {
    if (bus->requestFrom(SENSOR_ADDR, 2) == 2) {
      sensorData[sensorIndex] = bus->read() | (bus->read() << 8);
    } else {
      Serial.println("Erreur de lecture des données du capteur");
    }
  } else {
    Serial.println("Erreur de transmission I2C");
  }
}

// Fonction pour stocker les données des capteurs dans les buffers
void storeSensorData() {
  if (xSemaphoreTake(bufferMutex, portMAX_DELAY)) {
    unsigned long timeElapsed = millis() - startTime;

    // Stocker les données des quatre premiers capteurs dans buffer1
    int length1 = snprintf(buffer1 + bufferIndex1, sizeof(buffer1) - bufferIndex1, "%d,%d,%d,%d,%lu\n", sensorData[0], sensorData[1], sensorData[2], sensorData[3], timeElapsed);
    if (bufferIndex1 + length1 < sizeof(buffer1)) {
      bufferIndex1 += length1;
    } else {
      sendBuffer1();
      bufferIndex1 = 0;
      memset(buffer1, 0, sizeof(buffer1));
    }

    // Stocker les données des quatre derniers capteurs dans buffer2
    int length2 = snprintf(buffer2 + bufferIndex2, sizeof(buffer2) - bufferIndex2, "%d,%d,%d,%d,%lu\n", sensorData[4], sensorData[5], sensorData[6], sensorData[7], timeElapsed);
    if (bufferIndex2 + length2 < sizeof(buffer2)) {
      bufferIndex2 += length2;
    } else {
      sendBuffer2();
      bufferIndex2 = 0;
      memset(buffer2, 0, sizeof(buffer2));
    }

    xSemaphoreGive(bufferMutex);
  }
}

// Fonction pour envoyer les données du buffer1 via MQTT
void sendBuffer1() {
  client.publish("sensor1/data", buffer1);
}

// Fonction pour envoyer les données du buffer2 via MQTT
void sendBuffer2() {
  client.publish("sensor2/data", buffer2);
}
