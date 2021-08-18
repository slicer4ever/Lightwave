#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include <cstdarg>
#include <cstdio>
#include <algorithm>

bool LWFileStream::OpenStream(LWFileStream &Result, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream) {
	char8_t Buffer[512];
	char Modes[][4] = { "", "r", "w", "rw", "a", "", "", "", "", "rb", "wb", "rwb" };
	if (!ParsePath(FilePath, Buffer, sizeof(Buffer), ExistingStream)) return false;
	FILE *pFile = nullptr;
	pFile = fopen(Buffer, Modes[Flag]);
	if (!pFile) return false;
	Result = LWFileStream(pFile, Buffer, Flag, Allocator);
	return true;
}

uint8_t LWFileStream::ReadByte(void) {
	return (uint8_t)fgetc(m_FileObject);
}

uint32_t LWFileStream::ReadText(char8_t *Buffer, uint32_t BufferLen) {
	uint32_t BLen = std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t Len = (uint32_t)fread(Buffer, sizeof(char8_t), BLen, m_FileObject);
	if (BufferLen) Buffer[Len++] = '\0';
	return Len;
}

uint32_t LWFileStream::ReadTextLine(char8_t *Buffer, uint32_t BufferLen) {
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = 0;
	if (fgets((char*)Buffer, BufferLen, m_FileObject)) {
		for (; B != BL && *B; ++B, ++o) {
			if (*B == '\n') break;
		}
	}
	if (BufferLen) *B = '\0';
	return o + 1;
}

uint32_t LWFileStream::Read(uint8_t *Buffer, uint32_t Len) {
	size_t L = fread(Buffer, sizeof(uint8_t), Len, m_FileObject);
	return (uint32_t)L;
}

uint32_t LWFileStream::Write(const uint8_t *Buffer, uint32_t Len) {
	size_t L = fwrite(Buffer, sizeof(uint8_t), Len, m_FileObject);
	return (uint32_t)L;
}

LWFileStream &LWFileStream::WriteByte(uint8_t Byte) {
	fputc((int32_t)Byte, m_FileObject);
	return *this;
}

uint32_t LWFileStream::WriteText(const LWUTF8Iterator &Text) {
	auto cText = Text.c_str<1024 * 16>();
	return (uint32_t)fwrite(*cText, sizeof(char8_t), cText.m_DataLen, m_FileObject);
}

LWFileStream &LWFileStream::Seek(int32_t Offset, uint8_t SeekFlag) {
	const int32_t SeekIDs[] = { SEEK_CUR, SEEK_SET, SEEK_END };
	fseek(m_FileObject, Offset, SeekIDs[SeekFlag]);
	return *this;
}

LWFileStream &LWFileStream::Finished(void) {
	if (m_FileObject) {
		fclose(m_FileObject);
		m_FileObject = nullptr;
	}
	return *this;
}

LWFileStream &LWFileStream::Flush(void) {
	if (!m_FileObject) return *this;
	fflush(m_FileObject);
	return *this;
}

uint32_t LWFileStream::GetPosition(void) const {
	return (uint32_t)ftell(m_FileObject);
}

bool LWFileStream::EndOfStream(void) const {
	return feof(m_FileObject) != 0;
}

LWFileStream::LWFileStream(FILE *FileObject, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator) : m_FileObject(FileObject), m_FilePath(FilePath, Allocator), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(Flag) {
	if (!m_FileObject) return;
	struct stat Res;
	int32_t FileID = fileno(m_FileObject);
	if (fstat(FileID, &Res)) return;
	m_Length = (uint32_t)Res.st_size;
	m_CreateTime = (uint64_t)Res.st_ctime;
	m_ModifiedTime = (uint64_t)Res.st_atime;
	m_AccessedTime = (uint64_t)Res.st_atime;
}