/**
 * @file kernels.hpp
 * @brief CUDA kernel declarations for octree operations
 * 
 * Provides GPU-accelerated operations for:
 * - Distance computation between points
 * - Radius-based point filtering
 * - Nearest neighbor search initialization
 */

#pragma once

#include "geometry/point.hpp"
#include <cstddef>
#include <vector>

namespace CUDA {

// =============================================================================
// Kernel Configurations
// =============================================================================

constexpr int THREADS_PER_BLOCK = 256;

// =============================================================================
// Distance Computation Kernels
// =============================================================================

/**
 * Compute Euclidean distances from all points to a target point
 * 
 * @param d_points Device pointer to points array
 * @param target Query point
 * @param d_distances Output device pointer for distances
 * @param numPoints Number of points
 * @return true if successful, false otherwise
 */
bool computeDistancesGPU(
    const Point* d_points,
    const Point& target,
    float* d_distances,
    size_t numPoints
);

/**
 * Compute squared distances (faster, avoids sqrt)
 * 
 * @param d_points Device pointer to points array
 * @param target Query point
 * @param d_distances_sq Output device pointer for squared distances
 * @param numPoints Number of points
 * @return true if successful, false otherwise
 */
bool computeSquaredDistancesGPU(
    const Point* d_points,
    const Point& target,
    float* d_distances_sq,
    size_t numPoints
);

// =============================================================================
// Filtering Kernels
// =============================================================================

/**
 * Filter points within a specified radius from target
 * 
 * @param d_points Device pointer to points array
 * @param target Query point
 * @param radius Search radius
 * @param d_indices Output device pointer for indices of filtered points
 * @param d_count Output device pointer for count of filtered points
 * @param maxResults Maximum results to store
 * @param numPoints Total number of points
 * @return true if successful, false otherwise
 */
bool radiusFilterGPU(
    const Point* d_points,
    const Point& target,
    float radius,
    int* d_indices,
    int* d_count,
    int maxResults,
    size_t numPoints
);

/**
 * Find k-nearest neighbors using distances
 * 
 * @param d_distances Device pointer to precomputed distances
 * @param d_indices Output device pointer for indices of k-nearest
 * @param k Number of neighbors to find
 * @param numPoints Total number of points
 * @return true if successful, false otherwise
 */
bool findKNearestGPU(
    const float* d_distances,
    int* d_indices,
    int k,
    size_t numPoints
);

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * Validate kernel results by comparing with CPU computation
 * 
 * @param h_points Host points (for CPU reference)
 * @param target Query point
 * @param d_distances Device distances from kernel
 * @param tolerance Error tolerance for floating point comparison
 * @return true if results match CPU computation
 */
bool validateDistancesGPU(
    const std::vector<Point>& h_points,
    const Point& target,
    const float* d_distances,
    float tolerance = 1e-5f
);

} // namespace CUDA
