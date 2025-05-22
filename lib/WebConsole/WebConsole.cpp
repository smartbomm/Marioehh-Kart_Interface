#include "WebConsole.h"

WiFiServer server(81);
WiFiClient client;

void WebConsole::begin(uint8_t baud) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }
    server.begin();
}

void WebConsole::print(const char* str) {
    client = server.available();
    // Warte auf Daten vom Client
    while (client.connected() && !client.available()) {
        delay(1);
      }
  
      String request = client.readStringUntil('\r');
      Serial.print("Anfrage: ");
      Serial.println(request);
      client.flush();
  
      // HTTP-Antwort senden
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();  // Leerzeile trennt Header von Inhalt
      client.println("<!DOCTYPE html><html><body><h1>Hallo vom Arduino!</h1></body></html>");
  
      delay(1000);  // Kurze Pause, bevor Verbindung geschlossen wird
      client.stop();
      Serial.println("Client getrennt");
}