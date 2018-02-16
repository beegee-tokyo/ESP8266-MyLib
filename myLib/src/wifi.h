#ifndef wifi_h
#define wifi_h

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <leds.h>
#include <wifiAPinfo.h>

/** WiFiManager instance */
extern WiFiManager wifiManager;
// Flag if WiFi connection to saved settings was successfull
extern bool wmIsConnected;
/** WiFiUDP class for listening to UDP broadcasts */
extern WiFiUDP udpListener;
/** Flag if the config has changed */
extern bool shouldSaveConfig;

IPAddress connectWiFi(IPAddress statIP, IPAddress statGateWay, IPAddress statNetMask, const char* ssidName);
IPAddress connectWiFi(const char* ssidName);
void resetWiFiCredentials();
void reConnectWiFi();
int32_t getRSSI();
void sendDebug(String debugMsg, String senderID);
void sendRpiDebug(String debugMsg, String senderID);
void saveConfigCallback ();
void startListenToUDPbroadcast();
void stopListenToUDPbroadcast();
bool getIdFromUDPbroadcast(int udpMsgLength);
#endif
