#include "spiffs.h"

/** Device location */
String devLoc = "unknown";
/** Id for additional light */
String lightID = "";
/** Network address for additional light */
IPAddress lightIp (0, 0, 0, 0);
/** Id for camera */
String camID = "";
/** Network address for attached camera */
IPAddress camIp (0, 0, 0, 0);
/** Id for additional security device */
String secID = "";
/** Network address for connected security device */
IPAddress secIp (0, 0, 0, 0);

bool formatSPIFFS(String otaHost) {
	String debugMsg;
	bool result;
	uint32_t startTime = millis();
	sendRpiDebug("SPIFFS format started", otaHost);
	if (SPIFFS.format()){
		sendRpiDebug("SPIFFS formatted", otaHost);
		result = true;
	} else {
		sendRpiDebug("SPIFFS format failed", otaHost);
		result = false;
	}
	uint32_t endTime = millis();
	debugMsg = "Formatting took " + String(endTime-startTime) + "ms";
	sendRpiDebug(debugMsg, otaHost);
	return result;
}

boolean getConfigEntry(String entryID, char *value) {
	if (SPIFFS.exists("/config.json")) {
		File configFile = SPIFFS.open("/config.json", "r");
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);
		configFile.readBytes(buf.get(), size);
		configFile.close();
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (json.success()) {
			if (json.containsKey(entryID)) {
				strcpy(value,json[entryID]);
				return true;
			}
		}
	}
	return false;
}

boolean saveConfigEntry(String entryID, String value) {
	bool configFileExists = false;
	DynamicJsonBuffer jsonBuffer;
	File configFile;
	String jsonAsString;
	if (SPIFFS.exists("/config.json")) {
		// Some values already exists, read them first
		configFile = SPIFFS.open("/config.json", "r");
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);
		configFile.readBytes(buf.get(), size);
		configFile.close();
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (json.success()) {
			configFileExists = true;
			json.printTo(jsonAsString);
		}
	}

	// Write new value to config file
	configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		sendDebug("failed to open config file for writing", "SPIFFS");
		return false;
	} else {
		String debugMsg;
		if (configFileExists) {
			JsonObject& json = jsonBuffer.parseObject(jsonAsString);
			// change existing value
			json[entryID] = value;
			json.printTo(configFile);
			json.printTo(debugMsg);
		} else {
			JsonObject& json = jsonBuffer.createObject();
			// add new value
			json[entryID] = value;
			json.printTo(configFile);
			json.printTo(debugMsg);
		}
		configFile.close();
		sendDebug(debugMsg, "SPIFFS");
	}
	return true;
}
