#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <Espalexa.h>

int chimeneabajo = 5;
int chimeneaalto = 0;

const char* ssid = "SEBAS";
const char* password = "1128275193";

Espalexa alexita;

void Funcionchimeneabajo(uint8_t brightness);
void Funcionchimeneaalto(uint8_t brightness);

WiFiServer servidor(80);

//IPAddress ip_local(192,168,1,88);
//IPAddress gateway(192,168,1,254);
//IPAddress subnet(255,255,255,0);

void setup() {
  Serial.begin(115200);
  pinMode(chimeneabajo, OUTPUT);
  pinMode(chimeneaalto, OUTPUT);
  /*if (!WiFi.config(ip_local, gateway, subnet)){
    Serial.println("Error en configuracion");
  }*/
  ConectarWifi();
  alexita.addDevice("chimenea en bajo", Funcionchimeneabajo);
  alexita.addDevice("chimenea en alto", Funcionchimeneaalto);
  alexita.begin();
}

void loop() {
  ConectarWifi();
  alexita.loop();
  delay(200);
}

void ConectarWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) { 
      delay(1000);  
      Serial.print(".");
      Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    }
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void Funcionchimeneabajo(uint8_t brightness) {
  
  if (brightness) {
    digitalWrite(chimeneabajo, HIGH);
    Serial.println(" Chimenea en bajo encendida ");
  }
  else {
    digitalWrite(chimeneabajo, LOW);
    Serial.println(" Chimenea en bajo apagada ");
  }
}

void Funcionchimeneaalto(uint8_t brightness) {
  
   if (brightness) {
    digitalWrite(chimeneaalto, HIGH);
    Serial.println(" Chimenea en alto encendida ");
  }
  else {
    digitalWrite(chimeneaalto, LOW);
    Serial.println(" Chimenea en alto apagada ");
  }

}
