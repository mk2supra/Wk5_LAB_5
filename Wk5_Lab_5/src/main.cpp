#include <Arduino.h>
#include <P1AM.h>

enum MachineStates {
  Waiting,
  ColorSensing,
  CountedMove,
  eject,

};

MachineStates curState = Waiting;

//module variables
int modInput=1;
int modOutput=2;
int modAnalog=3;

//inputs
int pulse=1;
int lbIn=2; //light barrier in
int lbOut=3;
int lbW=4;
int lbR =5;
int lbB=6;

//outputs
int conv=1; //conveyor
int compressor=2;
int ejectW=3;
int ejectR=4;
int ejectB=5;
int armW=6;
int armR=7;
int armB=8;


//analog inputs
int color =1; //color sensor


//variables
int colorValue=10000; //blue was the highest, then red, then white lowest, meaning we have photo resistor before resistor
int distToEject=0;
int distMoved=0;
bool prevKeyState=false;
bool curkey=false;
char targetColor='b';


void setup() {
  //startup P1AM Modules
  delay(1000); //just incase delay
  while(!P1.init()){
  delay(100);
  };
  Serial.begin(9600);
  delay(1000); //just in case delay
}
bool InputTriggered(){
  return !P1.readDiscrete(modInput, lbIn);
}

bool OutputTriggered(){
  return !P1.readDiscrete(modInput, lbOut);
}

void ToggleConveyor(bool s){
  P1.writeDiscrete(s, modOutput, conv); //function that is toggling conveyor CAN SET ON OR OFF
}

int GetColor() {
  return P1.readAnalog(modAnalog, color);
}

bool GetPulseKey(){
  return P1.readDiscrete(modInput,pulse);
}

void ToggleCompressor(bool s){
P1.writeDiscrete(s,modOutput,compressor);
}

// robart arm stuff
//light sensor
bool WhiteReady(){
  return !P1.readDiscrete(modInput, lbW); //false value means something is there
}
bool RedReady(){
  return !P1.readDiscrete(modInput, lbR);
}
bool BlueReady(){
  return !P1.readDiscrete(modInput, lbB);
}
//trigger robart arm




void UseEjector(char c){
  int tempPin;
  if (c == 'w'){
tempPin=ejectW;
  }else if (c == 'r'){
tempPin=ejectR;
  }else {
tempPin=ejectB;

  }
  P1.writeDiscrete(true, modOutput, tempPin);
  delay(1500);
  P1.writeDiscrete(false, modOutput, tempPin);
}

void loop() {

  //testVVV
  //bool isOn = P1.readDiscrete(1,2);//first module second pin
  //Serial.println(isOn);
  //delay(100);

  switch (curState) {
    case Waiting:
      //wait for light barrier to be tripped
      //after tripped go 2 color sense and turn on conveyor
      if (InputTriggered()) {
        curState = ColorSensing;
        ToggleConveyor(true);
        colorValue=10000;
      }
      break;
      case ColorSensing:
      //get color and find min
    colorValue=min(GetColor(),colorValue);
      //Serial.println(colorValue);// lets us find the color value for each of the colors 
      //keep on going until second light barrier
      //then switch states
      // w==1849 r==4216 b==5060
      if (OutputTriggered()){
        curState=CountedMove;
        distMoved=0;
      }
      if(colorValue<2500){
          distToEject=3;
          targetColor='w';
        }else if (colorValue<4600){
         distToEject=9;
         targetColor='r';
        }else{
          distToEject=14;
          targetColor='b';
        }
        ToggleCompressor(true);
      break;

    case CountedMove:
      
    //wathch pulse key to move that far
      curkey=GetPulseKey();
      if (curkey&&!prevKeyState){
        distMoved++;
      }
      prevKeyState=curkey;
      
      //switch states and turn off conveyor when infront of correct ejector
      if (distMoved>=distToEject){
        curState=eject;
        ToggleConveyor(false);
      }
      break;

    case eject:
      UseEjector(targetColor);
      curState = Waiting;
      break;
      default:
      break;
  }

// since the robot arm is a different system I want to have its logic seperate from the sorter

//if light sensor triggered, then trigger robot arm
//when lb false there is somthing there 

if(WhiteReady()){
P1.writeDiscrete(true, modOutput, armW);
Serial.println("White puck there, arm triggered");
}else{
  P1.writeDiscrete(false, modOutput, armW);
  Serial.println("No White Puck ");
};
if(RedReady()){
P1.writeDiscrete(true, modOutput, armR);
Serial.println("Red puck there, arm triggered");
}else{
  P1.writeDiscrete(false, modOutput, armR);
  Serial.println("No RED Puck ");
};
if(BlueReady()){
P1.writeDiscrete(true, modOutput, armB);
Serial.println("Blue puck there, arm triggered");
}else{
  P1.writeDiscrete(false, modOutput, armB);
  Serial.println("No Blue Puck ");
};
 
}
