#include "LWVideo/LWImage.h"
#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWByteBuffer.h"
#include "cmp_core.h"
#include <png.h>
#include <cstring>
#include <cmath>
#include <algorithm>

bool LWImage::LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator,LWFileStream *ExistingStream){
	uint32_t Result = LWFileStream::IsExtensions(FilePath, "DDS", "dds", "PNG", "png", "TGA", "tga", "ktx2", "KTX2");
	if (Result < 2) return LoadImageDDS(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 4) return LoadImagePNG(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 6) return LoadImageTGA(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 8) return LoadImageKTX2(Image, FilePath, Allocator, ExistingStream);
	return false;
}

bool LWImage::LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	uint32_t Result = LWFileStream::IsExtensions(FilePath, "DDS", "dds", "PNG", "png", "TGA", "tga", "ktx2", "KTX2");
	if (Result < 2) return LoadImageDDS(Image, Buffer, Allocator);
	else if (Result < 4) return LoadImagePNG(Image, Buffer, Allocator);
	else if (Result < 6) return LoadImageTGA(Image, Buffer, Allocator);
	else if (Result < 8) return LoadImageKTX2(Image, Buffer, Allocator);
	return false;
}

bool LWImage::LoadImageTGA(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream){
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	LWByteBuffer Buf = LWByteBuffer((const int8_t*)MemBuffer, Stream.Length(), LWByteBuffer::BufferNotOwned);
	bool Result = LoadImageTGA(Image, Buf, Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageTGA(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	struct TGA_Header {
		uint16_t m_ColorMapOrigin;
		uint16_t m_ColorMapLength;
		uint16_t m_XOrigin;
		uint16_t m_YOrigin;
		uint16_t m_Width;
		uint16_t m_Height;
		uint8_t m_ColorMapSize;
		uint8_t m_IDLength;
		uint8_t m_ColorMapType;
		uint8_t m_ImageType;
		uint8_t m_PerPixelBits;
		uint8_t m_ImageDescriptor;
	};
	
	auto ReadHeader = [](LWByteBuffer &Buf, TGA_Header &Header)->bool {
		Header.m_IDLength = Buf.Read<uint8_t>();
		Header.m_ColorMapType = Buf.Read<uint8_t>();
		Header.m_ImageType = Buf.Read<uint8_t>();
		Header.m_ColorMapOrigin = Buf.Read<uint16_t>();
		Header.m_ColorMapLength = Buf.Read<uint16_t>();
		Header.m_ColorMapSize = Buf.Read<uint8_t>();
		Header.m_XOrigin = Buf.Read<uint16_t>();
		Header.m_YOrigin = Buf.Read<uint16_t>();
		Header.m_Width = Buf.Read<uint16_t>();
		Header.m_Height = Buf.Read<uint16_t>();
		Header.m_PerPixelBits = Buf.Read<uint8_t>();
		Header.m_ImageDescriptor = Buf.Read<uint8_t>();

		Header.m_ColorMapSize /= 8;
		Header.m_PerPixelBits /= 8;
		return true;
	};

	auto ReadColorMappedImage = [](LWByteBuffer &Buf, TGA_Header &H, LWImage &Img, LWAllocator &Allocator)->bool{
		const uint32_t MaxColorMapLength = 1024 * 256;
		const uint32_t RGB8 = 0xFE;
		const uint32_t RGBUnknown = 0xFF;
		char ColorMapBuffer[MaxColorMapLength];
		char IndexMapBuffer[MaxColorMapLength / 4];
		if (H.m_ColorMapLength*(H.m_ColorMapSize) > MaxColorMapLength) return false;
		uint32_t PackType = H.m_ColorMapSize == 4 ? RGBA8 : (H.m_ColorMapSize == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.Read<char>(ColorMapBuffer, H.m_ColorMapLength*H.m_ColorMapSize);
		Buf.Read<char>(IndexMapBuffer, H.m_Width*H.m_Height*H.m_PerPixelBits);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), 1, SRGBA, 0, nullptr, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		for (uint16_t i = 0; i < H.m_Width*H.m_Height; i++) {
			uint32_t IndexEntry = (uint32_t)IndexMapBuffer[i];
			if (H.m_PerPixelBits == 2) IndexEntry = (uint32_t)*(uint16_t*)(IndexMapBuffer + (i*H.m_PerPixelBits));
			char *ColorMapEntry = ColorMapBuffer + (IndexEntry * H.m_ColorMapSize);
			if (H.m_ColorMapSize == 4) {
				Texels[i * 4 + 0] = ColorMapEntry[2];
				Texels[i * 4 + 1] = ColorMapEntry[1];
				Texels[i * 4 + 2] = ColorMapEntry[0];
				Texels[i * 4 + 3] = ColorMapEntry[3];
			} else if (H.m_ColorMapSize == 3) {
				Texels[i * 4 + 0] = ColorMapEntry[2];
				Texels[i * 4 + 1] = ColorMapEntry[1];
				Texels[i * 4 + 2] = ColorMapEntry[0];
				Texels[i * 4 + 3] = 0xFF;
			}
		}
		return true;
	};

	auto ReadColorImage = [](LWByteBuffer &Buf, TGA_Header &H, LWImage &Img, LWAllocator &Allocator)->bool {
		const uint32_t RGB8 = 0xFE;
		const uint32_t RGBUnknown = 0xFF;
		uint32_t PackType = H.m_PerPixelBits == 4 ? RGBA8 : (H.m_PerPixelBits == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.OffsetPosition(H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), 1, SRGBA, 0, nullptr, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		//Offset so that we can reorganize in a forward fashion.
		uint16_t TexelOffset = (PackType == RGBA8 ? 0 : (H.m_Width*H.m_Height * 4) - (H.m_Width*H.m_Height * 3));
		uint8_t A = (PackType == RGBA8 ? 1 : 0);
		uint8_t B = (PackType == RGBA8 ? 0 : 0xFF);
		Buf.Read<uint8_t>(Texels + TexelOffset, H.m_Width*H.m_Height*H.m_PerPixelBits);
		//Reorganize to RGBA, and add alpha if necessary!
		for (uint16_t i = 0; i < H.m_Width*H.m_Height; i++) {
			uint8_t *LTexel = Texels + TexelOffset + (i * H.m_PerPixelBits);
			uint8_t *RTexel = Texels + i * 4;
			uint8_t Blue = LTexel[0];
			RTexel[0] = LTexel[2];
			RTexel[1] = LTexel[1];
			RTexel[2] = Blue;
			RTexel[3] = LTexel[3] * A + B;
		}
		return true;
	};

	auto ReadRunLengthColorMappedImage = [](LWByteBuffer &Buf, TGA_Header &H, LWImage &Img, LWAllocator &Allocator) {
		const uint32_t MaxColorMapLength = 1024 * 256;
		const uint32_t RGB8 = 0xFE;
		const uint32_t RGBUnknown = 0xFF;
		char ColorMapBuffer[MaxColorMapLength];
		char IndexMapBuffer[MaxColorMapLength / 4];
		if (H.m_ColorMapLength*H.m_ColorMapSize > MaxColorMapLength) return false;
		uint32_t PackType = H.m_ColorMapSize == 4 ? RGBA8 : (H.m_ColorMapSize == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.Read<char>(ColorMapBuffer, H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), 1, SRGBA, 0, nullptr, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		for (uint16_t i = 0; i < H.m_Width*H.m_Height;) {
			uint8_t RLHeader = Buf.Read<uint8_t>();;
			uint8_t RLCount = (RLHeader & 0x7F) + 1;
			if (RLHeader & 0x80) {
				Buf.Read<char>(IndexMapBuffer, H.m_PerPixelBits);
				uint32_t IndexEntry = (uint32_t)IndexMapBuffer[0];
				if (H.m_PerPixelBits == 2) IndexEntry = (uint32_t)*(uint16_t*)(IndexMapBuffer);
				char *ColorMapEntry = ColorMapBuffer + (IndexEntry * H.m_ColorMapSize);
				for (uint8_t k = 0; k < RLCount; k++) {
					if (H.m_ColorMapSize == 4) {
						Texels[(i + k) * 4 + 0] = ColorMapEntry[2];
						Texels[(i + k) * 4 + 1] = ColorMapEntry[1];
						Texels[(i + k) * 4 + 2] = ColorMapEntry[0];
						Texels[(i + k) * 4 + 3] = ColorMapEntry[3];
					} else if (H.m_ColorMapSize == 3) {
						Texels[(i + k) * 4 + 0] = ColorMapEntry[2];
						Texels[(i + k) * 4 + 1] = ColorMapEntry[1];
						Texels[(i + k) * 4 + 2] = ColorMapEntry[0];
						Texels[(i + k) * 4 + 3] = 0xFF;
					}
				}
			} else {
				Buf.Read<char>(IndexMapBuffer, H.m_PerPixelBits*RLCount);
				for (uint8_t k = 0; k < RLCount; k++) {
					uint32_t IndexEntry = (uint32_t)IndexMapBuffer[k];
					if (H.m_PerPixelBits == 2) IndexEntry = (uint32_t)*(uint16_t*)(IndexMapBuffer + (k * H.m_PerPixelBits));
					char *ColorMapEntry = ColorMapBuffer + (IndexEntry*H.m_ColorMapSize);
					if (H.m_ColorMapSize == 4) {
						Texels[(i + k) * 4 + 0] = ColorMapEntry[2];
						Texels[(i + k) * 4 + 1] = ColorMapEntry[1];
						Texels[(i + k) * 4 + 2] = ColorMapEntry[0];
						Texels[(i + k) * 4 + 3] = ColorMapEntry[3];
					} else if (H.m_ColorMapSize == 3) {
						Texels[(i + k) * 4 + 0] = ColorMapEntry[2];
						Texels[(i + k) * 4 + 1] = ColorMapEntry[1];
						Texels[(i + k) * 4 + 2] = ColorMapEntry[0];
						Texels[(i + k) * 4 + 3] = 0xFF;
					}
				}
			}
			i += (uint16_t)RLCount;
		}
		return true;
	};

	auto ReadRunLengthImage = [](LWByteBuffer &Buf, TGA_Header &H, LWImage &Img, LWAllocator &Allocator)->bool {
		const uint32_t RGB8 = 0xFE;
		const uint32_t RGBUnknown = 0xFF;
		uint8_t Buffer[1024];
		uint32_t PackType = H.m_PerPixelBits == 4 ? RGBA8 : (H.m_PerPixelBits == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.OffsetPosition(H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), 1, SRGBA, 0, nullptr, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		//Offset so that we can reorganize in a forward fashion.
		uint8_t A = (PackType == RGBA8 ? 1 : 0);
		uint8_t B = (PackType == RGBA8 ? 0 : 0xFF);

		for (uint16_t i = 0; i < H.m_Width*H.m_Height;) {
			uint8_t RLHeader = Buf.Read<uint8_t>();
			uint8_t RLCount = (RLHeader & 0x7F) + 1;
			if (RLHeader & 0x80) {
				Buf.Read<uint8_t>(Buffer, H.m_PerPixelBits);
				for (uint8_t k = 0; k < RLCount * 4; k += 4) {
					Texels[k + 0] = Buffer[2];
					Texels[k + 1] = Buffer[1];
					Texels[k + 2] = Buffer[0];
					Texels[k + 3] = Buffer[3] * A + B;
				}
			} else {
				Buf.Read<uint8_t>(Buffer, H.m_PerPixelBits * RLCount);
				for (uint8_t k = 0; k < RLCount; k++) {
					Texels[(i + k) * 4 + 0] = Buffer[(k * H.m_PerPixelBits) + 2];
					Texels[(i + k) * 4 + 1] = Buffer[(k * H.m_PerPixelBits) + 1];
					Texels[(i + k) * 4 + 2] = Buffer[(k * H.m_PerPixelBits) + 0];
					Texels[(i + k) * 4 + 3] = Buffer[(k * H.m_PerPixelBits) + 3] * A + B;
				}
			}
			i += (uint16_t)RLCount;
		}
		return true;
	};

	TGA_Header Header;
	if (!ReadHeader(Buffer, Header)) return false;
	Buffer.OffsetPosition(Header.m_IDLength);
	if (Header.m_ImageType == 1) return ReadColorMappedImage(Buffer, Header, Image, Allocator);
	else if (Header.m_ImageType == 2) return ReadColorImage(Buffer, Header, Image, Allocator);
	else if (Header.m_ImageType == 9) return ReadRunLengthColorMappedImage(Buffer, Header, Image, Allocator);
	else if (Header.m_ImageType == 10) return ReadRunLengthImage(Buffer, Header, Image, Allocator);
	return false;
}

bool LWImage::LoadImagePNG(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream){
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	LWByteBuffer Buf = LWByteBuffer((const int8_t*)MemBuffer, Stream.Length(), LWByteBuffer::BufferNotOwned);
	bool Result = LoadImagePNG(Image, Buf, Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImagePNG(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	const uint32_t RGB8 = 0xFE;
	//				PNG_COLOR_TYPE_:     _Gray, Pallete(Gray?), RGB,  RGB+Pallete, GRAY_ALPHA, (Gray_Alpha+Palette(Invalid?), RGB_ALPHA, (Color+Alpha+Palette(Invalid?))
	const uint32_t ColorPackTypes[8] = { R8,    R8,             RGB8, RGB8,        RG8,        RG8,                           RGBA8,     RGBA8 };
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) return false;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		return false;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		return false;
	}
	auto ReadFunc = [](png_structp png_ptr, png_bytep data, png_size_t length) -> void {
		LWByteBuffer *Buf = (LWByteBuffer*)png_get_io_ptr(png_ptr);
		Buf->Read<unsigned char>(data, (uint32_t)length);
		return;
	};
	const uint32_t MaxHeight = 8096;
	uint8_t *SubTexels[MaxHeight];
	png_uint_32 Width = 0;
	png_uint_32 Height = 0;
	int32_t bit_depth = 0;
	int32_t color_type = 0;
	int32_t interlace_type = 0;
	png_set_read_fn(png_ptr, (void*)&Buffer, ReadFunc);
	png_set_sig_bytes(png_ptr, 0);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &Width, &Height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);
	
	uint32_t PackType = ColorPackTypes[color_type];
	uint32_t RealPackType = (PackType == RGB8 ? SRGBA : PackType);
	if (Height > MaxHeight) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		return false;
	}
	png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) PackType = RealPackType = SRGBA;
	Image = LWImage(LWVector2i(Width, Height), 1,RealPackType, 0, nullptr, Allocator);
	uint32_t PNGByteSize = PackType == RGB8 ? 3 : GetBitSize(RealPackType) / 8;
	uint32_t RealByteSize = GetBitSize(RealPackType) / 8;

	uint8_t *PNGTexels = Image.GetTexels(0u) + (Width*Height*RealByteSize) - (Width*Height*PNGByteSize);
	for (uint32_t i = 0; i < Height; i++) SubTexels[i] = PNGTexels + (i*Width*PNGByteSize);
	png_read_image(png_ptr, SubTexels);
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	if (RealPackType != PackType) { //Map RGB8 to RGBA8
		uint8_t *RealTexels = Image.GetTexels(0u);
		for (uint32_t i = 0; i < Width*Height; i++) {
			RealTexels[i * 4 + 0] = PNGTexels[i * 3 + 0];
			RealTexels[i * 4 + 1] = PNGTexels[i * 3 + 1];
			RealTexels[i * 4 + 2] = PNGTexels[i * 3 + 2];
			RealTexels[i * 4 + 3] = 0xFF;
		}
	}
	return true;
};

bool LWImage::LoadImageDDS(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	LWByteBuffer Buf = LWByteBuffer((const int8_t*)MemBuffer, Stream.Length(), LWByteBuffer::BufferNotOwned);
	bool Result = LoadImageDDS(Image, Buf, Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageDDS(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	const uint32_t MagicHeader = 0x20534444;
	const uint32_t DX10Header = 0x30315844;
	const uint32_t DXT1Header = 0x31545844;
	const uint32_t DXT2Header = 0x32545844;
	const uint32_t DXT3Header = 0x33545844;
	const uint32_t DXT4Header = 0x34545844;
	const uint32_t DXT5Header = 0x35545844;
	const uint32_t DXT6Header = 0x36545844;
	const uint32_t DXT7Header = 0x37545844;

	const uint32_t DXT_ABGR32F = 116; //D3DFormat::D3DFMT_A32B32G32R32F, support for HDR images.

	const uint32_t DDSCAPS2_CUBEMAP = 0x200;
	const uint32_t DDSCAPS2_VOLUME = 0x200000;

	const uint32_t DDPF_ALPHAPIXELS = 0x1;
	const uint32_t DDPF_ALPHA = 0x2;
	const uint32_t DDPF_FOURCC = 0x4;
	const uint32_t DDPF_RGB = 0x40;
	const uint32_t DDPF_YUV = 0x200;
	const uint32_t DDPF_LUMINANCE = 0x20000;

	struct DDS_PixelFormat {
		uint32_t m_Flags;
		uint32_t m_Format;
		uint32_t m_BitCount;
		uint32_t m_RBitMask;
		uint32_t m_GBitMask;
		uint32_t m_BBitMask;
		uint32_t m_ABitMask;
	};

	struct DDS_Header {
		uint32_t m_Flags;
		uint32_t m_Height;
		uint32_t m_Width;
		uint32_t m_Pitch;
		uint32_t m_Depth;
		uint32_t m_MipmapCount;
		DDS_PixelFormat m_PixelFormat;
		uint32_t m_Caps1;
		uint32_t m_Caps2;
		uint32_t m_Caps3;
		uint32_t m_Caps4;
	};

	auto ReadPixelFormat = [](LWByteBuffer &Buf, DDS_PixelFormat &Format)->bool {
		uint32_t Size = Buf.Read<uint32_t>();
		if (Size != 32) return false;
		Format.m_Flags = Buf.Read<uint32_t>();
		Format.m_Format = Buf.Read<uint32_t>();
		Format.m_BitCount = Buf.Read<uint32_t>();
		Format.m_RBitMask = Buf.Read<uint32_t>();
		Format.m_GBitMask = Buf.Read<uint32_t>();
		Format.m_BBitMask = Buf.Read<uint32_t>();
		Format.m_ABitMask = Buf.Read<uint32_t>();
		return true;
	};

	auto ReadHeader = [&ReadPixelFormat](LWByteBuffer &Buf, DDS_Header &Header)->bool {
		uint32_t Size = Buf.Read<uint32_t>();
		if (Size != 124) return false;
		Header.m_Flags = Buf.Read<uint32_t>();
		Header.m_Height = Buf.Read<uint32_t>();
		Header.m_Width = Buf.Read<uint32_t>();
		Header.m_Pitch = Buf.Read<uint32_t>();
		Header.m_Depth = Buf.Read<uint32_t>();
		Header.m_MipmapCount = Buf.Read<uint32_t>();
		Buf.OffsetPosition(sizeof(uint32_t) * 11);
		if (!ReadPixelFormat(Buf, Header.m_PixelFormat)) return false;
		Header.m_Caps1 = Buf.Read<uint32_t>();
		Header.m_Caps2 = Buf.Read<uint32_t>();
		Header.m_Caps3 = Buf.Read<uint32_t>();
		Header.m_Caps4 = Buf.Read<uint32_t>();
		Buf.OffsetPosition(sizeof(uint32_t));
		return true;
	};
	uint32_t Header = Buffer.Read<uint32_t>();
	if (Header != MagicHeader) {
		fmt::print("dds has invalid header. {:#x} - {:#x}\n", Header, MagicHeader);
		return false;
	}
	DDS_Header dHeader;
	if (!ReadHeader(Buffer, dHeader)) return false;
	uint32_t PackType = -1;
	DDS_PixelFormat &PForm = dHeader.m_PixelFormat;
	if (PForm.m_Flags & DDPF_FOURCC) {
		if (PForm.m_Format == DXT1Header) PackType = BC1;
		else if (PForm.m_Format == DXT2Header) PackType = BC2;
		else if (PForm.m_Format == DXT3Header) PackType = BC2;
		else if (PForm.m_Format == DXT4Header) PackType = BC3;
		else if (PForm.m_Format == DXT5Header) PackType = BC3;
		else if (PForm.m_Format == DXT_ABGR32F) PackType = RGBA32F;
	} else {
		if ((PForm.m_Flags & (DDPF_RGB | DDPF_ALPHA)) != 0 && PForm.m_BitCount == 32) PackType = SRGBA;
		else if ((PForm.m_Flags & (DDPF_RGB)) != 0 && PForm.m_BitCount == 24) PackType = SRGBA;
		else if (PForm.m_Flags & (DDPF_LUMINANCE) && PForm.m_BitCount == 8) PackType = R8;
	}
	if (PackType == -1) return false;
	if (dHeader.m_Depth <= 1) Image = LWImage(LWVector2i(dHeader.m_Width, dHeader.m_Height), 1, PackType | (((dHeader.m_Caps2 & DDSCAPS2_CUBEMAP) != 0) ? LWImage::ImageCubeMap : 0), dHeader.m_MipmapCount ? (dHeader.m_MipmapCount - 1) : 0, nullptr, Allocator);
	else {
		if ((dHeader.m_Caps2 & DDSCAPS2_VOLUME) == 0) return false;
		Image = LWImage(LWVector3i(dHeader.m_Width, dHeader.m_Height, dHeader.m_Depth), 1, PackType, dHeader.m_MipmapCount ? (dHeader.m_MipmapCount - 1) : 0, nullptr, Allocator);
	}
	const int8_t *ReadBuffer = Buffer.GetReadBuffer()+Buffer.GetPosition();
	uint32_t  o = 0;
	uint32_t TexelCnt = (Image.GetMipmapCount() + 1);
	uint32_t LayerCnt = Image.GetLayers();

	bool HasAlpha = (PForm.m_Flags & DDPF_ALPHAPIXELS) != 0;
	uint32_t RIdx = PForm.m_RBitMask == 0xFF000000 ? 3 : (PForm.m_RBitMask == 0xFF0000) ? 2 : (PForm.m_RBitMask == 0xFF00 ? 1 : 0);
	uint32_t GIdx = PForm.m_GBitMask == 0xFF000000 ? 3 : (PForm.m_GBitMask == 0xFF0000) ? 2 : (PForm.m_GBitMask == 0xFF00 ? 1 : 0);
	uint32_t BIdx = PForm.m_BBitMask == 0xFF000000 ? 3 : (PForm.m_BBitMask == 0xFF0000) ? 2 : (PForm.m_BBitMask == 0xFF00 ? 1 : 0);
	uint32_t AIdx = 3;
	if(HasAlpha) AIdx = PForm.m_ABitMask == 0xFF000000 ? 3 : (PForm.m_ABitMask == 0xFF0000) ? 2 : (PForm.m_ABitMask == 0xFF00 ? 1 : 0);
	for (uint32_t d = 0; d < LayerCnt; d++) {
		for (uint32_t i = 0; i < TexelCnt; i++) {
			LWVector3i Size = i == 0 ? Image.GetSize3D() : Image.GetMipmapSize3D(i - 1);
			uint32_t Len = LWImage::GetLength3D(Size, PackType);
			uint8_t *Texels = Image.GetTexels(d, i);

			if (PackType == SRGBA) { //we need to potentially fix the rgb format if they are not layed out in the same order.
				bool RGB = PForm.m_BitCount == 24;
				if (RGB) Len = Size.x * Size.y * Size.z * 3;
				for (int32_t n = 0; n < Size.x * Size.y * Size.z; n++) {
					uint8_t *T = Texels + n * 4;
					const uint8_t *BT = (const uint8_t*)ReadBuffer + o + n * (RGB ? 3 : 4);
					T[0] = BT[RIdx];
					T[1] = BT[GIdx];
					T[2] = BT[BIdx];
					T[3] = RGB ? 0xFF : BT[AIdx];
				}
			} else if (PackType == RGBA32F && PForm.m_Format == DXT_ABGR32F) {
				//Need to fix the order to RGBA32
				bool RGB = PForm.m_BitCount == 96;
				if (RGB) Len = Size.x * Size.y * Size.z * 12;
				for (int32_t n = 0; n < Size.x * Size.y * Size.z; n++) {
					float *T = (float*)(Texels + n * 16);
					const float *BT = (const float*)(ReadBuffer + o + n * (RGB ? 12 : 16));
					//Temp patch for abgr32f images...
					T[0] = BT[0];
					T[1] = BT[1];
					T[2] = BT[2];
					T[3] = RGB ? 1.0f : BT[AIdx];
				}
			} else std::copy(ReadBuffer + o, ReadBuffer + o + Len, Texels);
			o += Len;
		}
	}
	Buffer.OffsetPosition(o);
	return true;
};

bool LWImage::LoadImageKTX2(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	LWByteBuffer Buf = LWByteBuffer((const int8_t*)MemBuffer, Stream.Length(), LWByteBuffer::BufferNotOwned);
	bool Result = LoadImageKTX2(Image, Buf, Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageKTX2(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	const uint32_t isETC1S = 0x1;
	const uint32_t isIFrame = 0x2;
	const uint32_t hasAlphaSlices = 0x4;
	struct KTXHeader {
		char m_Identifier[12];
		uint32_t m_vkFormat;
		uint32_t m_TypeSize;
		uint32_t m_PixelWidth;
		uint32_t m_PixelHeight;
		uint32_t m_PixelDepth;
		uint32_t m_LayerCount;
		uint32_t m_FaceCount;
		uint32_t m_LevelCount;
		uint32_t m_CompressionScheme;

		uint32_t m_dfOffset; //Dascriptor Field
		uint32_t m_dfLength;
		uint32_t m_kvOffset; //Key/Value
		uint32_t m_kvLength;
		uint64_t m_sgOffset; //SuperCompression Global data
		uint64_t m_sgLength;

		static bool Deserialize(KTXHeader &Header, LWByteBuffer &Buf) {
			const uint32_t IdentLen = 12;
			const char Ident[] = "«KTX 20»\r\n\x1A\n";
			Buf.Read<char>(Header.m_Identifier, IdentLen);
			if (!std::equal(Header.m_Identifier, Header.m_Identifier + IdentLen, Ident)) return false;
			Header.m_vkFormat = Buf.Read<uint32_t>();
			Header.m_TypeSize = Buf.Read<uint32_t>();
			Header.m_PixelWidth = Buf.Read<uint32_t>();
			Header.m_PixelHeight = Buf.Read<uint32_t>();
			Header.m_PixelDepth = Buf.Read<uint32_t>();
			Header.m_LayerCount = std::max<uint32_t>(1, Buf.Read<uint32_t>());
			Header.m_FaceCount = std::max<uint32_t>(1, Buf.Read<uint32_t>());
			Header.m_LevelCount = std::max<uint32_t>(1, Buf.Read<uint32_t>());
			Header.m_CompressionScheme = Buf.Read<uint32_t>();

			Header.m_dfOffset = Buf.Read<uint32_t>();
			Header.m_dfLength = Buf.Read<uint32_t>();
			Header.m_kvOffset = Buf.Read<uint32_t>();
			Header.m_kvLength = Buf.Read<uint32_t>();
			Header.m_sgOffset = Buf.Read<uint64_t>();
			Header.m_sgLength = Buf.Read<uint64_t>();
			return true;
		};

		uint32_t MapFormatToLWImage(void) {
			//Only a subset of VKFormats are supported, if an unsupported format is detected -1 is returned and the loading will fail.
			if (m_vkFormat == 13) return LWImage::R8; //VK_FORMAT_R8_UINT
			else if (m_vkFormat == 14) return LWImage::R8S; //VK_FORMAT_R8_SINT
			else if (m_vkFormat == 20) return LWImage::RG8; //VK_FORMAT_R8G8_UINT
			else if (m_vkFormat == 21) return LWImage::RG8S; //VK_FORMAT_R8G8_SINT
			else if (m_vkFormat == 41) return LWImage::RGBA8; //VK_FORMAT_R8G8B8A8_UINT
			else if (m_vkFormat == 42) return LWImage::RGBA8S; //VK_FORMAT_R8G8B8A8_SINT
			else if (m_vkFormat == 74) return LWImage::R16; //VK_FORMAT_R16_UINT
			else if (m_vkFormat == 75) return LWImage::R16S; //VK_FORMAT_R16_SINT
			else if (m_vkFormat == 81) return LWImage::RG16; //VK_FORMAT_R16G16_UINT
			else if (m_vkFormat == 82) return LWImage::RG16S; //VK_FORMAT_R16G16_SINT
			else if (m_vkFormat == 95) return LWImage::RGBA16; //VK_FORMAT_R16G16B16A16_UINT
			else if (m_vkFormat == 96) return LWImage::RGBA16S; //VK_FORMAT_R16G16B16A16_SINT
			else if (m_vkFormat == 98) return LWImage::R32; //VK_FORMAT_R32_UINT
			else if (m_vkFormat == 99) return LWImage::R32S; //VK_FORMAT_R32_SINT
			else if (m_vkFormat == 100) return LWImage::R32F; //VK_FORMAT_R32_SFLOAT
			else if (m_vkFormat == 101) return LWImage::RG32; //VK_FORMAT_R32G32_UINT
			else if (m_vkFormat == 102) return LWImage::RG32S; //VK_FORMAT_R32G32_SINT
			else if (m_vkFormat == 103) return LWImage::RG32F; //VK_FORMAT_R32G32_SFLOAT
			else if (m_vkFormat == 107) return LWImage::RGBA32; //VK_FORMAT_R32G32B32A32_UINT
			else if (m_vkFormat == 108) return LWImage::RGBA32S; //VK_FORMAT_R32G32B32A32_SINT
			else if (m_vkFormat == 109) return LWImage::RGBA32F; //VK_FORMAT_R32G32B32A32_SFLOAT
			else if (m_vkFormat == 124) return LWImage::DEPTH16; //VK_FORMAT_D16_UNORM
			else if (m_vkFormat == 126) return LWImage::DEPTH32; //VK_FORMAT_D32_SFLOAT
			else if (m_vkFormat == 129) return LWImage::DEPTH24STENCIL8; //VK_FORMAT_D24_UNORM_S8_UINT
			else if (m_vkFormat == 131) return LWImage::BC1; //VK_FORMAT_BC1_UNORM_BLOCK
			else if (m_vkFormat == 132) return LWImage::BC1_SRGB; //VK_FORMAT_BC1_SRGB_BLOCK
			else if (m_vkFormat == 135) return LWImage::BC2; //VK_FORMAT_BC2_UNORM_BLOCK
			else if (m_vkFormat == 136) return LWImage::BC2_SRGB; //VK_FORMAT_BC2_SRGB_BLOCK
			else if (m_vkFormat == 137) return LWImage::BC3; //VK_FORMAT_BC3_UNORM_BLOCK
			else if (m_vkFormat == 138) return LWImage::BC3_SRGB; //VK_FORMAT_BC3_SRGB_BLOCK
			else if (m_vkFormat == 139) return LWImage::BC4; //VK_FORMAT_BC4_UNORM_BLOCK
			else if (m_vkFormat == 141) return LWImage::BC5; //VK_FORMAT_BC5_UNORM_BLOCK
			else if (m_vkFormat == 142) return LWImage::BC5; //VK_FORMAT_BC5_SNORM_BLOCK
			else if (m_vkFormat == 143) return LWImage::BC6; //VK_FORMAT_BC6H_UFLOAT_BLOCK
			else if (m_vkFormat == 144) return LWImage::BC6; //VK_FORMAT_BC6H_SFLOAT_BLOCK
			else if (m_vkFormat == 145) return LWImage::BC7; //VK_FORMAT_BC7_UNORM_BLOCK
			else if (m_vkFormat == 146) return LWImage::BC7_SRGB; //VK_FORMAT_BC7_SRGB_BLOCK
			return -1;
		}
	};

	struct KTXImageDesc {
		uint32_t m_Flag;
		uint32_t m_rgbSliceOffset;
		uint32_t m_rgbSliceLength;
		uint32_t m_alphaSliceOffset;
		uint32_t m_alphaSliceLength;

		static bool Deserialize(KTXImageDesc &Desc, LWByteBuffer &Buf) {
			Desc.m_Flag = Buf.Read<uint32_t>();
			Desc.m_rgbSliceOffset = Buf.Read<uint32_t>();
			Desc.m_rgbSliceLength = Buf.Read<uint32_t>();
			Desc.m_alphaSliceOffset = Buf.Read<uint32_t>();
			Desc.m_alphaSliceLength = Buf.Read<uint32_t>();
			return true;
		};

		//Copy 8 bit texels.
		void CopyTexels(LWVector2i ImgSize, uint8_t *TargetTexels, const uint8_t *Buffer) {
			const uint8_t *rgbTexels = (const uint8_t*)Buffer + m_rgbSliceOffset;
			const uint8_t *alphaTexels = (const uint8_t*)Buffer + m_alphaSliceOffset;
			for (int32_t i = 0; i < ImgSize.x * ImgSize.y; i++) {
				uint8_t *T = TargetTexels + i * 4;
				const uint8_t *rgb = rgbTexels + i * 3;
				const uint8_t *a = rgbTexels + i;
				T[0] = rgb[0];
				T[1] = rgb[1];
				T[2] = rgb[2];
				T[3] = a[0];
			}
		};

		//Copy 16 bit texels.
		void CopyTexels(LWVector2i ImgSize, uint16_t *TargetTexels, const uint8_t *Buffer) {
			const uint16_t *rgbTexels = (const uint16_t*)Buffer + m_rgbSliceOffset;
			const uint16_t *alphaTexels = (const uint16_t*)Buffer + m_alphaSliceOffset;
			for (int32_t i = 0; i < ImgSize.x * ImgSize.y; i++) {
				uint16_t *T = TargetTexels + i * 4;
				const uint16_t *rgb = rgbTexels + i * 3;
				const uint16_t *a = rgbTexels + i;
				T[0] = rgb[0];
				T[1] = rgb[1];
				T[2] = rgb[2];
				T[3] = a[0];
			}
		};

		//Copy 32 bit texels.
		void CopyTexels(LWVector2i ImgSize, uint32_t *TargetTexels, const uint8_t *Buffer) {
			const uint32_t *rgbTexels = (const uint32_t*)Buffer + m_rgbSliceOffset;
			const uint32_t *alphaTexels = (const uint32_t*)Buffer + m_alphaSliceOffset;
			for (int32_t i = 0; i < ImgSize.x * ImgSize.y; i++) {
				uint32_t *T = TargetTexels + i * 4;
				const uint32_t *rgb = rgbTexels + i * 3;
				const uint32_t *a = rgbTexels + i;
				T[0] = rgb[0];
				T[1] = rgb[1];
				T[2] = rgb[2];
				T[3] = a[0];
			}
		};
	};

	struct KTXGlobalData {
		
		uint32_t m_Flag;
		uint16_t m_EndpointCount;
		uint16_t m_SelectorCount;
		uint32_t m_EndpointLength;
		uint32_t m_SelectorLength;
		uint32_t m_TableLength;
		uint32_t m_ExtenededLength;

		KTXImageDesc *m_ImageDescriptors = nullptr;
		const int8_t *m_Endpoints = nullptr;
		const int8_t *m_Selectors = nullptr;
		const int8_t *m_Tables = nullptr;
		const int8_t *m_Extended = nullptr;

		static bool Deserialize(KTXGlobalData &GD, const KTXHeader &Header, LWByteBuffer &Buf, LWAllocator &Allocator) {
			Buf.SetPosition(Header.m_dfOffset);
			GD.m_Flag = Buf.Read<uint32_t>();
			GD.m_EndpointCount = Buf.Read<uint16_t>();
			GD.m_SelectorCount = Buf.Read<uint16_t>();
			GD.m_EndpointLength = Buf.Read<uint32_t>();
			GD.m_SelectorLength = Buf.Read<uint32_t>();
			GD.m_TableLength = Buf.Read<uint32_t>();
			GD.m_ExtenededLength = Buf.Read<uint32_t>();
			uint32_t DescCnt = Header.m_LevelCount * Header.m_LayerCount * Header.m_FaceCount;
			GD.m_ImageDescriptors = Allocator.Allocate<KTXImageDesc>(DescCnt);
			for (uint32_t i = 0; i < DescCnt; i++) {
				if (!KTXImageDesc::Deserialize(GD.m_ImageDescriptors[i], Buf)) return false;
			}
			GD.m_Endpoints = Buf.GetReadBuffer() + Buf.GetPosition();
			Buf.OffsetPosition(GD.m_EndpointLength);
			GD.m_Selectors = Buf.GetReadBuffer() + Buf.GetPosition();
			Buf.OffsetPosition(GD.m_SelectorLength);
			GD.m_Tables = Buf.GetReadBuffer() + Buf.GetPosition();
			Buf.OffsetPosition(GD.m_TableLength);
			GD.m_Extended = Buf.GetReadBuffer() + Buf.GetPosition();
			return true;
		};

		~KTXGlobalData() {
			LWAllocator::Destroy(m_ImageDescriptors);
		};
		
	};

	KTXHeader Header;
	KTXGlobalData GlobalData;
	int8_t *WriteBuffer = nullptr;
	uint32_t o = Buffer.WriteStorage(0, WriteBuffer);
	if (!KTXHeader::Deserialize(Header, Buffer)) return false;
	uint32_t IFmt = Header.MapFormatToLWImage();
	if (IFmt == -1) return false;
	if (Header.m_CompressionScheme != 0) return false; //Don't support supercompression at the moment.
	if (Header.m_LayerCount != 1) return false;//Don't support layer image's at the moment.
	if (!KTXGlobalData::Deserialize(GlobalData, Header, Buffer, Allocator)) return false;
	if (Header.m_FaceCount > 1) {
		Image = LWImage(LWVector2i(Header.m_PixelWidth, Header.m_PixelHeight), 1, IFmt | LWImage::ImageCubeMap, Header.m_LevelCount, nullptr, Allocator);
	} else {
		Image = LWImage(LWVector2i(Header.m_PixelWidth, Header.m_PixelHeight), 1, IFmt, Header.m_LevelCount, nullptr, Allocator);
	}
	bool hasAlpha = (GlobalData.m_Flag & hasAlphaSlices) != 0;
	for (uint32_t lv = 0; lv < Header.m_LevelCount; lv++) {
		for(uint32_t la =0;la<Header.m_LayerCount;la++){
			for (uint32_t f = 0; f < Header.m_FaceCount; f++) {
				uint32_t rlv = Header.m_LevelCount - (lv + 1); //KTX2 stores miplevels in smallest to largest order.
				KTXImageDesc &KImg = GlobalData.m_ImageDescriptors[f + la * Header.m_FaceCount + lv * Header.m_FaceCount * Header.m_LayerCount];
				uint8_t *Texels = Image.GetTexels(f * Header.m_LevelCount + rlv);
				LWVector2i TSize = LWImage::MipmapSize2D(LWVector2i(Header.m_PixelWidth, Header.m_PixelHeight), rlv);
				uint32_t PackSize = LWImage::GetBitSize(IFmt) / 8;

				if (LWImage::CompressedType(IFmt) || LWImage::DepthType(IFmt) || !hasAlpha) {
					const uint8_t *SrcTexels = (const uint8_t*)WriteBuffer + KImg.m_rgbSliceOffset;
					std::copy(SrcTexels, SrcTexels+KImg.m_rgbSliceLength, Texels);
				} else if (hasAlpha) { //Assumed Alphaformat is rgba format.
					if (PackSize == 32) KImg.CopyTexels(TSize, Texels, (uint8_t*)WriteBuffer); //8 bit rgba format
					else if (PackSize == 64) KImg.CopyTexels(TSize, (uint16_t*)Texels, (uint8_t*)WriteBuffer); //16
					else if (PackSize == 128) KImg.CopyTexels(TSize, (uint32_t*)Texels, (uint8_t*)WriteBuffer); //32 bit rgba format.
				} else return false;
				Buffer.OffsetPosition(KImg.m_rgbSliceLength + KImg.m_alphaSliceLength);
			}
		}
	}

	return true;
}

bool LWImage::SaveImagePNG(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
		fmt::print("Error opening file to save png: '{}'\n", FilePath);
		return false;
	}
	LWByteBuffer BB;
	uint32_t Len = SaveImagePNG(Image, BB);
	if (!Len) return false;
	uint8_t *Buf = Allocator.Allocate<uint8_t>(Len);
	BB = LWByteBuffer((int8_t*)Buf, Len, LWByteBuffer::BufferNotOwned);
	if (SaveImagePNG(Image, BB)!=Len) {
		fmt::print("Image changed size.\n");
		LWAllocator::Destroy(Buf);
		return false;
	}
	Stream.Write((char*)Buf, Len);
	LWAllocator::Destroy(Buf);
	return true;
}

uint32_t LWImage::SaveImagePNG(LWImage &Image, LWByteBuffer &Buf) {
	const uint32_t RGB8 = 0xFE;
	struct Writer {
		LWByteBuffer &Buf;
		uint32_t BytesWritten;
	};

	Writer W = { Buf, 0 };
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) return W.BytesWritten;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, nullptr);
		return W.BytesWritten;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return W.BytesWritten;
	}
	auto WriteFunc = [](png_structp png_ptr, png_bytep data, png_size_t length) {
		Writer *W = (Writer*)png_get_io_ptr(png_ptr);
		W->BytesWritten += W->Buf.Write<unsigned char>((uint32_t)length, data);
		return;
	};
	
	LWVector2i Size = Image.GetSize2D();
	png_set_write_fn(png_ptr, (void*)&W, WriteFunc, nullptr);
	png_set_IHDR(png_ptr, info_ptr, Size.x, Size.y, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	uint8_t *p = Image.GetTexels(0);
	uint32_t Stride = LWImage::GetStride(Size.x, Image.GetPackType());
	for (int32_t y = 0; y < Size.y; y++) {
		png_write_row(png_ptr, p + Stride * y);
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return W.BytesWritten;
}

bool LWImage::SaveImageDDS(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
		fmt::print("Error opening file to save dds: '{}'\n", FilePath);
		return false;
	}
	LWByteBuffer BB;
	uint32_t Len = SaveImageDDS(Image, BB);
	if (!Len) return false;
	uint8_t *Buf = Allocator.Allocate<uint8_t>(Len);
	BB = LWByteBuffer((int8_t*)Buf, Len, LWByteBuffer::BufferNotOwned);
	if (SaveImageDDS(Image, BB) != Len) {
		fmt::print("Image has changed size.\n");
		LWAllocator::Destroy(Buf);
		return false;
	}
	Stream.Write((char*)Buf, Len);
	LWAllocator::Destroy(Buf);
	return true;
}

uint32_t LWImage::SaveImageDDS(LWImage &Image, LWByteBuffer &Buffer) {
	const uint32_t MagicHeader = 0x20534444;
	const uint32_t DX10Header = 0x30315844;
	const uint32_t DXT1Header = 0x31545844;
	const uint32_t DXT2Header = 0x32545844;
	const uint32_t DXT3Header = 0x33545844;
	const uint32_t DXT4Header = 0x34545844;
	const uint32_t DXT5Header = 0x35545844;
	const uint32_t DXT6Header = 0x36545844;
	const uint32_t DXT7Header = 0x37545844;

	const uint32_t DXT_ABGR32F = 116; //D3DFormat::D3DFMT_A32B32G32R32F, support for HDR images.


	const uint32_t DDSCAPS_COMPLEX = 0x8;
	const uint32_t DDSCAPS_TEXTURE = 0x1000;
	const uint32_t DDSCAPS_MIPMAP = 0x400000;

	const uint32_t DDSCAPS2_CUBEMAP = 0x200;
	const uint32_t DDSCAPS2_VOLUME = 0x200000;
	const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
	const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
	const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
	const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
	const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
	const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;

	const uint32_t DDSD_CAPS = 0x1;
	const uint32_t DDSD_HEIGHT = 0x2;
	const uint32_t DDSD_WIDTH = 0x4;
	const uint32_t DDSD_PITCH = 0x8;
	const uint32_t DDSD_PIXELFORMAT = 0x1000;
	const uint32_t DDSD_MIPMAPCOUNT = 0x20000;
	const uint32_t DDSD_LINEARSIZE = 0x80000;
	const uint32_t DDSD_DEPTH = 0x800000;

	const uint32_t DDPF_ALPHAPIXELS = 0x1;
	const uint32_t DDPF_ALPHA = 0x2;
	const uint32_t DDPF_FOURCC = 0x4;
	const uint32_t DDPF_RGB = 0x40;
	const uint32_t DDPF_YUV = 0x200;
	const uint32_t DDPF_LUMINANCE = 0x20000;

	struct DDS_PixelFormat {
		uint32_t m_Flags;
		uint32_t m_Format;
		uint32_t m_BitCount;
		uint32_t m_RBitMask;
		uint32_t m_GBitMask;
		uint32_t m_BBitMask;
		uint32_t m_ABitMask;
	};

	struct DDS_Header {
		uint32_t m_Flags;
		uint32_t m_Height;
		uint32_t m_Width;
		uint32_t m_Pitch;
		uint32_t m_Depth;
		uint32_t m_MipmapCount;
		DDS_PixelFormat m_PixelFormat;
		uint32_t m_Caps1;
		uint32_t m_Caps2;
		uint32_t m_Caps3;
		uint32_t m_Caps4;
	};

	auto WritePixelFormat = [](LWByteBuffer &Buf, DDS_PixelFormat &Format)->uint32_t {
		const uint32_t FormatStructSize = 32;
		uint32_t o = 0;
		o += Buf.Write<uint32_t>(FormatStructSize);
		o += Buf.Write<uint32_t>(Format.m_Flags);
		o += Buf.Write<uint32_t>(Format.m_Format);
		o += Buf.Write<uint32_t>(Format.m_BitCount);
		o += Buf.Write<uint32_t>(Format.m_RBitMask);
		o += Buf.Write<uint32_t>(Format.m_GBitMask);
		o += Buf.Write<uint32_t>(Format.m_BBitMask);
		o += Buf.Write<uint32_t>(Format.m_ABitMask);
		return o;
	};

	auto WriteHeader = [&WritePixelFormat](LWByteBuffer &Buf, DDS_Header &Header)->uint32_t {
		const uint32_t HeaderStructSize = 124;
		uint32_t Reserved[11] = {};
		uint32_t o = 0;
		o += Buf.Write<uint32_t>(HeaderStructSize);
		o += Buf.Write<uint32_t>(Header.m_Flags);
		o += Buf.Write<uint32_t>(Header.m_Height);
		o += Buf.Write<uint32_t>(Header.m_Width);
		o += Buf.Write<uint32_t>(Header.m_Pitch);
		o += Buf.Write<uint32_t>(Header.m_Depth);
		o += Buf.Write<uint32_t>(Header.m_MipmapCount);
		o += Buf.Write<uint32_t>(11, Reserved); 
		o += WritePixelFormat(Buf, Header.m_PixelFormat);
		o += Buf.Write<uint32_t>(Header.m_Caps1);
		o += Buf.Write<uint32_t>(Header.m_Caps2);
		o += Buf.Write<uint32_t>(Header.m_Caps3);
		o += Buf.Write<uint32_t>(Header.m_Caps4);
		o += Buf.Write<uint32_t>(0); //Reserved.
		return o;
	};

	uint32_t PackType = Image.GetPackType();
	LWVector3i Size = Image.GetSize3D();
	uint32_t Stride = GetStride(Size.x, PackType);
	uint32_t Mipmaps = Image.GetMipmapCount();
	uint32_t iType = Image.GetType();
	bool IsCompressed = CompressedType(PackType);
	bool IsDepth = DepthType(PackType);
	bool IsStencil = StencilType(PackType);
	if (IsDepth || IsStencil) return 0; //Unsupported formats for saving.

	DDS_Header dHeader;
	DDS_PixelFormat &pFormat = dHeader.m_PixelFormat;
	dHeader.m_Flags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | (IsCompressed ? DDSD_LINEARSIZE : DDSD_PITCH) | (Mipmaps>0?DDSD_MIPMAPCOUNT:0) | ((iType==Image3D || (iType==Image2D && Size.z>1)) ? DDSD_DEPTH : 0);
	dHeader.m_Width = Size.x;
	dHeader.m_Height = Size.y;
	dHeader.m_Pitch = IsCompressed ? GetLength3D(Size, PackType):Stride;
	dHeader.m_Depth = Size.z;
	dHeader.m_MipmapCount = Mipmaps>0?Mipmaps+1:0;
	pFormat.m_Flags = 0;
	if (IsCompressed) {
		pFormat.m_Flags |= DDPF_FOURCC;
		if (PackType == BC1) pFormat.m_Format = DXT1Header;
		else if (PackType == BC2) pFormat.m_Format = DXT3Header;
		else if (PackType == BC3) pFormat.m_Format = DXT5Header;

	} else {
		if (PackType == RGBA8 || PackType == RGBA8S || PackType==SRGBA) {
			pFormat.m_Flags |= DDPF_RGB | DDPF_ALPHA | DDPF_ALPHAPIXELS;
			pFormat.m_RBitMask = 0xFF;
			pFormat.m_GBitMask = 0xFF00;
			pFormat.m_BBitMask = 0xFF0000;
			pFormat.m_ABitMask = 0xFF000000;
			pFormat.m_BitCount = 32;
		} else if (PackType == R8 || PackType == R8S) {
			pFormat.m_Flags |= DDPF_LUMINANCE;
			pFormat.m_ABitMask = 0xFF;
			pFormat.m_BitCount = 8;
		}
	}
	dHeader.m_Caps1 = DDSCAPS_TEXTURE | (Mipmaps > 0 ? DDSCAPS_MIPMAP : 0) | (((iType == ImageCubeMap || iType == Image3D) || (iType == Image2D && Size.z > 1)) ? DDSCAPS_COMPLEX : 0);
	dHeader.m_Caps2 = (iType == ImageCubeMap ? (DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_NEGATIVEZ | DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ) : 0) | (iType == Image3D ? DDSCAPS2_VOLUME : 0);
	uint32_t o = 0;
	o += Buffer.Write<uint32_t>(MagicHeader);
	o += WriteHeader(Buffer, dHeader);
	
	uint32_t FaceCount = iType == ImageCubeMap ? 6 : 1;
	uint32_t LayerCount = Size.z;
	uint32_t TexelCnt = Mipmaps + 1;
	for (uint32_t i = 0; i < FaceCount * LayerCount; i++) {
		for (uint32_t m = 0; m < TexelCnt; m++) {
			LWVector3i iSize = m == 0 ? Size : Image.GetMipmapSize3D(m - 1);
			uint32_t Len = GetLength3D(iSize, PackType);
			uint8_t *Texels = Image.GetTexels(m + i * TexelCnt);
			o += Buffer.Write<uint8_t>(Len, Texels);
		}
	}
	return o;
}

uint32_t LWImage::RGBA8toBC(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat) {
	const uint32_t DXTBlockSize[] = { 8, 8, 16,16, 16,16, 8,8, 16,16, 16,16, 16,16};
	if (!InTexels) return GetLength2D(TexelsSize, BCFormat);

	uint32_t Stride = TexelsSize.x * sizeof(uint32_t);
	uint32_t dBlockSize = DXTBlockSize[BCFormat - BC1];
	uint32_t BlocksWidth = TexelsSize.x / 4;
	uint32_t BlocksHeight = TexelsSize.y / 4;

	for (uint32_t y = 0; y < BlocksHeight; y++) {
		for (uint32_t x = 0; x < BlocksWidth; x++) {
			const uint8_t *InBlock = InTexels + (x * 16 + y * 4 * Stride);
			uint8_t *OutBlock = OutTexels + x * dBlockSize + y * BlocksWidth * dBlockSize;
			if (BCFormat == BC1)	CompressBlockBC1((const unsigned char*)InBlock, Stride, (unsigned char*)OutBlock);
			else if (BCFormat == BC2) CompressBlockBC2((const unsigned char*)InBlock, Stride, (unsigned char*)OutBlock);
			else if (BCFormat == BC3) CompressBlockBC3((const unsigned char*)InBlock, Stride, (unsigned char*)OutBlock);
		}
	}
	return BlocksWidth * BlocksHeight * 8;
}

uint32_t LWImage::BCtoRGBA8(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat) {
	unsigned char TempBlock[64];
	const uint32_t DXTBlockSize[] = { 8, 8, 16,16, 16,16, 8,8, 16,16, 16,16, 16,16 };
	if (!InTexels) return TexelsSize.x * TexelsSize.y * 4;
	uint32_t Stride = TexelsSize.x * sizeof(uint32_t);
	uint32_t dBlockSize = DXTBlockSize[BCFormat - BC1];
	uint32_t BlocksWidth = TexelsSize.x / 4;
	uint32_t BlocksHeight = TexelsSize.y / 4;
	for (uint32_t y = 0; y < BlocksHeight; y++) {
		for (uint32_t x = 0; x < BlocksWidth; x++) {
			const uint8_t *InBlock = InTexels + x * dBlockSize + y * BlocksWidth * dBlockSize;
			uint8_t *OutBlock = OutTexels + (x * 16 + y * 4 * Stride);
			if (BCFormat == BC1) DecompressBlockBC1((const unsigned char*)InBlock, TempBlock);
			else if (BCFormat == BC2) DecompressBlockBC2((const unsigned char*)InBlock, TempBlock);
			else if (BCFormat == BC3) DecompressBlockBC3((const unsigned char*)InBlock, TempBlock);
			for (uint32_t oy = 0; oy < 4; oy++) {
				for (uint32_t ox = 0; ox < 4; ox++) {
					uint8_t *InTexels = TempBlock + ox * 4 + oy * 16;
					uint8_t *OutTexels = OutBlock + ox * 4 + oy * Stride;
					OutTexels[0] = InTexels[0];
					OutTexels[1] = InTexels[1];
					OutTexels[2] = InTexels[2];
					OutTexels[3] = InTexels[3];
				}
			}
		}
	}
	return TexelsSize.x * TexelsSize.y * 4;
}

template<class Type>
uint32_t WriteWeightedTexel(uint32_t TexelCnt, const Type **TexelPtr, const float *Weights, uint32_t ComponentCnt, Type *Buffer) {
	float c[4];
	for (uint32_t i = 0; i < TexelCnt; i++) {
		for (uint32_t n = 0; n < ComponentCnt; n++) {
			c[n] = ((i == 0) ? 0.0f : c[n]) + (float)TexelPtr[i][n] * Weights[i];
		}
	}
	
	for (uint32_t n = 0; n < ComponentCnt; n++) {
		Buffer[n] = (Type)c[n];
	}
	return sizeof(Type)*ComponentCnt;
};

uint32_t LWImage::CopyWeightedTexels(uint32_t PackType, uint32_t TexelCnt, const uint8_t **TexelPtrs, const float *TexelWeights, uint8_t *Buffer) {
	if (!Buffer) return GetBitSize(PackType) / 8;
	if (PackType == SRGBA) return WriteWeightedTexel<uint8_t>(TexelCnt, TexelPtrs, TexelWeights, 4, Buffer);
	else if (PackType == RGBA8) return WriteWeightedTexel<uint8_t>(TexelCnt, TexelPtrs, TexelWeights, 4, Buffer);
	else if (PackType == RGBA8S) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 4, (int8_t*)Buffer);
	else if (PackType == RGBA16) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 4, (uint16_t*)Buffer);
	else if (PackType == RGBA16S) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 4, (int16_t*)Buffer);
	else if (PackType == RGBA32) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 4, (uint32_t*)Buffer);
	else if (PackType == RGBA32S) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 4, (int32_t*)Buffer);
	else if (PackType == RGBA32F) return WriteWeightedTexel<float>(TexelCnt, (const float**)TexelPtrs, TexelWeights, 4, (float*)Buffer);
	else if (PackType == RG8) return WriteWeightedTexel<uint8_t>(TexelCnt, (const uint8_t**)TexelPtrs, TexelWeights, 2, (uint8_t*)Buffer);
	else if (PackType == RG8S) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 2, (int8_t*)Buffer);
	else if (PackType == RG16) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 2, (uint16_t*)Buffer);
	else if (PackType == RG16S) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 2, (int16_t*)Buffer);
	else if (PackType == RG32) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 2, (uint32_t*)Buffer);
	else if (PackType == RG32S) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 2, (int32_t*)Buffer);
	else if (PackType == RG32F) return WriteWeightedTexel<float>(TexelCnt, (const float**)TexelPtrs, TexelWeights, 2, (float*)Buffer);
	else if (PackType == R8) return WriteWeightedTexel<uint8_t>(TexelCnt, (const uint8_t**)TexelPtrs, TexelWeights, 1, (uint8_t*)Buffer);
	else if (PackType == R8S) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 1, (int8_t*)Buffer);
	else if (PackType == R16) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 1, (uint16_t*)Buffer);
	else if (PackType == R16S) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 1, (int16_t*)Buffer);
	else if (PackType == R32) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 1, (uint32_t*)Buffer);
	else if (PackType == R32S) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 1, (int32_t*)Buffer);
	else if (PackType == R32F) return WriteWeightedTexel<float>(TexelCnt, (const float**)TexelPtrs, TexelWeights, 1, (float*)Buffer);
	return 0;

}

uint32_t LWImage::SampleNearest1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	const uint8_t *T = Texels + ((uint32_t)(Length*u))*ByteSize;
	float w = 1.0f;
	return CopyWeightedTexels(PackType, 1, &T, &w, Buffer);
}

uint32_t LWImage::SampleLinear1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	float iLen = (1.0f / (float)Length)*0.5f;
	float Min = std::min<float>(std::max<float>(u - iLen, 0.0f), 1.0f-iLen);
	float Max = std::min<float>(std::max<float>(u + iLen, 0.0f), 1.0f-iLen);
	const uint8_t *T[2] = {
		Texels + ((uint32_t)(Length*Min))*ByteSize,
		Texels + ((uint32_t)(Length*Max))*ByteSize
	};
	float Ws[2] = { 0.5f, 0.5f };
	return CopyWeightedTexels(PackType, 2, T, Ws, Buffer);
}

uint32_t LWImage::SampleNearest2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	uint32_t Stride = Size.x*ByteSize;
	const uint8_t *T = Texels + (((uint32_t)(Size.x*UV.x))*ByteSize + ((uint32_t)(Size.y*UV.y))*Stride);
	float w = 1.0f;
	return CopyWeightedTexels(PackType, 1, &T, &w, Buffer);
}

uint32_t LWImage::SampleLinear2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	uint32_t Stride = Size.x*ByteSize;
	LWVector2f S = (1.0f / Size.CastTo<float>())*0.45f;
	LWVector2f Min = UV - S;
	LWVector2f Max = UV + S;
	Min = LWVector2f(std::min<float>(std::max<float>(Min.x, 0.0f), 1.0f-S.x), std::min<float>(std::max<float>(Min.y, 0.0f), 1.0f-S.y));
	Max = LWVector2f(std::min<float>(std::max<float>(Max.x, 0.0f), 1.0f-S.x), std::min<float>(std::max<float>(Max.y, 0.0f), 1.0f-S.y));
	const uint8_t *T[4] = {
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride,
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride
	};
	float Ws[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
	return CopyWeightedTexels(PackType, 4, T, Ws, Buffer);
}

uint32_t LWImage::SampleNearest3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, const LWVector3f &UVR, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	uint32_t Stride = Size.x*ByteSize;
	uint32_t DStride = Size.y * Stride;
	const uint8_t *T = Texels + ((uint32_t)(Size.x*UVR.x))*ByteSize + ((uint32_t)(Size.y*UVR.y))*Stride + ((uint32_t)(Size.z*UVR.z))*DStride;
	float w = 1.0f;
	return CopyWeightedTexels(PackType, 1, &T, &w, Buffer);
}

uint32_t LWImage::SampleLinear3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, const LWVector3f &UVR, uint8_t *Buffer) {
	uint32_t BitSize = GetBitSize(PackType);
	uint32_t ByteSize = BitSize / 8;
	uint32_t Stride = Size.x*ByteSize;
	uint32_t DStride = Size.y*Stride;
	LWVector3f S = (1.0f / Size.CastTo<float>())*0.5f;
	LWVector3f Min = UVR - S;
	LWVector3f Max = UVR + S;
	Min = LWVector3f(std::min<float>(std::max<float>(Min.x, 0.0f), 1.0f - S.x), std::min<float>(std::max<float>(Min.y, 0.0f), 1.0f - S.y), std::min<float>(std::max<float>(Min.z, 0.0f), 1.0f-S.z));
	Max = LWVector3f(std::min<float>(std::max<float>(Max.x, 0.0f), 1.0f - S.x), std::min<float>(std::max<float>(Max.y, 0.0f), 1.0f - S.y), std::min<float>(std::max<float>(Max.z, 0.0f), 1.0f-S.z));

	const uint8_t *T[8] = {
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride + ((uint32_t)(Size.z*Min.z))*DStride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride + ((uint32_t)(Size.z*Min.z))*DStride,
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride + ((uint32_t)(Size.z*Min.z))*DStride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride + ((uint32_t)(Size.z*Min.z))*DStride,
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride + ((uint32_t)(Size.z*Max.z))*DStride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Min.y))*Stride + ((uint32_t)(Size.z*Max.z))*DStride,
		Texels + ((uint32_t)(Size.x*Min.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride + ((uint32_t)(Size.z*Max.z))*DStride,
		Texels + ((uint32_t)(Size.x*Max.x))*ByteSize + ((uint32_t)(Size.y*Max.y))*Stride + ((uint32_t)(Size.z*Max.z))*DStride
	};
	float Ws[8] = { 0.125f, 0.125f, 0.125f, 0.125f, 
					0.125f, 0.125f, 0.125f, 0.125f };
	return CopyWeightedTexels(PackType, 8, T, Ws, Buffer);
}

uint32_t LWImage::MakeMipmapLevel1D(const uint8_t *Texels, uint32_t Width, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode) {
	if (PackType > R32F) return 0;
	uint32_t ByteSize = GetBitSize(PackType)/8;
	uint32_t w = MipmapSize1D(Width, MipmapLevel);	
	float iw = 1.0f / (float)w;
	if (Buffer) {
		uint32_t o = 0;
		for (uint32_t i = 0; i < w; i++) {
			if (SampleMode == NearestFilter) o += SampleNearest1D(Texels, Width, PackType, i*iw + iw * 0.5f, Buffer + o);
			else if (SampleMode == LinearFilter) o += SampleLinear1D(Texels, Width, PackType, i*iw + iw * 0.5f, Buffer + o);
		}
	}
	return w*ByteSize;
}

uint32_t LWImage::MakeMipmapLevel2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode) {
	if (PackType > R32F) return 0;
	uint32_t ByteSize = GetBitSize(PackType) / 8;
	LWVector2i S = MipmapSize2D(Size, MipmapLevel);
	LWVector2f iS = 1.0f / S.CastTo<float>();
	
	if (Buffer) {
		uint32_t o = 0;
		for (int32_t y = 0; y < S.y; y++) {
			for (int32_t x = 0; x < S.x; x++) {
				LWVector2f P = LWVector2f((float)x, (float)y)*iS + iS * 0.5f;
				if (SampleMode == NearestFilter) o += SampleNearest2D(Texels, Size, PackType, P, Buffer + o);
				else if (SampleMode == LinearFilter) o += SampleLinear2D(Texels, Size, PackType, P, Buffer + o);
			}
		}
	}
	return S.y * S.x*ByteSize;
}

uint32_t LWImage::MakeMipmapLevel3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode) {
	if (PackType > R32F) return 0;
	uint32_t ByteSize = GetBitSize(PackType) / 8;
	LWVector3i S = MipmapSize3D(Size, MipmapLevel);
	LWVector3f iS = 1.0f / S.CastTo<float>();	
	if (Buffer) {
		uint32_t o = 0;
		for (int32_t z = 0; z < S.z; z++) {
			for (int32_t y = 0; y < S.y; y++) {
				for (int32_t x = 0; x < S.x; x++) {
					LWVector3f P = LWVector3f((float)x, (float)y, (float)z)*iS + iS * 0.5f;
					if (SampleMode == NearestFilter) o += SampleNearest3D(Texels, Size, PackType, P, Buffer + o);
					else if (SampleMode == LinearFilter) o += SampleLinear3D(Texels, Size, PackType, P, Buffer + o);
				}
			}
		}
	}
	return S.z * (S.y*S.x*ByteSize);
}

uint32_t LWImage::MipmapCount(uint32_t Size){
	return (uint32_t)(log((Size)) / log(2)); //some platforms don't seem to have log2 
}

uint32_t LWImage::MipmapCount(const LWVector2i &Size){
	return MipmapCount(std::max<int32_t>(Size.x, Size.y));
}

uint32_t LWImage::MipmapCount(const LWVector3i &Size){
	return MipmapCount(std::max<int32_t>(std::max<int32_t>(Size.x, Size.y), Size.z));
}

uint32_t LWImage::MipmapSize1D(uint32_t SrcLen, uint32_t MipmapLevel){
	return (1u << MipmapLevel) >= SrcLen ? 1u : ((SrcLen) >> MipmapLevel);
}

LWVector2i LWImage::MipmapSize2D(const LWVector2i &SrcSize, uint32_t MipmapLevel) {
	int32_t MaxSize = 1 << MipmapLevel;
	return LWVector2i(
		SrcSize.x < MaxSize ? 1 : (((uint32_t)SrcSize.x) >> MipmapLevel),
		SrcSize.y < MaxSize ? 1 : (((uint32_t)SrcSize.y) >> MipmapLevel));
}

LWVector3i LWImage::MipmapSize3D(const LWVector3i &SrcSize, uint32_t MipmapLevel) {
	int32_t MaxSize = 1 << MipmapLevel;
	return LWVector3i(
		SrcSize.x < MaxSize ? 1 : (((uint32_t)SrcSize.x) >> MipmapLevel),
		SrcSize.y < MaxSize ? 1 : (((uint32_t)SrcSize.y) >> MipmapLevel),
		SrcSize.z < MaxSize ? 1 : (((uint32_t)SrcSize.z) >> MipmapLevel));
}


LWImage &LWImage::BuildMipmaps(uint32_t SampleMode) {
	uint32_t PackType = GetPackType();
	uint32_t iType = GetType();
	if (CompressedType(PackType)) return *this;
	if (m_MipmapCount != 0) return *this;
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_Texels[0]);
	if (!Alloc) return *this;
	m_MipmapCount = MipmapCount(m_Size.xyz());
	uint32_t TextureCnt = (m_MipmapCount + 1);
	for (int32_t i = m_Size.w - 1; i >= 0; i--) m_Texels[TextureCnt * i] = m_Texels[i]; //Reverse update all 0th level texel's.
	for (int32_t i = 0; i < m_Size.w; i++) {
		LWVector3i SrcSize = m_Size.xyz();
		for (uint32_t m = 1; m < m_MipmapCount; m++) {
			LWVector3i Size = GetMipmapSize3D(m - 1);
			uint32_t Len = GetLength3D(Size, PackType);
			m_Texels[i * TextureCnt + m] = Alloc->Allocate<uint8_t>(Len);
			if (iType == Image1D) MakeMipmapLevel1D(m_Texels[i * TextureCnt], SrcSize.x, PackType, m, m_Texels[i * TextureCnt + m], SampleMode);
			else if (iType == Image2D) MakeMipmapLevel2D(m_Texels[i * TextureCnt], SrcSize.xy(), PackType, m, m_Texels[i * TextureCnt + m], SampleMode);
			else if (iType == Image3D) MakeMipmapLevel3D(m_Texels[i * TextureCnt], SrcSize, PackType, m, m_Texels[i * TextureCnt + m], SampleMode);
			else if (iType == ImageCubeMap) MakeMipmapLevel2D(m_Texels[i * TextureCnt], SrcSize.xy(), PackType, m, m_Texels[i * TextureCnt + m], SampleMode);
		}
	}
	return *this;
}

LWImage &LWImage::Compress(uint32_t BCFormat) {
	uint32_t PackType = GetPackType();
	uint32_t iType = GetType();
	if ((PackType != RGBA8 || PackType != RGBA8S || PackType!=SRGBA) && (iType!=Image2D && iType!=ImageCubeMap)) return *this;
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_Texels[0]);
	if (!Alloc) return *this;
	uint32_t ArrayCnt = m_Size.w;
	uint32_t TexelCnt = m_MipmapCount + 1;
	for (uint32_t i = 0; i < ArrayCnt; i++) {
		for (uint32_t m = 0; m < TexelCnt; m++) {
			LWVector2i &Size = m == 0 ? m_Size.xy() : GetMipmapSize2D(m-1);
			uint32_t Len = GetLength2D(Size, BCFormat);
			uint8_t *oPixels = m_Texels[m + i * TexelCnt];
			uint8_t *nPixels = Alloc->Allocate<uint8_t>(Len);
			RGBA8toBC(Size, oPixels, nPixels, BCFormat);
			m_Texels[m + i * TexelCnt] = nPixels;
			LWAllocator::Destroy(oPixels);
		}
	}
	m_Flag = (m_Flag & ~PackTypeBits) | BCFormat;
	return *this;
}

LWImage &LWImage::Decompress(void) {
	uint32_t PackType = GetPackType();
	uint32_t iType = GetType();
	if ((PackType < BC1 || PackType >= BC7) && (iType != Image2D && iType != ImageCubeMap)) return *this;
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_Texels[0]);
	if (!Alloc) return *this;
	uint32_t ArrayCnt = m_Size.w;
	uint32_t TexelCnt = m_MipmapCount + 1;
	for (uint32_t i = 0; i < ArrayCnt; i++) {
		for (uint32_t m = 0; m < TexelCnt; m++) {
			LWVector2i &Size = m == 0 ? m_Size.xy() : GetMipmapSize2D(m-1);
			uint32_t Len = GetLength2D(Size, RGBA8);
			uint8_t *oPixels = m_Texels[m + i * TexelCnt];
			uint8_t *nPixels = Alloc->Allocate<uint8_t>(Len);
			BCtoRGBA8(Size, oPixels, nPixels, PackType);
			m_Texels[m + i * TexelCnt] = nPixels;
			LWAllocator::Destroy(oPixels);
		}
	}
	m_Flag = (m_Flag & ~PackTypeBits) | (PackType==BC1_SRGB || PackType==BC2_SRGB || PackType==BC3_SRGB || PackType==BC7_SRGB ? SRGBA : RGBA8);
	return *this;
}

uint32_t LWImage::GetBitSize(uint32_t PackType){
		int Sizes[] = { 
		sizeof(uint8_t)*8*4, //sRGBA
		sizeof(uint8_t)* 8 * 4,   //RGBA8
		sizeof(int8_t)* 8 * 4,  //RGBA8S
		sizeof(uint16_t)* 8 * 4,  //RGBA16
		sizeof(int16_t)* 8 * 4, //RGBA16S
		sizeof(uint32_t)* 8 * 4,  //RGBA32
		sizeof(int32_t)* 8 * 4, //RGBA32S
		sizeof(float)* 8 * 4,    //RGBA32F
		sizeof(uint8_t)* 8 * 2,   //RG8
		sizeof(int8_t)* 8 * 2,  //RG8S
		sizeof(uint16_t)* 8 * 2,  //RG16
		sizeof(int16_t)* 8 * 2, //RG16S
		sizeof(uint32_t)* 8 * 2,  //RG32
		sizeof(int32_t)* 8 * 2, //RG32S
		sizeof(float)* 8 * 2,    //RG32F
		sizeof(uint8_t)* 8,       //R8
		sizeof(int8_t)* 8,      //R8S
		sizeof(uint16_t)* 8,      //R16
		sizeof(int16_t)* 8,     //R16S
		sizeof(uint32_t)* 8 ,     //R32
		sizeof(int32_t)* 8,     //R32S
		sizeof(float)* 8,        //R32F
		16,                      //DEPTH16
		24,                      //DEPTH24
		32,                      //DEPTH32
		32,                      //DEPTH24STENCIL8
		8*8,                     //BC1
		8*8,                     //BC1_SRGB
		8*16,                    //BC2
		8*16,                    //BC2_SRGB
		8*16,                    //BC3
		8*16,                    //BC3_SRGB
		8*8,                     //BC4
		8*16,                    //BC5
		8*16,                    //BC6
		8*16,					 //BC7
		8*16                     //BC7_SRGB
		}; 
	return Sizes[PackType];
}

uint32_t LWImage::GetStride(int32_t Width, uint32_t PackType){
	uint32_t BitSize = GetBitSize(PackType);
	if(CompressedType(PackType)) return std::max<int32_t>(1, (Width + 3) / 4) * (BitSize / 8);
	return Width * ((BitSize + 7) / 8);
}

bool LWImage::CompressedType(uint32_t PackType) {
	return PackType >= BC1 && PackType<=BC7_SRGB;
}

bool LWImage::DepthType(uint32_t PackType) {
	return PackType >= DEPTH16 && PackType <= DEPTH24STENCIL8;
}

bool LWImage::StencilType(uint32_t PackType) {
	return PackType == DEPTH24STENCIL8;
}

bool LWImage::SRGBAType(uint32_t PackType) {
	return PackType == SRGBA || PackType == BC1_SRGB || PackType == BC2_SRGB || PackType == BC3_SRGB || PackType == BC7_SRGB;
}

LWImage &LWImage::SetSRGBA(bool bIsSRGBA) {
	uint32_t PackType = GetPackType();
	if (bIsSRGBA) {
		if ((PackType == RGBA8 || PackType == RGBA8S)) PackType = SRGBA;
		else if (PackType == BC1) PackType = BC1_SRGB;
		else if (PackType == BC2) PackType = BC2_SRGB;
		else if (PackType == BC3) PackType = BC3_SRGB;
		else if (PackType == BC7) PackType = BC7_SRGB;
	} else {
		if (PackType == SRGBA) PackType = RGBA8;
		else if (PackType == BC1_SRGB) PackType = BC1;
		else if (PackType == BC2_SRGB) PackType = BC2;
		else if (PackType == BC3_SRGB) PackType = BC3;
		else if (PackType == BC7_SRGB) PackType = BC7;
	}
	m_Flag = (m_Flag & ~PackTypeBits) | PackType;
	return *this;
}

uint32_t LWImage::GetLength1D(int32_t Width, uint32_t PackType) {
	return GetStride(Width, PackType);
}

uint32_t LWImage::GetLength2D(const LWVector2i &Size, uint32_t PackType) {
	if (CompressedType(PackType)) return GetStride(Size.x, PackType)*((Size.y + 3) / 4);
	return GetStride(Size.x, PackType)*Size.y;
}

uint32_t LWImage::GetLength3D(const LWVector3i &Size, uint32_t PackType) {
	if (CompressedType(PackType)) return GetStride(Size.x, PackType)*((Size.y + 3) / 4)*Size.z;
	return GetStride(Size.x, PackType)*Size.y*Size.z;
}

int32_t LWImage::GetMipmapSize1D(uint32_t MipLevel) const {
	return MipmapSize1D(m_Size.x, MipLevel + 1);
}

LWVector2i LWImage::GetMipmapSize2D(uint32_t MipLevel) const{
	return MipmapSize2D(m_Size.xy(), MipLevel + 1);
}

LWVector3i LWImage::GetMipmapSize3D(uint32_t MipLevel) const{
	return MipmapSize3D(m_Size.xyz(), MipLevel + 1);
}

uint32_t LWImage::GetPackType(void) const{
	return m_Flag&PackTypeBits;
}

uint32_t LWImage::GetType(void) const{
	return m_Flag & ImageTypeBits;
}

int32_t LWImage::GetSize1D(void) const{
	return m_Size.x;
}

LWVector2i LWImage::GetSize2D(void) const{
	return LWVector2i(m_Size.x, m_Size.y);
}

LWVector3i LWImage::GetSize3D(void) const{
	return m_Size.xyz();
}

uint32_t LWImage::GetMipmapCount(void) const{
	return m_MipmapCount;
}

uint32_t LWImage::GetLayers(void) const {
	return m_Size.w;
}

bool LWImage::isSRGBA(void) const {
	return SRGBAType(GetPackType());
}

bool LWImage::isCompressed(void) const {
	return CompressedType(GetPackType());
}

bool LWImage::isDepth(void) const {
	return DepthType(GetPackType());
}

bool LWImage::isStencil(void) const {
	return StencilType(GetPackType());
}

uint8_t *LWImage::GetTexels(uint32_t Index){
	return m_Texels[Index];
}

const uint8_t *LWImage::GetTexels(uint32_t Index) const {
	return m_Texels[Index];
}

uint8_t *LWImage::GetTexels(uint32_t Layer, uint32_t Mipmap) {
	return m_Texels[Layer * (m_MipmapCount + 1) + Mipmap];
}

const uint8_t *LWImage::GetTexels(uint32_t Layer, uint32_t Mipmap) const {
	return m_Texels[Layer * (m_MipmapCount + 1) + Mipmap];
}

uint8_t **LWImage::GetTexels(void) {
	return m_Texels;
}

LWImage &LWImage::operator = (LWImage &&Image){
	uint32_t oTotalTextures = Image.m_Size.w * (Image.m_MipmapCount + 1);
	uint32_t mTotalTextures = m_Size.w * (m_MipmapCount + 1);
	
	m_Size = std::exchange(Image.m_Size, LWVector4i(1,1,1,0));
	m_Flag = std::exchange(Image.m_Flag, 0);
	m_MipmapCount = std::exchange(Image.m_MipmapCount, 0);

	for (uint32_t i = 0; i < oTotalTextures; i++) {
		uint8_t *pTexels = m_Texels[i];
		m_Texels[i] = std::exchange(Image.m_Texels[i], nullptr);
		LWAllocator::Destroy(pTexels);
	}
	for (uint32_t i = oTotalTextures; i < mTotalTextures; i++) m_Texels[i] = LWAllocator::Destroy(m_Texels[i]);
	return *this;
}

LWImage &LWImage::operator = (const LWImage &Image){
	LWAllocator *Alloc = LWAllocator::GetAllocator(Image.m_Texels[0]);
	
	uint32_t oLayers = Image.m_Size.w;
	uint32_t mLayers = m_Size.w;
	uint32_t oTotalTextures = (Image.m_MipmapCount + 1);
	uint32_t mTotalTextures = (m_MipmapCount + 1);

	m_Size = Image.m_Size;
	m_Flag = Image.m_Flag;
	m_MipmapCount = Image.m_MipmapCount;
	uint32_t PackType = GetPackType();

	for (uint32_t i = 0; i < oLayers; i++) {
		for (uint32_t m = 0; m < oTotalTextures; m++) {
			uint32_t Len = GetLength3D(m == 0 ? m_Size.xyz() : GetMipmapSize3D(m - 1), PackType);
			uint32_t o = m + i * oTotalTextures;
			uint8_t *pTexels = m_Texels[o];
			m_Texels[o] = Alloc->Allocate<uint8_t>(Len);
			std::copy(Image.m_Texels[o], Image.m_Texels[o] + Len, m_Texels[o]);
			LWAllocator::Destroy(pTexels);
		}
	}
	uint32_t oTotal = oLayers * oTotalTextures;
	uint32_t mTotal = mLayers * mTotalTextures;
	for (uint32_t i = oTotal; i < mTotal; i++) m_Texels[i] = LWAllocator::Destroy(m_Texels[i]);
	return *this;
}

LWImage::LWImage(LWImage &&Image) {
	*this = std::move(Image);
}

LWImage::LWImage(const LWImage &Image) {
	*this = Image;
}

LWImage::LWImage(int32_t Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator) : LWImage(LWVector3i(Size, 1, 1), Layers, Image1D, PackType, MipmapCount, Texels, Allocator) {}

LWImage::LWImage(const LWVector2i &Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator) : LWImage(LWVector3i(Size, 1), (Layers * ((PackType&ImageCubeMap) != 0 ? 6 : 1)), (PackType&ImageCubeMap) != 0 ? ImageCubeMap : Image2D, (PackType & ~ImageCubeMap), MipmapCount, Texels, Allocator) {}

LWImage::LWImage(const LWVector3i &Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator) : LWImage(Size, Layers, Image3D, PackType, MipmapCount, Texels, Allocator) {}

LWImage::LWImage(const LWVector3i &Size, uint32_t Layers, uint32_t iType, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator) : m_Size(LWVector4i(Size, Layers)), m_MipmapCount(MipmapCount), m_Flag(PackType | iType) {
	uint32_t TextureCnt = (m_MipmapCount + 1);
	for (int32_t i = 0; i < m_Size.w; i++) {
		for (uint32_t m = 0; m < TextureCnt; m++) {
			uint32_t Len = GetLength3D(m==0?Size:GetMipmapSize3D(m-1), PackType);
			uint32_t n = i * TextureCnt + m;
			m_Texels[n] = Allocator.Allocate<uint8_t>(Len);
			if (Texels) std::copy(Texels[n], Texels[n] + Len, m_Texels[n]);
		}
	}
}

LWImage::~LWImage(){
	uint32_t TotalImages = m_Size.w * (m_MipmapCount + 1);
	for (uint32_t i = 0; i < TotalImages; i++) LWAllocator::Destroy(m_Texels[i]);
}