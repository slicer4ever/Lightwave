#include <LWCore/LWUnicode.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWPlatform/LWFileStream.h>
#include <LWPlatform/LWPlatform.h>
#include <LWEJson.h>
#include <LWEXML.h>

int32_t LWMain(int32_t argc, LWUTF8Iterator *Argv) {
	const int32_t Success = 0;
	const int32_t Failure = 1;
	if (argc < 2) {
		fmt::print("Error: No file was specified.\n");
		return 1;
	}
	LWAllocator_Default DefAlloc;
	LWFileStream Stream;
	int32_t Res = Success;
	for (int32_t i = 1; i < argc && Res==Success; ++i) {
		fmt::print("Processing file: '{}'\n", Argv[i]);
		if (!LWFileStream::OpenStream(Stream, Argv[i], LWFileStream::ReadMode | LWFileStream::BinaryMode, DefAlloc)) {
			fmt::print("Failed to open file: '{}'\n", Argv[i]);
			return Failure;
		}
		char8_t *B = DefAlloc.AllocateA<char8_t>(Stream.Length());
		Stream.Read(B, Stream.Length());
		LWUTF8Iterator BIter;
		uint32_t Len, RawLen;
		if (!LWUTF8I::Create(BIter, B, Stream.Length(), Len, RawLen)) {
			fmt::print("Malformed UTF-8 detected for {}.\n", Argv[i]);
			LWAllocator::Destroy(B);
			return Failure;
		}
		uint32_t ext = LWFileStream::IsExtensions(Argv[i], "json", "JSON", "xml", "XML");
		Res = Failure;
		if (ext < 2) {
			LWEJson *J = DefAlloc.Create<LWEJson>(DefAlloc);
			Res = LWEJson::Parse(*J, BIter)==true;
			LWAllocator::Destroy(J);
		} else if (ext < 4) {
			LWEXML *X = DefAlloc.Create<LWEXML>();
			Res = LWEXML::ParseBuffer(*X, DefAlloc, BIter, true)==true;
			LWAllocator::Destroy(X);
		}
		LWAllocator::Destroy(B);
		if (Res) {
			fmt::print("Error parsing: '{}'\n", Argv[i]);
			break;
		}
	}
	if (DefAlloc.GetAllocatedBytes()) {
		fmt::print("Memory leak detected: {}\n", DefAlloc.GetAllocatedBytes());
	}
	return Res;
}