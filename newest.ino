

/*************************************************** 
  Biometric Box Sketch for the optical Fingerprint sensor
  This sketch implements a two-level security scheme requiring the
  user to enter a passcode via the keypad before scanning a fingerprint
  for access.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Bill Earl for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
//#include <EEPROM.h> //EEPROM for later

// Define the states for the lock state machine
#define READY 2
#define OPEN 1
#define LOCKED 0

// State Variables:   Initialize to the locked state
int LockState = READY;
int digits[5] = {'0', '0', '0', '0'};
int pos = 0;
int digits4 = 0;
long tick = 0;
long lockTime = 0;
long dispTime = 0;
long totalTime = 0;
long remainingTime = 0;




// Keypad key matrix:
const byte rows = 4; 
const byte cols = 3; 
char keys[rows][cols] = 
{
   {'1','2','3'},
   {'4','5','6'},
   {'7','8','9'},
   {'*','0','#'}
};

// Keypad pin definitions
byte rowPins[rows] = {8, 7, 6, 5}; 
byte colPins[cols] = {4, 3, 2};  

// Instantiate the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// More pin definitions:
int LedPin = 10;
int SolenoidPin = 11;
int RedPwr = 52;
int RedWrite = 50;
int RedRead = 48;

Adafruit_7segment matrix = Adafruit_7segment();


void setup()                    
{
  #ifndef __AVR_ATtiny85__
    Serial.begin(9600);
    Serial.println("7 Segment Backpack Test");
  #endif
    matrix.begin(0x70);

   
   pinMode(LedPin, OUTPUT);
   pinMode(SolenoidPin, OUTPUT);
   
   pinMode(RedPwr, OUTPUT);
   //digitalWrite(RedPwr, HIGH);
   pinMode(RedWrite, OUTPUT);
   digitalWrite(RedWrite, LOW);   
   pinMode(RedRead, INPUT_PULLUP);
   
   // Flash hello
   for (int i = 0; i < 10; i++)
   {
     digitalWrite(LedPin, HIGH);
     delay(100);
     digitalWrite(LedPin, LOW);
     delay(100);
   }
   
   digitalWrite(SolenoidPin, HIGH);
   delay(1000);
   
   // Initialize state and communicatins
   setLockState(READY); 




}


void loop()                    
{  
   
 

  
   // Run the state machine:
   
   
   if (LockState == READY)
   {    


      
      char key = keypad.getKey();
      convert4ArrayTo4int(digits);
      displayDigits(dispTime);

      if (key == '#')
      {
        pos = 0;
        
        for (int i=0; i<=3; i++)
        {
          digits[i]= '0';
        }   
      }


      
      if ( (key != 0) && (key != '*') && (key != '#') && (pos < 4) )
      {
        digits[pos] = (key);
        pos++;      
      }



      if ((key == '*') && (pos ==  4))
      {
        //convert4ArraytoMilliseconds(digits);
        //lockTime = (totalTime + millis());
        setLockState(LOCKED);
      }

        
        

        
      if (digitalRead(RedRead) == LOW)
      {
         setLockState(OPEN);
         pos = 0;
        
        for (int i=0; i<=3; i++)
        {
          digits[i]= '0';
        }         
      }

       
     
   }









   // LOCKED state - hold the solenoid open for a limited time
   else if (LockState == LOCKED)
   {

    remainingTime = (lockTime - millis());
    convertMillisecondsTo4int(remainingTime);
    displayDigits(dispTime);

    
    if ( lockTime <= millis())
    {
        pos = 0;
        
        for (int i=0; i<=3; i++)
        {
          digits[i]= '0';
        }

        setLockState(READY);
    }


    
          
   }



  


    else if (LockState == OPEN)
    {

      delay(10000);
      setLockState(READY);
      
    }


    
    
   }






void convert4ArraytoMilliseconds (int digitarray[])
{
  long min10s = (digitarray[0] - 48);
  long min1s = (digitarray[1] - 48);
  long minutes = ((10*min10s) + (min1s));

  long sec10s = (digitarray[2] - 48);
  long sec1s = (digitarray[3] - 48);
  long seconds = ((10*sec10s) + (sec1s));

  totalTime = ((minutes*60*1000) + (seconds*1000));
}




void convertMillisecondsTo4int (long someTime)
{
  long totalSeconds = (someTime/1000);
  long dispMinutes = (totalSeconds/60);
  long dispSeconds = (totalSeconds - (dispMinutes*60));

  dispTime = ((dispMinutes*(100)) + (dispSeconds));  
}




void convert4ArrayTo4int (int digitarray[])
{ 
  
  dispTime = ( (1000*(digitarray[0] - 48)) + (100*(digitarray[1] - 48)) + (10*(digitarray[2] - 48)) + (1*(digitarray[3] - 48)) ); 
}




void displayDigits (int digits4)
{

  if ( (millis()) > tick)
  {
    
    matrix.print(digits4);
    matrix.drawColon(true);
    matrix.writeDisplay();
    matrix.setBrightness(0);
    tick = (millis()+100);
     
  }
  
}



/* EEPROM for later

//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }

long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }
*/


// Set the state and the time of the state change
void setLockState(int state)
{
   LockState = state;
   
   if (state == READY)
   {
             for (int i = 0; i < 10; i++)
      {
         digitalWrite(LedPin, HIGH);
         digitalWrite(RedPwr, HIGH);
         delay(100);
         digitalWrite(LedPin, LOW);
         digitalWrite(RedPwr, LOW);
         delay(100);
      }
      digitalWrite(LedPin, HIGH);
      digitalWrite(SolenoidPin, LOW);
    digitalWrite(RedPwr, HIGH);

   }




    if (state == OPEN)
     {

       

       
       digitalWrite(SolenoidPin, HIGH);        
       

       
  
     }



   
   else if (state == LOCKED)
   {
      for (int i = 0; i < 10; i++)
      {
         digitalWrite(LedPin, HIGH);
         digitalWrite(RedPwr, HIGH);
         delay(500);
         digitalWrite(LedPin, LOW);
         digitalWrite(RedPwr, LOW);
         delay(500);
      }

    
    digitalWrite(LedPin, HIGH);
    digitalWrite(SolenoidPin, LOW);
    digitalWrite(RedPwr, LOW);

    convert4ArraytoMilliseconds(digits);
    lockTime = (totalTime + millis());
   }
}

