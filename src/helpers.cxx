#include "helpers.hxx"

#include <print>

namespace helpers
{
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>
{
    std::pair<fs::path, fs::path> paths;
    std::vector<std::string> args(argv + 1, argc + argv);

    try
    {
        paths.first = args.at(0);
    }
    catch (const std::out_of_range& e)
    {
        std::println("No input file specified");
        std::exit(EXIT_FAILURE);
    }

    try
    {
        paths.second = args.at(1);
    }
    catch (const std::out_of_range& e)
    {
        std::println("No output file specified");
        std::exit(EXIT_FAILURE);
    }

    return paths;
}
} // namespace helpers
