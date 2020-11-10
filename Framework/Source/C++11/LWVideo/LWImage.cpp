#include "LWVideo/LWImage.h"
#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWByteBuffer.h"
#include <png.h>
#include <cstring>
#include <cmath>
#include <algorithm>

bool LWImage::LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream){
	uint32_t Result = LWFileStream::IsExtensions(FilePath, "DDS", "dds", "PNG", "png", "TGA", "tga", "ktx2", "KTX2");
	if (Result < 2) return LoadImageDDS(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 4) return LoadImagePNG(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 6) return LoadImageTGA(Image, FilePath, Allocator, ExistingStream);
	else if (Result < 8) return LoadImageKTX2(Image, FilePath, Allocator, ExistingStream);
	return false;
}

bool LWImage::LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
	uint32_t Result = LWFileStream::IsExtensions(FilePath, "DDS", "dds", "PNG", "png", "TGA", "tga", "ktx2", "KTX2");
	if (Result < 2) return LoadImageDDS(Image, Buffer, BufferLen, Allocator);
	else if (Result < 4) return LoadImagePNG(Image, Buffer, BufferLen, Allocator);
	else if (Result < 6) return LoadImageTGA(Image, Buffer, BufferLen, Allocator);
	else if (Result < 8) return LoadImageKTX2(Image, Buffer, BufferLen, Allocator);
	return false;
}

bool LWImage::LoadImageTGA(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream){
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	bool Result = LoadImageTGA(Image, MemBuffer, Stream.Length(), Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageTGA(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
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
		uint32_t PackType = H.m_ColorMapSize == 4 ? RGBA8U : (H.m_ColorMapSize == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.Read<char>(ColorMapBuffer, H.m_ColorMapLength*H.m_ColorMapSize);
		Buf.Read<char>(IndexMapBuffer, H.m_Width*H.m_Height*H.m_PerPixelBits);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), RGBA8, nullptr, 0, Allocator);
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
		uint32_t PackType = H.m_PerPixelBits == 4 ? RGBA8U : (H.m_PerPixelBits == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.OffsetPosition(H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), RGBA8U, nullptr, 0, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		//Offset so that we can reorganize in a forward fashion.
		uint16_t TexelOffset = (PackType == RGBA8U ? 0 : (H.m_Width*H.m_Height * 4) - (H.m_Width*H.m_Height * 3));
		uint8_t A = (PackType == RGBA8U ? 1 : 0);
		uint8_t B = (PackType == RGBA8U ? 0 : 0xFF);
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
		uint32_t PackType = H.m_ColorMapSize == 4 ? RGBA8U : (H.m_ColorMapSize == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.Read<char>(ColorMapBuffer, H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), RGBA8U, nullptr, 0, Allocator);
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
		uint32_t PackType = H.m_PerPixelBits == 4 ? RGBA8U : (H.m_PerPixelBits == 3 ? RGB8 : RGBUnknown);
		if (PackType == RGBUnknown) return false;
		Buf.OffsetPosition(H.m_ColorMapLength*H.m_ColorMapSize);
		Img = LWImage(LWVector2i((int32_t)H.m_Width, (int32_t)H.m_Height), RGBA8U, nullptr, 0, Allocator);
		uint8_t *Texels = Img.GetTexels(0u);
		//Offset so that we can reorganize in a forward fashion.
		uint8_t A = (PackType == RGBA8U ? 1 : 0);
		uint8_t B = (PackType == RGBA8U ? 0 : 0xFF);

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

	LWByteBuffer Buf = LWByteBuffer((const int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned);
	TGA_Header Header;
	if (!ReadHeader(Buf, Header)) return false;
	Buf.OffsetPosition(Header.m_IDLength);
	if (Header.m_ImageType == 1) return ReadColorMappedImage(Buf, Header, Image, Allocator);
	else if (Header.m_ImageType == 2) return ReadColorImage(Buf, Header, Image, Allocator);
	else if (Header.m_ImageType == 9) return ReadRunLengthColorMappedImage(Buf, Header, Image, Allocator);
	else if (Header.m_ImageType == 10) return ReadRunLengthImage(Buf, Header, Image, Allocator);
	return false;
}

bool LWImage::LoadImagePNG(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream){
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	bool Result = LoadImagePNG(Image, MemBuffer, Stream.Length(), Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImagePNG(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
	const uint32_t RGB8 = 0xFE;
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
	LWByteBuffer Buf((const int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned);
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
	png_set_read_fn(png_ptr, (void*)&Buf, ReadFunc);
	png_set_sig_bytes(png_ptr, 0);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &Width, &Height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr);

	uint32_t PackType = (color_type&PNG_COLOR_MASK_COLOR) ? ((color_type&PNG_COLOR_MASK_ALPHA) ? RGBA8U : RGB8) : R8;
	uint32_t RealPackType = (PackType == RGB8 ? RGBA8U : PackType);
	if (Height > MaxHeight) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		return false;
	}
	png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) PackType = RealPackType = RGBA8U;
	Image = LWImage(LWVector2i(Width, Height), RealPackType, nullptr, 0, Allocator);
	uint32_t PNGByteSize = PackType == RGB8 ? 3 : GetBitSize(RealPackType) / 8;
	uint32_t RealByteSize = GetBitSize(RealPackType) / 8;

	uint8_t *Texels = Image.GetTexels(0u) + (Width*Height*RealByteSize) - (Width*Height*PNGByteSize);
	for (uint32_t i = 0; i < Height; i++) SubTexels[i] = Texels + (i*Width*PNGByteSize);
	png_read_image(png_ptr, SubTexels);
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	if (RealPackType != PackType) {
		uint8_t *RealTexels = Image.GetTexels(0u);
		for (uint32_t i = 0; i < Width*Height; i++) {
			RealTexels[i * 4 + 0] = Texels[i * 3 + 0];
			RealTexels[i * 4 + 1] = Texels[i * 3 + 1];
			RealTexels[i * 4 + 2] = Texels[i * 3 + 2];
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
	bool Result = LoadImageDDS(Image, MemBuffer, Stream.Length(), Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageDDS(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
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
	LWByteBuffer ByteBuf((int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned);
	uint32_t Header = ByteBuf.Read<uint32_t>();
	if (Header != MagicHeader) {
		fmt::print("dds has incorrected header.\n");
		return false;
	}
	DDS_Header dHeader;
	if (!ReadHeader(ByteBuf, dHeader)) return false;
	uint32_t PackType = -1;
	DDS_PixelFormat &PForm = dHeader.m_PixelFormat;
	if (PForm.m_Flags & DDPF_FOURCC) {
		if (PForm.m_Format == DXT1Header) PackType = DXT1;
		else if (PForm.m_Format == DXT2Header) PackType = DXT2;
		else if (PForm.m_Format == DXT3Header) PackType = DXT3;
		else if (PForm.m_Format == DXT4Header) PackType = DXT4;
		else if (PForm.m_Format == DXT5Header) PackType = DXT5;
		else if (PForm.m_Format == DXT6Header) PackType = DXT6;
		else if (PForm.m_Format == DXT7Header) PackType = DXT7;
		else if (PForm.m_Format == DXT_ABGR32F) PackType = RGBA32F;
	} else {
		if ((PForm.m_Flags & (DDPF_RGB | DDPF_ALPHA)) != 0 && PForm.m_BitCount == 32) PackType = RGBA8U;
		else if ((PForm.m_Flags & (DDPF_RGB)) != 0 && PForm.m_BitCount == 24) PackType = RGBA8U;
		else if (PForm.m_Flags & (DDPF_LUMINANCE) && PForm.m_BitCount == 8) PackType = R8U;
	}
	if (PackType == -1) return false;
	if (dHeader.m_Depth <= 1) Image = LWImage(LWVector2i(dHeader.m_Width, dHeader.m_Height), PackType | (((dHeader.m_Caps2 & DDSCAPS2_CUBEMAP) != 0) ? LWImage::ImageCubeMap : 0), nullptr, dHeader.m_MipmapCount ? (dHeader.m_MipmapCount - 1) : 0, Allocator);
	else {
		if ((dHeader.m_Caps2 & DDSCAPS2_VOLUME) == 0) return false;
		Image = LWImage(LWVector3i(dHeader.m_Width, dHeader.m_Height, dHeader.m_Depth), PackType, nullptr, dHeader.m_MipmapCount ? (dHeader.m_MipmapCount - 1) : 0, Allocator);
	}
	uint32_t o = ByteBuf.GetPosition();
	uint32_t TexelCnt = (Image.GetMipmapCount() + 1);
	uint32_t ArrayCnt = Image.GetType() == LWImage::ImageCubeMap ? 6 : 1;

	bool HasAlpha = (PForm.m_Flags & DDPF_ALPHAPIXELS) != 0;
	uint32_t RIdx = PForm.m_RBitMask == 0xFF000000 ? 3 : (PForm.m_RBitMask == 0xFF0000) ? 2 : (PForm.m_RBitMask == 0xFF00 ? 1 : 0);
	uint32_t GIdx = PForm.m_GBitMask == 0xFF000000 ? 3 : (PForm.m_GBitMask == 0xFF0000) ? 2 : (PForm.m_GBitMask == 0xFF00 ? 1 : 0);
	uint32_t BIdx = PForm.m_BBitMask == 0xFF000000 ? 3 : (PForm.m_BBitMask == 0xFF0000) ? 2 : (PForm.m_BBitMask == 0xFF00 ? 1 : 0);
	uint32_t AIdx = 3;
	if(HasAlpha) AIdx = PForm.m_ABitMask == 0xFF000000 ? 3 : (PForm.m_ABitMask == 0xFF0000) ? 2 : (PForm.m_ABitMask == 0xFF00 ? 1 : 0);
	for (uint32_t d = 0; d < ArrayCnt; d++) {
		for (uint32_t i = 0; i < TexelCnt; i++) {
			LWVector3i Size = i == 0 ? Image.GetSize3D() : Image.GetMipmapSize3D(i - 1);
			uint32_t Len = LWImage::GetLength3D(Size, PackType);
			uint8_t *Texels = Image.GetTexels(d * TexelCnt + i);

			if (PackType == RGBA8U) { //we need to potentially fix the rgb format if they are not layed out in the same order.
				bool RGB = PForm.m_BitCount == 24;
				if (RGB) Len = Size.x * Size.y * Size.z * 3;
				for (int32_t n = 0; n < Size.x * Size.y * Size.z; n++) {
					uint8_t *T = Texels + n * 4;
					uint8_t *BT = (uint8_t*)Buffer + o + n * (RGB ? 3 : 4);
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
					float *BT = (float*)(Buffer + o + n * (RGB ? 12 : 16));
					//Temp patch for abgr32f images...
					T[0] = BT[0];
					T[1] = BT[1];
					T[2] = BT[2];
					T[3] = RGB ? 1.0f : BT[AIdx];
				}
			} else std::copy(Buffer + o, Buffer + o + Len, Texels);
			o += Len;
		}
	}

	return true;
};

bool LWImage::LoadImageKTX2(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) return false;
	uint8_t *MemBuffer = Allocator.Allocate<uint8_t>(Stream.Length());
	Stream.Read(MemBuffer, Stream.Length());
	bool Result = LoadImageKTX2(Image, MemBuffer, Stream.Length(), Allocator);
	LWAllocator::Destroy(MemBuffer);
	return Result;
}

bool LWImage::LoadImageKTX2(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator) {
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
			if (m_vkFormat == 13) return LWImage::R8U; //VK_FORMAT_R8_UINT
			else if (m_vkFormat == 14) return LWImage::R8; //VK_FORMAT_R8_SINT
			else if (m_vkFormat == 20) return LWImage::RG8U; //VK_FORMAT_R8G8_UINT
			else if (m_vkFormat == 21) return LWImage::RG8; //VK_FORMAT_R8G8_SINT
			else if (m_vkFormat == 41) return LWImage::RGBA8U; //VK_FORMAT_R8G8B8A8_UINT
			else if (m_vkFormat == 42) return LWImage::RGBA8; //VK_FORMAT_R8G8B8A8_SINT
			else if (m_vkFormat == 74) return LWImage::R16U; //VK_FORMAT_R16_UINT
			else if (m_vkFormat == 75) return LWImage::R16; //VK_FORMAT_R16_SINT
			else if (m_vkFormat == 81) return LWImage::RG16U; //VK_FORMAT_R16G16_UINT
			else if (m_vkFormat == 82) return LWImage::RG16; //VK_FORMAT_R16G16_SINT
			else if (m_vkFormat == 95) return LWImage::RGBA16U; //VK_FORMAT_R16G16B16A16_UINT
			else if (m_vkFormat == 96) return LWImage::RGBA16; //VK_FORMAT_R16G16B16A16_SINT
			else if (m_vkFormat == 98) return LWImage::R32U; //VK_FORMAT_R32_UINT
			else if (m_vkFormat == 99) return LWImage::R32; //VK_FORMAT_R32_SINT
			else if (m_vkFormat == 100) return LWImage::R32F; //VK_FORMAT_R32_SFLOAT
			else if (m_vkFormat == 101) return LWImage::RG32U; //VK_FORMAT_R32G32_UINT
			else if (m_vkFormat == 102) return LWImage::RG32; //VK_FORMAT_R32G32_SINT
			else if (m_vkFormat == 103) return LWImage::RG32F; //VK_FORMAT_R32G32_SFLOAT
			else if (m_vkFormat == 107) return LWImage::RGBA32U; //VK_FORMAT_R32G32B32A32_UINT
			else if (m_vkFormat == 108) return LWImage::RGBA32; //VK_FORMAT_R32G32B32A32_SINT
			else if (m_vkFormat == 109) return LWImage::RGBA32F; //VK_FORMAT_R32G32B32A32_SFLOAT
			else if (m_vkFormat == 124) return LWImage::DEPTH16; //VK_FORMAT_D16_UNORM
			else if (m_vkFormat == 126) return LWImage::DEPTH32; //VK_FORMAT_D32_SFLOAT
			else if (m_vkFormat == 129) return LWImage::DEPTH24STENCIL8; //VK_FORMAT_D24_UNORM_S8_UINT
			else if (m_vkFormat == 131) return LWImage::DXT1; //VK_FORMAT_BC1_UNORM_BLOCK
			else if (m_vkFormat == 132) return LWImage::DXT1; //VK_FORMAT_BC1_SRGB_BLOCK
			else if (m_vkFormat == 135) return LWImage::DXT2; //VK_FORMAT_BC2_UNORM_BLOCK
			else if (m_vkFormat == 136) return LWImage::DXT2; //VK_FORMAT_BC2_SRGB_BLOCK
			else if (m_vkFormat == 137) return LWImage::DXT3; //VK_FORMAT_BC3_UNORM_BLOCK
			else if (m_vkFormat == 138) return LWImage::DXT3; //VK_FORMAT_BC3_SRGB_BLOCK
			else if (m_vkFormat == 139) return LWImage::DXT4; //VK_FORMAT_BC4_UNORM_BLOCK
			else if (m_vkFormat == 140) return LWImage::DXT4; //VK_FORMAT_BC4_SRGB_BLOCK
			else if (m_vkFormat == 141) return LWImage::DXT5; //VK_FORMAT_BC5_UNORM_BLOCK
			else if (m_vkFormat == 142) return LWImage::DXT5; //VK_FORMAT_BC5_SNORM_BLOCK
			else if (m_vkFormat == 143) return LWImage::DXT6; //VK_FORMAT_BC6H_UFLOAT_BLOCK
			else if (m_vkFormat == 144) return LWImage::DXT6; //VK_FORMAT_BC6H_SFLOAT_BLOCK
			else if (m_vkFormat == 145) return LWImage::DXT7; //VK_FORMAT_BC7_UNORM_BLOCK
			else if (m_vkFormat == 146) return LWImage::DXT7; //VK_FORMAT_BC7_SRGB_BLOCK
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

	LWByteBuffer Buf((const int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned);
	KTXHeader Header;
	KTXGlobalData GlobalData;
	if (!KTXHeader::Deserialize(Header, Buf)) return false;
	uint32_t IFmt = Header.MapFormatToLWImage();
	if (IFmt == -1) return false;
	if (Header.m_CompressionScheme != 0) return false; //Don't support supercompression at the moment.
	if (Header.m_LayerCount != 1) return false;//Don't support layer image's at the moment.
	if (!KTXGlobalData::Deserialize(GlobalData, Header, Buf, Allocator)) return false;
	if (Header.m_FaceCount > 1) {
		Image = LWImage(LWVector2i(Header.m_PixelWidth, Header.m_PixelHeight), IFmt | LWImage::ImageCubeMap, nullptr, Header.m_LevelCount, Allocator);
	} else {
		Image = LWImage(LWVector2i(Header.m_PixelWidth, Header.m_PixelHeight), IFmt, nullptr, Header.m_LevelCount, Allocator);
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
					const uint8_t *SrcTexels = (const uint8_t*)Buffer + KImg.m_rgbSliceOffset;
					std::copy(SrcTexels, SrcTexels+KImg.m_rgbSliceLength, Texels);
				} else if (hasAlpha) { //Assumed Alphaformat is rgba format.
					if (PackSize == 32) KImg.CopyTexels(TSize, Texels, Buffer); //8 bit rgba format
					else if (PackSize == 64) KImg.CopyTexels(TSize, (uint16_t*)Texels, Buffer); //16
					else if (PackSize == 128) KImg.CopyTexels(TSize, (uint32_t*)Texels, Buffer); //32 bit rgba format.
				} else return false;
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
	uint32_t Len = SaveImagePNG(Image, nullptr, 0);
	if (!Len) return false;
	uint8_t *Buf = Allocator.Allocate<uint8_t>(Len);
	if (SaveImagePNG(Image, Buf, Len)!=Len) {
		fmt::print("Image has incorrect size.\n");
		LWAllocator::Destroy(Buf);
		return false;
	}
	Stream.Write((char*)Buf, Len);
	LWAllocator::Destroy(Buf);
	return true;
}

uint32_t LWImage::SaveImagePNG(LWImage &Image, uint8_t *Buffer, uint32_t BufferLen) {
	const uint32_t RGB8 = 0xFE;
	struct Writer {
		LWByteBuffer Buf;
		uint32_t BytesWritten;
	};

	Writer W = { LWByteBuffer((int8_t*)Buffer, BufferLen, LWByteBuffer::BufferNotOwned), 0 };
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
	if (PackType == RGBA8) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 4, (int8_t*)Buffer);
	else if (PackType == RGBA8U) return WriteWeightedTexel<uint8_t>(TexelCnt, TexelPtrs, TexelWeights, 4, Buffer);
	else if (PackType == RGBA16) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 4, (int16_t*)Buffer);
	else if (PackType == RGBA16U) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 4, (uint16_t*)Buffer);
	else if (PackType == RGBA32) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 4, (int32_t*)Buffer);
	else if (PackType == RGBA32U) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 4, (uint32_t*)Buffer);
	else if (PackType == RGBA32F) return WriteWeightedTexel<float>(TexelCnt, (const float**)TexelPtrs, TexelWeights, 4, (float*)Buffer);
	else if (PackType == RG8) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 2, (int8_t*)Buffer);
	else if (PackType == RG8U) return WriteWeightedTexel<uint8_t>(TexelCnt, (const uint8_t**)TexelPtrs, TexelWeights, 2, (uint8_t*)Buffer);
	else if (PackType == RG16) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 2, (int16_t*)Buffer);
	else if (PackType == RG16U) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 2, (uint16_t*)Buffer);
	else if (PackType == RG32) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 2, (int32_t*)Buffer);
	else if (PackType == RG32U) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 2, (uint32_t*)Buffer);
	else if (PackType == RG32F) return WriteWeightedTexel<float>(TexelCnt, (const float**)TexelPtrs, TexelWeights, 2, (float*)Buffer);
	else if (PackType == R8) return WriteWeightedTexel<int8_t>(TexelCnt, (const int8_t**)TexelPtrs, TexelWeights, 1, (int8_t*)Buffer);
	else if (PackType == R8U) return WriteWeightedTexel<uint8_t>(TexelCnt, (const uint8_t**)TexelPtrs, TexelWeights, 1, (uint8_t*)Buffer);
	else if (PackType == R16) return WriteWeightedTexel<int16_t>(TexelCnt, (const int16_t**)TexelPtrs, TexelWeights, 1, (int16_t*)Buffer);
	else if (PackType == R16U) return WriteWeightedTexel<uint16_t>(TexelCnt, (const uint16_t**)TexelPtrs, TexelWeights, 1, (uint16_t*)Buffer);
	else if (PackType == R32) return WriteWeightedTexel<int32_t>(TexelCnt, (const int32_t**)TexelPtrs, TexelWeights, 1, (int32_t*)Buffer);
	else if (PackType == R32U) return WriteWeightedTexel<uint32_t>(TexelCnt, (const uint32_t**)TexelPtrs, TexelWeights, 1, (uint32_t*)Buffer);
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
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	if (CompressedType(rPackType)) return *this;
	if (rImageType == Image1D) {
		uint32_t MipmapCnt = MipmapCount(m_Size.x);
		for (uint32_t i = m_MipmapCount+1; i <= MipmapCnt; i++) {
			int32_t Width = MipmapSize1D(m_Size.x, i);
			int32_t Len = GetLength1D(Width, rPackType);
			m_Texels[i] = m_Allocator->Allocate<uint8_t>(Len);
			MakeMipmapLevel1D(m_Texels[i - 1], MipmapSize1D(m_Size.x, i - 1), rPackType, 1, m_Texels[i], SampleMode);
		}
		m_MipmapCount = MipmapCnt;
	} else if (rImageType == Image2D) {
		uint32_t MipmapCnt = MipmapCount(LWVector2i(m_Size.x, m_Size.y));
		for (uint32_t i = m_MipmapCount + 1; i <= MipmapCnt; i++) {
			LWVector2i Size = MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), i);
			int32_t Len = GetLength2D(Size, rPackType);
			m_Texels[i] = m_Allocator->Allocate<uint8_t>(Len);
			MakeMipmapLevel2D(m_Texels[i - 1], MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), i - 1), rPackType, 1, m_Texels[i], SampleMode);
		}
		m_MipmapCount = MipmapCnt;
	} else if (rImageType == Image3D) {
		uint32_t MipmapCnt = MipmapCount(m_Size);
		for (uint32_t i = m_MipmapCount + 1; i <= MipmapCnt; i++) {
			LWVector3i Size = MipmapSize3D(m_Size, i);
			int32_t Len = GetLength3D(Size, rPackType);
			m_Texels[i] = m_Allocator->Allocate<uint8_t>(Len);
			MakeMipmapLevel3D(m_Texels[i - 1], MipmapSize3D(m_Size, i - 1), rPackType, 1, m_Texels[i], SampleMode);
		}
		m_MipmapCount = MipmapCnt;
	} else if (rImageType == ImageCubeMap) {
		uint32_t MipmapCnt = MipmapCount(LWVector2i(m_Size.x, m_Size.y));
		for (uint32_t i = 5; i < 6; i--) { //we need to reverse copy all current levels+mipmaps into their new positions first.
			for (uint32_t d = m_MipmapCount; d < (m_MipmapCount + 1); d--) {
				m_Texels[i*(MipmapCnt + 1) + d] = m_Texels[i*(m_MipmapCount + 1) + d];
			}
		}
		for (uint32_t i = 0; i < 6; i++) {
			for (uint32_t d = 0; d <= MipmapCnt; d++) {
				LWVector2i Size = MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), d);
				int32_t Len = GetLength2D(Size, rPackType);
				m_Texels[i*(MipmapCnt+1)+d] = m_Allocator->Allocate<uint8_t>(Len);
				MakeMipmapLevel2D(m_Texels[i*(MipmapCnt+1)+(d - 1)], MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), d - 1), rPackType, 1, m_Texels[i*(MipmapCnt+1)+d], SampleMode);
			}
		}
		m_MipmapCount = MipmapCnt;
	}
	return *this;
}

uint32_t LWImage::GetBitSize(uint32_t PackType){
		int Sizes[] = { 
		sizeof(int8_t)* 8 * 4,   //RGBA8
		sizeof(uint8_t)* 8 * 4,  //RGBA8U
		sizeof(int16_t)* 8 * 4,  //RGBA16
		sizeof(uint16_t)* 8 * 4, //RGBA16U
		sizeof(int32_t)* 8 * 4,  //RGBA32
		sizeof(uint32_t)* 8 * 4, //RGBA32U
		sizeof(float)* 8 * 4,    //RGBA32F
		sizeof(int8_t)* 8 * 2,   //RG8
		sizeof(uint8_t)* 8 * 2,  //RG8U
		sizeof(int16_t)* 8 * 2,  //RG16
		sizeof(uint16_t)* 8 * 2, //RG16U
		sizeof(int32_t)* 8 * 2,  //RG32
		sizeof(uint32_t)* 8 * 2, //RG32U
		sizeof(float)* 8 * 2,    //RG32F
		sizeof(int8_t)* 8,       //R8
		sizeof(uint8_t)* 8,      //R8U
		sizeof(int16_t)* 8,      //R16
		sizeof(uint16_t)* 8,     //R16U
		sizeof(int32_t)* 8 ,     //R32
		sizeof(uint32_t)* 8,     //R32U
		sizeof(float)* 8,        //R32F
		16,                      //DEPTH16
		24,                      //DEPTH24
		32,                      //DEPTH32
		32,                      //DEPTH24STENCIL8
		8*8,                     //DXT1
		8*16,                    //DXT2
		8*16,                    //DXT3
		8*8,                     //DXT4
		8*16,                    //DXT5
		8*16,					 //DXT6
		8*16                     //DXT7
		}; 
	return Sizes[PackType];
}

uint32_t LWImage::GetStride(int32_t Width, uint32_t PackType){
	uint32_t BitSize = GetBitSize(PackType);
	switch (PackType) {
	case DXT1:
	case DXT2:
	case DXT3:
	case DXT4:
	case DXT5:
	case DXT6:
	case DXT7:
		return std::max<int32_t>(1, (Width + 3) / 4)*(BitSize/8);
	};
	return Width * ((BitSize + 7) / 8);
}

bool LWImage::CompressedType(uint32_t PackType) {
	return PackType == DXT1 || PackType == DXT2 || PackType == DXT3 || PackType == DXT4 || PackType == DXT5 || PackType == DXT6 || PackType == DXT7;
}

bool LWImage::DepthType(uint32_t PackType) {
	return PackType == DEPTH16 || PackType == DEPTH24 || PackType == DEPTH24STENCIL8 || PackType == DEPTH32;
}

bool LWImage::StencilType(uint32_t PackType) {
	return PackType == DEPTH24STENCIL8;
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
	return MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), MipLevel + 1);
}

LWVector3i LWImage::GetMipmapSize3D(uint32_t MipLevel) const{
	return MipmapSize3D(m_Size, MipLevel + 1);
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
	return m_Size;
}

uint32_t LWImage::GetMipmapCount(void) const{
	return m_MipmapCount;
}

uint8_t *LWImage::GetTexels(uint32_t Index){
	return m_Texels[Index];
}

uint8_t **LWImage::GetTexels(void) {
	return m_Texels;
}

LWImage &LWImage::operator = (LWImage &&Image){
	m_Allocator = Image.m_Allocator;
	m_Size = Image.m_Size;
	m_Flag = Image.m_Flag;
	m_MipmapCount = Image.m_MipmapCount;

	uint32_t rImageType = (m_Flag&ImageTypeBits);
	uint32_t TotalTextures = (m_MipmapCount + 1);
	if (rImageType == ImageCubeMap) {
		uint32_t o = 0;
		for (uint32_t i = 0; i < 6; i++) {
			for (uint32_t d = 0; d < TotalTextures; d++, o++) m_Texels[o] = Image.m_Texels[o];
		}
	} else {
		for (uint32_t i = 0; i < TotalTextures; i++) m_Texels[i] = Image.m_Texels[i];
	}


	Image.m_MipmapCount = 0;
	Image.m_Flag = 0;
	Image.m_Texels[0] = nullptr;
	return *this;
}

LWImage &LWImage::operator = (const LWImage &Image){
	m_Allocator = Image.m_Allocator;
	m_Size = Image.m_Size;
	m_Flag = Image.m_Flag;
	m_MipmapCount = Image.m_MipmapCount;
	uint32_t TotalTextures = (m_MipmapCount + 1);
	LWVector3i NextSize = m_Size;
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	if (rImageType == ImageCubeMap) {
		uint32_t o = 0;
		for (uint32_t i = 0; i < 6; i++) {
			NextSize = m_Size;
			for (uint32_t d = 0; d < TotalTextures; d++, o++) {
				uint32_t Len = GetStride(NextSize.x, rPackType)*NextSize.y;
				m_Texels[o] = m_Allocator->Allocate<uint8_t>(Len);
				std::copy(Image.m_Texels[o], Image.m_Texels[o] + Len, m_Texels[o]);
				NextSize = LWVector3i(MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), (d + 1)), 0);
			}
		}
	} else {
		for (uint32_t i = 0; i < TotalTextures; i++) {
			uint32_t Len = 0;
			if (rImageType == Image1D) Len = GetLength1D(NextSize.x, rPackType);
			else if (rImageType == Image2D) Len = GetLength2D(LWVector2i(NextSize.x, NextSize.y), rPackType);
			else if (rImageType == Image3D) Len = GetLength3D(NextSize, rPackType);
			m_Texels[i] = m_Allocator->Allocate<uint8_t>(Len);
			std::copy(Image.m_Texels[i], Image.m_Texels[i] + Len, m_Texels[i]);
			if (rImageType == Image1D) NextSize = LWVector3i(Image.GetMipmapSize1D(i), 0, 0);
			else if (rImageType == Image2D) NextSize = LWVector3i(Image.GetMipmapSize2D(i), 0);
			else if (rImageType == Image3D) NextSize = Image.GetMipmapSize3D(i);
		}
	}
	return *this;
}

LWImage::LWImage(LWImage &&Image) : m_Allocator(Image.m_Allocator), m_Size(Image.m_Size), m_Flag(Image.m_Flag), m_MipmapCount(Image.m_MipmapCount){
	uint32_t rImageType = (m_Flag&ImageTypeBits);
	uint32_t TotalTextures = (m_MipmapCount + 1);
	if (rImageType == ImageCubeMap) {
		uint32_t o = 0;
		for (uint32_t i = 0; i < 6; i++) {
			for (uint32_t d = 0; d < TotalTextures; d++, o++) m_Texels[o] = Image.m_Texels[o];
		}
	} else {
		for (uint32_t i = 0; i < TotalTextures; i++) m_Texels[i] = Image.m_Texels[i];
	}
	
	Image.m_MipmapCount = 0;
	Image.m_Flag = 0;
	Image.m_Texels[0] = nullptr;
}

LWImage::LWImage(const LWImage &Image) : m_Allocator(Image.m_Allocator), m_Size(Image.m_Size), m_Flag(Image.m_Flag), m_MipmapCount(Image.m_MipmapCount){
	uint32_t TotalTextures = (m_MipmapCount + 1);
	LWVector3i NextSize = m_Size;
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	if(rImageType==ImageCubeMap){
		uint32_t o = 0;
		for (uint32_t i = 0; i < 6; i++) {
			NextSize = m_Size;
			for (uint32_t d = 0; d < TotalTextures; d++, o++) {
				uint32_t Len = GetStride(NextSize.x, rPackType)*NextSize.y;
				m_Texels[o] = m_Allocator->Allocate<uint8_t>(Len);
				std::copy(Image.m_Texels[o], Image.m_Texels[o] + Len, m_Texels[o]);
				NextSize = LWVector3i(MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), (d + 1)), 0);
			}
		}
	} else {
		for (uint32_t i = 0; i < TotalTextures; i++) {
			uint32_t Len = 0;
			if (rImageType == Image1D) Len = GetLength1D(NextSize.x, rPackType);
			else if (rImageType == Image2D) Len = GetLength2D(LWVector2i(NextSize.x, NextSize.y), rPackType);
			else if (rImageType == Image3D) Len = GetLength3D(NextSize, rPackType);
			m_Texels[i] = m_Allocator->Allocate<uint8_t>(Len);
			std::copy(Image.m_Texels[i], Image.m_Texels[i] + Len, m_Texels[i]);
			if (rImageType == Image1D) NextSize = LWVector3i(Image.GetMipmapSize1D(i), 0, 0);
			else if (rImageType == Image2D) NextSize = LWVector3i(Image.GetMipmapSize2D(i), 0);
			else if (rImageType == Image3D) NextSize = Image.GetMipmapSize3D(i);
		}
	}
}

LWImage::LWImage(int32_t Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator) : m_Allocator(&Allocator), m_Size(LWVector3i(Size, 1, 1)), m_Flag(PackType | Image1D), m_MipmapCount(MipmapCount) {
	uint32_t TotalTextures = (MipmapCount + 1);
	int32_t NextSize = m_Size.x;
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	for (uint32_t i = 0; i < TotalTextures; i++) {
		uint32_t Len = GetLength1D(NextSize, rPackType);
		m_Texels[i] = Allocator.Allocate<uint8_t>(Len);
		if (Texels) std::copy(Texels[i], Texels[i] + Len, m_Texels[i]);
		NextSize = GetMipmapSize1D(i);
	}
}

LWImage::LWImage(const LWVector2i &Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator) : m_Allocator(&Allocator), m_Size(LWVector3i(Size, 1)), m_Flag(PackType | ((PackType&ImageTypeBits)==ImageCubeMap?0:Image2D)), m_MipmapCount(MipmapCount){
	uint32_t TotalTextures = (MipmapCount + 1);
	LWVector2i NextSize = LWVector2i(m_Size.x, m_Size.y);
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	if (rImageType==ImageCubeMap) {
		for (uint32_t i = 0; i < 6; i++) {
			NextSize = LWVector2i(m_Size.x, m_Size.y);
			for (uint32_t d = 0; d < TotalTextures; d++) {
				uint32_t o = i * TotalTextures + d;
				uint32_t Len = GetLength2D(NextSize, rPackType);
				m_Texels[o] = Allocator.Allocate<uint8_t>(Len);
				if (Texels) std::copy(Texels[o], Texels[o] + Len, m_Texels[o]);
				NextSize = MipmapSize2D(LWVector2i(m_Size.x, m_Size.y), d + 1);
			}
		}
	} else {
		for (uint32_t i = 0; i < TotalTextures; i++) {
			uint32_t Len = GetLength2D(NextSize, rPackType);
			m_Texels[i] = Allocator.Allocate<uint8_t>(Len);
			if (Texels) std::copy(Texels[i], Texels[i] + Len, m_Texels[i]);
			NextSize = GetMipmapSize2D(i);
		}
	}
}

LWImage::LWImage(const LWVector3i &Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator) : m_Allocator(&Allocator), m_Size(Size), m_Flag(PackType | Image3D), m_MipmapCount(MipmapCount) {
	uint32_t TotalTextures = (MipmapCount + 1);
	LWVector3i NextSize = m_Size;
	uint32_t rPackType = m_Flag & PackTypeBits;
	uint32_t rImageType = m_Flag & ImageTypeBits;
	for (uint32_t i = 0; i < TotalTextures; i++) {
		uint32_t Len = GetLength3D(NextSize, rPackType);
		m_Texels[i] = Allocator.Allocate<uint8_t>(Len);
		if (Texels) std::copy(Texels[i], Texels[i] + Len, m_Texels[i]);
		NextSize = GetMipmapSize3D(i);
	}
}

LWImage::LWImage() : m_Allocator(nullptr), m_MipmapCount(0), m_Flag(0){
	m_Texels[0] = nullptr;
}

LWImage::~LWImage(){
	uint32_t ImageCnt = (m_MipmapCount + 1);
	if ((m_Flag&ImageTypeBits) == ImageCubeMap) ImageCnt *= 6;
	for (uint32_t i = 0; i < ImageCnt; i++) LWAllocator::Destroy(m_Texels[i]);

}