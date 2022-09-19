// Adjustable variables

float maxspeed = 0.3; // Maximum Speed Percentage - allows user to cap the speed for safety

int rate = 10; // How many times a second the drone updates

int escpin[4] = {9, 10, 11, 12}; // ESC Pins

int freq = 9600; // Serial Frequency

//--------------------------------------------------------

#include <Wire.h>
#include <Servo.h>

Servo esc[4]; // ESCs

float minspeed = 0.1; // Minimum Speed Percentage sent to the motors - enough to spin consistantly

int pwmrange[2] = {1000, 2000}; //Range of PWM where the motors operate

int mpu = 0x68; // MPU6050 address

float pt; // Previous Time - Time at which elapsed time was last calculated
float ct; // Current Time
float et; // Elapsed Time - Time that has past since last defined

int minmpu = 265; // Minimum MPU6050 output value - used for MPU6050 mapping
int maxmpu = 402; // Maximum MPU6050 output value - used for MPU6050 mapping

int escsignal[4]; // Signal sent to ESCs in terms of percentage of power

short raw[3]; // Raw output of MPU6050
short angle[3]; // Processed angle data from MPU6050

double prevangle[3]; // Previous processed angle data from MPU6050
double curangle[3]; // Current processed angle data from MPU6050

double vel[3]; // Angular Velocity

//Initalise Arduino
void setup(){
    Wire.begin();
    Wire.beginTransmission(mpu);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(freq);
    for (int i = 0; i < 4; i++){
        esc[i].attach(escpin[i], pwmrange[0], pwmrange[1]);
    }
}

// Collecting and processing MPU6050 data
void mpudataprocessing(){
    Wire.beginTransmission(mpu);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu, 14, true);

    for (int i = 0; i < 3; i++){
        raw[i] = Wire.read() << 8 | Wire.read();
        raw[i] = map(raw[i], minmpu, maxmpu, -90, 90);
    }

    angle[0] = RAD_TO_DEG * (atan2(-raw[1], -raw[2])+PI);
    angle[1] = RAD_TO_DEG * (atan2(-raw[0], -raw[2])+PI);
    angle[2] = RAD_TO_DEG * (atan2(-raw[0], -raw[1])+PI);

    pt = ct;
    ct = millis();
    et = (ct - pt)/1000;

    for (int i = 0; i < 3; i++){
        // Keep 
        if (angle[i] > 180){
            angle[i] = angle[i] - 360;
        }
        prevangle[i] = curangle[i];
        curangle[i] = angle[i];
        vel[i] = (curangle[i] - prevangle[i])/et;
    }
}

// Calculate pwm signal that needs to be sent to each motor based off the processed MPU6050 data (PID calculation)
void pwmcalculation(){
    for (int i = 0; i < 4; i++){
        esc[i].write(escsignal[i]*180);
    }
}

// Print angle values in serial - used for testing and debugging
void printangle(){
    Serial.print('Angle = ');
    Serial.print(angle[0]);
    Serial.print(' / ');
    Serial.print(angle[1]);
    Serial.print(' / ');
    Serial.println(angle[2]);
}

// Main code
void loop(){
    mpudataprocessing();
    printangle();
    delay(1000/rate);
}