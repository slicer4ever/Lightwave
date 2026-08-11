#include <LWCore/LWAllocator.h>
#include <LWCore/LWLogger.h>
#include <LWPlatform/LWFileStream.h>
#include <LWPlatform/LWDirectory.h>

LWLOG_DEFAULT

bool ParseSource(const LWUTF8I &Source, const LWUTF8I &FilePath) {
	LWUTF8I C = Source;
	LWUTF8I L = C;
	int32_t FuncCounter = 0;
	int32_t FormatSpecifierFuncCounters = 0;
	int32_t RequiredFormatSpecifiers = 0;
	bool bInParenthesis = false;
	bool bInCommentBlock = false;
	for(;C;++C) {
		if(bInCommentBlock) {
			if(*C=='*' && (*(C+1)=='/')) {
				bInCommentBlock = false;
				++C;
				continue;
			}
		}

		if(!bInParenthesis) {
			if(*C=='/') {
				if(*(C+1)=='/') {
					C.AdvanceLine(); //skip to next line.
					L = C;
					continue;
				}
				if(*(C+1)=='*') {
					bInCommentBlock = true;
					continue;
				}
			}else if(*C=='\n') {
				L = C;
			}else if(*C=='(') {
				FuncCounter++;
			}else if(*C==')') {
				if(RequiredFormatSpecifiers>0 && FuncCounter<=FormatSpecifierFuncCounters) {
					LWLogEvent<256>("{} - {}: Possible issue detected:", FilePath, LWUTF8I::CountLines(LWUTF8I(Source, C)));
					LWLogEvent<512>("{}", LWUTF8I(L+1, C.NextLine()-1));
					RequiredFormatSpecifiers = 0;
				}
				FuncCounter = std::max(0, FuncCounter-1);
			}else if(*C==',') {
				if(RequiredFormatSpecifiers>0 && FuncCounter==FormatSpecifierFuncCounters) {
					RequiredFormatSpecifiers--;
				}
			}else if(*C=='\"' && FuncCounter>0) {
				FormatSpecifierFuncCounters = FuncCounter;
				bInParenthesis=true;
			}
		}else {
			if(*C=='\"') bInParenthesis = false;
			else {
				if(*C=='{') RequiredFormatSpecifiers++;
			}
		}
	}	
	return true;
}

bool ParseFile(const LWUTF8I &Path, LWAllocator &Allocator) {
	LWFileStream Stream;
	if(!LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode|LWFileStream::BinaryMode, Allocator), "Could not open file '{}' to parse.", Path)) return false;
	char8_t *Buffer = Allocator.Allocate<char8_t>(Stream.Length());
	if(!LWLogCriticalIf<256>(Stream.Read(Buffer, Stream.Length())==Stream.Length(), "Failed to read file '{}'.", Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	bool bResult = ParseSource(Buffer, Path);
	LWAllocator::Destroy(Buffer);
	return bResult;
}

bool ParseDirectorys(const LWUTF8I &Path, LWAllocator &Allocator) {
	LWDirectory Dir;
	if(!LWLogCriticalIf<256>(LWDirectory::OpenDir(Dir, Path, Allocator), "Could not open directory '{}' to parse.", Path)) return false;
	uint32_t Length = Dir.GetTotalFiles();
	for(uint32_t i=0;i<Length;++i) {
		const LWFile *F = Dir.GetFile(i);
		if(F->isHidden() || !F->isReadable()) continue;
		if(F->GetName().CompareList(".", "..")!=-1) continue;
		auto FilePath = LWUTF8I::Fmt<512>("{}/{}", Path, F->GetName());
		if(F->isDirectory()) ParseDirectorys(FilePath, Allocator);
		else ParseFile(FilePath, Allocator);
	}
	return true;
}

int32_t LWMain(int32_t argc, LWUTF8Iterator *argv) {
	if(argc<2) {
		LWLogEvent("Require list of path arguments to be passed in(either directorys or files).");
		return 0;
	}

	LWAllocator_Default DefAlloc;
	for(int32_t i=1;i<argc;i++) {
		if(LWDirectory::DirExists(argv[i])) {
			if(*(argv[i].NextEnd()-1)=='/') argv[i] = LWUTF8I(argv[i], argv[i].NextEnd()-1);
			ParseDirectorys(argv[i], DefAlloc);
		} else ParseFile(argv[i], DefAlloc);
	}
	if(!LWLogCriticalIf<256>(DefAlloc.GetAllocatedBytes()==0, "MEMORY LEAK DETECTED: {}", DefAlloc.GetAllocatedBytes())) {
	}
	return 0;
}