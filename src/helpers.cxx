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

auto getPaths(std::vector<std::string> args) -> std::pair<fs::path, fs::path>
{
    auto inputFile{helpers::checkArg(args, 0)};
    auto outputFile{helpers::checkArg(args, 1)};

    if (!inputFile.has_value())
    {
        std::println("{}", inputFile.error());
        std::exit(EXIT_FAILURE);
    }

    if (!outputFile.has_value())
    {
        std::println("{}", outputFile.error());
        std::exit(EXIT_FAILURE);
    }
}
} // namespace helpers
