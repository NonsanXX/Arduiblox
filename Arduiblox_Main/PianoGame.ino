#include "PianoGame.h"
#include "pitches.h"

const int PIANO_POT_PIN = A3;

// Define base frequencies for both ranges
const int lowerRangeFreqs[] = {
  349,  // F4
  330,  // E4
  294,  // D4
  262   // C4
};

const int upperRangeFreqs[] = {
  554,  // C#5
  494,  // B4
  440,  // A4
  392   // G4
};

const int FREQ_ADJUSTMENT_RANGE = 50;

bool playPianoGame() {
  unsigned long previousMillis = 0;             // For potentiometer reading and tone updates
  unsigned long ledOnTime[4] = { 0, 0, 0, 0 };  // Stores the time each LED was turned on
  const int ledDuration = 200;                  // Duration for which each LED should stay on

  displayPianoStatus("Piano Mode", "R:Play R&Y:Back");
  // Set up game-specific pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    pinMode(leds[i], OUTPUT);
  }
  pinMode(buzzer, OUTPUT);

  while (true) {
    // Check for exit condition (two buttons)
    if (digitalRead(buttons[0]) == LOW && digitalRead(buttons[1]) == LOW) {
      delay(50);  // Debounce
      if (digitalRead(buttons[0]) == LOW && digitalRead(buttons[1]) == LOW) {
        playBackSound();
        break;  // Exit to main menu
      }
    }

    unsigned long currentMillis = millis();  // Current time in milliseconds
    int potValue = analogRead(PIANO_POT_PIN);
    bool isUpperRange = potValue > 512;

    // Calculate frequency adjustment based on potentiometer
    int frequencyAdjustment;
    if (isUpperRange) {
      frequencyAdjustment = map(potValue, 512, 1023, 0, FREQ_ADJUSTMENT_RANGE);
    } else {
      frequencyAdjustment = map(potValue, 0, 511, 0, FREQ_ADJUSTMENT_RANGE);
    }

    bool buttonPressed = false;
    int frequencyToPlay = 0;

    // Check each button and apply the appropriate base frequency
    for (int i = 3; i >= 0; i--) {
      if (digitalRead(buttons[i]) == LOW) {
        if (isUpperRange) {
          frequencyToPlay = upperRangeFreqs[i] + frequencyAdjustment;
        } else {
          frequencyToPlay = lowerRangeFreqs[i] + frequencyAdjustment;
        }
        buttonPressed = true;

        // Update display with current note and frequency
        updatePianoDisplay(frequencyToPlay, isUpperRange);

        // Turn on the LED without delay
        digitalWrite(leds[i], HIGH);
        ledOnTime[i] = currentMillis;  // Record the time this LED was turned on
        break;
      }
    }

    // Turn off LEDs after their duration has passed
    for (int i = 0; i < 4; i++) {
      if (digitalRead(leds[i]) == HIGH && (currentMillis - ledOnTime[i] >= ledDuration)) {
        digitalWrite(leds[i], LOW);  // Turn off the LED after 500 ms
      }
    }

    if (buttonPressed) {
      tone(buzzer, frequencyToPlay);  // Play the tone at the calculated frequency

      // Debug output
      Serial.print("Range: ");
      Serial.print(isUpperRange ? "Upper" : "Lower");
      Serial.print(" Frequency: ");
      Serial.print(frequencyToPlay);
      Serial.print(" Adjustment: ");
      Serial.println(frequencyAdjustment);
    } else {
      noTone(buzzer);  // Stop the tone if no button is pressed
    }
  }

  // After exiting the loop, turn on all LEDs (optional behavior)
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
  }
  return true;  // Game completed, return to main menu
}


void displayPianoStatus(const char* line1, const char* line2) {
  playSelectSound();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void updatePianoDisplay(int frequency, bool isUpperRange) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Note: ");
  lcd.print(getNoteString(frequency));

  lcd.setCursor(0, 1);
  lcd.print("Freq: ");
  lcd.print(frequency);
  lcd.print(" Hz");
}

const char* getNoteString(int frequency) {
  // Round frequency to nearest note
  if (frequency > 530) return "C#5";
  else if (frequency > 480) return "B4";
  else if (frequency > 420) return "A4";
  else if (frequency > 370) return "G4";
  else if (frequency > 340) return "F4";
  else if (frequency > 310) return "E4";
  else if (frequency > 280) return "D4";
  else return "C4";
}