# LFBB_CPP - Lock Free Bipartite Buffer
![CMake](https://github.com/DNedic/lfbb_cpp/actions/workflows/.github/workflows/cmake.yml/badge.svg)

LFBB_CPP is the C++ version of [LFBB](https://github.com/DNedic/lfbb), a bipartite buffer implementation written in standard C++11, suitable for all platforms, from deeply embedded to HPC uses. It is lock-free for single consumer single producer scenarios making it incredibly performant and easy to use.

## What is a bipartite buffer

A bipartite buffer is a variation of the classic [ring buffer](https://en.wikipedia.org/wiki/Circular_buffer) with the ability to always be able to provide the user with contigous memory regions inside the buffer for writing/reading if there is enough space/data.
[Here](https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist) is a nice writeup about the essence of bipartite buffers.

## Why use a bipartite buffer
A bipartite buffer should be used everywhere a ring buffer is used if you want:
* To offload transfers to DMA increasing the transfer speed and freeing up CPU time
* To avoid creating intermediate buffers for APIs that require contigous data
* To process data inside the buffer without dequeing it
* For scenarios where operations on data might fail or only some data might be used
* To use stdlib memcpy which is faster than bytewise implementations used in most queues and ring buffers

## Features
* Written in standard C++11, compatible with all platforms supporting it
* Lock free thread and multicore safe when used in single producer single consumer scenarios
* No dynamic allocation
* MIT Licensed
* Supports CMake [FetchContent()](https://cmake.org/cmake/help/latest/module/FetchContent.html)

## Advantages over the C version
* Type safety, the buffer is type and size templated
* Much simpler and less error-prone instantiation
* Higher performance due to compile-time known size and header-only implementation
* Encapsulation, the data buffer is now a class member

## How to get
There are three main ways to get the library:
* Using CMake [FetchContent()](https://cmake.org/cmake/help/latest/module/FetchContent.html)
* As a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
* By downloading a release from GitHub

## How to use
Shown here is an example of typical use:
* Consumer thread/interrupt
```cpp
auto read = lfbb_adc.ReadAcquire();

if (read.first != nullptr) {
  size_t data_used = DoStuffWithData(read);
  lfbb_adc.ReadRelease(data_used);
}
```

* Producer thread/interrupt
```cpp
if (!write_started) {
  auto *write_location = lfbb_adc.WriteAcquire(data.size());
  if (write_location != nullptr) {
    ADC_StartDma(&adc_dma_h, write_location, sizeof(data));
    write_started = true;
  }
} else {
  if (ADC_PollDmaComplete(&adc_dma_h) {
    lfbb_adc.WriteRelease(data.size());
    write_started = false;
  }
}
```

## Configuration
Some configuration may be needed for space efficiency on embedded systems and for performance on systems with nonstandard cacheline lengths.

The library offers two configuration defines ```LFBB_MULTICORE_HOSTED``` and ```LFBB_CACHELINE_LENGTH``` that can be passed by the build system or defined before including the library.

On embedded systems it is usually required to do manual cache synchronization, so ```LFBB_MULTICORE_HOSTED``` can be set to ```false``` to avoid wasting space on padding for cacheline alignment of indexes.

Some systems have a non-typical cacheline length (for instance the apple M1/M2 CPUs have a cacheline length of 128 bytes), and ```LFBB_CACHELINE_LENGTH``` should be set accordingly in those cases to avoid the false sharing phenomenom and the performance drop that comes from it.

## Dealing with caches on embedded systems
When using the library with DMA or multicore on embedded systems with cache it is necessary to perform manual cache synchronization in one of the following ways:
* Using platform specific data synchronization barriers (```DSB``` on ARM)
* By manually invalidating cache
* By setting the MPU/MMU up to not cache the data buffer

## Caveats
* The library does not implement alignment of writes and reads, it is up to the user to only write in factors they want the data to be aligned to, adequately size and align the buffer used
