#pragma once

#include <cstddef>

#include "encoding/encoding_types.hpp"
#include "structures/structure_types.hpp"

/// @brief Pure data struct with build metrics. Presentation (toCSV, operator<<)
/// lives in the benchmark suite.
struct BuildLog {
  // Data cloud size
  size_t cloudSize = 0;

  // The encoder used
  EncoderType encoding{};

  // Octree general parameters
  size_t maxLeafPoints = 0;
  SearchStructure structure{};

  // Build step times (vary between LinearOctree and Octree)
  double linearOctreeLeafTime = 0.0;
  double linearOctreeInternalTime = 0.0;
  double buildTime = 0.0;

  // Mem used in bytes
  size_t memoryUsed = 0;

  // Amount of nodes
  size_t totalNodes = 0;
  size_t leafNodes = 0;
  size_t internalNodes = 0;

  // Max depth and min radius at max depth
  size_t maxDepthSeen = 0;
  double minRadiusAtMaxDepth = 0.0;

  // Cache miss counts (if using hardware profiling)
  long long l1dMisses = 0;
  long long l2dMisses = 0;
  long long l3Misses = 0;
};
