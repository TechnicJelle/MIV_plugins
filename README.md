# Tech's [MIV](https://github.com/Stvff/MIV) plugins

Available plugins:
- `miv_stbi`: uses [stb_image](https://github.com/nothings/stb) for loading images
	- `.JPG`, `.JPEG`, `.JFIF`
	- `.PNG`
	- `.BMP`
	- `.PSD`
	- `.TGA`
	- `.GIF` Animated GIFs only show the first frame.
	- `.PPM` Only the binary format is supported (P6). The ascii format (P3) is not supported.
	- `.PGM` Only the binary format is supported (P5). The ascii format (P2) is not supported.
- `miv_libwebp`: uses the system-installed [libwebp](https://chromium.googlesource.com/webm/libwebp)
	- `.WEBP` Animated WEBPs only show the first frame.
- `miv_zipkra`: reads the file as zip using the system-installed [libzip](https://github.com/nih-at/libzip/),
  loads the metadata from it using the system-installed [libxml2](https://gitlab.gnome.org/GNOME/libxml2),
  and loads the `mergedimage.png` from it using [stb_image](https://github.com/nothings/stb)
	- `.KRA`

Planned plugins:
- `miv_libavif`: uses the system-installed [libavif](https://github.com/aomediacodec/libavif)
	- `.AVIF`
- `miv_libheif`: uses the system-installed [libheif](https://github.com/strukturag/libheif)
	- `.HEIC`
	- `.AVIF`
	- `.VVC`
	- `.AVC`
	- `.JPEG`
	- `.JPEG2000`
	- `.HTJ2K`
- `miv_libjxl`: uses the system-installed [libjxl](https://github.com/libjxl/libjxl)
	- `.JXL`

## [Click here to download!](../../releases/latest)


## Building

First, clone the repository. Make sure to clone recursively!

Then compile by running [`./compile.sh release`](compile.sh).
