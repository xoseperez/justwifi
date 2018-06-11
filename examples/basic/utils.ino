/*

JustWifi - Basic debugging callback utils

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

The JustWifi library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The JustWifi library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the JustWifi library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <JustWifi.h>

void infoWifi() {

    if (WiFi.isConnected()) {

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
        #if defined(ARDUINO_ARCH_ESP32)
            Serial.printf("[WIFI] HOST  %s\n", WiFi.getHostname());
        #else
            Serial.printf("[WIFI] HOST  %s\n", WiFi.hostname().c_str());
        #endif
        Serial.printf("[WIFI] ----------------------------------------------\n");

    }

    if (WiFi.getMode() & WIFI_AP) {

        Serial.printf("[WIFI] MODE AP --------------------------------------\n");
        Serial.printf("[WIFI] SSID  %s\n", jw.getAPSSID().c_str());
        Serial.printf("[WIFI] IP    %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("[WIFI] MAC   %s\n", WiFi.softAPmacAddress().c_str());
        Serial.printf("[WIFI] ----------------------------------------------\n");

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
        infoWifi();
    }

    if (code == MESSAGE_DISCONNECTED) {
        Serial.printf("[WIFI] Disconnected\n");
    }

    // -------------------------------------------------------------------------

    if (code == MESSAGE_ACCESSPOINT_CREATED) {
        infoWifi();
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

    // ------------------------------------------------------------------------

    if (code == MESSAGE_WPS_START) {
        Serial.printf("[WIFI] WPS started\n");
    }

    if (code == MESSAGE_WPS_SUCCESS) {
        Serial.printf("[WIFI] WPS succeded!\n");
    }

    if (code == MESSAGE_WPS_ERROR) {
        Serial.printf("[WIFI] WPS failed\n");
    }

    // ------------------------------------------------------------------------

    if (code == MESSAGE_SMARTCONFIG_START) {
        Serial.printf("[WIFI] Smart Config started\n");
    }

    if (code == MESSAGE_SMARTCONFIG_SUCCESS) {
        Serial.printf("[WIFI] Smart Config succeded!\n");
    }

    if (code == MESSAGE_SMARTCONFIG_ERROR) {
        Serial.printf("[WIFI] Smart Config failed\n");
    }

};
