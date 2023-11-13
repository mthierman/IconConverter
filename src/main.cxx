#define GDIPVER 0x0110

#include <Windows.h>
#include <gdiplus.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <print>
#include <utility>
#include <vector>

#include "helpers.hxx"

auto get_encoder_clsid(const std::wstring& format, CLSID* pClsid) -> bool;
auto get_bitmap(Gdiplus::Bitmap& bitmap, const int& size, CLSID* pClsid) -> std::vector<char>;
auto write_header(std::ofstream& outputStream, uint16_t count) -> void;
auto write_entry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t width,
                 uint8_t height, uint32_t offset) -> void;
auto write_bitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void;

auto main() -> int
{
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) !=
        Gdiplus::Status::Ok)
        return 0;

    auto argv{get_argv()};

    if (argv.size() == 3)
    {
        std::filesystem::path inputFile{argv[1]};
        std::filesystem::path outputFile{argv[2]};

        if (!std::filesystem::exists(inputFile))
            return 0;

        std::filesystem::path inputCanonical{std::filesystem::canonical(inputFile)};

        CLSID pngClsid;

        if (!get_encoder_clsid(L"image/png", &pngClsid))
            return 0;

        Gdiplus::Bitmap bitmap(inputCanonical.wstring().c_str());

        auto bitmap256{get_bitmap(bitmap, 256, &pngClsid)};
        auto bitmap96{get_bitmap(bitmap, 96, &pngClsid)};
        auto bitmap64{get_bitmap(bitmap, 64, &pngClsid)};
        auto bitmap48{get_bitmap(bitmap, 48, &pngClsid)};
        auto bitmap32{get_bitmap(bitmap, 32, &pngClsid)};
        auto bitmap24{get_bitmap(bitmap, 24, &pngClsid)};
        auto bitmap16{get_bitmap(bitmap, 16, &pngClsid)};

        std::vector<uint32_t> sizes;
        sizes.push_back(static_cast<uint32_t>(bitmap256.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap96.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap64.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap48.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap32.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap24.size()));
        sizes.push_back(static_cast<uint32_t>(bitmap16.size()));

        std::ofstream outputStream;
        uint16_t count{7};
        uint32_t offset{6 + (16 * static_cast<uint32_t>(count))};

        std::vector<uint32_t> pos;
        pos.push_back(offset);
        pos.push_back(offset + sizes[0]);
        pos.push_back(offset + sizes[0] + sizes[1]);
        pos.push_back(offset + sizes[0] + sizes[1] + sizes[2]);
        pos.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3]);
        pos.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4]);
        pos.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + sizes[5]);

        outputStream.open(outputFile, std::ios::binary);

        write_header(outputStream, count);

        write_entry(outputStream, bitmap256, 0, 0, pos[0]);
        write_entry(outputStream, bitmap96, 96, 96, pos[1]);
        write_entry(outputStream, bitmap64, 64, 64, pos[2]);
        write_entry(outputStream, bitmap48, 48, 48, pos[3]);
        write_entry(outputStream, bitmap32, 32, 32, pos[4]);
        write_entry(outputStream, bitmap24, 24, 24, pos[5]);
        write_entry(outputStream, bitmap16, 16, 16, pos[6]);

        write_bitmap(outputStream, bitmap256);
        write_bitmap(outputStream, bitmap96);
        write_bitmap(outputStream, bitmap64);
        write_bitmap(outputStream, bitmap48);
        write_bitmap(outputStream, bitmap32);
        write_bitmap(outputStream, bitmap24);
        write_bitmap(outputStream, bitmap16);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    return 0;
}

auto get_encoder_clsid(const std::wstring& format, CLSID* pClsid) -> bool
{
    UINT num{0};
    UINT size{0};

    if (Gdiplus::GetImageEncodersSize(&num, &size) != Gdiplus::Status::Ok)
        return false;

    std::vector<Gdiplus::ImageCodecInfo> codecs(size);

    if (Gdiplus::GetImageEncoders(num, size, codecs.data()) != Gdiplus::Status::Ok)
        return false;

    for (auto codec : codecs)
    {
        if (format == std::wstring{codec.MimeType})
        {
            *pClsid = codec.Clsid;
            return true;
        }
    }

    return false;
}

auto get_bitmap(Gdiplus::Bitmap& bitmap, const int& size, CLSID* pClsid) -> std::vector<char>
{
    auto newBitmap{std::make_unique<Gdiplus::Bitmap>(size, size)};
    Gdiplus::Image* img = newBitmap.get();
    Gdiplus::Graphics g(img);

    g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias8x8);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    g.ScaleTransform((static_cast<float>(size) / static_cast<float>(bitmap.GetWidth())) *
                         (bitmap.GetHorizontalResolution() / img->GetHorizontalResolution()),
                     (static_cast<float>(size) / static_cast<float>(bitmap.GetHeight())) *
                         (bitmap.GetVerticalResolution() / img->GetVerticalResolution()));

    g.DrawImage(&bitmap, 0, 0);

    IStream* istream{nullptr};
    if (FAILED(::CreateStreamOnHGlobal(nullptr, TRUE, &istream)))
        return std::vector<char>{};

    img->Save(istream, pClsid);

    HGLOBAL hGlobal{0};
    if (FAILED(::GetHGlobalFromStream(istream, &hGlobal)))
        return std::vector<char>{};

    auto bufsize{::GlobalSize(hGlobal)};

    auto* ptr{::GlobalLock(hGlobal)};
    if (ptr == nullptr)
        return std::vector<char>{};

    std::vector<char> vec(static_cast<char*>(ptr), static_cast<char*>(ptr) + bufsize);

    ::GlobalUnlock(hGlobal);
    istream->Release();

    return vec;
}

auto write_header(std::ofstream& outputStream, uint16_t count) -> void
{
    uint16_t reserved{0};
    uint16_t type{1};

    // Header
    // 0-1 Reserved, Must always be 0.
    outputStream.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
    // 2-3 Image type, 1 = icon (.ICO), 2 = cursor (.CUR).
    outputStream.write(reinterpret_cast<char*>(&type), sizeof(type));
    // 4-5 Number of images.
    outputStream.write(reinterpret_cast<char*>(&count), sizeof(count));
}

auto write_entry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t width,
                 uint8_t height, uint32_t offset) -> void
{
    uint8_t reserved{0};
    uint8_t colors{0};
    uint16_t planes{1};
    uint16_t bits{32};
    auto size{static_cast<uint32_t>(bitmap.size())};

    // Entry
    // 0 Image width in pixels. Range is 0-255. 0 means 256 pixels.
    outputStream.write(reinterpret_cast<char*>(&width), sizeof(width));
    // 1 Image height in pixels. Range is 0-255. 0 means 256 pixels.
    outputStream.write(reinterpret_cast<char*>(&height), sizeof(height));
    // 2 Number of colors in the color palette. Should be 0 if no palette.
    outputStream.write(reinterpret_cast<char*>(&colors), sizeof(colors));
    // 3 Reserved. Should be 0.
    outputStream.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
    // 4-5
    // .ICO: Color planes (0 or 1).
    // .CUR: Horizontal coordinates of the hotspot in pixels from the left.
    outputStream.write(reinterpret_cast<char*>(&planes), sizeof(planes));
    // 6-7
    // .ICO: Bits per pixel.
    // .CUR: Vertical coordinates of the hotspot in pixels from the top.
    outputStream.write(reinterpret_cast<char*>(&bits), sizeof(bits));
    // 8-11 Size of the image's data in bytes
    outputStream.write(reinterpret_cast<char*>(&size), sizeof(size));
    // 12-15 Offset of image data from the beginning of the file.
    outputStream.write(reinterpret_cast<char*>(&offset), sizeof(offset));
}

auto write_bitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void
{
    // Write image data. PNG must be stored in its entirety, with file header & must
    // be 32bpp ARGB format.
    auto bitmapSize{static_cast<int64_t>(bitmap.size())};
    auto charSize{static_cast<int64_t>(sizeof(char))};
    outputStream.write(reinterpret_cast<char*>(bitmap.data()), (bitmapSize * charSize));
}
