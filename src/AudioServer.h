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

    vTaskDelete(NULL);  // Remove tasks after execution
}