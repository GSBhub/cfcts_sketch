#include <SoftwareSerial.h> // MG2639 Lib uses SS Library
//#include <ESP8266WiFi.h>
//#include <FirebaseArduino.h>
#include "Adafruit_FONA.h"  // Cell shield library include

// Interrupt pin Initilization 
#define HALT_PIN 13 // 1 raises a HALT status in flight controller

// adafruit FONA standard output PINs
#define FONA_RX 3
#define FONA_TX 2
#define FONA_RST 4
 
#define FIREBASE_HOST "myfirstmapboxapp-11599.firebaseio.com"
#define FIREBASE_AUTH "jdi5ilRiQjD1QkT2zENBBOpex53NhqKBPyCNkMKO"

SoftwareSerial fonaSerial = SoftwareSerial(FONA_TX, FONA_RX); // init the address 
//SoftwareSerial *fonaSerial = &fonaSS; // I'll have to look into this, this is from the test method

//HardwareSerial *fonaSerial = &Serial1;

//device init
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

//position data variables
float latitude, longitude, speed_kph, heading, speed_mph, altitude;

//device ID information
String metadata_path;

void setup() {

  Serial.println("Attempting to start serial communications...");

  Serial.begin(115200); // this is the standard baud for our cell shield

  // halt until serial up (active wait) -> could be passive, potentially
   while (! Serial); // really, this seems to break it for some stupid reason

  Serial.println("Serial communicaitons started.");

  //Init network serial communications
  // set up interrupt pin
//  pinMode(HALT_PIN, OUTPUT); 

  ///////////////////////////// BEGIN ADAFRUIT FONA NET SETUP //////////////////////////////

  Serial.println("Attempting to start the FONA...");

  fonaSerial.begin(9600); 
  
  if (! fona.begin(fonaSerial)) {
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

  //////////////////////////// BEGIN DEVICE ID SETUP //////////////////////////

  char sim_id[21];

  // read the CCID (used w/ server to pull unique device ID information, potentially)
  fona.getSIMCCID(sim_id);  // make sure sim_id is at least 21 bytes!

  //String sim_id[21] = sim_temp_id ;
  
  Serial.print(F("SIM CCID = ")); Serial.println(sim_id);

  ///////////////////////////// END DEVICE ID SETUP ///////////////////////////

/**
  //////////////////////////// BEGIN FIREBASE CONNECTION SETUP //////////////////////////

  // connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  String path;
  path.concat("UserInfo/");
  path.concat(sim_id) ;

  // check if SIMCCID is associtaed with an existing firebase identifier
  if (Firebase.getString(path)){
      String drone_name = Firebase.getString(path);
      Serial.print("UAS name found! \nName:");
      Serial.print(drone_name + "\n");
      metadata_path.concat( "GeoFire/" );
      metadata_path.concat( drone_name );
    }
   else {
     Serial.print("No UAS associated with SIM ");
     Serial.print(sim_id);
     while(1); 
   }

  Serial.println("Connection to Firebase successful!"); 
*/
  ///////////////////////////// END FIREBASE CONNECTION SETUP ///////////////////////////

  
}
 
void loop() {

/**
/////////////PART 0: Server down, connection off////////
//Display, allow some sort of error message? - TESTING

if (Firebase.failed()){
    Serial.print("Connection error with Firebase:");
    Serial.println(Firebase.error());  
}
*/

/////////////PART 1: Location update///////////////////////
//Determine WHERE you are

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
/**
/////////////PART 2: Update server location///////////////
//Use firebase to send locational information to server

    Firebase.setFloat(metadata_path + "/1/0", latitude);
    Firebase.setFloat(metadata_path + "/1/1", longitude);
    if (Firebase.success()){
        Serial.println("Successful update of location information to Firebase!");
      } else {
      Serial.print("Error sending location to firebase:");
      Serial.println(Firebase.error());    
    }
*/

  String NFZ = ""; // Hardcoded NFZ from firebase, cannot access firebase

/////////////PART 3: Decision Making: NFZ///////////////
//Implement No-fly zone logic, prevent flight in the forward direction

     bool inNFZ = isThisPointInANoFlyZone(latitude, longitude);
     if (inNFZ) {
      
        Serial.println("Detected entry into no-fly zone!"); 
        analogWrite(HALT_PIN, 128); // send Return to sender signal to server (128 is a test PWM for now)
        while(1);
     }

     delay(6000);// wait for a few seconds
    
  } else {
    Serial.println("Waiting for FONA GPS 3D fix...");
    delay(300);
  }


}

  /*
   * this will determine if at drone has entered a nofly zone
   * it will get the no flyzones from firebase and the compare the 
   * current point to each of the nofly zone polygons and determine 
   * if the point is inside
   * 
   * Kudos to David Fay to writing these two methods
   */
bool isThisPointInANoFlyZone(double lat, double lng){

  FirebaseObject noFlyZones = Firebase.get("NoFlyZones");
  if (Firebase.success()){
      Serial.println("Successfully got no fly zones!");
      String data = noFlyZones.getString("zone1");
      Serial.println(data);
     }
     if (Firebase.failed()){
      Serial.print("get noflyzonse failed:");
      Serial.println(Firebase.error());
     }
   
  return false;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

