#include "LWCore/LWText.h"
#include "LWPlatform/LWFileStream.h"
#include "LWVideo/LWFont.h"
#include "LWVideo/LWVideoDriver.h"
#include "LWVideo/LWImage.h"
#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWCrypto.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdarg>

bool LWFontSimpleWriter::WriteTexture(LWTexture *Tex) {
	if (!m_TextureCount || m_Textures[m_TextureCount - 1] != Tex) {
		if (m_TextureCount >= MaxTextures) return false;
		m_Textures[m_TextureCount] = Tex;
		m_TextureVertices[m_TextureCount] = 0;
		m_TextureCount++;
	}
	return true;
}

/*!< \brief function passed to the font object for writing. */
bool LWFontSimpleWriter::WriteGlyph(LWTexture *Tex, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector2f &DistanceField, const LWVector4f &Color) {
	if (!WriteTexture(Tex)) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	LWVector2f BtmLeft = Position;
	LWVector2f TopLeft = Position + LWVector2f(0.0f, Size.y);
	LWVector2f BtmRight = Position + LWVector2f(Size.x, 0.0f);
	LWVector2f TopRight = Position + Size;

	//Remember TexCoord y/w is reversed.
	LWVector2f BtmLeftTC = LWVector2f(TexCoord.x, TexCoord.w);
	LWVector2f TopLeftTC = LWVector2f(TexCoord.x, TexCoord.y);
	LWVector2f BtmRightTC = LWVector2f(TexCoord.z, TexCoord.w);
	LWVector2f TopRightTC = LWVector2f(TexCoord.z, TexCoord.y);

	*(V + 0) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, DistanceField) };
	*(V + 1) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, DistanceField) };
	*(V + 2) = { LWVector4f(TopLeft, 0.0f, 1.0f), Color, LWVector4f(TopLeftTC, DistanceField) };
	*(V + 3) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, DistanceField) };
	*(V + 4) = { LWVector4f(BtmRight, 0.0f, 1.0f), Color, LWVector4f(BtmRightTC, DistanceField) };
	*(V + 5) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, DistanceField) };
	m_TextureVertices[m_TextureCount - 1] += 6;
	return true;
}

LWFontSimpleWriter::LWFontSimpleWriter(LWMesh<LWVertexUI> *Mesh) : m_Mesh(Mesh) {}


LWFont *LWFont::LoadFontAR(LWFileStream *Stream, LWVideoDriver *Driver, LWAllocator &Allocator) {
	//Reverse engineered Artery format from: (https://github.com/Chlumsky/artery-font-format)

	//Supported codepoint types.
	const uint32_t CodepointType_Unicode = 1;

	//Supported image encoding formats.
	const uint32_t ImageEncoding_PNG = 8;
	const uint32_t ImageEncoding_TGA = 9;

	struct ARHeader {
		char m_Tag[16];
		uint32_t m_Magic;
		uint32_t m_Version;
		uint32_t m_Flag;
		uint32_t m_RealType;
		uint32_t m_ReservedA[4];

		uint32_t m_MetaDataFormat;
		uint32_t m_MetaDataLength;
		uint32_t m_VariantCount;
		uint32_t m_VariantsLength;
		uint32_t m_ImageCount;
		uint32_t m_ImagesLength;
		uint32_t m_AppendixCount;
		uint32_t m_AppendixsLength;
		uint32_t m_ReservedB[8];

		static bool Deserialize(ARHeader &Header, LWByteBuffer &Buf) {
			const char *Artery_Font_Header_Tag = "ARTERY/FONT\0\0\0\0\0";
			const uint32_t Artery_Header_Version = 1;
			const uint32_t Artery_Header_Magic = 0x4d276a5cu;

			Buf.Read<char>(Header.m_Tag, sizeof(ARHeader::m_Tag));
			if (!std::equal(Header.m_Tag, Header.m_Tag + sizeof(ARHeader::m_Tag), Artery_Font_Header_Tag)) {
				std::cout << "Font header tag is invalid: '" << Header.m_Tag << "'" << std::endl;
				return false;
			}
			Header.m_Magic = Buf.Read<uint32_t>();
			if (Header.m_Magic != Artery_Header_Magic) {
				std::cout << "Font header magic is invalid." << std::endl;
				return false;
			}
			Header.m_Version = Buf.Read<uint32_t>();
			if (Header.m_Version != Artery_Header_Version) {
				std::cout << "Font version is not supported: " << Header.m_Version << std::endl;
				return false;
			}
			Header.m_Flag = Buf.Read<uint32_t>();
			Header.m_RealType = Buf.Read<uint32_t>();
			Buf.Read<uint32_t>(Header.m_ReservedA, 4);
			Header.m_MetaDataFormat = Buf.Read<uint32_t>();
			Header.m_MetaDataLength = Buf.Read<uint32_t>();
			Header.m_VariantCount = Buf.Read<uint32_t>();
			Header.m_VariantsLength = Buf.Read<uint32_t>();
			Header.m_ImageCount = Buf.Read<uint32_t>();
			Header.m_ImagesLength = Buf.Read<uint32_t>();
			Header.m_AppendixCount = Buf.Read<uint32_t>();
			Header.m_AppendixsLength = Buf.Read<uint32_t>();
			Buf.Read<uint32_t>(Header.m_ReservedB, 8);
			return true;
		}
	};

	struct ARFooter {
		uint32_t m_Salt;
		uint32_t m_Magic;
		uint32_t m_ReservedA[4];
		uint32_t m_TotalLength;
		uint32_t m_Checksum;

		static bool Deserialize(ARFooter &Footer, LWByteBuffer &Buf) {
			const uint32_t Artery_Footer_Magic = 0x55ccb363u;
			Footer.m_Salt = Buf.Read<uint32_t>();
			Footer.m_Magic = Buf.Read<uint32_t>();
			if (Footer.m_Magic != Artery_Footer_Magic) {
				std::cout << "Font Footer magic is incorrect." << std::endl;
				return false;
			}
			Buf.Read<uint32_t>(Footer.m_ReservedA, 4);
			Footer.m_TotalLength = Buf.Read<uint32_t>();
			Footer.m_Checksum = Buf.Read<uint32_t>();
			return true;
		}
	};

	struct ARBounds {
		float m_Left;
		float m_Bottom;
		float m_Right;
		float m_Top;

		LWVector2f GetSize(void) const {
			return LWVector2f(m_Right - m_Left, m_Top - m_Bottom);
		}

		static bool Deserialize(ARBounds &Bounds, LWByteBuffer &Buf) {
			Bounds.m_Left = Buf.Read<float>();
			Bounds.m_Bottom = Buf.Read<float>();
			Bounds.m_Right = Buf.Read<float>();
			Bounds.m_Top = Buf.Read<float>();
			return true;
		}
	};

	struct ARMetrics {
		float m_FontSize;
		float m_DistanceRange;

		float m_emSize;
		float m_Ascender;
		float m_Descender;
		float m_LineHeight;
		float m_UnderlineY;
		float m_UnderlineThickness;
		float m_ReservedA[24];

		static bool Deserialize(ARMetrics &Metrics, LWByteBuffer &Buf) {
			Metrics.m_FontSize = Buf.Read<float>();
			Metrics.m_DistanceRange = Buf.Read<float>();
			Metrics.m_emSize = Buf.Read<float>();
			Metrics.m_Ascender = Buf.Read<float>();
			Metrics.m_Descender = Buf.Read<float>();
			Metrics.m_LineHeight = Buf.Read<float>();
			Metrics.m_UnderlineY = Buf.Read<float>();
			Metrics.m_UnderlineThickness = Buf.Read<float>();
			Buf.Read<float>(Metrics.m_ReservedA, 24);
			return true;
		};
	};

	struct ARAdvance {
		float m_Horizontal;
		float m_Vertical;

		static bool Deserialize(ARAdvance &Advance, LWByteBuffer &Buf) {
			Advance.m_Horizontal = Buf.Read<float>();
			Advance.m_Vertical = Buf.Read<float>();
			return true;
		}
	};

	struct ARGlyph {
		uint32_t m_CodePoint;
		uint32_t m_Image;
		ARBounds m_PlaneBounds;
		ARBounds m_ImageBounds;
		ARAdvance m_Advance;

		static bool Deserialize(ARGlyph &Glyph, LWByteBuffer &Buf) {
			Glyph.m_CodePoint = Buf.Read<uint32_t>();
			Glyph.m_Image = Buf.Read<uint32_t>();
			if (!ARBounds::Deserialize(Glyph.m_PlaneBounds, Buf)) return false;
			if (!ARBounds::Deserialize(Glyph.m_ImageBounds, Buf)) return false;
			if (!ARAdvance::Deserialize(Glyph.m_Advance, Buf)) return false;
			return true;
		}
	};

	struct ARKernPair {
		uint32_t m_CodePointA;
		uint32_t m_CodePointB;
		ARAdvance m_Advance;

		static bool Deserialize(ARKernPair &Kern, LWByteBuffer &Buf) {
			Kern.m_CodePointA = Buf.Read<uint32_t>();
			Kern.m_CodePointB = Buf.Read<uint32_t>();
			if (!ARAdvance::Deserialize(Kern.m_Advance, Buf)) return false;
			return true;
		}
	};

	struct ARFontVariant {
		uint32_t m_Flag;
		uint32_t m_Weight;
		uint32_t m_CodePointType;
		uint32_t m_ImageType;
		uint32_t m_FallbackVariant;
		uint32_t m_FallbackGlyph;
		uint32_t m_ReservedA[6];
		ARMetrics m_Metrics;
		uint32_t m_NameLength;
		uint32_t m_MetaDataLength;
		uint32_t m_GlyphCount;
		uint32_t m_KernCount;
		char *m_Name = nullptr;
		char *m_MetaData = nullptr;
		ARGlyph *m_GlyphList = nullptr;
		ARKernPair *m_KernList = nullptr;

		static bool Deserialize(ARFontVariant &V, LWByteBuffer &Buf, LWAllocator &Allocator) {
			V.m_Flag = Buf.Read<uint32_t>();
			V.m_Weight = Buf.Read<uint32_t>();
			V.m_CodePointType = Buf.Read<uint32_t>();
			V.m_ImageType = Buf.Read<uint32_t>();
			V.m_FallbackVariant = Buf.Read<uint32_t>();
			V.m_FallbackGlyph = Buf.Read<uint32_t>();
			Buf.Read<uint32_t>(V.m_ReservedA, 6);
			if (!ARMetrics::Deserialize(V.m_Metrics, Buf)) return false;
			V.m_NameLength = Buf.Read<uint32_t>();
			V.m_MetaDataLength = Buf.Read<uint32_t>();
			V.m_GlyphCount = Buf.Read<uint32_t>();
			V.m_KernCount = Buf.Read<uint32_t>();
			if (V.m_NameLength) {
				V.m_Name = Allocator.AllocateArray<char>(V.m_NameLength + 1);
				Buf.ReadText(V.m_Name, V.m_NameLength + 1);
				Buf.AlignPosition(4);
			}
			if (V.m_MetaDataLength) {
				V.m_MetaData = Allocator.AllocateArray<char>(V.m_MetaDataLength + 1);
				Buf.ReadText(V.m_MetaData, V.m_MetaDataLength + 1);
				Buf.AlignPosition(4);
			}
			V.m_GlyphList = Allocator.AllocateArray<ARGlyph>(V.m_GlyphCount);
			V.m_KernList = Allocator.AllocateArray<ARKernPair>(V.m_KernCount);
			for (uint32_t i = 0; i < V.m_GlyphCount; i++) {
				if (!ARGlyph::Deserialize(V.m_GlyphList[i], Buf)) return false;
			}
			for (uint32_t i = 0; i < V.m_KernCount; i++) {
				if (!ARKernPair::Deserialize(V.m_KernList[i], Buf)) return false;
			}
			return true;
		}

		~ARFontVariant() {
			LWAllocator::Destroy(m_Name);
			LWAllocator::Destroy(m_MetaData);
			LWAllocator::Destroy(m_GlyphList);
			LWAllocator::Destroy(m_KernList);
		}
	};

	struct ARImageHeader {
		uint32_t m_Flags;
		uint32_t m_Encoding;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Channels;
		uint32_t m_PixelFormat;
		uint32_t m_ImageType;
		uint32_t m_RowLength;
		uint32_t m_Orientation;
		uint32_t m_ChildImages;
		uint32_t m_TextureFlags;
		uint32_t m_ReservedA[3];
		uint32_t m_MetaDataLength;
		uint32_t m_DataLength;
		char *m_MetaData = nullptr;
		const int8_t *m_Data = nullptr;


		static bool Deserialize(ARImageHeader &Image, LWByteBuffer &Buf, LWAllocator &Allocator) {
			Image.m_Flags = Buf.Read<uint32_t>();
			Image.m_Encoding = Buf.Read<uint32_t>();
			Image.m_Width = Buf.Read<uint32_t>();
			Image.m_Height = Buf.Read<uint32_t>();
			Image.m_Channels = Buf.Read<uint32_t>();
			Image.m_PixelFormat = Buf.Read<uint32_t>();
			Image.m_ImageType = Buf.Read<uint32_t>();
			Image.m_RowLength = Buf.Read<uint32_t>();
			Image.m_Orientation = Buf.Read<uint32_t>();
			Image.m_ChildImages = Buf.Read<uint32_t>();
			Image.m_TextureFlags = Buf.Read<uint32_t>();
			Buf.Read<uint32_t>(Image.m_ReservedA, 3);
			Image.m_MetaDataLength = Buf.Read<uint32_t>();
			Image.m_DataLength = Buf.Read<uint32_t>();
			if (Image.m_MetaDataLength) {
				Image.m_MetaData = Allocator.AllocateArray<char>(Image.m_MetaDataLength + 1);
				Buf.ReadText(Image.m_MetaData, Image.m_MetaDataLength + 1);
				Buf.AlignPosition(4);
			}
			Image.m_Data = Buf.GetReadBuffer() + Buf.GetPosition();
			Buf.OffsetPosition(Image.m_DataLength);
			Buf.AlignPosition(4);
			return true;
		};
	};

	struct ARAppendix {
		uint32_t m_MetaDataLength;
		uint32_t m_DataLength;
		char *m_MetaData = nullptr;
		const int8_t *m_Data = nullptr;

		static bool Deserialize(ARAppendix &Appendix, LWByteBuffer &Buf, LWAllocator &Allocator) {
			Appendix.m_MetaDataLength = Buf.Read<uint32_t>();
			Appendix.m_DataLength = Buf.Read<uint32_t>();
			if (Appendix.m_MetaDataLength) {
				Appendix.m_MetaData = Allocator.AllocateArray<char>(Appendix.m_MetaDataLength + 1);
				Buf.ReadText(Appendix.m_MetaData, Appendix.m_MetaDataLength + 1);
				Buf.AlignPosition(4);
			}
			Appendix.m_Data = Buf.GetReadBuffer() + Buf.GetPosition();
			Buf.OffsetPosition(Appendix.m_DataLength);
			Buf.AlignPosition(4);
			return true;
		}

		~ARAppendix() {
			LWAllocator::Destroy(m_MetaData);
		}
	};

	auto Cleanup = [](char *Buffer, ARFontVariant *Variants, ARImageHeader *Images, ARAppendix *Appendixs, LWFont *Font)->LWFont* {
		LWAllocator::Destroy(Buffer);
		LWAllocator::Destroy(Variants);
		LWAllocator::Destroy(Images);
		LWAllocator::Destroy(Appendixs);
		LWAllocator::Destroy(Font);
		return nullptr;
	};

	char *FileBuffer = Allocator.AllocateArray<char>(Stream->Length());
	Stream->Read(FileBuffer, Stream->Length());
	LWByteBuffer Buf = LWByteBuffer((const int8_t*)FileBuffer, Stream->Length(), LWByteBuffer::BufferNotOwned);
	ARHeader Header;
	ARFooter Footer;
	if (!ARHeader::Deserialize(Header, Buf)) return Cleanup(FileBuffer, nullptr, nullptr, nullptr, nullptr);
	uint32_t pPos = Buf.GetPosition();
	ARFontVariant *Variants = Allocator.AllocateArray<ARFontVariant>(Header.m_VariantCount);
	ARImageHeader *Images = Allocator.AllocateArray<ARImageHeader>(Header.m_ImageCount);
	ARAppendix *Appendixs = Allocator.AllocateArray<ARAppendix>(Header.m_AppendixCount);
	uint32_t SelectedVariant = -1;
	for (uint32_t i = 0; i < Header.m_VariantCount; i++) {
		if (!ARFontVariant::Deserialize(Variants[i], Buf, Allocator)) return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
		if (Variants[i].m_CodePointType == CodepointType_Unicode) SelectedVariant = i;
	}
	if ((Buf.GetPosition() - pPos) != Header.m_VariantsLength) {
		std::cout << "Error font's variants lengths invalid." << std::endl;
		return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	pPos = Buf.GetPosition();
	for (uint32_t i = 0; i < Header.m_ImageCount; i++) {
		if (!ARImageHeader::Deserialize(Images[i], Buf, Allocator)) return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	if ((Buf.GetPosition() - pPos) != Header.m_ImagesLength) {
		std::cout << "Error font's images lengths invalid." << std::endl;
		return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	pPos = Buf.GetPosition();
	for (uint32_t i = 0; i < Header.m_AppendixCount; i++) {
		if (!ARAppendix::Deserialize(Appendixs[i], Buf, Allocator)) return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	if ((Buf.GetPosition() - pPos) != Header.m_AppendixsLength) {
		std::cout << "Error font's appendixes lengths invalid." << std::endl;
		return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	if (!ARFooter::Deserialize(Footer, Buf)) return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	uint32_t CRC = LWCrypto::CRC32((uint8_t*)FileBuffer, Buf.GetPosition() - 4, ~0, false); //Have to set CRC32 finished to false as artery file does not do final ^0xFFFFFFFF to checksum.
	if (CRC != Footer.m_Checksum) {
		std::cout << "Checksum for font file is incorrect." << std::endl;
		return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	if (SelectedVariant == -1) {
		std::cout << "No supported font variant was found for font." << std::endl;
		return Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	}
	ARFontVariant &SelV = Variants[SelectedVariant];
	LWFont *Fnt = Allocator.Allocate<LWFont>(Driver, SelV.m_Metrics.m_LineHeight*SelV.m_Metrics.m_FontSize);
	for (uint32_t i = 0; i < Header.m_ImageCount; i++) {
		ARImageHeader &I = Images[i];
		LWImage TexImg;
		if (I.m_Encoding == ImageEncoding_PNG) {
			if (!LWImage::LoadImagePNG(TexImg, (const uint8_t*)I.m_Data, I.m_DataLength, Allocator)) {
				std::cout << "Error loading font png image." << std::endl;
				continue;
			}
		} else if (I.m_Encoding == ImageEncoding_TGA) {
			if (!LWImage::LoadImageTGA(TexImg, (const uint8_t*)I.m_Data, I.m_DataLength, Allocator)) {
				std::cout << "Error loading font tga image." << std::endl;
				continue;
			}
		} else {
			std::cout << "Font contains image that is not supported." << std::endl;
			continue;
		}
		LWTexture *Tex = Driver->CreateTexture(LWTexture::MinLinear | LWTexture::MagLinear, TexImg, Allocator);
		if (!Tex) {
			std::cout << "Failed to create texture for font." << std::endl;
			continue;
		}
		Fnt->SetTexture(i, Tex);
	}
	Fnt->SetErrorGlyph(SelV.m_FallbackGlyph);
	for (uint32_t i = 0; i < SelV.m_GlyphCount; i++) {
		ARGlyph &AG = SelV.m_GlyphList[i];
		LWGlyph *G = Fnt->GetGlyph(AG.m_CodePoint, true);
		LWTexture *Tex = Fnt->GetTexture(AG.m_Image);
		if(!Tex) continue;
		LWVector2f TexSize = Tex->Get2DSize().CastTo<float>();
		LWVector2f iTexSize = 1.0f / TexSize;
		G->m_Character = AG.m_CodePoint;
		G->m_TextureIndex = AG.m_Image;
		G->m_Advance = LWVector2f(AG.m_Advance.m_Horizontal, AG.m_Advance.m_Vertical)*SelV.m_Metrics.m_FontSize;
		G->m_Size = AG.m_ImageBounds.GetSize();
		G->m_Bearing = LWVector2f(AG.m_PlaneBounds.m_Left, -AG.m_PlaneBounds.m_Bottom)*SelV.m_Metrics.m_FontSize;
		G->m_SignedRange = iTexSize * SelV.m_Metrics.m_DistanceRange;
		G->m_TexCoord = LWVector4f(LWVector2f(AG.m_ImageBounds.m_Left, TexSize.y - AG.m_ImageBounds.m_Top) * iTexSize, LWVector2f(AG.m_ImageBounds.m_Right, TexSize.y - AG.m_ImageBounds.m_Bottom) * iTexSize);
	}
	for(uint32_t i=0;i<SelV.m_KernCount;i++){
		ARKernPair &K = SelV.m_KernList[i];
		Fnt->InsertKern(K.m_CodePointA, K.m_CodePointB, K.m_Advance.m_Horizontal*SelV.m_Metrics.m_FontSize);
	}
	Cleanup(FileBuffer, Variants, Images, Appendixs, nullptr);
	return Fnt;
};

LWFont *LWFont::LoadFontFNT(LWFileStream *Stream, LWVideoDriver *Driver, LWAllocator &Allocator) {
	char Buffer[512];
	LWFont *Fnt = nullptr;
	LWTexture *Tex = nullptr;

	LWVector4f Padding = LWVector4f(0.0f);

	auto ParseInfoNode = [](char *Line, LWVector4f &Padding)->bool {
		sscanf(Line, "info face = \"%*[^\"]\" size = %*d bold = %*d italic = %*d charset = \"\" unicode = %*d stretchH = %*d smooth = %*d aa = %*d padding = %f , %f , %f , %f", &Padding.x, &Padding.y, &Padding.z, &Padding.w);
		return true;
	};

	auto ParseCommonNode = [](char *Line, LWVideoDriver *Driver, const LWVector4f &Padding, LWAllocator &Allocator)->LWFont* {
		float LineHeight = 0.0f;
		sscanf(Line, "common lineHeight = %f", &LineHeight); //we only care about lineheight here.
		LineHeight -= ((Padding.y + Padding.w));
		return Allocator.Allocate<LWFont>(Driver, LineHeight);
	};

	auto ParsePageNode = [](char *Line, LWFileStream *ExistingStream, LWFont *Fnt, LWVideoDriver *Driver, LWAllocator &Allocator)->bool {
		char FilePathbuffer[256];
		uint32_t Index = 0;
		sscanf(Line, "page id = %d file = \"%[^\"]", &Index, FilePathbuffer);
		//std::cout << "Loading image: '" << FilePathbuffer << "'" << std::endl;
		LWImage TexImg;
		if (!LWImage::LoadImage(TexImg, FilePathbuffer, Allocator, ExistingStream)) {
			std::cout << "Error loading image: '" << FilePathbuffer << "'" << std::endl;
			return false;
		}
		LWTexture *FontTex = nullptr;

		FontTex = Driver->CreateTexture(LWTexture::MagLinear | LWTexture::MinLinear, TexImg, Allocator);		
		if (!FontTex) {
			std::cout << "Error creating font texture: '" << FilePathbuffer << "'" << std::endl;;
			return false;
		}
		Fnt->SetTexture(Index, FontTex);
		return true;
	};

	auto ParseCharNode = [](char *Line, LWFont *Fnt, const LWVector4f &Padding)->bool {
		uint32_t CharID = 0;
		int32_t x = 0;
		int32_t y = 0;
		int32_t width = 0;
		int32_t height = 0;
		int32_t xoff = 0;
		int32_t yoff = 0;
		int32_t xadvance = 0;
		int32_t Page = 0;
		float w = (Padding.x + Padding.z) - 2.0f; //need to correct for padding.
		float h = (Padding.y + Padding.w);
		sscanf(Line, "char id = %d x = %d y = %d width = %d height = %d xoffset = %d yoffset = %d xadvance = %d page = %d", &CharID, &x, &y, &width, &height, &xoff, &yoff, &xadvance, &Page);
		LWTexture *FntTex = Fnt->GetTexture(Page);
		LWVector2i TexSize = FntTex->Get2DSize();
		LWGlyph *G = Fnt->GetGlyph(CharID, true);
		G->m_Size = LWVector2f((float)width, (float)height);
		G->m_Advance = LWVector2f((float)xadvance - w, 0.0f);
		G->m_Bearing = LWVector2f((float)(xoff + Padding.x), -(float)(Fnt->GetLineSize() - (height)-(yoff + Padding.y)));
		G->m_TexCoord = LWVector4f(((float)x + 0.5f) / (float)TexSize.x, ((float)y + 0.5f) / (float)TexSize.y, ((float)(x + width) - 0.5f) / (float)TexSize.x, ((float)(y + height) - 0.5f) / (float)TexSize.y);
		G->m_Character = CharID;
		G->m_TextureIndex = Page;
		return true;
	};

	auto ParseKerningNode = [](char *Line, LWFont *Fnt)->bool {
		uint32_t Left = 0;
		uint32_t Right = 0;
		float Kern = 0.0f;
		sscanf(Line, "kerning first = %d second = %d amount = %f", &Left, &Right, &Kern);
		Fnt->InsertKern(Left, Right, Kern);
		return true;
	};
	bool Succedded = true;

	while (!Stream->EndOfStream() && Succedded) {
		Stream->ReadTextLine(Buffer, sizeof(Buffer));
		if (LWText::Compare(Buffer, "info", 4)) Succedded = ParseInfoNode(Buffer, Padding);
		else if (LWText::Compare(Buffer, "common", 6)) Succedded = (Fnt = ParseCommonNode(Buffer, Driver, Padding, Allocator)) != nullptr;
		else if (LWText::Compare(Buffer, "page", 4)) Succedded = ParsePageNode(Buffer, Stream, Fnt, Driver, Allocator);
		else if (LWText::Compare(Buffer, "char", 4)) Succedded = ParseCharNode(Buffer, Fnt, Padding);
		else if (LWText::Compare(Buffer, "kerning", 7)) Succedded = ParseKerningNode(Buffer, Fnt);
	}
	if (!Succedded && Fnt) {
		LWAllocator::Destroy(Fnt);
		return nullptr;
	}
	return Fnt;
}

LWFont *LWFont::LoadFontTTF(LWFileStream *Stream, LWVideoDriver *Driver, uint32_t emSize, uint32_t FirstChar, uint32_t NbrChars, LWAllocator &Allocator) {
	return LWFont::LoadFontTTF(Stream, Driver, emSize, 1, &FirstChar, &NbrChars, Allocator);
}

LWFont *LWFont::LoadFontTTF(LWFileStream *Stream, LWVideoDriver *Driver, uint32_t emSize, uint32_t RangeCount, const uint32_t *FirstChar, const uint32_t *NbrChars, LWAllocator &Allocator){
	FT_Library ftLib = nullptr;
	FT_Face ftFace = nullptr;
	FT_StreamDesc desc;
	desc.pointer = Stream;
	auto ReadFunc = [](FT_Stream str, unsigned long offset, unsigned char *Buffer, unsigned long Count)->unsigned long{
		FT_StreamRec *R = (FT_StreamRec*)str;
		FT_StreamDesc Desc = R->descriptor;
		LWFileStream *Stream = (LWFileStream *)Desc.pointer;
		Stream->Seek((int32_t)offset, LWFileStream::SeekStart);
		Stream->Read(Buffer, (uint32_t)Count);
		return Count;
	};

	auto CloseFunc = [](FT_Stream) {};//do nothing to actually close the stream.

	auto BuildTransformTable = [](uint8_t *Dst, uint8_t *Src, uint32_t Stride, uint32_t width, uint32_t height, uint32_t Kernelsize) {
		uint8_t Threshold = 175;
		uint8_t Steps = 255 / Kernelsize;
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {
				uint8_t v = Src[x + y*Stride];
				uint8_t k = Kernelsize;
				bool Inside = v > Threshold;
				int32_t xmin = x > Kernelsize ? x - Kernelsize : 0;
				int32_t xmax = (x + Kernelsize) > width ? width : x + Kernelsize;
				int32_t ymin = y > Kernelsize ? y - Kernelsize : 0;
				int32_t ymax = (y + Kernelsize) > height ? height : y + Kernelsize;
				for (int32_t ny = ymin; ny < ymax; ny++) {
					for (int32_t nx = xmin; nx < xmax; nx++) {
						uint32_t p = Src[nx + ny*Stride];
						if (p > Threshold && Inside) continue;
						else if (p <= Threshold && !Inside) continue;
						uint32_t mx = (nx > x ? nx - x : x - nx);
						uint32_t my = (ny > y ? ny - y : y - ny);
						k = std::min<uint32_t>(k, std::min<uint32_t>(mx, my));
					}
				}
				Dst[x + y*Stride] = (Kernelsize - k)*Steps;
			}
		}
	};

	FT_StreamRec SR = { nullptr, Stream->Length(), Stream->GetPosition(), desc, 0, ReadFunc, CloseFunc, nullptr, nullptr,  nullptr };
	FT_Open_Args Args = { FT_OPEN_STREAM, nullptr, 0,  nullptr, &SR, 0, 0, nullptr };
	uint32_t Error = 0;
	FT_Error ErrorCode = 0;
	//std::cout << "Loading font: " << Stream->GetFilePath().GetCharacters() << std::endl;
	if ((ErrorCode = FT_Init_FreeType(&ftLib)) != 0) Error = 1;
	else if ((ErrorCode = FT_Open_Face(ftLib, &Args, 0, &ftFace))!=0) Error = 2;
	else if ((ErrorCode = FT_Select_Charmap(ftFace, ft_encoding_unicode))!=0) Error = 3;
	else if ((ErrorCode = FT_Set_Char_Size(ftFace, emSize << 6, 0, 72, 0))!=0 ) Error = 4;
	if (Error) {
		if (ftFace) FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLib);
		std::cout << "Font loading error: " << Error << " Code: " << ErrorCode << std::endl;
		return nullptr;
	}
	const uint32_t MaxTextureWidth = 512; //an font texture can not be larger than 512 units wide, this is an attempt to make the texture square.
	uint32_t LongestLineWidth = 0;
	uint32_t CurrentLineWidth = 0;
	uint32_t TallestCharacter = 0;
	uint32_t WidestCharacter = 0;
	uint32_t TextureWidth = 0;
	uint32_t TextureHeight = 0;
	uint32_t LineCount = 1;
	uint32_t PackSize = LWImage::GetBitSize(LWImage::RGBA8) / 8;
	uint32_t KernelSize = 0;

	uint32_t PadWidth = KernelSize;
	uint32_t PadHeight = KernelSize;

	for (uint32_t n = 0; n < RangeCount; n++) {
		for (uint32_t i = FirstChar[n]; i < FirstChar[n] + NbrChars[n]; i++) {
			if (FT_Load_Char(ftFace, i, FT_LOAD_RENDER)) {
				std::cout << "Error loading glyph: " << i << std::endl;
			}
			CurrentLineWidth += ftFace->glyph->bitmap.width+PadWidth*2;
			if (CurrentLineWidth > MaxTextureWidth) {
				CurrentLineWidth = 0;
				LineCount++;
			}
			TallestCharacter = std::max<uint32_t>(ftFace->glyph->bitmap.rows+PadHeight*2, TallestCharacter);
			WidestCharacter = std::max<uint32_t>(ftFace->glyph->bitmap.width+PadWidth*2, WidestCharacter);
			LongestLineWidth = std::max<uint32_t>(CurrentLineWidth, LongestLineWidth);
		}
	}
	//Make 2N:
	TextureWidth = LWNext2N(LongestLineWidth);
	TextureHeight = LWNext2N(TallestCharacter*LineCount);
	//std::cout << "Creating texture: " << TextureWidth << " " << TextureHeight << std::endl;
	unsigned char *Texels = Allocator.AllocateArray<unsigned char>(TextureWidth*PackSize*TextureHeight);
	//unsigned char *DTexels = Allocator.AllocateArray<unsigned char>(TextureWidth*PackSize*TextureHeight);
	//memset(Texels, 0, PackSize*TextureWidth*TextureHeight);
	//std::cout << "Width: " << TextureWidth << " Height: " << TextureHeight << " Total Lines: " << LineCount << " Tallest: " << TallestCharacter << " Total: " << TotalGlyphCount << std::endl;
	uint32_t x = 0;
	uint32_t y = 0;
	bool HasKerning = FT_HAS_KERNING(ftFace);
	LWFont *F = Allocator.Allocate<LWFont>(Driver, ftFace->size->metrics.y_ppem);
	for (uint32_t n = 0; n < RangeCount; n++) {
		for (uint32_t i = FirstChar[n]; i < FirstChar[n] + NbrChars[n]; i++) {
			FT_Load_Char(ftFace, i, FT_LOAD_RENDER);
			if (x + (ftFace->glyph->bitmap.width+PadWidth*2)>TextureWidth) {
				x = 0;
				y += TallestCharacter;
			}
			LWGlyph *G = F->GetGlyph(i, true);
			G->m_Character = i;
			G->m_Size = LWVector2f((float)(ftFace->glyph->bitmap.width+PadWidth*2), (float)(ftFace->glyph->bitmap.rows+PadHeight*2));
			G->m_Advance = LWVector2f((float)(ftFace->glyph->advance.x >> 6), (float)(ftFace->glyph->advance.y >> 6));
			int32_t yMetric = (((ftFace->glyph->metrics.height >> 6) - (ftFace->glyph->metrics.horiBearingY >> 6)) + PadHeight);
			G->m_Bearing = LWVector2f((float)(ftFace->glyph->metrics.horiBearingX >> 6)-PadWidth, (float)yMetric);
			G->m_TexCoord = LWVector4f(((float)x + 0.5f) / (float)TextureWidth, ((float)y + 0.5f) / (float)TextureHeight, ((float)(x + ftFace->glyph->bitmap.width+PadWidth*2) - 0.5f) / (float)TextureWidth, ((float)(y + ftFace->glyph->bitmap.rows+PadHeight*2) - 0.5f) / (float)TextureHeight);
			G->m_TextureIndex = 0;

			uint32_t kc = 0;
			for (uint32_t p = 0; p < RangeCount; p++) {
				for (uint32_t k = FirstChar[p]; k < FirstChar[p] + NbrChars[p]; k++, kc++) {
					FT_Vector kern = { 0,0 };
					if (HasKerning) {
						FT_Get_Kerning(ftFace, FT_Get_Char_Index(ftFace, k), FT_Get_Char_Index(ftFace, i), FT_KERNING_DEFAULT, &kern);
					}
					F->InsertKern(k, i, (float)(kern.x >> 6));
				}
			}
			for (uint32_t r = 0; r < ftFace->glyph->bitmap.rows; r++) {
				for (uint32_t p = 0; p < ftFace->glyph->bitmap.width; p++) {
					unsigned char *T = Texels + ((x + p + PadWidth)*PackSize + (y + PadHeight + r)*TextureWidth*PackSize);
					for (uint32_t k = 0; k < PackSize; k++) T[k] = ftFace->glyph->bitmap.buffer[p + r*ftFace->glyph->bitmap.width];
				}
			}
			x += ftFace->glyph->bitmap.width+PadWidth*2;
		}
	}
	//BuildTransformTable(DTexels, Texels, PackSize*TextureWidth, TextureWidth, TextureHeight, 4);
	LWTexture *Tex = Driver->CreateTexture2D(LWTexture::MinLinear|LWTexture::MagLinear, LWImage::RGBA8, LWVector2i(TextureWidth, TextureHeight), &Texels, 0, Allocator);

	if (Tex) F->SetTexture(0, Tex);

	LWAllocator::Destroy(Texels);
	//LWAllocator::Destroy(DTexels);
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);
	if (!Tex) {
		std::cout << "Error making texture!" << std::endl;
		LWAllocator::Destroy(F);
		F = nullptr;
	}

	return F;
}

const char *LWFont::GetVertexShaderSource(void) {
	static const char FontSource[] = ""\
		"#module Vertex DirectX11_1\n"\
		"cbuffer UIUniform{\n"\
		"	float4x4 Matrix;\n"\
		"};\n"\
		"struct Vertex {\n"\
		"	float4 Position : POSITION;\n"\
		"	float4 Color : COLOR;\n"\
		"	float4 TexCoord : TEXCOORD;\n"\
		"};\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Pixel main(Vertex In) {\n"\
		"	Pixel O;\n"\
		"	O.Position = mul(Matrix, In.Position);\n"\
		"	O.Color = In.Color;\n"\
		"	O.TexCoord = In.TexCoord;\n"\
		"	return O;\n"\
		"}\n"\
		"#module Vertex OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"layout(std140) uniform UIUniform {\n"\
		"	mat4 Matrix;\n"\
		"};\n"\
		"in vec4 Position;\n"\
		"in vec4 Color;\n"\
		"in vec4 TexCoord;\n"\
		"out vec4 pColor;\n"\
		"out vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Vertex OpenGL2_1\n"\
		"struct UIData{\n"\
		"	mat4 Matrix;\n"\
		"};\n"\
		"attribute vec4 Position;\n"\
		"attribute vec4 Color;\n"\
		"attribute vec4 TexCoord;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"uniform UIData UIUniform;\n"\
		"void main(void) {\n"\
		"	gl_Position = UIUniform.Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Vertex OpenGLES2\n"\
		"attribute highp vec4 Position;\n"\
		"attribute lowp vec4 Color;\n"\
		"attribute lowp vec4 TexCoord;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform highp mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n";
	return FontSource;
}

const char *LWFont::GetPixelColorShaderSource(void) {
	static const char FontSource[] = ""\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Texture2D FontTex;\n"\
		"SamplerState FontTexSampler;\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	return In.Color*FontTex.Sample(FontTexSampler, In.TexCoord.xy);\n"\
		"}\n"\
		"#module Pixel OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"uniform sampler2D FontTex;\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 p_Color;\n"\
		"void main(void) {\n"\
		"	p_Color = texture(FontTex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"uniform sampler2D FontTex;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(FontTex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"uniform sampler2D FontTex;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(FontTex, pTexCoord.xy)*pColor;\n"\
		"}\n";
	return FontSource;

}

const char *LWFont::GetPixelMSDFShaderSource(void) {
	static const char FontSource[] = ""\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Texture2D FontTex;\n"\
		"SamplerState FontTexSampler;\n"\
		"float Median(float r, float g, float b){\n"\
		"	return max(min(r, g), min(max(r, g), b));\n"\
		"}\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	float2 mUnit = In.TexCoord.zw;\n"\
		"	float4 Sample = FontTex.Sample(FontTexSampler, In.TexCoord.xy);\n"\
		"	float sigDist = Median(Sample.r, Sample.g, Sample.b)-0.5f;\n"\
		"	sigDist *= dot(mUnit, 0.5f/fwidth(In.TexCoord.xy));\n"\
		"	float a = clamp(sigDist+0.5f, 0.0f, 1.0f);\n"\
		"	return In.Color*float4(1.0f, 1.0f, 1.0f, a);\n"\
		"}\n"\
		"#module Pixel OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"uniform sampler2D FontTex;\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 oColor;\n"\
		"float Median(float r, float g, float b){\n"\
		"	return max(min(r, g), min(max(r, g), b));\n"\
		"}\n"\
		"void main(void) {\n"\
		"	vec2 mUnit = pTexCoord.zw;\n"\
		"	vec4 Sample = texture(FontTex, pTexCoord.xy);\n"\
		"	float sigDist = Median(Sample.r, Sample.g, Sample.b)-0.5f;\n"\
		"	sigDist *= dot(mUnit, 0.5f/fwidth(pTexCoord.xy));\n"\
		"	float a = clamp(sigDist+0.5f, 0.0f, 1.0f);\n"\
		"	oColor = pColor*vec4(1.0f, 1.0f, 1.0f, a);\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"uniform sampler2D FontTex;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"float Median(float r, float g, float b){\n"\
		"	return max(min(r, g), min(max(r, g), b));\n"\
		"}\n"\
		"void main(void) {\n"\
		"	vec2 mUnit = pTexCoord.zw;\n"\
		"	vec4 Sample = texture2D(FontTex, pTexCoord.xy);\n"\
		"	float sigDist = Median(Sample.r, Sample.g, Sample.b)-0.5f;\n"\
		"	sigDist *= dot(mUnit, 0.5f/fwidth(pTexCoord.xy));\n"\
		"	float a = clamp(sigDist+0.5, 0.0, 1.0);\n"\
		"	gl_FragColor = pColor*vec4(1.0, 1.0, 1.0, a);\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"uniform sampler2D FontTex;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"float Median(float r, float g, float b){\n"\
		"	return max(min(r, g), min(max(r, g), b));\n"\
		"}\n"\
		"void main(void) {\n"\
		"	vec2 mUnit = pTexCoord.zw;\n"\
		"	vec4 Sample = texture2D(FontTex, pTexCoord.xy);\n"\
		"	float sigDist = Median(Sample.r, Sample.g, Sample.b)-0.5;\n"\
		"	sigDist *= dot(mUnit, 0.5/fwidth(pTexCoord.xy));\n"\
		"	float a = clamp(sigDist+0.5, 0.0, 1.0);\n"\
		"	gl_FragColor = pColor*vec4(1.0, 1.0, 1.0, a);\n"\
		"}\n";
	return FontSource;

}

LWFont &LWFont::SetTexture(uint32_t TextureIndex, LWTexture *Tex) {
	LWTexture *oldTex = m_TextureList[TextureIndex];
	m_TextureList[TextureIndex] = Tex;
	if (oldTex) m_Driver->DestroyTexture(oldTex);
	return *this;
}

LWVector4f LWFont::MeasureText(const LWText &Text, float Scale) {
	return MeasureText(Text, 0xFFFFFFFF, Scale);
}

LWVector4f LWFont::MeasureTextf(const LWText &Text, float Scale, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Scale);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return MeasureText(Buffer, 0xFFFFFFFF, Scale);
}

LWVector4f LWFont::MeasureText(const LWText &Text, uint32_t CharCount, float Scale) {
	LWVector2f Pos = LWVector2f(0.0f);
	LWGlyph *P = nullptr;
	LWVector4f BoundingVolume = LWVector4f(Pos, Pos);
	const uint8_t *S = LWText::FirstCharacter(Text.GetCharacters());
	for (; S && CharCount; S = LWText::Next(S), CharCount--) {
		uint32_t UTF = LWText::GetCharacter(S);
		if (UTF == (uint32_t)'\n') {
			Pos.x = 0.0f;
			Pos.y -= m_LineSize*Scale;
			P = nullptr;
			continue;
		}
		LWGlyph *G = GetGlyph(UTF);
		if (!G) {
			G = m_ErrorGlyph;
			if (!G) continue; //skip rendering characters we don't have a glyph for.
		}
		float Kern = 0;
		if (P) Kern = GetKernBetween(P->m_Character, G->m_Character)*Scale;
		P = G;
		LWVector2f BtmLeftPnt = LWVector2f(Pos.x + Kern + G->m_Bearing.x*Scale, Pos.y - G->m_Bearing.y*Scale);
		LWVector2f TopRightPnt = LWVector2f(Pos.x + Kern + G->m_Bearing.x*Scale + G->m_Size.x*Scale, Pos.y - G->m_Bearing.y*Scale + G->m_Size.y*Scale);
		if (!G->m_Size.x) TopRightPnt = LWVector2f(Pos.x + Kern + G->m_Bearing.x*Scale + G->m_Advance.x*Scale, Pos.y - G->m_Bearing.y*Scale + G->m_Size.y*Scale);
		BoundingVolume.x = std::min<float>(BtmLeftPnt.x, BoundingVolume.x);
		BoundingVolume.z = std::max<float>(TopRightPnt.x, BoundingVolume.z);
		BoundingVolume.y = std::max<float>(TopRightPnt.y, BoundingVolume.y);
		BoundingVolume.w = std::min<float>(BtmLeftPnt.y, BoundingVolume.w);
		
		Pos.x += (G->m_Advance.x*Scale + Kern);
	}
	return BoundingVolume;
}

LWVector4f LWFont::MeasureTextf(const LWText &Text, uint32_t CharCount, float Scale, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Scale);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return MeasureText(Buffer, CharCount, Scale);
}

uint32_t LWFont::CharacterAt(const LWText &Text, float Width, float Scale) {
	return CharacterAt(Text, Width, 0xFFFFFFFF, Scale);
}

uint32_t LWFont::CharacterAtf(const LWText &Text, float Width, float Scale, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Scale);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return CharacterAtf(Buffer, Width, 0xFFFFFFFF, Scale);
}

uint32_t LWFont::CharacterAt(const LWText &Text, float Width, uint32_t CharCount, float Scale) {
	LWVector2f Pos = LWVector2f(0.0f);
	LWGlyph *P = nullptr;
	LWVector4f BoundingVolume = LWVector4f(Pos, Pos);
	const uint8_t *S = LWText::FirstCharacter(Text.GetCharacters());
	uint32_t i = 0;
	for (; S && CharCount; S = LWText::Next(S), CharCount--, i++) {
		uint32_t UTF = LWText::GetCharacter(S);
		if (UTF == (uint32_t)'\n') {
			Pos.x = 0.0f;
			Pos.y -= m_LineSize*Scale;
			P = nullptr;
			continue;
		}
		LWGlyph *G = GetGlyph(UTF);
		if (!G) {
			G = m_ErrorGlyph;
			if (!G) continue; //skip rendering characters we don't have a glyph for.
		}
		float Kern = 0;
		if (P) Kern = GetKernBetween(P->m_Character, G->m_Character)*Scale;
		P = G;
		Pos.x += (G->m_Advance.x*Scale + Kern);
		if (Pos.x > Width + (G->m_Advance.x*Scale*0.5f)) return i;
	}
	return i;
}

uint32_t LWFont::CharacterAtf(const LWText &Text, float Width, uint32_t CharCount, float Scale, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Scale);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return CharacterAt(Buffer, Width, CharCount, Scale);
}

LWVector4f LWFont::DrawText(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer) {
	return DrawText(Text, 0xFFFFFFFF, Position, Scale, Color, Writer);
}

LWVector4f LWFont::DrawTextf(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Writer);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return DrawText(Buffer, 0xFFFFFFFF, Position, Scale, Color, Writer);
}

LWVector4f LWFont::DrawText(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer) {
	LWVector2f Pos = Position;
	LWGlyph *P = nullptr;
	LWVector4f BoundingVolume = LWVector4f(Pos, Pos);
	float iScale = 1.0f / Scale;
	const uint8_t *S = LWText::FirstCharacter(Text.GetCharacters());
	for (; S && CharCount; S = LWText::Next(S), CharCount--) {
		uint32_t UTF = LWText::GetCharacter(S);
		if (UTF == (uint32_t)'\n') {
			Pos.x = Position.x;
			Pos.y -= m_LineSize*Scale;
			P = nullptr;
			continue;
		}
		LWGlyph *G = GetGlyph(UTF);
		if (!G) {
			G = m_ErrorGlyph;
			if (!G) continue; //skip rendering characters we don't have a glyph for.
		}
		float Kern = 0;
		if (P) Kern = GetKernBetween(P->m_Character, G->m_Character)*Scale;
		P = G;
		if (G->m_Size.x) {
			LWVector2f Position = LWVector2f(Pos.x + Kern + G->m_Bearing.x*Scale, Pos.y - G->m_Bearing.y*Scale);
			LWVector2f Size = G->m_Size*Scale;
			LWVector2f BtmLeftPnt = Position;
			LWVector2f TopRightPnt = Position + Size;
			if (!Writer(m_TextureList[G->m_TextureIndex], Position, Size, G->m_TexCoord, G->m_SignedRange*iScale, Color)) break;
			BoundingVolume.x = std::min<float>(BtmLeftPnt.x, BoundingVolume.x);
			BoundingVolume.z = std::max<float>(TopRightPnt.x, BoundingVolume.z);
			BoundingVolume.y = std::max<float>(TopRightPnt.y, BoundingVolume.y);
			BoundingVolume.w = std::min<float>(BtmLeftPnt.y, BoundingVolume.w);
		}
		Pos.x += (G->m_Advance.x*Scale + Kern);
	}
	return BoundingVolume - LWVector4f(Position, Position);
}

LWVector4f LWFont::DrawTextf(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, LWFontWriteCallback Writer, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Writer);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return DrawText(Buffer, CharCount, Position, Scale, Color, Writer);
}

LWVector4f LWFont::DrawClippedText(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer) {
	return DrawClippedText(Text, 0xFFFFFFFF, Position, Scale, Color, AABB, Writer);
}

LWVector4f LWFont::DrawClippedTextf(const LWText &Text, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Writer);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return DrawClippedText(Buffer, 0xFFFFFFFF, Position, Scale, Color, AABB, Writer);
}

LWVector4f LWFont::DrawClippedText(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer) {
	LWVector2f Pos = Position;
	LWGlyph *P = nullptr;
	LWVector4f BoundingVolume = LWVector4f(Pos, Pos);
	float iScale = 1.0f / Scale;
	const uint8_t *S = LWText::FirstCharacter(Text.GetCharacters());
	for (; S && CharCount; S = LWText::Next(S), CharCount--) {
		uint32_t UTF = LWText::GetCharacter(S);
		if (UTF == (uint32_t)'\n') {
			Pos.x = Position.x;
			Pos.y -= m_LineSize*Scale;
			P = nullptr;
			continue;
		}
		LWGlyph *G = GetGlyph(UTF);
		if (!G) {
			G = m_ErrorGlyph;
			if(!G) continue; //skip rendering characters we don't have a glyph for.
		}
		float Kern = 0;
		if (P) Kern = GetKernBetween(P->m_Character, G->m_Character)*Scale;
		P = G;
		if (G->m_Size.x) {
			
			LWVector2f Position = LWVector2f(Pos.x + Kern + G->m_Bearing.x*Scale, Pos.y - G->m_Bearing.y*Scale);
			LWVector2f Size = G->m_Size*Scale;


			LWVector2f BtmLeftPnt = Position;
			LWVector2f TopRightPnt = Position+Size;
			if (!(BtmLeftPnt.x >= (AABB.x + AABB.z) || TopRightPnt.x < AABB.x || BtmLeftPnt.y >= (AABB.y + AABB.w) || TopRightPnt.y < AABB.y)) {
				
				float Width = Size.x;//(TopRightPnt.x - BtmLeftPnt.x);
				float Height = Size.y;//(TopRightPnt.y - BtmLeftPnt.y);
				float TexWidth = (G->m_TexCoord.z - G->m_TexCoord.x);
				float TexHeight = (G->m_TexCoord.y - G->m_TexCoord.w);

				float LeftRatio = (BtmLeftPnt.x < AABB.x) ? (AABB.x - BtmLeftPnt.x) / Width : 0.0f;
				float RightRatio = (TopRightPnt.x >= (AABB.x + AABB.z)) ? 1.0f - ((TopRightPnt.x - (AABB.x + AABB.z)) / Width) : 1.0f;
				float TopRatio = (TopRightPnt.y >= (AABB.y + AABB.w)) ? 1.0f - ((TopRightPnt.y - (AABB.y + AABB.w)) / Height) : 1.0f;
				float BtmRatio = (BtmLeftPnt.y < AABB.y) ? (AABB.y - BtmLeftPnt.y) / Height : 0.0f;

				TopRightPnt = BtmLeftPnt + LWVector2f(Width, Height)*LWVector2f(RightRatio, TopRatio);
				BtmLeftPnt = BtmLeftPnt + LWVector2f(Width, Height)*LWVector2f(LeftRatio, BtmRatio);

				LWVector2f BtmLeftTC = LWVector2f(G->m_TexCoord.x, G->m_TexCoord.w);
				LWVector2f TopRightTC = BtmLeftTC + LWVector2f(TexWidth, TexHeight)*LWVector2f(RightRatio, TopRatio);
				BtmLeftTC = BtmLeftTC + LWVector2f(TexWidth, TexHeight)*LWVector2f(LeftRatio, BtmRatio);
				if (!Writer(m_TextureList[G->m_TextureIndex], BtmLeftPnt, (TopRightPnt - BtmLeftPnt), LWVector4f(BtmLeftTC.x, TopRightTC.y, TopRightTC.x, BtmLeftTC.y), G->m_SignedRange*iScale, Color)) break;

				BoundingVolume.x = std::min<float>(BtmLeftPnt.x, BoundingVolume.x);
				BoundingVolume.z = std::max<float>(TopRightPnt.x, BoundingVolume.z);
				BoundingVolume.y = std::max<float>(TopRightPnt.y, BoundingVolume.y);
				BoundingVolume.w = std::min<float>(BtmLeftPnt.y, BoundingVolume.w);
				
			}
		}
		Pos.x += (G->m_Advance.x*Scale + Kern);
	}
	return BoundingVolume - LWVector4f(Position, Position);
}

LWVector4f LWFont::DrawClippedTextf(const LWText &Text, uint32_t CharCount, const LWVector2f &Position, float Scale, const LWVector4f &Color, const LWVector4f &AABB, LWFontWriteCallback Writer, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Writer);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)Text.GetCharacters(), lst);
	va_end(lst);
	return DrawClippedText(Buffer, CharCount, Position, Scale, Color, AABB, Writer);
}

LWFont &LWFont::InsertKern(uint32_t Left, uint32_t Right, float Kerning) {
	uint32_t Key = Left | (Right << 16); //yea yea, 32 bits, blah blah blah, hopefully these keys don't overlap.
	auto Res = m_KernTable.emplace(Key, Kerning);
	if (!Res.second) std::cout << "Kern collision: " << Left << " | " << Right << std::endl;
	return *this;
}

LWFont &LWFont::InsertGlyphName(const LWText &GlyphName, uint32_t GlyphID) {
	auto Res = m_GlyphNameMap.emplace(GlyphName.GetHash(), GlyphID);
	if (!Res.second) std::cout << "Glyphnamemap collision: '" << GlyphName.GetCharacters() << "'" << std::endl;
	return *this;
}

LWFont &LWFont::SetErrorGlyph(uint32_t Character) {
	m_ErrorGlyph = GetGlyph(Character, false);
	return *this;
}

LWGlyph *LWFont::GetGlyph(uint32_t Character, bool Insert) {
	if (Insert) {
		auto Res = m_GlyphTable.emplace(Character, LWGlyph());
		return &Res.first->second;
	}
	auto Itr = m_GlyphTable.find(Character);
	return Itr == m_GlyphTable.end() ? nullptr : &Itr->second;
}

uint32_t LWFont::GetGlyphName(const LWText &GlyphName) const {
	auto Iter = m_GlyphNameMap.find(GlyphName.GetHash());
	return Iter == m_GlyphNameMap.end() ? 0 : Iter->second;
}

uint32_t LWFont::GetGlyphName(uint32_t GlyphHash) const{
	auto Iter = m_GlyphNameMap.find(GlyphHash);
	return Iter == m_GlyphNameMap.end() ? 0 : Iter->second;
}

float LWFont::GetKernBetween(uint32_t Left, uint32_t Right) const{
	uint32_t Key = Left | (Right << 16); //yea yea, 32 bits, blah blah blah, hopefully these keys don't overlap.
	auto Itr = m_KernTable.find(Key);
	return Itr == m_KernTable.end() ? 0.0f : Itr->second;
}

LWGlyph *LWFont::GetErrorGlyph(void) {
	return m_ErrorGlyph;
}

float LWFont::GetLineSize(void) const {
	return m_LineSize;
}

LWTexture *LWFont::GetTexture(uint32_t Page){
	return m_TextureList[Page];
}

LWFont::LWFont(LWVideoDriver *Driver, float LineSize) : m_Driver(Driver), m_LineSize(LineSize), m_ErrorGlyph(nullptr) {
	std::fill(m_TextureList, m_TextureList + MaxTextures, nullptr);
}

LWFont::~LWFont() {
	for (uint32_t i = 0; i < MaxTextures; i++) {
		if (m_TextureList[i]) m_Driver->DestroyTexture(m_TextureList[i]);
	}
}