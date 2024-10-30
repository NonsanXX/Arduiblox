#ifndef PIANO_GAME_H
#define PIANO_GAME_H

#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "pitches.h"

// Function declarations
bool playPianoGame();
void displayPianoStatus(const char* line1, const char* line2);
void updatePianoDisplay(int frequency, bool isUpperRange);
const char* getNoteString(int frequency);

// External LCD declaration
extern LiquidCrystal_I2C lcd;

#endif