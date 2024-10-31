// SimonGame.ino
#include "SimonGame.h"
#include "pitches.h"

extern LiquidCrystal_I2C lcd;

int leds[4] = { 8, 9, 10, 11 };

#define maxLevel 100

int simonSequence[100];
int currentLevel = 1;
int gameState = 0;  // 0: Start, 1: Simon's turn, 2: Player's turn, 3: Game over

String playerName = "";
String morseCode = "";  // Temporary storage for current letter in Morse
String finalMorseSequence = "";  // Store the entire Morse code sequence

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
      // case 3: // Game Over
      //   displayGameOver();
      //   break;
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
  playSelectSound();
}

bool waitForStart() {
  while (true) {
    if (!mqttClient.connected())
    {
        ConnectMqtt();
        displayWelcomeSimon();
    }
    mqttClient.loop();

    if (digitalRead(buttons[0]) == LOW) { // Red button to start
      delay(50);  // Debounce
      if (digitalRead(buttons[0]) == LOW) {
        currentLevel = 1;
        gameState = 1;
        playSelectSound();
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
            gameEnded(0, currentLevel-1);

            //gameState = 3;  // Game over
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
              gameEnded(1, maxLevel);
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

void gameEnded(int status, int score){
  if (status == 0){
    displayGameOver();
  } else if (status = 1){
    displayVictory();
  }
  lcd.clear();
  lcdCenterPrintScore("Your score :", score);
  delay(2000);

  if(score > 0 && askToSaveScore()){
    lcdCenterPrintTR("Enter your name", "R:. Y:- G:SPACE");
    delay(100);
    sendScoreToMQTT(getPlayerName(), score);
  }
  gameState = 0;  // Back to start
  displayWelcomeSimon();
}

bool askToSaveScore(){
  lcdCenterPrintTR("Save score?", "R:YES Y:NO");
  while (true) {
    if (digitalRead(buttons[0]) == LOW) { 
      delay(50);  // Debounce
      if (digitalRead(buttons[0]) == LOW) {
        lcd.clear();
        playSelectSound();
        return true;
      }
    }
    if (digitalRead(buttons[1]) == LOW) { 
      delay(50);  // Debounce
      if (digitalRead(buttons[1]) == LOW) {
        playBackSound();
        return false;
      }
    }
  }
}

// Helper function to play sound for dot, dash, and other actions
void playSound(char type) {
  int duration = (type == '.') ? 100 : 300;  // Short for dot, longer for dash
  tone(buzzer, 1000, duration);  // Play tone at 1000 Hz
  delay(duration + 100);            // Wait before the next sound
}

// Helper function to convert Morse to character
char morseToChar(String morse) {
  if (morse == ".-") return 'A';
  else if (morse == "-...") return 'B';
  else if (morse == "-.-.") return 'C';
  else if (morse == "-..") return 'D';
  else if (morse == ".") return 'E';
  else if (morse == "..-.") return 'F';
  else if (morse == "--.") return 'G';
  else if (morse == "....") return 'H';
  else if (morse == "..") return 'I';
  else if (morse == ".---") return 'J';
  else if (morse == "-.-") return 'K';
  else if (morse == ".-..") return 'L';
  else if (morse == "--") return 'M';
  else if (morse == "-.") return 'N';
  else if (morse == "---") return 'O';
  else if (morse == ".--.") return 'P';
  else if (morse == "--.-") return 'Q';
  else if (morse == ".-.") return 'R';
  else if (morse == "...") return 'S';
  else if (morse == "-") return 'T';
  else if (morse == "..-") return 'U';
  else if (morse == "...-") return 'V';
  else if (morse == ".--") return 'W';
  else if (morse == "-..-") return 'X';
  else if (morse == "-.--") return 'Y';
  else if (morse == "--..") return 'Z';
  else if (morse == "-----") return '0';
  else if (morse == ".----") return '1';
  else if (morse == "..---") return '2';
  else if (morse == "...--") return '3';
  else if (morse == "....-") return '4';
  else if (morse == ".....") return '5';
  else if (morse == "-....") return '6';
  else if (morse == "--...") return '7';
  else if (morse == "---..") return '8';
  else if (morse == "----.") return '9';
  else if (morse == ".-.-.-") return '.';  // Period
  else if (morse == "--..--") return ',';  // Comma
  else if (morse == "..--..") return '?';  // Question mark
  else if (morse == "-.-.--") return '!';  // Exclamation mark
  else if (morse == "-.--.") return '(';   // Open parenthesis
  else if (morse == "-.--.-") return ')';   // Close parenthesis
  else if (morse == "---...") return ':';   // Colon
  else if (morse == "-.-.-.") return ';';   // Semicolon
  else if (morse == "-....-") return '-';   // Hyphen
  else if (morse == "..--.-") return '_';   // Underscore
  else if (morse == ".-..-.") return '"';   // Quotation mark
  else if (morse == ".----.") return '\'';  // Apostrophe
  else if (morse == "-...-") return '=';    // Equals sign
  else if (morse == ".-.-.") return '+';    // Plus sign
  else if (morse == "-..-.") return '/';    // Slash
  return '?';  // Return '?' for unknown Morse code sequences
}

// Function to play the final sequence of Morse code
void playFinalMorseSequence(String finalMorseSequence) {
  for (char &c : finalMorseSequence) {
    playSound(c);  // Play each sound in the sequence
  }
}

void updateLCD(String nameString,String text) {
  static int scrollIndex = 0;    // Current scroll position
  static unsigned long lastScrollTime = 0;  // Timer for scroll delay
  unsigned long scrollDelay = 500;  // Delay between shifts (milliseconds)

  // Reset scrollIndex if text has changed
  if (text.length() <= 16) {
    scrollIndex = 0;  // Reset to the start if within screen limit
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(nameString);
    lcd.setCursor(0, 1);
    lcd.print(text);  // Show entire text if within 16 characters
  } else {
    // Update display as soon as text exceeds 16 characters or changes
    if (scrollIndex < text.length() - 16) {
      scrollIndex = text.length() - 16;
    }

    // Only scroll if needed and if delay has passed
    if (millis() - lastScrollTime >= scrollDelay) {
      lastScrollTime = millis();  // Reset scroll timer
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(nameString);
      lcd.setCursor(0, 1);
      lcd.print(text.substring(scrollIndex, scrollIndex + 16));
    }
  }
}

String getPlayerName() {
  unsigned long currentTime;
  String playerName = "";
  String playerNameBuffer = "Name:";
  String morseCode = "";  // Temporary storage for current letter in Morse
  String displayText = "";  // Displayed on LCD
  String finalMorseSequence = "";  // Store the entire Morse code sequence

  while (true) {
    currentTime = millis();

    // Loop through each button to check its debounced state
    for (int i = 0; i < 4; i++) {
      int reading = digitalRead(buttons[i]);

      // Check if button state has changed
      if (reading != lastButtonState[i]) {
        lastDebounceTime[i] = currentTime; // Reset debounce timer
      }

      // If enough time has passed, check the stable state
      if ((currentTime - lastDebounceTime[i]) > debounceDelay) {
        // Button state has stabilized
        if (reading != buttonState[i]) {
          buttonState[i] = reading; // Update the button state

          // Only act on the button press (LOW to HIGH transition)
          if (buttonState[i] == LOW) {
            if (playerNameBuffer.length() >= 16){
              playBackSound();
              continue;
            }
            if (i == 0) {  // Red button: dot
              morseCode += ".";
              finalMorseSequence += ".";
              playSound('.');
              displayText += ".";  // Update display text
            } else if (i == 1) {  // Yellow button: dash
              morseCode += "-";
              finalMorseSequence += "-";
              playSound('-');
              displayText += "-";
            } else if (i == 2) {  // Green button: space (new letter)
              playerName += morseToChar(morseCode);  // Convert Morse to character
              playerNameBuffer += morseToChar(morseCode);
              morseCode = "";  // Reset for next letter
              finalMorseSequence += " ";
              displayText += " ";  // Add space for readability
              playSelectSound();
            } else if (i == 3) {  // Blue button: send (finalize name)
              if (morseCode.length() > 0) {  // Check if Morse code buffer has content
                playerName += morseToChar(morseCode);  // Add last letter
              }
              connectionSuccessTone();
              playFinalMorseSequence(finalMorseSequence);
              return playerName;  // End input and return name
            }
            updateLCD(playerNameBuffer, displayText);  // Update LCD display after each button press
          }
        }
      }
      lastButtonState[i] = reading;  // Save the last state
    }
  }
}

void sendScoreToMQTT(String name, int val) {
  if (!mqttClient.connected()) {
    ConnectMqtt();
  }
  mqttClient.loop();

  Serial.println(name);
  lcdCenterPrintTR("Sending to", "Leaderboard...");
  delay(2000);

  char buffer[100];
  snprintf(buffer, sizeof(buffer), "{ \"userid\": \"%s\", \"score\": %d }", name.c_str(), val);
  mqttClient.publish("arduiblox/simongame", buffer);

  lcdCenterPrintTR("Sending Score", "Successful!");
  connectionSuccessTone();
  delay(2000);
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

void lcdCenterPrintScore(const char* label, int score) {
  char buffer[20];  // Buffer for formatted string
  snprintf(buffer, sizeof(buffer), "%s %d", label, score);  // Format the string

  int length = strlen(buffer);  // Get the length of the string
  int position = (16 - length) / 2;  // Calculate starting position for centering

  lcd.setCursor(position, 0);  // Move to the center position on the first line
  lcd.print(buffer);  // Print the centered text
}
