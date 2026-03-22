#pragma once

#include "structures/structure_types.hpp"

enum SearchAlgo {
  NEIGHBORS_PTR,
  NEIGHBORS,
  NEIGHBORS_PRUNE,
  NEIGHBORS_STRUCT,
  NEIGHBORS_APPROX,
  NEIGHBORS_UNIBN,
  NEIGHBORS_PCLKD,
  NEIGHBORS_PCLOCT,
  NEIGHBORS_NANOFLANN,
  NEIGHBORS_PICOTREE,
  KNN_V2,
  KNN_NANOFLANN,
  KNN_PCLKD,
  KNN_PCLOCT,
  KNN_PICOTREE
};

constexpr std::pair<SearchAlgo, std::string_view> searchAlgoMap[] = {
    {SearchAlgo::NEIGHBORS_PTR, "neighborsPtr"},
    {SearchAlgo::NEIGHBORS, "neighbors"},
    {SearchAlgo::NEIGHBORS_PRUNE, "neighborsPrune"},
    {SearchAlgo::NEIGHBORS_STRUCT, "neighborsStruct"},
    {SearchAlgo::NEIGHBORS_APPROX, "neighborsApprox"},
    {SearchAlgo::NEIGHBORS_UNIBN, "neighborsUnibn"},
    {SearchAlgo::NEIGHBORS_PCLKD, "neighborsPCLKD"},
    {SearchAlgo::NEIGHBORS_PCLOCT, "neighborsPCLOct"},
    {SearchAlgo::NEIGHBORS_NANOFLANN, "neighborsNanoflann"},
    {SearchAlgo::NEIGHBORS_PICOTREE, "neighborsPico"},
    {SearchAlgo::KNN_V2, "KNNV2"},
    {SearchAlgo::KNN_NANOFLANN, "KNNNanoflann"},
    {SearchAlgo::KNN_PCLKD, "KNNPCLKD"},
    {SearchAlgo::KNN_PCLOCT, "KNNPCLOCT"},
    {SearchAlgo::KNN_PICOTREE, "KNNPico"}};

constexpr std::pair<SearchAlgo, SearchStructure> algoToStructureMap[] = {
    {SearchAlgo::NEIGHBORS_PTR, SearchStructure::PTR_OCTREE},
    {SearchAlgo::NEIGHBORS, SearchStructure::LINEAR_OCTREE},
    {SearchAlgo::NEIGHBORS_PRUNE, SearchStructure::LINEAR_OCTREE},
    {SearchAlgo::NEIGHBORS_STRUCT, SearchStructure::LINEAR_OCTREE},
    {SearchAlgo::NEIGHBORS_APPROX, SearchStructure::LINEAR_OCTREE},
    {SearchAlgo::NEIGHBORS_UNIBN, SearchStructure::UNIBN_OCTREE},
    {SearchAlgo::NEIGHBORS_PCLKD, SearchStructure::PCL_KDTREE},
    {SearchAlgo::NEIGHBORS_PCLOCT, SearchStructure::PCL_OCTREE},
    {SearchAlgo::NEIGHBORS_NANOFLANN, SearchStructure::NANOFLANN_KDTREE},
    {SearchAlgo::NEIGHBORS_PICOTREE, SearchStructure::PICOTREE},
    {SearchAlgo::KNN_V2, SearchStructure::LINEAR_OCTREE},
    {SearchAlgo::KNN_NANOFLANN, SearchStructure::NANOFLANN_KDTREE},
    {SearchAlgo::KNN_PCLKD, SearchStructure::PCL_KDTREE},
    {SearchAlgo::KNN_PCLOCT, SearchStructure::PCL_OCTREE},
    {SearchAlgo::KNN_PICOTREE, SearchStructure::PICOTREE}};

constexpr SearchStructure algoToStructure(SearchAlgo algo) {
  for (const auto &[key, val] : algoToStructureMap) {
    if (key == algo)
      return val;
  }
  return SearchStructure::PTR_OCTREE; // Default fallback
}

constexpr std::string_view searchAlgoToString(SearchAlgo algo) {
  for (const auto &[key, val] : searchAlgoMap) {
    if (key == algo)
      return val;
  }
  return "Unknown";
}