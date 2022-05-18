//=============================================
//ESP32 WebSocket Server: potentiometer voltage
//=============================================
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
//-----------------------------------------------
const char* ssid = "RESTREPO_R";
const char* password = "43257145";
//-----------------------------------------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//-----------------------------------------------
String JSONtxt;
//-----------------------------------------------
#include "webpage.h"
//-----------------------------------------------
void handleRoot()
{
  server.send(200,"text/html", webpageCont);
}
//====================================================================
void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("."); delay(500);  
  }
  WiFi.mode(WIFI_STA);
  Serial.print(" Local IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.begin(); webSocket.begin();
}
//====================================================================
void loop() 
{
  webSocket.loop(); server.handleClient();

  String POTvalString = String(float(analogRead(A0))/1170.0);
  JSONtxt = "{\"POT\":\""+POTvalString+"\"}";
  webSocket.broadcastTXT(JSONtxt);
}

Header file "webpage.h":

//=====================
//HTML code for webpage
//=====================
const char webpageCont[] PROGMEM = 
R"=====(
<!DOCTYPE HTML>
<html>
<title>ESP32 WebSocket Server</title>
<!---------------------------CSS-------------------------->
<style>
    #dynRectangle
    {
        width:0px;
        height:12px;
        top: 9px;
        background-color: red;
        z-index: -1;
        margin-top:8px;
    }
    body   {background-color:rgba(128,128,128,0.322); font-family:calibri}
    h1     {font-size: 40px; color: black; text-align: center}
    h2     {font-size: 30px; color: blue}
    h3     {font-size: 17px; color:blue}
    div.h1 {background-color: whitesmoke;}
</style>
<!--------------------------HTML-------------------------->
<body>
    <h1><div class="h1">ESP32 WebSocket Server</div></h1>
    <h2>
        POT voltage: <span style="color:rgb(216, 3, 3)" id="POTvalue">0</span> V  
    </h2>
    <h3>
        0V &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
        &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; 3.5V
        <div id="dynRectangle"></div>
    </h3>
</body>
<!----------------------JavaScript------------------------>
<script>
  InitWebSocket()
  function InitWebSocket()
  {
    websock = new WebSocket('ws://'+window.location.hostname+':81/');
    websock.onmessage=function(evt)
    {
       JSONobj = JSON.parse(evt.data);
       document.getElementById('POTvalue').innerHTML = JSONobj.POT;
       var pot = parseInt(JSONobj.POT * 135);
       document.getElementById("dynRectangle").style.width = pot+"px";
    }
  }
</script>
</html>
)=====";