# JustWifi change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [2.0.2] 2018-09-13
### Fixed
- Check NO_EXTRA_4K_HEAP flag for WPS support on SDK 2.4.2

## [2.0.1] 2018-06-30
### Fixed
- Memory leak in setSoftAP

## [2.0.0] 2018-06-11
### Added
- SmartConfig (a.k.a. ESP Touch) support
- WPS Support
- TravisCI build tests
- Codacy code test

### Changed
- Complete refactor (check examples to adapt your code)
- Changed license to LGPL-3.0

## [1.2.0] 2018-05-20
### Changed
- Moved to GitHub

## [1.1.9] 2018-05-08
### Fixed
- Removed unneeded reference to DNSServer.h

## [1.1.8] 2018-04-23
### Changed
- Added options for power saving (Thanks to Pablo Pousada Rial)
- Added Travis CI configuration (Thanks to @lobradov)

## [1.1.7] 2018-04-10
### Changed
- Increasing hostname size to 32 chars

## [1.1.6] 2018-01-10
### Changed
- Allow to register several callbacks
- Changes in the information parameters for scanning and connecting callbacks
- Updated example

### Deprecated
- onMessage method deprecated in favour of subscribe method

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
