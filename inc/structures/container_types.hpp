#pragma once

#include <string_view>
#include <utility>

enum class ContainerType { AoS, SoA };

constexpr std::pair<ContainerType, std::string_view> containerTypeMap[] = {
    {ContainerType::AoS, "PointsAoS"},
    {ContainerType::SoA, "PointsSoA"},
};

constexpr std::string_view containerTypeToString(ContainerType containerType) {
  for (const auto &[key, val] : containerTypeMap) {
    if (key == containerType)
      return val;
  }
  return "Unknown";
}