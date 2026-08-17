// Browser transport, browser-native coaching, and rendering for Signal Coach.
import { parsePulseLine } from './pulse-webserial-protocol.mjs?v=20260817-unor4-r2';
import { BrowserSignalCoach, adviceFor, stateColor } from './signal-coach-core.mjs?v=20260817-unor4-r2';
import { DualSignalCoach } from './ptt-coach-core.mjs?v=20260817-unor4-r2';

const pageOptions = new URLSearchParams(location.search);
if (window.self !== window.top || pageOptions.get('embedded') === '1') {
  document.documentElement.classList.add('is-embedded');
}

const HISTORY_LENGTH = 600;
const canvas = document.querySelector('#waveformCanvas');
const context = canvas.getContext('2d');
const display = document.querySelector('.display');
const deviceScreen = document.querySelector('.device-screen');
const connectBtn = document.querySelector('#connectBtn');
const resyncBtn = document.querySelector('#resyncBtn');
const coachMode = document.querySelector('#coachMode');
const sampleRate = document.querySelector('#sampleRate');
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
const liveInputFormat = document.querySelector('#liveInputFormat');
const sourceMetrics = document.querySelector('#sourceMetrics');
const rawSerial = document.querySelector('#rawSerial');
const singleWavePanel = document.querySelector('#singleWavePanel');
const singleReadoutPanel = document.querySelector('#singleReadoutPanel');
const pttWorkspace = document.querySelector('#pttWorkspace');
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
const unoR4Setup = document.querySelector('#unoR4Setup');
const copySketchBtn = document.querySelector('#copySketchBtn');
const copySketchStatus = document.querySelector('#copySketchStatus');
const unoR4Sketch = document.querySelector('#unoR4Sketch');

let port = null;
let reader = null;
let reading = false;
let history = new Array(HISTORY_LENGTH).fill(null);
let frameCount = 0;
let gaps = 0;
let lastSequence = null;
let lastFrame = null;
let lastCoach = { state: 'READY', quality: 0, locked: false, range: 0, bpm: 0, ibi: 0, beat: false };
let streamTimestamp = null;
let lastDeviceTimestamp = null;
let resyncUiTimer = null;
const browserCoach = new BrowserSignalCoach();
let dualCoach = new DualSignalCoach();
let proximalHistory = new Array(HISTORY_LENGTH).fill(null);
let distalHistory = new Array(HISTORY_LENGTH).fill(null);
let lastPtt = null;
let lastPttRenderAt = -Infinity;

function resizeCanvas() {
  const rectangle = canvas.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  const width = Math.max(1, Math.round(rectangle.width * dpr));
  const height = Math.max(1, Math.round(rectangle.height * dpr));
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
    context.setTransform(dpr, 0, 0, dpr, 0, 0);
  }
  drawWaveform();
  resizePttCanvas(proximalCanvas, proximalHistory, lastPtt?.proximal?.threshold ?? 550, '#087e91');
  resizePttCanvas(distalCanvas, distalHistory, lastPtt?.distal?.threshold ?? 550, '#997800');
}

function resizePttCanvas(target, values, threshold, color) {
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
  drawPttWaveform(target, drawing, values, threshold, color);
}

function drawPttWaveform(target, drawing, values, threshold, color) {
  const width = target.getBoundingClientRect().width;
  const height = target.getBoundingClientRect().height;
  drawing.fillStyle = '#fff';
  drawing.fillRect(0, 0, width, height);
  drawing.strokeStyle = '#dce7df';
  drawing.lineWidth = 1;
  for (let index = 1; index < 4; index += 1) {
    drawing.beginPath(); drawing.moveTo(0, (height * index) / 4); drawing.lineTo(width, (height * index) / 4); drawing.stroke();
  }
  const present = values.filter((value) => value !== null);
  if (present.length < 2) return;
  const low = Math.max(0, Math.min(...present, threshold) - 35);
  const high = Math.min(1023, Math.max(...present, threshold) + 35);
  const range = Math.max(1, high - low);
  const thresholdY = height - ((threshold - low) / range) * height;
  drawing.save();
  drawing.setLineDash([6, 5]);
  drawing.strokeStyle = '#a12626';
  drawing.beginPath(); drawing.moveTo(0, thresholdY); drawing.lineTo(width, thresholdY); drawing.stroke();
  drawing.restore();
  drawing.strokeStyle = color;
  drawing.lineWidth = 2.5;
  drawing.lineJoin = 'round';
  drawing.beginPath();
  let started = false;
  values.forEach((value, index) => {
    if (value === null) return;
    const x = (index / (HISTORY_LENGTH - 1)) * width;
    const y = height - ((value - low) / range) * height;
    if (!started) { drawing.moveTo(x, y); started = true; } else drawing.lineTo(x, y);
  });
  drawing.stroke();
}

function drawWaveform() {
  const width = canvas.getBoundingClientRect().width;
  const height = canvas.getBoundingClientRect().height;
  context.fillStyle = '#f4f8f5';
  context.fillRect(0, 0, width, height);

  context.strokeStyle = '#dce7df';
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
  const color = stateColor(lastCoach.state) === 'green' ? '#16784b' : stateColor(lastCoach.state) === 'yellow' ? '#997800' : '#087e91';

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

function nextSampleTimestamp(frame, explicitTimestamp = null) {
  const period = 1000 / Number(sampleRate.value);
  if (explicitTimestamp !== null) {
    streamTimestamp = explicitTimestamp;
    return streamTimestamp;
  }
  if (frame.timestampMs !== null) {
    const deviceTimestamp = frame.timestampMs;
    if (lastDeviceTimestamp === null) streamTimestamp = deviceTimestamp;
    else if (deviceTimestamp > lastDeviceTimestamp) streamTimestamp += deviceTimestamp - lastDeviceTimestamp;
    else streamTimestamp += period;
    lastDeviceTimestamp = deviceTimestamp;
    return streamTimestamp;
  }
  streamTimestamp = streamTimestamp === null ? 0 : streamTimestamp + period;
  return streamTimestamp;
}

function senderDiagnostics(frame) {
  const values = [];
  if (frame.sourceBpm !== null) values.push(`BPM ${frame.sourceBpm}`);
  if (frame.sourceIbi !== null) values.push(`IBI ${frame.sourceIbi}`);
  if (frame.sourceQuality !== null) values.push(`Q ${frame.sourceQuality}`);
  return values.length ? values.join(' · ') : '--';
}

function updateCoach(coach) {
  const color = stateColor(coach.state);
  display.classList.remove('state-blue', 'state-yellow', 'state-green');
  display.classList.add(`state-${color}`);
  coachLabel.textContent = coach.state.replaceAll('_', ' ');
  coachAdvice.textContent = adviceFor(coach.state);
  qualitySegments.forEach((segment, index) => segment.classList.toggle('filled', index < Math.floor((coach.quality * 10) / 12)));

  liveBpm.textContent = coach.locked && coach.bpm > 0 ? String(coach.bpm) : '--';
  bpmNote.textContent = coach.locked ? 'Stable signal detected in Chrome' : 'Wait for a clean signal';
  liveQuality.textContent = `${coach.quality}/12`;
}

function updateFrame(frame, line, explicitTimestamp = null) {
  if (frame.secondarySignal !== undefined) {
    updatePttFrame(frame, line, explicitTimestamp);
    return;
  }
  const timestamp = nextSampleTimestamp(frame, explicitTimestamp);
  frameCount += 1;
  if (frame.sequence !== null && lastSequence !== null && frame.sequence !== ((lastSequence + 1) & 0x7fffffff)) gaps += 1;
  if (frame.sequence !== null) lastSequence = frame.sequence;
  lastFrame = frame;
  lastCoach = browserCoach.update(frame.signal, timestamp);

  history.shift();
  history.push(frame.signal);
  drawWaveform();
  updateCoach(lastCoach);

  const simulated = frame.mode === 'SIM';
  waitingOverlay.hidden = true;
  resyncBtn.disabled = false;
  statusDot.className = `status-dot ${simulated ? 'simulation' : 'connected'}`;
  statusText.textContent = simulated ? 'Simulated bench replay' : 'Pulse stream connected';
  sourceMeta.textContent = `${frame.format} · coach runs in Chrome`;
  sourceName.textContent = simulated ? 'SIM PULSE' : 'PulseSensor';
  screenSource.textContent = 'CHROME';
  simulationWarning.hidden = !simulated;
  modeChip.textContent = simulated ? 'SIM' : 'LIVE';
  modeChip.className = simulated ? 'sim' : 'live';
  frameMeta.textContent = `${lastCoach.state.replaceAll('_', ' ')} · ${frameCount.toLocaleString()} samples · ${gaps} gaps`;
  signalMeta.textContent = `Signal ${frame.signal}`;
  liveSignal.textContent = String(frame.signal);
  liveBeat.textContent = lastCoach.beat ? '1' : '0';
  liveIbi.textContent = lastCoach.ibi > 0 ? String(lastCoach.ibi) : '--';
  liveInputFormat.textContent = frame.format;
  sourceMetrics.textContent = senderDiagnostics(frame);
  rawSerial.textContent = line;

  if (lastCoach.beat) {
    heart.classList.add('beat');
    bpmCard.classList.add('invert');
    setTimeout(() => { heart.classList.remove('beat'); bpmCard.classList.remove('invert'); }, 180);
  }
}

function qualityLabel(target, quality) {
  target.textContent = quality.state;
  target.className = quality.state === 'GOOD' ? 'good' : quality.state === 'CLIPPED' ? 'clipped' : '';
}

function updatePttFrame(frame, line, explicitTimestamp = null) {
  if (coachMode.value !== 'ptt') {
    coachMode.value = 'ptt';
    setCoachMode('ptt');
  }
  const timestamp = nextSampleTimestamp(frame, explicitTimestamp);
  frameCount += 1;
  lastFrame = frame;
  lastPtt = dualCoach.update(frame.signal, frame.secondarySignal, timestamp);
  proximalHistory.shift(); proximalHistory.push(lastPtt.proximal.signal10);
  distalHistory.shift(); distalHistory.push(lastPtt.distal.signal10);
  if (timestamp - lastPttRenderAt < 33 && lastPtt.ptt.latest === null) return;
  lastPttRenderAt = timestamp;
  resizePttCanvas(proximalCanvas, proximalHistory, lastPtt.proximal.threshold, '#087e91');
  resizePttCanvas(distalCanvas, distalHistory, lastPtt.distal.threshold, '#997800');
  qualityLabel(proximalQuality, lastPtt.proximalQuality);
  qualityLabel(distalQuality, lastPtt.distalQuality);
  proximalMeta.textContent = `Signal ${lastPtt.proximal.signal10} · threshold ${lastPtt.proximal.threshold} ${lastPtt.proximal.thresholdMode}`;
  distalMeta.textContent = `Signal ${lastPtt.distal.signal10} · threshold ${lastPtt.distal.threshold} ${lastPtt.distal.thresholdMode}`;
  if (lastPtt.ptt.latest !== null) latestPtt.textContent = lastPtt.ptt.latest.toFixed(1);
  medianPtt.textContent = lastPtt.ptt.median === null ? '--' : lastPtt.ptt.median.toFixed(1);
  acceptedPairs.textContent = String(lastPtt.ptt.accepted);
  rejectedPairs.textContent = `${lastPtt.ptt.rejected} / ${lastPtt.ptt.unmatched}`;
  if (lastPtt.proximalQuality.state === 'CLIPPED' || lastPtt.distalQuality.state === 'CLIPPED') {
    pttGuidance.textContent = 'A channel is clipping at the ADC rail. Reduce pressure, check power, and reposition before trusting PTT.';
  } else if (lastPtt.proximalQuality.state !== 'GOOD' || lastPtt.distalQuality.state !== 'GOOD') {
    pttGuidance.textContent = 'Hold both sensors lightly and still until both channels report GOOD.';
  } else if (lastPtt.ptt.accepted === 0) {
    pttGuidance.textContent = 'Both waves look usable. Waiting for repeatable same-cycle beat pairs.';
  } else {
    pttGuidance.textContent = 'Both channels pass signal quality. PTT is an educational timing measurement, not blood pressure.';
  }
  const simulated = frame.mode === 'SIM';
  waitingOverlay.hidden = true;
  resyncBtn.disabled = true;
  statusDot.className = `status-dot ${simulated ? 'simulation' : 'connected'}`;
  statusText.textContent = simulated ? 'Simulated two-sensor replay' : 'Two PulseSensors connected';
  sourceMeta.textContent = `${frame.format} · 250000 baud`;
  sourceName.textContent = simulated ? 'SIM DUAL PULSE' : 'Dual PulseSensor';
  simulationWarning.hidden = !simulated;
  modeChip.textContent = simulated ? 'SIM PTT' : 'LIVE PTT';
  modeChip.className = simulated ? 'sim' : 'live';
  frameMeta.textContent = `${frameCount.toLocaleString()} synchronized dual samples`;
  signalMeta.textContent = `A0 ${lastPtt.proximal.signal10} · A1 ${lastPtt.distal.signal10}`;
  liveSignal.textContent = `${lastPtt.proximal.signal10} / ${lastPtt.distal.signal10}`;
  liveBeat.textContent = `${Number(lastPtt.proximal.qualifiedBeat)} / ${Number(lastPtt.distal.qualifiedBeat)}`;
  liveIbi.textContent = lastPtt.ptt.latest === null ? '--' : lastPtt.ptt.latest.toFixed(1);
  liveQuality.textContent = `${lastPtt.proximalQuality.state} / ${lastPtt.distalQuality.state}`;
  liveInputFormat.textContent = frame.format;
  sourceMetrics.textContent = 'Browser detector · one-to-one pairing';
  rawSerial.textContent = line;
}

function resetPttState() {
  dualCoach = new DualSignalCoach();
  proximalHistory = new Array(HISTORY_LENGTH).fill(null);
  distalHistory = new Array(HISTORY_LENGTH).fill(null);
  lastPtt = null;
  lastPttRenderAt = -Infinity;
  latestPtt.textContent = '--';
  medianPtt.textContent = '--';
  acceptedPairs.textContent = '0';
  rejectedPairs.textContent = '0 / 0';
  applyPttSettings();
}

function applyPttSettings() {
  dualCoach.configureChannel('proximal', proximalThresholdMode.value, Number(proximalThreshold.value));
  dualCoach.configureChannel('distal', distalThresholdMode.value, Number(distalThreshold.value));
  dualCoach.setPairingWindow(Number(minimumPtt.value), Number(maximumPtt.value));
}

function setCoachMode(mode) {
  const ptt = mode === 'ptt';
  singleWavePanel.hidden = ptt;
  singleReadoutPanel.hidden = ptt;
  pttWorkspace.hidden = !ptt;
  unoR4Setup.hidden = !ptt;
  sampleRate.value = ptt ? '500' : '50';
  sourceMeta.textContent = ptt ? 'Dual PTT1 input · 250000 baud' : 'Single pulse-wave input · 115200 baud';
  resyncBtn.hidden = ptt;
  waitingOverlay.textContent = ptt ? 'CONNECT A PTT1 DUAL STREAM' : 'CONNECT A PULSE STREAM';
  if (ptt) {
    resetPttState();
    requestAnimationFrame(resizeCanvas);
  }
}

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

function showError(message) {
  errorMessage.textContent = message;
  errorMessage.hidden = false;
}

function resetCoachState() {
  streamTimestamp = null;
  lastDeviceTimestamp = null;
  browserCoach.reset();
}

function resetDisconnectedDisplay() {
  history = new Array(HISTORY_LENGTH).fill(null);
  frameCount = 0;
  gaps = 0;
  lastSequence = null;
  lastFrame = null;
  lastCoach = { state: 'READY', quality: 0, locked: false, range: 0, bpm: 0, ibi: 0, beat: false };
  resetCoachState();
  display.classList.remove('state-yellow', 'state-green');
  display.classList.add('state-blue');
  screenSource.textContent = 'CHROME';
  waitingOverlay.hidden = false;
  waitingOverlay.textContent = 'CONNECT A PULSE STREAM';
  if (resyncUiTimer !== null) clearTimeout(resyncUiTimer);
  resyncUiTimer = null;
  resyncBtn.textContent = 'Re-sync Signal';
  resyncBtn.classList.remove('confirmed');
  resyncBtn.disabled = true;
  sampleRate.disabled = false;
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
  liveInputFormat.textContent = '--';
  sourceMetrics.textContent = '--';
  rawSerial.textContent = 'waiting for data...';
  simulationWarning.hidden = true;
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
  coachMode.disabled = false;
  sourceMeta.textContent = coachMode.value === 'ptt' ? 'Dual PTT1 input · 250000 baud' : 'Single pulse-wave input · 115200 baud';
  resetDisconnectedDisplay();
}

async function connect() {
  if (!('serial' in navigator)) {
    showError('Web Serial is unavailable. Use desktop Chrome, Edge, or Brave over HTTPS.');
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    const baudRate = coachMode.value === 'ptt' ? 250000 : 115200;
    await port.open({ baudRate, bufferSize: 16384 });
    history = new Array(HISTORY_LENGTH).fill(null);
    frameCount = 0;
    gaps = 0;
    lastSequence = null;
    resetCoachState();
    reading = true;
    errorMessage.hidden = true;
    sampleRate.disabled = true;
    coachMode.disabled = true;
    connectBtn.textContent = 'Disconnect';
    statusText.textContent = 'Waiting for samples';
    sourceMeta.textContent = `Connected · USB serial · ${baudRate} baud`;
    waitingOverlay.textContent = 'WAITING FOR PULSE DATA';
    await readLoop();
  } catch (error) {
    reading = false;
    const failedPort = port;
    port = null;
    if (failedPort) {
      try { await failedPort.close(); } catch { /* Port never opened or already closed. */ }
    }
    sampleRate.disabled = false;
    coachMode.disabled = false;
    connectBtn.textContent = 'Connect';
    if (error.name !== 'NotFoundError') showError(`Connection failed: ${error.message}`);
  }
}

connectBtn.addEventListener('click', () => {
  if (port) disconnect().catch((error) => showError(error.message));
  else connect().catch((error) => showError(error.message));
});

coachMode.addEventListener('change', () => setCoachMode(coachMode.value));
[proximalThresholdMode, distalThresholdMode, proximalThreshold, distalThreshold, minimumPtt, maximumPtt].forEach((control) => {
  control.addEventListener('change', () => {
    try { applyPttSettings(); errorMessage.hidden = true; } catch (error) { showError(error.message); }
  });
});

resyncBtn.addEventListener('click', () => {
  if (streamTimestamp === null) return;
  lastCoach = browserCoach.resync(streamTimestamp);
  updateCoach(lastCoach);
  coachLabel.textContent = 'RESYNC';
  resyncBtn.textContent = 'Retuned — keep still';
  resyncBtn.classList.add('confirmed');
  if (resyncUiTimer !== null) clearTimeout(resyncUiTimer);
  resyncUiTimer = setTimeout(() => {
    resyncBtn.textContent = 'Re-sync Signal';
    resyncBtn.classList.remove('confirmed');
    resyncUiTimer = null;
  }, 900);
});

navigator.serial?.addEventListener('disconnect', (event) => {
  if (event.target === port) disconnect().catch(() => {});
});

window.addEventListener('resize', resizeCanvas);
new ResizeObserver(resizeCanvas).observe(deviceScreen);
resizeCanvas();

function startReplay() {
  connectBtn.hidden = true;
  const pttReplay = pageOptions.get('mode') === 'ptt';
  coachMode.value = pttReplay ? 'ptt' : 'single';
  setCoachMode(coachMode.value);
  coachMode.disabled = true;
  sampleRate.value = pttReplay ? '500' : '50';
  sampleRate.disabled = true;
  resetCoachState();
  let sequence = 0;
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
    const frame = {
      source: 'bench', format: 'Browser bench pulse', version: 0, mode: 'SIM', sequence,
      timestampMs: elapsed, signal, sourceBpm: null, sourceIbi: null,
      sourceBeat: null, sourceQuality: null, sourceState: null,
    };
    if (pttReplay) {
      const distalPhase = ((elapsed - 40) % 833 + 833) % 833;
      let distalPulse = 0;
      if (distalPhase < 55) distalPulse = distalPhase * 21;
      else if (distalPhase < 145) distalPulse = 1155 - (distalPhase - 55) * 10;
      else if (distalPhase < 205) distalPulse = 255 - (distalPhase - 145) * 4;
      frame.secondarySignal = Math.max(0, Math.min(4095, 1880 + Math.max(0, distalPulse)));
      frame.format = 'Browser bench PTT1';
    }
    updateFrame(frame, String(signal));
    sequence = (sequence + 1) & 0x7fffffff;
  }, 20);
  return timer;
}

if (pageOptions.get('mode') === 'ptt') coachMode.value = 'ptt';
if (pageOptions.get('bench') === '1') startReplay();

window.__PULSE_COACH_QA__ = {
  inject: (line, timestamp = null) => {
    const frame = parsePulseLine(line);
    if (frame) updateFrame(frame, line, timestamp);
    return { frame, coach: lastCoach };
  },
  reset: () => {
    browserCoach.reset();
    history = new Array(HISTORY_LENGTH).fill(null);
    frameCount = 0;
    gaps = 0;
    lastSequence = null;
    streamTimestamp = null;
    lastDeviceTimestamp = null;
  },
  getState: () => ({ frameCount, gaps, lastFrame, coach: lastCoach, ptt: lastPtt, mode: coachMode.value, canvasWidth: canvas.width, canvasHeight: canvas.height }),
};

setCoachMode(coachMode.value);
