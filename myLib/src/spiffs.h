#include <Arduino.h>
#include <FS.h>
#include "wifi.h"
#include <ArduinoJson.h>

/** Device location */
extern String devLoc;
/** Id for additional light */
extern String lightID;
/** Network address for additional light */
extern IPAddress lightIp;
/** Id for camera */
extern String camID;
/** Network address for attached camera */
extern IPAddress camIp;
/** Id for additional security device */
extern String secID;
/** Network address for connected security device */
extern IPAddress secIp;

bool formatSPIFFS(String otaHost);
boolean getConfigEntry(String entryID, char *value);
boolean saveConfigEntry(String entryID, String value);
