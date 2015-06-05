#include "GLTexture2FreeImage.h"

#define FREEIMAGE_LIB
#include "FreeImage.h"
#include "Leap/GL/Texture2.h"

#include <cassert>

// Load an image given a filepath.
FIBITMAP *LoadFreeImageBitmap (const std::string &filepath) {
  FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filepath.c_str(), 0);

  if (format == FIF_UNKNOWN) {
    format = FreeImage_GetFIFFromFilename(filepath.c_str());
  }

  if (FreeImage_FIFSupportsReading(format)) {
    return FreeImage_Load(format, filepath.c_str());
  } else {
    return nullptr;
  }
}

// This function will attempt to use the various "bitmap information" functions of
// FreeImage to determine the Texture2Params corresponding to the 
Leap::GL::Texture2 *AttemptToCreateGLTexture2FromFIBITMAP (FIBITMAP *bitmap, Leap::GL::Texture2Params params) {
  if (bitmap == nullptr) {
    // TODO: better error reporting
    throw std::runtime_error("error while loading image via FreeImage");
  }

  // For the following, see http://freeimage.sourceforge.net/documentation.html

  // FreeImage_GetImageType will return one of the following values.
  // FIT_UNKNOWN        Unknown format (returned value only, never use it as input value)
  // FIT_BITMAP         Standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
  // FIT_UINT16         Array of unsigned short: unsigned 16-bit
  // FIT_INT16          Array of short: signed 16-bit
  // FIT_UINT32         Array of unsigned long: unsigned 32-bit
  // FIT_INT32          Array of long: signed 32-bit
  // FIT_FLOAT          Array of float: 32-bit IEEE floating point
  // FIT_DOUBLE         Array of double: 64-bit IEEE floating point
  // FIT_COMPLEX        Array of FICOMPLEX: 2 x 64-bit IEEE floating point
  // FIT_RGB16          48-bit RGB image: 3 x 16-bit
  // FIT_RGBA16         64-bit RGBA image: 4 x 16-bit
  // FIT_RGBF           96-bit RGB float image: 3 x 32-bit IEEE floating point
  // FIT_RGBAF          128-bit RGBA float image: 4 x 32-bit IEEE floating point
  FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(bitmap);
  // FreeImage_GetColorType will return one of the following values.
  // FIC_MINISBLACK     Monochrome bitmap (1-bit) : first palette entry is black. Palletised bitmap (4 or 8-bit) and single channel non standard bitmap: the bitmap has a greyscale palette
  // FIC_MINISWHITE     Monochrome bitmap (1-bit) : first palette entry is white. Palletised bitmap (4 or 8-bit) : the bitmap has an inverted greyscale palette
  // FIC_PALETTE        Palettized bitmap (1, 4 or 8 bit)
  // FIC_RGB            High-color bitmap (16, 24 or 32 bit), RGB16 or RGBF
  // FIC_RGBALPHA       High-color bitmap with an alpha channel (32 bit bitmap, RGBA16 or RGBAF)
  // FIC_CMYK           CMYK bitmap (32 bit only)
//  FREE_IMAGE_COLOR_TYPE color_type = FreeImage_GetColorType(bitmap); // unused
  // FreeImage_GetBPP Returns the size of one pixel in the bitmap in bits. For example when
  // each pixel takes 32-bits of space in the bitmap, this function returns 32. Possible bit
  // depths are 1, 4, 8, 16, 24, 32 for standard bitmaps and 16-, 32-, 48-, 64-, 96- and
  // 128-bit for non standard bitmaps.
  unsigned bpp = FreeImage_GetBPP(bitmap);

  // The next section of code is effectively a "translation" from the FreeImage bitmap
  // information gathered above to Texture2Params, which is data directly usable by OpenGL.

  GLenum pixel_data_format;   // this is determined by the image data loaded by FreeImage
  GLenum pixel_data_type;     // this is determined by the image data loaded by FreeImage
  // NOTE: internal_format could/should be exposed as a configurable option to the user of
  // GLTexture2Loader.  Presumably OpenGL will pick the representation (byte, short, float, etc)
  // that is the fastest, though the human should be able to hint or override this.
  GLint internal_format;

  // NOTE: This code is written with the OpenGL 2.1 standard in mind.  It would benefit from
  // using OpenGL 3, in that the internal_format value could be set more accurately, so that
  // there is no data loss (e.g. loading float image data into 16-bit component representations).
  switch (image_type) {
    case FIT_BITMAP:
      // We only support 8, 24 and 32 bpp for now.  Perhaps monochrome bitmaps would be useful.
      if (bpp == 8) {
        pixel_data_format = GL_LUMINANCE;
        internal_format = GL_RGB;
      } else if (bpp == 24) {
        // See FreeImage.h (and FreeImage docs under Pixel access functions / Color model)
        pixel_data_format = (FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR) ? GL_BGR : GL_RGB;
        internal_format = GL_RGB8;
      } else if (bpp == 32) {
        // See FreeImage.h (and FreeImage docs under Pixel access functions / Color model)
        pixel_data_format = (FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR) ? GL_BGRA : GL_RGBA;
        internal_format = GL_RGBA8;
      } else {
        throw std::runtime_error("unsupported bits-per-pixel; only 8, 24 and 32 bpp are supported");
      }
      pixel_data_type = GL_UNSIGNED_BYTE;
      break;

    case FIT_UINT16:
      assert(bpp == 16 && "unexpected bpp value");
      pixel_data_format = GL_RED;
      pixel_data_type = GL_UNSIGNED_SHORT;
      internal_format = GL_R16UI;
      break;

    case FIT_INT16:
      assert(bpp == 16 && "unexpected bpp value");
      pixel_data_format = GL_RED;
      pixel_data_type = GL_SHORT;
      internal_format = GL_R16I;
      break;
      
    case FIT_UINT32:
      assert(bpp == 32 && "unexpected bpp value");
      pixel_data_format = GL_RED;
      pixel_data_type = GL_UNSIGNED_INT;
      internal_format = GL_R32UI;
      break;
      
    case FIT_INT32:
      assert(bpp == 32 && "unexpected bpp value");
      pixel_data_format = GL_RED;
      pixel_data_type = GL_INT;
      internal_format = GL_R32I;
      break;
      
    case FIT_FLOAT:
      assert(bpp == 32 && "unexpected bpp value");
      pixel_data_format = GL_RED;
      pixel_data_type = GL_FLOAT;
      internal_format = GL_R32F;
      break;

    case FIT_DOUBLE:
      assert(bpp == 64 && "unexpected bpp value");
      throw std::runtime_error("FIT_DOUBLE is not a supported type");
      // pixel_data_format = GL_RED;
      // pixel_data_type = GL_DOUBLE; // this is apparently not a valid value, so this type is unsupported.
      // internal_format = GL_R32F; // this choice is lossy
      break;

    case FIT_COMPLEX:
      assert(bpp == 128 && "unexpected bpp value");
      throw std::runtime_error("FIT_COMPLEX is not a supported type");
      // pixel_data_format = GL_RG;
      // pixel_data_type = GL_DOUBLE; // this is apparently not a valid value, so this type is unsupported..
      // internal_format = GL_RG32F; // this choice is lossy
      break;

    case FIT_RGB16:
      assert(bpp == 48 && "unexpected bpp value");
      // The RGB/BGR order consideration only applies to 24 and 32 bit formats (see FreeImage.h near FREEIMAGE_COLORORDER)
      pixel_data_format = GL_RGB;
      pixel_data_type = GL_UNSIGNED_SHORT;
      internal_format = GL_RGB16;
      break;

    case FIT_RGBA16:
      assert(bpp == 64 && "unexpected bpp value");
      // The RGB/BGR order consideration only applies to 24 and 32 bit formats (see FreeImage.h near FREEIMAGE_COLORORDER)
      pixel_data_format = GL_RGBA;
      pixel_data_type = GL_UNSIGNED_SHORT;
      internal_format = GL_RGBA16;
      break;

    case FIT_RGBF:
      assert(bpp == 96 && "unexpected bpp value");
      // The RGB/BGR order consideration only applies to 24 and 32 bit formats (see FreeImage.h near FREEIMAGE_COLORORDER)
      pixel_data_format = GL_RGB;
      pixel_data_type = GL_FLOAT;
      internal_format = GL_RGB32F_ARB;
      break;

    case FIT_RGBAF:
      assert(bpp == 128 && "unexpected bpp value");
      // The RGB/BGR order consideration only applies to 24 and 32 bit formats (see FreeImage.h near FREEIMAGE_COLORORDER)
      pixel_data_format = GL_RGBA;
      pixel_data_type = GL_FLOAT;
      internal_format = GL_RGBA32F_ARB;
      break;

    case FIT_UNKNOWN:
    default:
      throw std::runtime_error("unknown image type");
  }

  GLsizei width = FreeImage_GetWidth(bitmap);
  GLsizei height = FreeImage_GetHeight(bitmap);
  params.SetWidth(width);
  params.SetHeight(height);
  params.SetInternalFormat(internal_format);

  // FreeImage_GetBits Returns a pointer to the data-bits of the bitmap. It is up to you to
  // interpret these bytes correctly, according to the results of FreeImage_GetBPP, 
  // FreeImage_GetRedMask, FreeImage_GetGreenMask and FreeImage_GetBlueMask.  For a
  // performance reason, the address returned by FreeImage_GetBits is aligned on a 16 bytes
  // alignment boundary.  Note: FreeImage_GetBits will return NULL if the bitmap does not
  // contain pixel data (i.e. if it contains only header and possibly some or all metadata).
  // See also FreeImage_HasPixels.
  const void *raw_pixel_data = static_cast<const void *>(FreeImage_GetBits(bitmap));
  if (raw_pixel_data == nullptr) {
    throw std::runtime_error("FreeImage_GetBits returned nullptr, indicating there was no pixel data in the image.  We could add the capability to create an uninitialized Leap::GL::Texture2 from this.");
  }
  assert(bpp % 8 == 0 && "only whole-byte pixel formats are supported (convenience choice on the part of this function's design)");
  size_t bytes_per_pixel = bpp / 8;
  size_t raw_pixel_data_size = width * height * bytes_per_pixel;
  Leap::GL::Texture2PixelData pixel_data(pixel_data_format, pixel_data_type, raw_pixel_data, raw_pixel_data_size);
  // Create the Leap::GL::Texture2 using the derived parameters and pixel data.
  return new Leap::GL::Texture2(params, pixel_data);
}

Leap::GL::Texture2 *LoadGLTexture2UsingFreeImage (const std::string &filepath, const Leap::GL::Texture2Params &params) {
  FIBITMAP *bitmap = LoadFreeImageBitmap(filepath);
  try {
    Leap::GL::Texture2 *texture = AttemptToCreateGLTexture2FromFIBITMAP(bitmap, params);
    assert(texture != nullptr); // an exception should have been thrown instead of returning nullptr.
    FreeImage_Unload(bitmap);
    return texture;
  } catch (...) {
    FreeImage_Unload(bitmap);
    throw; // rethrow the exception.
  }
}
