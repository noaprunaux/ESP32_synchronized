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

SemaphoreHandle_t bufferMutex;
SemaphoreHandle_t i2cMutex;

TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);

hw_timer_t* timer = NULL;
volatile bool sampleFlag = false;
volatile bool startAcquisition = false;  // Flag to start acquisition

const int BOOT_PIN = 0;  // GPIO0 is usually the BOOT button

void IRAM_ATTR onTimer() {
  sampleFlag = true;
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupMQTT();
  setupSync();

  Serial.println("Master ready.");
  I2Cone.begin(32, 33, 400000);
  I2Ctwo.begin(21, 22, 400000);

  bufferMutex = xSemaphoreCreateMutex();
  i2cMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskMQTT, "MQTT Task", 10000, NULL, 1, NULL, 0);

  startTime = millis();

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  pinMode(BOOT_PIN, INPUT_PULLUP);  // Configure the BOOT button as input with pull-up
}

void loop() {
  if (sampleFlag && startAcquisition) {
    sampleFlag = false;
    readSensors();
  }

  // Check if the BOOT button is pressed
  if (digitalRead(BOOT_PIN) == LOW) {  // Active LOW
    sendSyncSignal();
    startAcquisition = true;  // Set the flag to start acquisition
    delay(500);  // Debounce delay
  }
}
