Change log for PC Engines sortbootorder
=======================================

Releases 4.0.x are based on PC Engines 20160304 release.
Releases 4.5.x are based on mainline support submitted in
[this gerrit ref](https://review.coreboot.org/#/c/14138/).

## [Unreleased]
## [v4.0.3] - 2017-01-03
### Added
- UART C and D toggling

## [v4.0.2] - 2016-12-09
### Changed
- versioning scheme to compatible with coreboot releases

### Fixed
- libpayload compilation procedure for legacy firmware

## [v4.5.2] - 2016-11-22

### Added
- support for mainline compilation

## [v4.0.1] - 2016-08-14
initial commit based on [coreboot_140908](http://pcengines.ch/tmp/coreboot_140908.tar.gz)

### Added
- `(disabled)` tag in menu
- `PXE` and `USB` enable options in menu
- `PXE` to bootorder menu
- support for `yangtzee fch spi controller`
- `PC Engines` header
- `README` and `Makefile`

### Changed
- merged all `USB` entries into one
- interface modification
- version bump to v1.2
- version bump to v1.3
- interface to lower case
- letter for save and exit to `S`
- user interface improvements

### Fixed
- used proper way to access extended SPI registers

[Unreleased]: https://github.com/pcengines/sortbootorder/compare/v4.0.3...coreboot-4.0.x
[v4.0.3]: https://github.com/pcengines/sortbootorder/compare/v4.0.2...v4.0.3
[v4.0.2]: https://github.com/pcengines/sortbootorder/compare/v4.0.1...v4.0.2
[v4.5.2]: https://github.com/pcengines/sortbootorder/compare/v4.0.1...v4.5.2
[v4.0.1]: https://github.com/pcengines/sortbootorder/compare/f652a858ff905f17688d841f866c2dedb371fb24...v4.0.1
