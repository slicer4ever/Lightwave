#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <algorithm>

uint32_t LWFileStream::MakeAbsolutePath(const LWText &FilePath, char *Buffer, uint32_t BufferLen){
	const uint32_t LocalBufferSize = 256;
	char LocalBuffer[LocalBufferSize];
	char LocalBufferB[LocalBufferSize];
	char ParsedBuffer[LocalBufferSize];
	char *LastC = LocalBufferB + LocalBufferSize;
	if (!LWFileStream::ParsePath(FilePath, ParsedBuffer, LocalBufferSize)) return 0;
	uint32_t ParseLen = (uint32_t)strlen(ParsedBuffer) + 1;
	if (LWFileStream::PathIsAbsolute(ParsedBuffer)) {
		std::memcpy(LocalBuffer, ParsedBuffer, std::min<uint32_t>(LocalBufferSize, ParseLen));
	} else {
		uint32_t Length = ((uint32_t)std::strlen(LWAppContext.m_AppDirectory)) + 1;
		std::memcpy(LocalBuffer, LWAppContext.m_AppDirectory, sizeof(char)*std::min<uint32_t>(Length, LocalBufferSize));
		uint32_t Len = strlen(LocalBuffer);
		if (Len != LocalBufferSize) LocalBuffer[Len++] = '/';
		if (Len != LocalBufferSize) LocalBuffer[Len] = '\0';
		std::memcpy(LocalBuffer + Len, ParsedBuffer, std::min<uint32_t>(LocalBufferSize - Len, ParseLen));
	}

	LocalBuffer[LocalBufferSize - 1] = '\0';
	//Clean up our absolute path!
	char *Last = LocalBuffer;
	char *B = LocalBufferB;
	for (char *L = LocalBuffer; *L && B != LastC; L++) {
		if (*L == '/' || *L == '\\') {
			*L = '\0';
			if (!strcmp(Last, "."))	B -= 2;
			else if (!strcmp(Last, "..")) {
				B -= 4;
				//back pedal until we get to the directory we are going up!
				for (; B != LocalBufferB && (*B != '\\' || *B != '/'); B--);
			}
			Last = L + 1;
			*L = '/';
		}
		*(B++) = *L;
	}
	if (B != LastC) *(B++) = '\0';
	LocalBufferB[LocalBufferSize - 1] = '\0';
	uint32_t Len = (uint32_t)(B - LocalBufferB);
	if (Buffer){
		std::memcpy(Buffer, LocalBufferB, std::min<uint32_t>(Len, BufferLen));
		Buffer[BufferLen - 1] = '\0';
	}
	return Len;
}

bool LWFileStream::GetFolderPath(uint32_t FolderID, char *Buffer, uint32_t BufferLen){
	if (FolderID == Fonts) snprintf(Buffer, BufferLen, "/system/fonts"); //Default assumed font path.
	else if (FolderID == App) snprintf(Buffer, BufferLen, "%c", AssetToken); //Default assumed app data path.
	else if (FolderID == User || FolderID==Game) snprintf(Buffer, BufferLen, "%s", LWAppContext.m_AppDirectory); //Default assumed program data path.
	else return false;
	return true;
}

bool LWFileStream::OpenStream(LWFileStream &Result, const char *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream) {
	char Buffer[512];
	char Modes[][4] = { "", "r", "w", "rw", "a", "", "", "", "", "rb", "wb", "rwb" };
	if (!ParsePath(FilePath, Buffer, sizeof(Buffer), ExistingStream)) return false;
	if (*Buffer != AssetToken) {
		FILE *pFile = nullptr;
		pFile = fopen(Buffer, Modes[Flag]);
		if (!pFile) return false;
		Result = LWFileStream(pFile, LWText(Buffer), Flag, Allocator);
	}else{
		if (Flag&WriteMode) return false;
		//Buffer+2 passing !/ to get to filepath.
		AAsset *pFIle = AAssetManager_open(LWAppContext.m_App->assetManager, Buffer + 2, AASSET_MODE_STREAMING);
		if (!pFIle) return false;
		Result = LWFileStream((FILE*)pFIle, LWText(Buffer), Flag|AssetMode, Allocator);
	}
	return true;
}

uint8_t LWFileStream::ReadByte(void) {
	uint8_t Result = 0;
	if (m_Flag&AssetMode) AAsset_read((AAsset*)m_FileObject, &Result, sizeof(uint8_t));
	else Result = (uint8_t)fgetc(m_FileObject);
	return Result;
}

uint32_t LWFileStream::ReadText(uint8_t *Buffer, uint32_t BufferLen) {
	uint32_t Len = 0;
	if (m_Flag&AssetMode) Len = (uint32_t)AAsset_read((AAsset*)m_FileObject, Buffer, sizeof(uint8_t)*BufferLen);
	else Len = (uint32_t)fread(Buffer, sizeof(uint8_t), BufferLen - 1, m_FileObject);
	Buffer[Len] = '\0';
	return Len;
}

uint32_t LWFileStream::ReadTextLine(uint8_t *Buffer, uint32_t BufferLen) {
	uint32_t Len = 0;
	*Buffer = '\0';
	if (m_Flag&AssetMode) {
		while ((uint32_t)AAsset_read((AAsset*)m_FileObject, Buffer + Len, sizeof(uint8_t)) == 1) {
			if (Buffer[Len] == '\0' || Buffer[Len] == '\n') {
				Buffer[Len] = '\0';
				return Len;
			}
			Len++;
		}
	} else {
		if (fgets((char*)Buffer, BufferLen, m_FileObject)) {
			for (; *Buffer;) {
				if (*Buffer == '\n') *Buffer = '\0';
				else {
					Buffer++;
					Len++;
				}
			}
		}
	}
	return Len;
}

uint32_t LWFileStream::Read(uint8_t *Buffer, uint32_t Len) {
	uint32_t rLen = 0;
	if (m_Flag&AssetMode) rLen = AAsset_read((AAsset*)m_FileObject, Buffer, sizeof(uint8_t)*Len);
	else {
		rLen = (uint32_t)fread(Buffer, sizeof(uint8_t), Len, m_FileObject);
	}
	return rLen;
}

uint32_t LWFileStream::Write(const uint8_t *Buffer, uint32_t Len) {
	if (m_Flag&AssetMode) return 0;
	return (uint32_t)fwrite(Buffer, sizeof(uint8_t), Len, m_FileObject);
}

LWFileStream &LWFileStream::WriteByte(uint8_t Byte) {
	if (m_Flag&AssetMode) return *this;
	fputc((int32_t)Byte, m_FileObject);
	return *this;
}

uint32_t LWFileStream::WriteText(const LWText &Text) {
	if (m_Flag&AssetMode) return 0;
	return (uint32_t)fprintf(m_FileObject, "%s", Text.GetCharacters());
}

LWFileStream &LWFileStream::Seek(int32_t Offset, uint8_t SeekFlag) {
	if (m_Flag&AssetMode) AAsset_seek((AAsset*)m_FileObject, (off_t)Offset, SeekFlag == SeekStart ? SEEK_SET : (SeekFlag == SeekEnd ? SEEK_END : SEEK_CUR));
	else fseek(m_FileObject, Offset, SeekFlag == SeekStart ? SEEK_SET : (SeekFlag == SeekEnd ? SEEK_END : SEEK_CUR));
	return *this;
}

LWFileStream &LWFileStream::Finished(void) {
	if (m_FileObject) {
		if (m_Flag&AssetMode) AAsset_close((AAsset*)m_FileObject);
		else fclose(m_FileObject);
		m_FileObject = nullptr;
	}
	return *this;
}

uint32_t LWFileStream::GetPosition(void) const {
	if (m_Flag&AssetMode) return m_Length-(uint32_t)AAsset_getRemainingLength((AAsset*)m_FileObject);
	return (uint32_t)ftell(m_FileObject);
}

bool LWFileStream::EndOfStream(void) const {
	if (m_Flag&AssetMode) return (uint32_t)AAsset_getRemainingLength((AAsset*)m_FileObject) == 0;
	return feof(m_FileObject) != 0;
}

LWFileStream::LWFileStream(FILE *FileObject, const LWText &FilePath, uint32_t Flag, LWAllocator &Allocator) : m_FileObject(FileObject), m_FilePath(LWText(FilePath.GetCharacters(), Allocator)), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(Flag) {
	char Buffer[512];
	MakeDirectoryPath(m_FilePath, Buffer, sizeof(Buffer));
	m_DirPath = LWText(Buffer, Allocator);
	if (m_FileObject) {
		if (m_Flag&AssetMode) m_Length = (uint32_t)AAsset_getLength((AAsset*)m_FileObject);
		else {
			fseek(m_FileObject, 0, SEEK_END);
			m_Length = (uint32_t)ftell(m_FileObject);
			fseek(m_FileObject, 0, SEEK_SET);
		}
	}
}
