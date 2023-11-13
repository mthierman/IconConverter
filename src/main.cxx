#define GDIPVER 0x0110

#include <Windows.h>
#include <gdiplus.h>

#include <algorithm>
#include <print>
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
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

        Gdiplus::Bitmap bitmap(inputCanonical.wstring().c_str());
        Gdiplus::Image* img = new Gdiplus::Bitmap(256, 256);
        Gdiplus::Graphics g(img);

        g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias8x8);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

        g.ScaleTransform(((float)256 / (float)bitmap.GetWidth()) *
                             (bitmap.GetHorizontalResolution() / img->GetHorizontalResolution()),
                         ((float)256 / (float)bitmap.GetHeight()) *
                             (bitmap.GetVerticalResolution() / img->GetVerticalResolution()));

        g.DrawImage(&bitmap, 0, 0);

        CLSID pngClsid;
        if (!GetEncoderClsid(L"image/png", &pngClsid))
            return 0;

        IStream* istream{nullptr};
        if (FAILED(::CreateStreamOnHGlobal(0, TRUE, &istream)))
            return 0;

        img->Save(istream, &pngClsid);
        img = nullptr;
        delete img;

        HGLOBAL hg{0};
        if (FAILED(::GetHGlobalFromStream(istream, &hg)))
            return 0;

        auto bufsize{::GlobalSize(hg)};
        std::vector<char> pngVec;
        pngVec.resize(bufsize);

        auto ptr{::GlobalLock(hg)};
        if (!ptr)
            return 0;

        memcpy(&pngVec[0], ptr, bufsize);
        ::GlobalUnlock(hg);

        // https://learn.microsoft.com/en-us/windows/apps/design/style/iconography/app-icon-construction
        // https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-icons
        // List of sizes: 16px 20px 24px 30px 32px 36px 40px 48px 60px 64px 72px 80px 96px 256px

        istream->Release();

        std::ofstream outputStream;
        uint8_t reserved{0};
        uint16_t type{1};
        uint16_t count{1};
        uint8_t width{0};
        uint8_t height{0};
        uint8_t colors{0};
        uint16_t planes{0};
        uint16_t bits{32};
        // We can get file size using std::filesystem but we should take buffer to match C#
        // implementation auto
        // size{static_cast<uint32_t>(std::filesystem::file_size(inputCanonical))};
        auto size{static_cast<uint32_t>(bufsize)};
        uint32_t offset{6 + 16};

        outputStream.open(outputFile, std::ios::binary);

        // Header
        // 0-1 Reserved, Must always be 0.
        outputStream.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
        outputStream.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
        // 2-3 Image type, 1 = icon (.ICO), 2 = cursor (.CUR).
        outputStream.write(reinterpret_cast<char*>(&type), sizeof(type));
        // 4-5 Number of images.
        outputStream.write(reinterpret_cast<char*>(&count), sizeof(count));

        // Entry
        // 0 Image width in pixels. Range is 0-255. 0 means 256 pixels.
        outputStream.write(reinterpret_cast<char*>(&width), sizeof(width));
        // 1 Image height in pixels. Range is 0-255. 0 means 256 pixels.
        outputStream.write(reinterpret_cast<char*>(&height), sizeof(height));
        // 2 Number of colors in the color palette. Should be 0 if no palette.
        outputStream.write(reinterpret_cast<char*>(&colors), sizeof(colors));
        // 3 Reserved.Should be 0.
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
        // Write image data. PNG must be stored in its entirety, with file header & must
        // be 32bpp ARGB format.
        outputStream.write(reinterpret_cast<char*>(&pngVec[0]), pngVec.size() * sizeof(char));
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    return 0;
}
