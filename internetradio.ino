#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Audio.h>

// --- PIN CONFIGURATION ---
// I2S Pins for the MAX98357A
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

// Rotary Encoder pins
#define ENCODER_CLK   32
#define ENCODER_DT    33
#define ENCODER_SW    15 

// Wi-Fi Credentials 
const char* ssid     = "moaiwlan";
const char* password = "Ossi1Paavo234";

// Structure to store station information
struct Station {
  const char* name;
  const char* url;
};

// List of available radio stations
Station stations[] = {
  {"Capital FM UK", "http://vis.media-ice.musicradio.com/CapitalMP3"},
  {"SRF 3 Switzerland", "http://stream.srg-ssr.ch/m/drs3/mp3_128"},
  {"Couleur 3 Swiss", "http://stream.srg-ssr.ch/m/couleur3/mp3_128"},
  {"Radio Swiss Pop", "http://stream.srg-ssr.ch/m/rsp/mp3_128"},
  {"Virgin Rock", "http://icy.unitedradio.it/Virgin.mp3"}
};

const int numStations = sizeof(stations) / sizeof(stations[0]); 

// Global Objects
TFT_eSPI tft = TFT_eSPI();
Audio audio;

// --- STATE MACHINE ---
enum Mode { STATION_MODE, VOLUME_MODE };
Mode currentMode = STATION_MODE; 

int stationIndex = 0;
int currentVolume = 5; 
int lastClkState;
unsigned long lastButtonPress = 0;
unsigned long lastRotationTime = 0; 

// Forward Declarations
void changeStation();
void updateVolumeDisplay();
void refreshUI();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 Internet Radio Starting ---");

  // Configure encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // Initialize display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // UI Design
  tft.drawCircle(120, 120, 119, TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM); 
  tft.setTextColor(TFT_GREEN);
  tft.drawString("INTERNET RADIO", 120, 60, 2);

  // Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  tft.drawString("Connecting WiFi...", 120, 120, 2);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Audio setup
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);

  lastClkState = digitalRead(ENCODER_CLK);
  
  // Start the first station
  changeStation();
}

void loop() {
  audio.loop(); 

  // 1. ENCODER LOGIC
  int currentClkState = digitalRead(ENCODER_CLK);

  if (currentClkState != lastClkState && currentClkState == LOW) {
    if (millis() - lastRotationTime > 50) {
      
      bool clockwise = (digitalRead(ENCODER_DT) != currentClkState);

      if (currentMode == STATION_MODE) {
        if (clockwise) stationIndex = (stationIndex + 1) % numStations;
        else stationIndex = (stationIndex - 1 + numStations) % numStations;
        changeStation();
      } 
      else { // VOLUME_MODE
        if (clockwise) currentVolume = min(currentVolume + 1, 21);
        else currentVolume = max(currentVolume - 1, 0);
        
        audio.setVolume(currentVolume);
        Serial.printf("Volume changed to: %d\n", currentVolume);
        updateVolumeDisplay();
      }
      
      lastRotationTime = millis(); 
    }
  }
  lastClkState = currentClkState;

  // 2. BUTTON MODE TOGGLE
  if (digitalRead(ENCODER_SW) == LOW) {
    if (millis() - lastButtonPress > 300) { 
      currentMode = (currentMode == STATION_MODE) ? VOLUME_MODE : STATION_MODE;
      Serial.print("Mode switched to: ");
      Serial.println(currentMode == VOLUME_MODE ? "VOLUME" : "STATION");
      refreshUI();
      lastButtonPress = millis();
    }
  }
}

void refreshUI() {
  tft.fillRect(15, 100, 210, 65, TFT_BLACK); 
  if (currentMode == VOLUME_MODE) updateVolumeDisplay();
  else changeStation();
}

void updateVolumeDisplay() {
  tft.fillRect(15, 100, 210, 65, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("VOLUME CONTROL", 120, 115, 2);
  
  int barWidth = map(currentVolume, 0, 21, 0, 100);
  tft.drawRect(70, 140, 100, 10, TFT_WHITE);
  tft.fillRect(70, 140, barWidth, 10, TFT_YELLOW);
}

void changeStation() {
  Serial.printf("Switching to station: %s\n", stations[stationIndex].name);
  Serial.printf("URL: %s\n", stations[stationIndex].url);
  
  audio.stopSong();
  tft.fillRect(15, 100, 210, 65, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("STATION SELECT", 120, 115, 2);
  tft.setTextColor(TFT_CYAN);
  tft.drawString(stations[stationIndex].name, 120, 145, 4);
  
  audio.connecttohost(stations[stationIndex].url); 
}
