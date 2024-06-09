#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace helpers
{
namespace fs = std::filesystem;

auto parse_app_name(std::vector<std::string> args) -> std::string;
auto parse_arg(std::vector<std::string> args, int position) -> std::expected<fs::path, std::string>;
} // namespace helpers
