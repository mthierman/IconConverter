using System.Drawing;
using System.Drawing.Imaging;

if (!OperatingSystem.IsWindows())
    return;
if (args.Length != 2)
    return;
if (!Path.Exists(Path.GetFullPath(args[0])))
    return;

var outDir = Directory.GetParent(Path.GetFullPath(args[1]));
if (outDir != null)
    Directory.CreateDirectory(outDir.ToString());

using var inputStream = new FileStream(System.IO.Path.GetFullPath(args[0]), FileMode.Open);
using var image = Image.FromStream(inputStream);
if (image is null)
    return;

using var bitmap = new Bitmap(image, new Size(256, 256));
if (bitmap is null)
    return;

using var memoryStream = new MemoryStream();
if (memoryStream is null)
    return;

bitmap.Save(memoryStream, ImageFormat.Png);

using var outputStream = new FileStream(System.IO.Path.GetFullPath(args[1]), FileMode.OpenOrCreate);
using var binaryWriter = new BinaryWriter(outputStream);
if (binaryWriter is null)
    return;

// Header
// 0-1 Reserved, Must always be 0.
binaryWriter.Write((byte)0);
binaryWriter.Write((byte)0);
// 2-3 Image type, 1 = icon (.ICO), 2 = cursor (.CUR).
binaryWriter.Write((short)1);
// 4-5 Number of images.
binaryWriter.Write((short)1);

// Entry
// 0 Image width in pixels. Range is 0-255. 0 means 256 pixels.
binaryWriter.Write((byte)0);
// 1 Image height in pixels. Range is 0-255. 0 means 256 pixels.
binaryWriter.Write((byte)0);
// 2 Number of colors in the color palette. Should be 0 if no palette.
binaryWriter.Write((byte)0);
// 3 Reserved. Should be 0.
binaryWriter.Write((byte)0);
// 4-5
// .ICO: Color planes (0 or 1).
// .CUR: Horizontal coordinates of the hotspot in pixels from the left.
binaryWriter.Write((short)0);
// 6-7
// .ICO: Bits per pixel.
// .CUR: Vertical coordinates of the hotspot in pixels from the top.
binaryWriter.Write((short)32);
// 8-11 Size of the image's data in bytes
binaryWriter.Write((int)memoryStream.Length);
// 12-15 Offset of image data from the beginning of the file.
binaryWriter.Write((int)(6 + 16));
// Write image data. PNG must be stored in its entirety, with file header & must
// be 32bpp ARGB format.
binaryWriter.Write(memoryStream.ToArray());

binaryWriter.Flush();
