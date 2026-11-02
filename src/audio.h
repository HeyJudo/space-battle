// audio.h - sound effects and background music.
// Sound only starts on the first click or key press (see main.cpp), so
// nothing blasts the moment the window opens.
#pragma once

enum class Sfx { Click, Laser, EnemyLaser, Hit, Deflect, Shield, Repair, Alarm, Explosion, Victory, Defeat, COUNT };

void StartAudio();     // call once, on the first click, tap or key press
bool AudioStarted();
void PlaySfx(Sfx sound);
void UpdateAudio();    // keeps the music going; follows the Settings toggles
void StopAudio();
