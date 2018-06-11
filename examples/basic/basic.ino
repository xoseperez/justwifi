/*

JustWifi - Basic example

This example shows how to define different networks and enable network scanning

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

void setup() {

    Serial.begin(115200);
    delay(2000);
    Serial.println();
    Serial.println();

    // -------------------------------------------------------------------------

    // Set WIFI hostname (otherwise it would be ESP-XXXXXX)
    jw.setHostname("justwifi");

    // Callbacks
    jw.subscribe(infoCallback);

    // -------------------------------------------------------------------------

    // AP mode only as fallback
    jw.enableAP(false);
    jw.enableAPFallback(true);

    // -------------------------------------------------------------------------

    // Enable STA mode (connecting to a router)
    jw.enableSTA(true);

    // Configure it to scan available networks and connect in order of dBm
    jw.enableScan(true);

    // Clean existing network configuration
    jw.cleanNetworks();

    // Add a network with password
    jw.addNetwork("home", "password");

    // Add a network with static IP
    jw.addNetwork("moms", "anotherpassword", "192.168.1.201", "192.168.1.1", "255.255.255.0");

    // Add an open network
    jw.addNetwork("work");

    // -------------------------------------------------------------------------

    Serial.println("[WIFI] Connecting Wifi...");

}

void loop() {

    // This call takes care of it all
    jw.loop();

    // Small delay to give some breath
    delay(10);

}
