#include <wifi.h>
#include <spiffs.h>

// WiFiManager instance
WiFiManager wifiManager;
// Static IP address set on first call to connectWiFi
IPAddress lastStatIP;
// Gateway IP address set on first call to connectWiFi
IPAddress lastStatGateWay;
// Netmask IP address set on first call to connectWiFi
IPAddress lastStatNetMask;
// SSID set on first call to connectWiFi
const char* lastSsidName;
// Flag if original connection was with static or dynamic IP address
bool wasStatic = false;
// Flag if WiFi connection to saved settings was successfull
bool wmIsConnected = false;

/** WiFiUDP class for listening to UDP broadcasts */
WiFiUDP udpListener;
/** Flag if the config has changed */
bool shouldSaveConfig = false;

/**
	connectWiFi
	Connect to WiFi AP or start captive portal if no WiFi credentials are saved
	if WiFi is not found or no credentials are entered for 3 minutes
	module is restarted
	This method variant is for static IP address
*/
IPAddress connectWiFi(IPAddress statIP, IPAddress statGateWay, IPAddress statNetMask, const char* ssidName) {
	doubleLedFlashStart(1);

	// Prepare WiFiManager for connection with fixed IP
	wifiManager.setSTAStaticIPConfig(statIP, statGateWay, statNetMask);
	// Set a timeout of 180 seconds before returning
	wifiManager.setConfigPortalTimeout(180);
	// Try to connect to known network
	if (!wifiManager.autoConnect(ssidName)) {
		// TODO new feature: instead of resetting here, return IPAddress of NULL and set flag for failed connection
		// Set flag for failed connection
		wmIsConnected = false;
		return ipFailed;
		// // If timeout occured try to reset the ESP
		// delay(3000);
		// ESP.reset();
		// delay(5000);
	}
	// Set flag for successfull connection
	wmIsConnected = true;
	doubleLedFlashStop();
	// Save ip, gateway, netmask and ssid name for reconnection
	lastStatIP = statIP;
	lastStatGateWay = statGateWay;
	lastStatNetMask = statNetMask;
	lastSsidName = ssidName;
	// Remember that we connected with static IP address
	wasStatic = true;
	return WiFi.localIP();
}

/**
	connectWiFi
	Connect to WiFi AP or start captive portal if no WiFi credentials are saved
	if WiFi is not found or no credentials are entered for 3 minutes
	the function returns with an IP address of NULL
	This method variant is for dynamic IP address
*/
IPAddress connectWiFi(const char* ssidName) {
	doubleLedFlashStart(1);

	// Set a timeout of 180 seconds before returning
	wifiManager.setConfigPortalTimeout(180);
	// Try to connect to known network
	if (!wifiManager.autoConnect(ssidName)) {
		// Set flag for failed connection
		wmIsConnected = false;
		return ipFailed;
	}
	// Set flag for successfull connection
	wmIsConnected = true;
	doubleLedFlashStop();
	// Save ssid name for reconnection
	lastSsidName = ssidName;
	// Remember that we connected with DHCP IP address
	wasStatic = true;
	return WiFi.localIP();
}

/**
	resetWiFiCredentials
	Delete saved WiFi credentials to force captive portal to be opened
	after next reboot
*/
void resetWiFiCredentials() {
	wifiManager.resetSettings();
	delay(3000);
	ESP.reset();
	delay(5000);
}

/**
	reConnectWiFi
	Reconnect to WiFi AP
	if no WiFi is found for 60 seconds
	module is restarted
*/
void reConnectWiFi() {
	if (wasStatic) {
		connectWiFi(lastStatIP, lastStatGateWay, lastStatNetMask, lastSsidName);
	} else {
		connectWiFi(lastSsidName);
	}
}

/**
 * Return signal strength or 0 if target SSID not found
 *
 * @return <code>int32_t</code>
 *              Signal strength as unsinged int or 0
 */
int32_t getRSSI() {
	/** Number of retries */
	byte retryNum = 0;
	/** Number of available networks */
	byte available_networks;
	/** The SSID we are connected to */
	// String target_ssid(ssid);
	String target_ssid(WiFi.SSID());

	while (retryNum <= 3) {
		retryNum++;
		available_networks= WiFi.scanNetworks();
		if (available_networks == 0) { // Retryone time
			available_networks = WiFi.scanNetworks();
		}

		for (int network = 0; network < available_networks; network++) {
			// if (WiFi.SSID(network).equals(target_ssid)) {
			if (WiFi.SSID(network).equals(WiFi.SSID())) {
				return WiFi.RSSI(network);
			}
		}
	}
	return 0;
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  shouldSaveConfig = true;
}

// For debug over TCP
void sendDebug(String debugMsg, String senderID) {
	doubleLedFlashStart(0.5);
	/** WiFiClient class to create TCP communication */
	WiFiClient tcpDebugClient;

	if (!tcpDebugClient.connect(ipDebug, tcpDebugPort)) {
		Serial.println("connection to Debug Android " + String(ipDebug[0]) + "." + String(ipDebug[1]) + "." + String(ipDebug[2]) + "." + String(ipDebug[3]) + " failed");
		tcpDebugClient.stop();
		doubleLedFlashStop();
		return;
	}

	// String sendMsg = OTA_HOST;
	debugMsg = senderID + " " + debugMsg;
	tcpDebugClient.print(debugMsg);

	tcpDebugClient.stop();
	doubleLedFlashStop();
}

// For debug over TCP to Raspberry Pi
void sendRpiDebug(String debugMsg, String senderID) {
	doubleLedFlashStart(0.5);
	/** WiFiClient class to create TCP communication */
	WiFiClient tcpDebugClient;

	if (!tcpDebugClient.connect(ipMonitor, tcpRpiDebugPort)) {
		Serial.println("connection to Debug PC " + String(ipMonitor[0]) + "." + String(ipMonitor[1]) + "." + String(ipMonitor[2]) + "." + String(ipMonitor[3]) + " failed");
		tcpDebugClient.stop();
		doubleLedFlashStop();
		// Send debug message as well to tablet
		sendDebug(debugMsg, senderID);
		return;
	}

	// String sendMsg = OTA_HOST;
	debugMsg = senderID + " " + debugMsg;
	tcpDebugClient.print(debugMsg);

	tcpDebugClient.stop();

	doubleLedFlashStop();

	// Send debug message as well to tablet
	sendDebug(debugMsg, "");
}

/**
	startListenToUDPbroadcast
	Start to listen for  UDP broadcast message
*/
void startListenToUDPbroadcast() {
	// Start UDP listener
	udpListener.begin(udpBcPort);
}

/**
	stopListenToUDPbroadcast
	Stop to listen for  UDP broadcast message
*/
void stopListenToUDPbroadcast() {
	// Start UDP listener
	udpListener.stop();
}

/**
	getIdFromUDPbroadcast
	Get active device IDs and IPs UDP broadcast message
*/
bool getIdFromUDPbroadcast(int udpMsgLength) {
	doubleLedFlashStart(0.1);
	bool result = false;
	byte udpPacket[udpMsgLength+1];
	IPAddress udpIP = udpListener.remoteIP();

	udpListener.read(udpPacket, udpMsgLength);
	udpPacket[udpMsgLength] = 0;

	udpListener.flush(); // empty UDP buffer for next packet

	// String debugMsg = "UDP broadcast from ";
	// udpIP = udpListener.remoteIP();
	// debugMsg += "Sender IP: " + String(udpIP[0]) + "." + String(udpIP[1]) + "." + String(udpIP[2]) + "." + String(udpIP[3]);
	// debugMsg += "\n Msg: " + String((char *)udpPacket);
	// sendDebug(debugMsg, "WIFI");

	/** Buffer for incoming JSON string */
	DynamicJsonBuffer jsonInBuffer;
	/** Json object for incoming data */
	JsonObject& jsonIn = jsonInBuffer.parseObject((char *)udpPacket);
	if (jsonIn.success() && jsonIn.containsKey("de")) {
		String device = jsonIn["de"]; //String device = jsonIn["device"];
		// String test = "";
		if (device == lightID) { // Found id for connected light device
			lightIp = udpIP;
			result = true;
			// test = "Light ID: " + lightIp.toString();
		} else if (device == camID) { // Found id for connected camera device
			camIp = udpIP;
			result = true;
			// test = "Camera ID: " + camIp.toString();
		} else if (device == secID) { // Found id for connected security device
			secIp = udpIP;
			result = true;
			// test = "Security ID: " + secIp.toString();
		}
		// if (test != "") {
		// 	Serial.println("Found connected " + test);
		// 	sendDebug(test, "wifi.cpp");
		// }
	}
	doubleLedFlashStop();
	return result;
}
