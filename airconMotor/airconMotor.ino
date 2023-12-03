#include <AWS_IOT.h>
#include <WiFi.h>

AWS_IOT motor_client;

const char* ssid = "AlpaVirus";
const char* password = "imbumjjang";
char HOST_ADDRESS[]="a2ichw3atwh8od-ats.iot.ap-northeast-2.amazonaws.com";
char CLIENT_ID[]= "YIB_ESP32";
char sTOPIC_NAME[]= ""; // subscribe topic name
char pTOPIC_NAME[]= "esp32/dcMotor";
char payload[512];
char rcvdPayload[512];

int status = WL_IDLE_STATUS;
const int INA = 15; //motor pinA
const int INB = 2;  //motor pinB
int motorSpeed = 0, power = 0;

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload,payLoad,payloadLen);
  rcvdPayload[payloadLen] = 0;

  Serial.print("Subscribe Messege");
  Serial.println(rcvdPayload);
  if (strstr(rcvdPayload,"OFF") != nullptr) power = 0;
  else if (strstr(rcvdPayload,"ON") != nullptr) power = 1;
  else if (strstr(rcvdPayload,"level2") != nullptr) power = 2;
}

void dcMotorPublish() {
  sprintf(payload,"{\"state\":{\"reported\":{\"isOn\":\"%d\"}}}", isOn);
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
  switch(power){
    case 0:
      motorSpeed = 0;
      analogWrite(INA, 0);
      analogWrite(INB, motorSpeed);
    case 1:
      motorSpeed = 150;
      analogWrite(INA, 0);
      analogWrite(INB, motorSpeed);
    case 2:
      motorSpeed = 200;
      analogWrite(INA, 0);
      analogWrite(INB, motorSpeed);
    default:
      motorSpeed = 0;
      analogWrite(INA, 0);
      analogWrite(INB, motorSpeed);
  }
  delay(1000);
}
