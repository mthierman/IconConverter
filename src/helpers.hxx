#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace helpers
{
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>;

auto getBitmap(fs::path inputFileCanonical, int size) -> std::vector<char>;
} // namespace helpers
