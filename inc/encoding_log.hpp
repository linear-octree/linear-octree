#pragma once

#include <cstddef>

#include "encoding/encoding_types.hpp"

/// @brief Pure data struct with encoding metrics. Presentation (toCSV,
/// operator<<) lives in the benchmark suite.
struct EncodingLog {
  // Data cloud size
  size_t cloudSize = 0;

  // The encoder being used
  EncoderType encoding{};

  // Encoding and sorting times
  double boundingBoxTime = 0.0;
  double encodingTime = 0.0;
  double sortingTime = 0.0;
};
