var Socket;
var estado;
var setTemp;
var sensorSet;

  document.getElementById('BTN_BAJO').addEventListener('click', boton_bajo);
  function boton_bajo() {
    estado = {estado: 'on'};
    Socket.send(JSON.stringify(estado));
    console.log(estado);
  }

  document.getElementById('BTN_ALTO').addEventListener('click', boton_alto);
  function boton_alto() {
    estado = {estado: 'alto'};
    Socket.send(JSON.stringify(estado));
    console.log(estado);
  }

  document.getElementById('BTN_OFF').addEventListener('click', boton_off);
  function boton_off() {
    estado = {estado: 'off'};
    Socket.send(JSON.stringify(estado));
    console.log(estado);
  }

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