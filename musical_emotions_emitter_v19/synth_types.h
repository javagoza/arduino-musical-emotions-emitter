#pragma once
#define NOTE_A4 69                           // Reference MIDI note (A4)
#define FREQ_A4 440.0f                       // Standard A4 frequency (440 Hz)
#define BUFFER_SIZE 512                      // Audio sample circular buffer size
#define BUFFER_MASK (BUFFER_SIZE - 1)

// --- MACROS FOR SAMPLE RATE SCALING ---

#define SCALE_SAMPLES(m) (((m) * SAMPLE_RATE) / 8000)
#define SCALE_PHASE_STEP(p) (((p) * 8000) / SAMPLE_RATE)

#define DELAY_BUFFER_SIZE SCALE_SAMPLES(256) // Delay/echo buffer size

// =============================================================================
// LIMITS AND VOICE POOL CONFIGURATION
// =============================================================================

#define MAX_VOICES 4                         // Maximum combined tracks per song
#define MAX_MELODIC_VOICES 4                 // Melodic synthesis engine pool size
#define MAX_PERCUSSION_VOICES 2              // Percussion engine pool size

// =============================================================================
// DATA TYPES AND ENUMERATIONS
// =============================================================================

// ADSR Envelope
enum class ADSREnvelopeState { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };

// Track/Voice type
enum class VoiceType : uint8_t {
    MELODY,
    PERCUSSION
};

// Available percussion instruments
enum class PercussionType : int8_t {
    NONE = 0,
    KICK = 1,
    SNARE = 2,
    HIHAT_CLOSED = 3,
    HIHAT_OPEN = 4,
    CRASH = 5,
    CLAP = 6,
    TOM_LOW = 7,
    TOM_MID = 8,
    TOM_HIGH = 9
};

// =============================================================================
// MUSICAL PARAMETERS AND SYNTHESIS STRUCTURES
// =============================================================================

// ADSR envelope parameters (times in ms, sustain level 0.0-1.0)
struct ADSRParameters {
    uint16_t attack_ms;
    uint16_t decay_ms;
    float sustain_level;
    uint16_t release_ms;
};

// Arpeggiator configuration
struct ArpeggioConfig {
    bool active;
    uint8_t semitones;                          // Arpeggio interval (e.g., +4 for major 3rd, +7 for perfect 5th)
    uint8_t speed;                              // Alternation speed in samples or subdivisions
};

// Complete definition of a melodic synthesis instrument
struct Instrument {
    const int16_t* wave_table;                  // Pointer to the waveform LUT (256 int16 samples)
    ADSRParameters adsr;                        // Volume envelope
    ArpeggioConfig arpeg;                       // Rapid arpeggio effect
    float volume;                               // Instrument relative output level (0.0 to 1.0)
    float vibrato_depth;                        // Vibrato depth
    float vibrato_freq;                         // Vibrato frequency in Hz
};

// Generic sequencer track / voice
struct Voice {
    const VoiceType type;                       // MELODIC or PERCUSSION
    const int8_t* const sequence;               // Relative semitones to tonic (Melody) or PercussionType (Percussion)
    const uint8_t* const rhythm;                // Step duration (4 = quarter note, 8 = eighth note, 16 = sixteenth note...)
    const uint16_t total_steps;                 // Pattern length
    const Instrument* const instrument;         // Instrument pointer (nullptr if PERCUSSION)
};

// Complete song structure
struct Song {
    const Voice* const voices;                  // Array of tracks/voices making up the song
    const uint8_t total_voices;                 // Number of configured voices
    const int tonic;                            // Tonic MIDI note (song base)
    const int bpm;                              // Tempo in beats per minute
    const char* title;                          // Song title
};

// =============================================================================
// RUNTIME STATE STRUCTURES
// =============================================================================

// Individual track state during playback
struct VoiceState {
    uint16_t step_idx = 0;                      // Current step index in the sequence
    uint32_t step_time_counter = 0;             // Elapsed samples in the current step
    uint32_t samples_total_duration = 0;         // Total duration of current step in samples
    uint32_t samples_gate_on_duration = 0;       // Active samples before 'release'
    uint8_t engine_idx = 0;                     // Assigned index in the corresponding pool (melodic or percussion)
};

// Global sequencer state
struct SequencerState {
    int current_song_idx = 0;                   // Index of the song currently playing
    VoiceState voices[MAX_VOICES];              // Individual state of each active voice
    bool playing = false;                       // True while the sequencer is actively generating a song
    bool song_finished = false;                 // True when the global song duration has been generated
    uint32_t song_duration_samples = 0;         // Duration of the longest voice sequence in samples
    uint32_t elapsed_samples = 0;               // Number of generated samples in the current song
};

struct AudioSample {
    uint32_t pwmDuty;                           // Duty cycle percentage value to adjust on the PWM carrier
    uint16_t dacVal;                            // 12-bit values for the internal DAC (0-4095)
};

// Circular buffer for DSP samples
struct AudioBuffer {
    AudioSample buffer[BUFFER_SIZE];
    volatile uint16_t head = 0;
    volatile uint16_t tail = 0;
};

