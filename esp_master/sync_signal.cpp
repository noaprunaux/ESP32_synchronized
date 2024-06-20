#include <Arduino.h>
#include <Ticker.h>
#include "sync_signal.h"

#define SYNC_PIN 17  // Pin for sending start signal and receiving ACK
#define RESET_PIN 16 // Pin for sending reset signal (EN pin)
#define ACK_TIMEOUT 100 // Timeout in milliseconds for ACK

Ticker syncTicker;
Ticker resetTicker;
volatile bool ackReceived = false;

void sendSyncSignal();
void sendResetSignal();
void IRAM_ATTR onAckReceived();

void setupSync() {
  pinMode(SYNC_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  // Set initial state of pins
  digitalWrite(SYNC_PIN, LOW);
  digitalWrite(RESET_PIN, LOW);

  Serial.println("Sync setup completed.");
}

void sendSyncSignal() {
  Serial.println("Sending sync signal");
  ackReceived = false;
  digitalWrite(SYNC_PIN, HIGH);
  delay(100); // Keep the signal high for 100ms
  digitalWrite(SYNC_PIN, LOW);

  // Switch to input mode to receive ACK
  pinMode(SYNC_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), onAckReceived, RISING);

  // Wait for ACK
  unsigned long startWait = millis();
  while (!ackReceived && (millis() - startWait < ACK_TIMEOUT)) {
    delay(1); // Small delay to prevent watchdog reset
  }

  if (ackReceived) {
    Serial.println("ACK received from slave");
  } else {
    
  }

  // Switch back to output mode
  detachInterrupt(digitalPinToInterrupt(SYNC_PIN));
  pinMode(SYNC_PIN, OUTPUT);
}

void sendResetSignal() {
  Serial.println("Sending reset signal");
  digitalWrite(RESET_PIN, HIGH);
  delay(100); // Keep the signal high for 100ms
  digitalWrite(RESET_PIN, LOW);
}

void IRAM_ATTR onAckReceived() {
  ackReceived = true;
  Serial.println("ACK signal detected.");
}
