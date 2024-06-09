#include "helpers.hxx"

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
