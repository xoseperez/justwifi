/*

JustWifi 1.1.3

Wifi Manager for ESP8266

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "JustWifi.h"
#include <functional>

//------------------------------------------------------------------------------
// CONSTRUCTOR
//------------------------------------------------------------------------------

JustWifi::JustWifi() {
    _softap.ssid = NULL;
    _timeout = 0;
}

JustWifi::~JustWifi() {
    cleanNetworks();
}

//------------------------------------------------------------------------------
// PRIVATE METHODS
//------------------------------------------------------------------------------

justwifi_states_t JustWifi::_connect(uint8_t id) {

    static uint8_t networkID;
    static justwifi_states_t state = STATE_NOT_CONNECTED;
    static unsigned long timeout;

    // Reset connection process
    if (id != 0xFF) {
        state = STATE_NOT_CONNECTED;
        networkID = id;
    }

    // Get network
    network_t entry = _network_list[networkID];

    // No state or previous network failed
    if (state == STATE_NOT_CONNECTED) {

        WiFi.mode(WIFI_STA);

        // Configure static options
        if (!entry.dhcp) {
            WiFi.config(entry.ip, entry.gw, entry.netmask, entry.dns);
        }

        // Connect
        _doCallback(MESSAGE_CONNECTING, entry.ssid);
        if (entry.channel == 0) {
            WiFi.begin(entry.ssid, entry.pass);
        } else {
            WiFi.begin(entry.ssid, entry.pass, entry.channel, entry.bssid);
        }
        timeout = millis();
        return (state = STATE_CONNECTING);

    }

    // Connected?
    if (WiFi.status() == WL_CONNECTED) {

        // Autoconnect only if DHCP, since it doesn't store static IP data
        WiFi.setAutoConnect(entry.dhcp);

        WiFi.setAutoReconnect(true);
        _doCallback(MESSAGE_CONNECTED);
        return (state = STATE_CONNECTED);

    }

    // Check timeout
    if (millis() - timeout > _connect_timeout) {
        _doCallback(MESSAGE_CONNECT_FAILED, entry.ssid);
        return (state = STATE_NOT_CONNECTED);
    }

    // Still waiting
    _doCallback(MESSAGE_CONNECT_WAITING);
    return state;

}

void JustWifi::_sortByRSSI() {

    bool first = true;
    _bestID = 0xFF;

    for (uint8_t i = 0; i < _network_list.size(); i++) {

        network_t * entry = &_network_list[i];

        // if no data skip
        if (entry->rssi == 0) continue;

        // Empty list
        if (first) {
            first = false;
            _bestID = i;
            entry->next = 0xFF;

        // The best so far
        } else if (entry->rssi > _network_list[_bestID].rssi) {
            entry->next = _bestID;
            _bestID = i;

        // Walk the list
        } else {

            network_t * current = &_network_list[_bestID];
            while (current->next != 0xFF) {
                if (entry->rssi > _network_list[current->next].rssi) {
                    entry->next = current->next;
                    current->next = i;
                    break;
                }
                current = &_network_list[current->next];
            }

            // Place it the last
            if (current->next == 0xFF) {
                current->next = i;
                entry->next = 0xFF;
            }

        }

    }

}

uint8_t JustWifi::_populate(uint8_t networkCount) {

    uint8_t count = 0;

    // Reset RSSI to disable networks that have disappeared
    for (uint8_t j = 0; j < _network_list.size(); j++) {
        _network_list[j].rssi = 0;
    }

    // Populate defined networks with scan data
    for (int8_t i = 0; i < networkCount; ++i) {

        String ssid_scan;
        int32_t rssi_scan;
        uint8_t sec_scan;
        uint8_t* BSSID_scan;
        int32_t chan_scan;
        bool hidden_scan;

        WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);

        bool known = false;

        for (uint8_t j = 0; j < _network_list.size(); j++) {

            network_t * entry = &_network_list[j];

            if (ssid_scan.equals(entry->ssid)) {

                // Check security
                if ((sec_scan != ENC_TYPE_NONE) && (entry->pass == NULL)) continue;

                known = true;
                count++;
                entry->rssi = rssi_scan;
                entry->security = sec_scan;
                entry->channel = chan_scan;
                memcpy((void*) &entry->bssid, (void*) BSSID_scan, sizeof(entry->bssid));
                break;
            }
        }

        char buffer[200];
        sprintf(buffer,
            "%s %s SEC:%s RSSI:%d CH:%d",
            (known ? "-->" : "   "),
            (char *) ssid_scan.c_str(),
            (sec_scan != ENC_TYPE_NONE ? "YES" : "NO "),
            rssi_scan,
            chan_scan
        );
        _doCallback(MESSAGE_FOUND_NETWORK, buffer);

    }

    return count;

}

void JustWifi::_scanNetworks() {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.scanNetworks(true, true);
}

int8_t JustWifi::_scanComplete() {

    static bool scanning = false;

    // Check if scanning
    int8_t scanResult = WiFi.scanComplete();
    if (scanResult == WIFI_SCAN_RUNNING) {
        if (!scanning) {
            _doCallback(MESSAGE_SCANNING);
            scanning = true;
        }
        return WIFI_SCAN_RUNNING;
    }

    scanning = false;

    if (scanResult == WIFI_SCAN_FAILED) {
        _doCallback(MESSAGE_SCAN_FAILED);
        return WIFI_SCAN_FAILED;
    }

    // Check networks
    if (scanResult == 0) {
        _doCallback(MESSAGE_NO_NETWORKS);
        return 0;
    }

    // Populate network list
    uint8_t count = _populate(scanResult);

    if (count == 0) {
        _doCallback(MESSAGE_NO_KNOWN_NETWORKS);
    }

    // Free memory
    WiFi.scanDelete();

    // Sort by RSSI
    _sortByRSSI();

    // Return the number of known available networks
    return count;

}

// _startSTA states:
//  0: Starting / No Connection
//  1: Scanning networks
//  2: Connecting
//  3: Connection successful
justwifi_states_t JustWifi::_startSTA(bool reset) {

    static uint8_t currentID;
    static justwifi_states_t state = STATE_NOT_CONNECTED;

    if (_network_list.size() == 0) {
        return (state = STATE_NOT_CONNECTED);
    }

    // Reset process
    if (reset) state = STATE_NOT_CONNECTED;

    // Starting over
    if (state == STATE_NOT_CONNECTED) {

        WiFi.enableSTA(true);

        if (_scan) {
            _scanNetworks();
            return (state = STATE_SCANNING);
        }

        return (state = _connect(currentID = 0));

    }

    // Scanning
    if (state == STATE_SCANNING) {

        int8_t scanResult = _scanComplete();

        if (scanResult == WIFI_SCAN_FAILED) {
            _scanNetworks();
        }

        if (scanResult < 0) {
            return state;
        }

        if (scanResult == 0) {
            return (state = STATE_NOT_CONNECTED);
        }

        return (state = _connect(currentID = _bestID));

    }

    // Connecting
    if (state == STATE_CONNECTING) {

        state = _connect();

        if (state != STATE_NOT_CONNECTED) {
            return state;
        }

        if (_scan) {
            currentID = _network_list[currentID].next;
            if (currentID == 0xFF) {
                return (state = STATE_NOT_CONNECTED);
            }
        } else {
            currentID++;
            if (currentID == _network_list.size()) {
                return (state = STATE_NOT_CONNECTED);
            }
        }

        return (state = _connect(currentID));

    }

    return state;

}

bool JustWifi::_startAP() {

    // Check if Soft AP configuration defined
    if (!_softap.ssid) {
        _softap.ssid = strdup(WiFi.hostname().c_str());
    }

    _doCallback(MESSAGE_ACCESSPOINT_CREATING);

    if (WiFi.status() == WL_CONNECTED) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_AP);
    }

    // Configure static options
    if (_softap.dhcp) {
        WiFi.softAPConfig(_softap.ip, _softap.gw, _softap.netmask);
    }

    if (_softap.pass) {
        WiFi.softAP(_softap.ssid, _softap.pass);
    } else {
        WiFi.softAP(_softap.ssid);
    }

    _doCallback(MESSAGE_ACCESSPOINT_CREATED);

    return true;

}

void JustWifi::_doCallback(justwifi_messages_t message, char * parameter) {
    if (_callback != NULL) _callback(message, parameter);
}

//------------------------------------------------------------------------------
// CONFIGURATION METHODS
//------------------------------------------------------------------------------

bool JustWifi::cleanNetworks() {
    for (uint8_t i = 0; i < _network_list.size(); i++) {
        network_t entry = _network_list[i];
        if (entry.ssid) free(entry.ssid);
        if (entry.pass) free(entry.pass);
    }
    _network_list.clear();
}

bool JustWifi::addNetwork(
    const char * ssid,
    const char * pass,
    const char * ip,
    const char * gw,
    const char * netmask,
    const char * dns
) {

    network_t new_network;

    // Check SSID too long or missing
    if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
        return false;
    }

    // Check PASS too long
    if (pass && strlen(pass) > 63) {
        return false;
    }

    // Copy network SSID
    new_network.ssid = strdup(ssid);
    if (!new_network.ssid) {
        return false;
    }

    // Copy network PASS
    if (pass && *pass != 0x00) {
        new_network.pass = strdup(pass);
        if (!new_network.pass) {
            free(new_network.ssid);
            return false;
        }
    } else {
        new_network.pass = NULL;
    }

    // Copy static config
    new_network.dhcp = true;
    if (ip && gw && netmask
        && *ip != 0x00 && *gw != 0x00 && *netmask != 0x00) {
        new_network.dhcp = false;
        new_network.ip.fromString(ip);
        new_network.gw.fromString(gw);
        new_network.netmask.fromString(netmask);
    }
    if (dns && *dns != 0x00) {
        new_network.dns.fromString(dns);
    }

    // Defaults
    new_network.rssi = 0;
    new_network.security = 0;
    new_network.channel = 0;
    new_network.next = 0xFF;

    // Store data
    _network_list.push_back(new_network);
    return true;

}

bool JustWifi::setSoftAP(
    const char * ssid,
    const char * pass,
    const char * ip,
    const char * gw,
    const char * netmask
) {

    // Check SSID too long or missing
    if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
        return false;
    }

    // Check PASS too long
    if (pass && strlen(pass) > 63) {
        return false;
    }

    // Copy network SSID
    _softap.ssid = strdup(ssid);
    if (!_softap.ssid) {
        return false;
    }

    // Copy network PASS
    if (pass && *pass != 0x00) {
        _softap.pass = strdup(pass);
        if (!_softap.pass) {
            _softap.ssid = NULL;
            return false;
        }
    }

    // Copy static config
    _softap.dhcp = false;
    if (ip && gw && netmask
        && *ip != 0x00 && *gw != 0x00 && *netmask != 0x00) {
        _softap.dhcp = true;
        _softap.ip.fromString(ip);
        _softap.gw.fromString(gw);
        _softap.netmask.fromString(netmask);
    }

    return true;
}

void JustWifi::setConnectTimeout(unsigned long ms) {
    _connect_timeout = ms;
}

void JustWifi::setReconnectTimeout(unsigned long ms) {
    _reconnect_timeout = ms;
}

void JustWifi::resetReconnectTimeout() {
    _timeout = millis();
}

void JustWifi::setHostname(const char * hostname) {
    WiFi.hostname(hostname);
}

void JustWifi::onMessage(TMessageFunction fn) {
    _callback = fn;
}

//------------------------------------------------------------------------------
// PUBLIC METHODS
//------------------------------------------------------------------------------

wl_status_t JustWifi::getStatus() {
    return WiFi.status();
}

String JustWifi::getAPSSID() {
    return String(_softap.ssid);
}

bool JustWifi::connected() {
    return (WiFi.status() == WL_CONNECTED);
}

bool JustWifi::disconnect() {
    _timeout = 0;
    WiFi.disconnect();
    _doCallback(MESSAGE_DISCONNECTED);
}

void JustWifi::setAPMode(justwifi_ap_modes_t mode) {
    _ap_mode = mode;
}

bool JustWifi::createAP() {
    return _startAP();
}

void JustWifi::scanNetworks(bool scan) {
    _scan = scan;
}

void JustWifi::loop() {

    static bool connecting = false;
    static bool reset = true;
    static justwifi_states_t state = STATE_NOT_CONNECTED;

    if (connecting) {

        // _startSTA may return:
        //  0: Could not connect
        //  1: Scanning networks
        //  2: Connecting
        //  3: Connection successful
        state = _startSTA(reset);
        reset = false;

        if (state == STATE_NOT_CONNECTED) {
            if (_ap_mode != AP_MODE_OFF) _startAP();
            connecting = false;
            _timeout = millis();
        }

        if (state == STATE_CONNECTED) {
            if (_ap_mode == AP_MODE_BOTH) _startAP();
            connecting = false;
        }

    } else {

        wl_status_t status = WiFi.status();
        if (status == WL_DISCONNECTED
            || status == WL_NO_SSID_AVAIL
            || status == WL_IDLE_STATUS
            || status == WL_CONNECT_FAILED) {

            if (
                (_timeout == 0)
                || (
                    (_reconnect_timeout > 0 )
                    && (_network_list.size() > 0)
                    && (millis() - _timeout > _reconnect_timeout)
                )
            ) {

                connecting = true;

            }

        // Capture auto-connections
        } else if (state != STATE_CONNECTED) {
            _doCallback(MESSAGE_CONNECTED);
            state = STATE_CONNECTED;
        }

        reset = true;

    }

}

JustWifi jw;
