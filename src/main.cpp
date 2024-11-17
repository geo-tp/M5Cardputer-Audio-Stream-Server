#include <WiFi.h>
#include <M5Cardputer.h>
#include <esp_http_server.h>
#include <Preferences.h>

#include "CardWifiSetup.h"
#include "CardDisplay.h"
#include "AudioServer.h"

void waitPress(){
  while(1){
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
      if (M5Cardputer.Keyboard.isPressed()) {
        return;
      }
    }
  }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    // Microphone configuration
    auto micConfig = M5Cardputer.Mic.config();
    micConfig.sample_rate = record_samplerate;
    micConfig.magnification = 64; // Increase/decrease volume
    micConfig.noise_filter_level = 64;
    micConfig.task_priority = 1; // Micro priority over others tasks
    M5Cardputer.Mic.config(micConfig);
    M5Cardputer.Speaker.end(); // To be sure
    M5Cardputer.Mic.begin();

    // Welcome Screen
    initialiseScreen();
    showMainTitle();
    showSubTitle();
    showGithubInfos();
    showQrCode();

    // Wait until key press
    waitPress();

    // WiFI Setup
    setupWifi();

    // Audio Stream Screen
    clearScreen();
    showMainTitle();
    showRecIndicator();
    showIP(WiFi.localIP());
    delay(3000);
    setBrightness(16);

    // Http server task on core 1
    xTaskCreatePinnedToCore(startServer, "HTTP Server Task", 4096, NULL, 1, NULL, 1);
}


void loop() {
    delay(1);
}
