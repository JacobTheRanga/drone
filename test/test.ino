#include <SoftwareSerial.h>

SoftwareSerial myserial(0, 1);

float num;

void setup(){
    pinMode(13, OUTPUT);
    myserial.begin(9600);
}

void serialInput(){
    if (!myserial.available() > 0) return;
    num = myserial.parseInt();
    if (num == 0) return;
    if (num == 1) {
        digitalWrite(13, HIGH);
        return;
    }
    digitalWrite(13, LOW);
}

void loop(){
    serialInput();
}