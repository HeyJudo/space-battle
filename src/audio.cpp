// audio.cpp - Kenney sound effects (CC0) and a low background drone.
// The drone is not a file: it is made here, sample by sample, from three
// slightly detuned tones passed through a slowly opening and closing filter.
#include "audio.h"
#include "raylib.h"
#include "game.h"
#include <cmath>

static bool started = false;
static Sound sounds[(int)Sfx::COUNT];

// One file per sound effect, in the same order as the Sfx list in audio.h.
static const char* SOUND_FILES[(int)Sfx::COUNT] = {
    "assets/sounds/click.ogg",
    "assets/sounds/laser.ogg",
    "assets/sounds/enemy_laser.ogg",
    "assets/sounds/hit.ogg",
    "assets/sounds/deflect.ogg",
    "assets/sounds/shield.ogg",
    "assets/sounds/repair.ogg",
    "assets/sounds/alarm.ogg",
    "assets/sounds/explosion.ogg",
    "assets/sounds/victory.ogg",
    "assets/sounds/defeat.ogg",
};

// ---------- The music drone ----------

const int DRONE_RATE = 22050;    // samples per second
const int DRONE_CHUNK = 4096;    // samples made at a time
const float DRONE_VOLUME = 0.22f;

static AudioStream drone;        // a stream we keep filling with new samples
static short droneBuffer[DRONE_CHUNK];
static double phaseA = 0, phaseB = 0, phaseC = 0, lfoPhase = 0;
static float filtered = 0;

// Fills the buffer with the next stretch of drone.
static void MakeDroneSamples() {
    const double ROOT = 49.0;  // a low G
    for (int i = 0; i < DRONE_CHUNK; i++) {
        // Phases go from 0 to 1 and wrap around, one wrap per wave cycle.
        phaseA = fmod(phaseA + ROOT / DRONE_RATE, 1.0);
        phaseB = fmod(phaseB + ROOT * 1.008 / DRONE_RATE, 1.0);
        phaseC = fmod(phaseC + ROOT * 1.5 / DRONE_RATE, 1.0);
        lfoPhase = fmod(lfoPhase + 0.1 / DRONE_RATE, 1.0);

        float sawA = (float)(2 * phaseA - 1);
        float sawB = (float)(2 * phaseB - 1);
        float triangle = (float)(4 * fabs(phaseC - 0.5) - 1);
        float mix = (sawA + sawB + triangle) / 3;

        // A simple low-pass filter: follow the signal slowly to soften its buzz.
        // The cutoff sweeps between about 210 and 510 Hz every 10 seconds.
        float cutoff = 360 + 150 * sinf((float)(2 * PI * lfoPhase));
        float k = 1 - expf(-2 * PI * cutoff / DRONE_RATE);
        filtered += (mix - filtered) * k;

        droneBuffer[i] = (short)(filtered * DRONE_VOLUME * 32767);
    }
}

// ---------- Public functions ----------

void StartAudio() {
    InitAudioDevice();
    for (int i = 0; i < (int)Sfx::COUNT; i++) {
        sounds[i] = LoadSound(SOUND_FILES[i]);
        SetSoundVolume(sounds[i], 0.55f);
    }
    SetSoundVolume(sounds[(int)Sfx::Click], 0.4f);
    SetSoundVolume(sounds[(int)Sfx::Alarm], 0.35f);

    SetAudioStreamBufferSizeDefault(DRONE_CHUNK);
    drone = LoadAudioStream(DRONE_RATE, 16, 1);  // 16-bit samples, one channel (mono)
    PlayAudioStream(drone);
    started = true;
}

bool AudioStarted() {
    return started;
}

void PlaySfx(Sfx sound) {
    if (started && settings.soundEffects)
        PlaySound(sounds[(int)sound]);
}

void UpdateAudio() {
    if (!started)
        return;
    // Music on: top up the stream whenever it has used a chunk. Music off: pause it.
    if (settings.music) {
        if (!IsAudioStreamPlaying(drone))
            ResumeAudioStream(drone);
        while (IsAudioStreamProcessed(drone)) {
            MakeDroneSamples();
            UpdateAudioStream(drone, droneBuffer, DRONE_CHUNK);
        }
    } else if (IsAudioStreamPlaying(drone)) {
        PauseAudioStream(drone);
    }
}

void StopAudio() {
    if (!started)
        return;
    for (int i = 0; i < (int)Sfx::COUNT; i++)
        UnloadSound(sounds[i]);
    UnloadAudioStream(drone);
    CloseAudioDevice();
    started = false;
}
