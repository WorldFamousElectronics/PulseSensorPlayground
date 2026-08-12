// Coach state model for the published four-value Arduino/ESP32 stream.
const Q_MAX = 12;

export const COACH_COPY = Object.freeze({
  READY: 'Connect your board to begin.',
  NO_SIGNAL: 'Check power and the signal wire. Then place the sensor on skin.',
  HOLD_STEADY: 'Use light, steady pressure. Do not squeeze the sensor.',
  SEARCHING: 'Waveform found. Keep still while the beat pattern settles.',
  GOOD_WAVE: 'Good waveform. Hold this position for a few more beats.',
  LOCKING: 'Almost there. Keep your hand relaxed and still.',
  SIGNAL_LOST: 'The signal moved. Reposition gently and hold still.',
  QUALIFIED: 'Signal looks consistent. BPM is ready to use.',
  RESYNC: 'Resetting to the waveform on screen. Keep still.',
});

function rangeScore(range, flatRange, goodRange) {
  if (range < flatRange) return 0;
  return Math.min(1, (range - flatRange) / Math.max(1, goodRange - flatRange));
}

export class LegacySignalCoach {
  constructor(options = {}) {
    this.flatRange = options.flatRange ?? 70;
    this.goodRange = options.goodRange ?? 170;
    this.windowSize = options.windowSize ?? 75;
    this.samples = [];
    this.beatTimes = [];
    this.quality = 0;
    this.state = 'NO_SIGNAL';
    this.lastTimestamp = null;
    this.lastBeatTimestamp = null;
  }

  reset() {
    this.samples = [];
    this.beatTimes = [];
    this.quality = 0;
    this.state = 'NO_SIGNAL';
    this.lastTimestamp = null;
    this.lastBeatTimestamp = null;
  }

  update(frame, timestamp = performance.now()) {
    this.samples.push(frame.signal);
    if (this.samples.length > this.windowSize) this.samples.shift();
    this.lastTimestamp = timestamp;

    if (frame.beat) {
      if (this.lastBeatTimestamp !== null) {
        const interval = timestamp - this.lastBeatTimestamp;
        if (interval >= 333 && interval <= 1500) {
          this.beatTimes.push(interval);
          if (this.beatTimes.length > 6) this.beatTimes.shift();
          this.quality = Math.min(Q_MAX, this.quality + 3);
        } else {
          this.quality = Math.max(0, this.quality - 2);
        }
      }
      this.lastBeatTimestamp = timestamp;
    }

    const minimum = Math.min(...this.samples);
    const maximum = Math.max(...this.samples);
    const range = maximum - minimum;
    const recentBeat = this.lastBeatTimestamp !== null && timestamp - this.lastBeatTimestamp <= 1800;

    if (range < this.flatRange) {
      this.quality = Math.max(0, this.quality - .22);
      this.state = 'NO_SIGNAL';
    } else if (!recentBeat && this.samples.length >= this.windowSize) {
      this.quality = Math.max(0, this.quality - .12);
      this.state = range < this.goodRange ? 'HOLD_STEADY' : 'SEARCHING';
    } else if (this.quality >= 10 && frame.bpm > 0) {
      this.state = 'QUALIFIED';
    } else if (this.quality > 0 || frame.bpm > 0) {
      this.state = 'LOCKING';
    } else {
      this.state = rangeScore(range, this.flatRange, this.goodRange) >= .75 ? 'GOOD_WAVE' : 'SEARCHING';
    }

    return {
      state: this.state,
      quality: Math.round(this.quality),
      locked: this.state === 'QUALIFIED',
      range,
      minimum,
      maximum,
    };
  }
}

export function stateColor(state) {
  if (state === 'QUALIFIED') return 'green';
  if (state === 'LOCKING' || state === 'SIGNAL_LOST' || state === 'RESYNC') return 'yellow';
  return 'blue';
}

export function adviceFor(state) {
  return COACH_COPY[state] ?? COACH_COPY.SEARCHING;
}
