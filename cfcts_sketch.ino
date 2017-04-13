//#include <ESP8266WiFi.h>
//#include <FirebaseArduino.h>
#include "Adafruit_FONA.h"  // Cell shield library include
#include <Servo.h> // servo library for our flight controller
#include <SoftwareSerial.h> // MG2639 Lib uses SS Library


// Interrupt pin Initilization 
#define HALT_PIN 13 // PWM raises a HALT status in flight controller

// adafruit FONA standard output PINs
#define FONA_RX 3
#define FONA_TX 2
#define FONA_RST 4
 
#define FIREBASE_HOST "myfirstmapboxapp-11599.firebaseio.com"
#define FIREBASE_AUTH "jdi5ilRiQjD1QkT2zENBBOpex53NhqKBPyCNkMKO"

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX); // init the address 
SoftwareSerial *fonaSerial = &fonaSS; // I'll have to look into this, this is from the test method

//HardwareSerial *fonaSerial = &Serial1;

//device init
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

//define servo attach for flight controller
Servo myservo;

//position data variables
float latitude, longitude, speed_kph, heading, speed_mph, altitude;

//device ID information
String metadata_path;

void setup() {

  while (! Serial); // really, this seems to break it for some stupid reason
  Serial.begin(115200); // this is the standard baud for our cell shield

  // halt until serial up (active wait) -> could be passive, potentially

  Serial.println("Serial communicaitons started.");

  //Init network serial communications
  // set up interrupt pin
  
  // this is the pin that the pwm signal will be sent on :
  myservo.attach(HALT_PIN);

  ///////////////////////////// BEGIN ADAFRUIT FONA NET SETUP //////////////////////////////

  Serial.println("Attempting to start the FONA...");

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


/////////////PART 3: Decision Making: NFZ///////////////
//Implement No-fly zone logic, prevent flight in the forward direction

     bool inNFZ = isThisPointInANoFlyZone(latitude, longitude);
     if (inNFZ) {
        Serial.println("Detected entry into no-fly zone!"); 
        // the is the pwm signal
        // put this in the section after the drone is dectected in the
        // NFZ and that is that
        myservo.writeMicroseconds(1800);        
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
   * Kudos to David Fay to writing everything below this point ---------------------------------------------
   */
//
//float createSteps(float a, float b, int number_of_steps){
// /*
//  * calculates the proper incriment between to vectors
//  */
//  
//  float delta = b - a;
//  float incriment = delta/number_of_steps;
//  return incriment;
//  
//}
//
//void flyLeg(float start_lat, float start_lng, float stop_lat,  float stop_lng, int number_of_steps){
//  /*
//   * simulates a drone transmiting its locaion to our database by taking two sets of corordinates
//   * and filling in the path between them with intermediate points.  
//   */
//
//  // calculate how far to step between each point 
//  // aka divied the distance between each point by the number of steps
//  float lat_step = createSteps(start_lat, stop_lat, number_of_steps);
//  float lng_step = createSteps(start_lng, stop_lng, number_of_steps);
//  float each_lat = start_lat;
//  float each_lng = start_lng;
//
//  // get to step'n
//  int i;
//  for (i = 0; i<number_of_steps; i = i+1){
//     each_lat = each_lat + lat_step;
//     each_lng = each_lng + lng_step;
//     // send the current locatin to the database
//     // TODO change this to transmit both points simultaniously
//     Firebase.setFloat("GeoFire/Rouge One/l/0", each_lat);
//     Firebase.setFloat("GeoFire/Rouge One/l/1", each_lng);
//
//      // Serial.println("Successfully updated Location!");
//      bool inNFZ = isThisPointInANoFlyZone(each_lat, each_lng);
//      if(inNFZ){
//        Serial.println("inside nofly zone");
//      }else{
//        Serial.println("not inside nofly zone");
//      }
//     
//     delay(100); 
////  }
////}

bool pointInsidePolygon(String polygonlist, float lat, float lng){
  /*
   * Determine if the lat/lng location in inside the polygon represented by 
   * the polygon list saved in kml format (lng,lat,alt).
   */

  int num_o_spaces = countSpaces(polygonlist);
  char space = 32;
  bool inside = false;
  
  // iterate over all of the points in the polygonlist string from the database      
  for(int i=0; i <= num_o_spaces; i++){
    // get the lng,lat,alt entry at position i from the polygon string
    String current_point = getValue(polygonlist, space, i);
    // get the lng entry from the locaion string at poisition i 
    String p1_lat_str = getValue(current_point, ',', 1);
    float p1_lat = p1_lat_str.toFloat();
    // get the lat entry from the location string at poisition i 
    String p1_lng_str = getValue(current_point, ',', 0);
    float p1_lng = p1_lng_str.toFloat();
    
    // do it all again for the next point to create a line between them
    String next_point = getValue(polygonlist, space, (i+1)%num_o_spaces);
    // lng
    String p2_lat_str = getValue(next_point, ',', 1);
    float p2_lat = p2_lat_str.toFloat();
    // lat
    String p2_lng_str = getValue(next_point, ',', 0);
    float p2_lng = p2_lng_str.toFloat();
   // check if a ray cast from this point intersects the edge of this polygon 
    inside = lineIntersect(inside, lng, lat, p1_lng, p1_lat, p2_lng, p2_lat);
  }
  return inside;       
}

bool lineIntersect(bool inside, const float x, const float y, const float p1_x, const float p1_y, const float p2_x, const float p2_y){
  /*
   * Cast a line from the point at x,y and determine if it intersects with a line beteween 
   * p1_x,p1_y and p2_x,p2_y
   * 
   */
   
  float min_y = min(p1_y, p2_y);
  if (y > min_y){
    float max_y = max(p1_y, p2_y);
    if (y <= max_y){
      float max_x = max(p1_x, p2_x);
      if(x <= max_x){
        if(p1_y != p2_y){
          float xinters = (y-p1_y)*(p2_x-p1_x)/(p2_y-p1_y)+ p1_x;
          if((p1_x==p2_x)or(x<=xinters)){
            inside = !inside; 
          }
        }
      }
    }
  }
  return inside;
}

String getValue(String data, char separator, int index){
  /*
   * Split a string by the characeter passed in seperator 
   * and get the section between the two seporators at the 
   * position specified by index.
   */
   
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

int countSpaces(String string){
  /*
   * Count how many spaces are in a string.
   * This will be used to determine how many points are in 
   * the polygonlist strings.  
   */

  int number_of_spaces = 0;
  int i;
  char space = 32;
  
  for (i = 0; i < string.length(); i++){
    char this_char = string[i];
//    Serial.println(this_char);
    if (this_char == space){
      number_of_spaces++;
    }
  }
  return number_of_spaces;
}

bool isThisPointInANoFlyZone(double lat, double lng){
  /*
   * Determine if this drone has entered any of the nofly zones
   * from our database.
   * 1. Get the no flyzones from firebase. 
   * 2. Compare the current location to each of the nofly zone polygons 
   * and determine if the point is inside.  
   */

  // get all of the no-fly zones 
  //FirebaseObject noFlyZones = Firebase.get("NoFlyZones");
  // check for success response  

    // get the number of no fly-zones from the first entry in the datbase
    int num_o_zones = 1; // only hardcoding one zone for now
    char space = 32;
    // start iterating through all the no-flyzones
    for(int z=0; z<num_o_zones; z++){
      char zone_num = (z+49);
      String zone = "zone_";
      String zone_key = zone+zone_num;
      String polygonlist = "-83.74171167612076,39.69924181876981,0 -83.74193698167801,39.69910148613672,0 -83.74302595853806,39.700046662183496,0 -83.74256998300552,39.700306686511084,0 -83.74171167612076,39.69924181876981,0";
      // check if the current location is in this no-fly zone
      if(pointInsidePolygon(polygonlist, lat, lng)){
        return true;
      }
    }

  return false;
}

//bool isThisPointInANoFlyZone(double lat, double lng){
//  /*
//   * Determine if this drone has entered any of the nofly zones
//   * from our database.
//   * 1. Get the no flyzones from firebase. 
//   * 2. Compare the current location to each of the nofly zone polygons 
//   * and determine if the point is inside.  
//   */
//
//  // get all of the no-fly zones 
//  // FIREBASE BROKEN, DEFINE STRINGS
//  // FirebaseObject noFlyZones = Firebase.get("NoFlyZones");
//  // check for success response  
//
//
//
//    // get the number of no fly-zones from the first entry in the datbase
//    int num_o_zones = 1; // Only hardcoding one zone for now
//    char space = 32;
//    // start iterating through all the no-flyzones
//    for(int z=0; z<num_o_zones; z++){
//      char zone_num = (z+49);
//      String zone = "zone_";
//      String zone_key = zone+zone_num;
//      String polygonlist = noFlyZones.getString(zone_key);
//      // check if the current location is in this no-fly zone
//      if(pointInsidePolygon(polygonlist, lat, lng)){
//        return true;
//      }
//    }
//
//  return false;
//}
//
//bool pointInsidePolygon(String polygonlist, float lat, float lng){
//  /*
//   * Determine if the lat/lng location in inside the polygon represented by 
//   * the polygon list saved in kml format (lng,lat,alt).
//   */
//
//  int num_o_spaces = countSpaces(polygonlist);
//  char space = 32;
//  bool inside = false;
//  
//  // iterate over all of the points in the polygonlist string from the database      
//  for(int i=0; i <= num_o_spaces; i++){
//    // get the lng,lat,alt entry at position i from the polygon string
//    String current_point = getValue(polygonlist, space, i);
//    // get the lng entry from the locaion string at poisition i 
//    String p1_lat_str = getValue(current_point, ',', 1);
//    float p1_lat = p1_lat_str.toFloat();
//    // get the lat entry from the location string at poisition i 
//    String p1_lng_str = getValue(current_point, ',', 0);
//    float p1_lng = p1_lng_str.toFloat();
//    
//    // do it all again for the next point to create a line between them
//    String next_point = getValue(polygonlist, space, (i+1)%num_o_spaces);
//    // lng
//    String p2_lat_str = getValue(next_point, ',', 1);
//    float p2_lat = p2_lat_str.toFloat();
//    // lat
//    String p2_lng_str = getValue(next_point, ',', 0);
//    float p2_lng = p2_lng_str.toFloat();
//   // check if a ray cast from this point intersects the edge of this polygon 
//    inside = lineIntersect(inside, lng, lat, p1_lng, p1_lat, p2_lng, p2_lat);
//  }
//  return inside;       
//}
//
//bool lineIntersect(bool inside, const float x, const float y, const float p1_x, const float p1_y, const float p2_x, const float p2_y){
//  /*
//   * Cast a line from the point at x,y and determine if it intersects with a line beteween 
//   * p1_x,p1_y and p2_x,p2_y
//   * 
//   */
//   
//  float min_y = std::min(p1_y, p2_y);
//  if (y > min_y){
//    float max_y = std::max(p1_y, p2_y);
//    if (y <= max_y){
//      float max_x = std::max(p1_x, p2_x);
//      if(x <= max_x){
//        if(p1_y != p2_y){
//          float xinters = (y-p1_y)*(p2_x-p1_x)/(p2_y-p1_y)+ p1_x;
//          if((p1_x==p2_x)or(x<=xinters)){
//            inside = !inside; 
//          }
//        }
//      }
//    }
//  }
//  return inside;
//}
//
//String getValue(String data, char separator, int index){
//  /*
//   * Split a string by the characeter passed in seperator 
//   * and get the section between the two seporators at the 
//   * position specified by index.
//   */
//   
//  int found = 0;
//  int strIndex[] = {0, -1};
//  int maxIndex = data.length()-1;
//
//  for(int i=0; i<=maxIndex && found<=index; i++){
//    if(data.charAt(i)==separator || i==maxIndex){
//        found++;
//        strIndex[0] = strIndex[1]+1;
//        strIndex[1] = (i == maxIndex) ? i+1 : i;
//    }
//  }
//
//  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
//}
//
//int countSpaces(String string){
//  /*
//   * Count how many spaces are in a string.
//   * This will be used to determine how many points are in 
//   * the polygonlist strings.  
//   */
//
//  int number_of_spaces = 0;
//  int i;
//  char space = 32;
//  
//  for (i = 0; i < string.length(); i++){
//    char this_char = string[i];
////    Serial.println(this_char);
//    if (this_char == space){
//      number_of_spaces++;
//    }
//  }
//  return number_of_spaces;
//}
