
//Pin Functions
#define motorLeftSpeedPin 5 //pwm
#define motorLeftForwardPin A0
#define motorLeftBackwardPin A1

#define motorRightSpeedPin 6 //pwm
#define motorRightForwardPin A2
#define motorRightBackwardPin A3

#define speakerPin A5
#define ledPin 2

//define commands
#define DEBUG false

#define STOP 0x00 //0000
#define FORWARD_DIRECTION 0x01 //0001
#define BACKWARD_DIRECTION 0x02 //0010 
#define LEFT_DIRECTION 0x04 //0100
#define RIGHT_DIRECTION 0x08 //1000
#define MOTORLEFT 0x10 //0001 0000
#define MOTORRIGHT 0x20 //0010 0000
#define SET_SPEED 0x40 //0100 0000

#define TURN_SPEED_OFFSET 50
#define MINIMUM_MOTOR_SPEED 100
#define MAXIMUM_MOTOR_SPEED 255

struct Motor
{
  byte mSide;
  byte mSpeed;
}motorLeft, motorRight;

struct Command
{
  byte cmdID;
  byte data1;
  byte data2;
  byte checkSum;
};

enum COMMAND_IDS
{
    INVALID_CMD = 0,
    DRIVE = 1,
    THREE_SIXTY = 2
};

byte currentDirection = 0x00;

void dbg_print(const char * s)
{
  
  #if DEBUG
    Serial.println(s);
  #endif
}

void processCommand(struct Command &command)
{
   //prcess recieved command
   switch(command.cmdID)
   {
     case DRIVE:
          dbg_print("Drive ...");
          driveCar(command);
          break;
     default:
       //unknown command and do nothing
       dbg_print("Invalid cmd received...");
       break;
     }
}


void driveCar(struct Command &command)
{
 
 if(command.data1 & STOP){
   stopAllMotors();
   dbg_print("Stop ...");
   return;
 }

 if (command.data1 & LEFT_DIRECTION){
   turnLeft();
   dbg_print("Turn Left ...");
 }else if (command.data1 & RIGHT_DIRECTION){
   turnRight();
   dbg_print("Turn Right ...");
 }else if (currentDirection & (RIGHT_DIRECTION | LEFT_DIRECTION)){
   //reset the direction bits
   Serial.println("Reseting Direction");
   currentDirection &= ~(RIGHT_DIRECTION | LEFT_DIRECTION);
   setAllMotorsSpeed(MAXIMUM_MOTOR_SPEED);
 }
 
 if (command.data1 & FORWARD_DIRECTION){
      goForward();
         
 }else if (command.data1 & BACKWARD_DIRECTION){
   goBackward();
   dbg_print("Drive Backward ...");
 }else{
   stopAllMotors();
   currentDirection &= ~(FORWARD_DIRECTION | BACKWARD_DIRECTION);
 }
 
 
 
 if (command.data1 & SET_SPEED){
   setAllMotorsSpeed(command.data2);
   dbg_print("Set Speed ...");
 }

}

void threeSixtyMode()
{
  Serial.println("360");
  setAllMotorsSpeed(MAXIMUM_MOTOR_SPEED);
  digitalWrite(motorLeftForwardPin,1);
  digitalWrite(motorLeftBackwardPin,0);
  
  digitalWrite(motorRightBackwardPin,1);
  digitalWrite(motorRightForwardPin,0);

}


void turnLeft()
{
    //slow down the left motor to turn right
    if (!(currentDirection & LEFT_DIRECTION)){
      motorLeft.mSpeed-=TURN_SPEED_OFFSET; 
      //motorLeft.mSpeed = 0;
      setMotorSpeed(motorLeft);
      currentDirection |= LEFT_DIRECTION;
    }
}

void turnRight()
{
  //slow down the right motor to turn right
  if (!(currentDirection & RIGHT_DIRECTION)){
    motorRight.mSpeed-=TURN_SPEED_OFFSET; 
    //motorRight.mSpeed = 0;
    setMotorSpeed(motorRight);
    currentDirection |= RIGHT_DIRECTION;
  }
}

void goForward()
{
  //if going backwards then stop motors and then go forward
  if(!(currentDirection & FORWARD_DIRECTION))
  {
    dbg_print("Drive Forward ...\n");
    //stopAllMotors();
    digitalWrite(motorLeftForwardPin,1);
    digitalWrite(motorRightForwardPin,1);
    setMotorSpeed(motorLeft);
    setMotorSpeed(motorRight);
    currentDirection |= FORWARD_DIRECTION; // set forward direction bit
    currentDirection &= ~BACKWARD_DIRECTION; // reset backward direction bit
  }
}

void goBackward()
{
  if(!(currentDirection & BACKWARD_DIRECTION))
  {
	  //if not going backwards then stop motors and start going backward
    //stopAllMotors();
    digitalWrite(motorLeftBackwardPin,1);
    digitalWrite(motorRightBackwardPin,1);
    setMotorSpeed(motorLeft);
    setMotorSpeed(motorRight);
    currentDirection |= BACKWARD_DIRECTION;  // set backward direction bit
    currentDirection &= ~FORWARD_DIRECTION; // reset forward direction bit
  }
}
void stopAllMotors()
{
  //setAllMotorsSpeed(0);
  digitalWrite(motorRightBackwardPin,0);
  digitalWrite(motorLeftBackwardPin,0);
  digitalWrite(motorRightForwardPin,0);
  digitalWrite(motorLeftForwardPin,0);
  delay(200);
}

void setAllMotorsSpeed(byte speedValue)
{
  dbg_print("Setting All Motors Speed");
  if((speedValue <= MAXIMUM_MOTOR_SPEED && speedValue >= MINIMUM_MOTOR_SPEED) || speedValue == 0){
    motorLeft.mSpeed = speedValue;
    motorRight.mSpeed = speedValue;
    setMotorSpeed(motorLeft);
    setMotorSpeed(motorRight);
  }else{
    dbg_print("Motor speed is two high or low");
  }
  
}

void setMotorSpeed(struct Motor &motor)
{
  dbg_print("Setting Motor Speed");
  Serial.println(motor.mSpeed);
  if(motor.mSide == MOTORLEFT){
    analogWrite(motorLeftSpeedPin, motor.mSpeed);
  }else if (motor.mSide == MOTORRIGHT){
    analogWrite(motorRightSpeedPin, motor.mSpeed);
  }else{
    dbg_print("Error Setting Motor Speed");
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(motorLeftSpeedPin, OUTPUT);
  pinMode(motorLeftForwardPin, OUTPUT);
  pinMode(motorLeftBackwardPin, OUTPUT);
  pinMode(motorRightSpeedPin, OUTPUT);
  pinMode(motorRightForwardPin, OUTPUT);
  pinMode(motorRightBackwardPin, OUTPUT); 
  motorLeft.mSpeed = MAXIMUM_MOTOR_SPEED;
  motorRight.mSpeed = MAXIMUM_MOTOR_SPEED;
  motorLeft.mSide = MOTORLEFT;
  motorRight.mSide = MOTORRIGHT;
  
}


void loop()
{
  Command incomingCmd;
  
  if(Serial.available() >= sizeof(Command)){
    //read the incoming data
    dbg_print("incoming data available\n");
    Command *mem = &incomingCmd;
    unsigned char *p = (unsigned char *)mem;
    for(int i=0;i<sizeof(Command);i++)
    {
      unsigned int data = Serial.read();  
      p[i] = data;
    }
      
    
    //verify checksum
     byte received_sum = incomingCmd.cmdID + incomingCmd.data1 + incomingCmd.data2;
     if (incomingCmd.cmdID == DRIVE && received_sum == incomingCmd.checkSum ) {
       //setAllMotorsSpeed(incomingCmd.data2);
       driveCar(incomingCmd);
       dbg_print("Good Cmd - checksum matched\n");
     }else if(incomingCmd.cmdID == THREE_SIXTY){
       if(incomingCmd.data1){
           threeSixtyMode();
       }else{
         stopAllMotors();    
       }
     }else {
            //Checksum didn't match, don't process the command
       dbg_print("Bad Cmd - invalid cmd or checksum didn't match\n");
       Serial.println("Error");
     }
  }
}
