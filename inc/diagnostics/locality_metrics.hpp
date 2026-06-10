#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

#include <omp.h>

#include "geometry/point_containers.hpp"
#include "structures/linear_octree.hpp"

/// @brief Pure data struct with G1 locality diagnostic results. Presentation
/// (toCSV, operator<<) lives in the benchmark suite.
struct LocalityMetrics {
  size_t k = 0;
  double sampleRate = 1.0;
  size_t nSamples = 0;

  double g1 = 0.0;
  double q1 = 0.0;
  double q2 = 0.0;
  double q3 = 0.0;
  double meanIndexDist = 0.0;
  double stdIndexDist = 0.0;

  double elapsedSeconds = 0.0;
};

namespace LocalityDiagnostics {

namespace detail {

/// @brief Fisher-Pearson skewness coefficient (G1) of an index-distance
/// histogram, given its mean and total count.
inline double computeG1(const std::unordered_map<size_t, size_t> &hist,
                         double mean, double total) {
  if (total <= 0) return 0.0;
  double s2 = 0.0, s3 = 0.0;
  for (auto &[d, c] : hist) {
    double x = static_cast<double>(d) - mean;
    s2 += x * x * c;
    s3 += x * x * x * c;
  }
  double sd = std::sqrt(s2 / total);
  return sd < 1e-10 ? 0.0 : (s3 / total) / (sd * sd * sd);
}

/// @brief Quantile of a histogram sorted by key (distance).
inline double
computeQuantile(const std::vector<std::pair<size_t, size_t>> &sortedHist,
                double total, double p) {
  double target = total * p, cum = 0;
  for (auto &[d, c] : sortedHist) {
    cum += c;
    if (cum >= target) return static_cast<double>(d);
  }
  return sortedHist.empty() ? 0.0
                             : static_cast<double>(sortedHist.back().first);
}

} // namespace detail

/**
 * @brief Computes the G1 (Fisher-Pearson skewness) locality diagnostic for an
 * already built and reordered LinearOctree.
 *
 * @details For a sample of points, runs a k-NN search through @p octree and
 * builds a histogram of the array-index distance between each sampled point
 * and each of its neighbours. The skewness (G1) of this distribution measures
 * how well the points' final memory layout (set by the SFC encoding used to
 * build the octree) preserves spatial locality: a low G1 and a low mean index
 * distance indicate that spatially-close points also tend to be stored close
 * together in memory.
 *
 * @param octree A LinearOctree already built on @p points.
 * @param points The (already-reordered) point cloud used to build @p octree.
 * Must be the same container/order as passed to the LinearOctree constructor.
 * @param k Number of neighbours to search for each sampled point.
 * @param sampleRate Fraction (0,1] of points to sample. 1.0 samples every
 * point.
 * @param seed RNG seed used to pick the sample when sampleRate < 1.0.
 */
template <PointContainer Container>
LocalityMetrics computeLocalityG1(const LinearOctree<Container> &octree,
                                   const Container &points, size_t k,
                                   double sampleRate = 1.0,
                                   uint64_t seed = 42) {
  LocalityMetrics metrics;
  metrics.k = k;
  metrics.sampleRate = sampleRate;

  size_t N = points.size();
  if (N == 0 || k == 0) return metrics;

  std::vector<size_t> sampleIndices(N);
  std::iota(sampleIndices.begin(), sampleIndices.end(), 0);
  if (sampleRate < 1.0) {
    size_t nSamples = std::max<size_t>(1, static_cast<size_t>(N * sampleRate));
    std::mt19937_64 rng(seed);
    std::shuffle(sampleIndices.begin(), sampleIndices.end(), rng);
    sampleIndices.resize(nSamples);
    std::sort(sampleIndices.begin(), sampleIndices.end());
  }
  metrics.nSamples = sampleIndices.size();

  auto t0 = std::chrono::steady_clock::now();

  std::unordered_map<size_t, size_t> histogram;
#pragma omp parallel
  {
    std::unordered_map<size_t, size_t> localHist;
    std::vector<size_t> idx(k);
    std::vector<double> dists(k);

#pragma omp for schedule(dynamic, 256)
    for (size_t s = 0; s < sampleIndices.size(); ++s) {
      size_t i = sampleIndices[s];
      size_t found = octree.knn(points[i], k, idx, dists);
      for (size_t j = 0; j < found; ++j) {
        size_t diff = i > idx[j] ? i - idx[j] : idx[j] - i;
        localHist[diff]++;
      }
    }

#pragma omp critical
    for (auto &[d, c] : localHist)
      histogram[d] += c;
  }

  metrics.elapsedSeconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - t0)
          .count();

  double total = 0, s1 = 0;
  for (auto &[d, c] : histogram) {
    total += c;
    s1 += static_cast<double>(d) * c;
  }
  if (total <= 0) return metrics;

  double mean = s1 / total;
  double s2 = 0;
  for (auto &[d, c] : histogram) {
    double x = static_cast<double>(d) - mean;
    s2 += x * x * c;
  }

  metrics.meanIndexDist = mean;
  metrics.stdIndexDist = std::sqrt(s2 / total);
  metrics.g1 = detail::computeG1(histogram, mean, total);

  std::vector<std::pair<size_t, size_t>> sortedHist(histogram.begin(),
                                                      histogram.end());
  std::sort(sortedHist.begin(), sortedHist.end());
  metrics.q1 = detail::computeQuantile(sortedHist, total, 0.25);
  metrics.q2 = detail::computeQuantile(sortedHist, total, 0.50);
  metrics.q3 = detail::computeQuantile(sortedHist, total, 0.75);

  return metrics;
}

} // namespace LocalityDiagnostics
