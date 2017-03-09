#include <SoftwareSerial.h> // MG2639 Lib uses SS Library
#include "Adafruit_FONA.h"  // Cell shield library include

// adafruit FONA standard output PINs
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4 

SoftwareSerial fonaSS = SoftwareSerial(Fona_TX, Fona_RX); // init the address 
SoftwareSerial *fonaPtr = &fonaSS; // I'll have to look into this, this is from the test method

// Interrupt pin Initilization 
const int HALT_PIN = 13; // 1 raises a HALT status in flight controller

// Local stored IP address
IPAddress myIP; 

// Firebase server information
IPAddress fireIP; // IP address of the file server
const char server[] = "some_firebase_site.com" 

//device init
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

//opt: serial device init
//  HardwareSerial *fonaSerial = &Serial1;

void setup() {

  // halt until serial up (active wait) -> could be passive, potentially
  while (! Serial);

  //Init network serial communications
  //Serial.begin(9600); // 9600 baud - try if 115200 does not work

  Serial.begin(115200); // this is the standard baud for our cell shield
  
  // set up interrupt pin
  pinMode(HALT_PIN, OUTPUT); 

  ///////////////////////////// BEGIN ARDUINO NETWORK SETUP //////////////////////////////

  // halt exec of program until some value over serial link
  // avoid using data unless we need to waste it!
  // serialTrigger(); 

  // turn module on, verify communication (4800 baud)
  fonaPtr->begin(4800);
  while (!fona.begin(*fonaSerial)){
    Serial.print(F("\nUnable to communicate with shield. Retrying"))
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F(".\n"));
    //retry
  }

  // enable GPRS, call before doing any TCP/2G transmissons
  
  while (! fona.enableGPRS(true)){
    Serial.print(F("\nUnable to open GPRS. Retrying"))
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F(".\n"));
    //retry
  } 

  // start FONA GPS device
  fona.enableGPS(true);
  Serial.print(F("Fona GPS started..."));

  //init local IP
  myIP = fona.localIP();
  Serial.print(F("Local IP Address: "));
  Serial.println(myIP);

  // perform DNS lookup of domain, IP
  int DNS_Status = gprs.hostByName(server, &serverIP);
  while (DNS_Status <= 0){
    Serial.println(F("Couldn't find the server IP. Retrying"));
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F(".\n"));
    //retry 
    DNS_Status = gprs.hostByName(server, &serverIP);
  }
  Serial.print(F("Server IP is: "));
  Serial.println(serverIP);

  // establish TCP connection to host with previous connection info
  int connect_Status = gprs.connect(serverIP, 80); // 80 is standard HTTP port

  while (connect_Status <= 0){
    Serial.println(F("Unable to connect. Retrying"))
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F("."));
    delay(300);
    Serial.print(F(".\n"));
    //retry   
    connect_Status = gprs.connect(serverIP, 80);
 }

  Serial.println(F("Successful connection! Sending HTTP GET Request"));
  Serial.println();

  // HTTP TEST 
  gprs.print("GET / HTTP/1.1\nHost: example.com\n\n");

  // Copy this block down to loop if experiencing networking issues /////
  /////////////////// END DEBUG NETWORK METHODS /////////////////////////


  //////////////////////////// END ARDUINO NETWORK SETUP ////////////////////////////////

  //////////////////////////// BEGIN FIREBASE CONNECTION SETUP //////////////////////////



  ///////////////////////////// END FIREBASE CONNECTION SETUP ///////////////////////////
 
}
 
void loop() {

/////////////PART 0: Server down, connection off////////
//Display, allow some sort of error message? - TESTING
if (!gprs.available()){
    Serial.println("Error! No bytes let from server!");
  }


/////////////PART 1: Map Analysis///////////////////////
//Determine WHERE you are, cast a ray to determine WHERE you will be

/////////////PART 2: Decision Making: NFZ///////////////
//Implement No-fly zone logic, prevent flight in the forward direction

/////////////PART 3: Update and sync data///////////////
//Upload current map data to the server, change NFZ as necessary
}

void serialTrigger() {
  Serial.println("Sending serial to start:");
  while (!Serial.available());
  Serial.read();  
}
