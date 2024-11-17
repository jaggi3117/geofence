#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "index.h"

#define M_PI 3.14159265358979323846264338327950288
#define rxGPS 5
#define txGPS 16

// const double fences[1][10][2] = {{{25.496294, 81.868131},
//                                   {25.496078, 81.868620},
//                                   {25.495740, 81.869094},
//                                   {25.495109, 81.870121},
//                                   {25.494715, 81.869881},
//                                   {25.494282, 81.869257},
//                                   {25.494875, 81.868203},
//                                   {25.495429, 81.867455},
//                                   {25.495779, 81.867627},
//                                   {25.496294, 81.868131},}
// };

//putting fences inside the memory 
//note -: first and last point should be same, polygon should be closed
//athletics ground
const float fences[1][10][2] = {{{25.495637, 81.863769},
                                  {25.495155, 81.8637389},
                                  {25.494293, 81.8637764},
                                  {25.494258, 81.864641},
                                  {25.494242, 81.865572},
                                  {25.494869, 81.865544},
                                  {25.495383, 81.865506},
                                  {25.495477, 81.864749},
                                  {25.495477, 81.864363},
                                  {25.495637, 81.863769},}
};

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
void pip();
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
        pip();        
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
      Serial.println("---------------------------");
      delay(1000);
    }
}

// void pip(){
//     int fenceSize = sizeof(fences[fence - 1])/sizeof(fences[fence - 1][0]);
//     double vectors[fenceSize][2];
//     for(int i = 0; i < fenceSize; i++){
//         vectors[i][0] = fences[fence - 1][i][0] - latitude;
//         vectors[i][1] = fences[fence - 1][i][1] - longitude;
//     }
//     double angle = 0;
//     double num, den;
//     for(int i = 0; i < fenceSize; i++){
//         num = (vectors[i%fenceSize][0])*(vectors[(i+1)%fenceSize][0])+ (vectors[i%fenceSize][1])*(vectors[(i+1)%fenceSize][1]);
//         den = (sqrt(pow(vectors[i%fenceSize][0],2) + pow(vectors[i%fenceSize][1],2)))*(sqrt(pow(vectors[(i+1)%fenceSize][0],2) + pow(vectors[(i+1)%fenceSize][1],2)));
//         angle = angle + (180*acos(num/den)/M_PI);
//     }
//     dtostrf(angle,9,7,cumulativeAngle);
//     if(angle > 358 && angle < 362)
//         targetStatus = 1;
//     else
//         targetStatus = 0;

// }

void pip() {
    int fenceSize = sizeof(fences[fence - 1]) / sizeof(fences[fence - 1][0]);
    
    // Shoelace algorithm to calculate the signed area of the polygon
    double area = 0;
    for (int i = 0; i < fenceSize; i++) {
        int next = (i + 1) % fenceSize;
        area += (fences[fence - 1][i][0] * fences[fence - 1][next][1]) - 
                (fences[fence - 1][next][0] * fences[fence - 1][i][1]);
    }
    area = abs(area) / 2.0;  // Area of the polygon

    // Now check if the point (latitude, longitude) is inside the polygon using the same method
    double area1 = 0, area2 = 0;
    for (int i = 0; i < fenceSize; i++) {
        int next = (i + 1) % fenceSize;
        // Calculate area between the point and the edges
        area1 += (fences[fence - 1][i][0] * fences[fence - 1][next][1]) - 
                 (fences[fence - 1][next][0] * fences[fence - 1][i][1]);
        area2 += (fences[fence - 1][i][0] * latitude) - (latitude * fences[fence - 1][next][1]);
    }

    // Check if the areas match
    if (abs(area1 - area2) < 1e-6) {
        targetStatus = 1;  // Point is inside the polygon
    } else {
        targetStatus = 0;  // Point is outside the polygon
    }

    // If the point is inside the polygon, calculate the angle as before
    double vectors[fenceSize][2];
    double angle = 0;
    for (int i = 0; i < fenceSize; i++) {
        vectors[i][0] = fences[fence - 1][i][0] - latitude;
        vectors[i][1] = fences[fence - 1][i][1] - longitude;
    }
    double num, den;
    for (int i = 0; i < fenceSize; i++) {
        num = (vectors[i % fenceSize][0]) * (vectors[(i + 1) % fenceSize][0]) +
              (vectors[i % fenceSize][1]) * (vectors[(i + 1) % fenceSize][1]);
        den = (sqrt(pow(vectors[i % fenceSize][0], 2) + pow(vectors[i % fenceSize][1], 2))) *
              (sqrt(pow(vectors[(i + 1) % fenceSize][0], 2) + pow(vectors[(i + 1) % fenceSize][1], 2)));
        angle = angle + (180 * acos(num / den) / M_PI);
    }

    // Convert the angle to a string (cumulativeAngle)
    dtostrf(angle, 9, 7, cumulativeAngle);
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