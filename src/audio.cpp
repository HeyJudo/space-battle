// audio.cpp - placeholder until the sound files are added.
#include "audio.h"

static bool started = false;

void StartAudio() { started = true; }
bool AudioStarted() { return started; }
void PlaySfx(Sfx) {}
void UpdateAudio() {}
void StopAudio() {}
