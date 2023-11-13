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

bool get_encoder_clsid(const WCHAR*, CLSID*);
std::vector<char> get_bitmap(const std::filesystem::path, const int, const int, CLSID*);
void write_header(std::ofstream&, uint16_t);
void write_entry(std::ofstream&, std::vector<char>, uint8_t, uint8_t, uint32_t);
void write_bitmap(std::ofstream&, std::vector<char>);

bool get_encoder_clsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num{0};
    UINT size{0};
    Gdiplus::GetImageEncodersSize(&num, &size);

    if (size == 0)
        return false;

    Gdiplus::ImageCodecInfo* pImageCodecInfo{nullptr};
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));

    if (pImageCodecInfo == 0)
        return false;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT i = 0; i < num; ++i)
    {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return true;
        }
    }

    free(pImageCodecInfo);
    return false;
}

std::vector<char> get_bitmap(const std::filesystem::path file, const int width, const int height,
                             CLSID* pClsid)
{
    std::vector<char> vec;

    Gdiplus::Bitmap bitmap(file.wstring().c_str());
    Gdiplus::Image* img = new Gdiplus::Bitmap(width, height);
    Gdiplus::Graphics g(img);

    g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias8x8);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    g.ScaleTransform((static_cast<float>(width) / static_cast<float>(bitmap.GetWidth())) *
                         (bitmap.GetHorizontalResolution() / img->GetHorizontalResolution()),
                     (static_cast<float>(height) / static_cast<float>(bitmap.GetHeight())) *
                         (bitmap.GetVerticalResolution() / img->GetVerticalResolution()));

    g.DrawImage(&bitmap, 0, 0);

    IStream* istream{nullptr};
    if (FAILED(::CreateStreamOnHGlobal(0, TRUE, &istream)))
        return vec;

    img->Save(istream, pClsid);
    img = nullptr;
    delete img;

    HGLOBAL hg{0};
    if (FAILED(::GetHGlobalFromStream(istream, &hg)))
        return vec;

    auto bufsize{::GlobalSize(hg)};
    vec.resize(bufsize);

    auto ptr{::GlobalLock(hg)};
    if (!ptr)
        return vec;

    memcpy(&vec[0], ptr, bufsize);

    ::GlobalUnlock(hg);
    istream->Release();

    return vec;
}

void write_header(std::ofstream& outputStream, uint16_t count)
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

void write_entry(std::ofstream& outputStream, std::vector<char> bitmap, uint8_t width,
                 uint8_t height, uint32_t offset)
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

void write_bitmap(std::ofstream& outputStream, std::vector<char> bitmap)
{
    // Write image data. PNG must be stored in its entirety, with file header & must
    // be 32bpp ARGB format.
    outputStream.write(reinterpret_cast<char*>(&bitmap[0]), bitmap.size() * sizeof(char));
}

int main(int argc, char** argv)
{
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) !=
        Gdiplus::Status::Ok)
        return 0;

    auto args{std::vector<std::string>(argv, argv + argc)};

    if (args.size() == 3)
    {
        std::filesystem::path inputFile{args[1]};
        std::filesystem::path outputFile{args[2]};

        if (!std::filesystem::exists(inputFile))
            return 0;

        std::filesystem::path inputCanonical{std::filesystem::canonical(inputFile)};

        CLSID pngClsid;
        CLSID bmpClsid;

        if (!get_encoder_clsid(L"image/png", &pngClsid))
            return 0;

        if (!get_encoder_clsid(L"image/bmp", &bmpClsid))
            return 0;

        // 16px 20px 24px 30px 32px 36px 40px 48px 60px 64px 72px 80px 96px 256px
        // std::vector<std::vector<char>> bitmaps;
        auto bitmap256{get_bitmap(inputCanonical, 256, 256, &pngClsid)};
        auto bitmap96{get_bitmap(inputCanonical, 96, 96, &pngClsid)};
        auto bitmap64{get_bitmap(inputCanonical, 64, 64, &pngClsid)};
        auto bitmap48{get_bitmap(inputCanonical, 48, 48, &pngClsid)};
        auto bitmap32{get_bitmap(inputCanonical, 32, 32, &pngClsid)};
        auto bitmap24{get_bitmap(inputCanonical, 24, 24, &pngClsid)};
        auto bitmap16{get_bitmap(inputCanonical, 16, 16, &pngClsid)};

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
        uint32_t headerOffset{6};
        uint32_t entryOffset{16};
        uint32_t countOffset{static_cast<uint32_t>(count)};

        std::vector<uint32_t> pos;
        pos.push_back((headerOffset + (entryOffset * countOffset)));
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0]);
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0] + sizes[1]);
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0] + sizes[1] +
                      sizes[2]);
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0] + sizes[1] +
                      sizes[2] + sizes[3]);
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0] + sizes[1] +
                      sizes[2] + sizes[3] + sizes[4]);
        pos.push_back((headerOffset + (entryOffset * countOffset)) + sizes[0] + sizes[1] +
                      sizes[2] + sizes[3] + sizes[4] + sizes[5]);

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
