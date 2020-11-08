#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <algorithm>

uint32_t LWFileStream::GetWorkingDirectory(char8_t *Buffer, uint32_t BufferLen) {
	uint32_t DirLen = (uint32_t)strlen(LWAppContext.m_AppDirectory);
	char8_t *B = Buffer;
	char8_t *BL = B + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t CopyLen = std::min<uint32_t>(DirLen, (uint32_t)(uintptr_t)(BL - B));
	std::copy(LWAppContext.m_AppDirectory, LWAppContext.m_AppDirectory + CopyLen, B);
	B += CopyLen;
	if (B + 1 < BL) *B++ == '/';
	if (BufferLen) *B = '\0';
	return DirLen + 2;
}

uint32_t LWFileStream::GetFolderPath(uint32_t FolderID, char8_t *Buffer, uint32_t BufferLen){
	if (FolderID == Fonts) return snprintf((char*)Buffer, BufferLen, "/system/fonts"); //Default assumed font path.
	else if (FolderID == App) return snprintf((char*)Buffer, BufferLen, "%c", AssetToken); //Default assumed app data path.
	else if (FolderID == User) return snprintf((char*)Buffer, BufferLen, "%s", LWAppContext.m_AppDirectory);
	else if (FolderID == Game) return snprintf((char*)Buffer, BufferLen, "%s", LWAppContext.m_AppDirectory);
	return 0;
}

bool LWFileStream::OpenStream(LWFileStream &Result, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream) {
	char8_t Buffer[512];
	const char Modes[][4] = { "", "r", "w", "rw", "a", "", "", "", "", "rb", "wb", "rwb" };
	auto OpenAsset = [&Buffer, &Modes, &Flag, &Result, &Allocator](void)->bool {
		if (Flag & WriteMode) return false;
		AAsset *aFile = AAssetManager_open(LWAppContext.m_App->assetManager, Buffer + 1, AASSET_MODE_STREAMING);
		if (!aFile) return false;
		Result = LWFileStream((FILE*)aFile, Buffer, Flag | AssetMode, Allocator);
		return true;
	};

	auto OpenFile = [&Buffer, &Modes, &Flag, &Result, &Allocator](void)->bool {
		FILE *pFile = nullptr;
		pFile = fopen((const char*)Buffer, Modes[Flag]);
		if (!pFile) return false;
		Result = LWFileStream(pFile, Buffer, Flag, Allocator);
		return true;
	};

	if (!ParsePath(FilePath, Buffer, sizeof(Buffer), ExistingStream)) return false;

	if (*Buffer == AssetToken) return OpenAsset();
	return OpenFile();
}

uint8_t LWFileStream::ReadByte(void) {
	typedef uint8_t(*Func_T)(void*);

	//typedef int32_t(*Func_T)(const LWUnicodeIterator<Type> &, int8_t*);
	auto ReadAsset = [](void *FileObject)->uint8_t {
		uint8_t Result;
		AAsset_read((AAsset*)FileObject, &Result, sizeof(uint8_t));
		return Result;
	};
	auto ReadFile = [](void *FileObject)->uint8_t {
		return (uint8_t)fgetc((FILE*)FileObject);
	};
	Func_T Funcs[] = { ReadFile, ReadAsset };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset];
}

uint32_t LWFileStream::ReadText(char8_t *Buffer, uint32_t BufferLen) {
	typedef uint32_t(*Func_T)(void*, char8_t*, uint32_t);
	auto ReadAsset = [](void *FileObject, char8_t *Buffer, uint32_t BufferLen) -> uint32_t {
		return (uint32_t)AAsset_read((AAsset*)m_FileObject, (char*)Buffer, BufferLen);
	};
	auto ReadFile = [](void *FileObject, char8_t *Buffer, uint32_t BufferLen) -> uint32_t {
		return (uint32_t)fread((char*)Buffer, sizeof(uint8_t), BufferLen, (FILE*)FileObject);
	};
	Func_T Funcs[] = { ReadFile, ReadAsset };
	
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t Len = Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Buffer, BufferLen);
	B = std::min<char8_t>(B + Len, BL);
	if (BufferLen) *B = '\0';
	return Len;
}

uint32_t LWFileStream::ReadTextLine(char8_t *Buffer, uint32_t BufferLen) {
	typedef uint32_t(*Func_T)(void*, char8_t*, uint32_t);
	auto ReadAsset = [](void *FileObject, char8_t *Buffer, uint32_t BufferLen)->uint32_t {
		//Have to read char by char. 
		char8_t *B = Buffer;
		char8_t *BL = Buffer + BufferLen;
		uint32_t o = 0;
		for (; B != BL; B++, o++) {
			if (!AAsset_read((AAsset*)FileObject, B, sizeof(char8_t))) break;
			if (*B == '\0' || *B == '\n') break;
		}
		return o;
	};

	auto ReadFile = [](void *FileObject, char8_t *Buffer, uint32_t BufferLen)->uint32_t {
		char8_t *B = Buffer;
		char8_t *BL = Buffer + BufferLen;
		uint32_t o = 0;
		if (fgets((char*)Buffer, BufferLen, (FILE*)FileObject)) {
			for (; B != BL && *B; ++B, ++o) {
				if (*B == '\n') break;
			}
		}
		return o;
	};
	Func_T Funcs[] = { ReadFile, ReadAsset };
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t Len = Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Buffer, BufferLen);
	B = std::min<char8_t*>(B + Len, B);
	if (BufferLen) *B = '\0';
	return Len+1;
}

uint32_t LWFileStream::Read(uint8_t *Buffer, uint32_t BufferLen) {
	typedef uint32_t(*Func_T)(void*, uint8_t*, uint32_t);
	auto ReadAsset = [](void *FileObject, uint8_t *Buffer, uint32_t BufferLen) -> uint32_t {
		return (uint32_t)AAsset_read((AAsset*)FileObject, Buffer, sizeof(uint8_t) * BufferLen);
	};

	auto ReadFile = [](void *FileObject, uint8_t *Buffer, uint32_t BufferLen) -> uint32_t {
		return (uint32_t)fread(Buffer, sizeof(uint8_t), BufferLen, (FILE*)FileObject);
	};
	Func_T Funcs[] = { ReadFile, ReadAsset };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Buffer, BufferLen);
}

uint32_t LWFileStream::Write(const uint8_t *Buffer, uint32_t Len) {
	typedef uint32_t(*Func_T)(void*, const uint8_t*, uint32_t);
	auto WriteAsset = [](void *FileObject, const uint8_t *Buffer, uint32_t Len) {
		return 0;
	};

	auto WriteFile = [](void *FileObject, const uint8_t *Buffer, uint32_t Len) {
		return (uint32_t)fwrite(Buffer, sizeof(uint8_t), Len, (FILE*)FileObject);
	};
	Func_T Funcs[] = { WriteFile, WriteAsset };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Buffer, Len);
}

LWFileStream &LWFileStream::WriteByte(uint8_t Byte) {
	typedef uint32_t(*Func_T)(void*, uint8_t);
	auto WriteAsset = [](void *FileObject, uint8_t Byte) {
		return;
	};
	auto WriteFile = [](void *FileObject, uint8_t Byte) {
		fputc((int32_t)Byte, (FILE*)FileObject);
		return;
	};

	Func_T Funcs[] = { WriteFile, WriteAsset }:
	Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Byte);
	return *this;
}

uint32_t LWFileStream::WriteText(const LWUTF8Iterator &Text) {
	typedef uint32_t(*Func_T)(void*, const LWUTF8Iterator &);
	auto WriteAsset = [](void *FileObject, const LWUTF8Iterator &Text)->uint32_t {
		return 0;
	};
	auto WriteFile = [](void *FileObject, const LWUTF8Iterator &Text)->uint32_t {
		auto cText = Text.c_str<1024 * 16>();
		return (uint32_t)fwrite(*cText, sizeof(char8_t), cText.m_DataLen, (FILE*)FileObject);
	};
	Func_T Funcs[] = { WriteFile, WriteAsset };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Text);
}

LWFileStream &LWFileStream::Seek(int32_t Offset, uint8_t SeekFlag) {
	typedef void(*Func_T)(void*, int32_t, uint8_t);
	auto SeekAsset = [](void *FileObject, int32_t Offset, uint8_t SeekFlag) {
		const uint32_t AssetSeekIDs[] = { SEEK_CUR, SEEK_SET, SEEK_END };
		AAsset_seek((AAsset*)FileObject, (off_t)Offset, AssetSeekIDs[SeekFlag]);
		return;
	};
	auto SeekFile = [](void *FileObject, int32_t Offset, uint8_t SeekFlag) {
		const uint32_t FileSeekIDs[] = { SEEK_CUR, SEEK_SET, SEEK_END };
		fseek((FILE*)FileObject, Offset, FileSeekIDs[SeekFlag]);
		return;
	};
	Func_T Funcs[] = { SeekFile, SeekAsset };
	Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, Offset, SeekFlag);
	return *this;
}

LWFileStream &LWFileStream::Finished(void) {
	typedef void(*Func_T)(void*);
	auto CloseAsset = [](void *FileObject) {
		AAsset_close((AAsset*)FileObject);
		return;
	};

	auto CloseFile = [](void *FileObject) {
		fclose((FILE*)FileObject);
		return;
	};
	Func_T Funcs[] = { CloseFile, CloseAsset };
	if (!m_FileObject) return *this;
	Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject);
	m_FileObject = nullptr;
	return *this;
}

uint32_t LWFileStream::GetPosition(void) const {
	typedef uint32_t(*Func_T)(void*, uint32_t);
	auto GetAssetPos = [](void *FileObject, uint32_t Len) -> uint32_t {
		return Len - (uint32_t)AAsset_getRemainingLength((AAsset*)FileObject);
	};

	auto GetFilePos = [](void *FileObject, uint32_t) -> uint32_t {
		return (uint32_t)ftell((FILE*)FileObject);
	};
	Func_T Funcs[] = { GetFilePos, GetAssetPos };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset];
};

bool LWFileStream::EndOfStream(void) const {
	typedef bool(*Func_T)(void*);
	auto AssetEOS = [](void *FileObject) -> bool {
		return (uint32_t)AAsset_getRemainingLength((AAsset*)FileObject) == 0;
	};

	auto FileEOS = [](void *FileObject) -> bool {
		return feof((FILE*)FileObject) != 0;
	};
	Func_T Funcs[] = { FileEOS, AssetEOS };
	return Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject);
}

LWFileStream::LWFileStream(FILE *FileObject, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator) : m_FileObject(FileObject), m_FilePath(FilePath, Allocator), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(Flag) {
	typedef void(*Func_T)(void*, uint32_t&, uint64_t &, uint64_t &, uint64_t &);
	auto PropertysAsset = [](void *FileObject, uint32_t &Length, uint64_t &CreateTime, uint64_t &ModifiedTime, uint64_t &AccessTime) {
		Length = (uint32_t)AAsset_getLength((Asset*)FileObject);
		return;
	};

	auto PropertysFile = [](void *FileObject, uint32_t &Length, uint64_t &CreateTime, uint64_t &ModifiedTime, uint64_t &AccessTime) {
		fseek((FILE*)FileObject, 0, SEEK_END);
		Length = (uint32_t)ftell((FILE*)FileObject);
		fseek((FILE*)FileObject, 0, SEEK_SET);
		return;
	};
	Func_T Funcs[] = { PropertysFile, PropertysAsset };
	if (!m_FileObject) return;
	Funcs[(m_Flag & AssetMode) >> AssetFlagBitOffset](m_FileObject, m_Length, m_CreateTime, m_ModifiedTime, m_AccessedTime);
}
