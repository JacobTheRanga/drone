// Adjustable variables

float maxspeed = 0.3; // Maximum Speed Percentage - allows user to cap the speed for safety

float setpoint[3] = {0, 0, 0}; // Destination in degrees

float gain[3] = {0.1, 0.1, 0.1}; // PWM signal multiplier || Proportional / Integral / Derivitive

int rate = 10; // How many times a second the drone updates

int escpin[4] = {9, 10, 11, 12}; // ESC Pins

int freq = 9600; // Serial Frequency

//--------------------------------------------------------

#include <Wire.h>
#include <Servo.h>

Servo esc[4]; // ESCs

bool startup = false; // Drone startup

float minspeed = 0.1; // Minimum Speed Percentage sent to the motors - enough to spin consistantly

int pwmrange[2] = {1000, 2000}; //Range of PWM where the motors operate

int mpu = 0x68; // MPU6050 address

float pt; // Previous Time - Time at which elapsed time was last calculated
float ct = 0; // Current Time
float et; // Elapsed Time - Time that has past since last defined

int minmpu = 265; // Minimum MPU6050 output value - used for MPU6050 mapping
int maxmpu = 402; // Maximum MPU6050 output value - used for MPU6050 mapping

int escsignal[4] = {minspeed, minspeed, minspeed, minspeed}; // Signal sent to ESCs in terms of percentage of power

short raw[3]; // Raw output of MPU6050
double angle[3]; // Processed angle data from MPU6050

double preverr[3]; // Previous processed angle data from MPU6050
double currerr[3] = {0, 0, 0}; // Current processed angle data from MPU6050

float err[3]; // Difference between setpoint and actual point (Proportion)
float errint[3]; // Amount of error over-correction (Integral)
float errderiv[3]; // Change in error (Derivitive)

float correction[3]; // How much to correct by

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

    // Read and map raw MPU6050 values
    for (int i = 0; i < 3; i++){
        raw[i] = Wire.read() << 8 | Wire.read();
        raw[i] = map(raw[i], minmpu, maxmpu, -90, 90);
    }

    // Turn raw values into angles
    angle[0] = RAD_TO_DEG * (atan2(-raw[1], -raw[2])+PI);
    angle[1] = RAD_TO_DEG * (atan2(-raw[0], -raw[2])+PI);
    angle[2] = RAD_TO_DEG * (atan2(-raw[0], -raw[1])+PI);

    for (int i = 0; i < 3; i++){
        // Keep angles between -180 and 180 for ease of use
        if (angle[i] > 180){
            angle[i] = angle[i] - 360;
        }
    }
}

// Start up motors
void motorstartup(){
    // Increases PWM signal 10 times a second by 1 for all 4 motors until the minimum speed is hit
    for (int i = 0; i < (minspeed*180); i++){
        for (int a = 0; a < 4; a++){
            esc[a].write(i);
        }
        delay(100);
    }
}

// Integral calculation
void intcalc(){

}

// Derivitive calculation
void derivcalc(){
    // Calculate change in time
    pt = ct;
    ct = millis();
    et = (ct - pt)/1000;

    // Calculate the change in angle (velocity)
    for (int i = 0; i < 3; i++){
        preverr[i] = currerr[i];
        currerr[i] = err[i];
        errderiv[i] = (currerr[i] - preverr[i])/et;

    }
}

// Main control system
void pidcontrol(){
    // Get the error value
    for (int i = 0; i < 3; i++){
        err[i] = angle[i] - setpoint[i];
    }

    // Calculate errors
    intcalc();
    derivcalc();

    // Calculate PID
    for (int i = 0; i < 3; i++){
        correction[i] = (err[i] * gain[0]) + (errint[i] * gain[1]) + (errderiv[i] * gain[2]);
    }

    if (correction[0] >= setpoint[0]){
        escsignal[1] = correction[0];
    }
    else {
        escsignal[0] = correction[0]*-1;
    }

    for (int i = 0; i < 4; i++){
        // Ensure motors don't fall below the minimum speed
        if (escsignal[i] < (minspeed*180)){
            escsignal[i] = minspeed*180;
        }
        // Write the signals to the motors
        esc[i].write(escsignal[i]*180);
    }
}

// Main drone control code
void drone(){
    if (startup == false){
        motorstartup();
    }
    else{
        pidcontrol();
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