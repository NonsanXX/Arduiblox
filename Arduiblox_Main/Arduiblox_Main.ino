#include "LiquidCrystal_I2C.h"
#include "SimonGame.h"
// Include other game headers here as you add more games

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define buzzer 6
#define BUTTON_UP 3
#define BUTTON_DOWN 4
#define BUTTON_SELECT 2

int currentSelection = 0;
int numGames = 1; // Increase this as you add more games

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  displayWelcomeMessage();
  displayMenu();
}

void loop() {
  if (digitalRead(BUTTON_UP) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_UP) == LOW) {
      currentSelection = (currentSelection - 1 + numGames) % numGames;
      displayMenu();
      while (digitalRead(BUTTON_UP) == LOW);
    }
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_DOWN) == LOW) {
      currentSelection = (currentSelection + 1) % numGames;
      displayMenu();
      while (digitalRead(BUTTON_DOWN) == LOW);
    }
  }

  if (digitalRead(BUTTON_SELECT) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_SELECT) == LOW) {
      launchGame(currentSelection);
      while (digitalRead(BUTTON_SELECT) == LOW);
    }
  }
}

void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select a game:");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(getGameName(currentSelection));
}

String getGameName(int index) {
  switch (index) {
    case 0:
      return "Simon";
    // Add more cases as you add games
    default:
      return "Unknown";
  }
}

void launchGame(int index) {
  switch (index) {
    case 0:
      if (playSimonGame()) {
        // Game completed normally
        lcd.clear();
        delay(2000);
      }
      break;
    // Add more cases as you add games
    default:
      break;
  }
  displayMenu(); // Return to menu after game ends or is exited
}

void displayWelcomeMessage() {
  lcd.setCursor(0, 0);
  lcd.print("   Welcome To   ");
  lcd.setCursor(0, 1);
  lcd.print(">  Arduiblox  <");
  delay(2000);
  lcd.clear();
  tone(buzzer, 1000);
  delay(300);
  noTone(buzzer);
}