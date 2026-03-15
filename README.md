# LinearOctree

## Background

LiDAR (Light and Ranging Detection) technology has now become the quintessential technique for collecting geospatial data from the earth's surface. This code implements a linearized octree based on ideas from Keller et al. and Behley et al. for fast fixed-radius neighbourhood searches, achieving better performance than other Octrees and KD-trees tested, such as nanoflann KD-tree, picoTree, PCL Octree and KD-Tree, and unibnOctree. We also analyze the performance of Morton and Hilbert Space Filling Curves (SFCs). SFC Reordering allows for faster searches and is essential for the construction of the linear Octree. Extensive benchmarking and result plotting code and scripts are also provided.

## Publication

This code is associated with a research paper published as a preprint:

**Efficient Neighbourhood Search in 3D Point Clouds Through Space-Filling Curves and Linear Octrees**

**Authors:** Pablo D. Viñambres, Miguel Yermo, Silvia R. Alcaraz, Oscar G. Lorenzo, Francisco F. Rivera, José C. Cabaleiro

**arXiv Preprint:** https://arxiv.org/abs/2603.06771 ([PDF](https://arxiv.org/pdf/2603.06771))

### Citation

If you use this code in your research, please cite the associated paper using one of the following formats:

**BibTeX:**
```bibtex
@article{Viñambres2026,
  title={Efficient Neighbourhood Search in 3D Point Clouds Through Space-Filling Curves and Linear Octrees},
  author={Viñambres, Pablo D. and Yermo, Miguel and Alcaraz, Silvia R. and Lorenzo, Oscar G. and Rivera, Francisco F. and Cabaleiro, José C.},
  journal={arXiv preprint arXiv:2603.06771},
  year={2026}
}
```

**APA:**
```
Viñambres, P. D., Yermo, M., Alcaraz, S. R., Lorenzo, O. G., Rivera, F. F., & Cabaleiro, J. C. (2026). Efficient neighbourhood search in 3D point clouds through space-filling curves and linear octrees. arXiv Preprint, 2603.06771.
```

**DOI:** https://doi.org/10.48550/arXiv.2603.06771

## Installation

### Core Dependencies (Required)

The library requires only **3 core dependencies**:

- **CMake** 3.14+
- **OpenMP** (for parallelization)
- **Eigen3** (linear algebra)
- **LASlib** (LAS file I/O)

**Install system packages:**
```bash
# Ubuntu/Debian
sudo apt-get install cmake libomp-dev libeigen3-dev
```

**Build and install LASlib:**
```bash
bash scripts/install_laslib.sh
```

### Building the Library

#### Standard Build (CPU only)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build --parallel $(nproc)
```

This produces:
- **Static library:** `build/liboctrees-benchmark_static.a`
- **Shared library:** `build/liboctrees-benchmark_shared.so`

#### With CUDA Support

Requires CUDA 12.5+ and a compatible compiler:

```bash
# Create conda environment with CUDA
conda create -n cuda-build -c conda-forge gcc=12 gxx=12 cmake ninja cuda-toolkit=12.5 -y
conda activate cuda-build

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DWITH_CUDA=ON .
cmake --build build --parallel $(nproc)
```

#### Installing the Library

To install the library to a local directory:

```bash
cmake --install build --prefix ./install
```

Or to the system:

```bash
sudo cmake --install build
```

### Tests

The library includes comprehensive unit tests using GoogleTest (automatically fetched during build):

**Build everything including tests:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build --parallel $(nproc)
```

**Run all tests:**
```bash
ctest --test-dir build --output-on-failure
```

**Run specific test:**
```bash
ctest --test-dir build -R LinearOctreeTest.RadiusSearch --output-on-failure
```

### Examples

A basic usage example is provided in `examples/example.cpp`. It is automatically compiled alongside the library:

```bash
# Run the example
./build/examples/example
```

This demonstrates:
- Loading a point cloud from file
- Creating a linear octree
- Performing radius and KNN searches

## Authorship
Grupo de Arquitectura de Computadores (GAC)  
Centro Singular de Investigación en Tecnologías Inteligentes (CiTIUS)  
Universidad de Santiago de Compostela (USC)  

Linear octree implementation, SFCs, benchmarking and plotting code from:
- Pablo Díaz Viñambres ([pablo.diaz.vinambres@rai.usc.es](mailto:pablo.diaz.vinambres@rai.usc.es))

Optimized search algorithm and vectorization of SFC encoders:
- Abel Rodríguez Calleja ([GitHub profile](https://github.com/Abel-Breaker))

Original pointer-based Octree, readers and program structure from: 
- Miguel Yermo García ([miguel.yermo@usc.es](mailto:miguel.yermo@usc.es))
- Silvia Rodríguez Alcaraz ([silvia.alcaraz@usc.es](mailto:silvia.alcaraz@usc.es))
