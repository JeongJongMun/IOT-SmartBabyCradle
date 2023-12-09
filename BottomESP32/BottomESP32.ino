#include <Servo.h>
#include <AWS_IOT.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "HX711.h"

/*

{ "state":
  {"desired":
    {"servo":"ON" }
  } 
}

*/

// BME280
Adafruit_BME280 bme; // I2C

// AWS
AWS_IOT iot_client;

const char* ssid = "KAU-Guest";
const char* password = "";

char HOST_ADDRESS[] = "";
char CLIENT_ID[] = "JJM_ESP32";

char sTOPIC_NAME[] = "$aws/things/Smart_Cradle/shadow/name/ESP32_bottom/update/delta";
char pTOPIC_NAME[] = "$aws/things/Smart_Cradle/shadow/name/ESP32_bottom/update"; // publish topic name

char payload[512];
char rcvdPayload[512];

// payloads
String servo = "OFF";
float temp = 0;
float humid = 0;
float weight = 0;

// HX711 LoadCell
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
HX711 scale;
const float threshold = 2.5;

// Servo Motor
const int servoPin = 18;
Servo myServo;
int angle = 0;
int start = 100, end = 150;

TaskHandle_t servoTaskHandle = NULL;
TaskHandle_t weightTaskHandle = NULL;

void Print_BME() {
  temp = bme.readTemperature();
  humid = bme.readHumidity();
	Serial.print("Temperature = \t");
	Serial.print(temp, 2);
	Serial.println(" *C");
	Serial.print("Humidity = \t");
	Serial.print(humid, 2);
	Serial.println(" %");
  delay(3000);
}

void mySubCallBackHandler(char* topicName, int payloadLen, char* payLoad)
{
    strncpy(rcvdPayload, payLoad, payloadLen);
    rcvdPayload[payloadLen] = 0;

    Serial.print("Subscribe Message:");
    Serial.println(rcvdPayload);
    if (strstr(rcvdPayload, "ON") != nullptr) servo = "ON";
    else if (strstr(rcvdPayload, "OFF") != nullptr) servo = "OFF";
}

void ServoStart() {
  if (servo == "ON") {
    for(angle = start; angle <= end; angle += 1) {
      myServo.write(angle);
      delay(15);
    }

    for(angle = end; angle >= start; angle -= 1) {
      myServo.write(angle);
      delay(15);
    }
  }
  delay(100);
}

void SetZeroPoint() {
    Serial.println("### Setting Zero Point ###");
    scale.set_scale(2280.f);  // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare(); // reset the scale to 0
}

void Publish() {
    sprintf(payload, "{\"state\":{\"reported\":{\"servo\":\"%s\", \"weight\":%.2f, \"temp\":%.2f, \"humid\":%.2f}}}", servo, weight * 170, temp, humid);

    if (iot_client.publish(pTOPIC_NAME, payload) == 0) {
        Serial.print("Publish Message:");
        Serial.println(payload);
    }
    else Serial.println("Publish failed");
    Serial.println();
}

void CheckBabyIn() {
    weight = scale.get_units(10);
    Serial.print("Weight:\t\t");
    Serial.println(weight, 1);

    Publish();
    scale.power_down(); // put the ADC in sleep mode
    delay(3000);
    scale.power_up();
}

// Multi Processor
void servoTask(void *pvParameters) {
    for (;;) {
        ServoStart();
    }
}

void weightTask(void *pvParameters) {
    for (;;) {
        CheckBabyIn();
    }
}

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  SetZeroPoint();

  // BME Set Up
	bool bme_status;
  bme_status = bme.begin(0x76); // bme280 I2C address = 0x76 
	if (!bme_status) {
		Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
		Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
		Serial.print(" ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
		Serial.print(" ID of 0x56-0x58 represents a BMP 280,\n");
		Serial.print(" ID of 0x60 represents a BME 280.\n");
		Serial.print(" ID of 0x61 represents a BME 680.\n");
		while (1) delay(10);
	}

  // WiFi Set Up
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode()); //++choi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to wifi");

  // Init AWS_IoT Client
  if (iot_client.connect(HOST_ADDRESS, CLIENT_ID) == 0) {
    Serial.println("Connected to AWS");
    delay(1000);

    if (iot_client.subscribe(sTOPIC_NAME, mySubCallBackHandler) == 0) Serial.println("Subscribe Successful");
    else Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
  
    // Create tasks for Servo and Weight
    xTaskCreatePinnedToCore(
        servoTask, // Task Func
        "Servo Task", // Task Name
        10000, // Stack size of Task
        NULL, // Param of Task
        1, // Priority of Task
        &servoTaskHandle, // handle to Keep Track of Task
        0 // pin Task to Core 0
    );

    xTaskCreatePinnedToCore(
        weightTask,
        "Weight Task",
        10000,
        NULL,
        1,
        &weightTaskHandle,
        1
    );

  }
  else {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }
  Publish();
}

void loop() {
  Print_BME();
}