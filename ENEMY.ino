#include <EEPROM.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define IR_Right A5
#define IR_Mid A4
#define IR_Left A3
#define reset 5

typedef struct Node Node;
typedef struct Queue Queue;

// 1 --> Left Motor
// 2 --> Right Motor

#define E1 10  // Enable Pin for motor 1
#define E2 11  // Enable Pin for motor 2

#define I1 8  // Control pin 1 for motor 1
#define I2 9  // Control pin 2 for motor 1
#define I3 12  // Control pin 1 for motor 2
#define I4 13  // Control pin 2 for motor 2

#define Green_Led 4  // Control pin 2 for motor 2

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
struct Node
{
    short cell,lvl;
};
struct Queue
{
    short f, n;
    Queue* base;
    Node* elems;
    void (*Init)(Queue*,short);
    void (*Push)(Queue*,Node);
    Node (*Pop)(Queue*);
    bool (*Empty)(Queue*);
    void (*Destroy)(Queue*);
};
void Initialize(Queue *self,short sz)
{
    self->f = self->n = 0;
    sz*=(sizeof (Node));
    self->elems = (Node*)malloc(sz);
    self->base = self;
}
void destroy(Queue* self)
{
    free(self->elems);
    self->elems = NULL;
}
void push(Queue* self,Node t)
{
    self->elems[self->n++] = t;
}

Node pop(Queue* self)
{
    return self->elems[self->f++];
}

bool empty(Queue* self)
{
    return self->n-self->f == 0;
}
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

short Rval, Mval, Lval;
short Last_Rval,Last_Mval,Last_Lval;
short Number_Of_Barriers,Current_Cell,New_Cell,Head,Width,Hight,Pac_Man_Position;
bool Barrier[64][4],Visited[64];    //URDL ---> 0123, Barrier[Cell_ID][Direction] = 1 -> Then there is a barrier here;
bool Turning = false,Moving = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
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
bool IsBlack(short x)
{
    return (x>=200);
}
void Stop()
{
   digitalRead(E1);digitalRead(E2);
}

void Turn(short dr)  //Must do the whole turning here because there is an operation will start after the turning is done.
{

    Stop();
    delay(100);
    if(dr == 1) analogWrite(E1, 130),analogWrite(E2, 0);
    else analogWrite(E2, 130),analogWrite(E1, 0);
    delay(500);
    do
    {
        Read_From_Sensors();
    }
    while(!IsBlack(Mval)&&!IsBlack(Rval)&&!IsBlack(Lval));
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

    analogWrite(E1, 140);
    analogWrite(E2, 140);

    if(IsBlack(M) && !IsBlack(L) && !IsBlack(R))  //Move straight.
    {
        analogWrite(E1, 140);
        analogWrite(E2, 140);
    }
    else if(!IsBlack(M) && !IsBlack(R) && IsBlack(L))  //Small turn to left
    {
        analogWrite(E1, v);
        analogWrite(E2, 140);
        delay(10);
    }
    else if(!IsBlack(M) && !IsBlack(L) && IsBlack(R))    //Small turn to right
    {
        analogWrite(E1, 140);
        analogWrite(E2,v);
        delay(10);
    }
}
short Getting_New_Cell(short dr,short cur)   //cur = current position.
{
    if(dr == 1) return (cur + 1);
    else if(dr == 3) return (cur - 1);
    else if(dr == 0) return (cur + Width);
    else if(dr == 2) return (cur - Width);
}

short BFS(short Start)
{
    Queue q;
    memset(Visited,0,sizeof Visited);
    q.Init = Initialize;
    q.Push = push;
    q.Pop = pop;
    q.Empty = empty;
    q.Destroy = destroy;
    q.Init(&q,Width*Hight*5);

    Node curr = {Start,0};
    q.Push(&q,curr);
    Visited[Start] = true;
    Node tmp;
    while(!q.Empty(&q))
    {
        curr = q.Pop(&q);
        if(curr.cell == Pac_Man_Position)
        {
            q.Destroy(&q);
            return curr.lvl;
        }
        for(short i=0; i<4; ++i)
        {
            tmp.cell = Getting_New_Cell(i,curr.cell);
            if(!Barrier[curr.cell][i] && !Visited[tmp.cell])
            {
                tmp.lvl = curr.lvl+1;
                q.Push(&q,tmp);
                Visited[tmp.cell] = true;
            }
        }
    }
    return 32000;
}
int tmp;
void setup()
{
    Serial.begin(9600);

    pinMode(IR_Right,INPUT);
    pinMode(IR_Mid,INPUT);
    pinMode(IR_Left,INPUT);
    
    pinMode(reset,INPUT);
    
    pinMode(E1, OUTPUT);
    pinMode(E2, OUTPUT);

    pinMode(I1, OUTPUT);
    pinMode(I2, OUTPUT);
    pinMode(I3, OUTPUT);
    pinMode(I4, OUTPUT);

    pinMode(Green_Led, OUTPUT);
    Current_Cell=EEPROM.read(200);
    Head = EEPROM.read(250); 
    Read_Input();
    Define_Fixed_Boundries();
}

bool start=0;

void loop()
{
      
    if(!digitalRead(reset)){
      Serial.println("here");
      Current_Cell=Width*Hight -1;
      Head =2;
      start=0; Moving = false; Pac_Man_Position = 0;
      Stop();
      digitalWrite(Green_Led,LOW);
      EEPROM.write(200,Current_Cell);
      EEPROM.write(250,Head);
      delay(5000);
    } 

     if(Current_Cell == Pac_Man_Position)
    {
        digitalWrite(Green_Led,HIGH);
        Stop();
        return;
    }
    else{
       digitalWrite(Green_Led,LOW);
    }
    
    if(Serial.available() > 0) 
    {
      start=1;  // If the car receives any data then the target start moving. So, the car should move now!
      tmp = Serial.read();
      
    }

    
    if(!start)return;
    
    digitalWrite(I1, HIGH);
    digitalWrite(I2, LOW);
    digitalWrite(I3, HIGH);
    digitalWrite(I4, LOW);

    
   
    
    if(Moving)
    {
        Read_From_Sensors();
        if( IsBlack(Rval) && IsBlack(Lval) && IsBlack(Mval) )
        {
          
            Stop();
            Moving = false;
            Pac_Man_Position = tmp; //If the target sent any new data, save it. Else move to the saved target position.
            Current_Cell = New_Cell;
            EEPROM.write(200,Current_Cell);
            EEPROM.write(250,Head);
         Serial.print("Pac_Man_Position : "); 
       Serial.println(Pac_Man_Position); 
        Serial.print("Current_Cell : "); 
         Serial.println(Current_Cell);  
         Serial.println("");     
        }
        else Move_Straight();

    }
    else  
    {
       // If we are here, then the car is not moving.So,we should get the new target cell.
      Pac_Man_Position = tmp;
       
        short mn = 32000,dr = 0;
        for(short i=0; i<4; ++i)
        {
            if(!Barrier[Current_Cell][i] && abs(Head-i) != 2)   // The second condition handling the opposite direction.
            {
                short cost = BFS(Getting_New_Cell(i,Current_Cell));
                if(cost<mn) mn = cost,dr = i;
            }
        }

        New_Cell = Getting_New_Cell(dr,Current_Cell);

        if(Head != dr) //Need to turn.
        {
            if((Head+1)%4 == dr) Turn(1);  //Turn Right, remember 1->Right
            else Turn(3); //Turn Left
            Head = dr;
        }
        Moving = true;
         Move_Straight();
        delay(300);
       
    }
    if(IsBlack(Mval) || IsBlack(Lval) || IsBlack(Rval))
    {
      Last_Lval = Lval;
      Last_Mval = Mval;
      Last_Rval = Rval;
    }
}

