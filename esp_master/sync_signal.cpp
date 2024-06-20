#include <Arduino.h>
#include <Ticker.h>
#include "sync_signal.h"

#define SYNC_PIN 17  // Pin pour envoyer le signal de synchronisation et recevoir l'ACK
#define RESET_PIN 16 // Pin pour envoyer le signal de réinitialisation (EN pin)
#define ACK_TIMEOUT 100 // Délai d'attente pour l'ACK en millisecondes

Ticker syncTicker;
Ticker resetTicker;
volatile bool ackReceived = false; // Indicateur pour vérifier si l'ACK a été reçu

void sendSyncSignal();
void sendResetSignal();
void IRAM_ATTR onAckReceived(); // Fonction d'interruption pour la réception de l'ACK

// Configuration de la synchronisation
void setupSync() {
  pinMode(SYNC_PIN, OUTPUT); // Configuration de SYNC_PIN comme sortie
  pinMode(RESET_PIN, OUTPUT); // Configuration de RESET_PIN comme sortie

  // État initial des pins
  digitalWrite(SYNC_PIN, LOW);
  digitalWrite(RESET_PIN, LOW);

  Serial.println("Sync setup completed."); // Indique que la configuration est terminée
}

// Envoi du signal de synchronisation
void sendSyncSignal() {
  Serial.println("Sending sync signal");
  ackReceived = false; // Réinitialiser l'indicateur d'ACK
  digitalWrite(SYNC_PIN, HIGH); // Mettre le signal de synchronisation à HIGH
  delay(100); // Garder le signal HIGH pendant 100 ms
  digitalWrite(SYNC_PIN, LOW); // Remettre le signal de synchronisation à LOW

  // Passer en mode entrée pour recevoir l'ACK
  pinMode(SYNC_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), onAckReceived, RISING); // Attacher l'interruption pour détecter l'ACK

  // Attendre l'ACK
  unsigned long startWait = millis();
  while (!ackReceived && (millis() - startWait < ACK_TIMEOUT)) {
    delay(1); // Petit délai pour éviter le reset du watchdog
  }

  if (ackReceived) {
    Serial.println("ACK received from slave"); // ACK reçu
  } else {
    Serial.println("ACK not received within timeout"); // Timeout sans recevoir l'ACK
  }

  // Repasser en mode sortie
  detachInterrupt(digitalPinToInterrupt(SYNC_PIN)); // Détacher l'interruption
  pinMode(SYNC_PIN, OUTPUT);
}

// Envoi du signal de réinitialisation
void sendResetSignal() {
  Serial.println("Sending reset signal");
  digitalWrite(RESET_PIN, HIGH); // Mettre le signal de réinitialisation à HIGH
  delay(100); // Garder le signal HIGH pendant 100 ms
  digitalWrite(RESET_PIN, LOW); // Remettre le signal de réinitialisation à LOW
}

// Fonction d'interruption pour la réception de l'ACK
void IRAM_ATTR onAckReceived() {
  ackReceived = true; // Indicateur d'ACK reçu
  Serial.println("ACK signal detected.");
}
