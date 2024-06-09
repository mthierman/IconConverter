#include <string>
#include <vector>
#include <print>
#include <filesystem>

namespace fs = std::filesystem;

auto main(int argc, char* argv[]) -> int
{
    std::vector<std::string> args(argv, argc + argv);

    auto app{fs::path(args.at(0)).filename().replace_extension("")};
    args.erase(args.begin());

    std::println("{}", app.string());

    for (const auto& arg : args)
    {
        std::println("{}", arg);
    }

    std::println("Test");
}
