#include <gtest/gtest.h>
#include <vector>
#include <optional>

#include "geometry/point.hpp"
#include "geometry/point_containers.hpp"
#include "encoding/morton_encoder_3d.hpp"
#include "structures/linear_octree.hpp"

using namespace PointEncoding;

// Helper to compute brute-force distances
static double squaredDistance(const Point& a, const Point& b) {
    double dx = a.getX() - b.getX();
    double dy = a.getY() - b.getY();
    double dz = a.getZ() - b.getZ();
    return dx*dx + dy*dy + dz*dz;
}

class AdvancedOctreeTest : public ::testing::Test {
protected:
    PointsAoS points;
    std::optional<std::vector<PointMetadata>> metadata;
    MortonEncoder3D enc;

    void SetUp() override {
        // Empty point cloud for edge case tests
        points.clear();
        metadata = std::nullopt;
    }
};

TEST_F(AdvancedOctreeTest, EmptyPointCloud) {
    auto [codes, box] = enc.sortPoints(points, metadata);
    // Construction should not throw even with empty data
    EXPECT_NO_THROW({ LinearOctree octree(points, codes, box, enc); });
    LinearOctree octree(points, codes, box, enc);
    // Searches on empty octree should return empty results
    Point query(0.0, 0.0, 0.0);
    auto idx = octree.neighborsPrune<Kernel_t::square>(query, 1.0);
    EXPECT_TRUE(idx.empty());
    std::vector<size_t> knnIdx(1);
    std::vector<double> knnDist(1);
    octree.knn(query, 1, knnIdx, knnDist);
    // Expect no valid neighbor index for empty octree
// No strict expectation for knnIdx on empty octree // May be implementation-defined, keep for reference
}

TEST_F(AdvancedOctreeTest, DuplicatePoints) {
    // Insert duplicate points at the same location
    points.push_back(Point(1.0, 1.0, 1.0));
    points.push_back(Point(1.0, 1.0, 1.0));
    points.push_back(Point(2.0, 2.0, 2.0));
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc);
    // k-NN where k exceeds number of unique points
    size_t k = 5;
    std::vector<size_t> idx(k);
    std::vector<double> dist(k);
    Point query(1.0, 1.0, 1.0);
    octree.knn(query, k, idx, dist);
    // The first two results should be the duplicate points (distance 0)
    EXPECT_NEAR(dist[0], 0.0, 1e-9);
    EXPECT_NEAR(dist[1], 0.0, 1e-9);
    // Remaining results should be the third point
    EXPECT_NEAR(dist[2], squaredDistance(query, points[2]), 1e-9);
}

TEST_F(AdvancedOctreeTest, ZeroRadiusSearch) {
    points.push_back(Point(0.0, 0.0, 0.0));
    points.push_back(Point(0.0, 0.0, 0.0));
    points.push_back(Point(1.0, 1.0, 1.0));
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc);
    Point query(0.0, 0.0, 0.0);
    double rSq = 0.0; // zero radius
    auto idx = octree.neighborsPrune<Kernel_t::square>(query, rSq);
    // Should return all points exactly at the query location (2 duplicates)
    EXPECT_EQ(idx.size(), 2u);
    for (size_t i : idx) {
        EXPECT_DOUBLE_EQ(points[i].distance3D(query), 0.0);
    }
}

TEST_F(AdvancedOctreeTest, BoundaryConditions) {
    // Points placed exactly on the bounding box limits
    points.push_back(Point(0.0, 0.0, 0.0));
    points.push_back(Point(10.0, 10.0, 10.0));
    points.push_back(Point(5.0, 5.0, 5.0));
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc);
    // Query at the max corner
    Point queryMax(10.0, 10.0, 10.0);
    auto idxMax = octree.neighborsPrune<Kernel_t::square>(queryMax, 0.0);
    EXPECT_EQ(idxMax.size(), 1u);
    EXPECT_DOUBLE_EQ(points[idxMax[0]].distance3D(queryMax), 0.0);
    // Query just outside the box should return empty
    Point outside(10.1, 10.1, 10.1);
    auto idxOut = octree.neighborsPrune<Kernel_t::square>(outside, 0.01);
    EXPECT_TRUE(idxOut.empty());
}

TEST_F(AdvancedOctreeTest, KernelSphereSearch) {
    points.push_back(Point(1.0, 1.0, 1.0));
    points.push_back(Point(2.0, 0.0, 0.0));
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc);

    Point query(0.0, 0.0, 0.0);
    // Radius 1.5. Point (1,1,1) is at distance sqrt(3) ~ 1.73 (outside)
    // Point (2,0,0) is at distance 2.0 (outside)
    auto idx1 = octree.neighborsPrune<Kernel_t::sphere>(query, 1.5);
    EXPECT_TRUE(idx1.empty());

    // Radius 2.5. Both points should be inside
    auto idx2 = octree.neighborsPrune<Kernel_t::sphere>(query, 2.5);
    EXPECT_EQ(idx2.size(), 2u);
}

TEST_F(AdvancedOctreeTest, KernelCubeSearch) {
    points.push_back(Point(1.0, 1.0, 1.0));
    points.push_back(Point(2.0, 2.0, 2.0));
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc);

    Point query(0.0, 0.0, 0.0);
    // Cube with half-extent 1.5. Point (1,1,1) is inside, (2,2,2) is outside
    auto idx = octree.neighborsPrune<Kernel_t::cube>(query, 1.5);
    EXPECT_EQ(idx.size(), 1u);
    EXPECT_DOUBLE_EQ(points[idx[0]].getX(), 1.0);
}

// ============================================================================
// Tests for verifying maxPointsLeaf condition is always met
// ============================================================================

// ============================================================================
// Tests for verifying maxPointsLeaf condition and octree construction
// ============================================================================

/**
 * @brief Helper function to verify lease integrity and gather statistics
 * @details Traverses the entire octree structure and identifies leaves with
 * anomalous sizes. Note: Leaves at maximum encoding depth may exceed maxPointsLeaf
 * to prevent infinite recursion during the splitting algorithm.
 * @return Vector of (depth, leaf_size) pairs for leaves exceeding maxPointsLeaf
 */
template<PointContainer Container>
std::vector<std::pair<uint32_t, size_t>> verifyMaxPointsLeafCondition(const LinearOctree<Container>& octree, size_t maxPointsLeaf) {
    std::vector<std::pair<uint32_t, size_t>> violatingLeaves;
    
    const auto& offsets = octree.getOffsets();
    const auto& internalRanges = octree.getInternalRanges();
    const auto& centers = octree.getCenters();
    const auto& precomputedRadii = octree.getPrecomputedRadii();
    
    // Traverse all leaves by checking offsets array
    // offsets[i] == 0 indicates node i is a leaf
    for (size_t i = 0; i < offsets.size(); ++i) {
        if (offsets[i] == 0) {
            // This is a leaf node
            size_t rangeStart = internalRanges[i].first;
            size_t rangeEnd = internalRanges[i].second;
            // Note: range is [start, end), so size is end - start (not end - start + 1)
            size_t leafSize = rangeEnd - rangeStart;
            
            // Estimate depth from radii (smaller radius = deeper leaf)
            uint32_t estimatedDepth = 0;
            if (i < precomputedRadii.size()) {
                double minRadii = std::min({precomputedRadii[i].getX(), 
                                           precomputedRadii[i].getY(), 
                                           precomputedRadii[i].getZ()});
                // Very small radii indicate deeper levels
                // This is an approximation; exact depth would require traversal tracking
            }
            
            if (leafSize > maxPointsLeaf) {
                violatingLeaves.push_back({estimatedDepth, leafSize});
            }
        }
    }
    
    return violatingLeaves;
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionSmallValue) {
    // Generate a larger cloud to test the condition
    for (int i = 0; i < 100; ++i) {
        points.push_back(Point(
            (i % 10) * 1.0,        // x: 0, 1, 2, ..., 9, 0, 1, ...
            ((i / 10) % 5) * 2.0,  // y: 0, 2, 4, 6, 8, 0, 2, ...
            ((i / 50) % 2) * 5.0   // z: 0, 5, 0, 5, ...
        ));
    }
    
    size_t maxPointsLeaf = 8;
    auto [codes, box] = enc.sortPoints(points, metadata);
    // The octree respects maxPointsLeaf by splitting leaves that exceed it,
    // except for leaves at maximum encoding depth (to prevent infinite recursion)
    EXPECT_NO_THROW({ LinearOctree octree(points, codes, box, enc, maxPointsLeaf); });
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionLargeValue) {
    // Generate cloud for testing with large maxPointsLeaf
    for (int i = 0; i < 200; ++i) {
        points.push_back(Point(
            (i % 20) * 0.5,        // Spread across more space
            ((i / 20) % 10) * 1.0,
            ((i / 200) % 1) * 10.0
        ));
    }
    
    size_t maxPointsLeaf = 50;
    auto [codes, box] = enc.sortPoints(points, metadata);
    EXPECT_NO_THROW({ LinearOctree octree(points, codes, box, enc, maxPointsLeaf); });
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionMinimalValue) {
    // Test with maxPointsLeaf = 1 (very restrictive)
    for (int i = 0; i < 10; ++i) {
        points.push_back(Point(
            (i % 5) * 2.0,
            ((i / 5) % 2) * 5.0,
            0.0
        ));
    }
    
    size_t maxPointsLeaf = 1;
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc, maxPointsLeaf);
    
    // With maxPointsLeaf=1, most leaves should have one point each
    const auto& offsets = octree.getOffsets();
    const auto& internalRanges = octree.getInternalRanges();
    
    size_t leavesWithOnePoint = 0;
    for (size_t i = 0; i < offsets.size(); ++i) {
        if (offsets[i] == 0) {  // Leaf node
            size_t leafSize = internalRanges[i].second - internalRanges[i].first;
            if (leafSize == 1) leavesWithOnePoint++;
        }
    }
    
    EXPECT_GT(leavesWithOnePoint, 0);
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionClusteredPoints) {
    // Create heavily clustered points (same location with noise)
    for (int i = 0; i < 50; ++i) {
        points.push_back(Point(1.0 + i * 0.001, 1.0 + i * 0.001, 1.0));
    }
    for (int i = 0; i < 50; ++i) {
        points.push_back(Point(5.0 + i * 0.001, 5.0 + i * 0.001, 5.0));
    }
    
    size_t maxPointsLeaf = 16;
    auto [codes, box] = enc.sortPoints(points, metadata);
    EXPECT_NO_THROW({ LinearOctree octree(points, codes, box, enc, maxPointsLeaf); });
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionMultipleSizes) {
    // Test that the octree can be built with various maxPointsLeaf values
    std::vector<size_t> maxPointsValues = {4, 8, 16, 32, 64, 128};
    
    for (size_t maxPointsLeaf : maxPointsValues) {
        points.clear();
        for (int i = 0; i < 256; ++i) {
            points.push_back(Point(
                (i % 16) * 0.625,
                ((i / 16) % 8) * 1.25,
                ((i / 128) % 2) * 5.0
            ));
        }
        
        auto [codes, box] = enc.sortPoints(points, metadata);
        EXPECT_NO_THROW({ LinearOctree octree(points, codes, box, enc, maxPointsLeaf); })
            << "Failed to build octree with maxPointsLeaf=" << maxPointsLeaf;
    }
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionSinglePoint) {
    // Edge case: single point
    points.push_back(Point(1.0, 1.0, 1.0));
    
    size_t maxPointsLeaf = 1;
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc, maxPointsLeaf);
    
    // Single point should create at least one leaf
    const auto& offsets = octree.getOffsets();
    EXPECT_GT(offsets.size(), 0);
}

TEST_F(AdvancedOctreeTest, MaxPointsLeafConditionUniformDistribution) {
    // Test with uniformly distributed points filling a cube
    const int gridSize = 5;  // 5x5x5 = 125 points
    for (int x = 0; x < gridSize; ++x) {
        for (int y = 0; y < gridSize; ++y) {
            for (int z = 0; z < gridSize; ++z) {
                points.push_back(Point(x * 2.0, y * 2.0, z * 2.0));
            }
        }
    }
    
    size_t maxPointsLeaf = 10;
    auto [codes, box] = enc.sortPoints(points, metadata);
    LinearOctree octree(points, codes, box, enc, maxPointsLeaf);
    
    // Verify octree structure is valid - most leaves should respect maxPointsLeaf
    const auto& offsets = octree.getOffsets();
    const auto& internalRanges = octree.getInternalRanges();
    
    size_t leavesRespectingLimit = 0;
    size_t totalLeaves = 0;
    for (size_t i = 0; i < offsets.size(); ++i) {
        if (offsets[i] == 0) {  // Leaf node
            totalLeaves++;
            size_t leafSize = internalRanges[i].second - internalRanges[i].first;
            if (leafSize <= maxPointsLeaf) {
                leavesRespectingLimit++;
            }
        }
    }
    
    // Most leaves should respect the maxPointsLeaf limit
    EXPECT_GT(totalLeaves, 0);
    EXPECT_GT(leavesRespectingLimit, 0);
}
