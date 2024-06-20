#include <WiFi.h>
#include "wifi_setup.h"

// Nom du réseau WiFi et mot de passe
const char* ssid = "LABORATOIRE";
const char* password = "gurvanaditpastouche";

// Fonction pour configurer la connexion WiFi
void setupWiFi() {
  WiFi.begin(ssid, password); // Commence la connexion au réseau WiFi
  while (WiFi.status() != WL_CONNECTED) { // Attend la connexion
    vTaskDelay(500 / portTICK_PERIOD_MS); // Délai de 500 ms pour FreeRTOS
    Serial.print("."); // Affiche un point pour indiquer la tentative de connexion
  }
  Serial.println("\nWiFi connected."); // Indique que la connexion est établie
}

// Fonction pour vérifier l'état de la connexion WiFi et reconnecter si nécessaire
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) { // Vérifie si la connexion WiFi est perdue
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    WiFi.disconnect(); // Déconnecte du réseau WiFi
    WiFi.begin(ssid, password); // Tente de se reconnecter au réseau WiFi
    while (WiFi.status() != WL_CONNECTED) { // Attend la reconnexion
      delay(500); // Délai de 500 ms pour attendre la reconnexion
      Serial.print("."); // Affiche un point pour indiquer la tentative de reconnexion
    }
    Serial.println("WiFi reconnected"); // Indique que la reconnexion est établie
  }
}
