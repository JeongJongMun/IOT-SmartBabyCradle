#include <WiFi.h>

const char* ap_ssid = "ESP32_IoT_1";
const char* ap_password = "11112222";
const char* sta_ssid = "KAU-Guest";
const char* sta_password = "";

WiFiServer server(80);

void WiFiSetup() {
  Serial.print("WIFI status = "); Serial.println(WiFi.getMode());
  WiFi.disconnect(true); delay(1000);

  WiFi.mode(WIFI_AP_STA); delay(1000);
  Serial.print("WIFI status = "); Serial.println(WiFi.getMode());
  WiFi.begin(sta_ssid, sta_password);


  for(; WiFi.status() != WL_CONNECTED; delay(1000))
    Serial.println("Connecting to WiFi..");

  Serial.println("Connected to wifi");
  Serial.println("STA IP address: "); Serial.println(WiFi.localIP());

  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: "); Serial.println(IP);
  
  server.begin();
}

void setup() {
  Serial.begin(115200);

  WiFiSetup();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<html>");
            client.println("<head>");
            client.println("<title>ESP32 IoT</title>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>Hello World!</h1>");
            client.println("</body>");
            client.println("</html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
