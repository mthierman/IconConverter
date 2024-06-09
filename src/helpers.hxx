#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace helpers
{
namespace fs = std::filesystem;

auto getAppName(std::vector<std::string> args) -> std::string;
auto checkArg(std::vector<std::string> args, int position) -> std::expected<fs::path, std::string>;
auto getPaths(std::vector<std::string> args) -> std::pair<fs::path, fs::path>;
} // namespace helpers
