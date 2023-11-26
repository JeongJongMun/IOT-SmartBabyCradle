#include <Servo.h>

static const int servoPin = 23;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 360; posDegrees+=5) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
    }

    delay(100);

    for(int posDegrees = 360; posDegrees >= 0; posDegrees-=5) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
    }
    
    delay(100);
}