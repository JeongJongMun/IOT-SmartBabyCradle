//AWS IoT코어를 통해 모빌 모터를 제어하는 코드
//delta subscribe, publish없음

#include <AWS_IOT.h>
#include <WiFi.h>

AWS_IOT motor_client;

const char* ssid = "Realmadrid";
const char* password = "kingofmadrid";
char HOST_ADDRESS[]="a2ichw3atwh8od-ats.iot.ap-northeast-2.amazonaws.com";
char CLIENT_ID[]= "YIB_ESP32";
char sTOPIC_NAME[]= "$aws/things/esp32_fan/shadow/name/ESP32_top/update/delta"; // subscribe topic name
char pTOPIC_NAME[]= "esp32/Motor";
char payload[512];
char rcvdPayload[512];

int status = WL_IDLE_STATUS;
const int INA = 15; //motor pinA
const int INB = 2;  //motor pinB
int motorSpeed = 0, isOn = 0;

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload,payLoad,payloadLen);
  rcvdPayload[payloadLen] = 0;

  Serial.print("Subscribe Messege");
  Serial.println(rcvdPayload);
  if (strstr(rcvdPayload,"ON") != nullptr) isOn = 1;
  else if (strstr(rcvdPayload,"OFF") != nullptr) isOn = 0;
}

//임시 publish 안쓰임
void dcMotorPublish() {
  sprintf(payload,"{\"state\":{\"reported\":{\"fan\":\"%d\"}}}", isOn);
  if (motor_client.publish(pTOPIC_NAME,payload)==0) {
    Serial.print("Publish Message:");
    Serial.println(payload);
  } else Serial.println("Publish failed");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(INA, OUTPUT);              
  pinMode(INB, OUTPUT);

  //Wifi - connection
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  delay(1000);

  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to wifi");

  //AWS - connection
  if(motor_client.connect(HOST_ADDRESS,CLIENT_ID) == 0) {
    Serial.println("Connected to AWS");
    delay(1000);
    if(motor_client.subscribe(sTOPIC_NAME,mySubCallBackHandler) == 0) {
      Serial.println("Subscribe Success!");
    }
    else {
      Serial.println("Subscribe failed..");
      while(1);
    }
  }
  else {
    Serial.println("AWS connection failed..");
    while(1);
  }
}

void loop() {
  //반환된 상태에 따라 mobile 제어(on,off) 
  if (isOn) {
    motorSpeed = 90;
    analogWrite(INA, 0);
    analogWrite(INB, motorSpeed);
    //dcMotorPublish()
  }
  else {
    motorSpeed = 0;
    analogWrite(INA, 0);
    analogWrite(INB, motorSpeed);
    //dcMotorPublish()
  }
  delay(1000);
}
