#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "LiquidCrystal_I2C.h"
#include "SimonGame.h"
// Include other game headers here as you add more games

const char* ssid = "12625_KMITL_2.4G";      // Your SSID
const char* password = "77501463";  // Your Password

// MQTT constants
// broker address, port, and client name
const char *MQTT_BROKER_ADRESS = "mqtt-dashboard.com";
const uint16_t MQTT_PORT = 1883;
const char *MQTT_CLIENT_NAME = "Arduiblox_Client";

WiFiClient network;
PubSubClient mqttClient(network);

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

  // Connect to WiFi
  connectToWiFi();
  InitMqtt();

  displayWelcomeMessage();
  displayMenu();
}

void loop() {
  HandleMqtt();

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

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi..");
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    // Update LCD with dots to show progress
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    Serial.println("\nConnected to WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check Settings");
  }
  delay(2000);
}

void OnMqttReceived(char *topic, byte *payload, unsigned int length){
    Serial.print("Received on ");
    Serial.print(topic);
    Serial.print(": ");

    String content = "";
    for (size_t i = 0; i < length; i++)
    {
        content.concat((char)payload[i]);
    }
    Serial.print(content);
    Serial.println();
}

void InitMqtt(){
    mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
    mqttClient.setCallback(OnMqttReceived);
}

void SubscribeMqtt(){
    mqttClient.subscribe("arduiblox/#");
}

void ConnectMqtt(){
    Serial.print("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
        SubscribeMqtt();
        Serial.println("MQTT CONNECTED!");
    }
    else
    {
        Serial.print("Failed MQTT connection, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");

        delay(5000);
    }
}

void HandleMqtt(){
    if (!mqttClient.connected())
    {
        ConnectMqtt();
    }
    mqttClient.loop();
}