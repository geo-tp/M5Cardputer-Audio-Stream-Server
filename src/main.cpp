#include <WiFi.h>
#include <M5Cardputer.h>
#include <esp_http_server.h>

// Audio settings
static constexpr const size_t record_length = 1920;
static constexpr const size_t record_samplerate = 16000;
static int16_t rec_data[record_length];

httpd_handle_t server = NULL;

// HTTP headers
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "audio/wav";
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: audio/wav\r\nContent-Length: %u\r\n\r\n";

// HTML page for the web server
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Audio Stream</title>
</head>
<body>
    <h1>Audio Stream</h1>
    <audio controls autoplay>
        <source src="/audio" type="audio/wav">
        Your browser does not support the audio element.
    </audio>
</body>
</html>
)rawliteral";

// Generate WAV header
void saveWavHeader(httpd_req_t *req, uint32_t sampleRate, uint32_t dataSize) {
    char wav_header[44];
    uint32_t byteRate = sampleRate * 1 * sizeof(int16_t);
    uint16_t blockAlign = 1 * sizeof(int16_t);
    uint16_t bitsPerSample = 16;

    memcpy(wav_header, "RIFF", 4);
    uint32_t chunkSize = 36 + dataSize;
    memcpy(wav_header + 4, &chunkSize, 4);
    memcpy(wav_header + 8, "WAVE", 4);
    memcpy(wav_header + 12, "fmt ", 4);
    uint32_t subChunk1Size = 16;
    memcpy(wav_header + 16, &subChunk1Size, 4);
    uint16_t audioFormat = 1; // PCM
    memcpy(wav_header + 20, &audioFormat, 2);
    uint16_t numChannels = 1;
    memcpy(wav_header + 22, &numChannels, 2);
    memcpy(wav_header + 24, &sampleRate, 4);
    memcpy(wav_header + 28, &byteRate, 4);
    memcpy(wav_header + 32, &blockAlign, 2);
    memcpy(wav_header + 34, &bitsPerSample, 2);
    memcpy(wav_header + 36, "data", 4);
    memcpy(wav_header + 40, &dataSize, 4);

    httpd_resp_send_chunk(req, wav_header, 44);
}

// Audio stream handler
static esp_err_t audioHandler(httpd_req_t *req) {
    esp_err_t res = ESP_OK;
    size_t _audio_buf_len = 0;
    uint8_t* _audio_buf = (uint8_t*)rec_data;

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    saveWavHeader(req, record_samplerate, 0xFFFFFFFF); // maximum 32-bit unsigned integer

    while (true) {
        if (M5Cardputer.Mic.record(rec_data, record_length, record_samplerate)) {
            _audio_buf_len = record_length * sizeof(int16_t);
            res = httpd_resp_send_chunk(req, (const char*)_audio_buf, _audio_buf_len);
            if (res != ESP_OK) {
                break;
            }
        }
    }
    return res;
}

// Listen on 80 with '/' uri as audio stream
// Param is for multi threading purpose
void startServer(void *pvParameters) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t audio_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = audioHandler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &audio_uri);
    }

    vTaskDelete(NULL);  // Supprime la tâche après exécution
}

// Get WiFi Credentials
void getCredentials(char* ssid, char* password) {
    M5Cardputer.Display.setCursor(28, 72);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.println("Enter WiFi SSID");
    M5Cardputer.Display.drawLine(0, M5Cardputer.Display.height() - 30, 
                                 M5Cardputer.Display.width(), M5Cardputer.Display.height() - 30, 
                                 TFT_DARKGREY); // delimiter for text input
    
    String input = "> ";
    M5Cardputer.Display.drawString(input, 4, M5Cardputer.Display.height() - 24);

    bool ssidDone = false;
    while (true) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                if (status.enter) {
                    input.remove(0, 2); // To remove '> '
                    if (!ssidDone) {
                        strcpy(ssid, input.c_str());
                        input = "> ";
                        ssidDone = true;
                        M5Cardputer.Display.fillRect(20, 55, M5Cardputer.Display.width(), 40, TFT_BLACK); // clear area
                        M5Cardputer.Display.setCursor(34, 72);
                        M5Cardputer.Display.println("Enter Password");
                    } else {
                        strcpy(password, input.c_str());
                        break;
                    }
                } else {
                    for (auto i : status.word) {
                        input += i;
                    }
                    if (status.del) {
                        if (input.length() > 2) {
                            input.remove(input.length() - 1);
                        }
                    }
                }

                // Reset area and draw new string
                M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 29, M5Cardputer.Display.width() - 1 , 25, TFT_BLACK);
                M5Cardputer.Display.drawString(input, 4, M5Cardputer.Display.height() - 24);
                delay(150);
            }
        }
    }
}


/*  ############################################################################################################################## */


void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    char ssid[32] = "";
    char password[32] = "";

    // Microphone configuration
    auto micConfig = M5Cardputer.Mic.config();
    micConfig.sample_rate = record_samplerate;
    micConfig.magnification = 64; // Increase/decrease volume
    micConfig.noise_filter_level = 64;
    micConfig.task_priority = 1; // Micro priority over others tasks
    M5Cardputer.Mic.config(micConfig);
    M5Cardputer.Speaker.end(); // To be sure
    M5Cardputer.Mic.begin();

    // Screen configuration
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(80);
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    // Main Title
    M5Cardputer.Display.drawRoundRect(10, 10, 220, 30, 5, TFT_DARKCYAN); // Around main title
    M5Cardputer.Display.setTextColor(TFT_DARKCYAN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(19, 18);
    M5Cardputer.Display.printf("AudioStreamServer");

    // Sub title
    M5Cardputer.Display.setCursor(21, 44);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.printf("Stream your microphone");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    
    // Get Wifi SSID and password
    getCredentials(ssid, password);

    // Wifi connection
    M5Cardputer.Display.fillRect(0, 40, M5Cardputer.Display.width(), 
                                 M5Cardputer.Display.height(), TFT_BLACK); // clear area
    M5Cardputer.Display.setCursor(58, 58);
    M5Cardputer.Display.println("Connecting");
    M5Cardputer.Display.setCursor(500, 65);
    M5Cardputer.Display.setTextColor(TFT_DARKGRAY);
    WiFi.begin(ssid, password);

    // We cant run without Wifi, we wait until status is ok
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        M5Cardputer.Display.print(".");
    }

    // Clear area
    M5Cardputer.Display.fillRect(0, 40, M5Cardputer.Display.width(), 
                                 M5Cardputer.Display.height(), TFT_BLACK);

    // Display Rec
    M5Cardputer.Display.fillCircle(96, 64, 8, RED);
    M5Cardputer.Display.setCursor(114, 58);
    M5Cardputer.Display.setTextColor(TFT_DARKGREY);
    M5Cardputer.Display.printf("REC");

    // Ip Address
    M5Cardputer.Display.setCursor(31, 85);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextSize(1.7);
    M5Cardputer.Display.println("Audio Stream Ready");
    M5Cardputer.Display.drawRoundRect(10, 105, 220, 23, 3, TFT_DARKGREY); // Around ip
    M5Cardputer.Display.setCursor(22, 110);
    M5Cardputer.Display.print("http://");
    M5Cardputer.Display.print(WiFi.localIP());
    M5Cardputer.Display.println("/");

    // Http server task on core 1
    xTaskCreatePinnedToCore(startServer, "HTTP Server Task", 4096, NULL, 1, NULL, 1);
}


void loop() {
    delay(1);
}