/*
 *  This sketch trys to Connect to the best AP based on a given list
 *
 */

#include <JustWifi.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Ticker.h>

DNSServer dnsServer;
#define DNS_PORT 53
Ticker ticker0;
Ticker ticker1;
Ticker ticker2;
Ticker ticker3;

void mDNSCallback(justwifi_messages_t code, char * parameter) {

    if (code == MESSAGE_CONNECTED) {

        // Configure mDNS
        if (MDNS.begin((char *) WiFi.hostname().c_str())) {
            Serial.printf("[MDNS] OK\n");
        } else {
            Serial.printf("[MDNS] FAIL\n");
        }

    }

}

void CaptivePortalCallback(justwifi_messages_t code, char * parameter) {
    if (code == MESSAGE_ACCESSPOINT_CREATED) {
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    }
}

void infoCallback(justwifi_messages_t code, char * parameter) {

    // -------------------------------------------------------------------------

    if (code == MESSAGE_TURNING_OFF) {
        Serial.printf("[WIFI] Turning OFF\n");
    }

    if (code == MESSAGE_TURNING_ON) {
        Serial.printf("[WIFI] Turning ON\n");
    }

    // -------------------------------------------------------------------------

    if (code == MESSAGE_SCANNING) {
        Serial.printf("[WIFI] Scanning\n");
    }

    if (code == MESSAGE_SCAN_FAILED) {
        Serial.printf("[WIFI] Scan failed\n");
    }

    if (code == MESSAGE_NO_NETWORKS) {
        Serial.printf("[WIFI] No networks found\n");
    }

    if (code == MESSAGE_NO_KNOWN_NETWORKS) {
        Serial.printf("[WIFI] No known networks found\n");
    }

    if (code == MESSAGE_FOUND_NETWORK) {
        Serial.printf("[WIFI] %s\n", parameter);
    }

    // -------------------------------------------------------------------------

    if (code == MESSAGE_CONNECTING) {
        Serial.printf("[WIFI] Connecting to %s\n", parameter);
    }

    if (code == MESSAGE_CONNECT_WAITING) {
        // too much noise
    }

    if (code == MESSAGE_CONNECT_FAILED) {
        Serial.printf("[WIFI] Could not connect to %s\n", parameter);
    }

    if (code == MESSAGE_CONNECTED) {

        uint8_t * bssid = WiFi.BSSID();

        Serial.printf("[WIFI] MODE STA -------------------------------------\n");
        Serial.printf("[WIFI] SSID  %s\n", WiFi.SSID().c_str());
        Serial.printf("[WIFI] BSSID %02X:%02X:%02X:%02X:%02X:%02X\n",
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]
        );
        Serial.printf("[WIFI] CH    %d\n", WiFi.channel());
        Serial.printf("[WIFI] RSSI  %d\n", WiFi.RSSI());
        Serial.printf("[WIFI] IP    %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("[WIFI] MAC   %s\n", WiFi.macAddress().c_str());
        Serial.printf("[WIFI] GW    %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("[WIFI] MASK  %s\n", WiFi.subnetMask().toString().c_str());
        Serial.printf("[WIFI] DNS   %s\n", WiFi.dnsIP().toString().c_str());
        Serial.printf("[WIFI] HOST  %s\n", WiFi.hostname().c_str());
        Serial.printf("[WIFI] ----------------------------------------------\n");

    }

    if (code == MESSAGE_DISCONNECTED) {
        Serial.printf("[WIFI] Disconnected\n");
    }

    // -------------------------------------------------------------------------

    if (code == MESSAGE_ACCESSPOINT_CREATED) {

        Serial.printf("[WIFI] MODE AP --------------------------------------\n");
        Serial.printf("[WIFI] SSID  %s\n", jw.getAPSSID().c_str());
        Serial.printf("[WIFI] IP    %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("[WIFI] MAC   %s\n", WiFi.softAPmacAddress().c_str());
        Serial.printf("[WIFI] ----------------------------------------------\n");

    }

    if (code == MESSAGE_ACCESSPOINT_DESTROYED) {
        Serial.printf("[WIFI] Disconnecting access point\n");
    }

    if (code == MESSAGE_ACCESSPOINT_CREATING) {
        Serial.printf("[WIFI] Creating access point\n");
    }

    if (code == MESSAGE_ACCESSPOINT_FAILED) {
        Serial.printf("[WIFI] Could not create access point\n");
    }

};

void setup() {

    Serial.begin(115200);
    delay(1000);

    // Set WIFI hostname (otherwise it would be ESP_XXXXXX)
    //jw.setHostname("JUSTWIFI");

    // Enable STA mode (connecting to a router)
    jw.enableSTA(false);

    // Configure it to scan available networks and connect in order of dBm
    jw.scanNetworks(true);

    // Clean existing network configuration
    jw.cleanNetworks();

    // Add a network with password
    jw.addNetwork("home", "password");

    // Add a network with static IP
    jw.addNetwork("moms", "anotherpassword", "192.168.1.201", "192.168.1.1", "255.255.255.0");

    // Add an open network
    jw.addNetwork("work");

    // Create Access Point if STA fails
    jw.enableAPFailsafe(true);

    // Set open access point, do not define to use the hostname
    //jw.setSoftAP("JUSTWIFI");

    // Set password protected access point
    //jw.setSoftAP("JUSTWIFI", "PASSWORD");

    // Callbacks
    jw.subscribe(infoCallback);
    jw.subscribe(mDNSCallback);
    jw.subscribe(CaptivePortalCallback);

	Serial.println("[WIFI] Connecting Wifi...");

    // Actions
    ticker0.attach(10, status);
    ticker1.once(5, actionAP);
    ticker2.once(20, actionSTA);
    ticker3.once(40, actionNoAP);

}

void actionNoAP() {
    jw.destroyAP();
}

void actionAP() {
    jw.createAP();
}

void actionSTA() {
    jw.enableSTA(true);
}

void actionDisconnect() {
    jw.disconnect();
}

void status() {
    Serial.printf("[WIFI] STA  : %s\n", jw.connected() ? "YES" : "NO");
    Serial.printf("[WIFI] AP   : %s\n", jw.connectable() ? "YES" : "NO");
    Serial.printf("[WIFI] #    : %d\n", WiFi.softAPgetStationNum());
}

void loop() {

    // This call takes care of it all
    jw.loop();

    // Small delay to give some breath
    delay(10);

}
