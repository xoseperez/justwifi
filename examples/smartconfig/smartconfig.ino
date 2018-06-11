/*

JustWifi - Smart Config example

This example shows how to use Smart Config (a.k.a. ESP Touch) to
provide the connection credentials to a device using a mobile app.

Compatible apps:

* ESP8266 SmartConfig
* IoT SmartConfig

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

    Serial.println("[WIFI] JustWifi Smart Config (ESP TOUCH) example");
    Serial.println("[WIFI] Start your Smart Config APP...");

    // Start Smartconfig join
    jw.startSmartConfig();

}

void loop() {

    // This call takes care of it all
    jw.loop();

    // Small delay to give some breath
    delay(10);

}
