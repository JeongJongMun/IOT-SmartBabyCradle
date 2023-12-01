#include <AWS_IOT.h>
#include <WiFi.h>
#include <math.h>

// WiFi 접속 정보
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

// AWS IoT 설정
AWS_IOT awsIot;
const char* mqtt_server = "yourAWSIoTEndpoint";
const char* clientID = "ESP32Client";
const char* decibel_topic = "esp32/decibel";
const char* crying_topic = "esp32/babycrying";

// 사운드 센서 핀 설정 및 울음 임계값 설정
const int SOUND_SENSOR_PIN = A0;
const int CRYING_THRESHOLD = 700;
const int QUIET_THRESHOLD = 400;
const unsigned long MIN_CRYING_DURATION = 5000;
const unsigned long MIN_QUIET_DURATION = 3000;
const float FREQUENCY_THRESHOLD = 0.7; // 주파수 변화율 임계값

// 패턴 분석을 위한 설정
const int PATTERN_SIZE = 10;
int decibelPattern[PATTERN_SIZE] = {0};
int patternIndex = 0;

unsigned long cryingStartTime = 0;
unsigned long quietStartTime = 0;
unsigned long lastDecibelPublishTime = 0;
int cryingCount = 0;
bool isCrying = false;

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

void loop() {
    int currentDecibel = analogRead(SOUND_SENSOR_PIN);
    unsigned long currentTime = millis();

    if (currentTime - lastDecibelPublishTime >= 1000) {
        decibelPattern[patternIndex] = currentDecibel;
        patternIndex = (patternIndex + 1) % PATTERN_SIZE;

        char decibelPayload[50];
        sprintf(decibelPayload, "{\"decibel\": %d}", currentDecibel);
        awsIot.publish(decibel_topic, decibelPayload);
        lastDecibelPublishTime = currentTime;

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
