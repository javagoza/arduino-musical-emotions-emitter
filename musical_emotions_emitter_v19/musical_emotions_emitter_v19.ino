#pragma once

// =============================================================================
// SOUND MESSENGER WITH AM MODULATION VIA PWM
// =============================================================================
// System Overview:
//
// 1. loop() computes audio samples "in the background" (outside the ISR)
//    using the synthesizers defined in dsp.h, leaving them ready
//    in a circular buffer (audio_buf).
//
// 2. A hardware timer ISR (cb_audio) consumes that buffer at a constant
//    rate (SAMPLE_RATE) and writes each sample to two outputs:
//    a) A 12-bit DAC ("normal" audio output).
//    b) A PWM signal (594 kHz by default, user-adjustable from the AM
//       Frequency setting) whose duty cycle is modulated to transmit the
//       audio signal as AM over a radio frequency (RF) carrier.
//
// 3. A Settings menu (UI_SETTINGS, see "SETTINGS MENU STATE" below) lets the
//    user adjust melody/percussion/master volume, the AM carrier
//    frequency, and which output(s) are active, all via the same rotary
//    encoder used for song selection.
//
// Separating DSP computation (loop) from hardware writing (ISR) is what
// allows CPU-heavy synthesis not to block or delay audio sampling,
// which must be rigorously periodic.
// =============================================================================

#include "synth_types.h"
#include "dsp.h"
// emotions songs
#include "emotions.h"
// demo songs
#include "demo.h"

#include "pwm.h"
#include "FspTimer.h"
#include "analogWave.h"
#include <math.h>

#include <stdint.h>
#include <string.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =============================================================================
// COMPILE-TIME AUDIO LOGGING SWITCH
// =============================================================================
// Set to 1 only when diagnostic Serial output/counters are needed. With 0,
// diagnostic state, log buffers and diagnostic functions are excluded from
// the build to save SRAM/flash and reduce runtime overhead.
#define ENABLE_AUDIO_LOGS 0


// =============================================================================
// SETTINGS MENU STATE
// =============================================================================
// Which physical output(s) are currently active.
enum class OutputMode : uint8_t {
    DAC_ONLY = 0,     // Audio DAC monitor only, RF carrier stopped
    RF_ONLY,          // RF carrier only, DAC held at idle (silent)
    DAC_AND_RF,       // Both outputs active (default)
    MUTE              // Both outputs silenced
};
constexpr uint8_t OUTPUT_MODE_COUNT = 4;


// =============================================================================
// HARDWARE CONFIGURATION AND GENERAL CONSTANTS
// =============================================================================

const uint8_t PIN_RFAM = 9;                  // Output pin for AM modulated RF carrier (PWM)
const uint8_t PIN_MONITOR_DAC = A0;          // Internal DAC pin for direct analog audio monitoring

// NOTE: the AM carrier frequency is a runtime setting (g_am_frequency_khz, adjustable from the
// Settings menu) defined in the "SETTINGS MENU STATE" section below.


// OLED Display Configuration (128x32)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary Encoder Pins
const int PIN_ENC_CLK = 2; // CLK (A) Pin
const int PIN_ENC_DT = 3;  // DT (B) Pin
const int PIN_ENC_SW = 4;  // Encoder Push Button Pin

// UI States Management
enum UIState {
    UI_MENU_SELECTION, // Browsing a song list (PLAYER[] or DEMO_SONGS[] -- see g_browsing_demo_songs)
    UI_PLAYING,        // Playing the selected song from whichever list is active
    UI_SETTINGS        // Settings menu: volume, AM frequency, output routing
};
UIState current_ui_state = UI_MENU_SELECTION;

int selected_song_index = 0;

// -----------------------------------------------------------------------
// Active Song List (normal playback vs. Demo Mode)
// -----------------------------------------------------------------------
// Demo Mode is NOT a separate UI state/render/playback path: it is simply
// PLAYER[] swapped out for DEMO_SONGS[] (see demo.h). Selecting and playing
// a demo goes through the exact same UI_MENU_SELECTION / UI_PLAYING code as
// a normal song -- same oscilloscope/monitoring panel, same RF/output-mode
// handling, same "return to the list when it finishes" behaviour. See
// toggleSongList() below.
const Song* g_current_song_list = PLAYER;
int g_current_song_count = TOTAL_SONGS;
bool g_browsing_demo_songs = false;

// -----------------------------------------------------------------------
// Rotary Encoder Debouncing (KY-040)
// -----------------------------------------------------------------------
// The rotation is decoded with a full quadrature state machine (the
// classic "Ben Buxton" full-step table) instead of a single CLK-edge plus
// a millis() time window. A single-edge + time debounce still lets a
// mechanical bounce that partially turns and springs back register as a
// real step (the "goes back to the previous option" bug). The state
// machine only reports CW/CCW when the encoder has travelled through a
// complete, valid sequence of quadrature states and landed back on a
// detent (R_START); a bounce that reverses mid-travel simply walks back
// down the same table path and produces no output at all.
#define ROT_DIR_NONE 0x0
#define ROT_DIR_CW   0x10
#define ROT_DIR_CCW  0x20

#define ROT_R_START     0x0
#define ROT_R_CW_FINAL  0x1
#define ROT_R_CW_BEGIN  0x2
#define ROT_R_CW_NEXT   0x3
#define ROT_R_CCW_BEGIN 0x4
#define ROT_R_CCW_FINAL 0x5
#define ROT_R_CCW_NEXT  0x6

// Table is indexed by [current_state][pin_state], where pin_state is
// (DT << 1 | CLK). Adapted from Ben Buxton's rotary encoder algorithm.
const uint8_t ROTARY_TRANSITION_TABLE[7][4] = {
    // ROT_R_START
    {ROT_R_START, ROT_R_CW_BEGIN, ROT_R_CCW_BEGIN, ROT_R_START},
    // ROT_R_CW_FINAL
    {ROT_R_CW_NEXT, ROT_R_START, ROT_R_CW_FINAL, ROT_R_START | ROT_DIR_CW},
    // ROT_R_CW_BEGIN
    {ROT_R_CW_NEXT, ROT_R_CW_BEGIN, ROT_R_START, ROT_R_START},
    // ROT_R_CW_NEXT
    {ROT_R_CW_NEXT, ROT_R_CW_BEGIN, ROT_R_CW_FINAL, ROT_R_START},
    // ROT_R_CCW_BEGIN
    {ROT_R_CCW_NEXT, ROT_R_START, ROT_R_CCW_BEGIN, ROT_R_START},
    // ROT_R_CCW_FINAL
    {ROT_R_CCW_NEXT, ROT_R_CCW_FINAL, ROT_R_START, ROT_R_START | ROT_DIR_CCW},
    // ROT_R_CCW_NEXT
    {ROT_R_CCW_NEXT, ROT_R_CCW_FINAL, ROT_R_CCW_BEGIN, ROT_R_START},
};
uint8_t rotary_encoder_state = ROT_R_START;

// -----------------------------------------------------------------------
// Encoder Button Debounce
// -----------------------------------------------------------------------
bool previous_button_state = HIGH;
unsigned long last_click_time = 0;
const unsigned long BUTTON_DEBOUNCE_MS = 40; // 200 ms made short encoder presses unreliable

// KY-040 modules share the same mechanical shaft/bushing between the
// rotation contacts and the push switch, so pressing the button can jolt
// the CLK/DT contacts enough to be misread as a rotation step. To prevent
// that, rotary decoding is suspended while the button is held down and for
// a short guard window after any confirmed press/release transition.
// last_button_activity_ms is updated by updateEncoderButton() on every
// debounced press/release edge, and is read by updateRotaryEncoder().
unsigned long last_button_activity_ms = 0;
const unsigned long ENCODER_BUTTON_ROTARY_GUARD_MS = 150; // suppress rotation right after button activity

// Long-press detection (button held down) used to toggle the active song
// list (normal songs <-> Demo Mode) or enter Settings Mode from the main
// menu, without needing any extra hardware or a second button. Two
// thresholds share the same physical button:
//   - LONG_PRESS_MS      (shorter hold) -> toggle song list (toggleSongList())
//   - SETTINGS_PRESS_MS  (longer hold)  -> Settings Mode
// SETTINGS_PRESS_MS fires live, while the button is still held (mirroring
// the original Demo-mode UX); the list-toggle action is instead resolved on
// button RELEASE, once we know the user did not keep holding past
// SETTINGS_PRESS_MS. See updateEncoderButton() for the full logic.
unsigned long button_press_start_time = 0;
bool button_long_press_fired = false;
bool button_settings_press_fired = false;

// Playback stop button state. A single physical press stops playback immediately.
bool playback_stop_button_latched = false;

const unsigned long LONG_PRESS_MS = 600;
const unsigned long SETTINGS_PRESS_MS = 1800;


enum class ClippingReason : uint8_t {
    MIX_OVERLOAD = 0,
    DELAY_OVERLOAD,
    OUTPUT_OVERLOAD
};


// --- Physical duty range derived constants, computed ONCE in setup() ---
constexpr float PI_F = 3.14159265358979323846f;

extern int16_t SINE_TABLE[256];
extern int16_t TRIANGLE_TABLE[256];
extern int16_t SQUARE_TABLE[256];
extern int16_t SAW_TABLE[256];
extern int16_t BRIGHT_TRIANGLE_TABLE[256];
extern int16_t WARM_TRIANGLE_TABLE[256];

extern float SINE_TABLE_F[256];
// -----------------------------------------------------------------------------
// HARDWARE INSTANCES AND GLOBAL STATE
// -----------------------------------------------------------------------------
PwmOut carrier_pwm(PIN_RFAM);
volatile uint32_t *g_duty_reg = nullptr; // Will point to GTCCR[0] or GTCCR[1]

// RF carrier control.
// The audio timer and DAC continue running when the carrier is stopped.
void stopRFCarrier() {
    carrier_pwm.suspend();
    pinMode(PIN_RFAM, OUTPUT);
    digitalWrite(PIN_RFAM, LOW);
}

void startRFCarrier() {
    carrier_pwm.resume();
}
FspTimer audio_timer;
analogWave dac(DAC);

// -----------------------------------------------------------------------------
// ENGINE POOLS AND EFFECTS (Static Polyphony Limit)
// -----------------------------------------------------------------------------
MelodySynth melodic_engines[MAX_MELODIC_VOICES];
PercussionSynth percussion_engines[MAX_PERCUSSION_VOICES];
DelayEffect delay_fx;
SequencerState seq;
AudioBuffer audio_buf;

// =============================================================================
// REAL-TIME OSCILLOSCOPE (128-sample waveform on the OLED)
// =============================================================================
// Design notes:
//  - Capture happens in loop() at PUSH time (inside the UI_PLAYING while()),
//    never inside cb_audio(). The ISR is not touched at all: no extra work,
//    no noInterrupts()/interrupts(), no risk of adding jitter to sample
//    output. This is intentional and different from touching the pop side.
//  - Drawing (I2C) is deliberately NOT free-running. It only fires in the
//    idle window where the while() loop stalls because the ring buffer is
//    full (audioBufferHasSpace() == false) -- i.e. the ISR has maximum lead
//    time and can keep draining uninterrupted while we block on I2C. It is
//    additionally throttled to GRAPH_UPDATE_INTERVAL_MS so a very fast/short
//    song can't turn this into a free-running I2C hog.
//  - Single capture buffer, single-threaded cooperative access: capture
//    pauses (wave_buffer_ready) while a frame is waiting to be drawn, so
//    there's no producer/consumer race and no lock needed anywhere.
constexpr uint8_t WAVE_GRAPH_SIZE = 128;
constexpr unsigned long GRAPH_UPDATE_INTERVAL_MS = 40; // ~25 FPS, tune to taste

// Playback oscilloscope zoom is deliberately a signed UI value. Zero is the
// normal playback view. Clockwise rotation increases the time scale and
// counter-clockwise rotation decreases it, with nine steps in either direction.
constexpr int8_t SCOPE_ZOOM_MIN = -9;
constexpr int8_t SCOPE_ZOOM_MAX = 9;

int8_t wave_capture_buf[WAVE_GRAPH_SIZE];
int8_t wave_capture_min_buf[WAVE_GRAPH_SIZE];
int8_t wave_capture_max_buf[WAVE_GRAPH_SIZE];
int8_t wave_capture_interval_min = 127;
int8_t wave_capture_interval_max = -127;
uint8_t  wave_capture_pos = 0;
uint16_t wave_capture_counter = 0;
uint16_t wave_capture_stride = 1;   // Current capture interval in audio samples.
uint16_t wave_capture_base_stride = 1; // Standard playback interval.

int8_t scope_zoom_level = 0;
bool scope_fullscreen = false;
bool scope_display_dirty = false;

// Lightweight scope metrics. They are accumulated only when a display sample
// is captured, never inside cb_audio(), and therefore add negligible DSP load.
float scope_peak = 0.0f;
float scope_sum_sq = 0.0f;
uint16_t scope_sample_count = 0;
uint16_t scope_zero_crossings = 0;
int8_t scope_prev_sample = 0;
bool  wave_buffer_ready = false;
unsigned long last_graph_draw_ms = 0;

// The rotary decoder is polled at the same playback checkpoint as the panic
// button. Its state is kept separate from menu navigation so rotation cannot
// change song selection while audio is running.
uint8_t playback_rotary_button_state = HIGH;

// TODO: audio_buf's actual capacity (BUFFER_MASK + 1) and SAMPLE_RATE live in
// synth_types.h, which wasn't in the files I reviewed. Before trusting this
// on real hardware, confirm that (capacity_in_samples / SAMPLE_RATE * 1000)
// is comfortably above the measured display.display() I2C blocking time
// (typically ~8-14ms for a 128x32 SSD1306 at 400kHz) -- otherwise the
// "buffer full" window isn't actually safe and you'll want to either shrink
// WAVE_GRAPH_SIZE, lower GRAPH_UPDATE_INTERVAL_MS's priority, or grow the
// ring buffer. audio_underrun_events/printAudioArtifactReport() (already in
// this file) will tell you empirically if the margin is too tight.

// Shared "currently playing Song" pointer, set by change_song() from
// whichever list is active (g_current_song_list -- PLAYER[] or
// DEMO_SONGS[]). load_step() and compute_next_dsp_sample() read from it, so
// the same mixer/sequencer code drives both song lists without duplication.
const Song* g_active_song = nullptr;


#if ENABLE_AUDIO_LOGS
// =============================================================================
// AUDIO DIAGNOSTICS
// =============================================================================
// Diagnostic storage is intentionally compact because this project runs on an
// embedded target with limited SRAM. The log stores only the first events that
// occur during one song, while aggregate counters keep the complete totals.

struct ClippingLogEntry {
    uint8_t step;
    uint32_t sample;
    float value;
    ClippingReason reason;
};

struct ClickLogEntry {
    uint8_t step;
    uint32_t sample;
    float previous;
    float current;
    float delta;
};

static constexpr uint8_t MAX_CLIPPING_LOG_ENTRIES = 24;
static constexpr uint8_t MAX_CLICK_LOG_ENTRIES = 32;
static constexpr float CLICK_DELTA_THRESHOLD = 0.25f;

ClippingLogEntry clipping_log[MAX_CLIPPING_LOG_ENTRIES];
ClickLogEntry click_log[MAX_CLICK_LOG_ENTRIES];

uint8_t clipping_log_count = 0;
uint8_t click_log_count = 0;
uint32_t clipping_event_count = 0;
uint32_t prevented_overload_count = 0;
uint32_t audio_sample_counter = 0;
uint32_t audio_underrun_samples = 0;
uint32_t audio_underrun_events = 0;
uint32_t audio_underrun_current_length = 0;
uint32_t audio_underrun_max_length = 0;
uint16_t audio_buffer_min_level = 0xFFFF;
uint16_t audio_buffer_max_level = 0;
uint64_t audio_buffer_level_sum = 0;
uint32_t audio_buffer_level_samples = 0;
bool audio_underrun_active = false;
volatile uint32_t dsp_call_count = 0;
volatile uint32_t dsp_us_sum = 0;
volatile uint32_t dsp_us_worst = 0;
volatile uint32_t isr_tick_count = 0;
float previous_audio_sample = 0.0f;
bool previous_audio_sample_valid = false;

const char* clippingReasonToString(ClippingReason reason) {
    switch (reason) {
        case ClippingReason::MIX_OVERLOAD:    return "MIX_OVERLOAD";
        case ClippingReason::DELAY_OVERLOAD:  return "DELAY_OVERLOAD";
        case ClippingReason::OUTPUT_OVERLOAD: return "OUTPUT_OVERLOAD";
        default:                              return "UNKNOWN";
    }
}

void resetAudioDiagnostics() {
    clipping_log_count = 0;
    click_log_count = 0;
    clipping_event_count = 0;
    prevented_overload_count = 0;
    audio_sample_counter = 0;
    audio_underrun_samples = 0;
    audio_underrun_events = 0;
    audio_underrun_current_length = 0;
    audio_underrun_max_length = 0;
    audio_buffer_min_level = 0xFFFF;
    audio_buffer_max_level = 0;
    audio_buffer_level_sum = 0;
    audio_buffer_level_samples = 0;
    audio_underrun_active = false;
    previous_audio_sample = 0.0f;
    previous_audio_sample_valid = false;
}

inline uint16_t audioBufferLevel() {
    return (uint16_t)((audio_buf.head - audio_buf.tail) & BUFFER_MASK);
}

void recordClippingEvent(float value, ClippingReason reason, uint8_t step) {
    clipping_event_count++;
    if (clipping_log_count >= MAX_CLIPPING_LOG_ENTRIES) return;
    ClippingLogEntry &entry = clipping_log[clipping_log_count++];
    entry.step = step;
    entry.sample = audio_sample_counter;
    entry.value = value;
    entry.reason = reason;
}

void recordClickEvent(float previous, float current, uint8_t step) {
    const float delta = current - previous;
    if (click_log_count >= MAX_CLICK_LOG_ENTRIES) return;
    ClickLogEntry &entry = click_log[click_log_count++];
    entry.step = step;
    entry.sample = audio_sample_counter;
    entry.previous = previous;
    entry.current = current;
    entry.delta = delta;
}

void updateAudioArtifactDiagnostics(float sample) {
    if (previous_audio_sample_valid) {
        const float delta = sample - previous_audio_sample;
        if (fabsf(delta) >= CLICK_DELTA_THRESHOLD) {
            const uint8_t step = (seq.current_song_idx >= 0)
                ? seq.voices[0].step_idx : 0;
            recordClickEvent(previous_audio_sample, sample, step);
        }
    }
    previous_audio_sample = sample;
    previous_audio_sample_valid = true;
}

void updateAudioBufferDiagnostics() {
    const uint16_t level = audioBufferLevel();
    if (level < audio_buffer_min_level) audio_buffer_min_level = level;
    if (level > audio_buffer_max_level) audio_buffer_max_level = level;
    audio_buffer_level_sum += level;
    audio_buffer_level_samples++;
}

void recordAudioUnderrun() {
    audio_underrun_samples++;
    if (!audio_underrun_active) {
        audio_underrun_events++;
        audio_underrun_active = true;
        audio_underrun_current_length = 0;
    }
    audio_underrun_current_length++;
    if (audio_underrun_current_length > audio_underrun_max_length) {
        audio_underrun_max_length = audio_underrun_current_length;
    }
}

void clearAudioUnderrunState() {
    audio_underrun_active = false;
    audio_underrun_current_length = 0;
}

void printDspPerformanceReport() {
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("        DSP PERFORMANCE REPORT"));
    Serial.println(F("========================================"));
    const float average_us = dsp_call_count > 0
        ? (float)dsp_us_sum / (float)dsp_call_count
        : 0.0f;
    const float budget_us = 1000000.0f / (float)SAMPLE_RATE;
    Serial.print(F("DSP calls: "));
    Serial.println(dsp_call_count);
    Serial.print(F("DSP average time (us): "));
    Serial.println(average_us, 2);
    Serial.print(F("DSP worst time (us): "));
    Serial.println(dsp_us_worst);
    Serial.print(F("Sample budget (us): "));
    Serial.println(budget_us, 2);
    Serial.print(F("ISR ticks: "));
    Serial.println(isr_tick_count);
    Serial.println(F("========================================"));
}

void printClippingLog() {
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("             CLIPPING REPORT"));
    Serial.println(F("========================================"));
    Serial.print(F("Events: "));
    Serial.println(clipping_event_count);
    Serial.print(F("Prevented overloads: "));
    Serial.println(prevented_overload_count);

    if (clipping_event_count == 0) {
        Serial.println(F("No clipping detected."));
    } else {
        for (uint8_t i = 0; i < clipping_log_count; ++i) {
            const ClippingLogEntry &entry = clipping_log[i];
            Serial.print('#');
            Serial.print(i);
            Serial.print(F(" step="));
            Serial.print(entry.step);
            Serial.print(F(" sample="));
            Serial.print(entry.sample);
            Serial.print(F(" value="));
            Serial.print(entry.value, 5);
            Serial.print(F(" reason="));
            Serial.println(clippingReasonToString(entry.reason));
        }
        if (clipping_event_count > clipping_log_count) {
            Serial.print(F("Additional clipping events not stored: "));
            Serial.println(clipping_event_count - clipping_log_count);
        }
    }

    Serial.println(F("Final AGC gain: 1.00000"));
    Serial.println(F("========================================"));
}

void printAudioArtifactReport() {
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("        AUDIO ARTIFACT REPORT"));
    Serial.println(F("========================================"));
    Serial.print(F("Clicks/discontinuities: "));
    Serial.println(click_log_count);
    Serial.print(F("Audio underrun samples: "));
    Serial.println(audio_underrun_samples);
    Serial.print(F("Audio underrun events: "));
    Serial.println(audio_underrun_events);
    Serial.print(F("Longest underrun: "));
    Serial.println(audio_underrun_max_length);

    const float average_level = audio_buffer_level_samples > 0
        ? (float)audio_buffer_level_sum / (float)audio_buffer_level_samples
        : 0.0f;
    Serial.print(F("Minimum buffer level: "));
    Serial.println(audio_buffer_min_level == 0xFFFF ? 0 : audio_buffer_min_level);
    Serial.print(F("Maximum buffer level: "));
    Serial.println(audio_buffer_max_level);
    Serial.print(F("Average buffer level: "));
    Serial.println(average_level, 2);

    for (uint8_t i = 0; i < click_log_count; ++i) {
        const ClickLogEntry &entry = click_log[i];
        Serial.print('#');
        Serial.print(i);
        Serial.print(F(" step="));
        Serial.print(entry.step);
        Serial.print(F(" sample="));
        Serial.print(entry.sample);
        Serial.print(F(" previous="));
        Serial.print(entry.previous, 5);
        Serial.print(F(" current="));
        Serial.print(entry.current, 5);
        Serial.print(F(" delta="));
        Serial.println(entry.delta, 5);
    }

    if (audio_underrun_samples > 0 && audio_underrun_current_length > 0) {
        // Keep the final active underrun episode reflected in the maximum.
        if (audio_underrun_current_length > audio_underrun_max_length) {
            audio_underrun_max_length = audio_underrun_current_length;
        }
    }
    Serial.println(F("========================================"));
}

#endif // ENABLE_AUDIO_LOGS

uint32_t MIDI_PHASE[128];
#define LUT_ASIN_SIZE 513 // Table resolution: power of 2 + 1 for interpolation
static uint16_t lut_asin_duty[LUT_ASIN_SIZE];

// AM Modulation constants (computed once at compile time)
constexpr float DUTY_MIN_PERC = 5.0f;
constexpr float DUTY_MAX_PERC = 45.0f;
constexpr float DUTY_CENTER_PERC = (DUTY_MIN_PERC + DUTY_MAX_PERC) * 0.5f; // 25.0f
constexpr float DUTY_HALF_RANGE_PERC = (DUTY_MAX_PERC - DUTY_MIN_PERC) * 0.5f; // 20.0f

// Cached in setup() from gpt_reg_base->GTPR
uint32_t g_period_counts = 0;
float SIN_MIN, SIN_MAX, SIN_CENTER, SIN_HALF_RANGE;

void initializePredistortion() {
    float duty_min_frac = DUTY_MIN_PERC / 100.0f; // 0.05
    float duty_max_frac = DUTY_MAX_PERC / 100.0f; // 0.45
    
    SIN_MIN        = sinf(PI_F * duty_min_frac);    // sin(9°)  ≈ 0.15643
    SIN_MAX        = sinf(PI_F * duty_max_frac);    // sin(81°) ≈ 0.98769
    SIN_CENTER     = (SIN_MAX + SIN_MIN) * 0.5f;
    SIN_HALF_RANGE = (SIN_MAX - SIN_MIN) * 0.5f;
}

// Converts a [-1, 1] sample to a value ready for GTCCR[1], WITH predistortion
// that linearizes the amplitude of the RF fundamental harmonic.
inline uint32_t sampleToRegister(float sample) {
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    // (Predistortion logic implementation)
    // Desired amplitude (linear in the sample) expressed as sin(pi*duty)
    float target_sin = SIN_CENTER + sample * SIN_HALF_RANGE;

    // Safety clamp: rounding errors can leave target_sin
    // slightly outside [-1,1], and asinf(x>1) returns NaN.
    if (target_sin > 1.0f)  target_sin = 1.0f;
    if (target_sin < -1.0f) target_sin = -1.0f;

    float duty_frac = asinf(target_sin) / PI_F;      // 0.0 .. 0.5 (fraction, no %)

    float counts_f = (float)g_period_counts * duty_frac;
    return (uint32_t)(counts_f + 0.5f);

}

void initializePredistortionLUT() {
    initializePredistortion(); // Fills SIN_MIN/MAX/CENTER/HALF_RANGE
    for (int i = 0; i < LUT_ASIN_SIZE; i++) {
        float sample = -1.0f + 2.0f * i / (LUT_ASIN_SIZE - 1);
        lut_asin_duty[i] = sampleToRegister(sample);
    }
}

// Converts a [-1, 1] sample directly to a value ready for GTCCR[1]
// Ultra-fast "producer" version, without hot asinf():
inline uint32_t sampleToRegisterLUT(float sample) {
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    float idx_f = (sample + 1.0f) * 0.5f * (LUT_ASIN_SIZE - 1);
    int idx = (int)(idx_f + 0.5f); // No interpolation (513 points are enough)
    return lut_asin_duty[idx];
}

// Symmetric mapping -1.0f..1.0f -> 0..4095 (centered at 2048, silence/idle)
constexpr float DAC_MAX_COUNTS = 4095.0f;

inline uint16_t sampleToDac(float sample) {
	if (sample > 1.0f)  sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;

    float val_f = (sample + 1.0f) * 0.5f * DAC_MAX_COUNTS;  // 0..4095
    return (uint16_t)(val_f + 0.5f);

}

// =============================================================================
// PRECALCULATION AUXILIARY FUNCTIONS
// =============================================================================
/**
 * @brief Draws one waveform frame using differential erase (previous frame
 * in BLACK, new frame in WHITE) instead of clearDisplay(), so only the two
 * polylines are touched -- no full-screen flicker, minimal bytes changed
 * in the SSD1306 GDDRAM before display.display() flushes it over I2C.
 * Called only from the "ring buffer full" idle window in loop(), never
 * from inside an ISR.
 */
// Reset only the display-side waveform accumulation. Audio synthesis and the
// audio ring buffer are deliberately untouched.
void resetScopeCaptureFrame() {
    wave_capture_pos = 0;
    wave_capture_counter = 0;
    wave_capture_interval_min = 127;
    wave_capture_interval_max = -127;
    wave_buffer_ready = false;
    scope_peak = 0.0f;
    scope_sum_sq = 0.0f;
    scope_sample_count = 0;
    scope_zero_crossings = 0;
    scope_prev_sample = 0;
}

// Additive zoom step, in audio samples per captured display point, applied
// per detent from the SECOND step onward (see updateScopeCaptureStride()).
// Chosen equal to the base stride (5 samples/point @ 16kHz, 40ms refresh,
// 128 points -- see the wave_capture_base_stride computation in setup()):
//   - Zoom OUT (level > 1): stride grows 5,10,15,...,45 samples/point at
//     level 9, i.e. one captured frame takes 8*STEP=... 40ms,80ms,...,360ms
//     to fill (WAVE_GRAPH_SIZE * stride / SAMPLE_RATE). Still comfortably
//     "alive" (< 0.4s to first paint) instead of doubling out to 45*... ms
//     the way the old x2-per-step scheme did (which reached 65535 clamped
//     within a handful of steps and then did nothing for the rest).
//   - Zoom IN (level < -1): stride shrinks by the same amount and clamps at
//     1 (finest possible: one raw sample per display column). Because the
//     base stride is only 5, this saturates after a single extra step --
//     that's expected, there simply isn't room for more resolution below
//     one sample per point.
// wave_capture_stride is uint16_t (max 65535) and WAVE_GRAPH_SIZE*stride
// must stay well under that engineering margin; with SCOPE_ZOOM_MAX/MIN at
// +/-9 and this step size the extremes are 45 and 1, both far from any
// overflow. If you want a wider zoom-out range, any STEP up to ~40 keeps
// the level-9 fill time under ~1s (128 * (5+8*STEP) / SAMPLE_RATE); avoid
// going much higher than that or the top end starts feeling frozen again
// even though it's technically still redrawing.
constexpr uint16_t SCOPE_ZOOM_STEP_SAMPLES = 5;

// Recalculate the sample interval after a zoom change.
//   |scope_zoom_level| == 0 -> not fullscreen, base stride (unused for draw).
//   |scope_zoom_level| == 1 -> first detent: fullscreen at the SAME time
//                              base as the normal view (no zoom applied yet).
//   |scope_zoom_level| >= 2 -> each further detent adds/subtracts
//                              SCOPE_ZOOM_STEP_SAMPLES samples/point.
void updateScopeCaptureStride() {
    const uint8_t magnitude = (uint8_t)abs((int)scope_zoom_level);

    if (magnitude == 0) {
        wave_capture_stride = wave_capture_base_stride;
        return;
    }

    const int32_t extra_steps = (int32_t)magnitude - 1; // 0 on the first detent
    int32_t stride;

    if (scope_zoom_level > 0) {
        stride = (int32_t)wave_capture_base_stride +
                 extra_steps * (int32_t)SCOPE_ZOOM_STEP_SAMPLES;
        if (stride > 65535L) stride = 65535L;
    } else {
        stride = (int32_t)wave_capture_base_stride -
                 extra_steps * (int32_t)SCOPE_ZOOM_STEP_SAMPLES;
        if (stride < 1L) stride = 1L;
    }

    wave_capture_stride = (uint16_t)stride;
}

// Apply one encoder step to the playback oscilloscope. This function performs
// no OLED I2C operation; the actual display transition is deferred until the
// existing ring-buffer-full safe window in loop().
void adjustPlaybackScopeZoom(bool clockwise) {
    int8_t next_level = scope_zoom_level;

    if (clockwise) {
        if (next_level >= SCOPE_ZOOM_MAX) return;
        ++next_level;
    } else {
        if (next_level <= SCOPE_ZOOM_MIN) return;
        --next_level;
    }

    scope_zoom_level = next_level;
    scope_fullscreen = (scope_zoom_level != 0);
    updateScopeCaptureStride();
    resetScopeCaptureFrame();
    scope_display_dirty = true;
}

// Decode one complete rotary detent during playback. This is intentionally
// called from the same bounded checkpoint that polls the panic button.
void updatePlaybackRotaryEncoder() {
    const uint8_t button_reading = (uint8_t)digitalRead(PIN_ENC_SW);

    if (button_reading != playback_rotary_button_state) {
        playback_rotary_button_state = button_reading;
        last_button_activity_ms = millis();
        rotary_encoder_state = ROT_R_START;
        return;
    }

    if (button_reading == LOW ||
        millis() - last_button_activity_ms < ENCODER_BUTTON_ROTARY_GUARD_MS) {
        rotary_encoder_state = ROT_R_START;
        return;
    }

    const uint8_t pin_state =
        (digitalRead(PIN_ENC_DT) << 1) | digitalRead(PIN_ENC_CLK);

    rotary_encoder_state =
        ROTARY_TRANSITION_TABLE[rotary_encoder_state & 0x0F][pin_state];

    const uint8_t direction = rotary_encoder_state & 0x30;
    if (direction == ROT_DIR_NONE) return;

    adjustPlaybackScopeZoom(direction == ROT_DIR_CW);
}

void drawFullscreenWaveformFrame() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Reserve four columns on the right for the vertical sound-power meter.
    // The waveform therefore has 124 horizontal pixels, while the captured
    // frame remains exactly 128 data points.
    constexpr int16_t WAVEFORM_WIDTH = SCREEN_WIDTH - 4;
    constexpr int16_t METER_X = WAVEFORM_WIDTH;

    const float rms = (scope_sample_count > 0)
        ? sqrtf(scope_sum_sq / (float)scope_sample_count) : 0.0f;
    const float peak = scope_peak;

    const int16_t centre_y = SCREEN_HEIGHT / 2;
    const int16_t amplitude = 15;

    for (uint8_t x = 0; x < WAVEFORM_WIDTH - 1; ++x) {
        const uint16_t i0 =
            ((uint32_t)x * (WAVE_GRAPH_SIZE - 1)) / (WAVEFORM_WIDTH - 1);
        const uint16_t i1 =
            ((uint32_t)(x + 1) * (WAVE_GRAPH_SIZE - 1)) / (WAVEFORM_WIDTH - 1);

        const int y0 = centre_y -
            ((int)wave_capture_buf[i0] * amplitude) / 127;
        const int y1 = centre_y -
            ((int)wave_capture_buf[i1] * amplitude) / 127;

        const int min_y0 = centre_y -
            ((int)wave_capture_max_buf[i0] * amplitude) / 127;
        const int max_y0 = centre_y -
            ((int)wave_capture_min_buf[i0] * amplitude) / 127;
        const int min_y1 = centre_y -
            ((int)wave_capture_max_buf[i1] * amplitude) / 127;
        const int max_y1 = centre_y -
            ((int)wave_capture_min_buf[i1] * amplitude) / 127;

        display.drawLine(x, y0, x + 1, y1, SSD1306_WHITE);
        display.drawFastVLine(x, min_y0, max(1, max_y0 - min_y0 + 1), SSD1306_WHITE);
        display.drawFastVLine(x + 1, min_y1, max(1, max_y1 - min_y1 + 1), SSD1306_WHITE);
    }

    // A faint reference axis makes the full-height waveform easier to read.
    for (uint8_t x = 0; x < WAVEFORM_WIDTH; x += 8) {
        display.drawPixel(x, centre_y, SSD1306_WHITE);
    }

    // Vertical RMS power bar, with a one-pixel peak marker. It uses the full
    // 32-pixel screen height and does not consume waveform columns.
    const int rms_height = constrain(
        (int)(rms * (SCREEN_HEIGHT - 1)), 0, SCREEN_HEIGHT - 1);
    const int peak_y = constrain(
        SCREEN_HEIGHT - 1 - (int)(peak * (SCREEN_HEIGHT - 1)),
        0, SCREEN_HEIGHT - 1);

    display.drawFastVLine(METER_X, 0, SCREEN_HEIGHT, SSD1306_WHITE);
    display.drawFastVLine(METER_X + 1, 0, SCREEN_HEIGHT, SSD1306_WHITE);

    if (rms_height > 0) {
        display.fillRect(
            METER_X + 2, SCREEN_HEIGHT - rms_height, 2, rms_height,
            SSD1306_WHITE);
    }

    display.drawFastHLine(METER_X, peak_y, 4, SSD1306_WHITE);

    display.display();

    scope_peak = 0.0f;
    scope_sum_sq = 0.0f;
    scope_sample_count = 0;
    scope_zero_crossings = 0;
}

void drawWaveformFrame() {
    if (scope_fullscreen) {
        drawFullscreenWaveformFrame();
        return;
    }

    // Layout for the 128x32 OLED:
    //   rows 0..6   compact status line
    //   rows 8..25  waveform
    //   rows 27..31 level meter
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    const float rms = (scope_sample_count > 0)
        ? sqrtf(scope_sum_sq / (float)scope_sample_count) : 0.0f;
    const float peak = scope_peak;

    // Zero-crossing frequency estimate. This is deliberately only an
    // approximate indication; with polyphony it represents the apparent
    // dominant periodicity, not an individual voice's exact pitch.
    float freq_hz = 0.0f;
    if (scope_sample_count > 1 && scope_zero_crossings >= 2 && wave_capture_stride > 0) {
        const float window_seconds =
            ((float)(scope_sample_count * wave_capture_stride)) / (float)SAMPLE_RATE;
        if (window_seconds > 0.0f) {
            freq_hz = ((float)scope_zero_crossings * 0.5f) / window_seconds;
        }
    }

    uint8_t active_voices = 0;
    if (g_active_song) {
        for (uint8_t v = 0; v < g_active_song->total_voices; ++v) {
            const VoiceState& st = seq.voices[v];
            const Voice& voice = g_active_song->voices[v];
            if (voice.type == VoiceType::MELODY) {
                if (melodic_engines[st.engine_idx].getADSRState() != ADSREnvelopeState::IDLE)
                    ++active_voices;
            } else if (voice.type == VoiceType::PERCUSSION) {
                // Percussion is event-driven; the configured percussion voice
                // is useful information even between individual hits.
                ++active_voices;
            }
        }
    }

    // Compact status: PK, RMS, voices and apparent frequency.
    display.setCursor(0, 0);
    display.print("P");
    display.print((int)(peak * 100.0f));
    display.print(" R");
    display.print((int)(rms * 100.0f));
    display.print(" V");
    display.print(active_voices);
    display.setCursor(72, 0);
    if (freq_hz >= 1000.0f) {
        display.print(freq_hz * 0.001f, 1);
        display.print("kHz");
    } else if (freq_hz >= 10.0f) {
        display.print((int)freq_hz);
        display.print("Hz");
    } else {
        display.print("--Hz");
    }

    // Centre line and waveform. Quantisation to int8_t keeps the scope RAM
    // small while still giving 18 vertical pixels of useful movement.
    for (uint8_t x = 0; x < WAVE_GRAPH_SIZE - 1; ++x) {
        const int y0 = 17 - ((int)wave_capture_buf[x] * 8) / 127;
        const int y1 = 17 - ((int)wave_capture_buf[x + 1] * 8) / 127;
        const int min_y = 17 - ((int)wave_capture_max_buf[x] * 8) / 127;
        const int max_y = 17 - ((int)wave_capture_min_buf[x] * 8) / 127;
        display.drawLine(x, y0, x + 1, y1, SSD1306_WHITE);
        display.drawFastVLine(
            x, min_y, max(1, max_y - min_y + 1), SSD1306_WHITE);
    }
    for (uint8_t x = 0; x < WAVE_GRAPH_SIZE; x += 8) {
        display.drawPixel(x, 17, SSD1306_WHITE);
    }

    // Bottom 5-pixel level meter: RMS fill, with a peak marker.
    const int meter_width = 126;
    const int rms_width = constrain((int)(rms * meter_width), 0, meter_width);
    const int peak_x = constrain((int)(peak * meter_width), 0, meter_width - 1);
    display.drawRect(0, 27, 128, 5, SSD1306_WHITE);
    if (rms_width > 1) display.fillRect(1, 28, rms_width, 3, SSD1306_WHITE);
    display.drawFastVLine(1 + peak_x, 27, 5, SSD1306_WHITE);

    // A peak above 95% is an immediate visual warning. No Serial logging and
    // no extra audio-path work is required.
    if (peak >= 0.95f) {
        display.fillRect(118, 1, 9, 5, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(119, 0);
        display.print("!");
        display.setTextColor(SSD1306_WHITE);
    }

    display.display();

    // Reset metrics after the frame has been consumed. The next frame is
    // accumulated independently in loop().
    scope_peak = 0.0f;
    scope_sum_sq = 0.0f;
    scope_sample_count = 0;
    scope_zero_crossings = 0;
} 

void precalculateMidiPhases() {
    for (int n = 0; n < 128; n++) {
        const float freq = FREQ_A4 * powf(2.0f, ((float)(n - NOTE_A4) / 12.0f));
        MIDI_PHASE[n] = (uint32_t)((freq * 4294967296.0f) / SAMPLE_RATE);
    }
}

// =============================================================================
// THREAD-SAFE / ISR-SAFE CIRCULAR BUFFER FUNCTIONS
// =============================================================================
inline bool audioBufferHasSpace() {
    return (((audio_buf.head + 1) & BUFFER_MASK) != audio_buf.tail);
}

inline bool audioBufferIsEmpty() {
    return (audio_buf.head == audio_buf.tail);
}

/**
 * @brief Attempts to push a sample into the circular buffer from loop().
 * @param sample Floating-point sample processed by the DSP.
 * @return true if successfully inserted, false if the buffer is full.
 */
bool audio_buffer_push(float sample) {
    const uint16_t next = (audio_buf.head + 1) & BUFFER_MASK;
    if (next == audio_buf.tail) {
        return false;
    }
    AudioSample &slot = audio_buf.buffer[audio_buf.head];
    slot.pwmDuty = sampleToRegisterLUT(sample);
    slot.dacVal = sampleToDac(sample);
    audio_buf.head = next;
    return true;
}

/**
 * @brief Consumes a sample from the buffer inside the ISR.
 * @param sample Reference where the read sample will be stored.
 * @return true if a sample was extracted, false if the buffer is empty.
 */
bool audio_buffer_pop(AudioSample &sample) {
    // NOTE: Since this runs INSIDE the cb_audio() ISR, equal or lower priority 
    // interrupts are already disabled by hardware.
    if (audio_buf.head == audio_buf.tail) {
#if ENABLE_AUDIO_LOGS
        recordAudioUnderrun();
#endif
        return false; // Buffer empty (Underflow)
    }
#if ENABLE_AUDIO_LOGS
    clearAudioUnderrunState();
#endif
    sample = audio_buf.buffer[audio_buf.tail];
    audio_buf.tail = (audio_buf.tail + 1) & BUFFER_MASK;
    return true;
}

// =============================================================================
// SEQUENCER ENGINE AND STEP MANAGEMENT
// =============================================================================
/**
 * @brief Calculates the song duration from the longest voice sequence.
 *
 * Each voice may have a different number of steps and rhythmic values. The song
 * ends at the latest sequence end time, rather than when a particular voice
 * reaches its last step.
 */
uint32_t calculateSongDurationSamples(const Song& song) {
    uint64_t longest_duration = 0;

    for (uint8_t v = 0; v < song.total_voices; ++v) {
        const Voice& voice = song.voices[v];
        uint64_t voice_duration = 0;

        for (uint16_t step = 0; step < voice.total_steps; ++step) {
            const float dur_sec =
                (60.0f / (float)song.bpm) * (4.0f / (float)voice.rhythm[step]);
            voice_duration += (uint32_t)(dur_sec * SAMPLE_RATE);
        }

        if (voice_duration > longest_duration) {
            longest_duration = voice_duration;
        }
    }

    if (longest_duration > 0xFFFFFFFFULL) {
        return 0xFFFFFFFFUL;
    }

    return (uint32_t)longest_duration;
}

/**
 * @brief Loads and triggers a new step within the sequence for a specific voice.
 */
void load_step(uint8_t voice_idx) {
    // Reads whichever Song is currently active -- a PLAYER[] song or a
    // DEMO_SONGS[] entry, both driven identically via g_active_song.
    const Song& song = *g_active_song;
    const Voice& voice = song.voices[voice_idx];
    VoiceState& voice_state = seq.voices[voice_idx];

    // Individual safety loop per voice (supports asynchronous durations)
    if (voice_state.step_idx >= voice.total_steps) {
        voice_state.step_idx = 0;
    }

    // Calculation of step duration in samples based on global BPM and rhythmic figure (4=quarter, 8=eighth, 16=sixteenth...)
    const float dur_sec = (60.0f / (float)song.bpm) * (4.0f / (float)voice.rhythm[voice_state.step_idx]);
    voice_state.samples_total_duration = (uint32_t)(dur_sec * SAMPLE_RATE);
    voice_state.samples_gate_on_duration = (uint32_t)(voice_state.samples_total_duration * 0.85f);
    voice_state.step_time_counter = 0;

    if (voice.type == VoiceType::MELODY) {
        const int note_val =
            constrain(voice.sequence[voice_state.step_idx] + song.tonic, 0, 127);

        melodic_engines[voice_state.engine_idx].setBasePhaseStep(MIDI_PHASE[note_val]);
        melodic_engines[voice_state.engine_idx].trigger(voice.instrument);

    } else {
        const PercussionType tp = (PercussionType)voice.sequence[voice_state.step_idx];
        if (tp != PercussionType::NONE) {
            percussion_engines[voice_state.engine_idx].trigger(tp);
        }
    }
}

/**
 * @brief Switches the song in the sequencer and assigns the synthesizer pool.
 * @param idx Index into whichever song list is currently active
 *            (g_current_song_list -- PLAYER[] or DEMO_SONGS[]).
 */
void change_song(int idx) {
    seq.current_song_idx = idx % g_current_song_count;
    g_active_song = &g_current_song_list[seq.current_song_idx];
    const Song& song = *g_active_song;

    uint8_t i_mel = 0;
    uint8_t i_perc = 0;

    for (uint8_t v = 0; v < song.total_voices; v++) {
        seq.voices[v] = VoiceState{};
    
        if (song.voices[v].type == VoiceType::MELODY) {
            if (i_mel >= MAX_MELODIC_VOICES) continue; // drop voice, don't corrupt memory
            seq.voices[v].engine_idx = i_mel;
            melodic_engines[i_mel].resetFilter();
            melodic_engines[i_mel].resetADSR();
            melodic_engines[i_mel].resetPhase();
            i_mel++;
        } else {
            if (i_perc >= MAX_PERCUSSION_VOICES) continue;
            seq.voices[v].engine_idx = i_perc;
            percussion_engines[i_perc].trigger(PercussionType::NONE);
            i_perc++;
        }
    
        load_step(v);
    }
}

/**
 * @brief DSP task executed from loop() to process the current multi-voice sample.
 */
 
 /**
 * @brief Soft-knee limiter used instead of a hard constrain() at the mix stage.
 *
 * BUGFIX / IMPROVEMENT: the previous code used constrain(x, -1.0f, 1.0f), a hard
 * clip. Any sample that exceeded +/-1.0 (e.g. after summing 3 melodic voices plus
 * percussion, or after delay feedback build-up) was truncated abruptly, which
 * generates strong odd-harmonic distortion and is a common cause of "gritty" or
 * "crunchy" DAC output. Below the knee threshold the signal passes through
 * untouched (bit-exact linear region); only samples that would have clipped are
 * gently saturated with a tanh curve, which sounds far less harsh.
 */
inline float softClip(float x) {
    const float knee = 0.7f;
    if (x > knee) {
        return knee + (1.0f - knee) * tanhf((x - knee) / (1.0f - knee));
    }
    if (x < -knee) {
        return -knee - (1.0f - knee) * tanhf((-x - knee) / (1.0f - knee));
    }
    return x;
}

/**
 * @brief DSP task executed from loop() to process the current multi-voice sample.
 */
// =============================================================================
// OUTPUT VOLUME CONTROL
// =============================================================================
// Global DAC output level. 1.0f = 100%, 0.8f = 80%, 0.0f = mute.
static float g_output_volume = 1.0f;

void setOutputVolumePercent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    g_output_volume = percent * 0.01f;
}

float getOutputVolumePercent() {
    return g_output_volume * 100.0f;
}

// =============================================================================
// MELODIC / PERCUSSION BUS BALANCE
// =============================================================================
// Independent relative-level controls for the melodic bus vs. the percussion
// bus, applied to melodic_accum/percussion_accum BEFORE the combined
// sqrt(total_active) normalization in compute_next_dsp_sample(). This keeps
// the headroom math from the earlier mixing bugfix intact -- these are pure
// relative-balance knobs, they don't change how many "voices worth" of
// headroom budget the mixer thinks it has.
// 1.0f = unchanged (100%), 0.5f = half as loud relative to the other bus, etc.
static float g_melodic_level = 1.0f;
static float g_percussion_level = 1.0f;

void setMelodicLevelPercent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    g_melodic_level = percent * 0.01f;
}

float getMelodicLevelPercent() {
    return g_melodic_level * 100.0f;
}

void setPercussionLevelPercent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    g_percussion_level = percent * 0.01f;
}

float getPercussionLevelPercent() {
    return g_percussion_level * 100.0f;
}

// =============================================================================
// LOGARITHMIC (AUDIO-TAPER) VOLUME MAPPING
// =============================================================================
// Human hearing perceives loudness roughly logarithmically -- an equal-dB
// change sounds like an equal loudness step regardless of the starting
// level -- rather than linearly. That's why real amplifiers use a
// "logarithmic"/"audio taper" potentiometer instead of a linear one: equal
// physical rotation gives equal dB (and so equal perceived loudness) steps.
// A linear taper crammed into the same 0..100% gain range instead barely
// changes the perceived loudness across most of its rotation and then
// rushes through most of the audible range in the last few steps near zero.
//
// volumeLevelToGainPercent() reproduces an audio-taper pot in software:
// level 0 is true silence (hard mute, matching a pot's end-stop), and
// levels 1..max_level are spaced evenly IN DECIBELS between
// VOLUME_TAPER_MIN_DB (the quietest non-mute step) and 0 dB / unity gain
// at the top step. Equal encoder clicks then correspond to equal
// perceived loudness changes across the whole range, instead of the
// previous straight "level * (100/max_level)" linear-in-gain mapping.
const float VOLUME_TAPER_MIN_DB = -45.0f; // dB at level 1 (quietest non-mute step)

float volumeLevelToGainPercent(uint8_t level, uint8_t max_level) {
    if (level == 0 || max_level == 0) {
        return 0.0f; // Hard mute -- true silence, like a pot's end-stop.
    }
    if (level >= max_level) {
        return 100.0f; // Top step is exactly unity gain (0 dB).
    }

    // Interpolate linearly in dB from VOLUME_TAPER_MIN_DB (level 1) up to
    // 0 dB (level == max_level), then convert back to the linear gain
    // percentage the existing 0-100% DSP gain API expects.
    const float level_fraction = (float)level / (float)max_level; // (0, 1)
    const float db = VOLUME_TAPER_MIN_DB * (1.0f - level_fraction);
    const float gain = powf(10.0f, db / 20.0f);
    return gain * 100.0f;
}



inline bool isDacEnabledForMode(OutputMode mode) {
    return mode == OutputMode::DAC_ONLY || mode == OutputMode::DAC_AND_RF;
}

inline bool isRfEnabledForMode(OutputMode mode) {
    return mode == OutputMode::RF_ONLY || mode == OutputMode::DAC_AND_RF;
}

const char* outputModeToString(OutputMode mode) {
    switch (mode) {
        case OutputMode::DAC_ONLY:   return "AUDIO ONLY";
        case OutputMode::RF_ONLY:    return "RF ONLY";
        case OutputMode::DAC_AND_RF: return "AUDIO + RF";
        case OutputMode::MUTE:       return "MUTE";
        default:                     return "?";
    }
}

const __FlashStringHelper* outputModeToShortString(OutputMode mode) {
    switch (mode) {
        case OutputMode::DAC_ONLY:   return F("AUDIO");
        case OutputMode::RF_ONLY:    return F("RF");
        case OutputMode::DAC_AND_RF: return F("A+RF");
        case OutputMode::MUTE:       return F("MUTE");
        default:                     return F("?");
    }
}

// Items navigable in the Settings menu, in encoder-rotation order.
enum class SettingItem : uint8_t {
    MELODY_VOLUME = 0,
    PERCUSSION_VOLUME,
    MASTER_VOLUME,
    AM_FREQUENCY,
    OUTPUT_SELECTION,
    SETTINGS_ITEM_COUNT   // sentinel: total number of navigable items
};

SettingItem g_settings_selected_item = SettingItem::MASTER_VOLUME;
bool g_settings_editing = false;   // false = browsing items, true = editing the selected one
unsigned long g_settings_last_activity_ms = 0;
const unsigned long SETTINGS_INACTIVITY_TIMEOUT_MS = 5000;

// --- Encoder-resolution volume levels (0..20 steps)
// mapped onto the existing 0-100% controls above through a
// logarithmic (audio-taper) curve -- see volumeLevelToGainPercent() 
// so each encoder step sounds like an equal loudness change to the ear.
const uint8_t VOLUME_LEVEL_MAX = 20;
const uint8_t VOLUME_LEVEL_MELODY_DEFAULT     = 20;
const uint8_t VOLUME_LEVEL_PERCUSSION_DEFAULT = 20;
const uint8_t VOLUME_LEVEL_MASTER_DEFAULT     = 20;

uint8_t g_melody_volume_level     = VOLUME_LEVEL_MELODY_DEFAULT;
uint8_t g_percussion_volume_level = VOLUME_LEVEL_PERCUSSION_DEFAULT;
uint8_t g_master_volume_level     = VOLUME_LEVEL_MASTER_DEFAULT;

void setMelodyVolumeLevel(int level) {
    if (level < 0) level = 0;
    if (level > VOLUME_LEVEL_MAX) level = VOLUME_LEVEL_MAX;
    g_melody_volume_level = (uint8_t)level;
    setMelodicLevelPercent(volumeLevelToGainPercent(g_melody_volume_level, VOLUME_LEVEL_MAX));
}

void setPercussionVolumeLevel(int level) {
    if (level < 0) level = 0;
    if (level > VOLUME_LEVEL_MAX) level = VOLUME_LEVEL_MAX;
    g_percussion_volume_level = (uint8_t)level;
    setPercussionLevelPercent(volumeLevelToGainPercent(g_percussion_volume_level, VOLUME_LEVEL_MAX));
}

void setMasterVolumeLevel(int level) {
    if (level < 0) level = 0;
    if (level > VOLUME_LEVEL_MAX) level = VOLUME_LEVEL_MAX;
    g_master_volume_level = (uint8_t)level;
    setOutputVolumePercent(volumeLevelToGainPercent(g_master_volume_level, VOLUME_LEVEL_MAX));
}

// --- AM carrier frequency, adjustable in the Settings menu. AM_STEP_KHZ
// is a fixed tuning step (not a strict broadcast channel grid); 9 kHz
// matches the MW channel spacing used across Europe/ITU Region 1, but can be
// changed here if a different region's grid (e.g. 10 kHz) is preferred.
const uint16_t AM_FREQ_MIN_KHZ = 531;
const uint16_t AM_FREQ_MAX_KHZ = 1602;
const uint16_t AM_FREQ_DEFAULT_KHZ = 594;
const uint16_t AM_STEP_KHZ = 9;

uint16_t g_am_frequency_khz = AM_FREQ_DEFAULT_KHZ;
OutputMode g_output_mode = OutputMode::DAC_AND_RF;

// Gates the ISR's DAC write independently of the RF carrier's own
// suspend/resume, so "RF only" and "Mute" can silence audio monitoring
// without touching the audio timer or the circular buffer consumer.
volatile bool g_dac_output_enabled = true;

static const float ACTIVE_VOICE_NORMALIZATION[5] = {
    0.0f,
    1.0f,
    0.7071067812f,
    0.5773502692f,
    0.5000000000f
};

float compute_next_dsp_sample() {
    const Song& song = *g_active_song;
    float melodic_accum = 0.0f;
    float percussion_accum = 0.0f;
    uint8_t melodic_count = 0;
    uint8_t percussion_count = 0;

    for (uint8_t v = 0; v < song.total_voices; v++) {
        const Voice& voice = song.voices[v];
        VoiceState& state = seq.voices[v];

        state.step_time_counter++;
        bool advance_step = false;

        if (voice.type == VoiceType::MELODY) {
            // only count voices that are actually sounding (not IDLE) for the
            // normalization below.
            if (melodic_engines[state.engine_idx].getADSRState() != ADSREnvelopeState::IDLE) {
                melodic_count++;
            }

            // ADSR Envelope: Release control on Gate-OFF
            if ((state.step_time_counter >= state.samples_gate_on_duration) &&
                (melodic_engines[state.engine_idx].getADSRState() != ADSREnvelopeState::RELEASE)) {
                melodic_engines[state.engine_idx].release();
            } else if (state.step_time_counter >= state.samples_total_duration) {
                advance_step = true;
            }

            melodic_accum += melodic_engines[state.engine_idx].process(voice.instrument, state.step_time_counter);
        } 
        else { // PERCUSSION
            // Only count voices that are actually sounding (not NONE), mirroring
            // the melodic_count logic above, so the mix normalization reflects
            // how many voices are truly contributing energy right now.
            if (percussion_engines[state.engine_idx].isActive()) {
                percussion_count++;
            }

            if (state.step_time_counter >= state.samples_total_duration) {
                advance_step = true;
            }

            percussion_accum += percussion_engines[state.engine_idx].process();
        }

        if (advance_step) {
            state.step_idx++;

            // Do not trigger a new step after the global song duration has ended.
            const bool is_last_song_sample =
                (seq.elapsed_samples + 1 >= seq.song_duration_samples);

            if (!is_last_song_sample) {
                load_step(v);
            }
        }
    }

    // Adaptive mixing: normalizes the COMBINED signal according to the total
    // number of active voices (melodic + percussion), not just melodic ones.
    //
    // g_melodic_level / g_percussion_level scale each bus before the combined
    // normalization, so they act as a pure relative-balance control (e.g. drop
    // the percussion to 50% under the melody) without touching the headroom
    // budget that sqrt(total_active) is protecting.
    const uint8_t total_active = melodic_count + percussion_count;
    const uint8_t normalization_index = (total_active > 4) ? 4 : total_active;
    const float combined_mix = (total_active > 0)
        ? ((melodic_accum * g_melodic_level) + (percussion_accum * g_percussion_level)) *
          ACTIVE_VOICE_NORMALIZATION[normalization_index]
        : 0.0f;
  
#if ENABLE_AUDIO_LOGS
    if (fabsf(combined_mix) > 1.0f) {
        recordClippingEvent(combined_mix, ClippingReason::MIX_OVERLOAD, seq.voices[0].step_idx);
        prevented_overload_count++;
    }
#endif

    const float clipped_mix = softClip(combined_mix);
    const float delayed_mix = delay_fx.process(clipped_mix);
  
#if ENABLE_AUDIO_LOGS
    if (fabsf(delayed_mix) > 1.0f) {
        recordClippingEvent(delayed_mix, ClippingReason::DELAY_OVERLOAD, seq.voices[0].step_idx);
        prevented_overload_count++;
    }
#endif
  
    const float output_sample = g_output_volume * softClip(delayed_mix);
  
#if ENABLE_AUDIO_LOGS
    if (fabsf(output_sample) > 1.0f) {
        recordClippingEvent(output_sample, ClippingReason::OUTPUT_OVERLOAD, seq.voices[0].step_idx);
    }

    updateAudioArtifactDiagnostics(output_sample);
    audio_sample_counter++;
    updateAudioBufferDiagnostics();
#endif

    // The song ends at the latest step boundary among all voices.
    seq.elapsed_samples++;
    if (seq.elapsed_samples >= seq.song_duration_samples) {
        seq.song_finished = true;
    }

    return output_sample;

}



// =============================================================================
// ISR AND TIMER MANAGEMENT
// =============================================================================

void cb_audio(timer_callback_args_t __attribute__((unused)) *p_args) {
#if ENABLE_AUDIO_LOGS
    isr_tick_count++;
#endif
    AudioSample sample;
    if (audio_buffer_pop(sample)) {
        // RF (modulated AM carrier) — GTCCRB of GPT7 channel.
        // Always written: whether it actually reaches the antenna is
        // controlled separately by suspending/resuming the carrier PWM
        // peripheral itself (see startRFCarrier()/stopRFCarrier())
        *g_duty_reg = sample.pwmDuty;
        // DAC output
        // Monitor — DAC12 channel 0 (A0), direct 16-bit write.
        // When DAC output is disabled (RF-only or Mute output modes), hold
        // the DAC at its silent midpoint instead of the live sample.
        R_DAC->DADR[0] = g_dac_output_enabled ? sample.dacVal : 2048;
    }
}

void configurePWMRegister() {
    FspTimer *timer_ptr = carrier_pwm.get_timer();
    uint8_t   channel   = timer_ptr->get_channel();

    uint32_t base = (uint32_t)R_GPT0 +
                     channel * ((uint32_t)R_GPT1 - (uint32_t)R_GPT0);
    R_GPT0_Type *gpt_reg_base = (R_GPT0_Type *) base;

    if (gpt_reg_base->GTIOR_b.OAE) {
        g_duty_reg = &gpt_reg_base->GTCCR[0];
    } else {
        g_duty_reg = &gpt_reg_base->GTCCR[1];
    }

    g_period_counts = gpt_reg_base->GTPR; 

    // buffer mode OFF on both channels
    gpt_reg_base->GTBER_b.CCRA = 0b00;
    gpt_reg_base->GTBER_b.CCRB = 0b00;
}

/**
 * @brief Applies an output-routing selection: which of DAC/RF are live.
 *        Safe to call at any time (menu, settings, or mid-playback).
 */
void applyOutputMode(OutputMode mode) {
    g_output_mode = mode;
    g_dac_output_enabled = isDacEnabledForMode(mode);
    if (isRfEnabledForMode(mode)) {
        startRFCarrier();
    } else {
        stopRFCarrier();
    }
}

/**
 * @brief Changes the AM carrier frequency at runtime and recalculates
 *        everything that depends on the PWM period: the duty-cycle register
 *        pointer/period (configurePWMRegister) and the arcsine predistortion
 *        LUT (initializePredistortionLUT), which linearizes the AM
 *        modulation and is only valid for the period it was built from.
 * @param freq_khz Desired frequency in kHz; clamped to
 *        [AM_FREQ_MIN_KHZ, AM_FREQ_MAX_KHZ].
 */
void setAMFrequencyKHz(uint16_t freq_khz) {
    if (freq_khz < AM_FREQ_MIN_KHZ) freq_khz = AM_FREQ_MIN_KHZ;
    if (freq_khz > AM_FREQ_MAX_KHZ) freq_khz = AM_FREQ_MAX_KHZ;
    g_am_frequency_khz = freq_khz;

    carrier_pwm.suspend();
    carrier_pwm.begin((float)g_am_frequency_khz * 1000.0f, 50.0f);
    configurePWMRegister();
    initializePredistortionLUT();

    // carrier_pwm.begin() leaves the peripheral running; restore the
    // run/stop state dictated by the currently selected output mode.
    if (!isRfEnabledForMode(g_output_mode)) {
        stopRFCarrier();
    }
}


// =============================================================================
// OLED DISPLAY RENDERING FUNCTIONS
// =============================================================================

// Maximum characters that fit on one 128 px-wide line at text size 1
// (6 px per glyph, matching the Adafruit_GFX default font: 5 px char + 1 px
// spacing). Used to keep every title/label to a single display line.
const uint8_t DISPLAY_MAX_CHARS_PER_LINE = 21;

/**
 * @brief Copies `title` into `out`, truncating with a trailing "..." if it
 *        would exceed `max_chars`, so song/demo names always render on a
 *        single display line instead of wrapping or running off-screen.
 * @param title     Null-terminated source string (e.g. Song::title).
 * @param out       Destination buffer.
 * @param out_size  Size of `out` in bytes (including the null terminator).
 * @param max_chars Maximum visible characters allowed (<= out_size - 1).
 */
void truncateTitleForDisplay(const char* title, char* out, size_t out_size, uint8_t max_chars) {
    if (out == nullptr || out_size == 0) return;
    if (title == nullptr) { out[0] = '\0'; return; }

    const size_t hard_limit = out_size - 1;
    const uint8_t limit = (max_chars <= hard_limit) ? max_chars : (uint8_t)hard_limit;

    const size_t len = strlen(title);
    if (len <= limit) {
        memcpy(out, title, len);
        out[len] = '\0';
        return;
    }

    // Reserve room for a trailing "..." ellipsis so truncation is visible
    // rather than silently cutting a word in half.
    const uint8_t ellipsis_len = 3;
    const uint8_t keep = (limit > ellipsis_len) ? (uint8_t)(limit - ellipsis_len) : 0;
    memcpy(out, title, keep);
    memcpy(out + keep, "...", ellipsis_len);
    out[keep + ellipsis_len] = '\0';
}

// Draw a compact music-note glyph in the top-left corner of the 128x32 display.
void drawSongIcon(int16_t x, int16_t y) {
    display.drawFastVLine(x + 4, y + 2, 6, SSD1306_WHITE);
    display.drawFastHLine(x + 4, y + 2, 3, SSD1306_WHITE);
    display.fillCircle(x + 2, y + 7, 1, SSD1306_WHITE);
}

// Draw a compact "D" glyph in the same top-left slot as drawSongIcon(), used
// instead of the music note whenever the Demo song list is active
// (g_browsing_demo_songs), so it's visible at a glance which list is
// currently loaded -- same spot, same size, no other layout change needed.
void drawDemoIcon(int16_t x, int16_t y) {
    display.setCursor(x, y);
    display.print('D');
}

// Draw a compact play triangle used by the song-selection header.
void drawPlayIcon(int16_t x, int16_t y) {
    display.fillTriangle(x, y, x, y + 8, x + 7, y + 4, SSD1306_WHITE);
}

// Draw a small ON/OFF status box. The front-panel label identifies the field.
void drawStatusBox(int16_t x, int16_t y, bool enabled) {
    // the small rect is to close to labels
    // display.drawRect(x, y, 18, 9, SSD1306_WHITE);
    display.setCursor(x + 3, y + 1);
    display.print(enabled ? F("ON") : F("OFF"));
}

// Draw the volume graphic bar used on the main song-selection screen.
// Ten compact segments give the OLED the same visual language as the edit bar.
void drawMainVolumeBar(uint8_t value) {
    const int16_t x = 40;
    const int16_t y = 18;
    const int16_t segment_width = 4;
    const int16_t segment_gap = 1;
    const int16_t segment_count = 10;

    const uint8_t clamped = (value > VOLUME_LEVEL_MAX) ? VOLUME_LEVEL_MAX : value;
    const uint8_t full_segments = clamped / 2;
    const bool half_segment = (clamped % 2) != 0;

    for (int16_t i = 0; i < segment_count; ++i) {
        const int16_t segment_x = x + i * (segment_width + segment_gap);
        display.drawRect(segment_x, y, segment_width, 6, SSD1306_WHITE);

        if (i < full_segments) {
            display.fillRect(segment_x + 1, y + 1, segment_width - 2, 4, SSD1306_WHITE);
        } else if (i == full_segments && half_segment) {
            display.fillRect(segment_x + 1, y + 1, 1, 4, SSD1306_WHITE);
        }
    }

    // The marker is drawn above the segment that contains the current value.
    int16_t marker_index = (clamped >= VOLUME_LEVEL_MAX)
        ? (segment_count - 1)
        : (clamped / 2);
    const int16_t marker_x = x + marker_index * (segment_width + segment_gap) + 1;
    display.drawFastVLine(marker_x, y - 2, 10, SSD1306_WHITE);
}

/**
 * @brief Renders the main song-selection screen using the Option 4 layout.
 *
 * Layout is deliberately designed for a physical 128x32 OLED:
 *   Top area    = song icon, index, title and play indicator.
 *   Bottom area = RF status, AUDIO status, VOLUME graphic/value and FR value.
 *
 * The front panel is assumed to be screen-printed with:
 *   RF | AUDIO | VOL | FR
 * so those labels do not consume OLED pixels.
 */
void renderMenuScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    const Song& current_song_ref = g_current_song_list[selected_song_index];

    // --- Top: song selection -------------------------------------------------
    if (g_browsing_demo_songs) {
        drawDemoIcon(0, 0);
    } else {
        drawSongIcon(0, 0);
    }

    display.setCursor(11, 0);
    if (selected_song_index + 1 < 10) display.print('0');
    display.print(selected_song_index + 1);

    char truncated_title[15];
    truncateTitleForDisplay(current_song_ref.title, truncated_title,
                             sizeof(truncated_title), 14);
    display.setCursor(25, 0);
    display.print(truncated_title);

    drawPlayIcon(120, 0);

    // Separator between the song area and the four physical front-panel fields.
    display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

    // --- Bottom: RF | AUDIO | VOL | FR --------------------------------------
    drawStatusBox(0, 18, isRfEnabledForMode(g_output_mode));
    drawStatusBox(20, 18, isDacEnabledForMode(g_output_mode));

    drawMainVolumeBar(g_master_volume_level);
    display.setCursor(88, 18);
    display.print(g_master_volume_level);

    display.setCursor(103, 18);
    display.print(g_am_frequency_khz);

    display.display();
}

/**
 * @brief Renders the active playback screen (BPM and song title).
 *        Shared by normal playback and Demo Mode -- see g_current_song_list.
 */
void renderPlayingScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    const Song& current_song_ref = g_current_song_list[seq.current_song_idx];
    display.print(g_browsing_demo_songs ? F("DEMO ") : F("PLAYING "));
    display.print(current_song_ref.bpm);
    display.print(F(" BPM"));

    display.setCursor(0, 16);
    char truncated_title[DISPLAY_MAX_CHARS_PER_LINE + 1];
    truncateTitleForDisplay(current_song_ref.title, truncated_title,
                             sizeof(truncated_title), DISPLAY_MAX_CHARS_PER_LINE);
    display.println(truncated_title);

    display.display();
}

// Draw a horizontal min-to-max bar with a movable current-value marker.
void drawSettingBar(int value, int min_value, int max_value) {
    const int16_t x0 = 8;
    const int16_t x1 = SCREEN_WIDTH - 9;
    const int16_t y = 14;
    const int16_t width = x1 - x0;

    if (max_value <= min_value) return;

    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    const int16_t marker_x = x0 + (int32_t)(value - min_value) * width / (max_value - min_value);

    display.drawRect(x0, y, width + 1, 5, SSD1306_WHITE);
    if (marker_x > x0) {
        display.fillRect(x0 + 1, y + 1, marker_x - x0, 3, SSD1306_WHITE);
    }

    // Triangle marker is intentionally above the track so the current value
    // remains visible at both hard limits.
    display.fillTriangle(marker_x - 3, y - 4, marker_x + 3, y - 4,
                         marker_x, y - 1, SSD1306_WHITE);
}

// Draw the four-state output selector used by the Option 4 edit screen.
void drawOutputBar() {
    const int16_t x0 = 7;
    const int16_t y = 12;
    const int16_t total_width = 114;
    const int16_t segment_width = total_width / OUTPUT_MODE_COUNT;
    const int active = (int)g_output_mode;

    for (int i = 0; i < OUTPUT_MODE_COUNT; ++i) {
        const int16_t x = x0 + i * segment_width;
        if (i == active) {
            display.fillRect(x, y, segment_width, 9, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.drawRect(x, y, segment_width, 9, SSD1306_WHITE);
            display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(x + 7, y + 1);
        display.print(outputModeToShortString(static_cast<OutputMode>(i)));
    }
    display.setTextColor(SSD1306_WHITE);
}

// Return a short English label suitable for the 128x32 setting header.
const __FlashStringHelper* settingTitle() {
    switch (g_settings_selected_item) {
        case SettingItem::MELODY_VOLUME:     return F("MELODY VOL");
        case SettingItem::PERCUSSION_VOLUME: return F("PERC VOL");
        case SettingItem::MASTER_VOLUME:     return F("MASTER VOL");
        case SettingItem::AM_FREQUENCY:      return F("AM FREQUENCY");
        case SettingItem::OUTPUT_SELECTION:  return F("OUTPUT");
        default:                             return F("SETTINGS");
    }
}

/**
 * @brief Renders the Option 4 graphical settings editor.
 *
 * Numeric settings use a full-width min-to-max bar. Output routing uses a
 * four-segment selector. The lower line always shows the exact current value.
 */
void renderSettingsScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Header: the selected setting, with edit mode shown by an inverted block.
    if (g_settings_editing) {
        display.fillRect(0, 0, 7, 8, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(1, 0);
        display.print('E');
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(9, 0);
    } else {
        display.setCursor(0, 0);
        display.print('>');
        display.setCursor(9, 0);
    }
    display.print(settingTitle());

    display.drawFastHLine(0, 8, SCREEN_WIDTH, SSD1306_WHITE);

    switch (g_settings_selected_item) {
        case SettingItem::MELODY_VOLUME:
            drawSettingBar(g_melody_volume_level, 0, VOLUME_LEVEL_MAX);
            display.setCursor(0, 22);
            display.print(F("0"));
            display.setCursor(117, 22);
            display.print(VOLUME_LEVEL_MAX);
            display.setCursor(48, 24);
            display.print(g_melody_volume_level);
            display.print(F(" / 20"));
            break;

        case SettingItem::PERCUSSION_VOLUME:
            drawSettingBar(g_percussion_volume_level, 0, VOLUME_LEVEL_MAX);
            display.setCursor(0, 22);
            display.print(F("0"));
            display.setCursor(117, 22);
            display.print(VOLUME_LEVEL_MAX);
            display.setCursor(48, 24);
            display.print(g_percussion_volume_level);
            display.print(F(" / 20"));
            break;

        case SettingItem::MASTER_VOLUME:
            drawSettingBar(g_master_volume_level, 0, VOLUME_LEVEL_MAX);
            display.setCursor(0, 22);
            display.print(F("0"));
            display.setCursor(117, 22);
            display.print(VOLUME_LEVEL_MAX);
            display.setCursor(48, 24);
            display.print(g_master_volume_level);
            display.print(F(" / 20"));
            break;

        case SettingItem::AM_FREQUENCY:
            drawSettingBar(g_am_frequency_khz, AM_FREQ_MIN_KHZ, AM_FREQ_MAX_KHZ);
            display.setCursor(0, 22);
            display.print(AM_FREQ_MIN_KHZ);
            display.print(F(" kHz"));
            display.setCursor(84, 22);
            display.print(AM_FREQ_MAX_KHZ);
            display.print(F(" kHz"));
            display.setCursor(43, 24);
            display.print(g_am_frequency_khz);
            display.print(F(" kHz"));
            break;

        case SettingItem::OUTPUT_SELECTION:
            drawOutputBar();
            display.setCursor(42, 24);
            display.print((int)g_output_mode + 1);
            display.print(outputModeToString(g_output_mode));
            break;

        default:
            break;
    }

    display.display();
}

// =============================================================================
// PLAYBACK CONTROL FUNCTIONS
// =============================================================================

/**
 * @brief Stops playback immediately and leaves the audio path in a silent state.
 *
 * The ring buffer is cleared inside a short interrupt-disabled critical section
 * so no queued samples can continue after the stop request. All synthesis and
 * delay state is reset so stale tails cannot leak into the next playback.
 */
void stopSongPlayback() {
    seq.playing = false;
    seq.song_finished = false;
    seq.elapsed_samples = 0;
    seq.song_duration_samples = 0;

    scope_zoom_level = 0;
    scope_fullscreen = false;
    scope_display_dirty = false;
    updateScopeCaptureStride();
    resetScopeCaptureFrame();

    // Silence both physical outputs before discarding queued audio.
    g_dac_output_enabled = false;
  
    noInterrupts();
    audio_buf.head = 0;
    audio_buf.tail = 0;
    interrupts();

    for (uint8_t i = 0; i < MAX_MELODIC_VOICES; ++i) {
        melodic_engines[i].resetADSR();
        melodic_engines[i].resetFilter();
        melodic_engines[i].resetPhase();
    }

    for (uint8_t i = 0; i < MAX_PERCUSSION_VOICES; ++i) {
        percussion_engines[i].trigger(PercussionType::NONE);
    }

    // Recreate the delay effect so no buffered echo survives the stop.
    delay_fx = DelayEffect();

    for (uint8_t i = 0; i < MAX_VOICES; ++i) {
        seq.voices[i] = VoiceState{};
    }

    // Prevent the release of the same stop press from triggering a menu action.
    previous_button_state = LOW;
    button_press_start_time = millis();
    button_long_press_fired = true;
    button_settings_press_fired = true;
    last_button_activity_ms = button_press_start_time;

    current_ui_state = UI_MENU_SELECTION;
    renderMenuScreen();
}

/**
 * @brief Polls the encoder push button while playback is active.
 *
 * Any newly detected press is treated as an emergency stop. Normal menu and
 * Settings button behavior remains unchanged because this handler is only
 * called while the sequencer is playing.
 */
void updatePlaybackStopButton() {
    if (!seq.playing) return;

    const bool button_pressed = (digitalRead(PIN_ENC_SW) == LOW);

    if (!button_pressed) {
        playback_stop_button_latched = false;
        return;
    }

    if (playback_stop_button_latched) return;

    playback_stop_button_latched = true;
    stopSongPlayback();
}

/**
 * @brief Starts playback of a selected song and updates the display before audio starts.
 * @param song_index Index of the song to play from whichever list is active
 *        (g_current_song_list -- PLAYER[] or DEMO_SONGS[]).
 */
void startSongPlayback(int song_index) {
    seq.current_song_idx = song_index % g_current_song_count;

    // Reset voice states for a clean sequence start
    for (int i = 0; i < MAX_VOICES; i++) {
        seq.voices[i].step_idx = 0;
        seq.voices[i].step_time_counter = 0;
    }

    seq.playing = true;
    seq.song_finished = false;
    seq.elapsed_samples = 0;

    scope_zoom_level = 0;
    scope_fullscreen = false;
    scope_display_dirty = false;
    updateScopeCaptureStride();
    resetScopeCaptureFrame();

    change_song(seq.current_song_idx);
    seq.song_duration_samples = calculateSongDurationSamples(*g_active_song);

    // Restore the output routing that was selected before the stop.
    applyOutputMode(g_output_mode);

    playback_stop_button_latched = false;

#if ENABLE_AUDIO_LOGS
    resetAudioDiagnostics();
#endif

    current_ui_state = UI_PLAYING;

    // Render playing screen once immediately before audio DSP loop takes over CPU
    renderPlayingScreen();
}

/**
 * @brief Toggles the active song list between the normal PLAYER[] songs and
 *        the DEMO_SONGS[] feature showcase (see demo.h). This is the whole
 *        of "Demo Mode" -- there is no separate UI state or playback path;
 *        selecting and playing a demo goes through the exact same
 *        UI_MENU_SELECTION / UI_PLAYING code as a normal song. Triggered by
 *        the encoder button's shorter long-press tier (LONG_PRESS_MS); the
 *        longer tier (SETTINGS_PRESS_MS) still opens the Settings menu.
 */
void toggleSongList() {
    g_browsing_demo_songs = !g_browsing_demo_songs;
    if (g_browsing_demo_songs) {
        g_current_song_list = DEMO_SONGS;
        g_current_song_count = TOTAL_DEMO_SONGS;
    } else {
        g_current_song_list = PLAYER;
        g_current_song_count = TOTAL_SONGS;
    }
    selected_song_index = 0;
    renderMenuScreen();
}

/**
 * @brief Enters the Settings menu from the main song-selection menu.
 *        Always starts on the first item, in navigation (not edit) mode.
 */
void enterSettingsMode() {
    current_ui_state = UI_SETTINGS;
    g_settings_selected_item = SettingItem::MELODY_VOLUME;
    g_settings_editing = false;
    g_settings_last_activity_ms = millis();
    renderSettingsScreen();
}

/**
 * @brief Leaves the Settings menu and returns to the song-selection menu.
 *        Any in-progress edit is simply left as-is (values are applied live
 *        as they're adjusted, so there is nothing to roll back or commit).
 */
void exitSettingsMode() {
    g_settings_editing = false;
    current_ui_state = UI_MENU_SELECTION;
    renderMenuScreen();
}

// =============================================================================
// USER INTERFACE INPUT HANDLERS (ACTIVE ONLY OUTSIDE NORMAL PLAYBACK)
// =============================================================================

// Return to the song-selection screen when the Settings UI has been idle.
// This keeps the main screen as the normal resting state and prevents a
// partially edited setting screen from remaining visible indefinitely.
void updateSettingsTimeout() {
    if (current_ui_state != UI_SETTINGS) return;
    if (millis() - g_settings_last_activity_ms >= SETTINGS_INACTIVITY_TIMEOUT_MS) {
        exitSettingsMode();
    }
}

/**
 * @brief Moves the Settings-menu selection to the next/previous item,
 *        wrapping around at both ends.
 */
void navigateSettingsItem(bool next) {
    const int count = (int)SettingItem::SETTINGS_ITEM_COUNT;
    int idx = (int)g_settings_selected_item;
    idx = next ? (idx + 1) % count : (idx - 1 + count) % count;
    g_settings_selected_item = (SettingItem)idx;
}

/**
 * @brief Increments/decrements the currently selected Settings item by one
 *        encoder step, applying the change live (volume, AM frequency —
 *        which also recalculates the predistortion LUT — and output mode).
 */
void adjustSelectedSetting(bool increase) {
    switch (g_settings_selected_item) {
        case SettingItem::MELODY_VOLUME:
            setMelodyVolumeLevel((int)g_melody_volume_level + (increase ? 1 : -1));
            break;
        case SettingItem::PERCUSSION_VOLUME:
            setPercussionVolumeLevel((int)g_percussion_volume_level + (increase ? 1 : -1));
            break;
        case SettingItem::MASTER_VOLUME:
            setMasterVolumeLevel((int)g_master_volume_level + (increase ? 1 : -1));
            break;
        case SettingItem::AM_FREQUENCY: {
            // Compute in signed arithmetic, then clamp explicitly to the
            // advertised range before handing the value to the hardware.
            int32_t new_freq_khz = (int32_t)g_am_frequency_khz +
                                   (increase ? (int32_t)AM_STEP_KHZ : -(int32_t)AM_STEP_KHZ);
            if (new_freq_khz < (int32_t)AM_FREQ_MIN_KHZ) new_freq_khz = AM_FREQ_MIN_KHZ;
            if (new_freq_khz > (int32_t)AM_FREQ_MAX_KHZ) new_freq_khz = AM_FREQ_MAX_KHZ;
            setAMFrequencyKHz((uint16_t)new_freq_khz);
            break;
        }
        case SettingItem::OUTPUT_SELECTION: {
            int idx = (int)g_output_mode;
            idx = increase ? (idx + 1) % OUTPUT_MODE_COUNT : (idx - 1 + OUTPUT_MODE_COUNT) % OUTPUT_MODE_COUNT;
            applyOutputMode((OutputMode)idx);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Handles rotary encoder rotation via polling.
 *
 * Decodes CLK/DT with a full quadrature state machine (see
 * ROTARY_TRANSITION_TABLE above) so a mechanical bounce that reverses
 * mid-turn produces no event instead of an extra/undone step. Rotation
 * is also ignored while the encoder button is pressed, or shortly after
 * it changes state, since pressing/releasing it can mechanically jog the
 * CLK/DT contacts on a KY-040 module and would otherwise be misread as a
 * rotation step (see the Encoder Button Debounce comment above).
 */
void updateRotaryEncoder() {
    // Only active in menu selection or settings (never during UI_PLAYING).
    // Demo Mode is not a separate state here -- browsing DEMO_SONGS[] still
    // goes through UI_MENU_SELECTION, see g_current_song_list/g_current_song_count.
    if (current_ui_state != UI_MENU_SELECTION && current_ui_state != UI_SETTINGS) return;

    const bool button_currently_pressed = (digitalRead(PIN_ENC_SW) == LOW);
    const bool within_button_guard_window =
        (millis() - last_button_activity_ms < ENCODER_BUTTON_ROTARY_GUARD_MS);

    if (button_currently_pressed || within_button_guard_window) {
        // Reset to the idle detent state so that any mechanical noise
        // picked up while suppressed does not leave the state machine
        // stuck mid-transition (which could otherwise fire a stale event
        // as soon as suppression ends).
        rotary_encoder_state = ROT_R_START;
        return;
    }

    const uint8_t pin_state =
        (digitalRead(PIN_ENC_DT) << 1) | digitalRead(PIN_ENC_CLK);
    rotary_encoder_state =
        ROTARY_TRANSITION_TABLE[rotary_encoder_state & 0x0F][pin_state];
    const uint8_t direction = rotary_encoder_state & 0x30;

    if (direction == ROT_DIR_NONE) {
        return; // No completed detent transition yet.
    }

    const bool clockwise = (direction == ROT_DIR_CW);

    if (current_ui_state == UI_SETTINGS) {
        g_settings_last_activity_ms = millis();
    }

    if (current_ui_state == UI_SETTINGS) {
        // Settings Mode: navigate items, or edit the selected one's
        // value, depending on the current nav/edit sub-mode.
        if (g_settings_editing) {
            adjustSelectedSetting(clockwise);
        } else {
            navigateSettingsItem(clockwise);
        }
        renderSettingsScreen();
    } else if (clockwise) {
        // Clockwise: next song in whichever list is active
        // (g_current_song_list -- normal songs or Demo Mode's DEMO_SONGS[]).
        // The redraw itself is handled by updateOLEDDisplay()'s selection-
        // change polling, same as before.
        selected_song_index++;
        if (selected_song_index >= g_current_song_count) {
            selected_song_index = 0;
        }
    } else {
        // Counter-clockwise: previous song
        selected_song_index--;
        if (selected_song_index < 0) {
            selected_song_index = g_current_song_count - 1;
        }
    }
}

/**
 * @brief Handles push button with debounce and two-tier long-press
 *        detection (song-list toggle / Settings Mode), plus in-menu
 *        short-press actions (play song, toggle Settings nav<->edit).
 *
 * Two press tiers share the same button, resolved as follows:
 *   - SETTINGS_PRESS_MS (the longer hold) fires LIVE, the instant the
 *     threshold is crossed while still in UI_MENU_SELECTION — same
 *     "fire while held" UX the original single-tier long-press had.
 *   - LONG_PRESS_MS (the shorter hold) can NOT be resolved live, because at
 *     that point we don't yet know whether the user will keep holding past
 *     SETTINGS_PRESS_MS. It is instead resolved on button RELEASE: if the
 *     total hold time falls in [LONG_PRESS_MS, SETTINGS_PRESS_MS) and
 *     Settings didn't already fire, the song list is toggled (normal songs
 *     <-> Demo Mode) via toggleSongList() then.
 *   - A hold inside UI_SETTINGS past LONG_PRESS_MS exits back to the menu
 *     (fires live, same pattern as entering Settings).
 */
void updateEncoderButton() {
    // Only active in menu selection or settings (never during UI_PLAYING).
    if (current_ui_state != UI_MENU_SELECTION && current_ui_state != UI_SETTINGS) return;

    bool button_reading = digitalRead(PIN_ENC_SW);

    // Live long-hold check #1: from the menu, past SETTINGS_PRESS_MS -> Settings.
    if (button_reading == LOW && previous_button_state == LOW &&
        !button_settings_press_fired && current_ui_state == UI_MENU_SELECTION &&
        (millis() - button_press_start_time > SETTINGS_PRESS_MS)) {
        button_settings_press_fired = true;
        button_long_press_fired = true;  // also suppress the release-time list-toggle action below
        enterSettingsMode();
    }

    // Live long-hold check #2: from inside Settings, past LONG_PRESS_MS -> back to menu.
    if (button_reading == LOW && previous_button_state == LOW &&
        !button_long_press_fired && current_ui_state == UI_SETTINGS &&
        (millis() - button_press_start_time > LONG_PRESS_MS)) {
        button_long_press_fired = true;
        exitSettingsMode();
    }

    // Detect button state change (Pull-up: LOW when pressed)
    if (button_reading != previous_button_state) {
        if (millis() - last_click_time > BUTTON_DEBOUNCE_MS) {
            last_click_time = millis();

            // Mark button activity so updateRotaryEncoder() suspends
            // rotation decoding for ENCODER_BUTTON_ROTARY_GUARD_MS: both
            // the press and the release can mechanically jog the CLK/DT
            // contacts on a KY-040 module.
            last_button_activity_ms = last_click_time;

            if (button_reading == LOW) {
                // Button pressed down: record start time for long-press checks
                button_press_start_time = millis();
                button_long_press_fired = false;
                button_settings_press_fired = false;
            } else {
                // Button released.
                if (!button_long_press_fired) {
                    const unsigned long held_ms = millis() - button_press_start_time;

                    if (current_ui_state == UI_MENU_SELECTION && held_ms >= LONG_PRESS_MS) {
                        // Held past the list-toggle threshold but released
                        // before reaching the (already live-checked) Settings one.
                        toggleSongList();
                    } else if (current_ui_state == UI_MENU_SELECTION) {
                        startSongPlayback(selected_song_index);
                    } else if (current_ui_state == UI_SETTINGS) {
                        // Short press in Settings: toggle between browsing
                        // items and editing the currently selected one.
                        g_settings_last_activity_ms = millis();
                        g_settings_editing = !g_settings_editing;
                        renderSettingsScreen();
                    }
                }
            }
        }
        previous_button_state = button_reading;
    }
}

/**
 * @brief Updates the OLED display during menu navigation when selection changes.
 */
void updateOLEDDisplay() {
    // OLED polling updates only happen in menu selection state
    if (current_ui_state != UI_MENU_SELECTION) {
        return;
    }

    // Refresh rate limiter (150 ms)
    static unsigned long last_display_update = 0;
    const unsigned long display_interval = 150;

    unsigned long current_millis = millis();
    if (current_millis - last_display_update < display_interval) {
        return;
    }

    // Detect change in selected song index
    static int last_selected_song = -1;
    if (selected_song_index == last_selected_song) {
        return;
    }

    last_selected_song = selected_song_index;
    last_display_update = current_millis;

    renderMenuScreen();
}

// =============================================================================
// SETUP UI HARDWARE
// =============================================================================

void setupEncoderAndDisplay() {
    // Rotary Encoder pins configuration
    #ifdef INPUT_PULLUP
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    #else
    pinMode(PIN_ENC_CLK, INPUT);
    digitalWrite(PIN_ENC_CLK, HIGH);
    pinMode(PIN_ENC_DT, INPUT);
    digitalWrite(PIN_ENC_DT, HIGH);
    #endif
  
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // Prime the rotary state machine at the idle detent and the button
    // debounce trackers at their resting (not-pressed) state.
    rotary_encoder_state = ROT_R_START;
    previous_button_state = digitalRead(PIN_ENC_SW);
    playback_rotary_button_state = (uint8_t)previous_button_state;

    // Initialize OLED display (usual I2C address 0x3C or 0x3D)
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // Display initialization fallback if needed
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

// =============================================================================
// SETUP AND MAIN LOOP
// =============================================================================

void setup() {
#if ENABLE_AUDIO_LOGS
    Serial.begin(115200);
#endif
    delay(50);
#if ENABLE_AUDIO_LOGS
    resetAudioDiagnostics();
#endif

    // Default volume levels, expressed on the 0-20 encoder scale requested
    // for the Settings menu (melody 20/20, percussion 15/20, master 16/20).
    setMelodyVolumeLevel(VOLUME_LEVEL_MELODY_DEFAULT);
    setPercussionVolumeLevel(VOLUME_LEVEL_PERCUSSION_DEFAULT);
    setMasterVolumeLevel(VOLUME_LEVEL_MASTER_DEFAULT);
    setupEncoderAndDisplay();
  
    analogWriteResolution(12);
    analogWrite(A0, 0);
  
    precalculateMidiPhases();

    // Spread WAVE_GRAPH_SIZE captured samples evenly across one display
    // refresh window: at SAMPLE_RATE samples/sec, GRAPH_UPDATE_INTERVAL_MS
    // worth of audio contains (SAMPLE_RATE * GRAPH_UPDATE_INTERVAL_MS / 1000)
    // samples; we want 128 of those spaced out, not the first 128 in a row.
    {
        const uint32_t samples_per_window =
            ((uint32_t)SAMPLE_RATE * GRAPH_UPDATE_INTERVAL_MS) / 1000UL;
        wave_capture_base_stride = (uint16_t)max(
            (uint32_t)1, samples_per_window / WAVE_GRAPH_SIZE);
        wave_capture_stride = wave_capture_base_stride;
    }

    // Carrier PWM configuration (g_am_frequency_khz, using default freq, at 50% initial duty)
    carrier_pwm.begin((float)g_am_frequency_khz * 1000.0f, 50.0f);
    configurePWMRegister();
    initializePredistortionLUT();
    // Apply the default output routing (both DAC and RF active), matching
    // the original always-on behavior before output selection existed.
    applyOutputMode(g_output_mode);

    // Waveform lookup tables precalculation
    for (int i = 0; i < 256; i++) {
        const float rad = (2.0f * M_PI * (float)i) / 256.0f;
        const float sine = sinf(rad);
        const float triangle = (i < 128) ? (-1.0f + (2.0f * i / 128.0f)) : (3.0f - (2.0f * i / 128.0f));
        SINE_TABLE_F[i] = sine;
        // Wave tables are stored as signed 16-bit fixed-point samples.
        // This cuts SRAM use from 6,144 B to 3,072 B.
        SINE_TABLE[i] = (int16_t)(sine * 32767.0f);
        TRIANGLE_TABLE[i] = (int16_t)(triangle * 32767.0f);

        // Brighter triangle: slightly sharper around the peaks
        const float bright_triangle = copysignf(
            powf(fabsf(triangle), 0.88f), triangle);

        // Warmer triangle: rounds the waveform slightly
        const float warm_triangle = copysignf(
            powf(fabsf(triangle), 1.12f), triangle);

        BRIGHT_TRIANGLE_TABLE[i] = (int16_t)(bright_triangle * 32767.0f);
        WARM_TRIANGLE_TABLE[i] = (int16_t)(warm_triangle * 32767.0f);
        SQUARE_TABLE[i] = (i < 128) ? 32767 : -32767;
        SAW_TABLE[i] = (int16_t)((-1.0f + (2.0f * i / 256.0f)) * 32767.0f);
    }

    // Prepare initial song structure and display menu
    change_song(0);
    current_ui_state = UI_MENU_SELECTION;
    renderMenuScreen();

    // Hardware timer startup at SAMPLE_RATE
    uint8_t timer_type = GPT_TIMER;
    const int8_t audio_ch = FspTimer::get_available_timer(timer_type);
    
    if (audio_ch >= 0) {
        audio_timer.begin(
            TIMER_MODE_PERIODIC,
            timer_type,
            audio_ch,
            static_cast<float>(SAMPLE_RATE),
            0.0f,
            cb_audio
        );
        audio_timer.setup_overflow_irq();
        audio_timer.open();
        audio_timer.start();
    }
}

void loop() {
    if (current_ui_state == UI_PLAYING && seq.playing) {
        // Audio playback mode: generate samples continuously while the ring buffer has space.
        // The stop button is polled at a bounded interval without touching the OLED.
        // NOTE: must be `static` -- this counter has to persist across loop()
        // calls. Once the ring buffer reaches steady state, the ISR drains it
        // one sample at a time, so a single loop() pass usually only pushes
        // one sample before audioBufferHasSpace() goes false again. A plain
        // local here would reset to 0 on every loop() call and (almost)
        // never reach the ">= 8" checkpoint below, starving both this poll
        // of updatePlaybackStopButton() and updatePlaybackRotaryEncoder().
        static uint8_t playback_button_poll_counter = 0;

        updatePlaybackStopButton();
        if (!seq.playing) return;

        while (audioBufferHasSpace() && !seq.song_finished && seq.playing) {
            if (++playback_button_poll_counter >= 8) {
                playback_button_poll_counter = 0;
                updatePlaybackStopButton();
                if (!seq.playing) break;

                // Rotation is decoded at exactly the same safe checkpoint as
                // the panic button, so it cannot run concurrently with the
                // critical audio synthesis call below.
                updatePlaybackRotaryEncoder();
            }
#if ENABLE_AUDIO_LOGS
            const uint32_t t0 = micros();
#endif
            const float next_sample = compute_next_dsp_sample();
#if ENABLE_AUDIO_LOGS
            const uint32_t dt = micros() - t0;
            dsp_call_count++;
            dsp_us_sum += dt;
            if (dt > dsp_us_worst) dsp_us_worst = dt;
#endif

            audio_buffer_push(next_sample);

            // Waveform capture: every generated audio sample contributes to the
            // current interval. When the interval reaches wave_capture_stride,
            // the minimum and maximum values are committed as one display point.
            // This preserves narrow peaks instead of replacing an interval with
            // one arbitrarily timed sample. The work remains in loop() context,
            // never inside cb_audio().
            if (!wave_buffer_ready) {
                const float clipped_sample = constrain(next_sample, -1.0f, 1.0f);
                const int8_t q = (int8_t)(clipped_sample * 127.0f);

                if (q < wave_capture_interval_min) wave_capture_interval_min = q;
                if (q > wave_capture_interval_max) wave_capture_interval_max = q;

                if (++wave_capture_counter >= wave_capture_stride) {
                    wave_capture_counter = 0;

                    wave_capture_min_buf[wave_capture_pos] = wave_capture_interval_min;
                    wave_capture_max_buf[wave_capture_pos] = wave_capture_interval_max;
                    wave_capture_buf[wave_capture_pos] =
                        (int8_t)(((int16_t)wave_capture_interval_min +
                                  (int16_t)wave_capture_interval_max) / 2);

                    const int16_t interval_peak =
                        max(abs((int)wave_capture_interval_min),
                            abs((int)wave_capture_interval_max));

                    ++wave_capture_pos;
                    wave_capture_interval_min = 127;
                    wave_capture_interval_max = -127;
                    const float interval_peak_level =
                        (float)interval_peak / 127.0f;
                    if (interval_peak_level > scope_peak) scope_peak = interval_peak_level;

                    // RMS remains based on committed display intervals. The
                    // waveform itself always retains the exact interval
                    // minimum and maximum, so narrow peaks cannot disappear.
                    const float interval_midpoint =
                        (float)wave_capture_buf[wave_capture_pos - 1] / 127.0f;
                    scope_sum_sq += interval_midpoint * interval_midpoint;
                    ++scope_sample_count;
                    if ((scope_prev_sample < 0 && q >= 0) ||
                        (scope_prev_sample > 0 && q <= 0)) {
                        ++scope_zero_crossings;
                    }
                    scope_prev_sample = q;

                    if (wave_capture_pos >= WAVE_GRAPH_SIZE) {
                        wave_capture_pos = 0;
                        wave_buffer_ready = true;
                    }
                }
            }
        }

        // The while() above only exits when the ring buffer is full (max ISR
        // lead time -> safest possible moment for an I2C burst) or the song
        // just finished. Still throttled to GRAPH_UPDATE_INTERVAL_MS so a
        // short/fast song can't turn this into a free-running I2C hog.
        if (scope_display_dirty && !wave_buffer_ready) {
            // A zoom change starts a fresh capture frame. The OLED update is
            // deferred until this same safe display window; no I2C operation
            // is introduced into the DSP loop.
            //
            // IMPORTANT: last_graph_draw_ms is shared with the
            // wave_buffer_ready block below. In fullscreen mode there is
            // nothing to draw yet (we're waiting on the first zoomed
            // frame), so this branch must NOT touch the timer -- doing so
            // was racing against the wave_buffer_ready block: the instant
            // the capture finished, that block would find the throttle
            // "just reset" (by this do-nothing branch), skip the draw, and
            // still unconditionally clear wave_buffer_ready, discarding the
            // completed frame. That repeated indefinitely, so fullscreen
            // never actually rendered anything.
            if (!scope_fullscreen) {
                if (millis() - last_graph_draw_ms >= GRAPH_UPDATE_INTERVAL_MS) {
                    last_graph_draw_ms = millis();
                    renderPlayingScreen();
                    scope_display_dirty = false;
                }
            }
            // scope_fullscreen: retain the existing OLED contents and wait
            // patiently for wave_buffer_ready, without touching the timer.
        }

        if (wave_buffer_ready) {
            const unsigned long now_ms = millis();
            if (now_ms - last_graph_draw_ms >= GRAPH_UPDATE_INTERVAL_MS) {
                last_graph_draw_ms = now_ms;
                if (!scope_fullscreen && scope_display_dirty) {
                    renderPlayingScreen();
                } else {
                    drawWaveformFrame();
                }
                scope_display_dirty = false;
            }
            wave_buffer_ready = false; // Drop/refill either way; capture resumes next iteration.
        }

        // When the song finishes all steps, wait for the circular buffer to fully drain
        // through the hardware timer ISR before transitioning back to menu.
        if (seq.song_finished && audioBufferIsEmpty()) {
            // Print DSP performance and diagnostic logs to Serial Monitor
#if ENABLE_AUDIO_LOGS
            printDspPerformanceReport();
            printClippingLog();
            printAudioArtifactReport();
#endif

            // Reset sequence state and return to menu
            seq.playing = false;
            seq.song_finished = false;
            seq.elapsed_samples = 0;
            current_ui_state = UI_MENU_SELECTION;

            // Render the menu screen on the OLED display. Demo Mode's
            // "return to the list when it finishes" behaviour falls out of
            // this for free: g_current_song_list/selected_song_index are
            // untouched, so if a demo was playing, this redraws the demo
            // list right where it was left.
            renderMenuScreen();
        }
    } else if (current_ui_state == UI_SETTINGS) {
        // Settings menu: poll the encoder/button and return automatically
        // after inactivity. Screen updates are triggered directly by the
        // handlers whenever the selected item or value changes.
        updateRotaryEncoder();
        updateEncoderButton();
        updateSettingsTimeout();
    } else {
        // UI Menu Selection mode: song is NOT playing.
        // Handle rotary encoder, button debounce, and OLED display refresh.
        updateRotaryEncoder();
        updateEncoderButton();
        updateOLEDDisplay();
    }
}