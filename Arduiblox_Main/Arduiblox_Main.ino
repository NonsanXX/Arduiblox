#include "LiquidCrystal_I2C.h"
#include "pitches.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

int buttons[4] = { 2, 3, 4, 5 };
int leds[4] = { 8, 9, 10, 11 };

#define buzzer 6
#define maxLevel 1

int simonSequence[100];
int currentLevel = 1;
int gameState = 0;  // 0: Start, 1: Simon's turn, 2: Player's turn, 3: Game over

void setup() {
  Serial.begin(9600);
  Serial.println("Game Init!");

  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    pinMode(leds[i], OUTPUT);
  }

  pinMode(buzzer, OUTPUT);

  lcd.init();
  lcd.backlight();
  displayWelcomeMessage();

  randomSeed(analogRead(0));
}

void loop() {
  switch (gameState) {
    case 0:  // Start game
      waitForStart();
      break;
    case 1:  // Simon's turn
      playSimonSequence();
      break;
    case 2:  // Player's turn
      getPlayerInput();
      break;
    case 3:  // Game over
      displayGameOver();
      break;
  }
}

void displayWelcomeMessage() {
  lcd.setCursor(0, 0);
  lcd.print("   Welcome To   ");
  lcd.setCursor(0, 1);
  lcd.print(">  Arduiblox  <");
  delay(2000);
  lcd.clear();
}

void waitForStart() {
  lcd.setCursor(0, 0);
  lcd.print("Press Red Button");
  lcd.setCursor(0, 1);
  lcd.print(" to Start Game  ");

  if (digitalRead(buttons[0]) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(buttons[0]) == LOW) {
      currentLevel = 1;
      gameState = 1;
      lcd.clear();
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

  for (int i = 0; i < currentLevel; i++) {
    int led = leds[simonSequence[i]];
    digitalWrite(led, HIGH);
    playBuzzer(simonSequence[i] + 1);
    digitalWrite(led, LOW);
    delay(400);
  }

  gameState = 2;
  lcd.setCursor(0, 1);
  lcd.print("   -- Play --   ");
}

void getPlayerInput() {
  static int playerStep = 0;

  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      delay(50);  // Debounce
      if (digitalRead(buttons[i]) == LOW) {
        digitalWrite(leds[i], HIGH);
        playBuzzer(i + 1);

        if (i != simonSequence[playerStep]) {
          gameState = 3;  // Game over
          return;
        }
        playerStep++;

        if (playerStep == currentLevel) {
          digitalWrite(leds[i], LOW);
          playerStep = 0;
          currentLevel++;
          if (currentLevel > maxLevel) {
            displayVictory();
            gameState = 0;
          } else {
            gameState = 1;  // Next level
          }
          return;
        }

        while (digitalRead(buttons[i]) == LOW)
          ;
        digitalWrite(leds[i], LOW);
      }
    }
  }
}

void displayGameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" !! You Lost !! ");
  lcd.setCursor(0, 1);
  lcd.print("!! GAME  OVER !!");

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
}

void displayVictory() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Congratulations!");
  lcd.setCursor(0, 1);
  lcd.print("You Win!");

  for (int i = 0; i < 3; i++) {
    tone(buzzer, 1000);
    delay(200);
    tone(buzzer, 1500);
    delay(200);
  }
  playSong();
  noTone(buzzer);
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