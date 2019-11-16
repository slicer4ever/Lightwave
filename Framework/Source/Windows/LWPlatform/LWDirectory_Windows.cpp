#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>
#include <algorithm>
#include <cstring>

bool LWDirectory::DirExists(const LWText &DirectoryPath, const LWFileStream *ExistingStream){
	char Buffer[256];
	if (!LWFileStream::ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream)) return false;
	uint32_t Len = (uint32_t)std::strlen(Buffer);
	if (Len + 3 >= sizeof(Buffer)) return false;
	if (Len != 0 && (*(Buffer + Len - 1) != '\\' || *(Buffer + Len - 1) != '/'))  *(Buffer + (Len++)) = '\\';
	*(Buffer + (Len++)) = '*';
	*(Buffer + (Len++)) = '\0'; 
	WIN32_FIND_DATA FileData;
	HANDLE H = FindFirstFileA(Buffer, &FileData);
	if (H == INVALID_HANDLE_VALUE) return false;
	FindClose(H);
	return true;
}

bool LWDirectory::OpenDir(LWDirectory &DirObject, const LWText &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream){
	char Buffer[256];
	LWFile Files[LWMAXFILES];
	if (!LWFileStream::ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream)) return false;
	uint32_t Len = (uint32_t)std::strlen(Buffer);
	if (Len + 3 >= sizeof(Buffer)) return false;
	if (Len!=0 && (*(Buffer + Len - 1) != '\\' || *(Buffer + Len - 1) != '/'))  *(Buffer + (Len++)) = '\\';
	*(Buffer + (Len++)) = '*';
	*(Buffer + (Len++)) = '\0';
	WIN32_FIND_DATA FileData;
	HANDLE H = FindFirstFileA(Buffer, &FileData);
	if (H == INVALID_HANDLE_VALUE) return false;
	uint32_t Count = 0;
	for (; FindNextFile(H, &FileData) && Count<LWMAXFILES; Count++){
		uint32_t Length = (uint32_t)std::strlen(FileData.cFileName)+1;
		uint32_t BLen = sizeof(Files[Count].m_Name);
		std::memcpy(Files[Count].m_Name, FileData.cFileName, std::min<uint32_t>(Length, BLen));
		if (Length >= BLen) Files[Count].m_Name[BLen - 1] = '\0';
		Files[Count].m_Flag = 0;
		Files[Count].m_Size = (uint64_t)FileData.nFileSizeHigh << 32 | (uint64_t)FileData.nFileSizeLow;
		Files[Count].m_Flag |= (FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? Directory : 0;
		Files[Count].m_Flag |= (FileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) ? Hidden : 0;
		Files[Count].m_Flag |= (FileData.dwFileAttributes&FILE_ATTRIBUTE_READONLY) ? ReadOnly : 0;
	}
	*(Buffer + (Len - 2)) = '\0';
	DirObject = LWDirectory(LWText(Buffer), Files, Count, Allocator);
	return true;
	
}

bool LWDirectory::CreateDir(const LWText &DirectoryPath, const LWFileStream *ExistingStream, bool MakeParents){
	char Buffer[256];
	if (!LWFileStream::ParsePath(DirectoryPath.GetCharacters(), Buffer, sizeof(Buffer), ExistingStream)) return false;
	if(MakeParents){
		char *Last = nullptr;
		for(char *P = Buffer; *P; P++){
			if(*P=='/' || *P=='\\'){
				if(Last){
					char Prev = *(Last + 1);
					*(Last + 1) = '\0';
					if(!CreateDirectory(Buffer, nullptr)){
						int Error = GetLastError();
						if (Error != ERROR_ALREADY_EXISTS && Error != ERROR_ACCESS_DENIED) return false;
					}
					*(Last + 1) = Prev;
				}
				Last = P;
			}
		}
	}
	if(!CreateDirectory(Buffer, nullptr)){
		if (GetLastError() != ERROR_ALREADY_EXISTS) return false;
	}
	return true;
}

