#pragma once

#include <expected>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

auto parse_app_name(std::vector<std::string> args) -> std::string;
auto parse_arg(std::vector<std::string> args, int position) -> std::expected<fs::path, std::string>;
