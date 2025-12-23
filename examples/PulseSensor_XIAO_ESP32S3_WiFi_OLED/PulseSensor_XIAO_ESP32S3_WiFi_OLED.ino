/*
 * PulseSensor_XIAO_ESP32S3_WiFi_OLED.ino
 * Version: 1.0.0
 * 
 * WiFi Dashboard + OLED + Library-powered beat detection.
 * Connect to "PulseSensor.com" WiFi - Open 192.168.4.1
 * Watch your pulse on OLED AND phone simultaneously!
 * 
 * Hardware: XIAO ESP32S3, PulseSensor, 1.3" SH1106 OLED
 * 
 * Wiring:
 *   PulseSensor Signal (Purple) - A0 (GPIO 1)
 *   PulseSensor Power (Red)     - 3.3V
 *   PulseSensor Ground (Black)  - GND
 *   OLED SDA                    - D4 (GPIO 5)
 *   OLED SCL                    - D5 (GPIO 6)
 *   OLED VCC                    - 3.3V
 *   OLED GND                    - GND
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

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

const char *WIFI_NAME = "PulseSensor.com";
#define PULSE_PIN 1
#define SDA_PIN 5
#define SCL_PIN 6
#define THRESHOLD 550
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

WebServer server(80);
WebSocketsServer webSocket(81);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
PulseSensorPlayground pulseSensor;

int clientCount = 0;
int BPM = 0;
bool beatDetected = false;
unsigned long lastBeatTime = 0;

#define WAVE_WIDTH 50
int waveBuffer[WAVE_WIDTH];
int waveIndex = 0;

static const unsigned char PROGMEM heartSmall[] = {
    0b01101100, 0b11111110, 0b11111110, 0b01111100,
    0b00111000, 0b00010000, 0b00000000};

static const unsigned char PROGMEM heartBig[] = {
    0b01101100, 0b11111110, 0b11111110, 0b11111110,
    0b01111100, 0b00111000, 0b00010000};

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>PulseSensor (Lib)</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: -apple-system, BlinkMacSystemFont, sans-serif; background: #111; color: white; min-height: 100vh; padding: 20px; }
    .container { max-width: 500px; margin: 0 auto; }
    h1 { text-align: center; margin-bottom: 20px; font-size: 1.4em; }
    .card { background: #222; border-radius: 15px; padding: 20px; margin-bottom: 15px; }
    .bpm-display { text-align: center; padding: 25px; display: flex; align-items: center; justify-content: center; gap: 20px; }
    .bpm-value { font-size: 4em; font-weight: bold; }
    .bpm-label { font-size: 1.1em; opacity: 0.7; }
    .bpm-hint { font-size: 0.85em; opacity: 0.5; margin-top: 5px; }
    .heart { font-size: 3em; transition: transform 0.1s; }
    .heart.beat { transform: scale(1.3); }
    .normal { color: #ff4444; }
    .invalid { color: #444; }
    canvas { width: 100%; height: 180px; background: #1a1a1a; border-radius: 10px; }
    .label { font-size: 0.8em; opacity: 0.6; margin-bottom: 8px; }
    .raw-value { text-align: center; font-size: 0.85em; opacity: 0.5; margin-top: 10px; }
    .status { text-align: center; padding: 10px; font-size: 0.85em; opacity: 0.6; }
    .connected { color: #4a4; }
    .disconnected { color: #a44; }
    .footer { text-align: center; margin-top: 20px; font-size: 0.75em; opacity: 0.4; }
    .footer a { color: #ff4444; text-decoration: none; }
  </style>
</head>
<body>
  <div class="container">
    <h1>PulseSensor (Library)</h1>
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
      <div class="raw-value">Signal: <span id="raw">--</span></div>
    </div>
    <div class="status" id="status">Connecting...</div>
    <div class="footer"><a href="https://pulsesensor.com">pulsesensor.com</a></div>
  </div>
  <script>
    const wave = document.getElementById('wave'), waveCtx = wave.getContext('2d');
    wave.width = wave.offsetWidth; wave.height = 180;
    let data = [], maxPts = 150, valid = false, bpm = 0;
    const ws = new WebSocket('ws://' + location.hostname + ':81');
    ws.onopen = () => { document.getElementById('status').textContent = 'Connected'; document.getElementById('status').className = 'status connected'; };
    ws.onclose = () => { document.getElementById('status').textContent = 'Disconnected'; document.getElementById('status').className = 'status disconnected'; };
    ws.onmessage = (e) => {
      const p = e.data.split(','), sig = parseInt(p[0]); bpm = parseInt(p[1]); const beat = p[2] === '1';
      valid = bpm >= 40 && bpm <= 200;
      document.getElementById('raw').textContent = sig;
      const bpmEl = document.getElementById('bpm'), heartEl = document.getElementById('heart'), hintEl = document.getElementById('hint');
      if (valid) { bpmEl.textContent = bpm; bpmEl.className = 'bpm-value normal'; heartEl.className = 'heart normal' + (beat ? ' beat' : ''); hintEl.textContent = 'Reading...'; }
      else { bpmEl.textContent = '--'; bpmEl.className = 'bpm-value invalid'; heartEl.className = 'heart invalid'; hintEl.textContent = 'Place finger on sensor'; }
      if (beat && valid) setTimeout(() => heartEl.classList.remove('beat'), 150);
      data.push(sig); if (data.length > maxPts) data.shift();
      waveCtx.fillStyle = '#1a1a1a'; waveCtx.fillRect(0, 0, wave.width, wave.height);
      if (data.length > 1) {
        const min = Math.min(...data), max = Math.max(...data), range = max - min || 1;
        waveCtx.strokeStyle = valid ? '#ff4444' : '#444'; waveCtx.lineWidth = 2; waveCtx.beginPath();
        for (let i = 0; i < data.length; i++) { const x = (i / maxPts) * wave.width, y = wave.height - ((data[i] - min) / range) * (wave.height - 20) - 10; i === 0 ? waveCtx.moveTo(x, y) : waveCtx.lineTo(x, y); }
        waveCtx.stroke();
      }
    };
  </script>
</body>
</html>
)rawliteral";

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) clientCount++;
  else if (type == WStype_DISCONNECTED) clientCount = max(0, clientCount - 1);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);

  unsigned long secs = millis() / 1000;
  unsigned long mins = secs / 60;
  secs = secs % 60;
  display.setCursor(90, 0);
  if (mins < 10) display.print("0");
  display.print(mins);
  display.print(":");
  if (secs < 10) display.print("0");
  display.print(secs);

  display.setCursor(0, 0);
  display.print("PulseSensor.com");

  display.setCursor(0, 12);
  display.print("Open: 192.168.4.1");

  display.setCursor(0, 24);
  display.print("Clients: ");
  display.print(clientCount);

  display.setCursor(0, 36);
  display.print("BPM: ");
  bool validBPM = BPM >= 40 && BPM <= 200 && millis() - lastBeatTime < 3000;
  if (validBPM) display.print(BPM);
  else display.print("--");

  bool recentBeat = millis() - lastBeatTime < 150;
  if (recentBeat) display.drawBitmap(50, 35, heartBig, 8, 7, SH110X_WHITE);
  else display.drawBitmap(50, 36, heartSmall, 8, 7, SH110X_WHITE);

  for (int i = 0; i < WAVE_WIDTH - 1; i++) {
    int idx = (waveIndex + i) % WAVE_WIDTH;
    int idxNext = (waveIndex + i + 1) % WAVE_WIDTH;
    display.drawLine(64 + i, 52 + waveBuffer[idx], 64 + i + 1, 52 + waveBuffer[idxNext], SH110X_WHITE);
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println("PulseSensor_XIAO_ESP32S3_WiFi_OLED.ino");
  Serial.println("PulseSensor.com");

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(0x3C, true)) {
    Serial.println("OLED not found!");
    while (1) delay(100);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 15);
  display.println("XIAO ESP32S3 Lib");
  display.setCursor(20, 30);
  display.println("PulseSensor.com");
  display.setCursor(30, 45);
  display.println("(Library)");
  display.display();
  delay(2000);

  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();

  WiFi.softAP(WIFI_NAME);

  server.on("/", []() { server.send_P(200, "text/html", DASHBOARD_HTML); });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  analogReadResolution(12);

  for (int i = 0; i < WAVE_WIDTH; i++) waveBuffer[i] = 4;

  Serial.println("=================================");
  Serial.println("  PulseSensor_XIAO_ESP32S3_WiFi_OLED.ino");
  Serial.println("  PulseSensor.com");
  Serial.print("  WiFi: "); Serial.println(WIFI_NAME);
  Serial.println("  Open: 192.168.4.1");
  Serial.println("=================================");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  int sensorValue = pulseSensor.getLatestSample();

  beatDetected = false;
  if (pulseSensor.sawStartOfBeat()) {
    beatDetected = true;
    lastBeatTime = millis();
    BPM = pulseSensor.getBeatsPerMinute();
  }

  static int sensorHigh = 0, sensorLow = 4095;
  if (sensorValue > sensorHigh) sensorHigh = sensorValue;
  if (sensorValue < sensorLow) sensorLow = sensorValue;

  int waveY = map(sensorValue, sensorLow, sensorHigh, 8, 0);
  waveY = constrain(waveY, 0, 8);
  waveBuffer[waveIndex] = waveY;
  waveIndex = (waveIndex + 1) % WAVE_WIDTH;

  String data = String(sensorValue) + "," + String(BPM) + "," +
                (beatDetected ? "1" : "0") + "," + String(clientCount);
  webSocket.broadcastTXT(data);

  updateDisplay();

  delay(10);
}
