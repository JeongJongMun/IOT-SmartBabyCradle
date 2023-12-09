//AWS IoT코어를 통해 모빌과 선풍기를 제어하는 코드

#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>

//AWS parameter
AWS_IOT motor_client; //iot class

<<<<<<< HEAD
char HOST_ADDRESS[]="xxx-xxx.iot.ap-northeast-2.amazonaws.com";
char CLIENT_ID[]= "YIB_ESP32";
char sTOPIC_NAME[]= ""; // subscribe topic name
char pTOPIC_NAME[]= ""; //임시 publish
char payload[512];
char rcvdPayload[512];
//wifi parameter
const char* ssid = "xxxx";
const char* password = "xxxx";
=======
char HOST_ADDRESS[]="a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com";
char CLIENT_ID[]= "YIB_ESP32";
char sTOPIC_NAME[]= "$aws/things/Smart_Cradle/shadow/name/ESP32_top/update/delta"; // subscribe topic name
char pTOPIC_NAME[]= "$aws/things/Smart_Cradle/shadow/name/ESP32_top/update"; //임시 publish
char payload[512];
char rcvdPayload[512];
//wifi parameter
const char* ssid = "Realmadrid";
const char* password = "kingofmadrid";
>>>>>>> cb8c97d37699d83a306b13d15d499465a31bb452
int status = WL_IDLE_STATUS;
//motor parameter
const int fanINA = 33;
const int fanINB = 32; 
const int mobileINA = 15;
const int mobileINB = 2;
int motorSpeed = 0, isMobileON = 0, isFanON = 0;
String fan="OFF";
String mobile="OFF";
struct FanMobileValues {
  const char *fanValue;
  const char *mobileValue;
};



// JSON 파싱 및 fan과 mobile 값을 반환하는 함수
FanMobileValues json_parser(const char *json) {
  // JSON 객체 생성
  DynamicJsonDocument doc(1024);

  // JSON 문자열 파싱
  deserializeJson(doc, json);

  // fan과 mobile 값을 구조체에 저장
  FanMobileValues values;
  values.fanValue = doc["state"]["fan"];
  values.mobileValue = doc["state"]["mobile"];

  return values;
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload,payLoad,payloadLen);
  rcvdPayload[payloadLen] = 0;
  //FanMobileValues result = json_parser(rcvdPayload);

  Serial.print("Subscribe Messege");
  Serial.println(rcvdPayload);
  //fan handle
  //if (result.fanValue == "ON") isFanON = 1;
  //else if (result.fanValue == "S") isFanON = 2;
  //else isFanON = 0;
  
  //mobile handle
  //if (result.mobileValue == "ON") isMobileON = 1;
  //else isMobileON = 0;
  if (strstr(rcvdPayload,"mobile") != nullptr) 
  {
    if (strstr(rcvdPayload, "ON") != nullptr) isMobileON = 1;
    else if(strstr(rcvdPayload, "OFF") != nullptr)isMobileON = 0;
    else Serial.println("no mobile value");
  }
  else if (strstr(rcvdPayload,"fan") != nullptr) 
  {
    if (strstr(rcvdPayload, "OFF") != nullptr) isFanON = 0;
    else if (strstr(rcvdPayload, "ON") != nullptr) isFanON = 1; 
    else Serial.println("no fan value");
  }
  else Serial.println("no payload");
}

//{"version": "","timestamp": ,"state": {"fan": "ON","mobile": "OFF"},"metadata": {"fan": {"timestamp": 1701951187},"mobile": {"timestamp": 1701951187}}}
void motorStatePublish()
{
  sprintf(payload,"{\"state\":{\"reported\":{\"fan\":\"%s\",\"mobile\":\"%s\"}}}", fan, mobile);
  if (motor_client.publish(pTOPIC_NAME,payload)==0) {
    Serial.print("Publish Message: ");
    Serial.println(payload);
  } else Serial.println("Publish failed");
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  pinMode(fanINA, OUTPUT);
  pinMode(fanINB, OUTPUT);
  pinMode(mobileINA, OUTPUT);
  pinMode(mobileINB, OUTPUT);

  //wifi - connection
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
  //mobile 속도 조절
  if (isMobileON) {
    motorSpeed = 255;
    mobile = "ON";
    analogWrite(mobileINA, 0);
    analogWrite(mobileINB, motorSpeed);
    Serial.println("1-m");
    //motorStatePublish();
  }
  else {
    motorSpeed = 0;
    mobile = "OFF";
    analogWrite(mobileINA, 0);
    analogWrite(mobileINB, motorSpeed);
    Serial.println("0-m");
    //motorStatePublish();
  }
  //fan모터 속도 조절
  switch(isFanON){
    case 0:
      motorSpeed = 0;
      fan = "OFF";
      analogWrite(fanINA, 0);
      analogWrite(fanINB, motorSpeed);
      Serial.println("0-f");
      motorStatePublish();
      break;
    case 1:
      motorSpeed = 255;
      fan = "ON";
      analogWrite(fanINA, 0);
      analogWrite(fanINB, motorSpeed);
      Serial.println("1-f");
      motorStatePublish();
      break;
    case 2:
      motorSpeed = 200;
      fan = "ON";
      analogWrite(fanINA, 0);
      analogWrite(fanINB, motorSpeed);
      Serial.println("2-f");
      motorStatePublish();
      break;
    default:
      motorSpeed = 0;
      fan = "OFF";
      analogWrite(fanINA, 0);
      analogWrite(fanINB, motorSpeed);
      Serial.println("defalut");
      motorStatePublish();
      break;
  }
  delay(1500);
}