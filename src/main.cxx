#include "helpers.hxx"

#include <print>

auto main(int argc, char* argv[]) -> int
{
    // auto args{std::vector<std::string>(argv, argc + argv)};
    // std::println("{} v{}", APP_NAME, APP_VERSION);

    auto paths{helpers::getPaths(argc, argv)};

    std::println("Exiting...");
}
