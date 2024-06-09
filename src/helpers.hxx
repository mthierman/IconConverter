#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace helpers
{
namespace fs = std::filesystem;

auto getAppName(std::vector<std::string> args) -> std::string;
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>;
} // namespace helpers
