# pragma once

#include <string_view>
#include <utility>

enum class Kernel_t
{
	circle,
	sphere,
	square,
	cube
};

constexpr std::pair<Kernel_t, std::string_view> kernelMap[] = {
	{Kernel_t::sphere, "sphere"},
	{Kernel_t::circle, "circle"},
	{Kernel_t::cube, "cube"},
	{Kernel_t::square, "square"}
};

constexpr std::string_view kernelToString(Kernel_t kernel) {
    for (const auto& [key, val] : kernelMap) {
        if (key == kernel) return val;
    }
    return "Unknown";
}