#include "Arduino.h"
#include <Wire.h>
#include <Encoder.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#define SLAVE_ADDRESS 0x04
//initialize global variables
//Pins for encoders
Encoder motorEnc(2, 5);
float speedRate;
int input1 = 3;
int input2 = 6;
int pinState1;
int pinState2;
int newMotor, newSpeed;
int currentMotor;
int currentSpeed;
int lastA = 0;
int lastB = 0;
int t;
//PI variables
float e_past= 0.0;
float u;
float uPrev;
float I= 0.0;
float Tc= millis();
float Ts= 0.0;
//PI variables
//Pins for motor/shield
int output4 = 4;
int output7 = 7;
int output8 = 8;
int output9 = 9;
int output10 = 10;
//Values from MiniProject*********************
float motorSpeed = 2.12;
int num;
int state = 0;
//**********************************************
int input12 = 12;
int ledON = 1;
int previoustime;
float desiredSpeed;
// variables for computation
int counter = 0;
bool docounting = false;
// variables for serial communication
String InputString = ""; // a string to hold incoming data
bool StringComplete = false;
void setup() {
//Setup I2C
pinMode(13, OUTPUT);
// initialize i2c as slave
Wire.begin(SLAVE_ADDRESS);
// define callbacks for i2c communication
Wire.onRequest(sendData);
Wire.onReceive(receiveData);
Serial.println("Ready!");
pinMode(13, OUTPUT);
Serial.begin(115200);
// reserve 200 bytes for the inputString:
InputString.reserve(200);
Serial.println("Ready!"); // Let anyone on the other end of the serial line know that Arduino is
ready
//pin mode for encoders
pinMode(input1, INPUT_PULLUP); // set pin to input with a pullup
pinMode(input2, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(input1), blink, CHANGE); //assign interrupt to input1
//pin mode for motor/shield
pinMode(output4, OUTPUT);
pinMode(output7, OUTPUT);
pinMode(output8, OUTPUT);
pinMode(output9, OUTPUT);
pinMode(output10, OUTPUT);
pinMode(input12, INPUT);
digitalWrite(output4, HIGH);
//delay(5000);
}

void loop() {
// put your main code here, to run repeatedly:
t = millis();
if (StringComplete) {
switch (InputString.charAt(0)) {
case 'S':
docounting = true;
break;
case 'E':
docounting = false;
break;
}
StringComplete = false;
}
// main code
if (docounting) {
counter++;
if (counter >= 100) {
Serial.println("Finished");
docounting = false;
}
}
else {
counter = 0;
}
while(millis()-t < 5){
}

//reads encoders
//----------mechanical encoder---------------
int time1 = millis();
float changeTime= 10000.0*(previoustime-time1)/1000.0;
float changeRad= (newSpeed)/10.0;
desiredSpeed= changeRad/changeTime;

//-----------motor encoder------------------
newMotor = motorEnc.read();
float totalTime= (time1)/1000.0;
//PI Variables
float Kp= 22.3;
float Ke= .192;
float Ki= 154;
//End of PI Variables
float changeMotor= ((newMotor)-(currentMotor));
float motorSpeed= (changeMotor/changeTime)*10000.0;
float speedConstant= 10000.0*(2.0*3.14)/3200.0;
float outputVoltage= 5.0;
speedRate = (speedConstant*motorSpeed)/(10000.0);
float e= (abs(desiredSpeed)-abs(speedRate));
previoustime = time1;

//----------------PI Control Loop------------------------------------------------------------------------------------------------------------
I= I+(Ts*e);
u= 1.0*e+(Ki*I);
if (abs(u) > 255) {
u = (u/abs(u))*255.0;
e= (e/abs(e))*min(255.0,abs(e));
I= (u-1.0*e)/Ki;
}
if (newSpeed>0) {
digitalWrite(output7, HIGH);
}
else{
digitalWrite(output7, LOW);
}
if (newSpeed == 0) {
u= 0;
}
analogWrite(output9, u);
Ts= totalTime-Tc;
Tc= totalTime;

//---------------PI Control Loop----------------------------------------------------------------------------------------------------------------
Serial.print(num);
Serial.print("\t");
Serial.print(state);
Serial.print("\t");
Serial.print(newSpeed);
Serial.print("\t");
Serial.print(speedRate);
Serial.print("\t");
Serial.print(totalTime, 4);
Serial.print("\t");
Serial.print(abs(desiredSpeed));
Serial.print("\t");
Serial.print(u);
Serial.println(" ");
currentMotor = newMotor;
currentSpeed = newSpeed;
}

void blink() {
//pinState is equal to input
pinState1 = digitalRead(input1);
pinState2= digitalRead(input2);
//if pinState doesn't equal the last state of A, counter increases
if ((pinState1 != lastA) && (pinState2 == lastA)){
if (newSpeed < 2550){
newSpeed++;
}
}
else if ((pinState2 != lastB) && (pinState2 == lastA) ){
if (newSpeed < 2550){
newSpeed++;
}}
else if ((pinState1 != lastA)){
if (newSpeed > -2550){
newSpeed--;
}}
else if ((pinState2 != lastB)){
if (newSpeed > -2550) {
newSpeed--;
}}
//last = current pinState
lastA = pinState1;
lastB = pinState2;
}
void serialEvent() {
while (Serial.available()) {
// get the new byte:
char inChar = (char)Serial.read();
// add it to the inputString:
InputString += inChar;
// if the incoming character is a newline, set a flag
// so the main loop can do something about it:
if (inChar == '\n') {
StringComplete = true;
}
}
}
//***********I2C Communication******************
void receiveData(int byteCount){
while(Wire.available()) {
state = Wire.read();
Serial.print("data received: ");
Serial.println(state);
if (state == 1){
newSpeed = uPrev;
analogWrite(output9, u);
}
if (state == 0){
uPrev = newSpeed;
newSpeed = 0;
}
}
}
void sendData(){
num = desiredSpeed * pow(10, 2);
Wire.write(num);
}
