#pragma once
#include "instruments.h"
// =============================================================================
// DEMO SONG LIST — Feature-showcase songs played through the normal player
// =============================================================================
// This file defines every demo Instrument/Voice/Song. It never touches
// PLAYER[]/TOTAL_SONGS (the normal song list) — it defines its own list,
// DEMO_SONGS[], played through the exact same synthesis engine
// (melodic_engines[], percussion_engines[], delay_fx) and the exact same
// UI_MENU_SELECTION / UI_PLAYING code as normal songs.
//
// Demo Mode is NOT a separate playback path or UI state: toggleSongList()
// in the .ino simply points g_current_song_list/g_current_song_count at
// DEMO_SONGS[]/TOTAL_DEMO_SONGS instead of PLAYER[]/TOTAL_SONGS. From then
// on, selecting and playing a demo goes through startSongPlayback() and
// renderPlayingScreen() exactly like a normal song -- same oscilloscope and
// monitoring panel, same RF/output-mode handling, same "return to the list
// when it finishes" behaviour.
//
// HOW TO WIRE THIS INTO THE .ino:
//   #include "demo.h" anywhere after synth_types.h (it only needs the
//   Instrument/Voice/Song type definitions). DEMO_SONGS[]/TOTAL_DEMO_SONGS
//   are then available for toggleSongList() to reference.
// =============================================================================


// =============================================================================
// CATEGORY 1 — PURE WAVEFORM COMPARISON (Timbral Foundation)
// One sustained tonic note per waveform. Volumes are pre-balanced by ear:
// SQUARE/SAW are harmonically denser and read louder than SINE at equal gain,
// so their `volume` is trimmed down a bit for a fair A/B comparison.
// =============================================================================

static const int8_t  melody_demo_wave[]  = { 0 };          // held tonic note
static const uint8_t rhythm_demo_wave[]  = { 1 };          // whole note -> long sustain

#define DEMO_WAVE_VOICE(inst) { { VoiceType::MELODY, melody_demo_wave, rhythm_demo_wave, 1, &(inst) } }

static const Voice voices_demo_wave_sine[]      = DEMO_WAVE_VOICE(INST_DEMO_WAVE_SINE);
static const Voice voices_demo_wave_triangle[]  = DEMO_WAVE_VOICE(INST_DEMO_WAVE_TRIANGLE);
static const Voice voices_demo_wave_square[]    = DEMO_WAVE_VOICE(INST_DEMO_WAVE_SQUARE);
static const Voice voices_demo_wave_saw[]       = DEMO_WAVE_VOICE(INST_DEMO_WAVE_SAW);
static const Voice voices_demo_wave_brighttri[] = DEMO_WAVE_VOICE(INST_DEMO_WAVE_BRIGHTTRI);
static const Voice voices_demo_wave_warmtri[]   = DEMO_WAVE_VOICE(INST_DEMO_WAVE_WARMTRI);

const Song song_demo_wave_sine      = { voices_demo_wave_sine,      1, 60, 50, "1.1 WAVE: SINE" };
const Song song_demo_wave_triangle  = { voices_demo_wave_triangle,  1, 60, 50, "1.2 WAVE: TRIANGLE" };
const Song song_demo_wave_square    = { voices_demo_wave_square,    1, 60, 50, "1.3 WAVE: SQUARE" };
const Song song_demo_wave_saw       = { voices_demo_wave_saw,       1, 60, 50, "1.4 WAVE: SAWTOOTH" };
const Song song_demo_wave_brighttri = { voices_demo_wave_brighttri, 1, 60, 50, "1.5 WAVE: BRIGHT TRI" };
const Song song_demo_wave_warmtri   = { voices_demo_wave_warmtri,   1, 60, 50, "1.6 WAVE: WARM TRI" };

// =============================================================================
// CATEGORY 2 — ADSR ENVELOPE ARTICULATION (Dynamic Shaping)
// Rapid repeated notes ("mi-mi-mi-mi"). Release time is deliberately LONGER
// than the gap between notes, so these entries only sound clean because of
// the hard-reset-on-trigger fix in ADSR::trigger() — without it the
// envelopes would blend into one continuous tone.
// =============================================================================

static const int8_t  melody_demo_adsr[] = { 4, 4, 4, 4, 4, 4, 4, 4 };   // "mi-mi-mi-mi..."
static const uint8_t rhythm_demo_adsr[] = { 8, 8, 8, 8, 8, 8, 8, 8 };

static const Voice voices_demo_adsr_staccato[] = {
  { VoiceType::MELODY, melody_demo_adsr, rhythm_demo_adsr, 8, &INST_DEMO_ADSR_STACCATO }
};
static const Voice voices_demo_adsr_longtail[] = {
  { VoiceType::MELODY, melody_demo_adsr, rhythm_demo_adsr, 8, &INST_DEMO_ADSR_LONGTAIL }
};

const Song song_demo_adsr_staccato = { voices_demo_adsr_staccato, 1, 60, 140, "2.1 ADSR: STACCATO" };
const Song song_demo_adsr_longtail = { voices_demo_adsr_longtail, 1, 60, 130, "2.2 ADSR: LONG RELEASE" };

// =============================================================================
// CATEGORY 3 — VIBRATO & PITCH MODULATION (Expressiveness)
// Same melodic phrase, same waveform/ADSR, only vibrato_depth/vibrato_freq
// change between entries: light -> medium -> deep+fast.
// =============================================================================

static const int8_t  melody_demo_vibrato[] = { 0, 4, 7, 12, 7, 4, 0 };
static const uint8_t rhythm_demo_vibrato[] = { 4, 4, 4, 4, 4, 4, 4 };

static const Voice voices_demo_vibrato_light[]  = { { VoiceType::MELODY, melody_demo_vibrato, rhythm_demo_vibrato, 7, &INST_DEMO_VIBRATO_LIGHT } };
static const Voice voices_demo_vibrato_medium[] = { { VoiceType::MELODY, melody_demo_vibrato, rhythm_demo_vibrato, 7, &INST_DEMO_VIBRATO_MEDIUM } };
static const Voice voices_demo_vibrato_deep[]   = { { VoiceType::MELODY, melody_demo_vibrato, rhythm_demo_vibrato, 7, &INST_DEMO_VIBRATO_DEEP } };

const Song song_demo_vibrato_light  = { voices_demo_vibrato_light,  1, 60, 68, "3.1 VIBRATO: LIGHT" };
const Song song_demo_vibrato_medium = { voices_demo_vibrato_medium, 1, 60, 68, "3.2 VIBRATO: MEDIUM" };
const Song song_demo_vibrato_deep   = { voices_demo_vibrato_deep,   1, 60, 68, "3.3 VIBRATO: DEEP+FAST" };

// =============================================================================
// CATEGORY 4 — ARPEGGIATOR SUB-SYSTEM (Rhythmic Movement)
// A held chord root; the arpeggiator internally alternates root <-> root+N
// semitones. Interval grows major 3rd -> perfect 5th -> octave, speed rises
// with it, so each entry gets audibly busier.
// =============================================================================

static const int8_t  melody_demo_arpeg[] = { 0, 0, 0, 0 };  // held root, arpeggiator does the rest
static const uint8_t rhythm_demo_arpeg[] = { 2, 2, 2, 2 };

static const Voice voices_demo_arpeg_m3[]  = { { VoiceType::MELODY, melody_demo_arpeg, rhythm_demo_arpeg, 4, &INST_DEMO_ARPEG_M3 } };
static const Voice voices_demo_arpeg_p5[]  = { { VoiceType::MELODY, melody_demo_arpeg, rhythm_demo_arpeg, 4, &INST_DEMO_ARPEG_P5 } };
static const Voice voices_demo_arpeg_oct[] = { { VoiceType::MELODY, melody_demo_arpeg, rhythm_demo_arpeg, 4, &INST_DEMO_ARPEG_OCT } };

const Song song_demo_arpeg_m3  = { voices_demo_arpeg_m3,  1, 60, 90, "4.1 ARPEGGIO: MAJ 3RD" };
const Song song_demo_arpeg_p5  = { voices_demo_arpeg_p5,  1, 60, 90, "4.2 ARPEGGIO: PERF 5TH" };
const Song song_demo_arpeg_oct = { voices_demo_arpeg_oct, 1, 60, 90, "4.3 ARPEGGIO: OCTAVE" };

// =============================================================================
// CATEGORY 5 — ANALOG-MODELED PERCUSSION SUITE (Rhythm Section)
// A single percussion voice cycling through every PercussionType, including
// the CLAP (staggered noise bursts) and the LFSR-driven noise voices
// (SNARE/HIHATs/CRASH/CLAP all use white_noise() internally).
// =============================================================================
static const int8_t percussion_demo_suite[] = {
  (int8_t)PercussionType::KICK,
  (int8_t)PercussionType::SNARE,
  (int8_t)PercussionType::HIHAT_CLOSED,
  (int8_t)PercussionType::HIHAT_OPEN,
  (int8_t)PercussionType::CLAP,
  (int8_t)PercussionType::TOM_LOW,
  (int8_t)PercussionType::TOM_MID,
  (int8_t)PercussionType::TOM_HIGH,
  (int8_t)PercussionType::CRASH
};
static const uint8_t rhythm_demo_suite[] = { 4, 4, 4, 4, 4, 4, 4, 4, 4 };

static const Voice voices_demo_percussion[] = {
  { VoiceType::PERCUSSION, percussion_demo_suite, rhythm_demo_suite, 9, nullptr }
};

const Song song_demo_percussion = { voices_demo_percussion, 1, 60, 100, "5.1 PERCUSSION SUITE" };

// =============================================================================
// CATEGORY 6 — SOFT-KNEE LIMITER & VOICE NORMALIZATION (Headroom Management)
// 3 melodic voices at full instrument volume (1.0f) stacked as a chord, plus
// a percussion voice, all sustaining together so the combined pre-clip
// signal repeatedly overshoots +/-1.0 and must lean on softClip()/
// ACTIVE_VOICE_NORMALIZATION to stay clean instead of crunching.
// =============================================================================

static const int8_t  melody_demo_limiter_root[]  = { 0, 0, 0, 0 };
static const int8_t  melody_demo_limiter_third[] = { 4, 4, 4, 4 };
static const int8_t  melody_demo_limiter_fifth[] = { 7, 7, 7, 7 };
static const uint8_t rhythm_demo_limiter[]       = { 2, 2, 2, 2 };
static const int8_t  percussion_demo_limiter[]   = {
  (int8_t)PercussionType::CRASH, (int8_t)PercussionType::KICK,
  (int8_t)PercussionType::CRASH, (int8_t)PercussionType::KICK
};

static const Voice voices_demo_limiter[] = {
  { VoiceType::MELODY,    melody_demo_limiter_root,  rhythm_demo_limiter, 4, &INST_DEMO_LIMITER_ROOT },
  { VoiceType::MELODY,    melody_demo_limiter_third, rhythm_demo_limiter, 4, &INST_DEMO_LIMITER_THIRD },
  { VoiceType::MELODY,    melody_demo_limiter_fifth, rhythm_demo_limiter, 4, &INST_DEMO_LIMITER_FIFTH },
  { VoiceType::PERCUSSION, percussion_demo_limiter,  rhythm_demo_limiter, 4, nullptr }
};

const Song song_demo_limiter = { voices_demo_limiter, 4, 60, 100, "6.1 LIMITER: MAX CHORD" };

// =============================================================================
// CATEGORY 7 — CIRCULAR DELAY & FEEDBACK ECHO (Spatial Processing)
// Rhythmic pluck arpeggio through the circular delay line with filtered feedback.
// Each pluck generates clearly audible slapback/echo reflections between notes.
// =============================================================================

static const int8_t  melody_demo_delay[] = { 0, 7, 12, 14, 12, 7 };
static const uint8_t rhythm_demo_delay[] = { 8, 8,  8,  8,  8, 8 }; // 6 eighths @ 120 BPM = 1.50s

static const Voice voices_demo_delay[] = {
  { VoiceType::MELODY, melody_demo_delay, rhythm_demo_delay, 6, &INST_DEMO_DELAY_PLUCK }
};

const Song song_demo_delay = { voices_demo_delay, 1, 60, 120, "7.1 DELAY: ECHO TAIL" };

// =============================================================================
// CATEGORY 8 — FULL MASTER POLYPHONY & AM RF BROADCASTING (System Integration)
// Lead (arpeggio+vibrato) + harmony + bass + full drum pattern, all voices
// active simultaneously through the delay/limiter chain. RF/output routing
// follows the Settings menu's g_output_mode, exactly like any normal song.
// =============================================================================

static const int8_t  melody_demo_full_lead[]    = { 0, 4, 7, 12, 7, 4, 0, 4 };
static const int8_t  melody_demo_full_harmony[] = { 7, 11, 14, 11, 12, 11, 7, 11 };
static const int8_t  melody_demo_full_bass[]    = { -12, -12, -5, -5, -7, -7, -12, -12 };
static const uint8_t rhythm_demo_full[]         = { 4, 4, 4, 4, 4, 4, 4, 4 };
static const int8_t  percussion_demo_full[]     = {
  (int8_t)PercussionType::KICK, (int8_t)PercussionType::HIHAT_CLOSED,
  (int8_t)PercussionType::SNARE, (int8_t)PercussionType::HIHAT_CLOSED,
  (int8_t)PercussionType::KICK, (int8_t)PercussionType::HIHAT_CLOSED,
  (int8_t)PercussionType::SNARE, (int8_t)PercussionType::CRASH
};

static const Voice voices_demo_full[] = {
  { VoiceType::MELODY,    melody_demo_full_lead,    rhythm_demo_full, 8, &INST_DEMO_FULL_LEAD },
  { VoiceType::MELODY,    melody_demo_full_harmony, rhythm_demo_full, 8, &INST_DEMO_FULL_HARMONY },
  { VoiceType::MELODY,    melody_demo_full_bass,    rhythm_demo_full, 8, &INST_DEMO_FULL_BASS },
  { VoiceType::PERCUSSION, percussion_demo_full,    rhythm_demo_full, 8, nullptr }
};

const Song song_demo_full_mix = { voices_demo_full, 4, 60, 120, "8.1 FULL MIX + RF TX" };

// =============================================================================
// CATEGORY 9 — ACOUSTIC & ORCHESTRAL INSTRUMENTS (4-Voice Polyphony)
// Emulates recognizable acoustic instruments using multi-voice layering,
// tailored ADSR volume envelopes, authentic vibrato, and polyphonic chords.
// Duration for each demo is strictly between 1.0 and 2.0 seconds.
// =============================================================================

// -----------------------------------------------------------------------------
// 9.1 ACOUSTIC GUITAR (Spanish Classical Fingerpicking — 1.50s)
// Crisp nylon/steel string arpeggio (E minor chord) with resonant body decay
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_guitar_b[] = { -12, -12, -12, -12, -12, -12 }; // Low E2 root
static const int8_t  melody_demo_guitar_5[] = {  -5,  -5,  -5,  -5,  -5,  -5 }; // B2 (5th)
static const int8_t  melody_demo_guitar_3[] = {   3,   3,   3,   3,   3,   3 }; // G3 (minor 3rd)
static const int8_t  melody_demo_guitar_h[] = {   7,  12,  10,   7,   3,   0 }; // B3, E4, D4, B3, G3, E3 melody
static const uint8_t rhythm_demo_guitar[]   = {   8,   8,   8,   8,   8,   8 }; // 6 eighths @ 120 BPM = 1.50s

static const Voice voices_demo_guitar[] = {
  { VoiceType::MELODY, melody_demo_guitar_b, rhythm_demo_guitar, 6, &INST_GUITAR_BASS },
  { VoiceType::MELODY, melody_demo_guitar_5, rhythm_demo_guitar, 6, &INST_GUITAR_MID  },
  { VoiceType::MELODY, melody_demo_guitar_3, rhythm_demo_guitar, 6, &INST_GUITAR_MID  },
  { VoiceType::MELODY, melody_demo_guitar_h, rhythm_demo_guitar, 6, &INST_GUITAR_HIGH }
};
const Song song_demo_guitar = { voices_demo_guitar, 4, 52, 120, "9.1 GUITAR" }; // Tonic E3 (52)

// -----------------------------------------------------------------------------
// 9.2 TUBULAR BELLS (Orchestral Chimes — 1.50s)
// Brass mallet strikes with rich inharmonic modal overtones & long resonance
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_bell_fund[] = {   4,   0 }; // E5 -> C5
static const int8_t  melody_demo_bell_5th[]  = {  11,   7 }; // B5 -> G5 (overtone 5th)
static const int8_t  melody_demo_bell_oct[]  = {  16,  12 }; // E6 -> C6 (high chime)
static const int8_t  melody_demo_bell_sub[]  = {  -8, -12 }; // E4 -> C4 (low resonator)
static const uint8_t rhythm_demo_bell[]      = {   2,   2 }; // 2 half notes @ 80 BPM = 1.50s

static const Voice voices_demo_tubular_bells[] = {
  { VoiceType::MELODY, melody_demo_bell_fund, rhythm_demo_bell, 2, &INST_BELL_FUNDAMENTAL },
  { VoiceType::MELODY, melody_demo_bell_5th,  rhythm_demo_bell, 2, &INST_BELL_STRIKE_M5   },
  { VoiceType::MELODY, melody_demo_bell_oct,  rhythm_demo_bell, 2, &INST_BELL_HIGH_OCT    },
  { VoiceType::MELODY, melody_demo_bell_sub,  rhythm_demo_bell, 2, &INST_BELL_SUB         }
};
const Song song_demo_tubular_bells = { voices_demo_tubular_bells, 4, 72, 80, "9.2 TUBULAR BELLS" }; // Tonic C5 (72)

// -----------------------------------------------------------------------------
// 9.3 ACOUSTIC GRAND PIANO (Grand Piano Voicing — 1.64s)
// Dynamic hammer-strike transient, multi-string unison body, rich Cmaj chord
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_piano_b[] = { -12, -12,  -5 }; // C2 -> C2 -> G2
static const int8_t  melody_demo_piano_t[] = {   7,   7,   7 }; // G3 tenor
static const int8_t  melody_demo_piano_a[] = {  16,  16,  16 }; // E4 alto
static const int8_t  melody_demo_piano_s[] = {  23,  24,  19 }; // B4 (maj7) -> C5 -> G4
static const uint8_t rhythm_demo_piano[]   = {   4,   4,   4 }; // 3 quarters @ 110 BPM = 1.636s

static const Voice voices_demo_piano[] = {
  { VoiceType::MELODY, melody_demo_piano_b, rhythm_demo_piano, 3, &INST_PIANO_BASS   },
  { VoiceType::MELODY, melody_demo_piano_t, rhythm_demo_piano, 3, &INST_PIANO_CHORD  },
  { VoiceType::MELODY, melody_demo_piano_a, rhythm_demo_piano, 3, &INST_PIANO_CHORD  },
  { VoiceType::MELODY, melody_demo_piano_s, rhythm_demo_piano, 3, &INST_PIANO_TREBLE }
};
const Song song_demo_piano = { voices_demo_piano, 4, 48, 110, "9.3 PIANO" }; // Tonic C3 (48)

// -----------------------------------------------------------------------------
// 9.4 TRUMPET (Brass Fanfare Section — 1.61s)
// Crisp brass embouchure swell, rich sawtooth harmonics, heroic vibrato
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_tpt_lead[] = {   0,   4,   7,  12 }; // Bb3 -> D4 -> F4 -> Bb4
static const int8_t  melody_demo_tpt_2[]    = {   0,   0,   4,   7 }; // Bb3 -> Bb3 -> D4 -> F4
static const int8_t  melody_demo_tpt_3[]    = {  -5,  -5,   0,   4 }; // F3 -> F3 -> Bb3 -> D4
static const int8_t  melody_demo_horn[]     = { -12, -12,  -5,   0 }; // Bb2 -> Bb2 -> F3 -> Bb3
static const uint8_t rhythm_demo_trumpet[]  = {   8,   8,   8,   2 }; // 3 eighths + half note @ 130 BPM = 1.615s

static const Voice voices_demo_trumpet[] = {
  { VoiceType::MELODY, melody_demo_tpt_lead, rhythm_demo_trumpet, 4, &INST_TRUMPET_LEAD    },
  { VoiceType::MELODY, melody_demo_tpt_2,    rhythm_demo_trumpet, 4, &INST_TRUMPET_SECTION },
  { VoiceType::MELODY, melody_demo_tpt_3,    rhythm_demo_trumpet, 4, &INST_TRUMPET_SECTION },
  { VoiceType::MELODY, melody_demo_horn,     rhythm_demo_trumpet, 4, &INST_FRENCH_HORN     }
};
const Song song_demo_trumpet = { voices_demo_trumpet, 4, 58, 130, "9.4 TRUMPET" }; // Tonic Bb3 (58)

// -----------------------------------------------------------------------------
// 9.5 VIOLIN (Solo Violin & String Quartet — 1.50s)
// Expressive bowed string attack, warm resonance, lyrical violin vibrato
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_vln_solo[] = {   5,   7 }; // D5 -> E5 solo
static const int8_t  melody_demo_vln_2[]    = {   0,   2 }; // A4 -> B4
static const int8_t  melody_demo_viola[]    = {  -4,  -2 }; // F4 -> G4
static const int8_t  melody_demo_cello[]    = { -17, -19 }; // D3 -> C3
static const uint8_t rhythm_demo_violin[]   = {   2,   2 }; // 2 half notes @ 80 BPM = 1.50s

static const Voice voices_demo_violin[] = {
  { VoiceType::MELODY, melody_demo_vln_solo, rhythm_demo_violin, 2, &INST_VIOLIN_SOLO },
  { VoiceType::MELODY, melody_demo_vln_2,    rhythm_demo_violin, 2, &INST_VIOLA_WARM  },
  { VoiceType::MELODY, melody_demo_viola,    rhythm_demo_violin, 2, &INST_VIOLA_WARM  },
  { VoiceType::MELODY, melody_demo_cello,    rhythm_demo_violin, 2, &INST_CELLO_BODY  }
};
const Song song_demo_violin = { voices_demo_violin, 4, 69, 80, "9.5 VIOLIN" }; // Tonic A4 (69)

// -----------------------------------------------------------------------------
// 9.6 DOUBLE BASS (Acoustic Upright Walking Bass — 1.78s)
// Deep wooden soundboard resonance, pizzicato attack transient & sub-bass punch
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_bass_wood[]   = {   0,   3,   5,   6 }; // E2 -> G2 -> A2 -> Bb2 (walking jazz bass)
static const int8_t  melody_demo_bass_sub[]    = {   0,   3,   5,   6 }; // Sub body fundamental
static const int8_t  melody_demo_bass_attack[] = {   0,   3,   5,   6 }; // Pluck transient
static const int8_t  melody_demo_bass_oct[]    = { -12, -12,  -7,  -7 }; // E1 & A1 sub thump
static const uint8_t rhythm_demo_contrabass[]  = {   4,   4,   4,   4 }; // 4 quarters @ 135 BPM = 1.777s

static const Voice voices_demo_contrabass[] = {
  { VoiceType::MELODY, melody_demo_bass_wood,   rhythm_demo_contrabass, 4, &INST_BASS_PIZZ_WOOD },
  { VoiceType::MELODY, melody_demo_bass_sub,    rhythm_demo_contrabass, 4, &INST_BASS_SUB_BODY  },
  { VoiceType::MELODY, melody_demo_bass_attack, rhythm_demo_contrabass, 4, &INST_BASS_ATTACK    },
  { VoiceType::MELODY, melody_demo_bass_oct,    rhythm_demo_contrabass, 4, &INST_BASS_SUB_BODY  }
};
const Song song_demo_contrabass = { voices_demo_contrabass, 4, 40, 135, "9.6 DOUBLE BASS" }; // Tonic E2 (40)

// =============================================================================
// CATEGORY 10 — ADVANCED HARMONIC & REED INSTRUMENTS (Multi-Voice Layering)
// Showcases additive pipe organ registrations, percussive mallet marimba,
// baroque plectrum harpsichord, expressive woodwind flute consort, and
// dual-reed musette accordion. Duration strictly between 1.0 and 2.0 seconds.
// =============================================================================

// -----------------------------------------------------------------------------
// 10.1 PIPE ORGAN (Cathedral Church / Drawbar Organ — 1.50s)
// 4-voice additive harmonic synthesis: 16' pedal, 8' principal, 4' octave, 2 2/3' tierce
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_organ_16[] = { -12,  -7 }; // C3 -> F3
static const int8_t  melody_demo_organ_8[]  = {   0,   5 }; // C4 -> F4
static const int8_t  melody_demo_organ_4[]  = {   7,   9 }; // G4 -> A4
static const int8_t  melody_demo_organ_m[]  = {  16,  17 }; // E5 -> F5
static const uint8_t rhythm_demo_organ[]    = {   2,   2 }; // 2 half notes @ 80 BPM = 1.50s

static const Voice voices_demo_pipe_organ[] = {
  { VoiceType::MELODY, melody_demo_organ_16, rhythm_demo_organ, 2, &INST_ORGAN_16FT    },
  { VoiceType::MELODY, melody_demo_organ_8,  rhythm_demo_organ, 2, &INST_ORGAN_8FT     },
  { VoiceType::MELODY, melody_demo_organ_4,  rhythm_demo_organ, 2, &INST_ORGAN_4FT     },
  { VoiceType::MELODY, melody_demo_organ_m,  rhythm_demo_organ, 2, &INST_ORGAN_MIXTURE }
};
const Song song_demo_pipe_organ = { voices_demo_pipe_organ, 4, 60, 80, "10.1 PIPE ORGAN" }; // Tonic C4 (60)

// -----------------------------------------------------------------------------
// 10.2 MARIMBA (Wooden Mallet Percussion — 1.50s)
// Struck rosewood bars with rapid resonant decay and zero sustain (Am/C chord)
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_marimba_b[] = { -12, -12,  -9,  -9,  -5,  -5 }; // C3, A2, G2
static const int8_t  melody_demo_marimba_t[] = {   0,   0,   0,   0,  -1,  -1 }; // C4, B3
static const int8_t  melody_demo_marimba_a[] = {   4,   4,   4,   4,   2,   2 }; // E4, D4
static const int8_t  melody_demo_marimba_h[] = {   7,  12,   9,  16,  11,  14 }; // G4, C5, A4, E5, B4, D5
static const uint8_t rhythm_demo_marimba[]   = {   8,   8,   8,   8,   8,   8 }; // 6 eighths @ 120 BPM = 1.50s

static const Voice voices_demo_marimba[] = {
  { VoiceType::MELODY, melody_demo_marimba_b, rhythm_demo_marimba, 6, &INST_MARIMBA_LOW  },
  { VoiceType::MELODY, melody_demo_marimba_t, rhythm_demo_marimba, 6, &INST_MARIMBA_MID  },
  { VoiceType::MELODY, melody_demo_marimba_a, rhythm_demo_marimba, 6, &INST_MARIMBA_MID  },
  { VoiceType::MELODY, melody_demo_marimba_h, rhythm_demo_marimba, 6, &INST_MARIMBA_HIGH }
};
const Song song_demo_marimba = { voices_demo_marimba, 4, 60, 120, "10.2 MARIMBA" }; // Tonic C4 (60)

// -----------------------------------------------------------------------------
// 10.3 HARPSICHORD (Baroque Plucked String / Clavecin — 1.50s)
// Quill plectrum pluck transient with rich sawtooth harmonics and 4-voice counterpoint
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_harpsi_b[] = { -12, -12, -10,  -8,  -7, -12 }; // D3 -> D3 -> E3 -> F#3 -> G3 -> D3
static const int8_t  melody_demo_harpsi_t[] = {  -5,  -5,  -5,  -3,  -2,  -5 }; // G3 -> G3 -> G3 -> A3 -> B3 -> G3
static const int8_t  melody_demo_harpsi_a[] = {   0,   0,   2,   4,   5,   0 }; // D4 -> D4 -> E4 -> F#4 -> G4 -> D4
static const int8_t  melody_demo_harpsi_s[] = {  12,  14,  16,  17,  16,  12 }; // D5 -> E5 -> F#5 -> G5 -> F#5 -> D5
static const uint8_t rhythm_demo_harpsi[]   = {   8,   8,   8,   8,   8,   8 }; // 6 eighths @ 120 BPM = 1.50s

static const Voice voices_demo_harpsichord[] = {
  { VoiceType::MELODY, melody_demo_harpsi_b, rhythm_demo_harpsi, 6, &INST_HARPSICHORD_BASS },
  { VoiceType::MELODY, melody_demo_harpsi_t, rhythm_demo_harpsi, 6, &INST_HARPSICHORD_MID  },
  { VoiceType::MELODY, melody_demo_harpsi_a, rhythm_demo_harpsi, 6, &INST_HARPSICHORD_MID  },
  { VoiceType::MELODY, melody_demo_harpsi_s, rhythm_demo_harpsi, 6, &INST_HARPSICHORD_TREB }
};
const Song song_demo_harpsichord = { voices_demo_harpsichord, 4, 62, 120, "10.3 HARPSICHORD" }; // Tonic D4 (62)

// -----------------------------------------------------------------------------
// 10.4 WOODWIND FLUTE (Renaissance Flute Consort — 1.50s)
// Air-column excitation, progressive breath attack, and lyrical diaphragmatic vibrato
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_flute_s[] = {   7,   5 }; // D5 -> C5 solo
static const int8_t  melody_demo_flute_a[] = {   4,   0 }; // B4 -> G4
static const int8_t  melody_demo_flute_t[] = {   0,  -3 }; // G4 -> E4
static const int8_t  melody_demo_flute_b[] = { -12,  -7 }; // G3 -> C4
static const uint8_t rhythm_demo_flute[]   = {   2,   2 }; // 2 half notes @ 80 BPM = 1.50s

static const Voice voices_demo_flute[] = {
  { VoiceType::MELODY, melody_demo_flute_s, rhythm_demo_flute, 2, &INST_FLUTE_SOLO  },
  { VoiceType::MELODY, melody_demo_flute_a, rhythm_demo_flute, 2, &INST_FLUTE_SOLO  },
  { VoiceType::MELODY, melody_demo_flute_t, rhythm_demo_flute, 2, &INST_FLUTE_TENOR },
  { VoiceType::MELODY, melody_demo_flute_b, rhythm_demo_flute, 2, &INST_FLUTE_BASS  }
};
const Song song_demo_flute = { voices_demo_flute, 4, 67, 80, "10.4 FLUTE CONSORT" }; // Tonic G4 (67)

// -----------------------------------------------------------------------------
// 10.5 ACCORDION (French Musette Accordion / Bandoneón — 1.50s)
// Free-reed dual reeds with authentic musette beating, bellows attack, and bass buttons
// -----------------------------------------------------------------------------

static const int8_t  melody_demo_acc_lead[]  = {  12,  12,  11,   9,   8,  12 }; // A4, G#4, F#4, F4, A4 melody
static const int8_t  melody_demo_acc_reed2[] = {   3,   3,   3,   3,   3,   3 }; // C4 accompaniment reed
static const int8_t  melody_demo_acc_fifth[] = {   7,   7,   7,   7,   7,   7 }; // E4 accompaniment reed
static const int8_t  melody_demo_acc_bass[]  = { -12, -12,  -5,  -5, -12, -12 }; // A2 -> E3 bellows bass
static const uint8_t rhythm_demo_acc[]       = {   8,   8,   8,   8,   8,   8 }; // 6 eighths @ 120 BPM = 1.50s

static const Voice voices_demo_accordion[] = {
  { VoiceType::MELODY, melody_demo_acc_lead,  rhythm_demo_acc, 6, &INST_ACCORDION_LEAD  },
  { VoiceType::MELODY, melody_demo_acc_reed2, rhythm_demo_acc, 6, &INST_ACCORDION_REED2 },
  { VoiceType::MELODY, melody_demo_acc_fifth, rhythm_demo_acc, 6, &INST_ACCORDION_REED2 },
  { VoiceType::MELODY, melody_demo_acc_bass,  rhythm_demo_acc, 6, &INST_ACCORDION_BASS  }
};
const Song song_demo_accordion = { voices_demo_accordion, 4, 57, 120, "10.5 ACCORDION" }; // Tonic A3 (57)

// =============================================================================
// DEMO PLAYLIST — this is what the rotary encoder steps through
// =============================================================================
const Song DEMO_SONGS[] = {
  song_demo_wave_sine,
  song_demo_wave_triangle,
  song_demo_wave_square,
  song_demo_wave_saw,
  song_demo_wave_brighttri,
  song_demo_wave_warmtri,
  song_demo_adsr_staccato,
  song_demo_adsr_longtail,
  song_demo_vibrato_light,
  song_demo_vibrato_medium,
  song_demo_vibrato_deep,
  song_demo_arpeg_m3,
  song_demo_arpeg_p5,
  song_demo_arpeg_oct,
  song_demo_percussion,
  song_demo_limiter,
  song_demo_delay,
  song_demo_full_mix,
  song_demo_guitar,
  song_demo_tubular_bells,
  song_demo_piano,
  song_demo_trumpet,
  song_demo_violin,
  song_demo_contrabass,
  song_demo_pipe_organ,
  song_demo_marimba,
  song_demo_harpsichord,
  song_demo_flute,
  song_demo_accordion
};
const int TOTAL_DEMO_SONGS = sizeof(DEMO_SONGS) / sizeof(DEMO_SONGS[0]);