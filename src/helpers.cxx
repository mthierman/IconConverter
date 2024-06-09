#include "helpers.hxx"

#include <wincodec.h>

#include <wil/com.h>

#include <print>

namespace helpers
{
auto getPaths(int argc, char* argv[]) -> std::pair<fs::path, fs::path>
{
    std::pair<fs::path, fs::path> paths;
    std::vector<std::string> args(argv + 1, argc + argv);

    try
    {
        paths.first = args.at(0);
    }
    catch (const std::out_of_range& e)
    {
        std::println("No input file specified");
        std::exit(EXIT_FAILURE);
    }

    try
    {
        paths.second = args.at(1);
    }
    catch (const std::out_of_range& e)
    {
        std::println("No output file specified");
        std::exit(EXIT_FAILURE);
    }

    return paths;
}

auto getBitmap(fs::path inputFileCanonical, int size) -> std::vector<char>
{
    wil::com_ptr<IWICImagingFactory> pFactory;
    wil::com_ptr<IWICBitmapDecoder> pDecoder;
    wil::com_ptr<IWICBitmapFrameDecode> pFrameDecode;
    wil::com_ptr<IWICBitmapEncoder> pEncoder;
    wil::com_ptr<IWICBitmapFrameEncode> pFrameEncode;
    wil::com_ptr<IWICFormatConverter> pFormatter;
    wil::com_ptr<IWICBitmapScaler> pScaler;
    wil::com_ptr<IWICStream> pStream;
    wil::com_ptr<IPropertyBag2> pPropertyBag;

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&pFactory))))
        return {};

    if (FAILED(pFactory->CreateDecoderFromFilename(inputFileCanonical.wstring().c_str(), NULL,
                                                   GENERIC_READ, WICDecodeMetadataCacheOnDemand,
                                                   &pDecoder)))
        return {};

    if (FAILED(pDecoder->GetFrame(0, &pFrameDecode)))
        return {};

    wil::unique_hglobal hglobal;
    wil::com_ptr<IStream> istream;

    if (FAILED(::CreateStreamOnHGlobal(hglobal.get(), TRUE, &istream)))
        return {};

    if (FAILED(pFactory->CreateBitmapScaler(&pScaler)))
        return {};
    if (FAILED(pScaler->Initialize(pFrameDecode.get(), size, size,
                                   WICBitmapInterpolationModeHighQualityCubic)))
        return {};

    UINT bytesPerPixel{4};
    UINT stride{size * bytesPerPixel};
    UINT bufsize{size * size * bytesPerPixel};

    std::vector<BYTE> scaledBuffer(bufsize);
    if (FAILED(pScaler->CopyPixels(NULL, stride, bufsize, scaledBuffer.data())))
        return {};

    if (FAILED(pFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder)))
        return {};
    if (FAILED(pEncoder->Initialize(istream.get(), WICBitmapEncoderNoCache)))
        return {};
    if (FAILED(pEncoder->CreateNewFrame(&pFrameEncode, &pPropertyBag)))
        return {};

    if (FAILED(pFrameEncode->Initialize(pPropertyBag.get())))
        return {};
    if (FAILED(pFrameEncode->SetSize(size, size)))
        return {};
    WICPixelFormatGUID pixelFormatDestination{GUID_WICPixelFormat32bppBGRA};
    if (FAILED(pFrameEncode->SetPixelFormat(&pixelFormatDestination)))
        return {};
    if (FAILED(pFrameEncode->WritePixels(size, stride, bufsize, scaledBuffer.data())))
        return {};

    if (FAILED(pFrameEncode->Commit()))
        return {};
    if (FAILED(pEncoder->Commit()))
        return {};

    if (FAILED(::GetHGlobalFromStream(istream.get(), &hglobal)))
        return {};
    auto vecBufsize{::GlobalSize(hglobal.get())};
    auto* ptr{::GlobalLock(hglobal.get())};
    if (ptr == nullptr)
        return {};

    std::vector<char> vec;
    vec.resize(vecBufsize);

    std::copy(static_cast<char*>(ptr), (static_cast<char*>(ptr) + vecBufsize), vec.begin());

    ::GlobalUnlock(hglobal.get());

    return vec;
}

auto writeHeader(std::ofstream& outputStream, uint16_t count) -> void
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

auto writeEntry(std::ofstream& outputStream, std::vector<char>& bitmap, uint8_t size,
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

auto writeBitmap(std::ofstream& outputStream, std::vector<char>& bitmap) -> void
{
    // Write image data. PNG must be stored in its entirety, with file header & must
    // be 32bpp ARGB format.
    auto bitmapSize{static_cast<int64_t>(bitmap.size())};
    auto charSize{static_cast<int64_t>(sizeof(char))};
    outputStream.write(reinterpret_cast<char*>(bitmap.data()), (bitmapSize * charSize));
}
} // namespace helpers
