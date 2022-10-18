// Adjustable variables

double maxpower[4] = {0.2, 0.2, 0.2, 0.2}; // Maximum power Percentage - allows user to cap the power for safety
double target = 0.2; // Power you wish to achieve

double setpoint[3] = {0, 0, 0}; // Destination in degrees

double gain[3] = {0.01, 0.01, 0.01}; // Motor power multiplier || Proportional / Integral / Derivitive

int rate = 10; // How many times a second the drone updates

int escpin[4] = {9, 10, 5, 6}; // ESC Pins

int freq = 9600; // Serial Frequency

// Hardware specific variables

double minpower[4] = {0.15, 0.15, 0.15, 0.15}; // Minimum power Percentage sent to the motors - enough to spin consistantly

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

int mpu = 0x68; // MPU6050 address

int pwmconst = 180; // Constant at which the motorpower is multiplied by

double motorpower[4] = {minpower[0], minpower[1], minpower[2], minpower[3]}; // Motor power in percentage

short raw[3]; // Raw output of MPU6050
double angle[3]; // Processed angle data from MPU6050

double preverr[3]; // Previous processed angle data from MPU6050
double currerr[3] = {0, 0, 0}; // Current processed angle data from MPU6050

double err[3]; // Difference between setpoint and actual point (Proportion)
double errint[3] = {0, 0, 0}; // Amount of error over-correction (Integral)
double errderiv[3]; // Change in error (Derivitive)

double correction[3]; // How much to correct by (PID final calculation)

float num; // Recieved data from serial monitor

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
    delay(2500);
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

    // Turn raw values into radians
    angle[0] = atan2(raw[1], raw[2]);
    angle[1] = atan2(raw[0], raw[2]);
    angle[2] = atan2(raw[0], raw[1]);

    for (int i = 0; i < 3; i++){
        // Keep values between -PI and PI for ease of use
        if (angle[i] > PI){
            angle[i] = angle[i] - (2 * PI);
        }
    }

    //Convert radians to angle
    for (int i; i < 3; i++){
        angle[i] = angle[i] * RAD_TO_DEG;
    }
}

// Ballancing just off the x-axis on a seesaw
void seesaw(){
    //  Map correction to power of the motors
    if (correction[0] >= setpoint[0]){
        motorpower[1] = target + correction[0];
        motorpower[0] = target - correction[0];
        return;
    }
    motorpower[0] = target + (correction[0]*-1);
    motorpower[1] = target - (correction[0]*-1);
}

// Limit and convert power values to PWM values and send them to the escs
void powertopwm(){
    for (int i = 0; i < 4; i++){
        // Ensure motors don"t fall below the minimum power except if the power is 0
        if (motorpower[i] < minpower[i] && motorpower[i] != 0){
            motorpower[i] = minpower[i];
        }
        // Ensure motor don"t exceed the maximum power
        if (motorpower[i] > maxpower[i]){
            motorpower[i] =  maxpower[i];
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

    // Calculate change in error value in terms of change in time = de/dt
    for (int i = 0; i < 3; i++){
        preverr[i] = currerr[i];
        currerr[i] = err[i];
        errderiv[i] = (currerr[i] - preverr[i])/rate;

    }

    // Calculate PID - Proportion + Integral + Derivitive
    for (int i = 0; i < 3; i++){
        correction[i] = (err[i] * gain[0]) + (errint[i] * gain[1]) + (errderiv[i] * gain[2]);
    }
}

// Main drone code
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
void changesetpoint(){
    for (int i = 10; i < 13; i++){
        if (i <= num < (i + 1)){
            setpoint[i-10] = (num-i)*1000;
        }
    }
}

// Able to change motor power through serial monitor 
void changepower(){
    for (int i = 0; i < 4; i++){
        if (i <= num < (i + 1)){
            motorpower[i] = num - i;
        }
    }
    if (4 <= num < 5){
        for (int i = 0; i < 4; i++){
                motorpower[i] = num - 4;
        }
    }
}

void readserial(){
    if (!Serial.available()) return;
    num = Serial.parseFloat();
    if (num = 0) return;

    if (0 <= num < 5){
        changepower();
    }
    if (10 <= num < 13){
        changesetpoint();
    }
}

// Main code loop
void loop(){
    mpudataprocessing();
    print(angle, 3);
    delay(1000/rate);
}