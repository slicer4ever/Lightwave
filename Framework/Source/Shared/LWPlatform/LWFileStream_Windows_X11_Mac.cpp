#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <cstdio>
#include <algorithm>

bool LWFileStream::OpenStream(LWFileStream &Result, const char *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream) {
	char Buffer[512];
	char Modes[][4] = { "", "r", "w", "rw", "a", "", "", "", "", "rb", "wb", "rwb" };
	if (!ParsePath(FilePath, Buffer, sizeof(Buffer), ExistingStream)) return false;
	FILE *pFile = nullptr;
	pFile = fopen(Buffer, Modes[Flag]);
	if (!pFile) return false;
	Result = LWFileStream(pFile, LWText(Buffer), Flag, Allocator);
	return true;
}

uint8_t LWFileStream::ReadByte(void) {
	return (uint8_t)fgetc(m_FileObject);
}

uint32_t LWFileStream::ReadText(uint8_t *Buffer, uint32_t BufferLen) {
	uint32_t Len = (uint32_t)fread(Buffer, sizeof(uint8_t), BufferLen, m_FileObject);
	Buffer[Len] = '\0';
	return Len;
}

uint32_t LWFileStream::ReadTextLine(uint8_t *Buffer, uint32_t BufferLen) {
	*Buffer = '\0';
	uint32_t Len = 0;
	if (fgets((char*)Buffer, BufferLen, m_FileObject)) {
		for (; *Buffer;) {
			if (*Buffer == '\n') *Buffer = '\0';
			else {
				Buffer++;
				Len++;
			}
		}
	}
	return Len;
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

uint32_t LWFileStream::WriteText(const LWText &Text) {
	size_t L = fprintf(m_FileObject, "%s", Text.GetCharacters());
	return (uint32_t)L;
}

LWFileStream &LWFileStream::Seek(int32_t Offset, uint8_t SeekFlag) {
	fseek(m_FileObject, Offset, SeekFlag == SeekStart ? SEEK_SET : (SeekFlag == SeekEnd ? SEEK_END : SEEK_CUR));
	return *this;
}

LWFileStream &LWFileStream::Finished(void) {
	if (m_FileObject) {
		fclose(m_FileObject);
		m_FileObject = nullptr;
	}
	return *this;
}

uint32_t LWFileStream::GetPosition(void) const {
	return (uint32_t)ftell(m_FileObject);
}

bool LWFileStream::EndOfStream(void) const {
	return feof(m_FileObject) != 0;
}

LWFileStream::LWFileStream(FILE *FileObject, const LWText &FilePath, uint32_t Flag, LWAllocator &Allocator) : m_FileObject(FileObject), m_FilePath(LWText(FilePath.GetCharacters(), Allocator)), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(Flag) {
	char Buffer[512];
	MakeDirectoryPath(m_FilePath, Buffer, sizeof(Buffer));
	m_DirPath = LWText(Buffer, Allocator);
	if (m_FileObject) {
		struct stat Res;
		int32_t FileID = fileno(m_FileObject);
		if (!fstat(FileID, &Res)) {
			m_Length = (uint32_t)Res.st_size;
			m_CreateTime = (uint64_t)Res.st_ctime;
			m_ModifiedTime = (uint64_t)Res.st_atime;
			m_AccessedTime = (uint64_t)Res.st_atime;
		}
	}

}