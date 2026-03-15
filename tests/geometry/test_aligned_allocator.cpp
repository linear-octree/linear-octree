#include <gtest/gtest.h>
#include <geometry/point_containers.hpp>
#include <cstring>

// Test aligned_allocator functionality
class AlignedAllocatorTest : public ::testing::Test {};

// Test 1: Verify pointer is aligned to 64 bytes
TEST_F(AlignedAllocatorTest, PointerAlignment) {
    std::vector<double, aligned_allocator<double, 64>> v(100);
    
    double* ptr = v.data();
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    
    EXPECT_EQ(addr % 64, 0) << "Pointer not aligned to 64 bytes";
}

// Test 2: Verify alignment with different sizes
TEST_F(AlignedAllocatorTest, AlignmentMultipleSizes) {
    for (size_t size : {1, 10, 64, 100, 1000, 10000}) {
        std::vector<double, aligned_allocator<double, 64>> v(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(v.data());
        EXPECT_EQ(addr % 64, 0) << "Failed for size " << size;
    }
}

// Test 3: Data preservation
TEST_F(AlignedAllocatorTest, DataPreservation) {
    std::vector<double, aligned_allocator<double, 64>> v(100);
    for (size_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<double>(i) * 3.14159;
    }
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_DOUBLE_EQ(v[i], static_cast<double>(i) * 3.14159);
    }
}

// Test 4: Alignment after resize
TEST_F(AlignedAllocatorTest, AlignmentAfterResize) {
    std::vector<double, aligned_allocator<double, 64>> v(50);
    v.resize(500);
    uintptr_t addr = reinterpret_cast<uintptr_t>(v.data());
    EXPECT_EQ(addr % 64, 0) << "Pointer not aligned after resize";
}

// Test 5: Alignment after push_back
TEST_F(AlignedAllocatorTest, AlignmentAfterPushBack) {
    std::vector<double, aligned_allocator<double, 64>> v(50);
    for (int i = 0; i < 500; ++i) {
        v.push_back(static_cast<double>(i));
    }
    uintptr_t addr = reinterpret_cast<uintptr_t>(v.data());
    EXPECT_EQ(addr % 64, 0);
}

// Test 6: PointsSoA alignment
TEST_F(AlignedAllocatorTest, PointsSoAAlignment) {
    PointsSoA soa(100);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(soa.dataX()) % 64, 0) << "X not aligned";
    EXPECT_EQ(reinterpret_cast<uintptr_t>(soa.dataY()) % 64, 0) << "Y not aligned";
    EXPECT_EQ(reinterpret_cast<uintptr_t>(soa.dataZ()) % 64, 0) << "Z not aligned";
    EXPECT_EQ(reinterpret_cast<uintptr_t>(soa.dataIds()) % 64, 0) << "IDs not aligned";
}

// Test 7: PointsSoA data integrity
TEST_F(AlignedAllocatorTest, PointsSoADataIntegrity) {
    PointsSoA soa(10);
    for (size_t i = 0; i < 10; ++i) {
        Point p{i, 1.0 * i, 2.0 * i, 3.0 * i};
        soa.set(i, p);
    }
    for (size_t i = 0; i < 10; ++i) {
        Point p = soa[i];
        EXPECT_EQ(p.id(), i);
        EXPECT_DOUBLE_EQ(p.getX(), 1.0 * i);
        EXPECT_DOUBLE_EQ(p.getY(), 2.0 * i);
        EXPECT_DOUBLE_EQ(p.getZ(), 3.0 * i);
    }
}