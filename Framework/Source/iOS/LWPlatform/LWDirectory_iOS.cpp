#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>

bool LWDirectory::DirExists(const LWText &DirectoryPath, const LWFileStream *ExistingStream){
	char Buffer[256]={"./"};
	int Offset = LWFileStream::PathIsAbsolute(DirectoryPath)?0:2;
	if (!LWFileStream::ParsePath(DirectoryPath, Buffer+Offset, sizeof(Buffer), ExistingStream)) return false;
	DIR *D = opendir(Buffer);
	if (!D) return false;
	closedir(D);
	return true;
}

bool LWDirectory::OpenDir(LWDirectory &DirObject, const LWText &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream){
	char Buffer[256]={"./"};
	char ParseBuffer[256];
    char FileBuffer[256];
	LWFile Files[LWMAXFILES];
	if (!LWFileStream::ParsePath(DirectoryPath, ParseBuffer, sizeof(Buffer), ExistingStream)) return false;
	uint32_t BufferOffset = LWFileStream::PathIsAbsolute(ParseBuffer)?0:2;
    strcpy(Buffer+BufferOffset, ParseBuffer);
	DIR *D = opendir(Buffer);
	if (!D) return false;
	uint32_t BufferLen = strlen(Buffer);
	if (BufferLen>0 && (Buffer[BufferLen-1] != '/' || Buffer[BufferLen-1] != '\\')) {
		Buffer[BufferLen] = '/';
		Buffer[BufferLen + 1] = '\0';
	}
	uint32_t Count = 0;
	struct dirent *f = nullptr;
	struct stat s;
	for (; (f = readdir(D))!=nullptr && Count<=LWMAXFILES; ){
		uint32_t Len = (uint32_t)std::strlen(f->d_name) + 1;
		uint32_t BLen = sizeof(Files[Count].m_Name);
		std::memcpy(Files[Count].m_Name, f->d_name, std::min<uint32_t>(Len, BLen));
		snprintf(FileBuffer, sizeof(FileBuffer), "%s%s", Buffer, f->d_name);
		if (stat(FileBuffer, &s) == -1) continue;
		if (Len >= BLen) Files[Count].m_Name[BLen - 1] = '\0';
		Files[Count].m_Flag = 0;
		Files[Count].m_Size = (uint64_t)s.st_size;
		Files[Count].m_Flag |= S_ISDIR(s.st_mode)? Directory : 0;
		Files[Count].m_Flag |= (*f->d_name == '.' && !strcmp(f->d_name, ".") && !strcmp(f->d_name, "..")) ? Hidden : 0;
		Files[Count].m_Flag |= access(FileBuffer, R_OK) == 0 ? (access(FileBuffer, W_OK) == 0 ? 0 : ReadOnly) : 0;
		Count++;
	}
	DirObject = LWDirectory(LWText(Buffer), Files, Count, Allocator);
	closedir(D);
	return true;
}

bool LWDirectory::CreateDir(const LWText &DirectoryPath, const LWFileStream *ExistingStream, bool MakeParents){
	char Buffer[256];
	if (!LWFileStream::ParsePath(DirectoryPath.GetCharacters(), Buffer, sizeof(Buffer), ExistingStream)) return false;
    if (MakeParents){
		char *Last = nullptr;
		for (char *P = Buffer; *P; P++){
			if (*P == '/' || *P == '\\'){
				if (Last){
					char Prev = *(Last + 1);
					*(Last + 1) = '\0';
                    errno = 0;
					if (mkdir(Buffer, S_IRWXU|S_IRWXG|S_IRWXG)!=0){
						int Error = errno;
                        if (Error!=EEXIST && Error!=EACCES && Error!=EISDIR) return false;
                        
					}
					*(Last + 1) = Prev;
				}
				Last = P;
			}
		}
	}
    errno = 0;
	if (mkdir(Buffer, S_IRWXU | S_IRWXG | S_IRWXG)!=0){
        int Error = errno;
        if (Error!=EEXIST && Error!=EISDIR) return false;
	}
	return true;
}

