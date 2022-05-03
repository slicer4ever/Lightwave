#include <LWCore/LWTypes.h>
#include <LWCore/LWUnicodeGraphemeIterator.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWByteStream.h>
#include <LWPlatform/LWFileStream.h>
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <iostream>

int32_t main(int32_t argc, char **argv) {
	//Download file from: http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakProperty.txt
	LWAllocator_Default DefAlloc;
	const char DefaultResultPath[] = "LWGraphemeTable.h";
	const char DefaultInPath[] = "GraphemeBreakProperty.txt";
	const char *ResultPath = DefaultResultPath;
	const char *InPath = DefaultInPath;
	for (int32_t i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "-in", 3)) InPath = argv[++i];
		else if (!strncmp(argv[i], "-out", 3)) ResultPath = argv[++i];
	}

	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, InPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, DefAlloc)) {
		std::cout << "Failed to open '" << InPath << "'" << std::endl;
		return 0;
	}
	char *Buffer = DefAlloc.Allocate<char>(Stream.Length()+1);
	Stream.ReadText(Buffer, Stream.Length());

	LWUTF8Iterator FileIter;
	uint32_t Len, RawLen;
	if (!LWUTF8Iterator::Create(FileIter, (char8_t*)Buffer, Len, RawLen)) {
		std::cout << "Error: Text was malformed." << std::endl;
		LWAllocator::Destroy(Buffer);
		return 0;
	}
	//std::cout << FileIter << std::endl;
	uint32_t GenLen = LWGraphemeTable::GenerateTable(nullptr, 0, FileIter);
	if (!GenLen) {
		std::cout << "Error: Failed to generate table." << std::endl;
		LWAllocator::Destroy(Buffer);
		return 0;
	}
	char *ResBuffer = DefAlloc.Allocate<char>(GenLen+1);
	if (LWGraphemeTable::GenerateTable(ResBuffer, GenLen + 1, FileIter) != GenLen) {
		std::cout << "Error: Generate table result changed?" << std::endl;
		LWAllocator::Destroy(ResBuffer);
		LWAllocator::Destroy(Buffer);
		return 0;
	}
	LWFileStream OutStream;
	if (!LWFileStream::OpenStream(OutStream, ResultPath, LWFileStream::WriteMode | LWFileStream::BinaryMode, DefAlloc)) {
		std::cout << "Error could not open outfile '" << ResultPath << "'" << std::endl;
		LWAllocator::Destroy(ResBuffer);
		LWAllocator::Destroy(Buffer);
		return 0;
	}
	OutStream.Write(ResBuffer, GenLen);
	LWAllocator::Destroy(ResBuffer);
	LWAllocator::Destroy(Buffer);
	std::cout << "Wrote file: '" << ResultPath << "'" << std::endl;
	return 0;
}