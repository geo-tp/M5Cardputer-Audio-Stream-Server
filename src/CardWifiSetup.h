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
    String data = "> ";

    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
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
        M5Cardputer.Display.drawString("Scanning Networks", 42, 55);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
        M5Cardputer.Display.drawString("Connect to WiFi", 52, 75);
        networksCount = WiFi.scanNetworks();

        if (networksCount == 0) {
            M5Cardputer.Display.drawString("No networks found", 45, 95);
            delay(2000);
        }
    }
    
    return networksCount;
}

static void drawWifiList(int numNetworks, int topIndex, int selectedIndex, int x, int y0, int visibleRows, int rowHeight)
{
    bool showBar = (numNetworks > visibleRows);
    int screenWidth = M5Cardputer.Display.width();
    int listHeight = visibleRows * rowHeight;
    int vbarWidth = showBar ? 8 : 0;
    int vbarX = screenWidth - vbarWidth;
    int listWidth = screenWidth - vbarWidth - 2;

    // Clear everything below the header
    M5Cardputer.Display.fillRect(0, y0 - 2, screenWidth, listHeight + 4, TFT_BLACK);

    // Draw visible rows
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY, TFT_DARKCYAN);
    for (int row = 0; row < visibleRows; row++) {
        int ssidIndex = topIndex + row;
        if (ssidIndex >= numNetworks) 
            break;

        int y = y0 + row * rowHeight;
        String ssid = WiFi.SSID(ssidIndex);
        String line;
        if (ssidIndex == selectedIndex)
            line = "-> " + ssid;
        else 
            line = "   " + ssid;

        M5Cardputer.Display.drawString(line, x, y);
    }

    if (!showBar)
        return;

    // Overwrite scrollbar on top of the text
    M5Cardputer.Display.fillRect(vbarX, y0 - 2, vbarWidth, listHeight + 4, TFT_BLACK);
    if (numNetworks > visibleRows) {
        int trackX = vbarX + 1;
        int trackY = y0;
        int trackWidth = vbarWidth - 2;
        int trackHeight = listHeight;

        // Scrollbar background
        M5Cardputer.Display.fillRect(trackX, trackY, trackWidth, trackHeight, TFT_DARKGREY);
        M5Cardputer.Display.drawRect(trackX, trackY, trackWidth, trackHeight, 0x05A3);

        // Scrollbar thumb
        int minThumb = 12;
        int thumbHeigth = (trackHeight * visibleRows) / numNetworks;
        if (thumbHeigth < minThumb) thumbHeigth = minThumb;

        // Thumb position maps scroll 'top' into [0 .. trackH - thumbH]
        int scrollRange = numNetworks - visibleRows;
        int thumbY = trackY;
        if (scrollRange > 0)
            thumbY = trackY + ((trackHeight - thumbHeigth) * topIndex) / scrollRange;

        // Draw scrollbar thumb
        M5Cardputer.Display.fillRect(trackX + 1, thumbY + 1, trackWidth - 2, thumbHeigth - 2, 0x05A3);
    }
}

String selectWifiNetwork(int numNetworks)
{
    int visibleRows = 6;
    int rowHeight = 18;
    int X = 1;
    int Y0 = 18;
    int selectedIndex = 0;  // Row which is currently selected
    int topIndex = 0;       // The first row on the list
    // The visible window is comprised of:
    // ===HEADER===                 
    // topIndex                     |S|
    // indexItem1                   |C|
    // indexItem2                   |R|
    // indexItem3                   |O|
    // indexItem4                   |L|
    // topIndex+visibleRows - 1     |L|

    // Select network header
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(0x05A3, TFT_DARKCYAN);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.drawString("Select Network", 1, 1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY, TFT_BLACK);

    drawWifiList(numNetworks, topIndex, selectedIndex, X, Y0, visibleRows, rowHeight);

    while (true) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            auto inputKey = M5Cardputer.Keyboard.keysState();

            // Up (';')
            if (M5Cardputer.Keyboard.isKeyPressed(';') && selectedIndex > 0) {
                selectedIndex--;
                // Means that when I press 'up' the top item isn't the top item anymore, it's second to selectedIndex, so replace it
                if (selectedIndex < topIndex) 
                    topIndex = selectedIndex;
                drawWifiList(numNetworks, topIndex, selectedIndex, X, Y0, visibleRows, rowHeight);
            }

            // Down ('.')
            if (M5Cardputer.Keyboard.isKeyPressed('.') && selectedIndex < (numNetworks - 1)) {
                selectedIndex++;
                // When you press 'down' the top item is not seen anymore, so it needs to be updated
                if (selectedIndex >= topIndex + visibleRows)
                    topIndex = selectedIndex - visibleRows + 1;
                drawWifiList(numNetworks, topIndex, selectedIndex, X, Y0, visibleRows, rowHeight);
            }

            if (inputKey.enter) {
                return WiFi.SSID(selectedIndex);
            }
        }
        delay(20);
    }
}

void setWifiCredentials(String ssid, String password) {
    preferences.begin("wifi_settings", false);
    preferences.putString(NVS_SSID_KEY, ssid);
    preferences.putString(NVS_PASS_KEY, password);
    preferences.end();
}

void getWifiCredentials(String &ssid, String &password) {
    preferences.begin("wifi_settings", false);
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
}

bool connectToWifi(String wifiSSID, String wifiPassword) {
    int tm = 0;

    WiFi.disconnect();
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.drawString("Connecting", 70, 55);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(30, 75);
    M5Cardputer.Display.setTextSize(0.5); // for the loading dots

    while (tm++ < 110 && WiFi.status() != WL_CONNECTED) {
        delay(100);
        M5Cardputer.Display.print(".");

        if (tm == 55) {
            M5Cardputer.Display.setCursor(30, 80);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.disconnect();
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.setTextSize(1.4);
    M5Cardputer.Display.drawString("Failed to connect to WiFi", 18, 90);
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

void setupWifi() {
    bool connected = false;
    int numNetworks = 0;
    String savedSSID;
    String savedPassword;
    String selectedSSID;
    String wifiPassword;

    while (!connected) {
        // Scan Networks
        numNetworks = scanWifiNetworks();

        // Select Network
        selectedSSID = selectWifiNetwork(numNetworks);

        // Get saved credentials
        getWifiCredentials(savedSSID, savedPassword);

        // Saved SSID is empty or selected SSID doesn't match saved SSID
        if (savedSSID.isEmpty() || savedSSID != selectedSSID) {
            wifiPassword = askWifiPassword(selectedSSID);
        } else {
            wifiPassword = savedPassword;
        }
        
        connected = connectToWifi(selectedSSID, wifiPassword);

        if (connected) { 
            setWifiCredentials(selectedSSID, wifiPassword);
        } else {
            setWifiCredentials("", ""); // erase creds
        }
    }
}