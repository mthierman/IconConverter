#pragma once

#include <Windows.h>
#include <string>

auto narrow(std::wstring in) -> std::string
{
    if (!in.empty())
    {
        auto inSize{static_cast<int>(in.size())};

        auto outSize{::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS,
                                           in.data(), inSize, nullptr, 0, nullptr, nullptr)};

        if (outSize > 0)
        {
            std::string out;
            out.resize(static_cast<size_t>(outSize));

            if (::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS,
                                      in.data(), inSize, out.data(), outSize, nullptr, nullptr) > 0)
                return out;
        }
    }

    return {};
}

auto widen(std::string in) -> std::wstring
{
    if (!in.empty())
    {
        auto inSize{static_cast<int>(in.size())};

        auto outSize{
            ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.data(), inSize, nullptr, 0)};

        if (outSize > 0)
        {
            std::wstring out;
            out.resize(static_cast<size_t>(outSize));

            if (::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.data(), inSize, out.data(),
                                      outSize) > 0)
                return out;
        }
    }

    return {};
}

auto get_argv() -> std::vector<std::string>
{
    int argc{0};
    std::vector<std::string> argv;

    auto wideArgs{::CommandLineToArgvW(::GetCommandLineW(), &argc)};

    for (int i = 0; i < argc; i++)
    {
        auto arg{narrow(wideArgs[i])};
        argv.emplace_back(arg);
    }

    ::LocalFree(wideArgs);

    return argv;
}
