#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "index.h"
#include <cmath>

// #define M_PI 3.14159265358979323846264338327950288
#define rxGPS 5
#define txGPS 16

//approx earth's radius in meters
#define EARTH_RADIUS 6371000  

double haversine(double lat1, double lon1, double lat2, double lon2) {
    //degree to radian 
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;

    // coordinate difference calculate
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    // Haversine formula
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1) * cos(lat2) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    // Distance in meters
    return EARTH_RADIUS * c;
}
// //room
double targetLat = 25.4951497;
double targetLon = 81.8691659;

//tondon
// double targetLat = 25.4955875;
// double targetLon = 81.8684455;



//allocate memory for all the necessary variables 
String ssid = "";
String password = "";
int WiFiConnectMode = 1; 

double latitude, longitude;
int sat;
String date;
char lati[12];
char longi[12];
int targetStatus;
int fence;
char cumulativeAngle[12];
int deviceStatus = 0;


//init the gps module at specified pins 
//make instance of Tinygps++ and init a web server (http port 80)
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;
ESP8266WebServer gpsServer(80);

//calling all necessary functions before the main code loop
void connectWifi();
void updateLatLon();
void pip(double latitude, double longitude, double targetLat, double targetLon);
void handleRoot();
void fenceSelect();
void gps_data();



//setting up the state of hardware
void setup(){
    Serial.begin(9600);
    gpsSerial.begin(9600);
    connectWifi();

    gpsServer.on("/", handleRoot);
    gpsServer.on("/status", fenceSelect);
    gpsServer.on("/values", gps_data);
    gpsServer.begin();
}
 
//main code to run on loop 
void loop(){
    while (gpsSerial.available()){
        deviceStatus = 1;
        updateLatLon();
        pip(latitude, longitude, targetLat, targetLon);        
        gpsServer.handleClient();
    }
    gpsServer.handleClient();
}

void connectWifi(){
    //hardcoding wifi config (ssid, password)
    String ssid = "narzo50"; //Access point ssid
    String password = "qk7drays"; //Access point password
    WiFi.begin(ssid,password);
    Serial.println("------------------------------------------------");
    Serial.print("Connecting to Access Point ");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("------------------------------------------------");
}

void updateLatLon(){
    if (gps.encode(gpsSerial.read()))
    {  
      sat = gps.satellites.value();
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      dtostrf(latitude,9,7,lati);
      dtostrf(longitude,9,7,longi);
      Serial.print("SATS: ");
      Serial.println(sat);
      Serial.print("LAT: ");
      Serial.println(latitude,6);
      Serial.print("LONG: ");
      Serial.println(longitude,6);
      Serial.print("ALT: ");
      Serial.println(gps.altitude.meters());
      Serial.print("SPEED: ");
      Serial.println(gps.speed.mps());
 
      Serial.print("Date: ");
      date = String(gps.date.day())+"/"+gps.date.month()+"/"+gps.date.year();
      Serial.println(date);
 
      Serial.print("Hour: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second());
      Serial.println(targetStatus);
      Serial.println(cumulativeAngle);
      Serial.println("---------------------------");
      delay(1000);
    }
}


void pip(double latitude, double longitude, double targetLat, double targetLon) {
    double radius = 34.0;  // 34 meters radius
    double distance = haversine(latitude, longitude, targetLat, targetLon);
    
    // Check if the distance is within the 34 meters radius
    if (distance <= radius) {
        targetStatus = 1;  // Inside the radius
    } else {
        targetStatus = 0;  // Outside the radius
    }
}


void handleRoot(){
    String s = webpage;
    gpsServer.send(200, "text/html", s);
}

void fenceSelect(){
    // fence = gpsServer.arg("fenceValue").toInt();
    //hardcoding fence 1 only using 1 fence at a time
    fence = 1;
    gpsServer.send(200, "text/plane", String(fence));
}

void gps_data(){
    String payload = String(sat) + "#" + date + "#" + lati + "#" + longi;
    if(targetStatus == 0)
        payload = payload + "#outside";
    else
        payload = payload + "#inside";
    payload = payload + "#" + cumulativeAngle;
    if(deviceStatus == 0)
        payload = payload + "#offline";
    else
        payload = payload + "#online";
    gpsServer.send(200, "text/plane", payload);
}