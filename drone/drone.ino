// Adjustable variables

double maxpower = 0.4; // Maximum power Percentage - allows user to cap the power for safety
double target = 0.3; // power you wish to achieve

double setpoint[3] = {0, 0, 0}; // Destination in degrees

double gain[3] = {0.01, 0.01, 0.01}; // Motor power multiplier || Proportional / Integral / Derivitive

int rate = 10; // How many times a second the drone updates

int escpin[4] = {9, 10, 11, 12}; // ESC Pins

int freq = 9600; // Serial Frequency

// Hardware specific variables

double minpower = 0.2; // Minimum power Percentage sent to the motors - enough to spin consistantly

int pwmrange[2] = {1000, 2000}; //Range of PWM where the motors operate

int minmpu = 265; // Minimum MPU6050 output value - used for MPU6050 mapping
int maxmpu = 402; // Maximum MPU6050 output value - used for MPU6050 mapping

//---------------------------------------------------------------------------------------------------------------

#include <Wire.h>
#include <Servo.h>

Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;

bool startup = false; // Drone startup

int pwm[4]; // PWM signal getting sent out to escs

int mpu = 0x68; // MPU6050 address

int pwmconst = 180; // Constant at which the motorpower is multiplied by

double pt; // Previous Time - Time at which elapsed time was last calculated
double ct = 0; // Current Time
double et; // Elapsed Time - Time that has past since last defined

double motorpower[4] = {minpower, minpower, minpower, minpower}; // Motor power in percentage

short raw[3]; // Raw output of MPU6050
double angle[3]; // Processed angle data from MPU6050

double preverr[3]; // Previous processed angle data from MPU6050
double currerr[3] = {0, 0, 0}; // Current processed angle data from MPU6050

double err[3]; // Difference between setpoint and actual point (Proportion)
double errint[3] = {0, 0, 0}; // Amount of error over-correction (Integral)
double errderiv[3]; // Change in error (Derivitive)

double correction[3]; // How much to correct by (PID final calculation)

int timer = 0; // Timer

//Initalise Arduino
void setup(){
    Wire.begin();
    Wire.beginTransmission(mpu);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(freq);
    motor1.attach(escpin[0],pwmrange[0],pwmrange[1]);
    motor2.attach(escpin[1],pwmrange[0],pwmrange[1]);
    motor3.attach(escpin[2],pwmrange[0],pwmrange[1]);
    motor4.attach(escpin[3],pwmrange[0],pwmrange[1]);
    motor1.write(0);
    motor2.write(0);
    motor3.write(0);
    motor4.write(0);
    delay(5000);
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

// Ballancing just off the x-axis on a seesaw
void seesaw(){
    //  Map correction to power of the motors
    if (correction[0] >= setpoint[0]){
        motorpower[1] = target + correction[0];
        motorpower[0] = target - correction[0];
    }
    else {
        motorpower[0] = target + (correction[0]*-1);
        motorpower[1] = target - (correction[0]*-1);
    }
}

// Convert power values to PWM values and send them to the escs
void powertopwm(){
    for (int i = 0; i < 4; i++){
        // Ensure motors don"t fall below the minimum power
        if (motorpower[i] < minpower){
            motorpower[i] = minpower;
        }
        // Ensure motor don"t exceed the maximum power
        if (motorpower[i] > maxpower){
            motorpower[i] =  maxpower;
        }
    }
    // Write the signals to the motors
    motor1.write(motorpower[0]*pwmconst);
    motor2.write(motorpower[1]*pwmconst);
    motor3.write(motorpower[2]*pwmconst);
    motor4.write(motorpower[3]*pwmconst);
}

// Main control system
void pidcontrol(){
    // Calculate error value = e
    for (int i = 0; i < 3; i++){
        err[i] = angle[i] - setpoint[i];
    }

    // Calculate change in time = dt
    timer++;
    pt = ct;
    ct = timer;
    et = (ct - pt)/rate;

    // Calculate change in error value in terms of change in time = de/dt
    for (int i = 0; i < 3; i++){
        preverr[i] = currerr[i];
        currerr[i] = err[i];
        errderiv[i] = (currerr[i] - preverr[i])/et;

    }

    // Calculate PID - Proportion + Derivitive + Integral
    for (int i = 0; i < 3; i++){
        correction[i] = (err[i] * gain[0]) + (errint[i] * gain[1]) + (errderiv[i] * gain[2]);
    }
}

// Main drone control code
void drone(){
    mpudataprocessing();
    pidcontrol();
    seesaw();
    powertopwm();
}

// Print function specifically designed for printing arrays - used for testing/debugging/tuning
void print(double var[], int len){
    for (int i = 0; i < len-1; i++){
        Serial.print(var[i]);
        Serial.print(" / ");
    }
    Serial.println(var[len-1]);
}

// Able to change setpoint through serial monitor
void changesetpoint(int axis){
    if (Serial.available()){
        setpoint[axis] = Serial.parseInt();
    }
}

// Able to change motor power through serial monitor
void changepower(int motor){
    if (Serial.available()){
        motorpower[motor] = Serial.parseInt();
    }
}

// Main code loop
void loop(){
    mpudataprocessing();
    pidcontrol();
    print(errderiv, 3);
    delay(1000/rate);
}