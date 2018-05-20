# JustWifi

JustWifi is a WIFI Manager library for the [Arduino Core for ESP8266][2]. The goal of the library is to manage ONLY the WIFI connection (no webserver, no mDNS,...) from code and in a reliable and flexible way.

[![version](https://img.shields.io/badge/version-1.2.0-brightgreen.svg)](CHANGELOG.md)
[![travis](https://travis-ci.org/xoseperez/justwifi.svg?branch=master)](https://travis-ci.org/xoseperez/justwifi)
[![codacy](https://img.shields.io/codacy/grade/4ccbea0317c4415eb2d1c562feced407/master.svg)](https://www.codacy.com/app/xoseperez/justwifi/dashboard)
[![license](https://img.shields.io/github/license/xoseperez/justwifi.svg)](LICENSE)
<br />
[![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xose%2eperez%40gmail%2ecom&lc=US&no_note=0&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHostedGuest)
[![twitter](https://img.shields.io/twitter/follow/xoseperez.svg?style=social)](https://twitter.com/intent/follow?screen_name=xoseperez)

There are other WIFI managers available. So how does JustWifi compare to them:

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

See examples.

[1]:https://github.com/tzapu/WiFiManager
[2]:https://github.com/esp8266/Arduino
