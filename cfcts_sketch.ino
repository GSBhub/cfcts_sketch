#include <SoftwareSerial.h> // MG2639 Lib uses SS Library
#include "Adafruit_FONA.h"  // Cell shield library include
//#include "FirebaseArduino.h" // firebase library include - unstable, does not work

// adafruit FONA standard output PINs
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4 

#define FIREBASE_HOST "https://myfirstmapboxapp-11599.firebaseio.com/"
#define FIREBASE_AUTH "token_or_secret"
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX); // init the address 
SoftwareSerial *fonaSerial = &fonaSS; // I'll have to look into this, this is from the test method

// Interrupt pin Initilization 
const int HALT_PIN = 13; // 1 raises a HALT status in flight controller

//device init
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

char url[80] = "https://myfirstmapboxapp-11599.firebaseio.com/"; // url for our firebase server

//position data variables
float latitude, longitude, speed_kph, heading, speed_mph, altitude;

void setup() {

  // halt until serial up (active wait) -> could be passive, potentially
  while (! Serial);

  //Init network serial communications
  //Serial.begin(9600); // 9600 baud - try if 115200 does not work
  Serial.begin(115200); // this is the standard baud for our cell shield
  
  // set up interrupt pin
  pinMode(HALT_PIN, OUTPUT); 

  ///////////////////////////// BEGIN ADAFRUIT FONA NET SETUP //////////////////////////////

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  Serial.println(F("FONA is OK"));
  // Try to enable GPRS

  Serial.println(F("Enabling GPS..."));
  fona.enableGPS(true);

  // Optionally configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);

  // Copy this block down to loop if experiencing networking issues /////
  /////////////////// END DEBUG NETWORK METHODS /////////////////////////

  //////////////////////////// END ADAFRUIT FONA NET SETUP ////////////////////////////////

  //////////////////////////// BEGIN FIREBASE CONNECTION SETUP //////////////////////////



  ///////////////////////////// END FIREBASE CONNECTION SETUP ///////////////////////////
 
}
 
void loop() {

/////////////PART 0: Server down, connection off////////
//Display, allow some sort of error message? - TESTING



/////////////PART 1: Map Analysis///////////////////////
//Determine WHERE you are, cast a ray to determine WHERE you will be

if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)){
  
    Serial.print("GPS lat:");
    Serial.println(latitude, 6);
    Serial.print("GPS long:");
    Serial.println(longitude, 6);
    Serial.print("GPS speed KPH:");
    Serial.println(speed_kph);
    Serial.print("GPS speed MPH:");
    speed_mph = speed_kph * 0.621371192;
    Serial.println(speed_mph);
    Serial.print("GPS heading:");
    Serial.println(heading);
    Serial.print("GPS altitude:");
    Serial.println(altitude);
  } else {
    Serial.println("Waiting for FONA GPS 3D fix...");
    delay(30);
  }


/////////////PART 2: Decision Making: NFZ///////////////
//Implement No-fly zone logic, prevent flight in the forward direction

/////////////PART 3: Update and sync data///////////////
//Upload current map data to the server, change NFZ as necessary
}
