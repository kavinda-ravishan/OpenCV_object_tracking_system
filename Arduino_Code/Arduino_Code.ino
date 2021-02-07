#include <Servo.h>

#define BAUD 38400
#define len 6
#define pinX 13
#define pinY 12

Servo servoX;
Servo servoY;

String data;
char schar;
char cdata[6]={'0','0','0','0','0','0'};
int numX=0;
int numY=0;
int posX=90;
int posY=90;

int charToint1(char num){

  if(num == '0') return 0;
  else if(num == '1') return 1;
  else if(num == '2') return 2;
  else if(num == '3') return 3;
  else if(num == '4') return 4;
  else if(num == '5') return 5;
  else if(num == '6') return 6;
  else if(num == '7') return 7;
  else if(num == '8') return 8;
  else if(num == '9') return 9;
  else return 0;
  }

  int charToint10(char num){

  if(num == '0') return 0;
  else if(num == '1') return 10;
  else if(num == '2') return 20;
  else if(num == '3') return 30;
  else if(num == '4') return 40;
  else if(num == '5') return 50;
  else if(num == '6') return 60;
  else if(num == '7') return 70;
  else if(num == '8') return 80;
  else if(num == '9') return 90;
  else return 0;
  }

    void Position(int* pos,int num,char dir,int range0=10,int range1=50,int add1=1,int range2=70,int add2=2,int add3=3){
    if(dir == '+'){
    if(num>range0 && num<=range1 && (*pos + add1)<=180) *pos = *pos + add1;
    else if(num>range1 && num<=range2 && (*pos + add2)<=180) *pos = *pos + add2;
    else if(num>range2 && (*pos + add3)<=180) *pos = *pos + add3;
    else if(*pos + add1>180 || *pos + add2>180 ||*pos + add3>180){
      *pos=180;
      }
    else *pos = *pos;
    }
    else if(dir == '-'){  
    if(num>range0 && num<=range1 && (*pos - add1)>=0) *pos = *pos - add1;
    else if(num>range1 && num<=range2 && (*pos - add2)>=0) *pos = *pos - add2;
    else if(num>range2 && (*pos - add3)>=0) *pos = *pos - add3;
    else if(*pos + add1<0 || *pos + add2<0 ||*pos + add3<0){
      *pos=0;
      }
    else *pos = *pos;
    }
    
    }

void setup() {
  servoX.attach(pinX);
  servoX.write(posX);
  servoY.attach(pinY);
  servoY.write(posY);

  for (short n = 0;n <= 53;n++) {
    pinMode(n, OUTPUT);
    digitalWrite(n, LOW);
  }
  
  Serial.begin(BAUD);
  while (!Serial){
  }  // wait for Serial comms to become ready
}

void loop() {

  if (Serial.available() <= len) {

    data = Serial.readStringUntil('\n');
  }
  
  data.toCharArray(cdata,6);

  numX = charToint10(cdata[0])+charToint1(cdata[1]);
  numY = charToint10(cdata[2])+charToint1(cdata[3]);
  schar = cdata[4];

  switch(schar){
    case 'a':
    Position(&posX,numX,'-');
    Position(&posY,numY,'-');
    break;
    
    case 'b':
    Position(&posX,numX,'-');
    Position(&posY,numY,'+');
    break;
    
    case 'c':
    Position(&posX,numX,'+');
    Position(&posY,numY,'+');
    break;
    
    case 'd':
    Position(&posX,numX,'+');
    Position(&posY,numY,'-');
    break;
    
    case 'e': 
    posX=90;
    posY=90;
    break;

    case 'l': 
    posX=posX+3;
    break;

    case 'r': 
    posX=posX-3;
    break;

    case 'u': 
    posY=posY-3;
    break;

    case 'p': 
    posY=posY+3;
    break;
    
    default:
    break;
  }

  servoX.write(posX);
  servoY.write(posY);
  cdata[4]={'0'};
  delay(10);

}
