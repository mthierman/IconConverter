#include "helpers.hxx"

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
