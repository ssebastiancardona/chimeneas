// config_wifimanager.ino
//
 
#if defined(ESP8266)
#include <ESP8266WiFi.h> //ESP8266WiFi.h .- ESP8266 Core WiFi Library
#else
#include <WiFi.h>  //WiFi.h .- ESP32 Core WiFi Library
#endif
 
#if defined(ESP8266)
 
#include <ESP8266WebServer.h> //ESP8266WebServer.h .- Servidor web local utilizado para servir el portal de configuración
 
#else
 
#include <WebServer.h> //WebServer.h .- Servidor DNS local utilizado para redireccionar todas las solicitudes al portal de configuración (https://github.com/zhouhan0126/DNSServer---esp32)
#endif
 
#include <DNSServer.h>//DNSServer.h .- Local WebServer usado para servir el portal de configuración (https://github.com/zhouhan0126/DNSServer---esp32)
#include <WiFiManager.h> //WiFiManager.h .- WiFi Configuration Magic (https://github.com/zhouhan0126/DNSServer---esp32) >> https://github.com/zhouhan0126/DNSServer---esp32 (ORIGINAL)
 
const int PIN_AP = 2; // pulsador para volver al modo AP
 
void configModeCallback (WiFiManager *myWiFiManager) {
 Serial.println("Modo de configuración ingresado");
 Serial.println(WiFi.softAPIP());
 
 Serial.println(myWiFiManager->getConfigPortalSSID());
}
 
//flag for saving data
bool shouldSaveConfig = false;
 
// En https://github.com/tzapu/WiFiManager
//callback notifying us of the need to save config
 
void saveConfigCallback () {
 Serial.println("Debería guardar la configuración");
 shouldSaveConfig = true;
}
 
void setup() {
 Serial.begin(9600);
 pinMode(PIN_AP, INPUT);
 //declaración de objeto wifiManager
 WiFiManager wifiManager;
 
 // utilizando ese comando, como las configuraciones se apagarán en la memoria
 // en caso de que la redacción se conecte automáticamente, ella é apagada.
 // wifiManager.resetSettings();
 
 //devolución de llamada para cuando entra en el modo de configuración AP
 wifiManager.setAPCallback(configModeCallback);
 //devolución de llamada cuando se conecta a una red, es decir, cuando pasa a trabajar en modo EST
 wifiManager.setSaveConfigCallback(saveConfigCallback);
 
 //crea una red de nombre ESP_AP con pass 12345678
 wifiManager.autoConnect("ESP_AP", "12345678");
}
 
void loop() {
 
 WiFiManager wifiManager;
 //si el botón se ha presionado
 if ( digitalRead(PIN_AP) == HIGH ) {
  Serial.println("reajustar"); //resetear intenta abrir el portal
  if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
   Serial.println("No se pudo conectar");
   delay(2000);
   ESP.restart();
   delay(1000);
  }
  Serial.println("conectado ESP_AP!!!");
 }
}