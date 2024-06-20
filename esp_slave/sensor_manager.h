#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "global_variables.h"

#define MULTIPLEXER1_ADDR 0x75
#define MULTIPLEXER2_ADDR 0x74
#define SENSOR_ADDR 0x6C
#define SENSOR_MEMORY_ADDR 0x30
#define NUM_SENSORS 8

extern TwoWire I2Cone;
extern TwoWire I2Ctwo;
extern SemaphoreHandle_t bufferMutex;
extern SemaphoreHandle_t i2cMutex;
extern int16_t sensorData[NUM_SENSORS];
extern char buffer1[1500];
extern char buffer2[1500];
extern int bufferIndex1;
extern int bufferIndex2;
extern const int BUFFER_SIZE;
extern unsigned long startTime;

extern volatile bool startAcquisition;  // Ajout de la déclaration de startAcquisition

void readSensors();
void readSensorsTask(void* parameter);
void selectMultiplexerChannel(TwoWire* bus, uint8_t multiplexerAddress, uint8_t channel);
void readSensorData(TwoWire* bus, int sensorIndex);
void storeSensorData();
void sendBuffer1();
void sendBuffer2();

#endif // SENSOR_MANAGER_H
