// audio.h - sound effects and background music.
// Browsers block sound until the player clicks or presses a key, so the
// audio device only starts on the first input (see main.cpp).
#pragma once

enum class Sfx { Click, Laser, EnemyLaser, Hit, Deflect, Shield, Repair, Alarm, Explosion, Victory, Defeat, COUNT };

void StartAudio();     // call once, on the first click, tap or key press
bool AudioStarted();
void PlaySfx(Sfx sound);
void UpdateAudio();    // keeps the music going; follows the Settings toggles
void StopAudio();
