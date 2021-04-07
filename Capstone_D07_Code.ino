
#include <SPI.h>
#include<Pixy2.h>
#include<Pixy2CCC.h>

Pixy2 pixy;
int leftpin1=2;
int leftpin2=3;
int rightpin1=4;
int rightpin2=5;

int sonicTx = 8;
int sonicRx = 9;

int button = 7;
//int8_t setLamp(uint8_t upper, uint8_t lower);
//
//uint8_t x = 0x0000001;
//uint8_t y = 0x0000000;

//////////////////////////////////
//Pixy API parameters
int count = 0;
float CenterZone = 0.35;
//int signature, x, y, width, height;
//float center_x, center_y, area;
//////////////////////////////////
//Ultrasonic API parameters
long duration;
float distance;
float v_sound = 0.034; // unit cm/us
///////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("pixystarting..\n");
  pixy.init();
  pixy.changeProg("color_connected_components");
  pinMode(leftpin1,OUTPUT);
  pinMode(leftpin2, OUTPUT);
  pinMode(rightpin1,OUTPUT);
  pinMode(rightpin2, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  pinMode(sonicTx, OUTPUT);
  pinMode(sonicRx, INPUT);

}


struct Data{
  int get_blocks;
  int signature, x, y, width, height;
  float center_x, center_y, area;
};


void loop() {
  // put your main code here, to run repeatedly:
  
  struct Data real_data = check();
  float turn = real_data.center_x;
  float area = real_data.area;


  if (digitalRead(button) == true){
    pixy.setLamp(1,0);
    //Clear Tx condition
    digitalWrite(sonicTx, LOW); 
    delayMicroseconds(2);
  }
  else{
    pixy.setLamp(0,0);
  }
  
  if(turn > -CenterZone && turn < CenterZone){//object in CenterZone, do nothing.
    turn = 0;
  }
  
  //Start the sensor
  digitalWrite(sonicTx,HIGH);
  delayMicroseconds(10);
  digitalWrite(sonicTx,LOW);
  //Get the echo
  duration = pulseIn(sonicRx,HIGH);

  //Calculate the distance
  distance = duration * v_sound/2;
  Serial.print("Distance:");
  Serial.print(distance);
  Serial.println(" cm");
  
  if(distance >= 35 && distance <= 70){
    if (real_data.get_blocks){
      if(turn < 0){
        turnLeft(255);
      }else if(turn > 0){
         turnRight(255);
      }else{
          forward(255);
        }
      }
  }else if(distance < 30){
      //object is out of the range(30cm, 70cm)
      backward(255);
      //if object is out of 70cm, then there is nothing we can do.
  }else{
      digitalWrite(leftpin1, LOW);//idle
      digitalWrite(leftpin2, LOW);
      digitalWrite(rightpin1, LOW);
      digitalWrite(rightpin2, LOW);
  }
}

   


void forward(int delay_time){
   int delay_finish1;
  if (digitalRead(button) == true) {
    digitalWrite(leftpin1, LOW);
    digitalWrite(leftpin2, HIGH);
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, HIGH);
    delay_finish1 = delay_motor(delay_time); //check if delay is interrupted by the user.
    if(delay_finish1 == 0){return;}
  }
  else {
    digitalWrite(leftpin1, LOW);
    digitalWrite(leftpin2, LOW);
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
  }
}


void backward(int delay_time){
   int delay_finish1;
  if (digitalRead(button) == true) {
    digitalWrite(leftpin1, HIGH);
    digitalWrite(leftpin2, LOW);
    digitalWrite(rightpin1, HIGH);
    digitalWrite(rightpin2, LOW);
    delay_finish1 = delay_motor(delay_time); //check if delay is interrupted by the user.
    if(delay_finish1 == 0){return;}
  }
  else {
    digitalWrite(leftpin1, LOW);
    digitalWrite(leftpin2, LOW);
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
  }
}


int delay_motor(int delay_time){
  for(int i=0; i<delay_time; i++){
      if(digitalRead(button)==false){return 0;}//check if delay is interrupted by the user. Return 0 if interrupted.
      delay(1);
  }
  return 1;//return 1 if there is no interruption.
}


void turnLeft(int delay_time){
  int delay_finish;
  if(digitalRead(button) == true){
    digitalWrite(leftpin1, LOW);//only left wheel spins
    digitalWrite(leftpin2, LOW);
    
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, HIGH);
    delay_finish = delay_motor(delay_time);//check if delay is interrupted by the user. Will return 1 if not interrupted.
    if(delay_finish == 0){return;}
    
  } else {
    digitalWrite(leftpin1, LOW);//both wheels stop
    digitalWrite(leftpin2, LOW);
    
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
  }
}


void turnRight(int delay_time){
  int delay_finish;
  if(digitalRead(button) == true){
    digitalWrite(leftpin1, LOW);//only left wheel spins
    digitalWrite(leftpin2, HIGH);
    
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
    delay_finish = delay_motor(delay_time);//check if delay is interrupted by the user. Will return 1 if not interrupted.
    if(delay_finish == 0){return;}
    
  } else {
    digitalWrite(leftpin1, LOW);//both wheels stop
    digitalWrite(leftpin2, LOW);
    
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
  }
}


struct Data check(){
  static int i = 0;
  int j;

  float center_x, center_y; 
  
  uint16_t blocks;
  char buf[32];
  blocks = pixy.ccc.getBlocks();
  
  struct Data data;
  if(blocks){data.get_blocks = 1;}else{data.get_blocks = 0;}

  if (blocks)
  {
    //signature = pixy.ccc.blocks[0].m_signature;
    data.height = pixy.ccc.blocks[0].m_height;
    data.width = pixy.ccc.blocks[0].m_width;
    data.x = pixy.ccc.blocks[0].m_x;
    data.y = pixy.ccc.blocks[0].m_y;
    center_x = (data.x + (data.width/2));
    center_y = (data.y + (data.height/2));

    data.center_x = mapfloat(center_x, 0, 316, -1, 1);
    data.center_y = mapfloat(center_y, 0, 208, -1, 1);
    data.area = data.width * data.height;

  }else{
    digitalWrite(leftpin1, LOW);//both wheels stop
    digitalWrite(leftpin2, LOW);
    
    digitalWrite(rightpin1, LOW);
    digitalWrite(rightpin2, LOW);
  }
  return data;
}


float mapfloat(long x, long in_min, long in_max, long out_min, long out_max){
  return (float)(out_min + ((out_max - out_min) / (float)(in_max - in_min)) * (x - in_min));
}
