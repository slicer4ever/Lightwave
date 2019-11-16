#include "LWPlatform/LWDirectory.h"
#include <cstdarg>

bool LWDirectory::OpenDirf(LWDirectory &DirObject, const LWText &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...){
	char Buffer[256];
	va_list lst;
	va_start(lst, ExistingStream);

	snprintf(Buffer, sizeof(Buffer), (const char*)DirectoryPath.GetCharacters(), lst);
	va_end(lst);
	return OpenDir(DirObject, LWText(Buffer), Allocator, ExistingStream);
}

bool LWDirectory::CreateDirf(const LWText &DirectoryPath, const LWFileStream *ExistingStream, bool MakeParents, ...){
	char Buffer[256];
	va_list lst;
	va_start(lst, MakeParents);
	snprintf(Buffer, sizeof(Buffer), (const char*)DirectoryPath.GetCharacters(), lst);
	va_end(lst);
	return CreateDir(LWText(Buffer), ExistingStream, MakeParents);
}


LWDirectory &LWDirectory::operator = (LWDirectory &&Other){
	m_DirectoryPath = std::move(Other.m_DirectoryPath);
	m_FolderCount = Other.m_FolderCount;
	m_FileCount = Other.m_FileCount;
	std::memcpy(m_FileList, Other.m_FileList, sizeof(LWFile)*(m_FolderCount + m_FileCount));
	return *this;
}

LWFile *LWDirectory::FindFile(const LWText &FileName){
	uint32_t Len = m_FileCount + m_FolderCount;
	for(uint32_t i=0;i<Len;i++){
		if (LWText(m_FileList[i].m_Name) == FileName) return m_FileList + i;
	}
	return nullptr;
}

LWFile *LWDirectory::GetFile(uint32_t i){
	return i >= (m_FolderCount + m_FileCount) ? nullptr : m_FileList + i;
}

uint32_t LWDirectory::GetFileCount(void){
	return m_FileCount;
}

uint32_t LWDirectory::GetFolderCount(void){
	return m_FolderCount;
}

const LWText &LWDirectory::GetDirectoryPath(void){
	return m_DirectoryPath;
}

LWDirectory::LWDirectory(LWDirectory &&Other) : m_DirectoryPath(std::move(Other.m_DirectoryPath)), m_FolderCount(Other.m_FolderCount), m_FileCount(Other.m_FileCount){
	std::memcpy(m_FileList, Other.m_FileList, sizeof(LWFile)*(m_FolderCount + m_FileCount));
}

LWDirectory::LWDirectory(const LWText &DirectoryPath, LWFile *FileList, uint32_t FileCount, LWAllocator &Allocator) : m_DirectoryPath(DirectoryPath.GetCharacters(), Allocator){
	std::memcpy(m_FileList, FileList, sizeof(LWFile)*FileCount);
	for(uint32_t i=0;i<FileCount;i++){
		if (m_FileList[i].m_Flag&Directory) m_FolderCount++;
		else m_FileCount++;
	}
}

LWDirectory::LWDirectory(){}