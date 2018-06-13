#include <EEPROM.h>

#define IR_Right A5
#define IR_Mid A4
#define IR_Left A3

// 1 --> Left Motor
// 2 --> Right Motor

#define E1 10  // Enable Pin for motor 1
#define E2 11  // Enable Pin for motor 2

#define I1 8  // Control pin 1 for motor 1
#define I2 9  // Control pin 2 for motor 1
#define I3 12  // Control pin 1 for motor 2
#define I4 13  // Control pin 2 for motor 2

#define Red_Led 6

#define reset 7

//....................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................


short Rval, Mval, Lval;
short Last_Rval,Last_Mval,Last_Lval;
short Number_Of_Barriers,Current_Cell,New_Cell,Head,Width,Hight,Sent_Direction,Counter=1;
bool Barrier[64][4],Visited[64];    //URDL ---> 0123, Barrier[Cell_ID][Direction] = 1 -> Then there is a barrier here;
bool Turning = false,Moving = false,start=false,Done = false;
char recieved;
int velocity=2;
void Read_Input()
{
Width = 5,Hight = 6;
}
void Define_Fixed_Boundries()
{
  short cell = 0;
    for(short i=0; i<Width; ++i,cell++)
    {
        Barrier[cell][2] = true;  //The first row.
        Barrier[cell+(Hight-1)*Width][0] = true;  //The last  row.
    }
    cell = 0;
    for(short i=0; i<Hight; ++i,cell+=Width)
    {
        Barrier[cell][3] = true;  //The first column.
        Barrier[cell+Width-1][1] = true;  //The last column.
    }
}
void Read_From_Sensors()
{
  Rval = analogRead(IR_Right);
  Lval = analogRead(IR_Left);
  Mval = analogRead(IR_Mid);
}
bool IsBlack(short x) { return (x>=200); }
void Stop(){ analogWrite(E1, 0),analogWrite(E2, 0); }


short Direction_Detector() // The opposite direction can be validate here.
{
 recieved='z';
 if(Serial.available() > 0){     
      recieved = Serial.read(); 
      start=1;  
    }
    
   if(recieved=='F')Sent_Direction=0;
   else if(recieved=='R')Sent_Direction=1;
   else if(recieved=='B')Sent_Direction=2;
   else if(recieved=='L')Sent_Direction=3;
   else if(recieved=='0')velocity=0;
   else if(recieved=='1')velocity=1;
   else if(recieved=='2')velocity=2;
   else if(recieved=='3')velocity=3;
   else if(recieved=='4')velocity=4;
}
void Turn(short dr)  //Must do the whole turning here because there is an operation will start after the turning is done.
{
    if(dr == 1) analogWrite(E1, 110),analogWrite(E2, 0);
    else analogWrite(E2, 110),analogWrite(E1, 0);
    delay(500);
    do
    {
        Read_From_Sensors();
    }
    while(!IsBlack(Mval)&&!IsBlack(Lval)&&!IsBlack(Rval));
    Stop();
    delay(100);
}
void Move_Straight()
{
    short L,R,M,v=80; 
    if(!IsBlack(Mval) && !IsBlack(Lval) && !IsBlack(Rval)) {
      M = Last_Mval,R = Last_Rval,L = Last_Lval;
      v=60;
    }
    else M = Mval,R = Rval,L = Lval;
   
    if((IsBlack(M) && !IsBlack(L) && !IsBlack(R)) || (IsBlack(M) && IsBlack(L) && IsBlack(R)))   //Move straight.
    {
        analogWrite(E1, 110);
        analogWrite(E2, 110);
    }
    else if(!IsBlack(M) && !IsBlack(R) && IsBlack(L))  //Small turn to left
    {
        analogWrite(E1, v);
        analogWrite(E2, 110);
        delay(10);
    }
    else if(!IsBlack(M) && !IsBlack(L) && IsBlack(R))    //Small turn to right
    {
        analogWrite(E1, 110);
        analogWrite(E2, v);
        delay(10);
    }
}
void Setting_New_Cell(short dr)
{
  if(dr == 1) New_Cell = Current_Cell + 1;
  else if(dr == 3) New_Cell = Current_Cell - 1;
  else if(dr == 0) New_Cell = Current_Cell + Width;
  else if(dr == 2) New_Cell = Current_Cell - Width;
}
void Calculate_Next_Move()
{
  if(Sent_Direction != Head && abs(Head-Sent_Direction) != 2 && !Barrier[Current_Cell][Sent_Direction])   // The second condition handling the opposite direction.
      {
          if((Head+1)%4==Sent_Direction) Turn(1);  //Turn Right, remember 1->Right
          else Turn(3); //Turn Left
          
          Setting_New_Cell(Sent_Direction);
          Head = Sent_Direction;
          Moving = true; Move_Straight(); delay(200);
      }
      else //Moving On, If it can !
      {
        if(!Barrier[Current_Cell][Head])  
        {
          Moving = true; Move_Straight(); delay(200);
          Setting_New_Cell(Head);
        }
        else Moving = false;
      }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  pinMode(reset,INPUT);
   
  pinMode(IR_Right,INPUT); pinMode(IR_Mid,INPUT); pinMode(IR_Left,INPUT);

  pinMode(E1, OUTPUT); pinMode(E2, OUTPUT);

  pinMode(I1, OUTPUT); pinMode(I2, OUTPUT);
  pinMode(I3, OUTPUT); pinMode(I4, OUTPUT);

  pinMode(Red_Led, OUTPUT);

  Read_Input();

  Define_Fixed_Boundries();

  Current_Cell=EEPROM.read(200);
  New_Cell = EEPROM.read(200);
  Head = EEPROM.read(250); 
  
  for(int i=0;i<64;i++){
    Visited[i]=EEPROM.read(300+i);
  }
  Visited[0]=1;
  Counter=1;
}

int i=0;

void loop() 
{
    digitalWrite(I1, HIGH);
    digitalWrite(I2, LOW);
    digitalWrite(I3, HIGH);
    digitalWrite(I4, LOW);

 /*if(!digitalRead(reset)){
  i=(i+3)%14;
 Serial.println(i);
 delay(500);
 }
 Serial1.write(i);
 
 return;
 */

    if(!digitalRead(reset)){
        Serial.println("here");
        Current_Cell=0;
        Head =0;
        start=0;
        Stop();
        Moving=0;
        Counter=1;
        Visited[0]=1;

        digitalWrite(Red_Led,LOW);
        EEPROM.write(200,Current_Cell);
        EEPROM.write(250,Head);
         for(int i=0;i<64;i++){
             EEPROM.write(300+i,0);
        }
        delay(2000);
      }
      

    if(Done)    //Here The Game is over.
    {
      Stop();
      digitalWrite(Red_Led,HIGH);
      return;
    }
    else digitalWrite(Red_Led,LOW);
    
  Direction_Detector();
  
  Read_From_Sensors();

  if(!start) return;
  
  if( IsBlack(Rval) && IsBlack(Lval) && IsBlack(Mval) )   //Reached a cell.
    {
      Current_Cell = New_Cell;
      EEPROM.write(200,Current_Cell);
      EEPROM.write(250,Head);
            
      if(!Visited[Current_Cell]) Counter++;   //If new cell, then increase the counter.
      Visited[Current_Cell] = true;    //Now make it visited.
      EEPROM.write(300+Current_Cell,1); 
      
      Moving = false;
    }

    Serial.println(Counter);
    if(Counter==Width*Hight)
    {
      Done = true;
      return;
    }
    
    if(Moving) Move_Straight();
    
    else Stop();

     if(!Moving)   //If the car is not moving, then check for the next move .  Remember that the car stops in every cell -just a little stop-.
    {
       Calculate_Next_Move();
    }
    
    if(IsBlack(Mval) || IsBlack(Lval) || IsBlack(Rval))
    {
      Last_Lval = Lval;
      Last_Mval = Mval;
      Last_Rval = Rval;
    }

  Serial1.write(Current_Cell);

}

