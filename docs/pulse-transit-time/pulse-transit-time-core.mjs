import { BrowserSignalCoach } from '../signal-coach/signal-coach-core.mjs?v=20260819-ptt-r1';

function median(values) {
  if (!values.length) return null;
  const sorted = [...values].sort((a, b) => a - b);
  const middle = Math.floor(sorted.length / 2);
  return sorted.length % 2 ? sorted[middle] : (sorted[middle - 1] + sorted[middle]) / 2;
}

export class SignalQualityWindow {
  constructor({ windowSize = 500, warmupSize = 100, minRange = 25, clipLow = 5, clipHigh = 1018 } = {}) {
    this.options = { windowSize, warmupSize, minRange, clipLow, clipHigh };
    this.samples = [];
  }

  update(signal) {
    this.samples.push(signal);
    if (this.samples.length > this.options.windowSize) this.samples.shift();
    const minimum = Math.min(...this.samples);
    const maximum = Math.max(...this.samples);
    const clipped = this.samples.filter((value) => value <= this.options.clipLow || value >= this.options.clipHigh).length;
    let state = 'GOOD';
    if (this.samples.length < this.options.warmupSize) state = 'WARMING';
    else if (clipped / this.samples.length >= 0.01) state = 'CLIPPED';
    else if (maximum - minimum < this.options.minRange) state = 'WEAK';
    return { state, minimum, maximum, range: maximum - minimum, clipped };
  }
}

export class PttPairer {
  constructor({ minimumMs = 5, maximumMs = 300, historyLength = 20 } = {}) {
    this.minimumMs = minimumMs;
    this.maximumMs = maximumMs;
    this.historyLength = historyLength;
    this.reset();
  }

  reset() {
    this.proximal = null;
    this.values = [];
    this.accepted = 0;
    this.rejected = 0;
    this.unmatched = 0;
  }

  setWindow(minimumMs, maximumMs) {
    if (!Number.isFinite(minimumMs) || !Number.isFinite(maximumMs) || minimumMs < 0 || maximumMs <= minimumMs) {
      throw new Error('PTT window must have a non-negative minimum below the maximum');
    }
    this.minimumMs = minimumMs;
    this.maximumMs = maximumMs;
  }

  pushProximal(timestamp) {
    if (this.proximal !== null) this.unmatched += 1;
    this.proximal = timestamp;
  }

  pushDistal(timestamp, qualityOkay = true) {
    if (this.proximal === null) {
      this.unmatched += 1;
      return null;
    }
    const delay = timestamp - this.proximal;
    if (delay < this.minimumMs) return null;
    this.proximal = null;
    if (delay > this.maximumMs || !qualityOkay) {
      this.rejected += 1;
      return null;
    }
    this.accepted += 1;
    this.values.push(delay);
    if (this.values.length > this.historyLength) this.values.shift();
    return delay;
  }

  expire(timestamp) {
    if (this.proximal !== null && timestamp - this.proximal > this.maximumMs) {
      this.proximal = null;
      this.unmatched += 1;
    }
  }

  snapshot(latest = null) {
    return {
      latest,
      median: median(this.values),
      accepted: this.accepted,
      rejected: this.rejected,
      unmatched: this.unmatched,
      pending: this.proximal !== null,
    };
  }
}

export class PulseTransitTimeCoach {
  constructor(options = {}) {
    const channelDefaults = { pulseThreshold: 550, thresholdMode: 'fixed' };
    this.proximal = new BrowserSignalCoach({ ...channelDefaults, ...options.proximal });
    this.distal = new BrowserSignalCoach({ ...channelDefaults, ...options.distal });
    this.proximalQuality = new SignalQualityWindow(options.quality);
    this.distalQuality = new SignalQualityWindow(options.quality);
    this.pairer = new PttPairer(options.pairing);
  }

  configureChannel(channel, mode, threshold) {
    const detector = channel === 'proximal' ? this.proximal : this.distal;
    detector.setThresholdMode(mode, threshold);
  }

  setPairingWindow(minimumMs, maximumMs) {
    this.pairer.setWindow(minimumMs, maximumMs);
  }

  update(proximalSignal, distalSignal, timestamp) {
    const proximal = this.proximal.update(proximalSignal, timestamp);
    const distal = this.distal.update(distalSignal, timestamp);
    const proximalQuality = this.proximalQuality.update(proximal.signal10);
    const distalQuality = this.distalQuality.update(distal.signal10);
    this.pairer.expire(timestamp);
    if (proximal.qualifiedBeat) this.pairer.pushProximal(timestamp);
    let latest = null;
    if (distal.qualifiedBeat) {
      latest = this.pairer.pushDistal(timestamp, proximalQuality.state === 'GOOD' && distalQuality.state === 'GOOD');
    }
    return {
      proximal,
      distal,
      proximalQuality,
      distalQuality,
      ptt: this.pairer.snapshot(latest),
    };
  }
}
