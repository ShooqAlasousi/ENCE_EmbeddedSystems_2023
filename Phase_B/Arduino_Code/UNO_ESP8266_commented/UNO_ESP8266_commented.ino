// LED connected to pin 13
#define LED 13

// Global flag which will be set by the ISR
volatile unsigned char gISRFlag2 = 0;

// ReloadTimer set to 0.4ms
const unsigned int gReloadTimer1 = 100;   // corresponds to 0.4ms

// Define globals for data buffers and handling
#define BUFF_SIZE 20
char  gIncomingChar;
char  gCommsMsgBuff[BUFF_SIZE];
int   iBuff = 0;
byte  gPackageFlag = 0;
byte  gProcessDataFlag = 0;

// Define global const for command parsing
#define COMMAND_SIZE 3
const char gStartCmd = "STR";
const char gStopCmd = "STP";
const char gGetCmd = "GET";

 /** @brief Compare two arrays with another and see if there is a match.
 *   @param a array a to be compared with B
 *   @param b array b to be comnpared with A
 *   @param size size of the arrays
 *   @return char 1 if match and char 0 if no match
 */
char compareArray(char a[], char b[], int size)
{
  int i;
  char result = 1;  // default: the arrays are equal
  
  // Iterate over the arrays according to <size>.
  for(i = 0; i<size; i++)
  {
    if(a[i]!=b[i])
    {
      result = 0;
      break;
    }
  }
  return result;
}

 /** @brief Setup the uC program by initializing the GPIO, Serial connection and timer1 ISR
 *   @return Void.
 */
void setup() {
  // Initialize the LED pin as OUTPUT
  pinMode(LED, OUTPUT);
  
  // Begin Serial connection at baudrate of 9600
  Serial.begin(9600);

  // Initialize Timer1 (16bit) -> Used for Serial Comms
  // Speed of Timer1 = 16MHz/64 = 250 KHz
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = gReloadTimer1;            // max value 2^16 - 1 = 65535
  TCCR1A |= (1<<WGM11);
  TCCR1B = (1<<CS11) | (1<<CS10);   // 64 prescaler
  TIMSK1 |= (1<<OCIE1A);
  interrupts();
}

 /** @brief Main loop of the uC
 *   @return Void.
 */
void loop() {
  char  auxMsgBuff[BUFF_SIZE];
  int auxCount = 0;
  unsigned char auxDigit = '0';
  
  // Attend Timer1 flag - receive commands through serial
  if(gISRFlag2 == 1)
  {    
    // Reset ISR Flag
    gISRFlag2 = 0;

    // Read serial
    gIncomingChar = Serial.read();

    // If normal character from package
    if(gPackageFlag == 1)
    {
      gCommsMsgBuff[iBuff] = gIncomingChar;
      iBuff++;

      // Safety mechanism in case "\n" is never sent
      if(iBuff == BUFF_SIZE)
      {
        gPackageFlag = 0;
        gProcessDataFlag = 1;
      }
    }

    // If start of the package
    if(gIncomingChar == '$')
    {    
      gPackageFlag = 1;  // Signal start of package
      
      // Clear Buffer
      for(int i=0; i<BUFF_SIZE; i++)
      {
        gCommsMsgBuff[i] = 0;
      }

      // set gCommsMsgBuff Index to zero
      iBuff = 0;
    }

    // If end of package
    if( (gIncomingChar == '\n') && (gPackageFlag == 1) )
    {
      // Signal end of package
      gPackageFlag = 0;
      gProcessDataFlag = 1;
    }
  }

  // Process serial commands
  if(gProcessDataFlag == 1)
  {
    // Reset flag for next iteration
    gProcessDataFlag = 0;

    // Compare received input to hardcoded command and drive LED accordingly
    if(compareArray(gCommsMsgBuff, gStartCmd, COMMAND_SIZE) == 1)
    {
      // Start timer function
      digitalWrite(LED, HIGH);
    }
  
    if(compareArray(gCommsMsgBuff, gStopCmd, COMMAND_SIZE) == 1)
    {
      // Stop timer function
      digitalWrite(LED, LOW);
    }

    if(compareArray(gCommsMsgBuff, gGetCmd, COMMAND_SIZE) == 1)
    {
      // Send clock status
      Serial.print("$00:01\n");
    }
    // ------
  }
}

 /** @brief ISR for Timer1 which gets triggered every 0.4ms
 *   @param ISR vector for timer1 ( TIMER1_COMPA_vect )
 *   @return Void.
 */
ISR(TIMER1_COMPA_vect)  // Timer1 interrupt service routine (ISR)
{
  if(Serial.available()>0)
  {
    gISRFlag2 = 1;
  }
}
