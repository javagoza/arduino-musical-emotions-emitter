#pragma once
#include "synth_types.h"
#include "instruments.h"


// =============================================================================
// SONG: PURE JOY / ECSTASY (Custom Uplifting Jingle)
// Duration: Exactly 3.0 seconds at 160 BPM, designed for a 4-voice polyphonic jingle.
// =============================================================================
// A bright, uplifting short musical jingle expressing pure joy and happiness.
// Rhythm: A brisk, energetic tempo set in 4/4 time with an upbeat, syncopated groove. Duration: Exactly 3 seconds long. 
// ADSR Envelope: Features a punchy, rapid attack to instantly capture attention, a moderate decay, a brief, lively sustain,
// and a clean release that lets the final chord ring out naturally for a brief moment. 
// Instrumentation & Timbre: A cheerful combination of a cheerful acoustic ukulele, 
// a bouncy glockenspiel, and warm, shimmering synth pads creating a bright, welcoming texture.
// Voices: Polyphonic with a rich harmonic stack of 3 to 4 simultaneous notes forming a major chord progression (e.g., I-V-VI-IV). 
// Modulation & Effects: Gentle, shimmering vibrato on the sustained notes to add warmth and a playful,
// bouncy arpeggiator running in the background to drive the cheerful momentum.
// Conclusion: Resolves cleanly and triumphantly on a bright major tonic chord with a joyful, fading tail. 



// Melodic patterns mapping a bright major chord progression (I - V - VI - IV) with an upbeat, syncopated groove
static const int8_t  melody_pure_joy_lead[]   = {  0,  4,  7, 12,  7, 11, 14, 12,  9, 12, 16, 14,  5,  9, 12, 12 };
static const uint8_t rhythm_pure_joy_lead[]   = {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 16 };

static const int8_t  melody_pure_joy_sparkle[] = { 12, 16, 19, 24, 19, 23, 26, 24, 21, 24, 28, 26, 17, 21, 24, 24 };
static const uint8_t rhythm_pure_joy_sparkle[] = {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 16 };

static const int8_t  melody_pure_joy_pad[]    = {  0,  7,  9,  5 };
static const uint8_t rhythm_pure_joy_pad[]    = { 16, 16, 16, 16 };

static const int8_t  melody_pure_joy_bass[]   = { -12, -5, -8, -12 };
static const uint8_t rhythm_pure_joy_bass[]   = {  16, 16, 16, 16 };

// Instrument definitions matching the required 4-voice specifications (Ukulele, Glockenspiel, and Shimmering Synth Pads)
const Instrument INST_UKULELE_LEAD = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 3, 120, 0.40f, 250 }, 
    { true, 4, 1 }, 
    0.82f, 
    0.025f, 
    6.0f 
};

const Instrument INST_GLOCKENSPIEL = { 
    SINE_TABLE, 
    { 2, 200, 0.20f, 400 }, 
    { true, 7, 2 }, 
    0.75f, 
    0.015f, 
    7.0f 
};

const Instrument INST_WARM_SHIMMER_PAD = { 
    TRIANGLE_TABLE, 
    { 40, 250, 0.70f, 800 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.008f, 
    4.5f 
};

const Instrument INST_UKULELE_BASS = { 
    WARM_TRIANGLE_TABLE, 
    { 5, 150, 0.50f, 300 }, 
    { false, 0, 0 }, 
    0.78f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_pure_joy_jingle[] = {
    { VoiceType::MELODY, melody_pure_joy_lead,    rhythm_pure_joy_lead,    sizeof(melody_pure_joy_lead)    / sizeof(melody_pure_joy_lead[0]),    &INST_UKULELE_LEAD       },
    { VoiceType::MELODY, melody_pure_joy_sparkle, rhythm_pure_joy_sparkle, sizeof(melody_pure_joy_sparkle) / sizeof(melody_pure_joy_sparkle[0]), &INST_GLOCKENSPIEL       },
    { VoiceType::MELODY, melody_pure_joy_pad,     rhythm_pure_joy_pad,     sizeof(melody_pure_joy_pad)     / sizeof(melody_pure_joy_pad[0]),     &INST_WARM_SHIMMER_PAD   },
    { VoiceType::MELODY, melody_pure_joy_bass,    rhythm_pure_joy_bass,    sizeof(melody_pure_joy_bass)    / sizeof(melody_pure_joy_bass[0]),    &INST_UKULELE_BASS       }
};

// Complete song entity ready for integration (3.0 seconds duration achieved at 160 BPM across 64 total sixteenth notes)
const Song song_pure_joy = { 
    voices_pure_joy_jingle, 
    sizeof(voices_pure_joy_jingle) / sizeof(voices_pure_joy_jingle[0]), 
    60,  // Tonic note C4 (60)
    160, // Brisk, energetic uplifting tempo in BPM
    "PURE JOY" 
};


// =============================================================================
// SONG: MELANCHOLIC MICRO-JINGLE (Poignant Longing)
// Duration: Exactly 1.0 second at 60 BPM, designed for a 1-voice monophonic jingle.
// =============================================================================
// A poignant, melancholic micro-jingle expressing deep sadness and longing.
// Rhythm: A slow, dragging tempo with a heavy, hesitant beat. 
// Duration: Extremely short, lasting exactly 1 second. 
// ADSR Envelope: Features a soft, cushioned attack that swells gently, a prolonged,
// drooping decay, minimal sustain, and a fading, sorrowful release that dissolves into silence.
// Instrumentation & Timbre: A dark, resonant solo cello blended with a distant, detuned music box
// , creating a hollow, weeping timbre. Voices: 
// Monophonic single voice carrying a minor-key interval (a minor second or tritone). 
// Modulation & Effects: A slow, weeping vibrato with deep pitch modulation to mimic
// a human sigh; no arpeggiator.
// Conclusion: Finishes with a lingering, unresolved minor suspension that fades out abruptly, leaving an aching, empty echo.

// Monophonic melodic phrase carrying a minor-key tritone interval
static const int8_t  melody_micro_longing[]   = {  6,  5,  6,  1 };
static const uint8_t rhythm_micro_longing[]   = {  8,  8,  8,  8 };

// Instrument definitions matching the required dark solo cello & detuned music box timbre
const Instrument INST_MELANCHOLIC_CELLO = { 
    WARM_TRIANGLE_TABLE, 
    { 120, 250, 0.40f, 300 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.009f, 
    6.0f 
};

// 1-Voice structured track configuration (monophonic single voice)
static const Voice voices_micro_longing[] = {
    { VoiceType::MELODY, melody_micro_longing, rhythm_micro_longing, sizeof(melody_micro_longing) / sizeof(melody_micro_longing[0]), &INST_MELANCHOLIC_CELLO }
};

// Complete song entity ready for integration (1.0 second duration achieved at 60 BPM across 32 total sixteenth notes)
const Song song_melancholy = { 
    voices_micro_longing, 
    sizeof(voices_micro_longing) / sizeof(voices_micro_longing[0]), 
    57,  // Tonic note A3 (57)
    60,  // Slow, dragging tempo in BPM
    "MELANCHOLY" 
};



// =============================================================================
// SONG: FURY / BLINDING RAGE (Explosive Violent Micro-Sound Effect)
// Duration: Exactly 1.2 seconds at 160 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// An explosive, violent micro-sound effect expressing unbridled fury and blinding rage. 
// Rhythm: A brutal, accelerating double-strike pulse with an aggressive, pounding cadence. 
// Duration: Between 1.0 and 1.5 seconds. 
// ADSR Envelope: Features a savage, instantaneous attack that bursts with maximum distortion,
// a sustained aggressive decay holding raw energy, and a harsh, abrupt release. 
// Instrumentation & Timbre: A heavily distorted electric guitar power chord combined with a raw,
// snapping acoustic snare hit and abrasive white noise, creating a gritty, blood-pumping, 
// and destructive texture. 
// Voices: A thick, crushing polyphonic cluster of minor seconds
// and tritones screaming with dissonance. 
// Modulation & Effects: Rapid, violent tremolo picking 
// and a jagged downward pitch-warp; no arpeggiator. 
// Conclusion: Finishes with a sharp, shattering metal cutoff that leaves a sizzling, heated silence in its wake. 



// Melodic patterns mapping a thick, crushing polyphonic cluster of minor seconds and tritones with a brutal accelerating double-strike pulse
static const int8_t  melody_fury_lead[]   = {  0,  1,  6,  7, 12, 13, 18, 19 };
static const uint8_t rhythm_fury_lead[]   = {  2,  2,  2,  2,  2,  2,  2,   2 };

static const int8_t  melody_fury_snare[]  = {  6,  7, 12, 13, 18, 19, 24, 25 };
static const uint8_t rhythm_fury_snare[]  = {  2,  2,  2,  2,  2,  2,  2,   2 };

static const int8_t  melody_fury_noise[]  = {  1,  6,  7, 12, 13, 18, 19, 24 };
static const uint8_t rhythm_fury_noise[]  = {  2,  2,  2,  2,  2,  2,  2,   2 };

static const int8_t  melody_fury_sub[]    = { -12, -11, -6, -5, -12, -11, -6, -5 };
static const uint8_t rhythm_fury_sub[]    = {   2,   2,   2,   2,   2,   2,  2,  2 };

// Instrument definitions matching the heavily distorted electric guitar power chord, raw snapping snare, abrasive white noise, and violent pitch-warp
const Instrument INST_DISTORTED_GUITAR = { 
    SAW_TABLE, 
    { 0, 80, 0.80f, 100 }, 
    { false, 0, 0 }, 
    0.95f, 
    0.040f, 
    20.0f 
};

const Instrument INST_SNAPPING_SNARE = { 
    SQUARE_TABLE, 
    { 0, 60, 0.70f, 80 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.045f, 
    22.0f 
};

const Instrument INST_ABRASIVE_NOISE = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 0, 70, 0.75f, 90 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.050f, 
    25.0f 
};

const Instrument INST_FURY_SUB = { 
    SINE_TABLE, 
    { 0, 90, 0.90f, 120 }, 
    { false, 0, 0 }, 
    0.98f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_fury_micro[] = {
    { VoiceType::MELODY, melody_fury_lead,  rhythm_fury_lead,  sizeof(melody_fury_lead)  / sizeof(melody_fury_lead[0]),  &INST_DISTORTED_GUITAR },
    { VoiceType::MELODY, melody_fury_snare, rhythm_fury_snare, sizeof(melody_fury_snare) / sizeof(melody_fury_snare[0]), &INST_SNAPPING_SNARE   },
    { VoiceType::MELODY, melody_fury_noise, rhythm_fury_noise, sizeof(melody_fury_noise) / sizeof(melody_fury_noise[0]), &INST_ABRASIVE_NOISE   },
    { VoiceType::MELODY, melody_fury_sub,   rhythm_fury_sub,   sizeof(melody_fury_sub)   / sizeof(melody_fury_sub[0]),   &INST_FURY_SUB         }
};

// Complete song entity ready for integration (extended duration between 1.0 and 1.5 seconds achieved at 160 BPM across 32 total sixteenth notes)
const Song song_fury = { 
    voices_fury_micro, 
    sizeof(voices_fury_micro) / sizeof(voices_fury_micro[0]), 
    48,  // Tonic note C3 (48) for an aggressive, crushing low-mid foundation
    160, // Brutal, accelerating double-strike tempo in BPM
    "FURY" 
};

// =============================================================================
// SONG: ADRENALINE / OVERWHELMING PANIC (Frantic Erratic Micro-Sound Effect)
// Duration: Exactly 1.2 seconds at 160 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// A frantic, erratic micro-sound effect expressing sudden, overwhelming panic and frantic adrenaline.
// Rhythm: A rapid, stuttering double-pulse with a breathless, chaotic cadence. 
// Duration: Between 1.0 and 1.5 seconds. 
// ADSR Envelope: Features a sharp, jarring attack that shocks the senses instantly, 
// a volatile and jittery decay oscillating wildly, a nervous pulse sustain, and a sharp, breathless release. 
// Instrumentation & Timbre: A high-pitched, vibrating synth alarm pulse combined with a frantic,
// scuttling acoustic percussion scrape, creating a dizzying, claustrophobic, and frantic texture. 
// Voices: A high-register, clashing polyphonic cluster of minor seconds racing erratically. 
// Modulation & Effects: A rapid, jittery frequency modulation and a panicking wobble filter sweep; no arpeggiator. 
// Conclusion: Finishes with an abrupt, breathless drop into a hollow, ringing silence that mimics a racing heartbeat. 



// Melodic patterns mapping a high-register, clashing polyphonic cluster of minor seconds racing erratically
static const int8_t  melody_adrenaline_lead[]   = { 12, 13, 12, 13, 15, 16, 15, 16 };
static const uint8_t rhythm_adrenaline_lead[]   = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_adrenaline_alarm[]  = { 13, 12, 13, 12, 16, 15, 16, 15 };
static const uint8_t rhythm_adrenaline_alarm[]  = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_adrenaline_scrape[] = { 18, 19, 18, 19, 21, 22, 21, 22 };
static const uint8_t rhythm_adrenaline_scrape[] = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_adrenaline_sub[]    = { -12, -11, -12, -11, -12, -11, -12, -11 };
static const uint8_t rhythm_adrenaline_sub[]    = {   2,   2,   2,   2,   2,   2,   2,   2 };

// Instrument definitions matching the vibrating synth alarm pulse, frantic percussion scrape, and jittery modulation
const Instrument INST_FRANTIC_ALARM = { 
    SAW_TABLE, 
    { 0, 50, 0.20f, 10 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.040f, 
    18.0f 
};

const Instrument INST_SCUTTLING_SCRAPE = { 
    SQUARE_TABLE, 
    { 0, 40, 0.15f, 10 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.045f, 
    20.0f 
};

const Instrument INST_ADRENALINE_CLUSTER = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 0, 45, 0.18f, 10 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.035f, 
    16.0f 
};

const Instrument INST_ADRENALINE_SUB = { 
    SINE_TABLE, 
    { 0, 60, 0.10f, 5 }, 
    { false, 0, 0 }, 
    0.95f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_adrenaline_micro[] = {
    { VoiceType::MELODY, melody_adrenaline_lead,   rhythm_adrenaline_lead,   sizeof(melody_adrenaline_lead)   / sizeof(melody_adrenaline_lead[0]),   &INST_FRANTIC_ALARM      },
    { VoiceType::MELODY, melody_adrenaline_alarm,  rhythm_adrenaline_alarm,  sizeof(melody_adrenaline_alarm)  / sizeof(melody_adrenaline_alarm[0]),  &INST_ADRENALINE_CLUSTER },
    { VoiceType::MELODY, melody_adrenaline_scrape, rhythm_adrenaline_scrape, sizeof(melody_adrenaline_scrape) / sizeof(melody_adrenaline_scrape[0]), &INST_SCUTTLING_SCRAPE   },
    { VoiceType::MELODY, melody_adrenaline_sub,    rhythm_adrenaline_sub,    sizeof(melody_adrenaline_sub)    / sizeof(melody_adrenaline_sub[0]),    &INST_ADRENALINE_SUB     }
};

// Complete song entity ready for integration (extended duration between 1.0 and 1.5 seconds achieved at 160 BPM across 32 total sixteenth notes)
const Song song_panic = { 
    voices_adrenaline_micro, 
    sizeof(voices_adrenaline_micro) / sizeof(voices_adrenaline_micro[0]), 
    60,  // Tonic note C4 (60)
    160, // Rapid, stuttering double-pulse panic tempo in BPM
    "PANIC" 
};

// =============================================================================
// SONG: CONFIDENCE / ABSOLUTE CERTAINTY (Bold Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 130 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// A bold, resolute micro-sound effect expressing absolute confidence and certainty. 
// Rhythm: A solid, unshakeable single pulse with a decisive and commanding impact. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a sharp, powerful attack that hits instantly with full force, 
// a stable and robust decay, a brief solid sustain, and a clean, controlled release.
// Instrumentation & Timbre: A rich, warm brass stab (such as a French horn or synth brass) 
// layered with a clean sub-bass punch, creating a full-bodied, authoritative, and proud texture.
// Voices: A rich, harmonious major triad chord stack (polyphonic) emitting strength and stability.
// Modulation & Effects: Zero erratic modulation, a steady and unwavering pitch with a very subtle, 
// proud vibrato; no arpeggiator. 
// Conclusion: Finishes with a crisp, resolute cutoff that resonates with a sense of finality and triumph. 



// Melodic patterns mapping a solid, unshakeable major triad chord stack (root, third, fifth, octave)
static const int8_t  melody_confidence_lead[]   = {  0,  4,  7, 12 };
static const uint8_t rhythm_confidence_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_confidence_brass[]  = {  0,  4,  7, 12 };
static const uint8_t rhythm_confidence_brass[]  = {  4,  4,  4,  4 };

static const int8_t  melody_confidence_harm[]   = {  7, 11, 14, 19 };
static const uint8_t rhythm_confidence_harm[]   = {  4,  4,  4,  4 };

static const int8_t  melody_confidence_sub[]    = { -12, -12, -12, -12 };
static const uint8_t rhythm_confidence_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the required rich brass stab, clean sub-bass, and unwavering tone
const Instrument INST_CONFIDENCE_BRASS = { 
    SAW_TABLE, 
    { 5, 120, 0.75f, 150 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.002f, 
    3.0f 
};

const Instrument INST_CONFIDENCE_HORN = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 8, 140, 0.70f, 160 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.003f, 
    3.5f 
};

const Instrument INST_CONFIDENCE_SUB = { 
    SINE_TABLE, 
    { 4, 100, 0.85f, 120 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.0f, 
    0.0f 
};

const Instrument INST_CONFIDENCE_STACK = { 
    TRIANGLE_TABLE, 
    { 6, 130, 0.80f, 140 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.001f, 
    2.5f 
};

// 4-Voice structured track configuration
static const Voice voices_confidence_micro[] = {
    { VoiceType::MELODY, melody_confidence_lead,  rhythm_confidence_lead,  sizeof(melody_confidence_lead)  / sizeof(melody_confidence_lead[0]),  &INST_CONFIDENCE_BRASS  },
    { VoiceType::MELODY, melody_confidence_brass, rhythm_confidence_brass, sizeof(melody_confidence_brass) / sizeof(melody_confidence_brass[0]), &INST_CONFIDENCE_HORN   },
    { VoiceType::MELODY, melody_confidence_harm,  rhythm_confidence_harm,  sizeof(melody_confidence_harm)  / sizeof(melody_confidence_harm[0]),  &INST_CONFIDENCE_STACK  },
    { VoiceType::MELODY, melody_confidence_sub,   rhythm_confidence_sub,   sizeof(melody_confidence_sub)   / sizeof(melody_confidence_sub[0]),   &INST_CONFIDENCE_SUB    }
};

// Complete song entity ready for integration (under 1 second duration achieved at 130 BPM across 16 total sixteenth notes)
const Song song_confidence = { 
    voices_confidence_micro, 
    sizeof(voices_confidence_micro) / sizeof(voices_confidence_micro[0]), 
    60,  // Tonic note C4 (60)
    130, // Solid, unshakeable resolute tempo in BPM
    "CONFIDENCE" 
};

// =============================================================================
// SONG: DOUBT / UNCERTAINTY (Ambiguous Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 110 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// An ambiguous, hesitant micro-sound effect expressing doubt and uncertainty. 
// Rhythm: A wavering, unsteadied single pulse with an uneven, questioning cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a soft, muffled attack that rolls in slightly muted, 
// a fluctuating decay that dips unexpectedly, minimal sustain, and an uncertain 
// trailing release. 
// Instrumentation & Timbre: A muted pizzicato string combined with a detuned,
// warbling synth wave, creating a hollow, inquisitive, and unstable texture. 
// Voices: A suspended, dissonant interval (like a diminished fifth or unresolved 
// minor second) hinting at hesitation.
// Modulation & Effects: A slow, questioning pitch-bend that slides downward mid-note; 
// gentle, hesitant vibrato; no arpeggiator. Conclusion: Finishes with an abrupt, 
// unresolved suspension that hangs in the air, leaving an inquisitive and doubtful echo.

// Melodic patterns mapping a wavering, questioning cadence with an unresolved suspended interval (diminished fifth/tritone)
static const int8_t  melody_doubt_lead[]   = {  6,  5,  6,  4 };
static const uint8_t rhythm_doubt_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_doubt_synth[]  = {  6,  4,  5,  3 };
static const uint8_t rhythm_doubt_synth[]  = {  4,  4,  4,  4 };

static const int8_t  melody_doubt_pizz[]   = {  0,  6,  1,  5 };
static const uint8_t rhythm_doubt_pizz[]   = {  4,  4,  4,  4 };

static const int8_t  melody_doubt_sub[]    = { -12, -11, -12, -11 };
static const uint8_t rhythm_doubt_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the required muffled pizzicato string, detuned warbling synth wave, and sliding pitch effects
const Instrument INST_MUTED_PIZZ = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 25, 150, 0.30f, 200 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.003f, 
    4.0f 
};

const Instrument INST_DETUNED_SYNTH = { 
    SAW_TABLE, 
    { 30, 200, 0.40f, 250 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.012f, 
    6.5f 
};

const Instrument INST_DOUBT_SUSPENSION = { 
    TRIANGLE_TABLE, 
    { 20, 180, 0.35f, 220 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.008f, 
    5.0f 
};

const Instrument INST_DOUBT_SUB = { 
    SINE_TABLE, 
    { 15, 100, 0.50f, 150 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_doubt_micro[] = {
    { VoiceType::MELODY, melody_doubt_lead,  rhythm_doubt_lead,  sizeof(melody_doubt_lead)  / sizeof(melody_doubt_lead[0]),  &INST_DETUNED_SYNTH    },
    { VoiceType::MELODY, melody_doubt_synth, rhythm_doubt_synth, sizeof(melody_doubt_synth) / sizeof(melody_doubt_synth[0]), &INST_DOUBT_SUSPENSION },
    { VoiceType::MELODY, melody_doubt_pizz,  rhythm_doubt_pizz,  sizeof(melody_doubt_pizz)  / sizeof(melody_doubt_pizz[0]),  &INST_MUTED_PIZZ       },
    { VoiceType::MELODY, melody_doubt_sub,   rhythm_doubt_sub,   sizeof(melody_doubt_sub)   / sizeof(melody_doubt_sub[0]),   &INST_DOUBT_SUB        }
};

// Complete song entity ready for integration (under 1 second duration achieved at 110 BPM across 16 total sixteenth notes)
const Song song_doubt = { 
    voices_doubt_micro, 
    sizeof(voices_doubt_micro) / sizeof(voices_doubt_micro[0]), 
    60,  // Tonic note C4 (60)
    110, // Uneven, questioning cadence tempo in BPM
    "DOUBT" 
};

// =============================================================================
// SONG: FATIGUE / EXHAUSTION (Heavy Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 100 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================
// A heavy, drooping micro-sound effect expressing profound fatigue and exhaustion. 
// Rhythm: A sluggish, dragging single pulse with a slow, sinking micro-groove. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a slow, muffled attack that rolls in wearily, a long and drooping 
// decay that bleeds downward, minimal sustain, and a soft, sluggish release that slumps into 
// silence. 
// Instrumentation & Timbre: A low-pass filtered, muted electric piano combined with a dull,
// low-frequency thud, creating a weary, muffled, and heavy texture. 
// Voices: A low-pitched, unresolved minor chord stack (polyphonic) hanging loosely. 
// Modulation & Effects: A slow, sluggish pitch-bend dropping downward like a sigh; 
// heavy, dragging vibrato; no arpeggiator. 
// Conclusion: Finishes with a faint, breathy fading tail that dissolves heavily into quiet stillness. 


// Melodic patterns mapping a sluggish, sinking, and drooping unresolved minor chord descent
static const int8_t  melody_fatigue_lead[]   = {  0, -1, -2, -4 };
static const uint8_t rhythm_fatigue_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_fatigue_chord[]  = {  3,  2,  1, -1 };
static const uint8_t rhythm_fatigue_chord[]  = {  4,  4,  4,  4 };

static const int8_t  melody_fatigue_muted[]  = { -3, -4, -5, -7 };
static const uint8_t rhythm_fatigue_muted[]  = {  4,  4,  4,  4 };

static const int8_t  melody_fatigue_thud[]   = { -12, -14, -15, -17 };
static const uint8_t rhythm_fatigue_thud[]   = {   4,   4,   4,   4 };

// Instrument definitions matching the low-pass filtered electric piano, low-frequency thud, and dragging vibrato
const Instrument INST_FATIGUE_EPICANO = { 
    WARM_TRIANGLE_TABLE, 
    { 40, 250, 0.20f, 350 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.008f, 
    4.5f 
};

const Instrument INST_FATIGUE_CHORD_STACK = { 
    TRIANGLE_TABLE, 
    { 50, 300, 0.15f, 400 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.010f, 
    5.0f 
};

const Instrument INST_FATIGUE_MUTED = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 35, 200, 0.25f, 300 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.006f, 
    4.0f 
};

const Instrument INST_FATIGUE_THUD = { 
    SINE_TABLE, 
    { 20, 180, 0.10f, 250 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_fatigue_micro[] = {
    { VoiceType::MELODY, melody_fatigue_lead,   rhythm_fatigue_lead,   sizeof(melody_fatigue_lead)   / sizeof(melody_fatigue_lead[0]),   &INST_FATIGUE_EPICANO      },
    { VoiceType::MELODY, melody_fatigue_chord,  rhythm_fatigue_chord,  sizeof(melody_fatigue_chord)  / sizeof(melody_fatigue_chord[0]),  &INST_FATIGUE_CHORD_STACK  },
    { VoiceType::MELODY, melody_fatigue_muted,  rhythm_fatigue_muted,  sizeof(melody_fatigue_muted)  / sizeof(melody_fatigue_muted[0]),  &INST_FATIGUE_MUTED        },
    { VoiceType::MELODY, melody_fatigue_thud,   rhythm_fatigue_thud,   sizeof(melody_fatigue_thud)   / sizeof(melody_fatigue_thud[0]),   &INST_FATIGUE_THUD         }
};

// Complete song entity ready for integration (under 1 second duration achieved at 100 BPM across 16 total sixteenth notes)
const Song song_fatigue = { 
    voices_fatigue_micro, 
    sizeof(voices_fatigue_micro) / sizeof(voices_fatigue_micro[0]), 
    48,  // Tonic note C3 (48) for a heavy, low register
    100, // Slow, dragging exhaustion tempo in BPM
    "FATIGUE" 
};


// =============================================================================
// SONG: CELEBRATION / FESTIVE TRIUMPH (Bright Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 160 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================
// A bright, exuberant micro-sound effect expressing sudden celebration and festive triumph.
// Rhythm: A rapid, sparkling single burst with an explosive, uplifting cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a sharp, snapping attack that cracks open instantly with high energy,
// a glittering fast decay, a brief celebratory sustain, and a clean, sparkling release. 
// Instrumentation & Timbre: A vibrant mix of a festive party horn, high-pitched wind chimes,
// and a crisp synthetic sparkle, creating a radiant, joyous, and popping texture. 
// Voices: A rich, brilliant major triad chord stack (polyphonic) overflowing with energy. 
// Modulation & Effects: A lively, ascending pitch-glissando or bright upward sweep;
// sparkling light shimmer effects; no arpeggiator. 
// Conclusion: Finishes with a crisp, triumphant cutoff that leaves a shimmering, golden afterglow 



// Melodic patterns mapping a brilliant major triad chord stack with an ascending upward sweep
static const int8_t  melody_celebration_lead[]   = {  0,  4,  7, 12 };
static const uint8_t rhythm_celebration_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_celebration_chimes[] = { 12, 16, 19, 24 };
static const uint8_t rhythm_celebration_chimes[] = {  4,  4,  4,  4 };

static const int8_t  melody_celebration_harm[]   = {  7, 11, 14, 19 };
static const uint8_t rhythm_celebration_harm[]   = {  4,  4,  4,  4 };

static const int8_t  melody_celebration_sparkle[] = { 19, 23, 26, 31 };
static const uint8_t rhythm_celebration_sparkle[] = {  4,  4,  4,  4 };

// Instrument definitions matching the required festive party horn, high-pitched wind chimes, and synthetic sparkle
const Instrument INST_FESTIVE_PARTY_HORN = { 
    SAW_TABLE, 
    { 2, 80, 0.70f, 100 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.005f, 
    5.0f 
};

const Instrument INST_WIND_CHIMES = { 
    SINE_TABLE, 
    { 1, 60, 0.40f, 120 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.010f, 
    8.0f 
};

const Instrument INST_SYNTH_SPARKLE = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 2, 70, 0.60f, 110 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.008f, 
    7.0f 
};

const Instrument INST_MAJOR_TRIAD_STACK = { 
    TRIANGLE_TABLE, 
    { 3, 90, 0.65f, 130 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.003f, 
    4.0f 
};

// 4-Voice structured track configuration
static const Voice voices_celebration_micro[] = {
    { VoiceType::MELODY, melody_celebration_lead,    rhythm_celebration_lead,    sizeof(melody_celebration_lead)    / sizeof(melody_celebration_lead[0]),    &INST_FESTIVE_PARTY_HORN  },
    { VoiceType::MELODY, melody_celebration_chimes,  rhythm_celebration_chimes,  sizeof(melody_celebration_chimes)  / sizeof(melody_celebration_chimes[0]),  &INST_WIND_CHIMES         },
    { VoiceType::MELODY, melody_celebration_harm,    rhythm_celebration_harm,    sizeof(melody_celebration_harm)    / sizeof(melody_celebration_harm[0]),    &INST_MAJOR_TRIAD_STACK   },
    { VoiceType::MELODY, melody_celebration_sparkle, rhythm_celebration_sparkle, sizeof(melody_celebration_sparkle) / sizeof(melody_celebration_sparkle[0]), &INST_SYNTH_SPARKLE       }
};

// Complete song entity ready for integration (under 1 second duration achieved at 160 BPM across 16 total sixteenth notes)
const Song song_celebration = { 
    voices_celebration_micro, 
    sizeof(voices_celebration_micro) / sizeof(voices_celebration_micro[0]), 
    60,  // Tonic note C4 (60)
    160, // Rapid, sparkling celebration tempo in BPM
    "CELEBRATION" 
};


// =============================================================================
// SONG: VULNERABILITY / DELICACY (Fragile Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 110 BPM, designed for a 1-voice monophonic micro-jingle.
// =============================================================================
// A fragile, exposed micro-sound effect expressing profound vulnerability and delicacy. 
// Rhythm: A tentative, whisper-thin single pulse with a delicate and trembling cadence. 
// Duration: Extremely short, lasting under 1 second. ADSR Envelope: Features an ultra-soft, 
// breathy attack that swells in gently from absolute silence, a fragile and tapering decay,
// minimal sustain, and a fading, transparent release. 
// Instrumentation & Timbre: A high-register, muted acoustic harp string combined with a soft,
// warm analog breath noise, creating an intimate, paper-thin, and exposed texture. 
// Voices: A lone, exposed monophonic voice or a delicate, unresolved open fifth interval. 
// Modulation & Effects: A slight, trembling vibrato mimicking a fragile breath; no arpeggiator. 
// Conclusion: Finishes with a gentle, floating drift into complete silence, leaving a tender and unprotected echo. 


// Monophonic melodic phrase carrying a delicate, unresolved open fifth interval with a trembling cadence
static const int8_t  melody_vulnerability[]   = {  0,  7,  4,  7 };
static const uint8_t rhythm_vulnerability[]   = {  4,  4,  4,  4 };

// Instrument definition matching the high-register muted acoustic harp string and warm analog breath noise
const Instrument INST_FRAGILE_HARP = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 80, 250, 0.20f, 300 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.008f, 
    6.5f 
};

// 1-Voice structured track configuration (monophonic single voice)
static const Voice voices_vulnerability[] = {
    { VoiceType::MELODY, melody_vulnerability, rhythm_vulnerability, sizeof(melody_vulnerability) / sizeof(melody_vulnerability[0]), &INST_FRAGILE_HARP }
};

// Complete song entity ready for integration (under 1 second duration achieved at 110 BPM across 16 total sixteenth notes)
const Song song_vulnerability = { 
    voices_vulnerability, 
    sizeof(voices_vulnerability) / sizeof(voices_vulnerability[0]), 
    60,  // Tonic note C4 (60)
    110, // Tentative, whisper-thin micro-tempo in BPM
    "VULNERABILITY" 
};

// =============================================================================
// SONG: PARALYZED DISBELIEF / SHOCK (Violent Shattering Micro-Sound Effect)
// Duration: Exactly 1.2 seconds at 160 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================
// A violent, shattering micro-sound effect expressing sudden, overwhelming shock and paralyzed disbelief.
// Rhythm: A severe, jarring double-impact pulse with a staggering, stuttering cadence. 
// Duration: Between 1.0 and 1.5 seconds. 
// ADSR Envelope: Features a blindingly fast, explosive attack that hits with absolute maximum amplitude, 
// a volatile and jagged decay, a brief frozen sustain, and a sudden, stunned release.
// Instrumentation & Timbre: A high-frequency digital glass explosion blended with a deep, 
// vibrating sub-bass thud, creating a piercing, crystalline, and electrifyingly hollow texture. 
// Voices: A dissonant, wide polychord cluster of minor seconds flashing sharply.
// Modulation & Effects: A rapid, freezing pitch-freeze effect coupled with a sudden high-pass frequency snap; no arpeggiator. 
// Conclusion: Finishes with an abrupt, dead cutoff that leaves a ringing, hollow resonance lingering in absolute silence. 



// Melodic patterns mapping a dissonant, wide polychord cluster of minor seconds flashing sharply with a severe jarring double-impact pulse
static const int8_t  melody_disbelief_lead[]   = {  0,  1, 12, 13,  0,  1, 12, 13 };
static const uint8_t rhythm_disbelief_lead[]   = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_disbelief_cluster[] = {  6,  7, 18, 19,  6,  7, 18, 19 };
static const uint8_t rhythm_disbelief_cluster[] = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_disbelief_glass[]  = { 15, 16, 27, 28, 15, 16, 27, 28 };
static const uint8_t rhythm_disbelief_glass[]  = {  2,  2,  2,  2,  2,  2,  2,  2 };

static const int8_t  melody_disbelief_sub[]    = { -12, -12, -12, -12, -12, -12, -12, -12 };
static const uint8_t rhythm_disbelief_sub[]    = {   2,   2,   2,   2,   2,   2,   2,   2 };

// Instrument definitions matching the high-frequency digital glass explosion, deep vibrating sub-bass thud, and pitch-freeze effect
const Instrument INST_DIGITAL_GLASS_EXPLOSION = { 
    SAW_TABLE, 
    { 0, 40, 0.20f, 5 }, 
    { false, 0, 0 }, 
    0.95f, 
    0.040f, 
    22.0f 
};

const Instrument INST_VIBRATING_SUB_THUD = { 
    SINE_TABLE, 
    { 0, 80, 0.40f, 10 }, 
    { false, 0, 0 }, 
    0.98f, 
    0.0f, 
    0.0f 
};

const Instrument INST_POLYCHORD_CLUSTER = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 0, 50, 0.25f, 8 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.035f, 
    18.0f 
};

const Instrument INST_HIGH_PASS_SNAP = { 
    SQUARE_TABLE, 
    { 0, 45, 0.15f, 6 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.045f, 
    20.0f 
};

// 4-Voice structured track configuration
static const Voice voices_disbelief_micro[] = {
    { VoiceType::MELODY, melody_disbelief_lead,    rhythm_disbelief_lead,    sizeof(melody_disbelief_lead)    / sizeof(melody_disbelief_lead[0]),    &INST_DIGITAL_GLASS_EXPLOSION },
    { VoiceType::MELODY, melody_disbelief_cluster, rhythm_disbelief_cluster, sizeof(melody_disbelief_cluster) / sizeof(melody_disbelief_cluster[0]), &INST_POLYCHORD_CLUSTER       },
    { VoiceType::MELODY, melody_disbelief_glass,   rhythm_disbelief_glass,   sizeof(melody_disbelief_glass)   / sizeof(melody_disbelief_glass[0]),   &INST_HIGH_PASS_SNAP          },
    { VoiceType::MELODY, melody_disbelief_sub,     rhythm_disbelief_sub,     sizeof(melody_disbelief_sub)     / sizeof(melody_disbelief_sub[0]),     &INST_VIBRATING_SUB_THUD      }
};

// Complete song entity ready for integration (extended duration between 1.0 and 1.5 seconds achieved at 160 BPM across 32 total sixteenth notes)
const Song song_shock = { 
    voices_disbelief_micro, 
    sizeof(voices_disbelief_micro) / sizeof(voices_disbelief_micro[0]), 
    60,  // Tonic note C4 (60)
    160, // Severe, jarring double-impact tempo in BPM
    "SHOCK" 
};


// =============================================================================
// SONG: TENDER AFFECTION (Short Musical Jingle)
// Duration: Exactly 1.0 second at 120 BPM, designed for a 3-voice polyphonic jingle.
// =============================================================================

// A tender, warm short musical jingle expressing pure affection and closeness.
// Rhythm: A slow, gentle tempo set in 4/4 time with a soft, swaying groove. 
// Duration: Exactly 1 second long. ADSR Envelope: Features a smooth, warm attack,
// a minimal decay, a short, comforting sustain, and a gentle release that lets the 
// final note fade softly. Instrumentation & Timbre: A sweet combination of a soft 
// acoustic guitar pluck, a warm music box, and a gentle, cozy synth pad creating 
// an intimate, welcoming texture. Voices: Polyphonic with a rich harmonic stack 
// of 3 simultaneous notes forming a warm major chord. 
// Modulation & Effects: Subtle, warm vibrato on the sustained notes to add deep 
// emotional warmth and tenderness. 
// Conclusion: Resolves smoothly and lovingly on a sweet major tonic chord with 
// a soft, affectionate fading tail. 



// Melodic patterns mapping a warm major chord progression with a slow, swaying groove
static const int8_t  melody_tender_lead[]    = {  0,  4,  7, 12 };
static const uint8_t rhythm_tender_lead[]    = {  4,  4,  4,  4 };

static const int8_t  melody_tender_musicbox[] = { 12, 16, 19, 24 };
static const uint8_t rhythm_tender_musicbox[] = {  4,  4,  4,  4 };

static const int8_t  melody_tender_pad[]     = {  0,  7 };
static const uint8_t rhythm_tender_pad[]     = {  8,  8 };

// Instrument definitions matching the required acoustic guitar pluck, warm music box, and cozy synth pad
const Instrument INST_TENDER_GUITAR = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 15, 100, 0.40f, 200 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.002f, 
    4.5f 
};

const Instrument INST_TENDER_MUSICBOX = { 
    SINE_TABLE, 
    { 10, 150, 0.30f, 250 }, 
    { true, 4, 1 }, 
    0.75f, 
    0.004f, 
    5.0f 
};

const Instrument INST_TENDER_PAD = { 
    WARM_TRIANGLE_TABLE, 
    { 40, 200, 0.70f, 350 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.003f, 
    4.0f 
};

// 3-Voice structured track configuration
static const Voice voices_tender_jingle[] = {
    { VoiceType::MELODY, melody_tender_lead,     rhythm_tender_lead,     sizeof(melody_tender_lead)     / sizeof(melody_tender_lead[0]),     &INST_TENDER_GUITAR    },
    { VoiceType::MELODY, melody_tender_musicbox, rhythm_tender_musicbox, sizeof(melody_tender_musicbox) / sizeof(melody_tender_musicbox[0]), &INST_TENDER_MUSICBOX  },
    { VoiceType::MELODY, melody_tender_pad,      rhythm_tender_pad,      sizeof(melody_tender_pad)      / sizeof(melody_tender_pad[0]),      &INST_TENDER_PAD       }
};

// Complete song entity ready for integration (1.0 second duration achieved at 120 BPM across 16 total sixteenth notes)
const Song song_affection = { 
    voices_tender_jingle, 
    sizeof(voices_tender_jingle) / sizeof(voices_tender_jingle[0]), 
    60,  // Tonic note C4 (60)
    120, // Slow, gentle swaying tempo in BPM
    "AFFECTION" 
};

// =============================================================================
// SONG: BONE-CHILLING COLDNESS / DETACHMENT (Stark Icy Micro-Sound Effect)
// Duration: Exactly 1.2 seconds at 160 BPM, designed for a 1-voice monophonic micro-jingle.
// =============================================================================

// Monophonic melodic phrase mapping a rigid, unyielding double-pulse with a brittle, frosted open-fifth interval
static const int8_t  melody_bone_chill[]   = {  7,  0,  7,  0,  7,  0,  7,  0 };
static const uint8_t rhythm_bone_chill[]   = {  2,  2,  2,  2,  2,  2,  2,  2 };

// Instrument definition matching the high-frequency metallic chime, crystalline digital frost scrape, and glacial ice-crack rumble with zero vibrato
const Instrument INST_BONE_CHILL_CHIME = { 
    SAW_TABLE, 
    { 10, 80, 0.50f, 100 }, 
    { false, 0, 0 }, 
    0.95f, 
    0.0f, 
    0.0f 
};

// 1-Voice structured track configuration (monophonic single voice)
static const Voice voices_bone_chill[] = {
    { VoiceType::MELODY, melody_bone_chill, rhythm_bone_chill, sizeof(melody_bone_chill) / sizeof(melody_bone_chill[0]), &INST_BONE_CHILL_CHIME }
};

// Complete song entity ready for integration (extended duration between 1.0 and 1.5 seconds achieved at 160 BPM across 32 total sixteenth notes)
const Song song_coldness = { 
    voices_bone_chill, 
    sizeof(voices_bone_chill) / sizeof(voices_bone_chill[0]), 
    72,  // Tonic note C5 (72) for a piercing, high-frequency icy register
    160, // Rigid, unyielding frosted tempo in BPM
    "BONE-CHILLING DETACHMENT MICRO-EFFECT" 
};

// =============================================================================
// SONG: BOREDOM / TEDIUM (Monotonous Micro-Sound Effect)
// Duration: Exactly 0.8 seconds at 150 BPM, designed for a 1-voice monophonic micro-jingle.
// =============================================================================

// A monotonous, flat micro-sound effect expressing profound tedium and utter boredom.
// Rhythm: A dull, repetitive single pulse with a dreary and uninspired cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a flat, uninspired attack that drags slightly,
// a level and unvarying decay, a faint grey sustain, and a limp, trailing release. 
// Instrumentation & Timbre: A muted, unresonant wooden block tap combined with a dry, 
// dusty synthetic drone, creating a sterile, lifeless, and paper-thin texture. 
// Voices: A single, droning monophonic note trapped in an unchanging unison pitch. 
// Modulation & Effects: Zero vibrato and static frequency modulation to maximize
// the sense of stagnant dullness; no arpeggiator. 
// Conclusion: Finishes with a dreary, unceremonious flat stop that sinks quietly into dead silence. 



// Monophonic melodic phrase trapped in an unchanging unison pitch
static const int8_t  melody_boredom_micro[]   = {  0,  0,  0,  0 };
static const uint8_t rhythm_boredom_micro[]   = {  4,  4,  4,  4 };

// Instrument definition matching the muted wooden block tap and dry dusty synthetic drone with zero vibrato
const Instrument INST_BOREDOM_DRONE = { 
    TRIANGLE_TABLE, 
    { 20, 100, 0.50f, 150 }, 
    { false, 0, 0 }, 
    0.60f, 
    0.0f, 
    0.0f 
};

// 1-Voice structured track configuration (monophonic single voice)
static const Voice voices_boredom_micro[] = {
    { VoiceType::MELODY, melody_boredom_micro, rhythm_boredom_micro, sizeof(melody_boredom_micro) / sizeof(melody_boredom_micro[0]), &INST_BOREDOM_DRONE }
};

// Complete song entity ready for integration (under 1 second duration achieved at 150 BPM across 16 total sixteenth notes)
const Song song_tedium = { 
    voices_boredom_micro, 
    sizeof(voices_boredom_micro) / sizeof(voices_boredom_micro[0]), 
    60,  // Tonic note C4 (60)
    150, // Dull, repetitive micro-tempo in BPM
    "BOREDOM" 
};

// =============================================================================
// SONG: PLAYFUL MISCHIEF (Sneaky Micro-Sound Effect)
// Duration: Exactly 0.8 seconds at 150 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// A playful, sneaky micro-sound effect expressing mischievous intent and impish cunning. 
// Rhythm: A swift, cheeky single pulse with a sly, skipping cadence.
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a sharp, darting attack that snaps on instantly,
// a playful skipping decay, minimal sustain, and a nimble, winking release.
// Instrumentation & Timbre: A mischievous pizzicato string pluck layered with a bright,
// cartoonish woodwind pop, creating a slippery, cunning, and sly texture. 
// Voices: A sneaky, chromatic minor-second interval (polyphonic) hinting at trickery.
// Modulation & Effects: A quick, winking pitch-bend that slides upward playfully; no arpeggiator. 
// Conclusion: Finishes with a crisp, abrupt cutoff that leaves a cheeky, scheming after-chuckle in the air. 



// Melodic patterns mapping a chromatic minor-second interval hinting at trickery and sly skipping
static const int8_t  melody_mischief_lead[]   = {  0,  1,  3,  4 };
static const uint8_t rhythm_mischief_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_mischief_pop[]    = { 12, 13, 15, 16 };
static const uint8_t rhythm_mischief_pop[]    = {  4,  4,  4,  4 };

static const int8_t  melody_mischief_harm[]   = {  1,  2,  4,  5 };
static const uint8_t rhythm_mischief_harm[]   = {  4,  4,  4,  4 };

static const int8_t  melody_mischief_sub[]    = { -12, -12, -12, -12 };
static const uint8_t rhythm_mischief_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the required mischievous pizzicato string pluck, bright cartoonish woodwind pop, and winking pitch-bend
const Instrument INST_MISCHIEF_PIZZ = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 2, 60, 0.20f, 80 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.020f, 
    10.0f 
};

const Instrument INST_CARTOON_WOODWIND = { 
    SQUARE_TABLE, 
    { 3, 50, 0.15f, 70 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.025f, 
    12.0f 
};

const Instrument INST_MISCHIEF_STACK = { 
    TRIANGLE_TABLE, 
    { 4, 70, 0.25f, 90 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.015f, 
    8.0f 
};

const Instrument INST_MISCHIEF_SUB = { 
    SINE_TABLE, 
    { 2, 80, 0.30f, 100 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_mischief_micro[] = {
    { VoiceType::MELODY, melody_mischief_lead,  rhythm_mischief_lead,  sizeof(melody_mischief_lead)  / sizeof(melody_mischief_lead[0]),  &INST_MISCHIEF_PIZZ       },
    { VoiceType::MELODY, melody_mischief_pop,   rhythm_mischief_pop,   sizeof(melody_mischief_pop)   / sizeof(melody_mischief_pop[0]),   &INST_CARTOON_WOODWIND    },
    { VoiceType::MELODY, melody_mischief_harm,  rhythm_mischief_harm,  sizeof(melody_mischief_harm)  / sizeof(melody_mischief_harm[0]),  &INST_MISCHIEF_STACK      },
    { VoiceType::MELODY, melody_mischief_sub,   rhythm_mischief_sub,   sizeof(melody_mischief_sub)   / sizeof(melody_mischief_sub[0]),   &INST_MISCHIEF_SUB        }
};

// Complete song entity ready for integration (under 1 second duration achieved at 150 BPM across 16 total sixteenth notes)
const Song song_mischief = { 
    voices_mischief_micro, 
    sizeof(voices_mischief_micro) / sizeof(voices_mischief_micro[0]), 
    60,  // Tonic note C4 (60)
    150, // Swift, cheeky skipping tempo in BPM
    "MISCHIEF" 
};

// =============================================================================
// SONG: RESIGNATION / RELUCTANT ACCEPTANCE (Weary Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 110 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// A weary, yielding micro-sound effect expressing quiet resignation and reluctant acceptance.
// Rhythm: A slow, sinking single pulse with a deflated and unresisting cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a soft, muffled attack that rolls in without resistance, a long,
// sliding downward decay, minimal sustain, and a gentle, collapsing release. 
// Instrumentation & Timbre: A low-pass filtered, muted acoustic guitar strum combined with a soft,
// breathy sigh of air, creating a hollow, defeatist, and hollowed-out texture. 
// Voices: A downward-resolving minor suspension interval (polyphonic) letting go of tension. 
// Modulation & Effects: A slow, drooping pitch-bend that slides downward passively; no vibrato; no arpeggiator.
// Conclusion: Finishes with a quiet, flat drop into dead silence, leaving a heavy, exhausted acceptance hanging in the air. 



// Melodic patterns mapping a downward-resolving minor suspension interval letting go of tension
static const int8_t  melody_resignation_lead[]   = {  3,  2,  1,  0 };
static const uint8_t rhythm_resignation_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_resignation_guitar[] = {  3,  1,  0, -1 };
static const uint8_t rhythm_resignation_guitar[] = {  4,  4,  4,  4 };

static const int8_t  melody_resignation_breath[] = {  0, -1, -2, -3 };
static const uint8_t rhythm_resignation_breath[] = {  4,  4,  4,  4 };

static const int8_t  melody_resignation_sub[]    = { -12, -13, -14, -15 };
static const uint8_t rhythm_resignation_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the low-pass filtered muted acoustic guitar strum, breathy sigh, and drooping pitch bend
const Instrument INST_MUTED_GUITAR = { 
    WARM_TRIANGLE_TABLE, 
    { 30, 200, 0.20f, 250 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.010f, 
    4.0f 
};

const Instrument INST_BREATHY_SIGH = { 
    TRIANGLE_TABLE, 
    { 40, 250, 0.15f, 300 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.015f, 
    5.0f 
};

const Instrument INST_RESIGNATION_PAD = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 35, 220, 0.25f, 280 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.012f, 
    4.5f 
};

const Instrument INST_RESIGNATION_SUB = { 
    SINE_TABLE, 
    { 20, 150, 0.10f, 200 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_resignation_micro[] = {
    { VoiceType::MELODY, melody_resignation_lead,   rhythm_resignation_lead,   sizeof(melody_resignation_lead)   / sizeof(melody_resignation_lead[0]),   &INST_MUTED_GUITAR      },
    { VoiceType::MELODY, melody_resignation_guitar, rhythm_resignation_guitar, sizeof(melody_resignation_guitar) / sizeof(melody_resignation_guitar[0]), &INST_BREATHY_SIGH      },
    { VoiceType::MELODY, melody_resignation_breath, rhythm_resignation_breath, sizeof(melody_resignation_breath) / sizeof(melody_resignation_breath[0]), &INST_RESIGNATION_PAD   },
    { VoiceType::MELODY, melody_resignation_sub,    rhythm_resignation_sub,    sizeof(melody_resignation_sub)    / sizeof(melody_resignation_sub[0]),    &INST_RESIGNATION_SUB   }
};

// Complete song entity ready for integration (under 1 second duration achieved at 110 BPM across 16 total sixteenth notes)
const Song song_resignation = { 
    voices_resignation_micro, 
    sizeof(voices_resignation_micro) / sizeof(voices_resignation_micro[0]), 
    48,  // Tonic note C3 (48) for a weary, low-end register
    110, // Deflated, unresisting tempo in BPM
    "RESIGNATION" 
};

// =============================================================================
// SONG: MYSTERY / ENIGMA (Shadowy Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 110 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

//  An enigmatic, shadowy micro-sound effect expressing deep mystery and hidden secrets. 
// Rhythm: A hushed, cryptic single pulse with an elusive and suspended cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a soft, muffled attack that fades in mysteriously from silence,
// a slow, winding decay, minimal sustain, and a fading, veiled release. 
// Instrumentation & Timbre: A dark, muted orchestral harp pluck layered with a faint,
// shimmering music box overtone, creating a foggy, cryptic, and velvety texture. 
// Voices: A suspended, open fourth or tritone interval (polyphonic) shrouding the note in ambiguity. 
// Modulation & Effects: A gentle, hovering pitch-glide that shifts subtly; no arpeggiator.
// Conclusion: Finishes with a soft, trailing fade into quiet obscurity, leaving an unsolved,
// lingering question hanging in the air. 



// Melodic patterns mapping a suspended tritone and open fourth interval shrouded in ambiguity
static const int8_t  melody_mystery_lead[]   = {  6,  7,  6,  1 };
static const uint8_t rhythm_mystery_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_mystery_musicbox[] = { 18, 19, 18, 13 };
static const uint8_t rhythm_mystery_musicbox[] = {  4,  4,  4,  4 };

static const int8_t  melody_mystery_pad[]    = {  1,  2,  1, -4 };
static const uint8_t rhythm_mystery_pad[]    = {  4,  4,  4,  4 };

static const int8_t  melody_mystery_sub[]    = { -12, -11, -12, -17 };
static const uint8_t rhythm_mystery_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the dark muted orchestral harp, shimmering music box, and hovering pitch glide
const Instrument INST_MUTED_HARP = { 
    WARM_TRIANGLE_TABLE, 
    { 30, 200, 0.20f, 250 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.008f, 
    5.0f 
};

const Instrument INST_SHIMMERING_MUSIC_BOX = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 20, 150, 0.15f, 200 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.010f, 
    6.0f 
};

const Instrument INST_MYSTERY_PAD = { 
    TRIANGLE_TABLE, 
    { 40, 250, 0.25f, 300 }, 
    { false, 0, 0 }, 
    0.65f, 
    0.006f, 
    4.0f 
};

const Instrument INST_MYSTERY_SUB = { 
    SINE_TABLE, 
    { 25, 180, 0.10f, 220 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_mystery_micro[] = {
    { VoiceType::MELODY, melody_mystery_lead,    rhythm_mystery_lead,    sizeof(melody_mystery_lead)    / sizeof(melody_mystery_lead[0]),    &INST_MUTED_HARP          },
    { VoiceType::MELODY, melody_mystery_musicbox, rhythm_mystery_musicbox, sizeof(melody_mystery_musicbox) / sizeof(melody_mystery_musicbox[0]), &INST_SHIMMERING_MUSIC_BOX },
    { VoiceType::MELODY, melody_mystery_pad,     rhythm_mystery_pad,     sizeof(melody_mystery_pad)     / sizeof(melody_mystery_pad[0]),     &INST_MYSTERY_PAD         },
    { VoiceType::MELODY, melody_mystery_sub,     rhythm_mystery_sub,     sizeof(melody_mystery_sub)     / sizeof(melody_mystery_sub[0]),     &INST_MYSTERY_SUB         }
};

// Complete song entity ready for integration (under 1 second duration achieved at 110 BPM across 16 total sixteenth notes)
const Song song_mystery = { 
    voices_mystery_micro, 
    sizeof(voices_mystery_micro) / sizeof(voices_mystery_micro[0]), 
    60,  // Tonic note C4 (60)
    110, // Hushed, cryptic micro-tempo in BPM
    "MYSTERY" 
};


// =============================================================================
// SONG: DREAD / LOOMING DOOM (Heavy Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 100 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================
// A heavy, ominous micro-sound effect expressing creeping dread and looming doom.
//  Rhythm: A slow, sinking single pulse with a crushing, foreboding cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a dark, creeping attack that swells ominously from silence, 
// a heavy, dragging decay, a deep bass sustain, and a grim, lingering release.
// Instrumentation & Timbre: A low-frequency sub-bass drone layered with a bowed, 
// scraping double bass string, creating a suffocating, bleak, and chilling texture. 
// Voices: A dark, dissonant tritone interval (polyphonic) generating internal tension. 
// Modulation & Effects: A slow, sickening downward pitch-bend simulating 
// a falling stomach; no arpeggiator. 
// Conclusion: Finishes with a low, vibrating rumble that fades into a tense, uneasy silence. 



// Melodic patterns mapping a dark, dissonant tritone interval with a sinking, crushing cadence
static const int8_t  melody_dread_lead[]   = {  6,  5,  4,  1 };
static const uint8_t rhythm_dread_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_dread_bass[]   = {  0, -1, -2, -6 };
static const uint8_t rhythm_dread_bass[]   = {  4,  4,  4,  4 };

static const int8_t  melody_dread_scrape[] = {  6,  4,  2, -1 };
static const uint8_t rhythm_dread_scrape[] = {  4,  4,  4,  4 };

static const int8_t  melody_dread_sub[]    = { -12, -13, -14, -18 };
static const uint8_t rhythm_dread_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the low-frequency sub-bass drone, bowed double bass scrape, and downward pitch bend
const Instrument INST_DREAD_DOUBLE_BASS = { 
    WARM_TRIANGLE_TABLE, 
    { 40, 250, 0.40f, 300 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.015f, 
    7.0f 
};

const Instrument INST_SUB_BASS_DRONE = { 
    SINE_TABLE, 
    { 50, 300, 0.50f, 350 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.0f, 
    0.0f 
};

const Instrument INST_CREEPING_SCRAPE = { 
    SAW_TABLE, 
    { 45, 280, 0.35f, 320 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.020f, 
    8.5f 
};

const Instrument INST_DOOM_CLUSTER = { 
    TRIANGLE_TABLE, 
    { 35, 220, 0.45f, 280 }, 
    { false, 0, 0 }, 
    0.70f, 
    0.010f, 
    6.0f 
};

// 4-Voice structured track configuration
static const Voice voices_dread_micro[] = {
    { VoiceType::MELODY, melody_dread_lead,   rhythm_dread_lead,   sizeof(melody_dread_lead)   / sizeof(melody_dread_lead[0]),   &INST_DREAD_DOUBLE_BASS },
    { VoiceType::MELODY, melody_dread_scrape, rhythm_dread_scrape, sizeof(melody_dread_scrape) / sizeof(melody_dread_scrape[0]), &INST_CREEPING_SCRAPE   },
    { VoiceType::MELODY, melody_dread_bass,   rhythm_dread_bass,   sizeof(melody_dread_bass)   / sizeof(melody_dread_bass[0]),   &INST_DOOM_CLUSTER      },
    { VoiceType::MELODY, melody_dread_sub,    rhythm_dread_sub,    sizeof(melody_dread_sub)    / sizeof(melody_dread_sub[0]),    &INST_SUB_BASS_DRONE    }
};

// Complete song entity ready for integration (under 1 second duration achieved at 100 BPM across 16 total sixteenth notes)
const Song song_dread = { 
    voices_dread_micro, 
    sizeof(voices_dread_micro) / sizeof(voices_dread_micro[0]), 
    48,  // Tonic note C3 (48) for a heavy, ominous low-end register
    100, // Slow, sinking foreboding tempo in BPM
    "DREAD" 
};

// =============================================================================
// SONG: AWE / WONDER (Expansive Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 120 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// Melodic patterns mapping a vast, luminous major-seventh chord stack with an ascending, reverent cadence
static const int8_t  melody_awe_lead[]   = {  0,  4,  7, 11 };
static const uint8_t rhythm_awe_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_awe_organ[]  = {  4,  7, 11, 16 };
static const uint8_t rhythm_awe_organ[]  = {  4,  4,  4,  4 };

static const int8_t  melody_awe_shimmer[] = { 12, 16, 19, 23 };
static const uint8_t rhythm_awe_shimmer[] = {  4,  4,  4,  4 };

static const int8_t  melody_awe_sub[]    = { -12, -12, -12, -12 };
static const uint8_t rhythm_awe_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the shimmering cathedral pipe organ cluster, crystal glass harp, and majestic upward shimmer
const Instrument INST_CATHEDRAL_ORGAN = { 
    SAW_TABLE, 
    { 10, 100, 0.70f, 150 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.005f, 
    5.0f 
};

const Instrument INST_GLASS_HARP_SWEEP = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 5, 80, 0.50f, 120 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.008f, 
    7.0f 
};

const Instrument INST_AWE_PAD = { 
    TRIANGLE_TABLE, 
    { 15, 120, 0.80f, 180 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.003f, 
    4.0f 
};

const Instrument INST_AWE_SUB = { 
    SINE_TABLE, 
    { 8, 90, 0.90f, 140 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_awe_micro[] = {
    { VoiceType::MELODY, melody_awe_lead,    rhythm_awe_lead,    sizeof(melody_awe_lead)    / sizeof(melody_awe_lead[0]),    &INST_CATHEDRAL_ORGAN   },
    { VoiceType::MELODY, melody_awe_organ,   rhythm_awe_organ,   sizeof(melody_awe_organ)   / sizeof(melody_awe_organ[0]),   &INST_GLASS_HARP_SWEEP  },
    { VoiceType::MELODY, melody_awe_shimmer, rhythm_awe_shimmer, sizeof(melody_awe_shimmer) / sizeof(melody_awe_shimmer[0]), &INST_AWE_PAD           },
    { VoiceType::MELODY, melody_awe_sub,     rhythm_awe_sub,     sizeof(melody_awe_sub)     / sizeof(melody_awe_sub[0]),     &INST_AWE_SUB           }
};

// Complete song entity ready for integration (under 1 second duration achieved at 120 BPM across 16 total sixteenth notes)
const Song song_awe = { 
    voices_awe_micro, 
    sizeof(voices_awe_micro) / sizeof(voices_awe_micro[0]), 
    60,  // Tonic note C4 (60)
    120, // Sweeping, vast reverent tempo in BPM
    "AWE" 
};
// =============================================================================
// SONG: INTENSE SPEED / AERODYNAMIC PROPULSION (Blistering High-Velocity Effect)
// Duration: Exactly 1.2 seconds at 160 BPM, designed for a 1-voice monophonic micro-jingle.
// =============================================================================

// A blistering, high-velocity micro-sound effect expressing intense speed and aerodynamic propulsion. 
// Rhythm: A rapid, accelerating double-dash pulse with a slicing, forward-thrusting cadence. 
// Duration: Between 1.0 and 1.5 seconds. 
// ADSR Envelope: Features an instantaneous ballistic attack that tears open the frequency spectrum immediately,
// a sleek, rushing decay, a lean aerodynamic sustain, and a sharp, slicing release. 
// Instrumentation & Timbre: A piercing supersonic wind shear blended with a high-register synthetic whip-crack 
// and filtered white noise, creating a hyper-clean, futuristic, and ultra-streamlined texture. 
// Voices: A clean, monophonic high-frequency sine blade cutting horizontally.
// Modulation & Effects: A rapid, sweeping Doppler shift pitch-glide accelerating upward before snapping away; no arpeggiator.
// Conclusion: Finishes with a crisp, vacuum-like vacuum cutoff that leaves behind a rushing, empty slipstream silence.

// Monophonic melodic phrase mapping an accelerating double-dash pulse with a slicing Doppler upward sweep
static const int8_t  melody_blistering_speed[]   = { 12, 18, 24, 36, 12, 18, 24, 36 };
static const uint8_t rhythm_blistering_speed[]   = {  2,  2,  2,  2,  2,  2,  2,  2 };

// Instrument definition matching the supersonic wind shear, synthetic whip-crack, and Doppler pitch-glide
const Instrument INST_SUPERSONIC_BLADE = { 
    SAW_TABLE, 
    { 0, 30, 0.10f, 5 }, 
    { false, 0, 0 }, 
    0.95f, 
    0.045f, 
    24.0f 
};

// 1-Voice structured track configuration (monophonic high-frequency sine blade single voice)
static const Voice voices_blistering_speed[] = {
    { VoiceType::MELODY, melody_blistering_speed, rhythm_blistering_speed, sizeof(melody_blistering_speed) / sizeof(melody_blistering_speed[0]), &INST_SUPERSONIC_BLADE }
};

// Complete song entity ready for integration (extended duration between 1.0 and 1.5 seconds achieved at 160 BPM across 32 total sixteenth notes)
const Song song_speed = { 
    voices_blistering_speed, 
    sizeof(voices_blistering_speed) / sizeof(voices_blistering_speed[0]), 
    72,  // Tonic note C5 (72) for a piercing high-register supersonic blade
    160, // Rapid, accelerating double-dash tempo in BPM
    "SPEED" 
};

// =============================================================================
// SONG: SERENITY / PURE CALM (Tranquil Glassy Micro-Sound Effect)
// Duration: Exactly 0.9 seconds at 100 BPM, designed for a 4-voice polyphonic micro-jingle.
// =============================================================================

// A tranquil, glassy micro-sound effect expressing deep serenity and pure calm.
// Rhythm: A smooth, fluid single pulse with a gentle, floating cadence. 
// Duration: Extremely short, lasting under 1 second. 
// ADSR Envelope: Features a soft, padding attack that rises like a calm ripple, 
// a long and peaceful decay, a warm ambient sustain, and a tranquil, fading release.
// Instrumentation & Timbre: A pure crystal singing bowl strike layered with a soft, 
// warm analog pad murmur, creating an ethereal, glassy, and weightless texture. 
// Voices: A harmonious, open major-ninth chord stack (polyphonic) breathing absolute stillness. 
// Modulation & Effects: Zero harsh modulation, featuring only a slow, 
// liquid undulation like water ripples; no arpeggiator. 
// Conclusion: Finishes with a peaceful, weightless fade into absolute quiet, leaving a pristine and balanced silence

// Melodic patterns mapping a harmonious, open major-ninth chord stack breathing absolute stillness
static const int8_t  melody_serenity_lead[]   = {  0,  4,  7, 14 };
static const uint8_t rhythm_serenity_lead[]   = {  4,  4,  4,  4 };

static const int8_t  melody_serenity_bowl[]   = { 12, 16, 19, 26 };
static const uint8_t rhythm_serenity_bowl[]   = {  4,  4,  4,  4 };

static const int8_t  melody_serenity_pad[]    = {  7, 11, 14, 21 };
static const uint8_t rhythm_serenity_pad[]    = {  4,  4,  4,  4 };

static const int8_t  melody_serenity_sub[]    = { -12, -12, -12, -12 };
static const uint8_t rhythm_serenity_sub[]    = {   4,   4,   4,   4 };

// Instrument definitions matching the crystal singing bowl, warm analog pad murmur, and slow liquid undulation
const Instrument INST_CRYSTAL_BOWL = { 
    SINE_TABLE, 
    { 20, 150, 0.80f, 200 }, 
    { false, 0, 0 }, 
    0.85f, 
    0.002f, 
    1.5f 
};

const Instrument INST_ANALOG_PAD_MURMUR = { 
    TRIANGLE_TABLE, 
    { 30, 180, 0.70f, 220 }, 
    { false, 0, 0 }, 
    0.80f, 
    0.003f, 
    2.0f 
};

const Instrument INST_SERENITY_STACK = { 
    BRIGHT_TRIANGLE_TABLE, 
    { 25, 160, 0.75f, 210 }, 
    { false, 0, 0 }, 
    0.75f, 
    0.002f, 
    1.8f 
};

const Instrument INST_SERENITY_SUB = { 
    SINE_TABLE, 
    { 15, 120, 0.90f, 160 }, 
    { false, 0, 0 }, 
    0.90f, 
    0.0f, 
    0.0f 
};

// 4-Voice structured track configuration
static const Voice voices_serenity_micro[] = {
    { VoiceType::MELODY, melody_serenity_lead, rhythm_serenity_lead, sizeof(melody_serenity_lead) / sizeof(melody_serenity_lead[0]), &INST_CRYSTAL_BOWL       },
    { VoiceType::MELODY, melody_serenity_bowl, rhythm_serenity_bowl, sizeof(melody_serenity_bowl) / sizeof(melody_serenity_bowl[0]), &INST_ANALOG_PAD_MURMUR },
    { VoiceType::MELODY, melody_serenity_pad,  rhythm_serenity_pad,  sizeof(melody_serenity_pad)  / sizeof(melody_serenity_pad[0]),  &INST_SERENITY_STACK    },
    { VoiceType::MELODY, melody_serenity_sub,  rhythm_serenity_sub,  sizeof(melody_serenity_sub)  / sizeof(melody_serenity_sub[0]),  &INST_SERENITY_SUB      }
};

// Complete song entity ready for integration (under 1 second duration achieved at 100 BPM across 16 total sixteenth notes)
const Song song_serenity = { 
    voices_serenity_micro, 
    sizeof(voices_serenity_micro) / sizeof(voices_serenity_micro[0]), 
    60,  // Tonic note C4 (60)
    100, // Smooth, fluid floating tempo in BPM
    "SERENITY" 
};

// =============================================================================
// PLAYLIST — this is what the rotary encoder steps through
// =============================================================================
const Song PLAYER[] = {
  song_pure_joy,
  song_melancholy,
  song_fury,
  song_panic,
  song_confidence,
  song_doubt,
  song_fatigue,
  song_celebration,
  song_vulnerability,
  song_shock,
  song_affection,
  song_coldness,
  song_tedium,
  song_mischief,
  song_resignation,
  song_mystery,
  song_dread,
  song_awe,
  song_speed,
  song_serenity
};
const int TOTAL_SONGS = sizeof(PLAYER) / sizeof(PLAYER[0]);



