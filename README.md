# JustWifi

JustWifi is a WIFI Manager library for the [Arduino Core for ESP8266][2]. The goal of the library is to manage ONLY the WIFI connection (no webserver, no mDNS,...) from code and in a reliable and flexible way.

[![version](https://img.shields.io/badge/version-2.0.2-brightgreen.svg)](CHANGELOG.md)
[![travis](https://travis-ci.org/xoseperez/justwifi.svg?branch=dev)](https://travis-ci.org/xoseperez/justwifi)
[![codacy](https://img.shields.io/codacy/grade/4ccbea0317c4415eb2d1c562feced407/dev.svg)](https://www.codacy.com/app/xoseperez/justwifi/dashboard)
[![license](https://img.shields.io/github/license/xoseperez/justwifi.svg)](LICENSE)
<br />
[![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xose%2eperez%40gmail%2ecom&lc=US&no_note=0&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHostedGuest)
[![twitter](https://img.shields.io/twitter/follow/xoseperez.svg?style=social)](https://twitter.com/intent/follow?screen_name=xoseperez)

## Features

The main features of the JustWifi library are:

* Configure multiple possible networks
* Scan wifi networks so it can try to connect to only those available, in order of signal strength
* Smart Config support (when built with -DJUSTWIFI_ENABLE_SMARTCONFIG, tested with ESP8266 SmartConfig or IoT SmartConfig apps)
* WPS support (when built with -DJUSTWIFI_ENABLE_WPS)
* Fallback to AP mode
* Configurable timeout to try to reconnect after AP fallback
* AP+STA mode
* Static IP (autoconnect is disabled when using static IP)
* Single debug/action callback

## Usage

See examples.

## License

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
