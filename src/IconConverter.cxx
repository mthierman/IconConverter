#include <expected>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

auto parse_app_name(std::vector<std::string> args) -> std::string
{
    return fs::path(args.at(0)).filename().replace_extension("").string();
}

auto parse_arg(std::vector<std::string> args, int position) -> std::expected<fs::path, std::string>
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

auto main(int argc, char* argv[]) -> int
{
    auto args{std::vector<std::string>(argv, argc + argv)};

    auto appName{parse_app_name(args)};
    auto inputFile{parse_arg(args, 0)};
    auto outputFile{parse_arg(args, 1)};

    if (!inputFile.has_value())
    {
        std::println("{}", inputFile.error());
        return EXIT_FAILURE;
    }

    if (!outputFile.has_value())
    {
        std::println("{}", outputFile.error());
        return EXIT_FAILURE;
    }

    std::println("Exiting...");
}
