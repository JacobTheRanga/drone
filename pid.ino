float maxspeed = 0.3; // Maximum Speed Percentage - allows user to cap the speed for safety

int rate = 10; // How many times a second the drone updates

int flpin = 9; // Front Left Motor Pin
int frpin = 10; // Front Right Motor Pin
int blpin = 11; // Back Left Motor Pin
int brpin = 12; // Back Right Motor Pin

int freq = 9600; // Serial Frequency

//--------------------------------------------------------

#include <Wire.h>
#include <Servo.h>

Servo fl; // Front Left Motor
Servo fr; // Front Right Motor
Servo bl; // Back Left Motor
Servo br; // Back Right Motor

float minspeed = 0.1; // Minimum Speed Percentage sent to the motors - enough to spin consistantly

int pwmrange[2] = {1000, 2000}; //Range of PWM where the motors operate

int mpu = 0x68; // MPU6050 address

float pt; // Previous Time - Time at which elapsed time was last calculated
float ct; // Current Time
float et; // Elapsed Time - Time that has past since last defined

int minmpu = 265; // Minimum MPU6050 output value - used for MPU6050 initalisation
int maxmpu = 402; // Maximum MPU6050 output value - used for MPU6050 initalisation

int flspeed; // Front Left Motor Speed Percentage
int frspeed; // Front Right Motor Speed Percentage
int blspeed; // Back Left Motor Speed Percentage
int brspeed; // Back Right Motor Speed Percentage

short raw[3]; // Raw output of MPU6050
short mapped[3]; // Mapped output of MPU6050

double x; // X-Axis Angle
double y; // Y-Axis Angle
double z; // Z-Axis Angle

double pmapped[3]; // Previous Mapped MPU6050
double cmapped[3]; // Current Mapped MPU6050

double vel[3]; // Raw Angular Velocity

double xvel; // X-Axis Velocity
double yvel; // Y-Axis Velocity
double zvel; // Z-Axis Velocity

//Initalise Arduino
void setup(){
    Wire.begin();
    Wire.beginTransmission(mpu);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(freq);
    fl.attach(flpin, pwmrange[0], pwmrange[1]);
    fr.attach(frpin, pwmrange[0], pwmrange[1]);
    bl.attach(blpin, pwmrange[0], pwmrange[1]);
    br.attach(brpin, pwmrange[0], pwmrange[1]);
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

    mapped[0] = RAD_TO_DEG * (atan2(-raw[1], -raw[2])+PI);
    mapped[1] = RAD_TO_DEG * (atan2(-raw[0], -raw[2])+PI);
    mapped[2] = RAD_TO_DEG * (atan2(-raw[0], -raw[1])+PI);

    pt = ct;
    ct = millis();
    et = (ct - pt)/1000;

    for (int i = 0; i < 3; i++){
        if (mapped[i] > 180){
            mapped[i] = mapped[i] - 360;
        }
        pmapped[i] = cmapped[i];
        cmapped[i] = mapped[i];
        vel[i] = (cmapped[i] - pmapped[i])/et;
    }

    xvel = vel[0];
    yvel = vel[1];
    zvel = vel[2];

    x = mapped[0];
    y = mapped[1];
    z = mapped[2];
}

// Calculate pwm signal that needs to be sent to each motor based off the processed MPU6050 data (PID calculation)
void pwmcalculation(){
    Serial.print('AHHHHHHHHHH');
}

// Main code
void loop(){
    mpudataprocessing();
    pwmcalculation();
    fl.write(flspeed*180);
    fr.write(frspeed*180);
    bl.write(blspeed*180);
    br.write(brspeed*180);
    delay(1000/rate);
}