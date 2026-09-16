#pragma once

#include "synth_types.h"

// -----------------------------------------------------------------------------
// GLOBAL WAVE TABLES
// -----------------------------------------------------------------------------
int16_t SINE_TABLE[256];
int16_t TRIANGLE_TABLE[256];
int16_t SQUARE_TABLE[256];
int16_t SAW_TABLE[256];
int16_t BRIGHT_TRIANGLE_TABLE[256];
int16_t WARM_TRIANGLE_TABLE[256];

float SINE_TABLE_F[256];

const uint32_t SAMPLE_RATE = 16000;          // Sample rate (16 kHz)

// =============================================================================
// AUDIO SYNTHESIS ENGINE (DSP)
// =============================================================================

// =============================================================================
// ADSR — Volume Envelope (Attack / Decay / Sustain / Release)
// =============================================================================
class ADSR {
  private:
    ADSREnvelopeState _state = ADSREnvelopeState::IDLE;
    float _level = 0.0f;
    float _att_step = 0.0f;
    float _dec_step = 0.0f;
    float _rel_step = 0.0f;
    float _sustain_level = 0.0f;

  public:
    void trigger(const ADSRParameters& params) {
      // If a note retriggers before the previous one's RELEASE phase finished the
      // envelope just kept riding wherever it already was instead of restarting.
      // Consecutive/repeated notes (e.g. "mi-mi", "do-do") would then blend into
      // one continuous tone instead of sounding as separate articulated notes.
      // Forcing a hard reset here gives every note a clean, audible attack.
      _sustain_level = params.sustain_level;
      _att_step = 1.0f / (params.attack_ms * (SAMPLE_RATE / 1000.0f) + 1.0f);
      _dec_step = (1.0f - params.sustain_level) / (params.decay_ms * (SAMPLE_RATE / 1000.0f) + 1.0f);
      _rel_step = params.sustain_level / (params.release_ms * (SAMPLE_RATE / 1000.0f) + 1.0f);
      _level = 0.0f;
      _state = ADSREnvelopeState::ATTACK;
    }

    void release() {
      if (_state != ADSREnvelopeState::IDLE) {
        _state = ADSREnvelopeState::RELEASE;
      }
    }

    void reset() {
      _state = ADSREnvelopeState::IDLE;
      _level = 0.0f;
    }

    ADSREnvelopeState getState() const {
      return _state;
    }

    float process() {
      const float att_step = _att_step;
      const float dec_step = _dec_step;
      const float rel_step = _rel_step;

      switch (_state) {
        case ADSREnvelopeState::IDLE:
          _level = 0.0f;
          break;

        case ADSREnvelopeState::ATTACK:
          _level += att_step;
          if (_level >= 1.0f) {
            _level = 1.0f;
            _state = ADSREnvelopeState::DECAY;
          }
          break;

        case ADSREnvelopeState::DECAY:
          _level -= dec_step;
          if (_level <= _sustain_level) {
            _level = _sustain_level;
            _state = ADSREnvelopeState::SUSTAIN;
          }
          break;

        case ADSREnvelopeState::SUSTAIN:
          _level = _sustain_level;
          break;

        case ADSREnvelopeState::RELEASE:
          _level -= rel_step;
          if (_level <= 0.0f) {
            _level = 0.0f;
            _state = ADSREnvelopeState::IDLE;
          }
          break;
      }

      return _level;
    }
};

// =============================================================================
// FAST SINE LOOKUP
// =============================================================================
// Uses the 256-sample sine table.
// phase is expressed in table units: 0.0 .. 256.0.
static inline float sineTableLookup(float phase) {
  while (phase >= 256.0f) phase -= 256.0f;
  while (phase < 0.0f) phase += 256.0f;

  const uint8_t index = (uint8_t)phase;
  const uint8_t next = (uint8_t)(index + 1u);
  const float fraction = phase - (float)index;
  const float a = SINE_TABLE_F[index];
  const float b = SINE_TABLE_F[next];
  return a + fraction * (b - a);
}


// =============================================================================
// MelodySynth — Monophonic Melodic Synthesizer per Channel
// =============================================================================
class MelodySynth {
  private:
    ADSR _adsr;
    uint32_t _base_phase_step = 0;
    uint32_t _phase = 0;
    float _lpf_filter = 0.0f;                  // Low-pass filter (smoothing)
    uint32_t _vibrato_phase = 0;
    uint32_t _vibrato_phase_step = 0;
    float _arpeggio_ratio = 1.0f;
    uint32_t _arpeggio_sub_step_duration = 1;
    uint32_t _arpeggio_counter = 0;
    bool _arpeggio_alt = false;
    const Instrument* _cached_instrument = nullptr;

  public:
    void trigger(const Instrument* inst) {
      _adsr.trigger(inst->adsr);
      _arpeggio_counter = 0;
      _arpeggio_alt = false;
      cacheInstrumentParameters(inst);
    }

    void release() {
      _adsr.release();
    }

    void resetADSR() {
      _adsr.reset();
    }

    void resetFilter() {
      _lpf_filter = 0.0f;
    }

    ADSREnvelopeState getADSRState() const {
      return _adsr.getState();
    }

    void setBasePhaseStep(uint32_t step) {
      _base_phase_step = step;
    }

    void resetPhase() {
      _phase = 0;
      _vibrato_phase = 0;
    }

    void cacheInstrumentParameters(const Instrument* inst) {
      if (!inst || inst == _cached_instrument) return;

      _cached_instrument = inst;
      if (inst->vibrato_freq > 0.0f) {
        const float phase_step =
            (4294967296.0f * inst->vibrato_freq) /
            (float)SAMPLE_RATE;
        _vibrato_phase_step = (uint32_t)phase_step;
      } else {
        _vibrato_phase_step = 0;
      }

      _arpeggio_ratio = 1.0f;
      if (inst->arpeg.active && inst->arpeg.speed > 0) {
        _arpeggio_sub_step_duration = SCALE_SAMPLES(125) / inst->arpeg.speed;
        if (_arpeggio_sub_step_duration == 0) _arpeggio_sub_step_duration = 1;
        _arpeggio_ratio = powf(2.0f, (float)inst->arpeg.semitones / 12.0f);
      } else {
        _arpeggio_sub_step_duration = 1;
      }
    }

    float process(const Instrument* inst, uint32_t step_time_counter) {
      if (!inst || _adsr.getState() == ADSREnvelopeState::IDLE) {
        return 0.0f;
      }

      // Instrument parameters are cached by trigger(); no per-sample cache work.

      // --- 1. Vibrato Calculation ---
      float current_phase_step = (float)_base_phase_step;
      if (inst->vibrato_depth > 0.0f && _vibrato_phase_step > 0.0f) {
        _vibrato_phase += _vibrato_phase_step;

        const uint8_t vibrato_index = _vibrato_phase >> 24;

        const float vibrato_mod =
            1.0f + (inst->vibrato_depth * SINE_TABLE_F[vibrato_index]);
        current_phase_step *= vibrato_mod;
      }

      // --- 2. Arpeggiator Calculation ---
      if (inst->arpeg.active && inst->arpeg.speed > 0) {
        if (++_arpeggio_counter >= _arpeggio_sub_step_duration) {
          _arpeggio_counter = 0;
          _arpeggio_alt = !_arpeggio_alt;
        }

        if (_arpeggio_alt) {
          current_phase_step *= _arpeggio_ratio;
        }
      }


      // --- 3. Phase Accumulation and Wavetable Lookup (with linear interpolation) ---
      _phase += (uint32_t)current_phase_step;

      uint8_t index_lut      = (_phase >> 24) & 0xFF;
      uint8_t index_lut_next = (index_lut + 1) & 0xFF;              // wraps 255 -> 0 automatically
      float   frac           = (float)((_phase >> 16) & 0xFF) / 256.0f; // fractional part, 0.0–~0.996

      float sample_a = (float)inst->wave_table[index_lut] * (1.0f / 32767.0f);
      float sample_b = (float)inst->wave_table[index_lut_next] * (1.0f / 32767.0f);
      float raw_sample = sample_a + frac * (sample_b - sample_a);




      // --- 4. Low-pass Filter (Smoothing) ---
      _lpf_filter += 0.3f * (raw_sample - _lpf_filter);

      // --- 5. ADSR Envelope Application ---
      float envelope = _adsr.process();

      return _lpf_filter * envelope * inst->volume;
    }
};

// =============================================================================
// PercussionSynth — Analog Percussion Synthesizer via DSP Modeling
// =============================================================================
class PercussionSynth {
  private:
    PercussionType _active = PercussionType::NONE;
    uint32_t _step = 0;
    uint32_t _lfsr = 0xACE1u;                    // LCG white noise generator seed
    float _kick_phase = 0.0f;                    // BUGFIX: was `static float` inside process(),
    float _tom_phase = 0.0f;                     // shared by every PercussionSynth instance and
                                                   // never reset on trigger(); now per-instance state.

    // everything below replaces per-sample expf()/sinf() calls (each hit
    // previously did 1-2 expf() + 1 sinf() PER SAMPLE, on top of the melodic
    // engines) with:
    //   1) incremental multiplicative decay: exp(-t/tau) is recomputed each
    //      sample as env *= coeff, where coeff = exp(-1/(tau*SAMPLE_RATE)) is
    //      computed ONCE per trigger() (a rare event, not 16000x/sec).
    //   2) the existing precalculated SINE_TABLE_F[] LUT instead of calling
    //      sinf() directly, exactly like MelodySynth already does.
    // Mathematically these produce the same curve as the original expf() calls
    // (env_n = coeff^n = exp(-n/tau/SR)), just far cheaper per sample.
    float _amp_env = 0.0f;
    float _amp_coeff = 1.0f;
    float _freq_env = 0.0f;
    float _freq_coeff = 1.0f;
    float _snare_body_phase = 0.0f;
    float _snare_body_env = 0.0f;
    float _snare_body_coeff = 1.0f;

    float white_noise() {
      _lfsr = (_lfsr >> 1) ^ (-(_lfsr & 1u) & 0xB400u);
      return ((float)(_lfsr & 0xFFFF) / 32768.0f) - 1.0f;
    }

    // sinf(phase_in_table_units * 2*PI/256) via the precalculated LUT, with the
    // same linear interpolation MelodySynth uses for the melodic oscillators.
    static float sineLUT(float phase_table_units) {
      uint8_t idx      = (uint8_t)phase_table_units;
      uint8_t idx_next = (idx + 1) & 0xFF;
      float   frac     = phase_table_units - (float)idx;
      float   a = SINE_TABLE_F[idx];
      float   b = SINE_TABLE_F[idx_next];
      return a + frac * (b - a);
    }

    // exp(-1/(tau_seconds * SAMPLE_RATE)), the per-sample multiplier that
    // reproduces exp(-t/tau) incrementally. Only called from trigger(), i.e.
    // once per note/hit, never from the per-sample process() path.
    static float decayCoeff(float tau_seconds) {
      return expf(-1.0f / (tau_seconds * (float)SAMPLE_RATE));
    }

  public:
    void trigger(PercussionType type) {
      _active = type;
      _step = 0;
      _kick_phase = 0.0f;                        // BUGFIX: reset oscillator phase on every hit,
      _tom_phase = 0.0f;                          // otherwise the attack starts mid-waveform (click/pop).

      _amp_env = 1.0f;
      _freq_env = 1.0f;
      _snare_body_env = 1.0f;
      _snare_body_phase = 0.0f;

      switch (type) {
        case PercussionType::KICK:
          _amp_coeff  = decayCoeff(0.08f);
          _freq_coeff = decayCoeff(0.015f);
          break;
        case PercussionType::SNARE:
          _amp_coeff        = decayCoeff(0.05f);
          _snare_body_coeff = decayCoeff(0.02f);
          break;
        case PercussionType::HIHAT_CLOSED:
          _amp_coeff = decayCoeff(0.02f);
          break;
        case PercussionType::HIHAT_OPEN:
          _amp_coeff = decayCoeff(0.15f);
          break;
        case PercussionType::CRASH:
          _amp_coeff = decayCoeff(0.4f);
          break;
        case PercussionType::TOM_LOW:
        case PercussionType::TOM_MID:
        case PercussionType::TOM_HIGH:
          _amp_coeff  = decayCoeff(0.1f);
          _freq_coeff = decayCoeff(0.02f);
          break;
        default:
          break;
      }
    }

    void setActive(PercussionType type) {
      trigger(type);
    }

    bool isActive() const {
      return _active != PercussionType::NONE;
    }

    float process() {
      if (_active == PercussionType::NONE) return 0.0f;

      float sample = 0.0f;

      switch (_active) {
        case PercussionType::KICK: {
            // Kick: Sine wave with rapid frequency drop (pitch sweep) and fast decay
            float freq = 60.0f + 120.0f * _freq_env;
            float phase_inc = freq * (256.0f / (float)SAMPLE_RATE);
            _kick_phase += phase_inc;
            if (_kick_phase >= 256.0f) _kick_phase -= 256.0f;
            // Keep every individual voice within [-1, 1] and let the mixer/soft-clip handle summation.
            sample = sineLUT(_kick_phase) * _amp_env * 1.0f;
            _amp_env  *= _amp_coeff;
            _freq_env *= _freq_coeff;
            break;
          }

        case PercussionType::SNARE: {
            // Snare: Filtered white noise mixed with a brief tonal body
            float noise = white_noise();
            // 0.2 rad/sample converted to table-index units (256 units = 2*PI rad)
            _snare_body_phase += 0.2f * (256.0f / (2.0f * (float)M_PI));
            if (_snare_body_phase >= 256.0f) _snare_body_phase -= 256.0f;
            float body = sineLUT(_snare_body_phase) * _snare_body_env;
            sample = (noise * 0.7f + body * 0.3f) * _amp_env * 0.9f;
            _amp_env         *= _amp_coeff;
            _snare_body_env  *= _snare_body_coeff;
            break;
          }

        case PercussionType::HIHAT_CLOSED: {
            // Closed Hi-Hat: High-frequency metallic noise with very fast decay
            float noise = white_noise();
            sample = noise * _amp_env * 0.6f;
            _amp_env *= _amp_coeff;
            break;
          }

        case PercussionType::HIHAT_OPEN: {
            // Open Hi-Hat: Longer metallic noise
            float noise = white_noise();
            sample = noise * _amp_env * 0.5f;
            _amp_env *= _amp_coeff;
            break;
          }

        case PercussionType::CRASH: {
            // Crash: Long metallic noise with wide frequency spectrum
            float noise = white_noise();
            sample = noise * _amp_env * 0.7f;
            _amp_env *= _amp_coeff;
            break;
          }

        case PercussionType::CLAP: {
            // Handclap: Multiple staggered noise bursts.
            float decay_envelope;
            if (_step < SCALE_SAMPLES(15) || (_step > SCALE_SAMPLES(25) && _step < SCALE_SAMPLES(35))) {
              decay_envelope = expf(-(float)(_step % SCALE_SAMPLES(20)) / (0.03f * (float)SAMPLE_RATE));
              sample = white_noise() * decay_envelope * 0.8f;
            } else {
              decay_envelope = expf(-(float)_step / (0.1f * (float)SAMPLE_RATE));
              sample = white_noise() * decay_envelope * 0.4f;
            }
            break;
          }

        case PercussionType::TOM_LOW:
        case PercussionType::TOM_MID:
        case PercussionType::TOM_HIGH: {
            float base_freq = (_active == PercussionType::TOM_LOW) ? 90.0f : ((_active == PercussionType::TOM_MID) ? 130.0f : 180.0f);
            float freq = base_freq + 40.0f * _freq_env;
            float phase_inc = freq * (256.0f / (float)SAMPLE_RATE);
            _tom_phase += phase_inc;
            if (_tom_phase >= 256.0f) _tom_phase -= 256.0f;
            sample = sineLUT(_tom_phase) * _amp_env * 1.0f;
            _amp_env  *= _amp_coeff;
            _freq_env *= _freq_coeff;
            break;
          }

        default:
          sample = 0.0f;
          break;
      }

      _step++;
      // Automatically stop the percussion voice after sufficient time
      if (_step > SCALE_SAMPLES(8000)) {
        _active = PercussionType::NONE;
      }

      return sample;
    }
};

// =============================================================================
// DelayEffect — Echo / Delay Effect with Feedback
// =============================================================================
class DelayEffect {
  private:
    float _buffer[DELAY_BUFFER_SIZE] = {0.0f};    // Circular delay line
    uint16_t _ptr = 0;                          // Read/write pointer
    float _feedback_prev = 0.0f;                // State for feedback filtering
    static constexpr float FEEDBACK = 0.45f;    // Feedback gain (0.0 to 1.0)

  public:
    float process(float input_sample) {
      // 1. Read delayed sample from the past
      const float delayed_sample = _buffer[_ptr];

      // 2. Simple low-pass filtering on feedback path to prevent harsh high frequencies
      _feedback_prev = 0.7f * _feedback_prev + 0.3f * delayed_sample;

      // 3. Mix current input sample with filtered feedback and write back to buffer
      // Bound what we store so the delay line itself never grows
      // past full scale.
      float buffer_input = constrain(input_sample + (_feedback_prev * FEEDBACK), -1.0f, 1.0f);
      _buffer[_ptr] = buffer_input;

      // 4. Advance circular pointer with bounds check
      _ptr++;
      if (_ptr >= DELAY_BUFFER_SIZE) {
        _ptr = 0;
      }

      // 5. Output mix: Dry + Wet signal
      return input_sample + (delayed_sample * 0.70f);
    }
};


