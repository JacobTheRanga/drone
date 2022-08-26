#include<Wire.h>
#include<Servo.h>

Servo motor1;
Servo motor2;
 
const int MPU_addr=0x68;
 
int minVal=265;
int maxVal=402;

int maxpwm = 30;
int maxangle = 10;
int change = 1;
int rate = 10;

int motor1pwm = 0;
int motor2pwm = 0;

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
    motor1.attach(9, 1000, 2000);
    motor2.attach(10, 1000, 2000);
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

    if (x > maxangle){
        if (motor1pwm < maxpwm){
        motor1pwm = motor1pwm +change;
        }
        if (motor2pwm > 0){
        motor2pwm = motor2pwm -change;
        }
    }
    if (x < -maxangle){
        if (motor2pwm < maxpwm){
        motor2pwm = motor2pwm +change;
        }
        if (motor1pwm > 0){
        motor1pwm = motor1pwm -change;
        }
    }
    motor1.write(motor1pwm);
    motor2.write(motor2pwm);
    Serial.print(motor1pwm);
    Serial.print(' ');
    Serial.println(motor2pwm);
    delay(1000/rate);
}