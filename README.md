# JustWifi

JustWifi is a WIFI Manager library for the [Arduino Core for ESP8266][2]. The goal of the library is to manage ONLY the WIFI connection (no webserver, no mDNS,...) from code and in a reliable and flexible way.

There are other WIFI managers available. SO how does JustWifi compare to them:

* [WiFiManager][1] is a connection manager with fallback web configuration portal.
* [WiFiMulti][2] is the default multi-network manager that comes with Arduino Core for ESP8266

The table below summarizes the main differences between WiFiManager, WifiMulti and JustWifi:

||WiFiManager|JustWifi|WifiMulti|
|-|-|-|-|
| AP fallback | YES | YES | NO |
| AP+STA mode | NO | YES | NO |
| Configuration portal | YES | NO | NO |
| Configure networks from code | NO | YES | YES
| Custom parameters | YES | NO | NO |
| Persist confguration | YES | NO | NO |
| Allow multiple networks | NO | YES | YES |
| Check the signal strength  | YES* | YES | YES |
| Support static IPs | YES | YES | NO |

* WifiManager filters networks by RSSI, but does not try to connect in order of signal strength


## Features

The main features of the JustWifi library are:

* Configure multiple possible networks
* Scan wifi networks so it can try to connect to only those available, in order of signal strength
* Fallback to AP mode
* Configurable timeout to try to reconnect after AP fallback
* AP+STA mode
* Static IP (autoconnect is disabled when using static IP)
* Single debug/action callback

## Usage

TODO

[1]:https://github.com/tzapu/WiFiManager
[2]:https://github.com/esp8266/Arduino
