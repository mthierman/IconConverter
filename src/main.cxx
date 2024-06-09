#include "helpers.hxx"

#include <print>

auto main(int argc, char* argv[]) -> int
{
    auto paths{helpers::getPaths(argc, argv)};

    std::println("Input file: {}", paths.first.string());
    std::println("Output file: {}", paths.second.string());
}
