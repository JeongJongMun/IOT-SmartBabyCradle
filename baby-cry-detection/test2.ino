#include <AWS_IOT.h>
#include <WiFi.h>
#include <SPI.h>
#include <math.h>
#include "arduinoFFT.h" // Include the FFT library

// WiFi 접속 정보
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

// AWS IoT 설정
AWS_IOT awsIot;
const char* mqtt_server = "yourAWSIoTEndpoint";
const char* clientID = "ESP32Client";
const char* decibel_topic = "esp32/decibel";
const char* crying_topic = "esp32/babycrying";

// ISD 1820 녹음 모듈 핀 설정
int rec = 5; // Record pin
int play = 4; // Playback pin

// MCP3204 ADC 설정
const int CS_PIN = 10; // Chip Select for MCP3204

// FFT 설정
arduinoFFT FFT = arduinoFFT(); // Create FFT object
const unsigned int samples = 128; // Number of samples. Should be a power of 2
const double samplingFrequency = 1000; // Sampling frequency in Hz
double vReal[samples];
double vImag[samples];

// 사운드 센서 및 패턴 분석 설정
const int SOUND_SENSOR_PIN = A0;
const int PATTERN_SIZE = 10;
int decibelPattern[PATTERN_SIZE] = {0};
int patternIndex = 0;
unsigned long cryingStartTime = 0;
unsigned long quietStartTime = 0;
unsigned long lastDecibelPublishTime = 0;
int cryingCount = 0;
bool isCrying = false;
const int CRYING_THRESHOLD = 700;
const int QUIET_THRESHOLD = 400;
const unsigned long MIN_CRYING_DURATION = 5000;
const unsigned long MIN_QUIET_DURATION = 3000;
const float FREQUENCY_THRESHOLD = 0.7;

void setup_wifi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
}

void setup() {
    Serial.begin(115200);
    setup_wifi();

    if (awsIot.connect(mqtt_server, clientID) == 0) {
        Serial.println("Connected to AWS IoT");
    } else {
        Serial.println("AWS IoT connection failed");
        while (1);
    }
    pinMode(SOUND_SENSOR_PIN, INPUT);
    pinMode(rec, OUTPUT);
    pinMode(play, OUTPUT);
    SPI.begin();
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
}

float calculateCorrelation() {
    float sumX = 0, sumY = 0, sumX2 = 0, sumY2 = 0, sumXY = 0;
    float n = PATTERN_SIZE;
    
    for (int i = 0; i < PATTERN_SIZE; i++) {
        sumX += i;
        sumY += decibelPattern[i];
        sumX2 += i * i;
        sumY2 += decibelPattern[i] * decibelPattern[i];
        sumXY += i * decibelPattern[i];
    }

    float numerator = n * sumXY - sumX * sumY;
    float denominator = sqrt((n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY));

    if (denominator == 0) return 0;
    return numerator / denominator;
}

int readMCP3204(int channel) {
    digitalWrite(CS_PIN, LOW);
    int command = 0b11000000 | (channel << 3); // Command to read from channel
    SPI.transfer(command); // Start bit, SGL/DIFF and D2
    int highByte = SPI.transfer(0x00); // D1 and D0, then zeroes
    int lowByte = SPI.transfer(0x00); // Zeroes
    digitalWrite(CS_PIN, HIGH);

    return ((highByte & 0x0F) << 8) | lowByte; // Combine both bytes
}

void loop() {
    if (currentTime - lastDecibelPublishTime >= 10000) {
        decibelPattern[patternIndex] = currentDecibel;
        patternIndex = (patternIndex + 1) % PATTERN_SIZE;

        char decibelPayload[50];
        sprintf(decibelPayload, "{\"decibel\": %d}", currentDecibel);
        awsIot.publish(decibel_topic, decibelPayload);
        lastDecibelPublishTime = currentTime;
        digitalWrite(rec, HIGH);
        delay(10000); // Record for 10 seconds
        digitalWrite(rec, LOW);
    
        digitalWrite(play, HIGH);
        delay(10); // Play the recorded sound
        // Read and process the digitized audio data
        for (int i = 0; i < samples; i++) {
            vReal[i] = readMCP3204(0); // Read from MCP3204
            vImag[i] = 0;
            delayMicroseconds(1000000 / samplingFrequency);
        }
        digitalWrite(play, LOW);
    
        // Perform FFT on the data
        FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
        FFT.ComplexToMagnitude(vReal, vImag, samples);
        double peakFrequency = FFT.MajorPeak(vReal, samples, samplingFrequency);

        // Frequency analysis to detect baby crying
        if (peakFrequency > 300 && peakFrequency < 600) {
            // Detected baby crying
            float correlation = calculateCorrelation();
            if (correlation < 0.7) { // 패턴이 자연스러운 경우
                if (currentDecibel > CRYING_THRESHOLD && !isCrying) {
                    cryingStartTime = currentTime;
                    isCrying = true;
                }
    
                if (isCrying && currentDecibel < QUIET_THRESHOLD) {
                    if ((currentTime - cryingStartTime) > MIN_CRYING_DURATION) {
                        cryingCount++;
                        isCrying = false;
                    }
                    quietStartTime = currentTime;
                }
    
                if ((currentTime - quietStartTime) > MIN_QUIET_DURATION && cryingCount >= 2) {
                    char cryingPayload[50];
                    sprintf(cryingPayload, "{\"status\": \"Baby is repeatedly crying\"}");
                    awsIot.publish(crying_topic, cryingPayload);
                    cryingCount = 0;
                }
            }
        }
    }
}