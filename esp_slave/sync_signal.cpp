#include <Arduino.h>
#include "sync_signal.h"
#include "global_variables.h"

#define SYNC_PIN 17  // Pin pour recevoir le signal de synchronisation
#define ACK_PIN 17   // Utiliser la même broche pour envoyer l'ACK

void setupSync() {
  pinMode(SYNC_PIN, INPUT);  // Configure SYNC_PIN comme entrée
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), onSyncReceived, RISING);  // Attache une interruption pour détecter le signal de synchronisation
}

void IRAM_ATTR onSyncReceived() {
  startAcquisition = true;  // Définit le drapeau pour démarrer l'acquisition

  // Désactive temporairement l'interruption
  detachInterrupt(digitalPinToInterrupt(SYNC_PIN));
  
  // Envoie un ACK
  sendAck();

  // Réactive l'interruption
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), onSyncReceived, RISING);
}

void sendAck() {
  pinMode(ACK_PIN, OUTPUT);  // Change la broche en sortie
  digitalWrite(ACK_PIN, HIGH);
  delay(10);  // Maintenir le signal HIGH pendant 10ms pour garantir que le maître le capte
  digitalWrite(ACK_PIN, LOW);
  pinMode(ACK_PIN, INPUT);  // Revenir à l'entrée après avoir envoyé l'ACK
}
