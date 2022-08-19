#include<Wire.h>
 
const int MPU_addr=0x68;
 
int minVal=265;
int maxVal=402;

short accel[3];

int gyro[3];
 
double mappedGyro[3];

double x;
double y;
double z;
 
void setup(){
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(9600);
}
void loop(){
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);
    for (int i = 0; i < 3; i++){
        accel[i] = Wire.read()<<8|Wire.read();
    }
    for (int i = 0; i < 3; i++){
        gyro[i] = map(accel[i],minVal,maxVal,-90,90);
    }
    mappedGyro[0] = RAD_TO_DEG * (atan2(-gyro[1], -gyro[2])+PI);
    mappedGyro[1] = RAD_TO_DEG * (atan2(-gyro[0], -gyro[2])+PI);
    mappedGyro[2] = RAD_TO_DEG * (atan2(-gyro[0], -gyro[1])+PI);
    
    for (int i = 0; i < 3; i++){
        if (mappedGyro[i] > 180){
            mappedGyro[i] = mappedGyro[i] - 360;
        }
    }

    x = mappedGyro[0];
    y = mappedGyro[1];
    z = mappedGyro[2];

    Serial.print(x);
    Serial.print(' ');
    Serial.print(y);
    Serial.print(' ');
    Serial.println(z);
    delay(10);
}