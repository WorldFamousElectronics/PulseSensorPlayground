/*
 * PulseSensor_XIAO_ESP32S3_WiFi_Lib.ino
 * Version: 1.0.0
 * 
 * WiFi Dashboard with library-powered beat detection.
 * Connect to "PulseSensor.com" WiFi - Open 192.168.4.1
 * 
 * Hardware: XIAO ESP32S3, PulseSensor
 * 
 * Wiring:
 *   PulseSensor Signal (Purple) - A0 (GPIO 1)
 *   PulseSensor Power (Red)     - 3.3V
 *   PulseSensor Ground (Black)  - GND
 * 
 * >> https://pulsesensor.com/pages/pulsesensor_xiao_esp32s3
 *
 * Copyright World Famous Electronics LLC - see LICENSE
 * Contributors:
 *   Joel Murphy, https://pulsesensor.com
 *   Yury Gitman, https://pulsesensor.com
 *
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 *
 * This software is not intended for medical use.
 */

#include <PulseSensorPlayground.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* WIFI_NAME = "PulseSensor.com";
const int PULSE_PIN = 1;
const int LED_PIN = 21;

PulseSensorPlayground pulseSensor;
WebServer server(80);
WebSocketsServer webSocket(81);
int clientCount = 0;

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>PulseSensor Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #111;
      color: white;
      min-height: 100vh;
      padding: 20px;
    }
    .container { max-width: 500px; margin: 0 auto; }
    h1 { text-align: center; margin-bottom: 20px; font-size: 1.4em; font-weight: 500; }
    .card {
      background: #222;
      border-radius: 15px;
      padding: 20px;
      margin-bottom: 15px;
    }
    .bpm-display {
      text-align: center;
      padding: 25px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 20px;
    }
    .bpm-value { font-size: 4em; font-weight: bold; }
    .bpm-label { font-size: 1.1em; opacity: 0.7; }
    .bpm-hint { font-size: 0.85em; opacity: 0.5; margin-top: 5px; }
    .heart { font-size: 3em; transition: transform 0.1s; }
    .heart.beat { transform: scale(1.3); }
    .normal { color: #ff4444; }
    .invalid { color: #444; }
    canvas {
      width: 100%;
      height: 180px;
      background: #1a1a1a;
      border-radius: 10px;
    }
    .label { font-size: 0.8em; opacity: 0.6; margin-bottom: 8px; }
    .status { text-align: center; padding: 10px; font-size: 0.85em; opacity: 0.6; }
    .connected { color: #4a4; }
    .disconnected { color: #a44; }
    .footer { text-align: center; margin-top: 20px; font-size: 0.75em; opacity: 0.4; }
    .footer a { color: #ff4444; text-decoration: none; }
  </style>
</head>
<body>
  <div class="container">
    <h1>PulseSensor Dashboard</h1>
    <div class="card bpm-display">
      <span class="heart normal" id="heart">&#9829;</span>
      <div>
        <div class="bpm-value normal" id="bpm">--</div>
        <div class="bpm-label">BPM</div>
        <div class="bpm-hint" id="hint">Place finger on sensor</div>
      </div>
    </div>
    <div class="card">
      <div class="label">Pulse Waveform</div>
      <canvas id="wave"></canvas>
    </div>
    <div class="status" id="status">Connecting...</div>
    <div class="footer">Powered by <a href="https://pulsesensor.com">PulseSensor.com</a></div>
  </div>
  <script>
    const canvas = document.getElementById('wave');
    const ctx = canvas.getContext('2d');
    canvas.width = canvas.offsetWidth;
    canvas.height = 180;
    let data = [];
    const maxPts = 150;

    const ws = new WebSocket('ws://' + location.hostname + ':81');
    ws.onopen = () => {
      document.getElementById('status').textContent = 'Connected';
      document.getElementById('status').className = 'status connected';
    };
    ws.onclose = () => {
      document.getElementById('status').textContent = 'Disconnected';
      document.getElementById('status').className = 'status disconnected';
    };
    ws.onmessage = (e) => {
      const p = e.data.split(',');
      const sig = parseInt(p[0]);
      const bpm = parseInt(p[1]);
      const beat = p[2] === '1';

      const valid = bpm >= 40 && bpm <= 200;
      const bpmEl = document.getElementById('bpm');
      const heartEl = document.getElementById('heart');
      const hintEl = document.getElementById('hint');

      if (valid) {
        bpmEl.textContent = bpm;
        bpmEl.className = 'bpm-value normal';
        heartEl.className = 'heart normal' + (beat ? ' beat' : '');
        hintEl.textContent = 'Reading pulse...';
      } else {
        bpmEl.textContent = '--';
        bpmEl.className = 'bpm-value invalid';
        heartEl.className = 'heart invalid';
        hintEl.textContent = 'Place finger on sensor';
      }
      if (beat && valid) setTimeout(() => heartEl.classList.remove('beat'), 150);

      data.push(sig);
      if (data.length > maxPts) data.shift();
      drawWave();
    };

    function drawWave() {
      ctx.fillStyle = '#1a1a1a';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      if (data.length < 2) return;
      const min = Math.min(...data), max = Math.max(...data), range = max - min || 1;
      ctx.strokeStyle = '#ff4444';
      ctx.lineWidth = 2;
      ctx.beginPath();
      for (let i = 0; i < data.length; i++) {
        const x = (i / maxPts) * canvas.width;
        const y = canvas.height - ((data[i] - min) / range) * (canvas.height - 20) - 10;
        i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
  </script>
</body>
</html>
)rawliteral";

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) clientCount++;
  else if (type == WStype_DISCONNECTED) clientCount = max(0, clientCount - 1);
}

void setup() {
  Serial.begin(115200);
  Serial.println("PulseSensor_XIAO_ESP32S3_WiFi_Lib.ino");
  Serial.println("PulseSensor.com");

  // Initialize PulseSensor
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();

  // Start WiFi
  WiFi.softAP(WIFI_NAME);
  server.on("/", []() { server.send_P(200, "text/html", DASHBOARD_HTML); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  Serial.println("WiFi Started: PulseSensor.com");
  Serial.println("Open: 192.168.4.1");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Get data from library
  int signal = pulseSensor.getLatestSample();
  int bpm = pulseSensor.getBeatsPerMinute();
  bool beat = pulseSensor.sawStartOfBeat();

  // Send to connected browsers
  String data = String(signal) + "," + String(bpm) + "," + (beat ? "1" : "0");
  webSocket.broadcastTXT(data);

  delay(10);
}
