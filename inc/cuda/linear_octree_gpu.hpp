#pragma once

/**
 * @file linear_octree_gpu.hpp
 * @brief GPU memory management for Linear Octree
 * 
 * This file defines the GPU data structures and memory management
 * for the linear octree using Thrust vectors.
 */

#include "geometry/point.hpp"
#include "geometry/box.hpp"
#include <vector>
#include <memory>

// Forward declarations for CUDA-specific types
// These are defined only when compiled with nvcc
namespace CUDA {

/**
 * @class LinearOctreeGPU
 * @brief GPU implementation of Linear Octree data
 * 
 * Stores the octree structure in device memory (GPU VRAM) for efficient
 * parallel queries. Compatible with CPU LinearOctree after data transfer.
 */
class LinearOctreeGPU {
public:
    // Octree topology arrays (device memory pointers)
    struct {
        float* d_points;                    // Device: sorted points
        uint32_t* d_offsets;                // Device: first child index per node
        float* d_centers;                   // Device: node centers
        float* d_radii;                     // Device: node radii by depth
        size_t* d_ranges_start;             // Device: range start per node
        size_t* d_ranges_end;               // Device: range end per node
    } deviceMem;
    
    // Host-side metadata
    struct {
        size_t numPoints = 0;
        size_t numNodes = 0;
        uint32_t maxDepth = 0;
        Box boundingBox;
        bool isTransferred = false;
    } metadata;
    
public:
    /**
     * @brief Constructor
     */
    LinearOctreeGPU();
    
    /**
     * @brief Destructor - frees device memory
     */
    ~LinearOctreeGPU();
    
    /**
     * @brief Transfer octree data from CPU to GPU
     * 
     * @param points Vector of points (host memory)
     * @param offsets Vector of offsets (host memory)
     * @param centers Vector of node centers (host memory)
     * @param radii Vector of node radii (host memory)
     * @param rangesStart Vector of range starts (host memory)
     * @param rangesEnd Vector of range ends (host memory)
     * @param box Bounding box of the octree
     * @param maxDepth Maximum octree depth
     * @return true if transfer successful, false otherwise
     */
    bool transferFromHost(
        const std::vector<Point>& points,
        const std::vector<uint32_t>& offsets,
        const std::vector<Point>& centers,
        const std::vector<Point>& radii,
        const std::vector<size_t>& rangesStart,
        const std::vector<size_t>& rangesEnd,
        const Box& box,
        uint32_t maxDepth
    );
    
    /**
     * @brief Transfer octree data from GPU back to CPU
     * 
     * @param points Output: points (will be resized)
     * @param offsets Output: offsets (will be resized)
     * @return true if transfer successful, false otherwise
     */
    bool transferToCPU(
        std::vector<Point>& points,
        std::vector<uint32_t>& offsets
    ) const;
    
    /**
     * @brief Free all device memory
     */
    void clearDeviceMemory();
    
    /**
     * @brief Get estimated memory usage on GPU
     * @return Memory usage in bytes
     */
    size_t getDeviceMemoryUsage() const;
    
    /**
     * @brief Check if octree has been transferred to GPU
     * @return true if data is on device, false otherwise
     */
    bool isOnDevice() const { return metadata.isTransferred; }
    
    /**
     * @brief Get number of points
     */
    size_t getNumPoints() const { return metadata.numPoints; }
    
    /**
     * @brief Get number of nodes
     */
    size_t getNumNodes() const { return metadata.numNodes; }
    
    /**
     * @brief Validate GPU memory (copy back and compare with CPU)
     * 
     * @param cpuPoints Reference CPU points for validation
     * @param tolerance Allowed error tolerance (for floating point)
     * @return true if validation passes, false otherwise
     */
    bool validateDeviceMemory(
        const std::vector<Point>& cpuPoints,
        float tolerance = 1e-6f
    ) const;
};

/**
 * @class GPUMemoryPool
 * @brief Pre-allocated memory pool for GPU computations
 * 
 * Reduces allocation overhead for repeated searches by pre-allocating
 * buffers for search results and temporary data.
 */
class GPUMemoryPool {
private:
    struct {
        size_t* d_resultIndices = nullptr;
        size_t* d_resultCount = nullptr;
        float* d_distances = nullptr;
        size_t capacity = 0;
    } buffers;
    
public:
    /**
     * @brief Constructor
     * @param initialCapacity Initial buffer size (default: 100,000 results)
     */
    GPUMemoryPool(size_t initialCapacity = 100000);
    
    /**
     * @brief Destructor - frees all buffers
     */
    ~GPUMemoryPool();
    
    /**
     * @brief Allocate or resize GPU buffers
     * @param newCapacity New buffer size
     * @return true if successful, false if allocation failed
     */
    bool allocate(size_t newCapacity);
    
    /**
     * @brief Get result indices buffer
     */
    size_t* getResultIndices() { return buffers.d_resultIndices; }
    
    /**
     * @brief Get result count buffer
     */
    size_t* getResultCount() { return buffers.d_resultCount; }
    
    /**
     * @brief Get distances buffer
     */
    float* getDistances() { return buffers.d_distances; }
    
    /**
     * @brief Get current capacity
     */
    size_t getCapacity() const { return buffers.capacity; }
    
    /**
     * @brief Free all GPU memory
     */
    void clear();
};

/**
 * @namespace GPUUtils
 * @brief GPU utility functions for memory management
 */
namespace GPUUtils {

/**
 * @brief Check CUDA availability and get device info
 * @return true if CUDA-capable device is available
 */
bool isCUDAAvailable();

/**
 * @brief Get GPU memory info
 * @param freeMem Output: free memory in bytes
 * @param totalMem Output: total device memory in bytes
 * @return true if query successful
 */
bool getMemoryInfo(size_t& freeMem, size_t& totalMem);

/**
 * @brief Synchronize GPU computation
 * @return true if successful
 */
bool synchronizeGPU();

/**
 * @brief Copy data from host to device
 * @param hostPtr Host memory pointer
 * @param devicePtr Device memory pointer
 * @param sizeBytes Number of bytes to copy
 * @return true if successful
 */
bool copyHostToDevice(const void* hostPtr, void* devicePtr, size_t sizeBytes);

/**
 * @brief Copy data from device to host
 * @param devicePtr Device memory pointer
 * @param hostPtr Host memory pointer
 * @param sizeBytes Number of bytes to copy
 * @return true if successful
 */
bool copyDeviceToHost(const void* devicePtr, void* hostPtr, size_t sizeBytes);

/**
 * @brief Allocate GPU memory
 * @param ptr Output: device memory pointer
 * @param sizeBytes Size of memory to allocate
 * @return true if successful
 */
bool allocateDeviceMemory(void*& ptr, size_t sizeBytes);

/**
 * @brief Free GPU memory
 * @param ptr Device memory pointer to free
 * @return true if successful
 */
bool freeDeviceMemory(void* ptr);

} // namespace GPUUtils

} // namespace CUDA
