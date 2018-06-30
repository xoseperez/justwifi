/*

JustWifi 2.0.0

Wifi Manager for ESP8266

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

#include "JustWifi.h"
#include <functional>

// -----------------------------------------------------------------------------
// WPS callbacks
// -----------------------------------------------------------------------------

#if defined(JUSTWIFI_ENABLE_WPS)

#include "user_interface.h"

wps_cb_status _jw_wps_status;

void _jw_wps_status_cb(wps_cb_status status) {
    _jw_wps_status = status;
}

#endif // defined(JUSTWIFI_ENABLE_WPS)

//------------------------------------------------------------------------------
// CONSTRUCTOR
//------------------------------------------------------------------------------

JustWifi::JustWifi() {
    _softap.ssid = NULL;
    _timeout = 0;
    WiFi.enableAP(false);
    WiFi.enableSTA(false);
    snprintf_P(_hostname, sizeof(_hostname), PSTR("ESP_%06X"), ESP.getChipId());
}

JustWifi::~JustWifi() {
    cleanNetworks();
}

//------------------------------------------------------------------------------
// PRIVATE METHODS
//------------------------------------------------------------------------------

void JustWifi::_disable() {

    // See https://github.com/esp8266/Arduino/issues/2186
    if (strncmp_P(ESP.getSdkVersion(), PSTR("1.5.3"), 5) == 0) {
        if ((WiFi.getMode() & WIFI_AP) > 0) {
            WiFi.mode(WIFI_OFF);
            delay(10);
            WiFi.enableAP(true);
        } else {
            WiFi.mode(WIFI_OFF);
        }

    }

}
uint8_t JustWifi::_sortByRSSI() {

    bool first = true;
    uint8_t bestID = 0xFF;

    for (uint8_t i = 0; i < _network_list.size(); i++) {

        network_t * entry = &_network_list[i];

        // if no data skip
        if (entry->rssi == 0) continue;

        // Empty list
        if (first) {
            first = false;
            bestID = i;
            entry->next = 0xFF;

        // The best so far
        } else if (entry->rssi > _network_list[bestID].rssi) {
            entry->next = bestID;
            bestID = i;

        // Walk the list
        } else {

            network_t * current = &_network_list[bestID];
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

    return bestID;

}

String JustWifi::_encodingString(uint8_t security) {
    if (security == ENC_TYPE_WEP) return String("WEP ");
    if (security == ENC_TYPE_TKIP) return String("WPA ");
    if (security == ENC_TYPE_CCMP) return String("WPA2");
    if (security == ENC_TYPE_AUTO) return String("AUTO");
    return String("OPEN");
}

uint8_t JustWifi::_populate(uint8_t networkCount) {

    uint8_t count = 0;

    // Reset RSSI to disable networks that have disappeared
    for (uint8_t j = 0; j < _network_list.size(); j++) {
        _network_list[j].rssi = 0;
        _network_list[j].scanned = false;
    }

    String ssid_scan;
    int32_t rssi_scan;
    uint8_t sec_scan;
    uint8_t* BSSID_scan;
    int32_t chan_scan;
    bool hidden_scan;

    // Populate defined networks with scan data
    for (int8_t i = 0; i < networkCount; ++i) {

        WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);

        bool known = false;

        for (uint8_t j = 0; j < _network_list.size(); j++) {

            network_t * entry = &_network_list[j];

            if (ssid_scan.equals(entry->ssid)) {

                // Check security
                if ((sec_scan != ENC_TYPE_NONE) && (entry->pass == NULL)) continue;

                // In case of several networks with the same SSID
                // we want to get the one with the best RSSI
                // Thanks to Robert (robi772 @ bitbucket.org)
                if (entry->rssi < rssi_scan || entry->rssi == 0) {
                    entry->rssi = rssi_scan;
                    entry->security = sec_scan;
                    entry->channel = chan_scan;
                    entry->scanned = true;
                    memcpy((void*) &entry->bssid, (void*) BSSID_scan, sizeof(entry->bssid));
                }

                count++;
                known = true;
                break;

            }

        }

		{
		    char buffer[128];
		    sprintf_P(buffer,
		        PSTR("%s BSSID: %02X:%02X:%02X:%02X:%02X:%02X CH: %2d RSSI: %3d SEC: %s SSID: %s"),
		        (known ? "-->" : "   "),
		        BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5],
		        chan_scan,
                rssi_scan,
                _encodingString(sec_scan).c_str(),
		        ssid_scan.c_str()
		    );
		    _doCallback(MESSAGE_FOUND_NETWORK, buffer);
		}

    }

    return count;

}

uint8_t JustWifi::_doSTA(uint8_t id) {

    static uint8_t networkID;
    static uint8_t state = RESPONSE_START;
    static unsigned long timeout;

    // Reset connection process
    if (id != 0xFF) {
        state = RESPONSE_START;
        networkID = id;
    }

    // Get network
    network_t entry = _network_list[networkID];

    // No state or previous network failed
    if (RESPONSE_START == state) {

        WiFi.persistent(false);
        _disable();
        WiFi.enableSTA(true);
        WiFi.hostname(_hostname);

        // Configure static options
        if (!entry.dhcp) {
            WiFi.config(entry.ip, entry.gw, entry.netmask, entry.dns);
        }

        // Connect
		{
            char buffer[128];
            if (entry.scanned) {
                snprintf_P(buffer, sizeof(buffer),
                    PSTR("BSSID: %02X:%02X:%02X:%02X:%02X:%02X CH: %02d, RSSI: %3d, SEC: %s, SSID: %s"),
                    entry.bssid[0], entry.bssid[1], entry.bssid[2], entry.bssid[3], entry.bssid[4], entry.bssid[5],
                    entry.channel,
                    entry.rssi,
                    _encodingString(entry.security).c_str(),
                    entry.ssid
                );
            } else {
                snprintf_P(buffer, sizeof(buffer), PSTR("SSID: %s"), entry.ssid);
            }
		    _doCallback(MESSAGE_CONNECTING, buffer);
        }

        if (entry.channel == 0) {
            WiFi.begin(entry.ssid, entry.pass);
        } else {
            WiFi.begin(entry.ssid, entry.pass, entry.channel, entry.bssid);
        }

        timeout = millis();
        return (state = RESPONSE_WAIT);

    }

    // Connected?
    if (WiFi.status() == WL_CONNECTED) {

        // Autoconnect only if DHCP, since it doesn't store static IP data
        WiFi.setAutoConnect(entry.dhcp);

        WiFi.setAutoReconnect(true);
        _doCallback(MESSAGE_CONNECTED);
        return (state = RESPONSE_OK);

    }

    // Check timeout
    if (millis() - timeout > _connect_timeout) {
        WiFi.enableSTA(false);
        _doCallback(MESSAGE_CONNECT_FAILED, entry.ssid);
        return (state = RESPONSE_FAIL);
    }

    // Still waiting
    _doCallback(MESSAGE_CONNECT_WAITING);
    return state;

}

bool JustWifi::_doAP() {

    // If already created recreate
    if (_ap_connected) enableAP(false);

    // Check if Soft AP configuration defined
    if (!_softap.ssid) {
        _softap.ssid = strdup(_hostname);
    }

    _doCallback(MESSAGE_ACCESSPOINT_CREATING);

    WiFi.enableAP(true);

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

    _ap_connected = true;
    return true;

}

uint8_t JustWifi::_doScan() {

    static bool scanning = false;

    // If not scanning, start scan
    if (false == scanning) {
        WiFi.disconnect();
        WiFi.enableSTA(true);
        WiFi.scanNetworks(true, true);
        _doCallback(MESSAGE_SCANNING);
        scanning = true;
        return RESPONSE_WAIT;
    }

    // Check if scanning
    int8_t scanResult = WiFi.scanComplete();
    if (WIFI_SCAN_RUNNING == scanResult) {
        return RESPONSE_WAIT;
    }

    // Scan finished
    scanning = false;

    // Sometimes the scan fails,
    // this will force the scan to restart
    if (WIFI_SCAN_FAILED == scanResult) {
        _doCallback(MESSAGE_SCAN_FAILED);
        return RESPONSE_WAIT;
    }

    // Check networks
    if (0 == scanResult) {
        _doCallback(MESSAGE_NO_NETWORKS);
        return RESPONSE_FAIL;
    }

    // Populate network list
    uint8_t count = _populate(scanResult);

    // Free memory
    WiFi.scanDelete();

    if (0 == count) {
        _doCallback(MESSAGE_NO_KNOWN_NETWORKS);
        return RESPONSE_FAIL;
    }

    // Sort networks by RSSI
    _currentID = _sortByRSSI();
    return RESPONSE_OK;

}

void JustWifi::_doCallback(justwifi_messages_t message, char * parameter) {
    for (unsigned char i=0; i < _callbacks.size(); i++) {
        (_callbacks[i])(message, parameter);
    }
}

String JustWifi::_MAC2String(const unsigned char* mac) {
    char buffer[20];
    snprintf(
        buffer, sizeof(buffer),
        "%02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
    return String(buffer);
}

void JustWifi::_machine() {

    #if false
        static unsigned char previous = 0xFF;
        if (_state != previous) {
            previous = _state;
            Serial.printf("_state: %u, WiFi.getMode(): %u\n", _state, WiFi.getMode());
        }
    #endif

    switch(_state) {

        // ---------------------------------------------------------------------

        case STATE_IDLE:

            // Should we connect in STA mode?
            if (WiFi.status() != WL_CONNECTED) {

                if (_sta_enabled) {
                    if (_network_list.size() > 0) {
                        if ((0 == _timeout) || ((_reconnect_timeout > 0) && (millis() - _timeout > _reconnect_timeout))) {
                            _currentID = 0;
                            _state = _scan ? STATE_SCAN_START : STATE_STA_START;
                            return;
                        }
                    }
                }

                // Fallback
                if (!_ap_connected & _ap_fallback_enabled) {
                    _state = STATE_FALLBACK;
                }

            }


            break;

        // ---------------------------------------------------------------------

        case STATE_SCAN_START:
            _doScan();
            _state = STATE_SCAN_ONGOING;
            break;

        case STATE_SCAN_ONGOING:
            {
                uint8_t response = _doScan();
                if (RESPONSE_OK == response) {
                    _state = STATE_STA_START;
                } else if (RESPONSE_FAIL == response) {
                    _state = STATE_FALLBACK;
                }
            }
            break;

        // ---------------------------------------------------------------------

        case STATE_STA_START:
            _doSTA(_currentID);
            _state = STATE_STA_ONGOING;
            break;

        case STATE_STA_ONGOING:
            {
                uint8_t response = _doSTA();
                if (RESPONSE_OK == response) {
                    _state = STATE_STA_SUCCESS;
                } else if (RESPONSE_FAIL == response) {
                    _state = STATE_STA_START;
                    if (_scan) {
                        _currentID = _network_list[_currentID].next;
                        if (_currentID == 0xFF) {
                            _state = STATE_STA_FAILED;
                        }
                    } else {
                        _currentID++;
                        if (_currentID == _network_list.size()) {
                            _state = STATE_STA_FAILED;
                        }
                    }
                }
            }
            break;

        case STATE_STA_FAILED:
            _state = STATE_FALLBACK;
            break;

        case STATE_STA_SUCCESS:
            _state = STATE_IDLE;
            break;

        // ---------------------------------------------------------------------

        #if defined(JUSTWIFI_ENABLE_WPS)

        case STATE_WPS_START:

            _doCallback(MESSAGE_WPS_START);

            _disable();

            if (!WiFi.enableSTA(true)) {
                _state = STATE_WPS_FAILED;
                return;
            }

            WiFi.disconnect();

            if (!wifi_wps_disable()) {
                _state = STATE_WPS_FAILED;
                return;
            }

            // so far only WPS_TYPE_PBC is supported (SDK 1.2.0)
            if (!wifi_wps_enable(WPS_TYPE_PBC)) {
                _state = STATE_WPS_FAILED;
                return;
            }

            _jw_wps_status = (wps_cb_status) 5;
            if (!wifi_set_wps_cb((wps_st_cb_t) &_jw_wps_status_cb)) {
                _state = STATE_WPS_FAILED;
                return;
            }

            if (!wifi_wps_start()) {
                _state = STATE_WPS_FAILED;
                return;
            }

            _state = STATE_WPS_ONGOING;
            break;

        case STATE_WPS_ONGOING:
            if (5 == _jw_wps_status) {
                // Still ongoing
            } else if (WPS_CB_ST_SUCCESS == _jw_wps_status) {
                _state = STATE_WPS_SUCCESS;
            } else {
                _state = STATE_WPS_FAILED;
            }
            break;

        case STATE_WPS_FAILED:
            _doCallback(MESSAGE_WPS_ERROR);
            wifi_wps_disable();
            _state = STATE_FALLBACK;
            break;

        case STATE_WPS_SUCCESS:
            _doCallback(MESSAGE_WPS_SUCCESS);
            wifi_wps_disable();
            addCurrentNetwork(true);
            _state = STATE_IDLE;
            break;

        #endif // defined(JUSTWIFI_ENABLE_WPS)

        // ---------------------------------------------------------------------

        #if defined(JUSTWIFI_ENABLE_SMARTCONFIG)

        case STATE_SMARTCONFIG_START:

            _doCallback(MESSAGE_SMARTCONFIG_START);

            enableAP(false);

            if (!WiFi.beginSmartConfig()) {
                _state = STATE_SMARTCONFIG_FAILED;
                return;
            }

            _state = STATE_SMARTCONFIG_ONGOING;
            _start = millis();

            break;

        case STATE_SMARTCONFIG_ONGOING:
            if (WiFi.smartConfigDone()) {
                _state = STATE_SMARTCONFIG_SUCCESS;
            } else if (millis() - _start > JUSTWIFI_SMARTCONFIG_TIMEOUT) {
                _state = STATE_SMARTCONFIG_FAILED;
            }
            break;

        case STATE_SMARTCONFIG_FAILED:
            _doCallback(MESSAGE_SMARTCONFIG_ERROR);
            WiFi.stopSmartConfig();
            WiFi.enableSTA(false);
            _state = STATE_FALLBACK;
            break;

        case STATE_SMARTCONFIG_SUCCESS:
            _doCallback(MESSAGE_SMARTCONFIG_SUCCESS);
            addCurrentNetwork(true);
            _state = STATE_IDLE;
            break;

        #endif // defined(JUSTWIFI_ENABLE_SMARTCONFIG)

        // ---------------------------------------------------------------------

        case STATE_FALLBACK:
            if (!_ap_connected & _ap_fallback_enabled) _doAP();
            _timeout = millis();
            _state = STATE_IDLE;
            break;

        default:
            _state = STATE_IDLE;
            break;

    }

}

//------------------------------------------------------------------------------
// CONFIGURATION METHODS
//------------------------------------------------------------------------------

void JustWifi::cleanNetworks() {
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
    const char * dns,
    bool front
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
    new_network.scanned = false;

    // Store data
    if (front) {
        _network_list.insert(_network_list.begin(), new_network);
    } else {
        _network_list.push_back(new_network);
    }
    return true;

}

bool JustWifi::addCurrentNetwork(bool front) {
    return addNetwork(
        WiFi.SSID().c_str(),
        WiFi.psk().c_str(),
        NULL, NULL, NULL, NULL,
        front
    );
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
    if (_softap.ssid) free(_softap.ssid);
    _softap.ssid = strdup(ssid);
    if (!_softap.ssid) {
        return false;
    }

    // Copy network PASS
    if (pass && *pass != 0x00) {
        if (_softap.pass) free(_softap.pass);
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

    if ((WiFi.getMode() & WIFI_AP) > 0) {

    // https://github.com/xoseperez/justwifi/issues/4
    if (_softap.pass) {
        WiFi.softAP(_softap.ssid, _softap.pass);
    } else {
        WiFi.softAP(_softap.ssid);
        }

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
    strncpy(_hostname, hostname, sizeof(_hostname));
}

void JustWifi::subscribe(TMessageFunction fn) {
    _callbacks.push_back(fn);
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

bool JustWifi::connectable() {
    return _ap_connected;
}

void JustWifi::disconnect() {
    _timeout = 0;
    WiFi.disconnect();
    WiFi.enableSTA(false);
    _doCallback(MESSAGE_DISCONNECTED);
}

void JustWifi::turnOff() {
    WiFi.disconnect();
    WiFi.enableAP(false);
    WiFi.enableSTA(false);
    WiFi.forceSleepBegin();
    delay(1);
    _doCallback(MESSAGE_TURNING_OFF);
    _sta_enabled = false;
    _state = STATE_IDLE;
}

void JustWifi::turnOn() {
    WiFi.forceSleepWake();
    delay(1);
    setReconnectTimeout(0);
    _doCallback(MESSAGE_TURNING_ON);
    WiFi.enableSTA(true);
    _sta_enabled = true;
    _state = STATE_IDLE;
}

#if defined(JUSTWIFI_ENABLE_WPS)
void JustWifi::startWPS() {
    _state = STATE_WPS_START;
}
#endif // defined(JUSTWIFI_ENABLE_WPS)

#if defined(JUSTWIFI_ENABLE_SMARTCONFIG)
void JustWifi::startSmartConfig() {
    _state = STATE_SMARTCONFIG_START;
}
#endif // defined(JUSTWIFI_ENABLE_SMARTCONFIG)

void JustWifi::enableSTA(bool enabled) {
    _sta_enabled = enabled;
}

void JustWifi::enableAP(bool enabled) {
    if (enabled) {
        _doAP();
    } else {
        WiFi.softAPdisconnect();
        WiFi.enableAP(false);
        _ap_connected = false;
        _doCallback(MESSAGE_ACCESSPOINT_DESTROYED);
    }
}

void JustWifi::enableAPFallback(bool enabled) {
    _ap_fallback_enabled = enabled;
}


void JustWifi::enableScan(bool scan) {
    _scan = scan;
}

void JustWifi::loop() {
    _machine();
}

JustWifi jw;
