// ESP32 has two cores: APPlication core and PROcess core (the one that runs ESP32 SDK stack)
#define APP_CPU 1
#define PRO_CPU 0

#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
#include "home_wifi_multi.h"

OV2640 cam;

WebServer server(80);

// rtos task handles
// Streaming is implemented with 3 tasks:
TaskHandle_t tMjpeg;   // client connections
TaskHandle_t tCam;     // 프레임 로컬 저장
TaskHandle_t tStream;  // 연결된 모든 클라이언트에 스트리밍

// 프레임 동기화 - 다음 프레임으로 대체될 때 스트리밍 버퍼를 방지
SemaphoreHandle_t frameSync = NULL;

// 스트리밍 중인 큐 스토어
QueueHandle_t streamingClients;

//25 FPS 목표 속도
const int FPS = 14;

// 50ms(20Hz) 간격으로 요청
const int WSINTERVAL = 100;


// Server Connection
void mjpegCB(void* pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(WSINTERVAL);

  frameSync = xSemaphoreCreateBinary();
  xSemaphoreGive( frameSync );

  // 연결된 클라이언트 대기열 10대
  streamingClients = xQueueCreate( 10, sizeof(WiFiClient*) );

  // setup()

  // RTOS task
  xTaskCreatePinnedToCore(
    camCB,        // callback
    "cam",        // name
    4096,         // stacj size
    NULL,         // parameters
    2,            // priority
    &tCam,        // RTOS task handle
    APP_CPU);     // core

  //  연결된 모든 클라이언트에 스트림 푸시
  xTaskCreatePinnedToCore(
    streamCB,
    "strmCB",
    4 * 1024,
    NULL, //(void*) handler,
    2,
    &tStream,
    APP_CPU);

  server.on("/mjpeg/1", HTTP_GET, handleJPGSstream);
  server.on("/jpg", HTTP_GET, handleJPG);
  server.onNotFound(handleNotFound);

  //  서버 시작
  server.begin();

  // loop()
  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    server.handleClient();

    taskYIELD();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

volatile size_t camSize;    // 현재 프레임 크기, byte
volatile char* camBuf;    

// RTOS task 프레임
void camCB(void* pvParameters) {

  TickType_t xLastWakeTime;

  //  현재 원하는 프레임 속도, 실행 간격
  const TickType_t xFrequency = pdMS_TO_TICKS(1000 / FPS);

  // mutex
  portMUX_TYPE xSemaphore = portMUX_INITIALIZER_UNLOCKED;

  //  2개의 프레임, 각각의 크기 및 현재 프레임의 인덱스를 가리키는 포인터
  char* fbs[2] = { NULL, NULL };
  size_t fSize[2] = { 0, 0 };
  int ifb = 0;

  // loop() section 
  xLastWakeTime = xTaskGetTickCount();

  for (;;) {

    cam.run();
    size_t s = cam.getSize();

    //  프레임 크기가 이전에 할당된 것보다 큰 경우 125% size up
    if (s > fSize[ifb]) {
      fSize[ifb] = s * 4 / 3;
      fbs[ifb] = allocateMemory(fbs[ifb], fSize[ifb]);
    }

    //  프레임 로컬에 copy
    char* b = (char*) cam.getfb();
    memcpy(fbs[ifb], b, s);
    taskYIELD();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    //  현재 클라이언트로 스트리밍 중인 프레임이 없는 경우에만 프레임을 전환
    xSemaphoreTake( frameSync, portMAX_DELAY );

    //  인터럽트 허용 x
    portENTER_CRITICAL(&xSemaphore);
    camBuf = fbs[ifb];
    camSize = s;
    ifb++;
    ifb &= 1;
    portEXIT_CRITICAL(&xSemaphore);
    xSemaphoreGive( frameSync );
    xTaskNotifyGive( tStream );
    taskYIELD();
  }
}


//PSRAM이 있는 경우 memory 할당
char* allocateMemory(char* aPtr, size_t aSize) {
  if (aPtr != NULL) free(aPtr); //버퍼 비우기

  size_t freeHeap = ESP.getFreeHeap();
  char* ptr = NULL;
  // 2/3초과시 psram
  if ( aSize > freeHeap * 2 / 3 ) {
    if ( psramFound() && ESP.getFreePsram() > aSize ) {
      ptr = (char*) ps_malloc(aSize);
    }
  }
  else {
    //  여유시
    ptr = (char*) malloc(aSize);
  }

  // 메모리 포인터가 NULL이면 종료
  if (ptr == NULL) {
    ESP.restart();
  }
  return ptr;
}


// ==== STREAMING ======================================================
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);


// 클라이언트의 연결 요청 처리
void handleJPGSstream(void)
{
  if ( !uxQueueSpacesAvailable(streamingClients) ) return;

  WiFiClient* client = new WiFiClient();
  *client = server.client();
  client->write(HEADER, hdrLen);
  client->write(BOUNDARY, bdrLen);

  xQueueSend(streamingClients, (void *) &client, 0);
  if ( eTaskGetState( tCam ) == eSuspended ) vTaskResume( tCam );
  if ( eTaskGetState( tStream ) == eSuspended ) vTaskResume( tStream );
}


// ==== Actually stream content to all connected clients ========================
void streamCB(void * pvParameters) {
  char buf[16];
  TickType_t xLastWakeTime;
  TickType_t xFrequency;

  //  첫 프레임 후 다음 화면까지 대기
  ulTaskNotifyTake( pdTRUE,        
                    portMAX_DELAY );

  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    xFrequency = pdMS_TO_TICKS(1000 / FPS);
    //  보고 있을 때
    UBaseType_t activeClients = uxQueueMessagesWaiting(streamingClients);
    if ( activeClients ) {
      xFrequency /= activeClients;
      WiFiClient *client;
      xQueueReceive (streamingClients, (void*) &client, 0);

      //  클라이언트 연결 체크
      if (!client->connected()) {
        delete client;
      }
      else {
        //프레임 변경 방지위해 세미포어 잡기
        xSemaphoreTake( frameSync, portMAX_DELAY );

        client->write(CTNTTYPE, cntLen);
        sprintf(buf, "%d\r\n\r\n", camSize);
        client->write(buf, strlen(buf));
        client->write((char*) camBuf, (size_t)camSize);
        client->write(BOUNDARY, bdrLen);
        xQueueSend(streamingClients, (void *) &client, 0);
        xSemaphoreGive( frameSync );
        taskYIELD();
      }
    }
    else {
      vTaskSuspend(NULL);
    }
    taskYIELD();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}



const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

// 1frame upload
void handleJPG(void)
{
  WiFiClient client = server.client();

  if (!client.connected()) return;
  cam.run();
  client.write(JHEADER, jhdLen);
  client.write((char*)cam.getfb(), cam.getSize());
}


// invaild url
void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text / plain", message);
}


void setup()
{

  // Setup Serial connection:
  Serial.begin(115200);
  delay(1000); // wait for a second to let Serial connect

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters: pick one
   //config.frame_size = FRAMESIZE_UXGA;
   //config.frame_size = FRAMESIZE_SVGA;
   //config.frame_size = FRAMESIZE_QVGA;
  config.frame_size = FRAMESIZE_VGA;
  // config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality = 12;
  config.fb_count = 3;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  if (cam.init(config) != ESP_OK) {
    Serial.println("Error initializing the camera");
    delay(10000);
    ESP.restart();
  }

  // connect to WiFi
  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.print("Stream Link: http://");
  Serial.print(ip);
  Serial.println("/mjpeg/1");

  // Start RTOS task
  xTaskCreatePinnedToCore(
    mjpegCB,
    "mjpeg",
    4 * 1024,
    NULL,
    2,
    &tMjpeg,
    APP_CPU);
}

void loop() {
  vTaskDelay(1000);
}
