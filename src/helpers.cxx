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
} // namespace helpers
