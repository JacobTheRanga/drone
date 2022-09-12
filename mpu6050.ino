#include<Wire.h>
#include<Servo.h>

Servo motor1;
Servo motor2;
 
const int mpu=0x68;

float previousTime, currentTime, elapsedTime;

int minVal=265;
int maxVal=402;

int maxpwm = 30;
int maxAngle = 5;

int maxSpeed = 2;

int changeRate = 1;
int rate = 10;

int motor1pwm = 0;
int motor2pwm = 0;

short raw[3];

int gyro[3];
 
double mappedGyro[3], processedGyro[3], prev[3], change[3], vel[3];

double x, y, z, xvel, yvel, zvel;
 
void setup(){
    Wire.begin();
    Wire.beginTransmission(mpu);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(9600);
    motor1.attach(9, 1000, 2000);
    motor2.attach(10, 1000, 2000);
}
void loop(){
    Wire.beginTransmission(mpu);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu,14,true);
    for (int i = 0; i < 3; i++){
        raw[i] = Wire.read()<<8|Wire.read();
        gyro[i] = map(raw[i],minVal,maxVal,-90,90);
    }
    mappedGyro[0] = RAD_TO_DEG * (atan2(-gyro[1], -gyro[2])+PI);
    mappedGyro[1] = RAD_TO_DEG * (atan2(-gyro[0], -gyro[2])+PI);
    mappedGyro[2] = RAD_TO_DEG * (atan2(-gyro[0], -gyro[1])+PI);

    previousTime = currentTime;
    currentTime = millis();
    elapsedTime = (currentTime - previousTime)/1000;

    for (int i = 0; i < 3; i++){
        if (mappedGyro[i] > 180){
            mappedGyro[i] = mappedGyro[i] - 360;
        }
        prev[i] = processedGyro[i];
        processedGyro[i] = mappedGyro[i];
        change[i] = processedGyro[i] - prev[i];
        vel[i] = change[i] / elapsedTime;
    }

    xvel = vel[0];
    yvel = vel[1];
    zvel = vel[2];

    x = processedGyro[0];
    y = processedGyro[1];
    z = processedGyro[2];

    if (x > maxAngle || xvel > maxSpeed){
        if (motor1pwm < maxpwm){
        motor1pwm = motor1pwm +changeRate;
        }
        if (motor2pwm > 0){
        motor2pwm = motor2pwm -changeRate;
        }
    }
    if (x < -maxAngle || xvel < -maxSpeed){
        if (motor2pwm < maxpwm){
        motor2pwm = motor2pwm +change;
        }
        if (motor1pwm > 0){
        motor1pwm = motor1pwm -changeRate;
        }
    }
    motor1.write(motor1pwm);
    motor2.write(motor2pwm);
    Serial.print(motor1pwm);
    Serial.print(' ');
    Serial.println(motor2pwm);
    delay(1000/rate);
}