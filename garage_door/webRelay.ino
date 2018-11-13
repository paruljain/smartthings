/*
  General purpose momentary web operated relay
  (c) Parul Jain 2018
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

#define RELAY D1
#define PULSE_SIZE 500 //relay pulse 0.5s

void handleRelay() {
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(RELAY, HIGH);
  delay(PULSE_SIZE);
  digitalWrite(RELAY, LOW);
  digitalWrite(BUILTIN_LED, HIGH);
  server.send(200, "text/plain", "OK");
}

void handleNotFound() {
  server.send(404);
}

void wifiSetup() {
  char ssid[] = "Your WiFi SSID";
  char pass[] = "Your WiFi password";

  //Change the following to whatever works on your home network
  IPAddress ip(192, 168, 1, 23);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress dns(192, 168, 1, 1);
  
  WiFi.config(ip, dns, gateway, subnet);
    
  //Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ... ");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void webServerSetup() {
  server.on("/relay", HTTP_GET, handleRelay);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(9600);
  delay(10);
  pinMode(RELAY, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(RELAY, LOW);
  wifiSetup();
  webServerSetup();
}

void loop() {
  server.handleClient();  
}
