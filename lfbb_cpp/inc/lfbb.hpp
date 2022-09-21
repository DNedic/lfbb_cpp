/**************************************************************
 * @file lfbb.hpp
 * @brief A bipartite buffer implementation written in standard
 * c++11 suitable for both low-end microcontrollers all the way
 * to HPC machines. Lock-free for single consumer single
 * producer scenarios.
 * @version	1.0.3
 * @date 21. September 2022
 * @author Djordje Nedic
 **************************************************************/

/**************************************************************
 * Copyright (c) 2022 Djordje Nedic
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LFBB_CPP - Lock Free Bipartite Buffer
 *
 * Author:          Djordje Nedic <nedic.djordje2@gmail.com>
 * Version:         v1.0.3
 **************************************************************/

/************************** INCLUDE ***************************/
#ifndef LFBB_HPP
#define LFBB_HPP

#include <atomic>
#include <cstdlib>

/************************** DEFINE ****************************/

#ifndef LFBB_MULTICORE_HOSTED
#define LFBB_MULTICORE_HOSTED false
#endif

#ifndef LFBB_CACHELINE_LENGTH
#define LFBB_CACHELINE_LENGTH 64U
#endif

/*************************** TYPES ****************************/

template <typename T, size_t size>
class LfBb {
  /********************** PUBLIC METHODS ************************/
 public:
  LfBb();

  /**
   * @brief Acquires a linear region in the bipartite buffer for writing
   * @param[in] Free linear space in the buffer required
   * @retval Pointer to the beginning of the linear space
   */
  T *WriteAcquire(const size_t free_required);

  /**
   * @brief Releases the bipartite buffer after a write
   * @param[in] Elements written to the linear space
   * @retval None
   */
  void WriteRelease(const size_t written);

  /**
   * @brief Acquires a linear region in the bipartite buffer for reading
   * @retval Pair containing the pointer to the beginning of the area and
   * elements available
   */
  std::pair<T *, size_t> ReadAcquire();

  /**
   * @brief Releases the bipartite buffer after a read
   * @param[in] Elements read from the linear region
   * @retval None
   */
  void ReadRelease(const size_t read);

  /********************* PRIVATE METHODS ************************/
 private:
  size_t GetFree(const size_t w, const size_t r) const;

  /********************** PRIVATE MEMBERS ***********************/
  T _data[size]; /**< Data array */
#ifdef LFBB_MULTICORE_HOSTED
  alignas(LFBB_CACHELINE_LENGTH) std::atomic_size_t _r; /**< Read index */
  alignas(LFBB_CACHELINE_LENGTH) std::atomic_size_t _w; /**< Write index */
  alignas(LFBB_CACHELINE_LENGTH)
      std::atomic_size_t _i; /**< Invalidated space index */
#else
  std::atomic_size_t _r; /**< Read index */
  std::atomic_size_t _w; /**< Write index */
  std::atomic_size_t _i; /**< Invalidated space index */
#endif
  bool _write_wrapped; /**< Write wrapped flag, used only in the producer */
  bool _read_wrapped;  /**< Read wrapped flag, used only in the consumer */
};

/************************** INCLUDE ***************************/

/* Include the implementation */
#include "lfbb_impl.hpp"

#endif /* LFBB_HPP */
