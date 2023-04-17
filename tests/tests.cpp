#include <algorithm>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include "lfbb.hpp"

#define countof(arr) std::extent<typeof(arr), 0>::value // cannot replace with an inline function

TEST_CASE("Write to the beginning", "[write_beginning]") {
  LfBb<uint8_t, 512U> lfbb;
  const uint8_t test_data[320] = {0xE5U};

  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  REQUIRE(write_location != nullptr);

  std::copy(std::begin(test_data), std::end(test_data), write_location);

  lfbb.WriteRelease(countof(test_data));

  auto read = lfbb.ReadAcquire();

  REQUIRE(read.first != nullptr);
  REQUIRE(read.second == countof(test_data));
  REQUIRE(std::equal(std::begin(test_data), std::end(test_data), read.first));
}

TEST_CASE("Try to acquire too much data", "[acquire_too_much]") {
  LfBb<uint8_t, 512U> lfbb;
  auto *write_location = lfbb.WriteAcquire(512U + rand());
  REQUIRE(write_location == nullptr);
}

TEST_CASE("Try to acquire read with an empty buffer", "[read_empty]") {
  LfBb<uint8_t, 512U> lfbb;

  auto read = lfbb.ReadAcquire();
  REQUIRE(read.first == nullptr);
  REQUIRE(read.second == 0);
}

TEST_CASE("Write with overflow condition", "[write_overflow]") {
  LfBb<uint32_t, 512U> lfbb;
  const uint32_t test_data[320] = {0xE5A1D2C3U};

  /* Write to the start and read the data back */
  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  std::copy(std::begin(test_data), std::end(test_data), write_location);
  lfbb.WriteRelease(countof(test_data));

  auto read = lfbb.ReadAcquire();
  lfbb.ReadRelease(countof(test_data));

  /* Write again, this time the oveflow will trigger and should give us the
   * beginning again */
  const uint32_t test_data2[240] = {0xA3B2C1D0U};
  write_location =
      lfbb.WriteAcquire(countof(test_data2));
  REQUIRE(write_location != nullptr);
  std::copy(std::begin(test_data2), std::end(test_data2), write_location);
  lfbb.WriteRelease(countof(test_data2));

  read = lfbb.ReadAcquire();
  REQUIRE(read.first != nullptr);
  REQUIRE(read.second == countof(test_data2));
  REQUIRE(std::equal(std::begin(test_data2), std::end(test_data2), read.first));
}

TEST_CASE("Read data written after overflow condition write",
          "[read_after_overflow_write]") {
  LfBb<int16_t, 512U> lfbb;
  const int16_t test_data[320] = {-222};

  /* Write to the start and read the data back */
  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  std::copy(std::begin(test_data), std::end(test_data), write_location);
  lfbb.WriteRelease(countof(test_data));

  auto read = lfbb.ReadAcquire();
  lfbb.ReadRelease(countof(test_data));

  /* Write again, this time the oveflow will trigger and should give us the
   * beginning again */
  const int16_t test_data2[240] = {-66};
  write_location =
      lfbb.WriteAcquire(countof(test_data2));
  std::copy(std::begin(test_data2), std::end(test_data2), write_location);
  lfbb.WriteRelease(countof(test_data2));

  read = lfbb.ReadAcquire();
  lfbb.ReadRelease(countof(test_data2));

  /* Write again, without overflow and read back */
  const uint8_t test_data3[120] = {0xBCU};
  write_location =
      lfbb.WriteAcquire(countof(test_data3));
  std::copy(std::begin(test_data3), std::end(test_data3), write_location);
  lfbb.WriteRelease(countof(test_data3));

  read = lfbb.ReadAcquire();
  REQUIRE(read.first != nullptr);
  REQUIRE(read.second == countof(test_data3));
  REQUIRE(std::equal(std::begin(test_data3), std::end(test_data3), read.first));
}

TEST_CASE("Interleaved write and read with enough space",
          "[interleaved_success]") {
  LfBb<double, 512U> lfbb;
  const double test_data[320] = {42.4242};

  /* 1. Complete write */
  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  std::copy(std::begin(test_data), std::end(test_data), write_location);
  lfbb.WriteRelease(countof(test_data));

  /* 2. Read acquire, the linear space previously written is reserved for
   * reading now */
  auto read = lfbb.ReadAcquire();

  /* 3. Write acquire, a linear space after the read linear space is reserved
   * for writing and is copied to*/
  const double test_data2[120] = {-123.123};
  write_location =
      lfbb.WriteAcquire(countof(test_data2));
  REQUIRE(write_location != nullptr);
  std::copy(std::begin(test_data2), std::end(test_data2), write_location);

  /* 4. Compare the data */
  REQUIRE(std::equal(std::begin(test_data), std::end(test_data), read.first));
}

TEST_CASE("Interleaved write and read with enough space 2",
          "[interleaved_success2]") {
  LfBb<char, 512U> lfbb;
  const char test_data[320] = {'A'};

  /* 1. Complete write */
  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  std::copy(std::begin(test_data), std::end(test_data), write_location);
  lfbb.WriteRelease(countof(test_data));

  /* 2. Write acquire, a linear space after the read linear space is reserved
   * for writing and is copied to*/
  const char test_data2[120] = {'b'};
  write_location = lfbb.WriteAcquire(countof(test_data2));
  std::copy(std::begin(test_data2), std::end(test_data2), write_location);

  /* 3. Read acquire, the linear space previously written is reserved for
   * reading now */
  auto read = lfbb.ReadAcquire();
  REQUIRE(read.first != nullptr);
  REQUIRE(std::equal(std::begin(test_data), std::end(test_data), read.first));
}

TEST_CASE("Interleaved write and read without enough space",
          "[interleaved_fail]") {
  LfBb<uint8_t, 512U> lfbb;
  const uint8_t test_data[320] = {0xE5U};

  /* 1. Complete write */
  auto *write_location =
      lfbb.WriteAcquire(countof(test_data));
  std::copy(std::begin(test_data), std::end(test_data), write_location);
  lfbb.WriteRelease(countof(test_data));

  /* 2. Read acquire, the linear space previously written is reserved for
   * reading now */
  auto read = lfbb.ReadAcquire();

  /* 3. Write acquire, attempt to acquire more linear space than available */
  const uint8_t test_data2[240] = {0xA3U};
  write_location =
      lfbb.WriteAcquire(countof(test_data2));
  REQUIRE(write_location == nullptr);
}

TEST_CASE("Test keeping the chunk of data when write ends exactly in the end of the buffer.",
          "[exact_end_write_release_proper_invalidation_test]") {
  constexpr size_t size = 8;
  LfBb<uint8_t, size * 2> lfbb;

  // First half, no overflow.
  auto base = lfbb.WriteAcquire(size);
  lfbb.WriteRelease(size);
  lfbb.ReadAcquire();
  lfbb.ReadRelease(size);

  // Second half, write overflow.
  auto write_buf_second_half = lfbb.WriteAcquire(size);
  lfbb.WriteRelease(size);

  // Second half, read overflow.
  auto read_buf_second_half = lfbb.ReadAcquire().first;
  REQUIRE((read_buf_second_half - base) == (write_buf_second_half - base));
}
