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
                if (status.del && data.length() > 2) {
                    data.remove(data.length() - 1);
                }
                if (status.enter) {
                    data.remove(0, 2);
                    return data;
                }
                M5Cardputer.Display.fillRect(0, y - 4, M5Cardputer.Display.width(), 25, BLACK);
                M5Cardputer.Display.drawString(data, 4, y);
            }
        }
        delay(20);
    }
}

int scanWifiNetworks() {
    int networksCount = 0;

    while (networksCount == 0) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
        M5Cardputer.Display.setTextSize(1.6);
        M5Cardputer.Display.drawString("Scanning Networks", 42, 60);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
        M5Cardputer.Display.drawString("Connect to WiFi", 52, 80);
        networksCount = WiFi.scanNetworks();

        if (networksCount == 0) {
            M5Cardputer.Display.drawString("No networks found", 45, 100);
            delay(2000);
        }
    }
    
    return networksCount;
}

String selectWifiNetwork(int numNetworks) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
        M5Cardputer.Display.setTextSize(1.6);
        M5Cardputer.Display.drawString("Select Network", 1, 1);
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

bool connectToSavedWiFi(String selectedSSID) {
    String savedSSID = "";
    String savedPassword = "";
    int tm = 0;

    preferences.begin("wifi_settings", false);
    delay(200);
    savedSSID = preferences.getString(NVS_SSID_KEY, "");
    savedPassword = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();

    if (savedSSID.isEmpty()) {
        return false;
    } 

    if (savedSSID != selectedSSID) {
        return false;
    }

    WiFi.disconnect();
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.drawString("Connecting", 70, 60);

    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(30, 80);
    M5Cardputer.Display.setTextSize(0.5);

    while (tm++ < 110 && WiFi.status() != WL_CONNECTED) {
        delay(100);
        M5Cardputer.Display.print(".");

        if (tm == 55) {
            M5Cardputer.Display.setCursor(30, 85);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.disconnect();
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.setTextSize(1.4);
    M5Cardputer.Display.drawString("Failed to connect to WiFi", 18, 95);
    delay(3000);
    return false;    
}

String askWifiPassword(String ssid) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.drawString("SSID: " + ssid, 2, 2);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.drawString("Enter the password", 37, 90);
    return inputText("> ", 4, M5Cardputer.Display.height() - 24);
}

void saveWifiCredentials(String ssid, String password) {
    preferences.begin("wifi_settings", false);
    preferences.putString(NVS_SSID_KEY, ssid);
    preferences.putString(NVS_PASS_KEY, password);
    preferences.end();
}

void eraseWifiCredentials() {
    preferences.begin("wifi_settings", false);
    preferences.putString(NVS_SSID_KEY, "");
    preferences.putString(NVS_PASS_KEY, "");
    preferences.end(); 
}

void setupWifi() {
    bool connected = false;
    while (!connected)
    {
        // Scan Networks
        int numNetworks = scanWifiNetworks();

        // Select Network
        String wifiSSID = selectWifiNetwork(numNetworks);
        String wifiPassword = "";

        // Try to connect with saved creds
        connected = connectToSavedWiFi(wifiSSID);
        if (!connected) {
            wifiPassword = askWifiPassword(wifiSSID);
            saveWifiCredentials(wifiSSID, wifiPassword);
            connected = connectToSavedWiFi(wifiSSID);
        }

        // Still not connected with the new creds
        if (!connected) { 
            eraseWifiCredentials(); // erase creds
        };
    }
}