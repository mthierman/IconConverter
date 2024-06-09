#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace helpers
{
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>;
auto getBitmap(fs::path inputFileCanonical, int size) -> std::vector<char>;
auto writeHeader(std::ofstream& outputStream, uint16_t count) -> void;
auto writeEntry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t size,
                uint32_t offset) -> void;
auto writeBitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void;
} // namespace helpers
