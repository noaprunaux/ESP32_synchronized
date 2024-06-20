#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Déclarations externes des variables globales
extern volatile bool sampleFlag;
extern volatile bool startAcquisition;
extern hw_timer_t* timer;

#endif // GLOBAL_VARIABLES_H
