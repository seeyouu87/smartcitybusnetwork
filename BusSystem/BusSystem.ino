#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <LGSM.h>
#include <SPI.h>
#include "PubNub.h"

//import all the necessary files for GPRS connectivity
#include "LGPRS.h"
#include "LGPRSClient.h"

#include <LGPS.h>
#include "GPSWaypoint.h"
#include "GPS_functions.h"

#define pushButton 10

GPSWaypoint* gpsPosition;


//define the required keys for using PubNub
char pubkey[] = "pub-c-5fc603c1-9b43-443e-bc21-0d5b0395ffdd";//"demo";
char subkey[] = "sub-c-b96d0938-83dc-11e5-9e96-02ee2ddab7fe";//"demo";
char channel[] = "bus-map";

void setup()
{
    pinMode(pushButton, INPUT);

    Serial.begin(9600);
    Serial.println("Serial setup");

    //Connect to the GRPS network in order to send/receive PubNub messages
    Serial.println("Attach to GPRS network with correct APN settings from your mobile network provider");
    //example here is with mobile provider EE in the UK
    //attachGPRS(const char *apn, const char *username, const char *password);
  
     while (!LGPRS.attachGPRS("sunsurf", "65", "user123")) //m1 APN setting
    {
    Serial.println(" No GPRS ");
    delay(1000);
    }

    Serial.println("LGPRS setup");

    PubNub.begin(pubkey, subkey);
    Serial.println("PubNub setup");

    LGPS.powerOn();
}


void loop()
{
    Serial.println("Getting GPS Data");
    //Aquire GPS position
    char GPS_formatted[] = "GPS fixed";
    gpsPosition = new GPSWaypoint();
    gpsSentenceInfoStruct gpsDataStruct;
    getGPSData(gpsDataStruct, GPS_formatted, gpsPosition);
    Serial.println(" GPS Data aquired");


    char* buffer_latitude = new char[30];
    sprintf(buffer_latitude, "%2.6f", gpsPosition->latitude);

    char* buffer_longitude = new char[30];
    sprintf(buffer_longitude, "%2.6f", gpsPosition->longitude);
    int buttonState = digitalRead(pushButton);
    String status ="OK";
    if(buttonState==0){
      status = "Break Down";
    }
    String upload_GPS = "[{\"latlng\":[" + String(buffer_latitude) + "," + String(buffer_longitude)+ "], \"data\":[\""+gpsPosition->speed+"\",\""+ status +"\"]}]";
    
    const char* upload_char = upload_GPS.c_str();
     
    //Once Position is Aquired, upload it to PubNub
    LGPRSClient *client;

    
    Serial.println("publishing a message");
    client = PubNub.publish(channel, upload_char, 60);
    if (!client) {
        Serial.println("publishing error");
        delay(1000);
        return;
    }
    while (client->connected()) {
        while (client->connected() && !client->available()); // wait
        char c = client->read();
        Serial.print(c);
    }
    client->stop();
    Serial.println();
    flash(true);


    delay(5000);
}
