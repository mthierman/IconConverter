#include "helpers.hxx"

#include <wil/com.h>

#include <cstdint>
#include <fstream>
#include <print>

auto main(int argc, char* argv[]) -> int
{
    auto coUninitialize{wil::CoInitializeEx(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)};

    auto [inputFile, outputFile]{helpers::getPaths(argc, argv)};

    std::println("Input file: {}", inputFile.string());
    std::println("Output file: {}", outputFile.string());

    if (!fs::exists(inputFile))
    {
        std::println("Input file does not exist, aborting...");
        std::exit(EXIT_FAILURE);
    }

    fs::path inputFileCanonical;

    try
    {
        inputFileCanonical = fs::canonical(inputFile);
    }
    catch (const fs::filesystem_error& e)
    {
        std::println("Canonical input file failure: {}, aborting...", e.what());
        std::exit(EXIT_FAILURE);
    }

    if (inputFileCanonical.empty())
    {
        std::println("Input file empty, aborting...");
        std::exit(EXIT_FAILURE);
    }

    std::println("Input file canonical: {}", inputFileCanonical.string());

    std::vector<std::vector<char>> bitmaps;
    std::vector<int> bitmapSizes{256, 128, 96, 80, 72, 64, 60, 48, 40, 36, 32, 30, 24, 20, 16};

    for (auto i = 0; i < bitmapSizes.size(); i++)
    {
        bitmaps.push_back(helpers::getBitmap(inputFileCanonical.wstring().c_str(), bitmapSizes[i]));
    }

    std::vector<uint32_t> sizes;
    sizes.reserve(bitmaps.size());
    for (auto const& bitmap : bitmaps)
    {
        sizes.push_back(static_cast<uint32_t>(bitmap.size()));
    }

    std::ofstream outputStream;
    uint16_t count{static_cast<uint16_t>(bitmapSizes.size())};
    uint32_t offset{6 + (16 * static_cast<uint32_t>(count))};

    std::vector<uint32_t> positions;
    positions.push_back(offset);
    auto cumulate{offset};
    for (auto i = 0; i < sizes.size(); ++i)
    {
        cumulate += sizes[i];
        positions.push_back(cumulate);
    }

    std::vector<uint8_t> dimensions;
    dimensions.reserve(sizes.size());
    for (const auto& size : sizes)
    {
        dimensions.push_back(static_cast<uint8_t>(size));
    }

    outputStream.open(outputFile, std::ios::binary);

    helpers::writeHeader(outputStream, count);

    helpers::writeEntry(outputStream, bitmaps[0], 0, positions[0]);
    for (int i = 1; i < static_cast<int>(bitmapSizes.size()); i++)
    {
        helpers::writeEntry(outputStream, bitmaps[i], dimensions[i], positions[i]);
    }

    for (int i = 0; i < static_cast<int>(bitmapSizes.size()); i++)
    {
        helpers::writeBitmap(outputStream, bitmaps[i]);
    }
}
