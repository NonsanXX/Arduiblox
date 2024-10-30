// SimonGame.ino
#include "SimonGame.h"
#include "pitches.h"

extern LiquidCrystal_I2C lcd;

int buttons[4] = { 2, 3, 4, 5 };
int leds[4] = { 8, 9, 10, 11 };

#define maxLevel 10

int simonSequence[100];
int currentLevel = 1;
int gameState = 0;  // 0: Start, 1: Simon's turn, 2: Player's turn, 3: Game over

// New variables for improved button handling
unsigned long lastDebounceTime[4] = { 0, 0, 0, 0 };
unsigned long debounceDelay = 50;
bool buttonState[4] = { HIGH, HIGH, HIGH, HIGH };
bool lastButtonState[4] = { HIGH, HIGH, HIGH, HIGH };

bool playSimonGame() {
  // Initialize game-specific variables
  currentLevel = 1;
  gameState = 0;

  // Set up game-specific pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    pinMode(leds[i], OUTPUT);
  }
  pinMode(buzzer, OUTPUT);

  displayWelcomeSimon();

  while (true) { // Continue until game over

    switch (gameState) {
      case 0: // Start game
        if (!waitForStart()) {
          return false; // Return to main menu
        }
        break;
      case 1: // Simon's turn
        playSimonSequence();
        break;
      case 2: // Player's turn
        getPlayerInput();
        break;
      case 3: // Game Over
        displayGameOver();
        break;
    }
  }
  return true; // Game completed, return to main menu
}

void displayWelcomeSimon() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Simon Game   ");
  lcd.setCursor(0, 1);
  lcd.print(" R:Start Y:Back ");
  playEnterSound();
}

bool waitForStart() {
  while (true) {
    HandleMqtt();

    if (digitalRead(buttons[0]) == LOW) { // Red button to start
      delay(50);  // Debounce
      if (digitalRead(buttons[0]) == LOW) {
        currentLevel = 1;
        gameState = 1;
        playEnterSound();
        Serial.println("Game Started!");
        lcd.clear();
        return true;
      }
    }
    if (digitalRead(buttons[1]) == LOW) { // Blue button to go back
      delay(50);  // Debounce
      if (digitalRead(buttons[1]) == LOW) {
        playBackSound();
        Serial.println("Returning back to main menu");
        return false;
      }
    }
  }
}

void playSimonSequence() {
  lcd.setCursor(4, 0);
  lcd.print("Level: ");
  lcd.print(currentLevel);
  lcd.setCursor(0, 1);
  lcd.print(" -- Memorize -- ");
  delay(1500);

  simonSequence[currentLevel - 1] = random(0, 4);
  Serial.print("Level: ");
  Serial.println(currentLevel);
  Serial.print("[");
  for (int i = 0; i < currentLevel; i++) {
    Serial.print(" ");
    Serial.print(simonSequence[i] + 1);
    int led = leds[simonSequence[i]];
    digitalWrite(led, HIGH);
    playBuzzer(simonSequence[i] + 1);
    digitalWrite(led, LOW);
    delay(400);
  }
  Serial.println(" ]");
  Serial.println("Your Input :");
  Serial.print("[");

  gameState = 2;
  lcd.setCursor(0, 1);
  lcd.print("   -- Play --   ");
}

void getPlayerInput() {
  static int playerStep = 0;

  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttons[i]);

    if (reading != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != buttonState[i]) {
        buttonState[i] = reading;

        if (buttonState[i] == LOW) {
          Serial.print(" ");
          Serial.print(i + 1);
          digitalWrite(leds[i], HIGH);
          playBuzzer(i + 1);

          if (i != simonSequence[playerStep]) {
            Serial.println(" ]");
            sendScoreToMQTT(currentLevel-1);

            gameState = 3;  // Game over
            playerStep = 0;
            return;
          }

          playerStep++;

          if (playerStep == currentLevel) {
            Serial.println(" ]");
            digitalWrite(leds[i], LOW);
            playerStep = 0;
            currentLevel++;
            Serial.println("PASSED!");
            if (currentLevel > maxLevel) {
              displayVictory();
              gameState = 0;
              displayWelcomeSimon();
            } else {
              gameState = 1;  // Next level
            }
            return;
          }
        } else {
          digitalWrite(leds[i], LOW);
        }
      }
    }

    lastButtonState[i] = reading;
  }
}

void sendScoreToMQTT(int val){
  char buffer[10]; // Adjust buffer size as needed
  sprintf(buffer, "%d", val);
  mqttClient.publish("arduiblox/simongame", buffer);
}

void displayGameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" !! You Lost !! ");
  lcd.setCursor(0, 1);
  lcd.print("!! GAME  OVER !!");
  Serial.println("!! GAME  OVER !!");

  tone(buzzer, 350);
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
  }
  delay(1000);
  noTone(buzzer);
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], LOW);
  }
  delay(1000);

  gameState = 0;  // Back to start
  displayWelcomeSimon();
}

void displayVictory() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Congratulations!");
  lcd.setCursor(0, 1);
  lcd.print("    You Win!    ");

  for (int i = 0; i < 3; i++) {
    tone(buzzer, 1000);
    delay(200);
    tone(buzzer, 1500);
    delay(200);
  }
  playSong();
  delay(2000);
}

void playBuzzer(int x) {
  tone(buzzer, 650 + (x * 100));
  delay(300);
  noTone(buzzer);
}

void playSong() {
  int melody[] = {
    NOTE_C4,
    REST,
    NOTE_G4,
    REST,
    NOTE_AS4,
    NOTE_C5,
    NOTE_AS4,
    REST,
    NOTE_F4,
    NOTE_DS4,
    REST,
    NOTE_C4,
    REST,
    NOTE_G4,
    REST,
    NOTE_AS4,
    NOTE_C5,
    NOTE_AS4,
    REST,
    NOTE_F4,
    NOTE_DS4,
    REST,
    NOTE_C4,
    REST,
    NOTE_G4,
    REST,
    NOTE_AS4,
    NOTE_C5,
    NOTE_AS4,
    REST,
    NOTE_F4,
    NOTE_DS4,
    REST,

    NOTE_C4,
    REST,
    NOTE_E4,
    REST,
    NOTE_G4,
    NOTE_A4,
    NOTE_AS4,
    NOTE_C5,
    REST,
    NOTE_C5,
    REST,
    NOTE_AS4,
    REST,
    NOTE_A4,
    REST,
    NOTE_AS4,
    REST,
    NOTE_AS4,
    NOTE_C5,
    REST,
    NOTE_AS4,
    NOTE_A4,
    REST,
    REST,
    NOTE_C5,
    REST,
    NOTE_AS4,
    REST,
    NOTE_A4,
    REST,
    NOTE_AS4,
    REST,
    NOTE_E5,
    REST,

    NOTE_C5,
    REST,
    NOTE_C5,
    REST,
    NOTE_AS4,
    REST,
    NOTE_A4,
    REST,
    NOTE_AS4,
    REST,
    NOTE_AS4,
    NOTE_C5,
    REST,
    NOTE_AS4,
    NOTE_A4,
    REST,
    REST,
    NOTE_C5,
    REST,
    NOTE_AS4,
    REST,
    NOTE_A4,
    REST,
    NOTE_AS4,
    REST,
    NOTE_E4,
    REST,
  };

  int durations[] = {
    4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,
    4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,
    4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,

    4, 8, 4, 8, 4, 4, 4,
    8, 16, 8, 16, 8, 16, 8, 16,
    8, 16, 8, 8, 16, 8, 8, 16,
    4,
    8, 16, 8, 16, 8, 16, 8, 4, 8,
    4,

    8, 16, 8, 16, 8, 16, 8, 16,
    8, 16, 8, 8, 16, 8, 8, 16,
    4,
    8, 16, 8, 16, 8, 16, 8, 4, 8,
    1
  };


  int size = sizeof(durations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 1000 / durations[note];
    tone(buzzer, melody[note], duration);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);

    //stop the tone playing:
    noTone(buzzer);
  }
}

void playEnterSound(){
      tone(buzzer, 1000);
      delay(100);
      noTone(buzzer);
}

void playBackSound(){
      tone(buzzer, 500);
      delay(100);
      noTone(buzzer);
}