// Browser transport and rendering for the Signal Coach tester preview.
import { parsePulseLine } from './pulse-webserial-protocol.mjs';
import { LegacySignalCoach, adviceFor, stateColor } from './signal-coach-core.mjs';

const HISTORY_LENGTH = 600;
const canvas = document.querySelector('#waveformCanvas');
const context = canvas.getContext('2d');
const display = document.querySelector('.display');
const connectBtn = document.querySelector('#connectBtn');
const statusDot = document.querySelector('#statusDot');
const statusText = document.querySelector('#statusText');
const sourceMeta = document.querySelector('#sourceMeta');
const sourceName = document.querySelector('#sourceName');
const screenSource = document.querySelector('#screenSource');
const errorMessage = document.querySelector('#errorMessage');
const simulationWarning = document.querySelector('#simulationWarning');
const waitingOverlay = document.querySelector('#waitingOverlay');
const heart = document.querySelector('#heart');
const bpmCard = document.querySelector('#bpmCard');
const liveBpm = document.querySelector('#liveBpm');
const bpmNote = document.querySelector('#bpmNote');
const coachLabel = document.querySelector('#coachLabel');
const coachAdvice = document.querySelector('#coachAdvice');
const qualitySegments = [...document.querySelectorAll('#qualityBar span')];
const modeChip = document.querySelector('#modeChip');
const frameMeta = document.querySelector('#frameMeta');
const signalMeta = document.querySelector('#signalMeta');
const liveSignal = document.querySelector('#liveSignal');
const liveBeat = document.querySelector('#liveBeat');
const liveIbi = document.querySelector('#liveIbi');
const liveQuality = document.querySelector('#liveQuality');
const rawSerial = document.querySelector('#rawSerial');

let port = null;
let reader = null;
let reading = false;
let history = new Array(HISTORY_LENGTH).fill(null);
let frameCount = 0;
let gaps = 0;
let lastSequence = null;
let lastFrame = null;
let lastCoach = { state: 'READY', quality: 0, locked: false, range: 0 };
const legacyCoach = new LegacySignalCoach();

function resizeCanvas() {
  const rectangle = canvas.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  canvas.width = Math.max(1, Math.round(rectangle.width * dpr));
  canvas.height = Math.max(1, Math.round(rectangle.height * dpr));
  context.setTransform(dpr, 0, 0, dpr, 0, 0);
  drawWaveform();
}

function drawWaveform() {
  const width = canvas.getBoundingClientRect().width;
  const height = canvas.getBoundingClientRect().height;
  context.fillStyle = '#060a06';
  context.fillRect(0, 0, width, height);

  context.strokeStyle = '#143323';
  context.lineWidth = 1;
  for (let x = 0; x <= width; x += width / 8) {
    context.beginPath(); context.moveTo(x, 0); context.lineTo(x, height); context.stroke();
  }
  for (let y = 0; y <= height; y += height / 4) {
    context.beginPath(); context.moveTo(0, y); context.lineTo(width, y); context.stroke();
  }

  const values = history.filter((value) => value !== null);
  if (values.length < 2) return;
  const minimum = Math.min(...values);
  const maximum = Math.max(...values);
  const padding = Math.max(35, (maximum - minimum) * .13);
  const low = minimum - padding;
  const high = maximum + padding;
  const range = Math.max(1, high - low);
  const color = stateColor(lastCoach.state) === 'green' ? '#6ef58a' : stateColor(lastCoach.state) === 'yellow' ? '#ffe34d' : '#5be7ff';

  context.strokeStyle = color;
  context.lineWidth = 3;
  context.lineJoin = 'round';
  context.beginPath();
  let started = false;
  history.forEach((value, index) => {
    if (value === null) return;
    const x = (index / (HISTORY_LENGTH - 1)) * width;
    const y = height - ((value - low) / range) * height;
    if (!started) { context.moveTo(x, y); started = true; }
    else context.lineTo(x, y);
  });
  context.stroke();
}

function coachFromFrame(frame, now) {
  if (frame.source === 'sticks3') {
    return {
      state: frame.state,
      quality: frame.quality,
      locked: frame.bpm > 0 && frame.quality >= 10,
      range: null,
    };
  }
  return legacyCoach.update(frame, now);
}

function updateCoach(frame, coach) {
  const color = stateColor(coach.state);
  display.classList.remove('state-blue', 'state-yellow', 'state-green');
  display.classList.add(`state-${color}`);
  coachLabel.textContent = coach.state.replaceAll('_', ' ');
  coachAdvice.textContent = adviceFor(coach.state);
  qualitySegments.forEach((segment, index) => segment.classList.toggle('filled', index < Math.floor((coach.quality * 10) / 12)));

  const showBpm = coach.locked && frame.bpm > 0;
  liveBpm.textContent = showBpm ? String(frame.bpm) : '--';
  bpmNote.textContent = showBpm ? 'Stable signal detected' : 'Wait for a clean signal';
  liveQuality.textContent = `${coach.quality}/12`;
}

function updateFrame(frame, line, now = performance.now()) {
  frameCount += 1;
  if (frame.source === 'sticks3' && lastSequence !== null && frame.sequence !== ((lastSequence + 1) & 0x7fffffff)) gaps += 1;
  if (frame.source === 'sticks3') lastSequence = frame.sequence;
  lastFrame = frame;
  lastCoach = coachFromFrame(frame, now);

  history.shift();
  history.push(frame.signal);
  drawWaveform();
  updateCoach(frame, lastCoach);

  const simulated = frame.mode === 'SIM';
  const sticks3 = frame.source === 'sticks3';
  waitingOverlay.hidden = true;
  statusDot.className = `status-dot ${simulated ? 'simulation' : 'connected'}`;
  statusText.textContent = simulated ? 'Simulated bench replay' : 'PulseSensor connected';
  sourceMeta.textContent = sticks3 ? `PulseLink StickS3 · ${frame.mode} · PSWS v${frame.version}` : 'PulseSensor Playground · live four-value stream';
  sourceName.textContent = sticks3 ? (simulated ? 'SIM PULSE' : 'PulseSensor') : 'PulseSensor';
  screenSource.textContent = sticks3 ? 'STICKS3' : 'ARDUINO / ESP32';
  simulationWarning.hidden = !simulated;
  modeChip.textContent = frame.mode;
  modeChip.className = simulated ? 'sim' : 'live';
  frameMeta.textContent = `${lastCoach.state.replaceAll('_', ' ')} · ${frameCount.toLocaleString()} frames · ${gaps} gaps`;
  signalMeta.textContent = `Signal ${frame.signal}`;
  liveSignal.textContent = String(frame.signal);
  liveBeat.textContent = frame.beat ? '1' : '0';
  liveIbi.textContent = frame.ibi > 0 ? String(frame.ibi) : '--';
  rawSerial.textContent = line;

  if (frame.beat) {
    heart.classList.add('beat');
    if (lastCoach.locked) bpmCard.classList.add('invert');
    setTimeout(() => { heart.classList.remove('beat'); bpmCard.classList.remove('invert'); }, 180);
  }
}

function showError(message) {
  errorMessage.textContent = message;
  errorMessage.hidden = false;
}

function resetDisconnectedDisplay() {
  history = new Array(HISTORY_LENGTH).fill(null);
  frameCount = 0;
  gaps = 0;
  lastSequence = null;
  lastFrame = null;
  lastCoach = { state: 'READY', quality: 0, locked: false, range: 0 };
  legacyCoach.reset();
  display.classList.remove('state-yellow', 'state-green');
  display.classList.add('state-blue');
  screenSource.textContent = 'USB';
  waitingOverlay.hidden = false;
  waitingOverlay.textContent = 'CONNECT YOUR BOARD';
  liveBpm.textContent = '--';
  bpmNote.textContent = 'Wait for a clean signal';
  coachLabel.textContent = 'READY';
  coachAdvice.textContent = adviceFor('READY');
  qualitySegments.forEach((segment) => segment.classList.remove('filled'));
  modeChip.textContent = 'WAITING';
  modeChip.className = '';
  frameMeta.textContent = 'No samples received';
  signalMeta.textContent = 'Signal --';
  liveSignal.textContent = '--';
  liveBeat.textContent = '--';
  liveIbi.textContent = '--';
  liveQuality.textContent = '--';
  rawSerial.textContent = 'waiting for data...';
  drawWaveform();
}

async function readLoop() {
  const decoder = new TextDecoder();
  let buffer = '';
  while (reading && port?.readable) {
    reader = port.readable.getReader();
    try {
      while (reading) {
        const { value, done } = await reader.read();
        if (done) break;
        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split(/\r?\n/);
        buffer = lines.pop() ?? '';
        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed) continue;
          rawSerial.textContent = trimmed;
          try {
            const frame = parsePulseLine(trimmed);
            if (frame) updateFrame(frame, trimmed);
          } catch (error) {
            console.warn('Ignored invalid serial frame:', error.message);
          }
        }
      }
    } finally {
      reader.releaseLock();
      reader = null;
    }
  }
}

async function disconnect() {
  reading = false;
  if (reader) await reader.cancel();
  const activePort = port;
  port = null;
  if (activePort) await activePort.close();
  connectBtn.textContent = 'Connect';
  statusDot.className = 'status-dot';
  statusText.textContent = 'Disconnected';
  sourceMeta.textContent = 'USB serial · 115200 baud';
  resetDisconnectedDisplay();
}

async function connect() {
  if (!('serial' in navigator)) {
    showError('Web Serial is unavailable. Use desktop Chrome, Edge, or Brave over HTTPS.');
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200, bufferSize: 8192 });
    history = new Array(HISTORY_LENGTH).fill(null);
    frameCount = 0; gaps = 0; lastSequence = null; legacyCoach.reset();
    reading = true;
    errorMessage.hidden = true;
    connectBtn.textContent = 'Disconnect';
    statusText.textContent = 'Waiting for samples';
    sourceMeta.textContent = 'Connected · USB serial · 115200 baud';
    waitingOverlay.textContent = 'WAITING FOR DATA';
    await readLoop();
  } catch (error) {
    reading = false;
    const failedPort = port;
    port = null;
    if (failedPort) {
      try { await failedPort.close(); } catch { /* Port never opened or already closed. */ }
    }
    connectBtn.textContent = 'Connect';
    if (error.name !== 'NotFoundError') showError(`Connection failed: ${error.message}`);
  }
}

connectBtn.addEventListener('click', () => {
  if (port) disconnect().catch((error) => showError(error.message));
  else connect().catch((error) => showError(error.message));
});

navigator.serial?.addEventListener('disconnect', (event) => {
  if (event.target === port) disconnect().catch(() => {});
});

window.addEventListener('resize', resizeCanvas);
new ResizeObserver(resizeCanvas).observe(canvas);
resizeCanvas();

function startReplay() {
  connectBtn.hidden = true;
  let sequence = 0;
  let quality = 0;
  let beatCount = 0;
  const started = performance.now();
  const timer = setInterval(() => {
    const elapsed = Math.trunc(performance.now() - started);
    const phase = elapsed % 833;
    let pulse = 0;
    if (phase < 55) pulse = phase * 21;
    else if (phase < 145) pulse = 1155 - (phase - 55) * 10;
    else if (phase < 205) pulse = 255 - (phase - 145) * 4;
    else if (phase < 245) pulse = (phase - 205) * 5;
    else if (phase < 315) pulse = 200 - (phase - 245) * 3;
    const signal = Math.max(0, Math.min(4095, 1880 + Math.max(0, pulse)));
    const beat = phase < 20 && (elapsed < 20 || ((elapsed - 20) % 833) > 813) ? 1 : 0;
    if (beat) { beatCount += 1; if (beatCount > 2) quality = Math.min(12, quality + 3); }
    const locked = quality >= 10;
    const state = locked ? 'QUALIFIED' : quality ? 'LOCKING' : 'GOOD_WAVE';
    const frame = { source:'sticks3', version:1, mode:'SIM', sequence, timestampMs:elapsed, signal,
      bpm:locked ? 72 : 0, ibi:locked ? 833 : 0, beat:locked ? beat : 0, quality, state };
    updateFrame(frame, `PSWS,1,SIM,${sequence},${elapsed},${signal},${frame.bpm},${frame.ibi},${frame.beat},${quality},${state}`);
    sequence = (sequence + 1) & 0x7fffffff;
  }, 20);
  return timer;
}

if (new URLSearchParams(location.search).get('bench') === '1') startReplay();

window.__PULSE_COACH_QA__ = {
  inject: (line, now = performance.now()) => {
    const frame = parsePulseLine(line);
    if (frame) updateFrame(frame, line, now);
    return { frame, coach: lastCoach };
  },
  reset: () => { legacyCoach.reset(); history = new Array(HISTORY_LENGTH).fill(null); frameCount = 0; gaps = 0; lastSequence = null; },
  getState: () => ({ frameCount, gaps, lastFrame, coach:lastCoach, canvasWidth:canvas.width, canvasHeight:canvas.height }),
};
