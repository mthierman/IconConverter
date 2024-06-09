#include "helpers.hxx"

#include <print>

namespace helpers
{
auto getAppName(std::vector<std::string> args) -> std::string
{
    return fs::path(args.at(0)).filename().replace_extension("").string();
}

auto checkArg(std::vector<std::string> args, int position) -> std::expected<fs::path, std::string>
{
    args.erase(args.begin());

    try
    {
        return args.at(position);
    }
    catch (const std::out_of_range& e)
    {
        switch (position)
        {
            case 0:
                return std::unexpected("No input file specified");
            case 1:
                return std::unexpected("No output file specified");
            default:
                return std::unexpected("");
        }
    }
}

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
