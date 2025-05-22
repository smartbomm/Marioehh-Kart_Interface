#include <SPI.h>
#include <WiFiNINA.h>

String htmlPage = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Virtuelle Konsole</title>
      <style>
        body {
          font-family: monospace;
          background: black;
          color: lime;
          margin: 0;
          padding: 10px;
        }
        #log {
          height: 80vh;
          overflow-y: auto;
          white-space: pre-wrap;
          border: 1px solid lime;
          padding: 5px;
          background-color: #000;
        }
        input {
          background: black;
          color: lime;
          border: 1px solid lime;
          width: 100%;
          padding: 5px;
          font-family: monospace;
        }
      </style>
    </head>
    <body>
      <h2>Virtuelle Konsole</h2>
      <div id="log"></div>
      <input id="input" placeholder="Befehl eingeben..." autofocus />
      <script>
        const log = document.getElementById("log");
        const input = document.getElementById("input");
        const MAX_LINES = 500;
        let lines = [];
    
        const ws = new WebSocket("ws://" + location.host + "/ws");
    
        ws.onmessage = (event) => {
          addLine(event.data);
        };
    
        input.addEventListener("keydown", function(e) {
          if (e.key === "Enter" && ws.readyState === WebSocket.OPEN) {
            const command = input.value.trim();
            if (command.length > 0) {
              ws.send(command);
              addLine("> " + command);
            }
            input.value = "";
          }
        });
    
        function addLine(text) {
          lines.push(text);
          if (lines.length > MAX_LINES) {
            lines.shift(); // älteste Zeile entfernen
          }
          log.innerText = lines.join("\n");
    
          // Nur scrollen, wenn Benutzer am unteren Ende ist
          const nearBottom = log.scrollHeight - log.scrollTop - log.clientHeight < 50;
          if (nearBottom) {
            log.scrollTop = log.scrollHeight;
          }
        }
      </script>
    </body>
    </html>
    )rawliteral";
    

// WLAN-Zugangsdaten
char ssid[] = "WG-Net";
char pass[] = "144+WG_pmtj&1844";

// Port 80 für HTTP
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // WLAN verbinden
  Serial.print("Verbinde mit WLAN...");
  int status = WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nVerbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Starte den Webserver
  server.begin();
}

void loop() {
  WiFiClient client = server.available();  // Warte auf einen neuen Client
  if (client) {
    Serial.println("Neuer Client verbunden");

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
    client.println(htmlPage);

    delay(1);  // Kurze Pause, bevor Verbindung geschlossen wird
    client.stop();
    Serial.println("Client getrennt");
  }
}
