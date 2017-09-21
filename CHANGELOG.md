# JustWifi change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [1.1.5] 2017-09-22
### Fixed
- Fix bug when no hostname defined (#1)
- When multiple routers have the same SSID, choose the one with strongest signal (#2). Thanks to Robert (robi772 @ bitbucket.org)!

### Changed
- Callbacks output more info about networks

## [1.1.4] 2017-08-24
### Fixed
- Fixed code according to Arduino-ESP8266 issue https://github.com/esp8266/Arduino/issues/2186

## [1.1.3] 2017-01-17
### Changed
- Disable autoconnect when using static IP

## [1.1.2] 2017-01-11
### Fixed
- Do not disable STA mode after disconnect

## [1.1.1] 2016-12-29
### Fixed
- Remove declaration from example, jw is a singleton

## [1.1.0] 2016-09-29
- Initial working version as standalone library
