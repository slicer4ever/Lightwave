#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>

//LWFile:
bool LWFile::isDirectory(void) const {
	return (m_Flag & LWDirectory::Directory) != 0;
}

bool LWFile::isHidden(void) const {
	return (m_Flag & LWDirectory::Hidden) != 0;
}

bool LWFile::isReadable(void) const {
	return (m_Flag & LWDirectory::CanRead) != 0;
}

bool LWFile::isWriteable(void) const {
	return (m_Flag & LWDirectory::CanWrite) != 0;
}

LWUTF8Iterator LWFile::GetName(void) const {
	return m_Name;
}

LWFile::LWFile(const LWUTF8Iterator &Name, uint64_t Size, uint32_t Flag) : m_Size(Size), m_Flag(Flag) {
	Name.Copy(m_Name, sizeof(m_Name));
}

//LWDirectory:
bool LWDirectory::CreateDir(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream) {
	char8_t Buffer[1024];
	uint32_t Len = 0;
	if (!(Len = LWDirectory::ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream))) return false;
	LWUTF8Iterator C = LWUTF8Iterator(Buffer, Len);
	LWUTF8Iterator P = C;
	for (; !C.AtEnd();++C) {
		if (*C == '/' || *C == '\\') {
			if (P != C) {
				if (!CreateDir(LWUTF8Iterator(P, C), true)) return false;
			}
		}
	}
	if (P != C) return CreateDir(LWUTF8Iterator(P, C), false);
	return true;
}

uint32_t LWDirectory::ParsePath(const LWUTF8Iterator &DirectoryPath, char8_t *Buffer, uint32_t BufferLen, const LWFileStream *ExistingStream) {
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = LWFileStream::ParsePath(DirectoryPath, Buffer, BufferLen, ExistingStream);
	if (!o) return 0;
	B = std::min<char8_t*>(B + --o, BL);
	if (BufferLen) {
		if (!LWFileStream::PathIsAbsolute(Buffer)) {
			if (B + 2 < BL) {
				std::copy_backward(Buffer, B, B + 2);
				Buffer[0] = '.';
				Buffer[1] = '/';
				B += 2;
			}
			o += 2;
		}
	} else o += 2; //Add space for possible './'.
	if (BufferLen) {
		if (*(B - 1) != '/' && *(B - 1) != '\\') {
			if (B != BL) *B++ = '/';
			o++;
		}
		*B = '\0';
	} else o++; //Include space for possible '/' although we won't know if it's needed until after Buffer is correctly constructed.
	return o+1;
}

bool LWDirectory::PushFile(const LWFile &F) {
	m_FileList.push_back(F);
	if (F.isDirectory()) m_FolderCount++;
	else m_FileCount++;
	return true;
}

LWDirectory &LWDirectory::operator = (LWDirectory &&Other){
	m_DirectoryPath = std::move(Other.m_DirectoryPath);
	m_FileList = std::move(Other.m_FileList);
	m_FolderCount = Other.m_FolderCount;
	m_FileCount = Other.m_FileCount;
	return *this;
}

const LWFile *LWDirectory::FindFile(const LWUTF8Iterator &FileName) const{
	for (auto &&F : m_FileList) {
		if (FileName.Compare(F.m_Name)) return &F;
	}
	return nullptr;
}

const LWFile *LWDirectory::GetFile(uint32_t i) const {
	return i >= (uint32_t)m_FileList.size() ? nullptr : &m_FileList[i];
}

uint32_t LWDirectory::GetTotalFiles(void) const {
	return (uint32_t)m_FileList.size();
}

uint32_t LWDirectory::GetFileCount(void) const {
	return m_FileCount;
}

uint32_t LWDirectory::GetFolderCount(void) const {
	return m_FolderCount;
}

const LWUTF8 &LWDirectory::GetDirectoryPath(void) const{
	return m_DirectoryPath;
}

LWDirectory::LWDirectory(LWDirectory &&Other) : m_DirectoryPath(std::move(Other.m_DirectoryPath)), m_FolderCount(Other.m_FolderCount), m_FileCount(Other.m_FileCount){
	m_FileList = std::move(Other.m_FileList);
}

LWDirectory::LWDirectory(const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator) : m_DirectoryPath(DirectoryPath, Allocator){
	
}