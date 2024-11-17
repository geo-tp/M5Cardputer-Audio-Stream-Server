#include <M5Cardputer.h>

void showMainTitle() {
    M5Cardputer.Display.drawRoundRect(10, 10, 220, 30, 5, TFT_DARKCYAN); // Around main title
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(19, 18);
    M5Cardputer.Display.printf("AudioStreamServer");
}

void showSubTitle() {
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.setCursor(21, 44);
    M5Cardputer.Display.printf("Stream your microphone");

}
void showGithubInfos() {
    M5Cardputer.Display.setTextColor(TFT_DARKGRAY);
    M5Cardputer.Display.setTextSize(1.2);
    M5Cardputer.Display.setCursor(134, 70);
    M5Cardputer.Display.printf("<- Github");
    M5Cardputer.Display.setCursor(133, 83);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.printf("1.1 Version");
    M5Cardputer.Display.setCursor(129, 95);
    M5Cardputer.Display.printf("Geo - Mmatuda");
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(117, 114);
    M5Cardputer.Display.setTextSize(1.4);
    M5Cardputer.Display.printf("Press any key");
}

void showQrCode() {
    M5Cardputer.Display.drawRect(30, 60, ((M5Cardputer.Display.height() / 2) + 4), ((M5Cardputer.Display.height() / 2) + 4), TFT_DARKCYAN);
    M5Cardputer.Display.qrcode("https://github.com/geo-tp/M5Cardputer-Audio-Stream-Server", 32, 62, M5Cardputer.Display.height() / 2, 4);
}

void showIP(const IPAddress& ip) {
    M5Cardputer.Display.setCursor(20, 85);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextSize(1.7);
    M5Cardputer.Display.println("Audio Stream Address");
    M5Cardputer.Display.drawRoundRect(10, 105, 220, 23, 3, TFT_DARKGREY); // Around ip
    M5Cardputer.Display.setCursor(22, 110);
    M5Cardputer.Display.print("http://");
    M5Cardputer.Display.print(ip);
    M5Cardputer.Display.println("/");
    M5Cardputer.Display.setCursor(31, 50);
    M5Cardputer.Display.setTextColor(TFT_RED);
}

void showRecIndicator() {
    M5Cardputer.Display.fillCircle(96, 64, 8, TFT_DARKCYAN);
    M5Cardputer.Display.setCursor(114, 58);
    M5Cardputer.Display.setTextColor(TFT_DARKGREY);
    M5Cardputer.Display.printf("REC");
}

void showStreamAddress(const IPAddress& ip) {
    M5Cardputer.Display.setCursor(20, 85);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextSize(1.7);
    M5Cardputer.Display.println("Audio Stream Address");
    M5Cardputer.Display.drawRoundRect(10, 105, 220, 23, 3, TFT_DARKGREY); // Around ip
    M5Cardputer.Display.setCursor(22, 110);
    M5Cardputer.Display.print("http://");
    M5Cardputer.Display.print(ip);
    M5Cardputer.Display.println("/");
}

void clearScreen() {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
}

void setBrightness(int value) {
    M5Cardputer.Display.setBrightness(value);
}

void initialiseScreen() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(80);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
}