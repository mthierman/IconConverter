#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

auto get_bitmap(const std::filesystem::path& inputCanonical, const int& size) -> std::vector<char>;
auto write_header(std::ofstream& outputStream, uint16_t count) -> void;
auto write_entry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t size,
                 uint32_t offset) -> void;
auto write_bitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void;
