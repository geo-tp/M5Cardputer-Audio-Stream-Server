/**
 * @file CardWifiSetup.h
 * @author Aurélio Avanzi Dutch version Roland Breedveld adapted by Milton Matuda for Mic WebServer
 * @brief https://github.com/cyberwisk/M5Card_Wifi_KeyBoard_Setup/tree/main/M5Card_Wifi_KeyBoard_Setup
 * @version Apha 0.4BR
 * @date 2024-11-14
 *
 * @Hardwares: M5Cardputer - https://docs.m5stack.com/en/core/Cardputer
 * @Dependent Librarys:
 * M5Cardputer: https://github.com/m5stack/M5Cardputer
 * WiFi: https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
 **/

#include <WiFi.h>
#define NVS_SSID_KEY "wifi_ssid"
#define NVS_PASS_KEY "wifi_pass"

String CFG_WIFI_SSID;
String CFG_WIFI_PASS;
Preferences preferences;

String inputText(const String& prompt, int x, int y) {
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    String data = "> ";
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextScroll(true);
    M5Cardputer.Display.drawString(prompt, x, y);
    while (1) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                M5Cardputer.Speaker.tone(1437, 20);
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                for (auto i : status.word) {
                    data += i;
                }
                if (status.del) {
                    data.remove(data.length() - 1);
                }
                if (status.enter) {
                    data.remove(0, 2);
                    M5Cardputer.Display.println(data);
                    return data;
                }
                M5Cardputer.Display.fillRect(0, y - 4, M5Cardputer.Display.width(), 25, BLACK);
                M5Cardputer.Display.drawString(data, 4, y);
            }
        }
        delay(20);
    }
}

void displayWiFiInfo() {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(1, 1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.drawString("WiFi connecting.", 35, 1);
    M5Cardputer.Display.setTextColor(TFT_DARKGRAY);
    M5Cardputer.Display.drawString("SSID: " + WiFi.SSID(), 1, 18);
    M5Cardputer.Display.drawString("IP: " + WiFi.localIP().toString(), 1, 33);
    int8_t rssi = WiFi.RSSI();
    M5Cardputer.Display.drawString("RSSI: " + String(rssi) + " dBm", 1, 48);
    delay(1000);
}

String scanAndDisplayNetworks() {
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0) {
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
        M5Cardputer.Display.drawString("Scan Networks", 1, 15);
        return "";
    } else {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_DARKGRAY);
        M5Cardputer.Display.drawString("Found Networks:", 1, 1);
        int selectedNetwork = 0;
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
        while (1) {
            for (int i = 0; i < 5 && i < numNetworks; ++i) {
                String ssid = WiFi.SSID(i);
                if (i == selectedNetwork) {
                    M5Cardputer.Display.drawString("-> " + ssid, 1, 18 + i * 18);
                } else {
                    M5Cardputer.Display.drawString("   " + ssid, 1, 18 + i * 18);
                }
            }
            M5Cardputer.Display.drawString("Select Network.", 1, 108);
            M5Cardputer.update();
            if (M5Cardputer.Keyboard.isChange()) {
                if (M5Cardputer.Keyboard.isPressed()) {
                    M5Cardputer.Speaker.tone(1437, 20);
                    M5Cardputer.Display.fillRect(0, 11, 20, 75, BLACK);
                    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                    if (M5Cardputer.Keyboard.isKeyPressed(';') && selectedNetwork > 0) {
                        selectedNetwork--;
                    }
                    if (M5Cardputer.Keyboard.isKeyPressed('.') && selectedNetwork < min(4, numNetworks - 1)) {
                        selectedNetwork++;
                    }
                    if (status.enter) {
                        return WiFi.SSID(selectedNetwork);
                    }
                }
            }
            delay(20);
        }
    }
}

void connectToWiFi() {
    CFG_WIFI_SSID = "";
    CFG_WIFI_PASS = "";

    preferences.begin("wifi_settings", false);
    delay(200);
    CFG_WIFI_SSID = preferences.getString(NVS_SSID_KEY, "");
    CFG_WIFI_PASS = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
    WiFi.disconnect();
    WiFi.begin(CFG_WIFI_SSID.c_str(), CFG_WIFI_PASS.c_str());

    int tm = 0;
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.print("Connecting :");
    while (tm++ < 110 && WiFi.status() != WL_CONNECTED) {
        M5Cardputer.update();
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.drawString("BtnG0 clean Config.", 1, 108);
        if (M5Cardputer.BtnA.isPressed()){
                for (int i=0; i < 5; i++){
                  M5Cardputer.Speaker.tone(1337, 90);
                  delay(90);
                  M5Cardputer.Speaker.tone(1437, 90);
                  delay(90);
                }
                M5Cardputer.Speaker.tone(1337, 120);
                delay(120);                
                Preferences preferences;
                preferences.begin("wifi_settings", false);
                preferences.clear();
                preferences.end();
                M5Cardputer.Display.clear();
                M5Cardputer.Display.drawString("Memory Cleared.", 1, 60);
                delay(1000);
                ESP.restart();
                return;
         } else {
          M5Cardputer.Display.setTextColor(TFT_DARKGRAY);
          delay(100);
          M5Cardputer.Display.print(".");
         }
    }

    if (WiFi.status() == WL_CONNECTED) {
        displayWiFiInfo();
        delay(250);
    } else {      
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
        M5Cardputer.Display.drawString("Searching WiFi", 2, 2);
        CFG_WIFI_SSID = scanAndDisplayNetworks();
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.drawString("SSID: " + CFG_WIFI_SSID, 2, 2);
        M5Cardputer.Display.drawString("Password:", 2, 20);
        CFG_WIFI_PASS = inputText("> ", 4, M5Cardputer.Display.height() - 24);

        //Preferences preferences;
        preferences.begin("wifi_settings", false);
        preferences.putString(NVS_SSID_KEY, CFG_WIFI_SSID);
        preferences.putString(NVS_PASS_KEY, CFG_WIFI_PASS);
        preferences.end();
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.drawString("SSID Password Registered.", 2, 60);
        WiFi.begin(CFG_WIFI_SSID.c_str(), CFG_WIFI_PASS.c_str());
        delay(100);
        displayWiFiInfo();
    }
}
