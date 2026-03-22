#pragma once

#include <string_view>
#include <utility>

enum SearchStructure {
  PTR_OCTREE,
  LINEAR_OCTREE,
  UNIBN_OCTREE,
  PCL_OCTREE,
  PCL_KDTREE,
  NANOFLANN_KDTREE,
  PICOTREE
};

constexpr std::pair<SearchStructure, std::string_view> structureMap[] = {
    {SearchStructure::PTR_OCTREE, "ptrOct"},
    {SearchStructure::LINEAR_OCTREE, "linOct"},
    {SearchStructure::UNIBN_OCTREE, "uniOct"},
    {SearchStructure::PCL_OCTREE, "pclOct"},
    {SearchStructure::PCL_KDTREE, "pclKD"},
    {SearchStructure::NANOFLANN_KDTREE, "nanoKD"},
    {SearchStructure::PICOTREE, "picoTree"}};

constexpr std::string_view searchStructureToString(SearchStructure structure) {
  for (const auto &[key, val] : structureMap) {
    if (key == structure)
      return val;
  }
  return "Unknown";
}