#include "LWCore/LWTypes.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWFileStream.h"
#include "Font.h"


bool ProcessFont(const LWUTF8Iterator &FilePath, LWAllocator &Allocator) {
	LWFileStream Stream;
	if(!LWFileStream::OpenStream(Stream, FilePath, LWFileStream::ReadMode|LWFileStream::BinaryMode, Allocator)){
		fmt::print("Failed to open: '{}'\n", FilePath);
		return false;
	}
	Font *F = Font::LoadFontTTF(&Stream, 16, '0', 1, Allocator);
	
	for(int32_t i=0;i<Font::MaxTextures;i++){
		LWImage &Img = F->GetImage(i);
		if (Img.GetSize2D() <= LWVector2i(1, 1)) continue;
		fmt::print("{}: {}\n", i, Img.GetSize2D());
		if (!LWImage::SaveImagePNG(Img, LWUTF8I::Fmt<64>("Atlas_{}.png", i), Allocator)) {
			fmt::print("Failed to save image.\n");
		}
	}
	LWAllocator::Destroy(F);
	return true;
}


int32_t LWMain(int32_t argc, LWUTF8I *argv) {
	LWAllocator_Default DefAlloc;
	ProcessFont("App:coolvetica rg it.ttf", DefAlloc);
	return 0;
}