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
auto write_entry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t size,
                 uint32_t offset) -> void;
auto write_bitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void;

auto main() -> int
{
    ULONG_PTR gdiplusToken{0};
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) !=
        Gdiplus::Status::Ok)
        return 0;

    auto argv{get_argv()};

    if (argv.size() == 3)
    {
        std::filesystem::path inputFile{argv[1]};
        std::filesystem::path outputFile{argv[2]};
        std::filesystem::path inputCanonical;

        if (!std::filesystem::exists(inputFile))
            return 0;

        try
        {
            inputCanonical = std::filesystem::canonical(inputFile);
        }
        catch (std::filesystem::filesystem_error& /*e*/)
        {
            return 0;
        }

        if (inputCanonical.empty())
            return 0;

        CLSID pngClsid;

        if (!get_encoder_clsid(L"image/png", &pngClsid))
            return 0;

        Gdiplus::Bitmap inputBitmap(inputCanonical.wstring().c_str());

        std::vector<std::vector<char>> bitmaps;
        std::vector<int> bitmapSizes{256, 96, 64, 48, 32, 24, 16};

        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[0], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[1], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[2], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[3], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[4], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[5], &pngClsid));
        bitmaps.push_back(get_bitmap(inputBitmap, bitmapSizes[6], &pngClsid));

        std::vector<uint32_t> sizes;
        sizes.reserve(bitmaps.size());
        for (auto const& bitmap : bitmaps)
        {
            sizes.push_back(static_cast<uint32_t>(bitmap.size()));
        }

        std::ofstream outputStream;
        uint16_t count{7};
        uint32_t offset{6 + (16 * static_cast<uint32_t>(count))};

        std::vector<uint32_t> positions;
        positions.push_back(offset);
        positions.push_back(offset + sizes[0]);
        positions.push_back(offset + sizes[0] + sizes[1]);
        positions.push_back(offset + sizes[0] + sizes[1] + sizes[2]);
        positions.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3]);
        positions.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4]);
        positions.push_back(offset + sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] +
                            sizes[5]);

        std::vector<uint8_t> dimensions;
        dimensions.reserve(sizes.size());
        for (const auto& size : sizes)
        {
            dimensions.push_back(static_cast<uint8_t>(size));
        }

        outputStream.open(outputFile, std::ios::binary);

        write_header(outputStream, count);

        write_entry(outputStream, bitmaps[0], 0, positions[0]);
        write_entry(outputStream, bitmaps[1], dimensions[1], positions[1]);
        write_entry(outputStream, bitmaps[2], dimensions[2], positions[2]);
        write_entry(outputStream, bitmaps[3], dimensions[3], positions[3]);
        write_entry(outputStream, bitmaps[4], dimensions[4], positions[4]);
        write_entry(outputStream, bitmaps[5], dimensions[5], positions[5]);
        write_entry(outputStream, bitmaps[6], dimensions[6], positions[6]);

        write_bitmap(outputStream, bitmaps[0]);
        write_bitmap(outputStream, bitmaps[1]);
        write_bitmap(outputStream, bitmaps[2]);
        write_bitmap(outputStream, bitmaps[3]);
        write_bitmap(outputStream, bitmaps[4]);
        write_bitmap(outputStream, bitmaps[5]);
        write_bitmap(outputStream, bitmaps[6]);
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
        return {};

    img->Save(istream, pClsid);

    HGLOBAL hGlobal{nullptr};
    if (FAILED(::GetHGlobalFromStream(istream, &hGlobal)))
        return {};

    auto bufsize{::GlobalSize(hGlobal)};

    auto* ptr{::GlobalLock(hGlobal)};
    if (ptr == nullptr)
        return {};

    std::vector<char> vec;
    vec.resize(bufsize);

    std::copy(static_cast<char*>(ptr), (static_cast<char*>(ptr) + bufsize), vec.begin());

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

auto write_entry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t size,
                 uint32_t offset) -> void
{
    uint8_t reserved{0};
    uint8_t colors{0};
    uint16_t planes{1};
    uint16_t bits{32};
    auto bitmapSize{static_cast<uint32_t>(bitmap.size())};

    // Entry
    // 0 Image width in pixels. Range is 0-255. 0 means 256 pixels.
    outputStream.write(reinterpret_cast<char*>(&size), sizeof(size));
    // 1 Image height in pixels. Range is 0-255. 0 means 256 pixels.
    outputStream.write(reinterpret_cast<char*>(&size), sizeof(size));
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
    outputStream.write(reinterpret_cast<char*>(&bitmapSize), sizeof(bitmapSize));
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
