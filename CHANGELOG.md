# Changelog

## 1.0.0

- Initial release

## 1.0.1

- Refactored for performance improvements and code quality
- Improved documentation in regards to configuration

## 1.0.2

- Improved performance by using a flag instead of infering write wraps

## 1.0.3

- Fix bug where _write_wrapped was not initialized

## 1.1.0

- Set ```LFBB_MULTICORE_HOSTED``` to ```false``` by default. This is a more embedded-friendly configuration which doesn't impact hosted platforms performance significantly if the library is left unconfigured but avoids wasting space on embedded devices. When using the library on hosted platforms passing ```LFBB_MULTICORE_HOSTED``` as ```true``` is now advised for maximum performance.

## 1.1.1

- A critical bug where valid data would be invalidated if the write ended exactly on the end of the buffer was fixed - [@KOLANICH](https://github.com/KOLANICH)

## 1.2.0

- A `std::span` based API was added in addition to the regular one that can be used with C++20 and up
