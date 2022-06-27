var Socket;
var estado;
var setTemp;
var sensorSet;

  document.getElementById('BTN_LLAMA').addEventListener('click', boton_ajuste_llama);
  function boton_ajuste_llama() {
    sensorSet = document.getElementById("SENSOR_TEXTO").value;
    estado = {sensorSet: sensorSet};
    Socket.send(JSON.stringify(estado));
    console.log(estado);
    console.log(sensorSet);
    console.log(setTemp);
  }

  document.getElementById('BTN_TEMP').addEventListener('click', boton_ajuste_temp);
  function boton_ajuste_temp() {
    setTemp = document.getElementById("TEMP_TEXTO").value;
    estado = {sensorTemp: setTemp};
    Socket.send(JSON.stringify(estado));
    console.log(estado);
    console.log(setTemp);
    console.log(sensorSet);
  }

  function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event) {
      processCommand(event);
    };
  }
  
  function processCommand(event) {
    var obj = JSON.parse(event.data);    
    document.getElementById('rand').innerHTML = obj.sensor;
    document.getElementById('temp').innerHTML = obj.temp;
    document.getElementById('tempVal').innerHTML = obj.tempVal;
    document.getElementById('sensorVal').innerHTML = obj.sensorVal;
    console.log(obj.sensor);
    console.log(obj.temp);
  }
  window.onload = function(event) {
    init();
  }