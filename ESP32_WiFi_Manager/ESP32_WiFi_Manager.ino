/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

//Alexa
//#include <Espalexa.h>

int chimeneabajo = 16;
int chimeneaalto = 17;

/////Espalexa alexita;

void Funcionchimeneabajo(uint8_t brightness);
void Funcionchimeneaalto(uint8_t brightness);

//WiFiServer servidor(90);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Las siguientes variables se utilizan para buscar el SSID, la contraseña, la dirección IP y la puerta de enlace
// en la solicitud HTTP POST realizada cuando se envía el formulario.
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";

// los ssid, pass, ip, y gateway guardan los valores del SSID,
// la contraseña, la dirección IP y la puerta de enlace enviada en el formulario.
String ssid;
String pass;
String ip;
String gateway;

// El SSID, la contraseña, la dirección IP y la puerta de enlace cuando se envían
// se guardan en archivos en el sistema de archivos ESP. Las siguientes variables se refieren a la ruta de esos archivos.
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

/*La dirección IP y la puerta de enlace de la estación se envían en el formulario de Wi-Fi Manager.
La subred está codificada, pero puede modificar fácilmente este proyecto con otro campo para incluir la subred, si es necesario.*/

IPAddress localIP;
// IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
// IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Variables de tiempo
unsigned long previousMillis = 0;
const long interval = 10000; // Es usada como tiempo de espera de la conexion WiFi

// Pines GPIO
const int ledPin = 2;
const int ledPin0 = 0;
const int ledPin4 = 4;

// Almacena el estado de la salida del pin GPIO
String ledState;
String conteo;
String inputMessage;
int inputParam;
bool encendida = false; // Almacena el estado de la chimenea
// Variables de tiempo de actualizacion estado de cuenta regresiva
int periodo = 1000;
unsigned long TiempoAhora = 0;

const char *PARAM_INPUT_11 = "input1";

// Inicializacion del SPIFFS "SPI Flash File System"
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("Ocurrio un error mientras se iniciaba el sistema SPIFFS");
  }
  Serial.println("SPIFFS Iniciado Correctamente");
}

// Leyendo archivos desde el sistema SPIFFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Grabando archivo en sistema SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- frite failed");
  }
}

// Inicializacion de WiFi
bool initWiFi()
{
  if (ssid == "" || ip == "")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
  {
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

// Reemplaza el marcador de estado con el valor actual
String processor(const String &var)
{
  if (var == "STATE")
  {
    if (digitalRead(ledPin) || digitalRead(ledPin0))
    {
      ledState = "ON";
    }
    else
    {
      ledState = "OFF";
    }
    return ledState;
  }
  else if (var == "CONTEO")
  {
    return conteo;
  }
  return String();
}

void setup()
{
  // Velocidad de transferencia del puerto serial
  Serial.begin(115200);
  //Alexa
  pinMode(chimeneabajo, OUTPUT);
  pinMode(chimeneaalto, OUTPUT);
  //alexita.addDevice("chimenea en bajo", Funcionchimeneabajo);
  //alexita.addDevice("chimenea en alto", Funcionchimeneaalto);
  //alexita.begin();
  //FinAlexa

  initSPIFFS();

  // Set GPIO as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(ledPin0, OUTPUT);
  digitalWrite(ledPin0, LOW);
  pinMode(ledPin4, OUTPUT);
  digitalWrite(ledPin4, LOW);

  // Comienza a leer los archivos para obtener el SSID, la contraseña,
  // la dirección IP y la puerta de enlace previamente guardados.
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile(SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi())
  {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.serveStatic("/", SPIFFS, "/");

    // Experimento de entrada de datos
    server.on("/onAlt", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(ledPin0, HIGH);
      digitalWrite(ledPin, HIGH);
      encendida = true;
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(ledPin0, HIGH);
      digitalWrite(ledPin, HIGH);
      encendida = true;
      if (request->hasParam(PARAM_INPUT_11))
      {
        inputMessage = request->getParam(PARAM_INPUT_11)->value();
        inputParam = inputMessage.toInt();
        Serial.println("entradaString");
        Serial.println(inputMessage);
        Serial.println("entradaString");
      }
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.serveStatic("/", SPIFFS, "/");
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.serveStatic("/", SPIFFS, "/");

    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(ledPin, HIGH);
      digitalWrite(ledPin0, LOW);
      encendida = true;
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(ledPin, LOW);
      digitalWrite(ledPin0, LOW);
      encendida = false;
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.begin();
  }
  else
  {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("CALIDEZ", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wifimanager.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });
    server.begin();
  }
}

void loop()
{
  //alexita.loop();

  if (encendida)
  {
    if (millis() > TiempoAhora + periodo)
    {
      TiempoAhora = millis();
      inputParam = inputParam - 1;
      conteo = (String)inputParam;
      Serial.println(conteo);
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    }
    if (encendida && inputParam <= 0)
    {
      digitalWrite(ledPin, LOW);
      digitalWrite(ledPin0, LOW);
      encendida = false;
      inputParam = 0;
    }
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