#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

Ticker ticker;

//Change the following IP address to your smartthings hub IP address
#define CALLBACK "http://192.168.1.204:39500/notify"

#define RING_BUFFER_SIZE  128

#define SYNC_HIGH       600
#define SYNC_LOW        600
#define SYNC_TOLL       100           // +- Tolerance for sync pulse

#define PULSE_LONG      400
#define PULSE_SHORT     220
#define PULSE_TOLL      100           // +- Tolerance for bit timming

#define BIT1_HIGH       PULSE_LONG
#define BIT1_LOW        PULSE_SHORT
#define BIT0_HIGH       PULSE_SHORT
#define BIT0_LOW        PULSE_LONG

#define PULSE_SHORT_NOISE     PULSE_SHORT - PULSE_TOLL      // anything shorter that this is noise
#define PULSE_LONG_NOISE      SYNC_HIGH + SYNC_TOLL         // anything longer that this is noise

#define SYNCPULSECNT      4                   // 4 sync pulses (8 edges)
#define SYNCPULSEEDGES    (SYNCPULSECNT * 2 )

#define DATABYTESCNT      7                   // Number of bytes to look for 
#define DATABITSCNT       (DATABYTESCNT * 8)  // Number of bits to look for
#define DATABITSEDGES     (DATABITSCNT * 2)   // Number of edges to look for

// The pulse durations are measured in micro seconds between pulse edges.
unsigned long pulseDurations[RING_BUFFER_SIZE];   // where we store the pulse edges
unsigned int syncIndex  = 0;                  // index of the last bit time of the sync signal
unsigned int dataIndex  = 0;                  // index of the first bit time of the data bits (syncIndex+1)
bool         syncFound = false;               // true if sync pulses found
bool         received  = false;               // true if enough sync pulses bits are found
unsigned int changeCount = 0;                 // Count of pulses edges

unsigned char dataBytes[DATABYTESCNT];        // Decoded data storage
unsigned long mytime = 0;                     // event time
float temp = 0;                               // temperature
int   hum  = 0;

#define DATAPIN            D2               // 2 is interrupt
byte interruptPin = DATAPIN;
#define MyInterrupt (digitalPinToInterrupt(interruptPin))
#define MyLED             BUILTIN_LED

/* ************************************************************* */
/*
*    Will print 8-bit formatted hex
*/
void PrintHex8(uint8_t *data, uint8_t length) 
{
   char tmp[length*2+1];
   byte first;
   int j = 0;
   for (uint8_t i = 0; i < length; i++) 
     {
       first = (data[i] >> 4) | 48;
       if (first > 57) tmp[j] = first + (byte)39;
       else tmp[j] = first ;
       j++;
    
       first = (data[i] & 0x0F) | 48;
       if (first > 57) tmp[j] = first + (byte)39; 
       else tmp[j] = first;
       j++;
     }
   tmp[length*2] = 0;
   Serial.print("0x");
   Serial.print(tmp);
}


//* ************************************************************* */
// Prints a byte as binary with leading zero's
void printBits(byte myByte)
{
   for(byte mask = 0x80; mask; mask >>= 1)
   {
     if(mask  & myByte)
         Serial.print('1');
     else
         Serial.print('0');
   }
}


//* ************************************************************* */
//    Checksum of bits
uint8_t CheckSum(uint8_t const message[], unsigned nBytes) 
{
    unsigned int sum = 0;
    unsigned i;  
    for (i = 0; i <= nBytes; ++i) 
    {
      sum = sum + message[i];
    }
    sum = (sum & 0x000000ff);  
    return ((uint8_t) sum );
}


/* ************************************************************* */
/*
 * Look for the sync pulse train of 4 high-low pulses of
 * 600 uS high and 600 uS low.
 * idx is index of last captured bit duration.
 * Search backwards 8 times looking for 4 pulses
 * approximately 600uS long.
 *
 */
bool isSync(unsigned int idx) 
{
   // check if we've received 4 sync pulses of correct timing
   for( int i = 0; i < SYNCPULSEEDGES; i += 2 )
   {
      unsigned long t1 = pulseDurations[(idx+RING_BUFFER_SIZE-i) % RING_BUFFER_SIZE];
      unsigned long t0 = pulseDurations[(idx+RING_BUFFER_SIZE-i-1) % RING_BUFFER_SIZE];      
      
      // If any of the preceeding 8 pulses are out of bounds, short or long,
      // return false, no sync found
      if( t0<(SYNC_HIGH-SYNC_TOLL) || t0>(SYNC_HIGH+SYNC_TOLL) || t1<(SYNC_LOW-SYNC_TOLL)  || t1>(SYNC_LOW+SYNC_TOLL) )
              { return false; }
    }
   return true;
}


/* ************************************************************* */
/* Interrupt  handler 
 * Set to interrupt on edge (level change) high or low transition.
 * Change the state of the Arduino LED on each interrupt. 
 */
void interrupt_handler() 
{
   volatile static unsigned long duration = 0;
   volatile static unsigned long lastTime = 0;
   volatile static unsigned int ringIndex = 0;
   volatile static unsigned int syncCount = 0;
   volatile static unsigned int bitState  = 0;

   // Ignore this interrupt if we haven't finished processing the previous 
   // received block in the main loop.
   if( received == true ) {return;}                     // return, we are not finish with processor last block

   bitState = digitalRead (DATAPIN);
   digitalWrite(MyLED, 1-bitState);                       // LED to show receiver activity

   // calculating timing since last change
   long time = micros();
   duration = time - lastTime;
   lastTime = time;

   // Known errors in bit stream are runt's --> short and long pulses.
   // If we ever get a really short, or really long 
   // pulse's we know there is an error in the bit stream
   // and should start over.
   if ( (duration > (PULSE_LONG_NOISE)) || (duration < (PULSE_SHORT_NOISE)) )    // This pulse must be noise...   
   {
      received = false;
      syncFound = false;
      changeCount = 0;                                  // restart, start looking for sync and data bits again
   }

   // if we have good data, store data in ring buffer
   ringIndex = (ringIndex + 1) % RING_BUFFER_SIZE;
   pulseDurations[ringIndex] = duration;
   changeCount++;                                       // found another edge

   // detected sync signal
   if( isSync (ringIndex) )                              // check for sync on each bit received
   {
      syncFound = true;
      changeCount = 0;                                   // lets restart looking for data bits again
      syncIndex = ringIndex;
      dataIndex = (syncIndex + 1) % RING_BUFFER_SIZE;
    }
    
   // If a sync has been found, then start looking for the
   //  data bit edges in DATABITSEDGES
   if( syncFound )
   {       
      // not enough bits yet?, so no full message block has been received yet
      if( changeCount < DATABITSEDGES )            
        { received = false; }
      
      else 
      
      if( changeCount >= DATABITSEDGES )            // check for too many bits
        {      
          changeCount = DATABITSEDGES;              // lets keep bits we have, checksum will kill this block if bad
          detachInterrupt(MyInterrupt);             // disable interrupt to avoid new data corrupting the buffer
          received = true;   
        }
    }    // end of if syncFound
}    // end of interrupt_handler

/* ************************************************************* */
/* ************************************************************* */
/* ************************************************************* */
/* ************************************************************* */
/*
 * Convert pulse durations to bits.
 * 
 * 1 bit ~0.4 msec high followed by ~0.2 msec low
 * 0 bit ~0.2 msec high followed by ~0.4 msec low
 * 
 */
int convertTimingToBit(unsigned int t0, unsigned int t1) 
{
   if( t0 > (BIT1_HIGH-PULSE_TOLL) && t0 < (BIT1_HIGH+PULSE_TOLL) && t1 > (BIT1_LOW-PULSE_TOLL) && t1 < (BIT1_LOW+PULSE_TOLL) )
      { return 1; }
   else if( t0 > (BIT0_HIGH-PULSE_TOLL) && t0 < (BIT0_HIGH+PULSE_TOLL) && t1 > (BIT0_LOW-PULSE_TOLL) && t1 < (BIT0_LOW+PULSE_TOLL) )
      { return 0; }
   return -1;                   // error, if undefined bit timimg
}

/* ************************************************************* */
int acurite_getHumidity (uint8_t byte) 
{
    // range: 1 to 99 %RH
    int humidity = byte & 0x7F;
    return humidity;
}


/* ************************************************************* */
float acurite_getTemp_6044M(byte hibyte, byte lobyte) 
{
  // range -40 to 158 F, -40º C to 70º C --> returns temp in deg C
  int highbits = (hibyte & 0x0F) << 7;
  int lowbits = lobyte & 0x7F;
  int rawtemp = highbits | lowbits;
  float temp = (rawtemp / 10.0) - 100;
  return temp;
}


/* ************************************************************* */
float convCF(float c) 
{
  return ((c * 1.8) + 32);
}


/* ************************************************************* */
uint16_t acurite_txr_getSensorId(uint8_t byte)
{
    return ((byte & 0xc0) >> 6);
}

/* ************************************************************* */
uint16_t acurite_txr_getSensorSN(uint8_t hibyte, uint8_t lobyte)
{
    return ((hibyte & 0x3f) << 8) | lobyte;
}


/* ************************************************************* */
bool acurite_txr_getBattery(uint8_t battery)
{
  if ( (battery & 0x80) == 0x80 )     // check if battery is low
      { return true; }                // Yes, its low
  return false;
}

struct Acurite {
  bool newData;
  char id;
  uint16_t serial;
  float temperature;
  int humidity;
  bool lowBatt;
} acurite;

// Also 00592TX, 06002M, 06044 and others....
void decode_Acurite_6044(byte dataBytes[])
{
  byte ID = acurite_txr_getSensorId(dataBytes[0]);
  if ( ID == 0x3) acurite.id = 'A';
  else if ( ID == 0x2) acurite.id = 'B';
  else if ( ID == 0x0) acurite.id = 'C';
    
  acurite.serial = acurite_txr_getSensorSN(dataBytes[0], dataBytes[1]);
  acurite.temperature = convCF(acurite_getTemp_6044M(dataBytes[4], dataBytes[5]));
  acurite.humidity = acurite_getHumidity(dataBytes[3]);
  acurite.lowBatt = acurite_txr_getBattery(dataBytes[2]);
  acurite.newData = true;
}

unsigned long currentMillis = 0;                  // a 1 Minute clock timer
unsigned long interval = 60000;                   // = 60 sec --> 1 Minure
unsigned long previousMillis = 0;
unsigned long Minute = 0;
unsigned long BlockFailCounter  = 0;
unsigned long CSFailCounter    = 0;

void decode()
{
  //lets setup a long duration timer at 1 minute tick
  currentMillis = millis ();                                  // get current time
  if (currentMillis - previousMillis >= interval)
        { previousMillis = currentMillis; Minute++; }         // add one to minute couter if time..

   if( received == true )                                     // check to see if we have a full block of bits to decode
   {
      // disable interrupt to avoid new data corrupting the buffer
      detachInterrupt(MyInterrupt);
      
      unsigned int startIndex, stopIndex, ringIndex;
      bool fail = false;

      // Build a byte array with the raw data received
      fail = false; // reset bit decode error flag

      // clear the data bytes array
      for( int i = 0; i < DATABYTESCNT; i++ ) { dataBytes[i] = 0; }
      ringIndex = (syncIndex +1 ) % RING_BUFFER_SIZE;
      for( int i = 0; i < DATABITSCNT; i++ ) {
        int bit = convertTimingToBit ( pulseDurations[ringIndex % RING_BUFFER_SIZE], 
                                        pulseDurations[(ringIndex +1 ) % RING_BUFFER_SIZE] );                                                                           
        if( bit < 0 ) { // check for a valid bit, ie: 1 or zero, -1 = error
          fail = true;
          break; //exit loop
        }                        
        else {
          dataBytes[i/8] |= bit << ( 7 - (i % 8)); // pack into a byte
        }   
        ringIndex += 2;
      }

      if (!fail) {                                                 // if fail, we decoded some of the bits are wrong, so don't process this block
        if ( CheckSum (dataBytes, 5) == dataBytes[6] ) {           // if Check Sum is good... 
          decode_Acurite_6044(dataBytes);                          // decode all of the bits in the block
        }   // End of: if ( CheckSum (dataBytes, 5) == dataBytes[6] )
        else {
            CSFailCounter++;                                       // if Check Sum is bad, keep count
        }
      }   // End of if (!fail)...
      else  { BlockFailCounter++; }                                // if block decode is bad, keep count
          
      received = false;
      syncFound = false;
      
      // re-enable interrupt
      attachInterrupt (MyInterrupt, interrupt_handler, CHANGE);
      
   }    // end of:  if( received == true )
}   // end of: loop

void acuriteSetup()
{
  pinMode(DATAPIN, INPUT);              // data interrupt pin set for input
   
  pinMode(MyInterrupt, INPUT_PULLUP);
  pinMode(MyLED, OUTPUT);               // LED output
  digitalWrite(MyLED, HIGH);

  attachInterrupt(MyInterrupt, interrupt_handler, CHANGE);
  acurite.newData = false;
  ticker.attach(0.25, decode);
}

uint16_t eventSeq = 0;

void sendEvent() {
  if (callback.length() == 0) { return; }
  eventSeq++;

  String msg = "<root><temperature>" + String(acurite.temperature) + "</temperature>";
  msg += "<id>" + String(acurite.id) + "</id>";
  msg += "<serial>" + String(acurite.serial) + "</serial>";
  msg += "<humidity>" + String(acurite.humidity) + "</humidity>";
  msg += "<lowbatt>" + String(acurite.lowBatt) + "</lowbatt>";
  msg += "</root>";
  uint8_t buf[msg.length()];
  for (int i=0; i<sizeof(buf); i++) {
    buf[i] = msg[i];
  }
  HTTPClient http;
  http.begin(CALLBACK);
  http.addHeader("Content-Type", "text/xml");
  http.addHeader("NT", "upnp:event");
  http.addHeader("NTS", "upnp:propchange");
  http.addHeader("SID", "uuid:1234");
  http.addHeader("SEQ", String(eventSeq));
  http.sendRequest("NOTIFY", buf, sizeof(buf));
  http.end();
  //Serial.println(msg);
}

void wifiSetup() {
  //WiFi settings
  char ssid[] = "Your WiFi SSID";
  char pass[] = "Your WiFi password";

  //Change the following to match your home network
  IPAddress ip(192, 168, 1, 22);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress dns(192, 168, 1, 1);
  
  WiFi.config(ip, dns, gateway, subnet);
    
  //Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ... ");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  wifiSetup();
  acuriteSetup();
}

void loop() {
  if (acurite.newData) {
    acurite.newData = false;
    sendEvent();
  }
}
