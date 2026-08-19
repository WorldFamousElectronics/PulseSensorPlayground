import { parsePulseTransitTimeLine } from './pulse-transit-time-protocol.mjs?v=20260819-ptt-r1';
import { PulseTransitTimeCoach } from './pulse-transit-time-core.mjs?v=20260819-ptt-r1';

const pageOptions = new URLSearchParams(location.search);
if (window.self !== window.top || pageOptions.get('embedded') === '1') {
  document.documentElement.classList.add('is-embedded');
}

const HISTORY_LENGTH = 600;
const connectBtn = document.querySelector('#connectBtn');
const statusDot = document.querySelector('#statusDot');
const statusText = document.querySelector('#statusText');
const sourceMeta = document.querySelector('#sourceMeta');
const errorMessage = document.querySelector('#errorMessage');
const simulationWarning = document.querySelector('#simulationWarning');
const proximalCanvas = document.querySelector('#proximalCanvas');
const distalCanvas = document.querySelector('#distalCanvas');
const proximalThresholdMode = document.querySelector('#proximalThresholdMode');
const distalThresholdMode = document.querySelector('#distalThresholdMode');
const proximalThreshold = document.querySelector('#proximalThreshold');
const distalThreshold = document.querySelector('#distalThreshold');
const minimumPtt = document.querySelector('#minimumPtt');
const maximumPtt = document.querySelector('#maximumPtt');
const proximalQuality = document.querySelector('#proximalQuality');
const distalQuality = document.querySelector('#distalQuality');
const proximalMeta = document.querySelector('#proximalMeta');
const distalMeta = document.querySelector('#distalMeta');
const latestPtt = document.querySelector('#latestPtt');
const medianPtt = document.querySelector('#medianPtt');
const acceptedPairs = document.querySelector('#acceptedPairs');
const rejectedPairs = document.querySelector('#rejectedPairs');
const pttGuidance = document.querySelector('#pttGuidance');
const modeChip = document.querySelector('#modeChip');
const frameMeta = document.querySelector('#frameMeta');
const signalMeta = document.querySelector('#signalMeta');
const rawSerial = document.querySelector('#rawSerial');
const copySketchBtn = document.querySelector('#copySketchBtn');
const copySketchStatus = document.querySelector('#copySketchStatus');
const unoR4Sketch = document.querySelector('#unoR4Sketch');
const settingsControls = [proximalThresholdMode, distalThresholdMode, proximalThreshold, distalThreshold, minimumPtt, maximumPtt];

let port = null;
let reader = null;
let readTask = null;
let reading = false;
let coach = new PulseTransitTimeCoach();
let proximalHistory = new Array(HISTORY_LENGTH).fill(null);
let distalHistory = new Array(HISTORY_LENGTH).fill(null);
let lastResult = null;
let lastFrame = null;
let frameCount = 0;
let streamTimestamp = null;
let lastDeviceTimestamp = null;
let lastRenderAt = -Infinity;

function qualityLabel(target, quality) {
  target.textContent = quality.state;
  target.className = quality.state === 'GOOD' ? 'good' : quality.state === 'CLIPPED' ? 'clipped' : '';
}

function drawWaveform(target, values, threshold, color) {
  const rectangle = target.getBoundingClientRect();
  if (!rectangle.width || !rectangle.height) return;
  const dpr = window.devicePixelRatio || 1;
  const width = Math.max(1, Math.round(rectangle.width * dpr));
  const height = Math.max(1, Math.round(rectangle.height * dpr));
  const drawing = target.getContext('2d');
  if (target.width !== width || target.height !== height) {
    target.width = width;
    target.height = height;
    drawing.setTransform(dpr, 0, 0, dpr, 0, 0);
  }
  const cssWidth = rectangle.width;
  const cssHeight = rectangle.height;
  drawing.fillStyle = '#fff';
  drawing.fillRect(0, 0, cssWidth, cssHeight);
  drawing.strokeStyle = '#dce7df';
  drawing.lineWidth = 1;
  for (let index = 1; index < 4; index += 1) {
    drawing.beginPath();
    drawing.moveTo(0, (cssHeight * index) / 4);
    drawing.lineTo(cssWidth, (cssHeight * index) / 4);
    drawing.stroke();
  }
  const present = values.filter((value) => value !== null);
  if (present.length < 2) return;
  const low = Math.max(0, Math.min(...present, threshold) - 35);
  const high = Math.min(1023, Math.max(...present, threshold) + 35);
  const range = Math.max(1, high - low);
  const thresholdY = cssHeight - ((threshold - low) / range) * cssHeight;
  drawing.save();
  drawing.setLineDash([6, 5]);
  drawing.strokeStyle = '#a12626';
  drawing.beginPath();
  drawing.moveTo(0, thresholdY);
  drawing.lineTo(cssWidth, thresholdY);
  drawing.stroke();
  drawing.restore();
  drawing.strokeStyle = color;
  drawing.lineWidth = 2.5;
  drawing.lineJoin = 'round';
  drawing.beginPath();
  let started = false;
  values.forEach((value, index) => {
    if (value === null) return;
    const x = (index / (HISTORY_LENGTH - 1)) * cssWidth;
    const y = cssHeight - ((value - low) / range) * cssHeight;
    if (!started) {
      drawing.moveTo(x, y);
      started = true;
    } else {
      drawing.lineTo(x, y);
    }
  });
  drawing.stroke();
}

function resizeCanvases() {
  drawWaveform(proximalCanvas, proximalHistory, lastResult?.proximal?.threshold ?? Number(proximalThreshold.value), '#087e91');
  drawWaveform(distalCanvas, distalHistory, lastResult?.distal?.threshold ?? Number(distalThreshold.value), '#997800');
}

function applySettings() {
  coach.configureChannel('proximal', proximalThresholdMode.value, Number(proximalThreshold.value));
  coach.configureChannel('distal', distalThresholdMode.value, Number(distalThreshold.value));
  coach.setPairingWindow(Number(minimumPtt.value), Number(maximumPtt.value));
}

function clearSession() {
  coach = new PulseTransitTimeCoach();
  proximalHistory = new Array(HISTORY_LENGTH).fill(null);
  distalHistory = new Array(HISTORY_LENGTH).fill(null);
  lastResult = null;
  lastFrame = null;
  frameCount = 0;
  streamTimestamp = null;
  lastDeviceTimestamp = null;
  lastRenderAt = -Infinity;
  qualityLabel(proximalQuality, { state: 'WARMING' });
  qualityLabel(distalQuality, { state: 'WARMING' });
  proximalMeta.textContent = `Signal -- · threshold ${proximalThreshold.value} ${proximalThresholdMode.value}`;
  distalMeta.textContent = `Signal -- · threshold ${distalThreshold.value} ${distalThresholdMode.value}`;
  latestPtt.textContent = '--';
  medianPtt.textContent = '--';
  acceptedPairs.textContent = '0';
  rejectedPairs.textContent = '0 / 0';
  pttGuidance.textContent = 'Upload the Pulse Transit Time sender, then connect both sensors.';
  modeChip.textContent = 'WAITING';
  modeChip.className = '';
  frameMeta.textContent = 'No synchronized samples received';
  signalMeta.textContent = 'A0 -- · A1 --';
  rawSerial.textContent = 'waiting for PTT1 data...';
  resizeCanvases();
}

function resetSession() {
  clearSession();
  applySettings();
}

function nextTimestamp(frame, explicitTimestamp = null) {
  if (explicitTimestamp !== null) {
    streamTimestamp = explicitTimestamp;
    return streamTimestamp;
  }
  const deviceTimestamp = frame.timestampMs;
  if (lastDeviceTimestamp === null) streamTimestamp = deviceTimestamp;
  else if (deviceTimestamp > lastDeviceTimestamp) streamTimestamp += deviceTimestamp - lastDeviceTimestamp;
  else streamTimestamp += 2;
  lastDeviceTimestamp = deviceTimestamp;
  return streamTimestamp;
}

function updateFrame(frame, line, explicitTimestamp = null) {
  const timestamp = nextTimestamp(frame, explicitTimestamp);
  frameCount += 1;
  lastFrame = frame;
  lastResult = coach.update(frame.proximalSignal, frame.distalSignal, timestamp);
  proximalHistory.shift();
  proximalHistory.push(lastResult.proximal.signal10);
  distalHistory.shift();
  distalHistory.push(lastResult.distal.signal10);
  if (timestamp - lastRenderAt < 33 && lastResult.ptt.latest === null) return;
  lastRenderAt = timestamp;
  resizeCanvases();
  qualityLabel(proximalQuality, lastResult.proximalQuality);
  qualityLabel(distalQuality, lastResult.distalQuality);
  proximalMeta.textContent = `Signal ${lastResult.proximal.signal10} · threshold ${lastResult.proximal.threshold} ${lastResult.proximal.thresholdMode}`;
  distalMeta.textContent = `Signal ${lastResult.distal.signal10} · threshold ${lastResult.distal.threshold} ${lastResult.distal.thresholdMode}`;
  if (lastResult.ptt.latest !== null) latestPtt.textContent = lastResult.ptt.latest.toFixed(1);
  medianPtt.textContent = lastResult.ptt.median === null ? '--' : lastResult.ptt.median.toFixed(1);
  acceptedPairs.textContent = String(lastResult.ptt.accepted);
  rejectedPairs.textContent = `${lastResult.ptt.rejected} / ${lastResult.ptt.unmatched}`;
  if (lastResult.proximalQuality.state === 'CLIPPED' || lastResult.distalQuality.state === 'CLIPPED') {
    pttGuidance.textContent = 'A channel is clipping at the ADC rail. Reduce pressure, check power, and reposition before trusting PTT.';
  } else if (lastResult.proximalQuality.state !== 'GOOD' || lastResult.distalQuality.state !== 'GOOD') {
    pttGuidance.textContent = 'Hold both sensors lightly and still until both channels report GOOD.';
  } else if (lastResult.ptt.accepted === 0) {
    pttGuidance.textContent = 'Both waves look usable. Waiting for repeatable same-cycle beat pairs.';
  } else {
    pttGuidance.textContent = 'Both channels pass signal quality. PTT is an educational timing measurement, not blood pressure.';
  }
  const simulated = frame.mode === 'SIM';
  statusDot.className = `status-dot ${simulated ? 'simulation' : 'connected'}`;
  statusText.textContent = simulated ? 'Simulated two-sensor replay' : 'Two PulseSensors connected';
  sourceMeta.textContent = `${frame.format} · 250000 baud`;
  simulationWarning.hidden = !simulated;
  modeChip.textContent = simulated ? 'SIM PTT' : 'LIVE PTT';
  modeChip.className = simulated ? 'sim' : 'live';
  frameMeta.textContent = `${frameCount.toLocaleString()} synchronized dual samples`;
  signalMeta.textContent = `A0 ${lastResult.proximal.signal10} · A1 ${lastResult.distal.signal10}`;
  rawSerial.textContent = line;
}

function showError(message) {
  errorMessage.textContent = message;
  errorMessage.hidden = false;
}

async function readLoop() {
  const decoder = new TextDecoder();
  let buffer = '';
  while (reading && port?.readable) {
    const activeReader = port.readable.getReader();
    reader = activeReader;
    try {
      while (reading) {
        const { value, done } = await activeReader.read();
        if (done) break;
        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split(/\r?\n/);
        buffer = lines.pop() ?? '';
        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed) continue;
          rawSerial.textContent = trimmed;
          try {
            const frame = parsePulseTransitTimeLine(trimmed);
            if (frame) updateFrame(frame, trimmed);
          } catch (error) {
            console.warn('Ignored invalid Pulse Transit Time frame:', error.message);
          }
        }
      }
    } finally {
      activeReader.releaseLock();
      if (reader === activeReader) reader = null;
    }
  }
}

async function finalizeConnection(targetPort) {
  if (!targetPort || targetPort !== port) return false;
  reading = false;
  port = null;
  const activeReader = reader;
  const activeReadTask = readTask;
  if (activeReader) {
    try { await activeReader.cancel(); } catch { /* The stream already failed or closed. */ }
  }
  if (activeReadTask) {
    try { await activeReadTask; } catch { /* The caller reports read failures after cleanup. */ }
  }
  if (readTask === activeReadTask) readTask = null;
  try { await targetPort.close(); } catch { /* The device may already be gone or never opened. */ }
  connectBtn.textContent = 'Connect two-sensor stream';
  settingsControls.forEach((control) => { control.disabled = false; });
  statusDot.className = 'status-dot';
  statusText.textContent = 'Disconnected';
  sourceMeta.textContent = 'PTT1 dual stream · 250000 baud';
  simulationWarning.hidden = true;
  clearSession();
  return true;
}

async function connect() {
  if (!('serial' in navigator)) {
    showError('Web Serial is unavailable. Use desktop Chrome, Edge, or Brave over HTTPS.');
    return;
  }
  let requestedPort = null;
  try {
    requestedPort = await navigator.serial.requestPort();
    port = requestedPort;
    await requestedPort.open({ baudRate: 250000, bufferSize: 16384 });
    resetSession();
    reading = true;
    errorMessage.hidden = true;
    connectBtn.textContent = 'Disconnect';
    statusText.textContent = 'Waiting for synchronized samples';
    sourceMeta.textContent = 'Connected · USB serial · 250000 baud';
    readTask = readLoop();
    await readTask;
    if (port === requestedPort) await finalizeConnection(requestedPort);
  } catch (error) {
    if (requestedPort && port === requestedPort) await finalizeConnection(requestedPort);
    if (error.name !== 'NotFoundError') showError(`Connection failed: ${error.message}`);
  }
}

connectBtn.addEventListener('click', () => {
  if (port) finalizeConnection(port).catch((error) => showError(error.message));
  else connect().catch((error) => showError(error.message));
});

settingsControls.forEach((control) => {
  control.addEventListener('change', () => {
    try {
      resetSession();
      errorMessage.hidden = true;
    } catch (error) {
      showError(error.message);
    }
  });
});

copySketchBtn.addEventListener('click', async () => {
  const sketch = unoR4Sketch.textContent.trim();
  try {
    await navigator.clipboard.writeText(sketch);
    copySketchBtn.textContent = 'Copied';
    copySketchStatus.textContent = 'Paste into a new Arduino sketch.';
  } catch {
    const selection = window.getSelection();
    const range = document.createRange();
    range.selectNodeContents(unoR4Sketch);
    selection.removeAllRanges();
    selection.addRange(range);
    copySketchStatus.textContent = 'Sketch selected. Press Command-C to copy.';
  }
  setTimeout(() => { copySketchBtn.textContent = 'Copy sketch'; }, 1800);
});

navigator.serial?.addEventListener('disconnect', (event) => {
  finalizeConnection(event.target).catch(() => {});
});

window.addEventListener('resize', resizeCanvases);

function pulseSample(timestamp, delay = 0) {
  const phase = ((timestamp - delay) % 833 + 833) % 833;
  if (phase < 50) return 500 + Math.round(phase * 3);
  if (phase < 140) return 650 - Math.round((phase - 50) * 1.5);
  return 500;
}

function startReplay() {
  connectBtn.hidden = true;
  settingsControls.forEach((control) => { control.disabled = true; });
  resetSession();
  let timestamp = 0;
  const timer = setInterval(() => {
    for (let sample = 0; sample < 10; sample += 1) {
      const frame = {
        source: 'bench',
        format: 'Browser bench PTT1',
        mode: 'SIM',
        timestampUs: timestamp * 1000,
        timestampMs: timestamp,
        proximalSignal: pulseSample(timestamp),
        distalSignal: pulseSample(timestamp, 40),
      };
      updateFrame(frame, `PTT1,${frame.timestampUs},${frame.proximalSignal},${frame.distalSignal}`, timestamp);
      timestamp += 2;
    }
  }, 20);
  return timer;
}

window.__PULSE_TRANSIT_TIME_QA__ = {
  inject: (line, timestamp = null) => {
    const frame = parsePulseTransitTimeLine(line);
    if (frame) updateFrame(frame, line, timestamp);
    return { frame, result: lastResult };
  },
  reset: resetSession,
  getState: () => ({
    frameCount,
    lastFrame,
    result: lastResult,
    canvasWidths: [proximalCanvas.width, distalCanvas.width],
    connected: Boolean(port),
    button: connectBtn.textContent,
    status: statusText.textContent,
    source: sourceMeta.textContent,
    controlsDisabled: settingsControls.some((control) => control.disabled),
  }),
};

resetSession();
if (pageOptions.get('bench') === '1') startReplay();
