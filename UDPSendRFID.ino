#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//:: Config Wifi Ethernet
#define WIFI_SSID   "SSID"              // WIFI SSID
#define WIFI_PASS   "PASS"              // WIFI PASS
IPAddress WIFI_IP   (192, 168, 0, 50);  // IP
IPAddress WIFI_GW   (192, 168, 0, 1);   // Gateway
IPAddress WIFI_SN   (255, 255, 255, 0); // SubNet
IPAddress WIFI_DNS  (192, 168, 0, 1);   // DNS

//:: Config UDP Server
#define UDP_SERVER_PORT (9000)          // Port UDP
#define UDP_SERVER_IP "192.168.0.10"    // Server IP

//:: Global Variable
WiFiUDP Udp;
char incomingPacket[255];
int ind;
bool NewData = false;
bool NewTag = false;
bool frame1 = false;
bool frame2 = false;
String inData;

// ..:: Function Conectar
void conectar(){
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.config(WIFI_IP, WIFI_DNS, WIFI_GW, WIFI_SN); 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Udp.begin(UDP_SERVER_PORT);
}

// ..:: Function Setup
void setup(){
  Serial.begin(9600);
  Serial.println();
  Serial.printf("Connecting to %s ", WIFI_SSID);
  conectar();
}

//..:: Function Read Serial - ex: 4F001285865E
char getChar(){ // Buffer usado para comparar strings com a saída
  if (Serial.available() && !NewData) {
    Serial.println("Recebendo dados pela serial");
    NewData = true;
    inData = "";
    frame1 = false;
    frame2 = false;
  }
  
  while (Serial.available()) {       //Only run when there is data available
    if (ind < 13){
      char inChar = Serial.read();
      if ((inChar == '4')&&(!frame1)&&(!frame2)){
        frame1 = true;
        inData += inChar;
      }
      if ((inChar == 'F')&&(frame1)&&(!frame2)) frame2 = true;
      if (frame1 && frame2) {
        if (NewData) inData += inChar;
        if (inData.length() == 12) {
          NewData = false;
          NewTag = true;
        }
      }
    }
  }
  if (NewTag){
    Serial.print("inData:");
    Serial.println(inData);
    SendtoServer(inData);
    NewTag = false;
  }
}

// ..:: Function Send UDP
void SendtoServer(String data){
  char datachar[14];
  data.toCharArray(datachar,13);
  Udp.beginPacket(UDP_SERVER_IP, UDP_SERVER_PORT);
  Udp.write(datachar);
  Udp.endPacket();
}

// ..:: Function Loop
void loop(){
  if (WiFi.status() != WL_CONNECTED) { conectar(); }
  if (Serial.available()) getChar();
  int packetSize = Udp.parsePacket();
  if (packetSize){ // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0){
      incomingPacket[len] = 0;
    }
  }
}

