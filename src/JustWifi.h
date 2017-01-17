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

#ifndef JustWifi_h
#define JustWifi_h

#include <functional>
#include <vector>
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

#define DEFAULT_CONNECT_TIMEOUT     10000
#define DEFAULT_RECONNECT_INTERVAL  60000

#ifdef DEBUG_ESP_WIFI
#ifdef DEBUG_ESP_PORT
#define DEBUG_WIFI_MULTI(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#endif
#endif

#ifndef DEBUG_WIFI_MULTI
#define DEBUG_WIFI_MULTI(...)
#endif

typedef struct {
    char * ssid;
    char * pass;
    bool dhcp;
    IPAddress ip;
    IPAddress gw;
    IPAddress netmask;
    IPAddress dns;
    int32_t rssi;
    uint8_t security;
    uint8_t channel;
    uint8_t bssid[6];
    uint8_t next;
} network_t;

typedef enum {
    AP_MODE_OFF,
    AP_MODE_ALONE,
    AP_MODE_BOTH
} justwifi_ap_modes_t;

typedef enum {
    STATE_NOT_CONNECTED,
    STATE_SCANNING,
    STATE_CONNECTING,
    STATE_CONNECTED
} justwifi_states_t;

typedef enum {
    MESSAGE_SCANNING,
    MESSAGE_SCAN_FAILED,
    MESSAGE_NO_NETWORKS,
    MESSAGE_FOUND_NETWORK,
    MESSAGE_NO_KNOWN_NETWORKS,
    MESSAGE_CONNECTING,
    MESSAGE_CONNECT_WAITING,
    MESSAGE_CONNECT_FAILED,
    MESSAGE_CONNECTED,
    MESSAGE_ACCESSPOINT_CREATING,
    MESSAGE_ACCESSPOINT_FAILED,
    MESSAGE_ACCESSPOINT_CREATED,
    MESSAGE_DISCONNECTED,
    MESSAGE_HOSTNAME_ERROR
} justwifi_messages_t;

class JustWifi {

    public:

        JustWifi();
        ~JustWifi();

        typedef std::function<void(justwifi_messages_t, char *)> TMessageFunction;

        bool cleanNetworks();
        bool addNetwork(
            const char * ssid,
            const char * pass = NULL,
            const char * ip = NULL,
            const char * gw = NULL,
            const char * netmask = NULL,
            const char * dns = NULL
        );
        bool setSoftAP(
            const char * ssid,
            const char * pass = NULL,
            const char * ip = NULL,
            const char * gw = NULL,
            const char * netmask = NULL
        );

        bool disconnect();
        bool connected();
        bool createAP();
        wl_status_t getStatus();
        String getAPSSID();
        void scanNetworks(bool scan);
        void setAPMode(justwifi_ap_modes_t mode);
        void setHostname(const char * hostname);
        void setConnectTimeout(unsigned long ms);
        void setReconnectTimeout(unsigned long ms = DEFAULT_RECONNECT_INTERVAL);
        void resetReconnectTimeout();
        void onMessage(TMessageFunction fn);
        void loop();

    private:

        std::vector<network_t> _network_list;
        network_t _softap { NULL, NULL };
        TMessageFunction _callback = NULL;
        unsigned long _connect_timeout = DEFAULT_CONNECT_TIMEOUT;
        unsigned long _reconnect_timeout = DEFAULT_RECONNECT_INTERVAL;
        unsigned long _timeout = 0;
        justwifi_ap_modes_t _ap_mode = AP_MODE_ALONE;
        bool _scan = false;
        uint8_t _bestID;

        justwifi_states_t _connect(uint8_t id = 0xFF);
        justwifi_states_t _startSTA(bool reset);
        bool _startAP();
        uint8_t _populate(uint8_t networkCount);
        void _sortByRSSI();
        void _scanNetworks();
        int8_t _scanComplete();
        void _doCallback(justwifi_messages_t message, char * parameter = NULL);

};

extern JustWifi jw;

#endif
