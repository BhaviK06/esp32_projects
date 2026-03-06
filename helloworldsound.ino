#include <Audio.h>          // ESP32_AudioI2S library
#include <FS.h>             // File system
#include <SPIFFS.h>         // SPIFFS file system

Audio audio;                // Sound object

void setup() {
  Serial.begin(115200);  // Serial monitor
  Serial.println("Initializing...");

  // Initializing SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS could not be started.");
    while (1);  // Stop in this loop
  }
  Serial.println("SPIFFS in operation!");

  // Check for MP3
  if (!SPIFFS.exists("/helloworld.mp3")) {
    Serial.println("MP3 not found in SPIFFS, remember to upload sketch data");
    while (1);  // Stop in this loop
  }

  // Play sound from SPIFFS
  audio.setPinout(25, 26, 27);  // I2S pins: BCLK, LRC, DIN
  audio.connecttoFS(SPIFFS, "/helloworld.mp3");  // Connect to SPIFFS
  audio.setVolume(15);  // Set volume

  Serial.println("Playing MP3");
}

void loop() {
  audio.loop();  // Continuously call audio.loop() to play the MP3
}
