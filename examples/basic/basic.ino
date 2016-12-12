/*
 *  This sketch trys to Connect to the best AP based on a given list
 *
 */

#include <JustWifi.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

void setup() {

    Serial.begin(115200);
    delay(10);

    // Set WIFI hostname (otherwise it would be ESP_XXXXXX)
    //jw.setHostname("justwifi");

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

    // Do not create Access Point
    // jw.setAPMode(AP_MODE_OFF);

    // Create Access Point only if STA fails
    jw.setAPMode(AP_MODE_ALONE);

    // Create Access Point and STA
    //jw.setAPMode(AP_MODE_BOTH);

    // Set open access point, do not define to use the hostname
    //jw.setSoftAP("JUSTWIFI");

    // Set password protected access point
    // jw.setSoftAP("JUSTWIFI", "PASSWORD");

    // Message callbacks
    jw.onMessage([](justwifi_messages_t code, char * parameter) {

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

            Serial.printf("[WIFI] MODE STA -------------------------------------\n");
            Serial.printf("[WIFI] SSID %s\n", WiFi.SSID().c_str());
            Serial.printf("[WIFI] IP   %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("[WIFI] MAC  %s\n", WiFi.macAddress().c_str());
            Serial.printf("[WIFI] GW   %s\n", WiFi.gatewayIP().toString().c_str());
            Serial.printf("[WIFI] MASK %s\n", WiFi.subnetMask().toString().c_str());
            Serial.printf("[WIFI] DNS  %s\n", WiFi.dnsIP().toString().c_str());
            Serial.printf("[WIFI] HOST %s\n", WiFi.hostname().c_str());
            Serial.printf("[WIFI] ----------------------------------------------\n");

            // Configure mDNS
            if (MDNS.begin((char *) WiFi.hostname().c_str())) {
                Serial.printf("[MDNS] OK\n");
            } else {
                Serial.printf("[MDNS] FAIL\n");
            }

        }

        if (code == MESSAGE_ACCESSPOINT_CREATED) {

            Serial.printf("[WIFI] MODE AP --------------------------------------\n");
            Serial.printf("[WIFI] SSID %s\n", jw.getAPSSID().c_str());
            Serial.printf("[WIFI] IP   %s\n", WiFi.softAPIP().toString().c_str());
            Serial.printf("[WIFI] MAC  %s\n", WiFi.softAPmacAddress().c_str());
            Serial.printf("[WIFI] ----------------------------------------------\n");

        }

        if (code == MESSAGE_DISCONNECTED) {
            Serial.printf("[WIFI] Disconnected\n");
        }

        if (code == MESSAGE_ACCESSPOINT_CREATING) {
            Serial.printf("[WIFI] Creating access point\n");
        }

        if (code == MESSAGE_ACCESSPOINT_FAILED) {
            Serial.printf("[WIFI] Could not create access point\n");
        }

    });

	Serial.println("[WIFI] Connecting Wifi...");

}

void loop() {

    // This call takes care of it all
    jw.loop();

    // Small delay to give some breath
    delay(10);

}
