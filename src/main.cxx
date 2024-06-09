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
}
