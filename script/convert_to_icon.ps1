function New-RcFile
{   
    <#
        .SYNOPSIS
            Create an .RC file
        .DESCRIPTION
            Create an .RC file and add the .ICO file created by ConvertTo-Icon to it
        .EXAMPLE
            NewRcFile -From .\Example.ico -To .\Example.rc
    #>

    [CmdletBinding()]
    param(
        # Input file
        [Parameter(Mandatory = $true, ValueFromPipelineByPropertyName = $true, Position = 0)]
        [string]$From,

        # Output file
        [Parameter(Mandatory = $true, ValueFromPipelineByPropertyName = $true, Position = 1)]
        [string]$To
    )

    process
    {
        $InputPath = [System.IO.Path]::GetFullPath($From)
        $InputFileName = [System.IO.Path]::GetFileName($InputPath)
        $OutputPath = [System.IO.Path]::GetFullPath($To)
        "1 ICON `"$($InputFileName)`"" | Out-File -FilePath $OutputPath
    }
}

function ConvertTo-Icon
{   
    <#
        .SYNOPSIS
            Convert 256x256 .PNG to .ICO
        .DESCRIPTION
            Convert a PNG image to a Windows icon file
        .EXAMPLE
            ConvertTo-Icon -From .\Example.png -To .\Example.ico

            CMake - Generate .ico and .rc file for app icon:
                execute_process(
                    COMMAND powershell "Import-Module ${CMAKE_SOURCE_DIR}/libs/clap-wrapper/cmake/convert_to_icon.ps1; ConvertTo-Icon ${CMAKE_SOURCE_DIR}/resources/test.png ${CMAKE_BINARY_DIR}/resources/${SA_TARGET}.ico"
                    COMMAND powershell "Import-Module ${CMAKE_SOURCE_DIR}/libs/clap-wrapper/cmake/convert_to_icon.ps1; New-RcFile ${CMAKE_BINARY_DIR}/resources/${SA_TARGET}.ico ${CMAKE_BINARY_DIR}/resources/${SA_TARGET}.rc"
                )

            CMake - Add .rc file to executable:
                target_sources(${SA_TARGET} PRIVATE
                    ${CLAP_WRAPPER_CMAKE_CURRENT_SOURCE_DIR}/src/detail/standalone/windows/winutils.cpp
                    ${CMAKE_BINARY_DIR}/resources/${SA_TARGET}.rc
                )

            We will load the icon from .rc file in the window class:
                wcex.hIcon = (HICON)::LoadImageW(::GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1), IMAGE_ICON, 0, 0,
                                                 LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED);
                wcex.hIconSm = (HICON)::LoadImageW(::GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1), IMAGE_ICON, 0, 0,
                                                   LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED);
    #>
    
    [CmdletBinding()]
    param(
        # Input file
        [Parameter(Mandatory = $true, ValueFromPipelineByPropertyName = $true, Position = 0)]
        [string]$From,

        # Output file
        [Parameter(Mandatory = $true, ValueFromPipelineByPropertyName = $true, Position = 1)]
        [string]$To
    )

    begin
    {
        Add-Type -AssemblyName System.Drawing, System.IO
    }
    
    process
    {
        #region Load
        $InputPath = [System.IO.Path]::GetFullPath($From)
        $OutputPath = [System.IO.Path]::GetFullPath($To)

        if (!(Test-Path $InputPath)) { return; }
        if (!(Test-Path ($OutputPath | Split-Path))) { New-Item -ItemType Directory ($OutputPath | Split-Path) }
        
        $Image = [System.Drawing.Image]::FromFile($InputPath)
        $Width = $Image.Width
        $Height = $Image.Height
        $Size = New-Object Drawing.Size $Width, $Height
        $Bitmap = New-Object Drawing.Bitmap $Image, $Size
        #endregion Load

        #region Save
        $MemoryStream = New-Object System.IO.MemoryStream
        $Bitmap.Save($MemoryStream, [System.Drawing.Imaging.ImageFormat]::Png)
        $CreateFile = [System.IO.File]::Create($OutputPath)
        $BinaryWriter = New-Object System.IO.BinaryWriter($CreateFile)
        
        # Header
        # 0-1 Reserved, Must always be 0.
        $BinaryWriter.Write([byte]0)
        $BinaryWriter.Write([byte]0)
        # 2-3 Image type, 1 = icon (.ICO), 2 = cursor (.CUR).
        $BinaryWriter.Write([int16]1)
        # 4-5 Number of images.
        $BinaryWriter.Write([int16]1)

        # Entry
        # 0 Image width in pixels. Range is 0-255. 0 means 256 pixels.
        $BinaryWriter.Write([byte]0)
        # 1 Image height in pixels. Range is 0-255. 0 means 256 pixels.
        $BinaryWriter.Write([byte]0)
        # 2 Number of colors in the color palette. Should be 0 if no palette.
        $BinaryWriter.Write([byte]0)
        # 3 Reserved. Should be 0.
        $BinaryWriter.Write([byte]0)
        # 4-5
        # .ICO: Color planes (0 or 1).
        # .CUR: Horizontal coordinates of the hotspot in pixels from the left.
        $BinaryWriter.Write([int16]0)
        # 6-7
        # .ICO: Bits per pixel.
        # .CUR: Vertical coordinates of the hotspot in pixels from the top.
        $BinaryWriter.Write([int16]32)
        # 8-11 Size of the image's data in bytes
        $BinaryWriter.Write([int]$MemoryStream.Length)
        # 12-15 Offset of image data from the beginning of the file.
        $BinaryWriter.Write([int](6 + 16))
        # Write image data. PNG must be stored in its entirety, with file header & must be 32bpp ARGB format.
        $BinaryWriter.Write($MemoryStream.ToArray())
        #endregion Save
        
        #region Dispose
        $BinaryWriter.Flush()
        $CreateFile.Close()
        $BinaryWriter.Dispose()
        $MemoryStream.Dispose()
        $Bitmap.Dispose()
        $Image.Dispose()
        #endregion Dispose
    }
}
