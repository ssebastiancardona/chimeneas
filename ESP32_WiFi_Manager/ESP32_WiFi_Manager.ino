/*********
 - Molestias de interferencia electromagnetica
 - Sensor de Temperatura con ajuste desde la paguina.
 - Esteresis (Ajuste de encendido a partir de la temperatura con margen variable)

 /// Secuencia de encendido ///

 - 3 segundos de chispa X 1 segundo apagado (la secuencia se repite 3) si no arranca apagado total
 - En esa secuenca abre primero la valvula de baja, siemrpe arranca en bajo.
 - Cuando se pasa a alto se abre la segunda electro valvula

 /// Funciones suiche de encendido ///

 - Pulsador 1, se pulsa una vez y enciende en bajo, se espera a que estabilice y si se pulsa de nuevo sube a llama alta
 - Pulsador 2, apaga la chimenea
 - Suiche apagado total

*********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <WebSocketsServer.h> // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>
#include <EEPROM.h>

#define EEPROM_SIZE 4000
char direccion0 = 0;
const int direccion1 = 1;
// Alexa
//#include <Espalexa.h>

int chimeneabajo = 34;
int chimeneaalto = 17;

/////Espalexa alexita;

void Funcionchimeneabajo(uint8_t brightness);
void Funcionchimeneaalto(uint8_t brightness);

// WiFiServer servidor(90);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets
StaticJsonDocument<200> doc_tx;
StaticJsonDocument<200> doc_rx;

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
const long interval = 1000; // Es usada como tiempo de espera de la conexion WiFi
const long interval1 = 1000;

// Pines GPIO
const int GasBajo = 23; // Apertura valvula baja de Gas
const int Chispa = 22;  // Encendido chispa
const int GasAlto = 21; // Apertura valvula alta de Gas

const int ContBaj = 16; // Encendido Bajo Chimenea
const int ContAlt = 4;  // Encendido Alto Chimenea
const int ContAp = 0;   // Apagado Chimenea

const int Sensor = 34; // Sensor de llama
double temperatura;    // Temperatura enviada por el sensor MCP9700 al pin
int sensorVal = 0;     // Valor seteado por el usuario
int tempVal = 0;       // Valor seteado por el usuario

int sensorVal1 = 0;
int tempVal1 = 0;
int intentos = 0;

bool iniciarCicloEncendido;
bool blink;

int lectura = 0;

// Almacena el estado de la salida del pin GPIO
String ledState;
String conteo;
String inputMessage;
int inputParam;
bool encendida = false; // Almacena el estado de la chimenea
// Variables de tiempo de actualizacion estado de cuenta regresiva
bool estadoAlto = false; // Almacena el estado actual de la chimenea (Bajo = false, Alto = True)
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
    if (currentMillis - previousMillis >= 10000)
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
    if (digitalRead(GasBajo) || digitalRead(GasAlto))
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
  EEPROM.begin(EEPROM_SIZE);
  initSPIFFS();

  // Alexa
  pinMode(chimeneabajo, OUTPUT);
  pinMode(chimeneaalto, OUTPUT);
  // alexita.addDevice("chimenea en bajo", Funcionchimeneabajo);
  // alexita.addDevice("chimenea en alto", Funcionchimeneaalto);
  // alexita.begin();
  // FinAlexa

  // Set GPIO as an OUTPUT
  pinMode(GasBajo, OUTPUT);
  digitalWrite(GasBajo, LOW);
  pinMode(Chispa, OUTPUT);
  digitalWrite(Chispa, LOW);
  pinMode(GasAlto, OUTPUT);
  digitalWrite(GasAlto, LOW);

  pinMode(ContBaj, INPUT);
  pinMode(ContAlt, INPUT);
  pinMode(ContAp, INPUT);

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
      digitalWrite(GasAlto, HIGH);
      digitalWrite(GasBajo, HIGH);
      encendida = true;
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(GasAlto, HIGH);
      digitalWrite(GasBajo, HIGH);
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
      digitalWrite(GasBajo, HIGH);
      digitalWrite(GasAlto, LOW);
      encendida = true;
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      digitalWrite(GasBajo, LOW);
      digitalWrite(GasAlto, LOW);
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
  webSocket.begin();                 // start websocket
  webSocket.onEvent(webSocketEvent); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
}

void loop()
{
  webSocket.loop(); // Update function for the webSockets
  sensorVal1 = EEPROM.read(direccion0) * 13;
  tempVal1 = EEPROM.read(direccion1);
  lectura = analogRead(Sensor);

  unsigned long now = millis(); // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  if ((unsigned long)(now - previousMillis) >= interval)
  { // check if "interval" ms has passed since last time the clients were updated
    temperatura = analogRead(35) * 3.3 / 4095;
    temperatura = temperatura - 0.5;
    temperatura = temperatura / 0.01;

    String jsonString = "";
    JsonObject object = doc_tx.to<JsonObject>();
    object["sensor"] = lectura;
    object["temp"] = temperatura;
    object["sensorVal"] = sensorVal1;
    object["tempVal"] = tempVal1;
    serializeJson(doc_tx, jsonString);
    // Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    previousMillis = now;
    if (blink == true && iniciarCicloEncendido == true)
    {
      digitalWrite(Chispa, blink);
      blink = false;
      return;
    }
    else if (blink == false)
    {
      digitalWrite(Chispa, blink);
      blink = true;
      intentos++;
    }
  }
  if(intentos >=10){
    digitalWrite(GasBajo, LOW);
    digitalWrite(GasAlto, LOW);
    encendida = false;
    iniciarCicloEncendido = false;
    blink = false;
    intentos = 0;
  }

  if (encendida == true){
    digitalWrite(GasBajo, HIGH);
    if(lectura > sensorVal1){
      iniciarCicloEncendido = true;
    }else if(lectura < sensorVal1){
      iniciarCicloEncendido = false;
    }
  }
}

// "num"-Es un numero asignado al cliente. "type"-
void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{ // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
  switch (type)
  {                         // switch on the type of information sent
  case WStype_DISCONNECTED: // if a client is disconnected, then type == WStype_DISCONNECTED
    Serial.println("Client " + String(num) + " disconnected");
    break;
  case WStype_CONNECTED: // if a client is connected, then type == WStype_CONNECTED
    Serial.println("Client " + String(num) + " connected");
    // optionally you can add code here what to do when connected
    break;
  case WStype_TEXT: // if a client has sent data, then type == WStype_TEXT
    DeserializationError error = deserializeJson(doc_rx, payload);
    if (error)
    {
      Serial.print("deserializeJson() failed");
      return;
    }
    else
    {
      const char *estadoEnviado = doc_rx["estado"];
      String estado = String(estadoEnviado);

      const char *sensorSet = doc_rx["sensorSet"];
      sensorVal = String(sensorSet).toInt();
      const char *setTemp = doc_rx["sensorTemp"];
      tempVal = String(setTemp).toInt();

      if (sensorVal != 0)
      {
        EEPROM.write(direccion0, sensorVal); // EEPROM.put(address, boardId);
        delay(10);
        EEPROM.commit();
        sensorVal1 = EEPROM.read(direccion0);
        Serial.println("Entro al if SensorVal y su valor es: " + String(sensorVal1));
      }

      if (tempVal != 0)
      {
        EEPROM.write(direccion1, tempVal); // EEPROM.put(address, boardId);
        delay(10);
        EEPROM.commit();
        tempVal1 = EEPROM.read(direccion1);
        Serial.println("Entro al if tempVal y su valor es: " + String(tempVal1));
      }

      if (estado == "on")
      {
        encendida = true;
        digitalWrite(GasAlto, LOW);
        Serial.print("El estado de la chimenea es: ");
        Serial.println(encendida);
      }

      if (estado == "alto")
      {
        encendida = true;
        digitalWrite(GasBajo, HIGH);
        digitalWrite(GasAlto, HIGH);
        Serial.println("El estado de la chimenea es: ");
        Serial.print(encendida);
        Serial.println("El estado de la llama es: ");
        Serial.print(estadoAlto);
      }

      if (estado == "off")
      {
        encendida = false;
        iniciarCicloEncendido = false;
        digitalWrite(GasAlto, LOW);
        digitalWrite(GasBajo, LOW);
        intentos = 0;       
        Serial.println("El estado de la chimenea es:" + String(encendida));
      }
    }
    break;
  }
}

void Funcionchimeneabajo(uint8_t brightness)
{

  if (brightness)
  {
    digitalWrite(chimeneabajo, HIGH);
    Serial.println(" Chimenea en bajo encendida ");
  }
  else
  {
    digitalWrite(chimeneabajo, LOW);
    Serial.println(" Chimenea en bajo apagada ");
  }
}

void Funcionchimeneaalto(uint8_t brightness)
{

  if (brightness)
  {
    digitalWrite(chimeneaalto, HIGH);
    Serial.println(" Chimenea en alto encendida ");
  }
  else
  {
    digitalWrite(chimeneaalto, LOW);
    Serial.println(" Chimenea en alto apagada ");
  }
}
