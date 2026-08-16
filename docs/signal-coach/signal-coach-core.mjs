// Browser-native Signal Coach.
//
// The detector is a JavaScript port of the real-human-tested PulseLink
// v1.1-resync algorithm. Incoming transports provide pulse-wave samples only;
// this module owns beat detection, confidence, BPM, IBI, and coach state.

const DEFAULTS = Object.freeze({
  pulseThreshold: 550,
  thresholdMode: 'adaptive',
  refractoryMs: 250,
  noBeatTimeoutMs: 3000,
  staleMs: 1500,
  minBpm: 40,
  maxBpm: 180,
  minIbi: 333,
  maxIbi: 1500,
  minAmplitude: 20,
  qualitySteps: 12,
  qualityLock: 10,
  qualityUp: 3,
  qualityDown: 1,
  qualityFastUp: 6,
  bpmAverageCount: 10,
  rangeSnap: 80,
  rearmRange: 120,
  rearmNoBeatMs: 2200,
  rearmCooldownMs: 3500,
  flatRange: 90,
  flatAmplitude: 12,
  resyncLabelMs: 900,
  resyncFastMs: 6000,
  beatFlashMs: 200,
});

export const COACH_COPY = Object.freeze({
  READY: 'Connect a pulse-wave stream to begin.',
  NO_SIGNAL: 'Check power and the signal wire. Then place the sensor on skin.',
  HOLD_STEADY: 'Use light, steady pressure. Do not squeeze the sensor.',
  SEARCHING: 'Waveform found. Keep still while Chrome finds the beat pattern.',
  GOOD_WAVE: 'Good waveform. Hold this position for a few more beats.',
  LOCKING: 'Almost there. Keep your hand relaxed and still.',
  SIGNAL_LOST: 'The signal moved. Reposition gently and hold still.',
  QUALIFIED: 'Signal looks consistent. Browser BPM is ready.',
  RESYNC: 'Chrome retuned to the waveform on screen. Keep still.',
});

function clamp(value, minimum, maximum) {
  return Math.max(minimum, Math.min(maximum, value));
}

function nowMs() {
  return globalThis.performance?.now?.() ?? Date.now();
}

export class BrowserSignalCoach {
  constructor(options = {}) {
    this.options = { ...DEFAULTS, ...options };
    if (!['adaptive', 'fixed'].includes(this.options.thresholdMode)) {
      throw new Error('threshold mode must be adaptive or fixed');
    }
    this.reset();
  }

  setThresholdMode(mode, threshold = this.options.pulseThreshold) {
    if (!['adaptive', 'fixed'].includes(mode)) throw new Error('threshold mode must be adaptive or fixed');
    if (!Number.isFinite(threshold) || threshold < 0 || threshold > 1023) throw new Error('threshold must be 0..1023');
    this.options.thresholdMode = mode;
    this.options.pulseThreshold = Math.round(threshold);
    this.threshold = this.options.pulseThreshold;
    this.peak = this.threshold;
    this.trough = this.threshold;
    this.pulsing = false;
    return this.lastResult;
  }

  reset(timestamp = null) {
    const start = timestamp ?? 0;
    const initial = this.options.pulseThreshold;
    this.inputDivisor = null;
    this.signal = initial;
    this.minimum = initial - this.options.rangeSnap / 2;
    this.maximum = initial + this.options.rangeSnap / 2;
    this.threshold = initial;
    this.peak = initial;
    this.trough = initial;
    this.pulsing = false;
    this.ibi = 600;
    this.amplitude = 0;
    this.quality = 0;
    this.bpm = 0;
    this.rates = [];
    this.locked = false;
    this.firstBeat = true;
    this.secondBeat = false;
    this.lastBeat = start;
    this.lastQualifiedBeat = null;
    this.lastRearm = start;
    this.rearms = 0;
    this.resyncLabelUntil = -Infinity;
    this.resyncFastUntil = -Infinity;
    this.beatFlashUntil = -Infinity;
    this.sampleCount = 0;
    this.state = 'READY';
    this.lastResult = this.snapshot(start, false, false);
    return this.lastResult;
  }

  normalizeSignal(rawSignal) {
    if (!Number.isFinite(rawSignal)) throw new Error('pulse sample must be numeric');
    if (rawSignal < 0 || rawSignal > 65535) throw new Error('pulse sample is outside the supported range');
    if (this.inputDivisor === null) this.inputDivisor = rawSignal > 1300 ? 4 : 1;
    if (rawSignal > 1300) this.inputDivisor = 4;
    return clamp(Math.round(rawSignal / this.inputDivisor), 0, 1023);
  }

  trackRange(signal) {
    this.minimum = Math.min(this.minimum + 1, signal);
    this.maximum = Math.max(this.maximum - 1, signal);
    if (this.maximum - this.minimum < this.options.rangeSnap) {
      this.minimum = signal - this.options.rangeSnap / 2;
      this.maximum = signal + this.options.rangeSnap / 2;
    }
  }

  qualifies(interval) {
    const rate = interval ? Math.floor(60000 / interval) : 0;
    return rate >= this.options.minBpm
      && rate <= this.options.maxBpm
      && interval >= this.options.minIbi
      && interval <= this.options.maxIbi
      && this.amplitude >= this.options.minAmplitude;
  }

  smoothedBpm(interval) {
    this.rates.push(interval);
    if (this.rates.length > this.options.bpmAverageCount) this.rates.shift();
    const total = this.rates.reduce((sum, value) => sum + value, 0);
    return Math.floor((60000 * this.rates.length) / total);
  }

  rearm(timestamp) {
    const midpoint = this.options.thresholdMode === 'fixed'
      ? this.options.pulseThreshold
      : this.maximum > this.minimum
      ? Math.floor((this.minimum + this.maximum) / 2)
      : this.options.pulseThreshold;
    this.threshold = midpoint;
    this.peak = midpoint;
    this.trough = midpoint;
    this.pulsing = false;
    this.quality = 0;
    this.bpm = 0;
    this.rates = [];
    this.locked = false;
    this.rearms += 1;
    this.lastRearm = timestamp;
    this.lastBeat = timestamp;
  }

  resync(timestamp = nowMs()) {
    const midpoint = this.options.thresholdMode === 'fixed'
      ? this.options.pulseThreshold
      : this.maximum > this.minimum
      ? Math.floor((this.minimum + this.maximum) / 2)
      : this.options.pulseThreshold;
    this.threshold = midpoint;
    this.peak = midpoint;
    this.trough = midpoint;
    this.pulsing = false;
    this.ibi = 600;
    this.amplitude = Math.max(this.amplitude, this.maximum - this.minimum);
    this.firstBeat = true;
    this.secondBeat = false;
    this.quality = 0;
    this.bpm = 0;
    this.rates = [];
    this.locked = false;
    this.rearms += 1;
    this.lastBeat = timestamp;
    this.lastRearm = timestamp;
    this.lastQualifiedBeat = timestamp;
    this.resyncLabelUntil = timestamp + this.options.resyncLabelMs;
    this.resyncFastUntil = timestamp + this.options.resyncFastMs;
    this.state = 'RESYNC';
    this.lastResult = this.snapshot(timestamp, false, false);
    return this.lastResult;
  }

  detect(timestamp) {
    const elapsed = timestamp - this.lastBeat;
    const gate = Math.floor((this.ibi * 3) / 5);
    let candidateBeat = false;
    let qualifiedBeat = false;

    if (this.signal < this.threshold && elapsed > gate && this.signal < this.trough) {
      this.trough = this.signal;
    }
    if (this.signal > this.threshold && this.signal > this.peak) this.peak = this.signal;

    if (elapsed > this.options.refractoryMs
        && elapsed > gate
        && this.signal > this.threshold
        && !this.pulsing) {
      this.pulsing = true;
      candidateBeat = true;
      const interval = elapsed;
      this.lastBeat = timestamp;

      if (this.secondBeat) {
        this.secondBeat = false;
        this.ibi = interval;
      } else if (this.firstBeat) {
        this.firstBeat = false;
        this.secondBeat = true;
      } else {
        this.ibi = interval;
        qualifiedBeat = this.qualifies(interval);
        if (qualifiedBeat) {
          this.bpm = this.smoothedBpm(interval);
          this.lastQualifiedBeat = timestamp;
          const step = timestamp < this.resyncFastUntil
            ? this.options.qualityFastUp
            : this.options.qualityUp;
          this.quality = Math.min(this.options.qualitySteps, this.quality + step);
        } else {
          this.quality = Math.max(0, this.quality - this.options.qualityDown);
        }
        this.locked = this.quality >= this.options.qualityLock;
        if (this.locked && qualifiedBeat) this.beatFlashUntil = timestamp + this.options.beatFlashMs;
      }
    }

    if (this.signal < this.threshold && this.pulsing) {
      this.pulsing = false;
      this.amplitude = this.peak - this.trough;
      this.threshold = this.options.thresholdMode === 'adaptive'
        ? this.trough + Math.floor(this.amplitude / 2)
        : this.options.pulseThreshold;
      this.peak = this.threshold;
      this.trough = this.threshold;
    }

    if (this.lastQualifiedBeat === null
        || timestamp - this.lastQualifiedBeat > this.options.noBeatTimeoutMs) {
      this.locked = false;
      this.quality = 0;
      this.bpm = 0;
    }

    if (!this.locked
        && this.maximum - this.minimum >= this.options.rearmRange
        && timestamp - this.lastBeat >= this.options.rearmNoBeatMs
        && timestamp - this.lastRearm >= this.options.rearmCooldownMs) {
      this.rearm(timestamp);
    }

    return { candidateBeat, qualifiedBeat };
  }

  coachState(timestamp) {
    const range = this.maximum - this.minimum;
    if (timestamp < this.resyncLabelUntil) return 'RESYNC';

    const fresh = this.lastQualifiedBeat !== null
      && timestamp - this.lastQualifiedBeat <= this.options.staleMs;
    if (fresh && this.locked && this.quality >= this.options.qualitySteps) return 'QUALIFIED';
    if (fresh && (this.locked || this.quality > 0)) return 'LOCKING';
    if (this.locked) return 'SIGNAL_LOST';
    if (range < this.options.flatRange || this.amplitude < this.options.flatAmplitude) return 'NO_SIGNAL';
    if (this.amplitude < this.options.minAmplitude) return 'HOLD_STEADY';
    if (range >= this.options.rearmRange) return 'GOOD_WAVE';
    return 'SEARCHING';
  }

  snapshot(timestamp, candidateBeat, qualifiedBeat) {
    return {
      state: this.state,
      quality: Math.round(this.quality),
      locked: this.state === 'QUALIFIED',
      bpm: this.state === 'QUALIFIED' ? this.bpm : 0,
      ibi: this.state === 'QUALIFIED' ? this.ibi : 0,
      beat: this.state === 'QUALIFIED' && qualifiedBeat,
      candidateBeat,
      qualifiedBeat,
      amplitude: Math.round(this.amplitude),
      threshold: Math.round(this.threshold),
      thresholdMode: this.options.thresholdMode,
      range: Math.round(this.maximum - this.minimum),
      minimum: Math.round(this.minimum),
      maximum: Math.round(this.maximum),
      signal10: this.signal,
      rearms: this.rearms,
      sampleCount: this.sampleCount,
      timestamp,
    };
  }

  update(frameOrSignal, timestamp = nowMs()) {
    const rawSignal = typeof frameOrSignal === 'number' ? frameOrSignal : frameOrSignal.signal;
    this.signal = this.normalizeSignal(rawSignal);
    this.sampleCount += 1;
    this.trackRange(this.signal);
    const { candidateBeat, qualifiedBeat } = this.detect(timestamp);
    this.state = this.coachState(timestamp);
    this.lastResult = this.snapshot(timestamp, candidateBeat, qualifiedBeat);
    return this.lastResult;
  }
}

// Kept as an import-compatible alias for early beta links and downstream tests.
export class LegacySignalCoach extends BrowserSignalCoach {}

export function stateColor(state) {
  if (state === 'QUALIFIED') return 'green';
  if (state === 'LOCKING' || state === 'SIGNAL_LOST' || state === 'RESYNC') return 'yellow';
  return 'blue';
}

export function adviceFor(state) {
  return COACH_COPY[state] ?? COACH_COPY.SEARCHING;
}
