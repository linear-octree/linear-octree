#pragma once

#include "encoding/encoding_types.hpp"
#include "kernels/kernel_types.hpp"
#include "search_algorithms/search_types.hpp"
#include "structures/container_types.hpp"
#include "structures/structure_types.hpp"

#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <omp.h>
#include <optional>
#include <set>
#include <vector>

namespace fs = std::filesystem;

class main_options {
public:
  // Files & paths
  fs::path inputFile{};
  fs::path outputDirName{"out"};
  std::string inputFileName{};

  // Container type
  ContainerType containerType =
      ContainerType::AoS; // default == AoS == std::vector<Point> wrapper

  // Benchmark parameters
  std::vector<float> benchmarkRadii{2.5, 5.0, 7.5, 10.0};
  std::vector<size_t> benchmarkKValues{10, 50, 250, 1000};

  size_t repeats{2};
  size_t numSearches{10000};

  std::set<Kernel_t> kernels{Kernel_t::sphere, Kernel_t::circle, Kernel_t::cube,
                             Kernel_t::square};
  std::set<SearchAlgo> searchAlgos{
      SearchAlgo::NEIGHBORS_PTR, SearchAlgo::NEIGHBORS,
      SearchAlgo::NEIGHBORS_PRUNE, SearchAlgo::NEIGHBORS_STRUCT};
  std::set<SearchStructure> searchStructures{SearchStructure::PTR_OCTREE,
                                             SearchStructure::LINEAR_OCTREE};
  std::set<EncoderType> encodings{
      EncoderType::NO_ENCODING,          EncoderType::MORTON_ENCODER_3D,
      EncoderType::MORTON_ENCODER_2D_X,  EncoderType::MORTON_ENCODER_2D_Y,
      EncoderType::MORTON_ENCODER_2D_Z,  EncoderType::HILBERT_ENCODER_3D,
      EncoderType::HILBERT_ENCODER_2D_X, EncoderType::HILBERT_ENCODER_2D_Y,
      EncoderType::HILBERT_ENCODER_2D_Z,
  };

  bool debug{false};
  bool buildEncBenchmarks{false};
  std::optional<SearchStructure> memoryStructure{std::nullopt};
  bool localityBenchmarks{false};
  bool cacheProfiling{false};
  bool checkResults{false};
  bool useWarmup{true};
  std::vector<double> approximateTolerances{50.0};
  std::vector<int> numThreads{omp_get_max_threads()};
  bool sequentialSearches{false};
  bool searchAll{false};
  size_t maxPointsLeaf = 128;
  double pclOctResolution = 0.1;
};

extern main_options mainOptions;

enum LongOptions : int {
  HELP,
  INPUT,
  CONTAINER_TYPE,
  OUTPUT,
  RADII,
  K_VALUES,
  REPEATS,
  SEARCHES,
  KERNELS,
  SEARCH_ALGOS,
  ENCODINGS,

  DEBUG,
  BUILD_ENC,
  MEMORY,
  LOCALITY,
  CACHE_PROFILING,
  CHECK,
  NO_WARMUP,
  APPROXIMATE_TOLERANCES,
  NUM_THREADS,
  SEQUENTIAL_SEARCH_SET,
  MAX_POINTS_LEAF,
  PCL_OCT_RESOLUTION
};

// Define short options
const char *const short_opts = "h:i:c:o:r:v:s:t:b:k:a:e:cb:";

// Define long options
const option long_opts[] = {
    {"help", no_argument, nullptr, LongOptions::HELP},
    {"input", required_argument, nullptr, LongOptions::INPUT},
    {"container-type", required_argument, nullptr, LongOptions::CONTAINER_TYPE},
    {"output", required_argument, nullptr, LongOptions::OUTPUT},
    {"radii", required_argument, nullptr, LongOptions::RADII},
    {"kvalues", required_argument, nullptr, LongOptions::K_VALUES},
    {"repeats", required_argument, nullptr, LongOptions::REPEATS},
    {"searches", required_argument, nullptr, LongOptions::SEARCHES},
    {"kernels", required_argument, nullptr, LongOptions::KERNELS},
    {"search-algos", required_argument, nullptr, LongOptions::SEARCH_ALGOS},
    {"encodings", required_argument, nullptr, LongOptions::ENCODINGS},

    {"debug", no_argument, nullptr, LongOptions::DEBUG},
    {"build-enc", no_argument, nullptr, LongOptions::BUILD_ENC},
    {"memory", required_argument, nullptr, LongOptions::MEMORY},
    {"locality", no_argument, nullptr, LongOptions::LOCALITY},
    {"cache-profiling", no_argument, nullptr, LongOptions::CACHE_PROFILING},
    {"check", no_argument, nullptr, LongOptions::CHECK},
    {"no-warmup", no_argument, nullptr, LongOptions::NO_WARMUP},
    {"approx-tol", required_argument, nullptr,
     LongOptions::APPROXIMATE_TOLERANCES},
    {"num-threads", required_argument, nullptr, LongOptions::NUM_THREADS},
    {"sequential", no_argument, nullptr, LongOptions::SEQUENTIAL_SEARCH_SET},
    {"max-leaf", required_argument, nullptr, LongOptions::MAX_POINTS_LEAF},
    {"pcl-oct-resolution", required_argument, nullptr,
     LongOptions::PCL_OCT_RESOLUTION},
    {nullptr, 0, nullptr, 0}};

void printHelp();
void setDefaults();
std::set<Kernel_t> parseKernelOptions(const std::string &kernelStr);
std::set<SearchAlgo> parseSearchAlgoOptions(const std::string &kernelStr);
std::set<EncoderType> parseEncodingOptions(const std::string &kernelStr);
std::string getKernelListString();
std::string getSearchAlgoListString();
std::string getEncoderListString();
void processArgs(int argc, char **argv);