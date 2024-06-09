#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace helpers
{
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>;
} // namespace helpers
