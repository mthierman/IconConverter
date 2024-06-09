#include "helpers.hxx"

#include <wil/com.h>

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
}
