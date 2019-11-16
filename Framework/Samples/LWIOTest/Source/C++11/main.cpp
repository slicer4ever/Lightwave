#include <LWCore/LWAllocator.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWFileStream.h>
#include <LWPlatform/LWDirectory.h>
#include <iostream>
#include <errno.h>
//This test runs through the LWFileStream methods, and creates and destroys files.

bool TestFolderPath(const char *FolderName, uint32_t FolderID){
	char Buffer[256];
	if(!LWFileStream::GetFolderPath(FolderID, Buffer, sizeof(Buffer))){
		std::cout << "Error retrieving '" << FolderName << "' path." << std::endl;
		return false;
	}
	std::cout << FolderName << ": '" << Buffer << "'" << std::endl;
	return true;
}

int LWMain(int, char **){
	//This sample does not compile for android due to write permissions being denied for anywhere except the sdcard.

	std::cout << "Checking file paths:" << std::endl;
	LWAllocator_Default Allocator;
	char Buffer[256];
    char NameBuffer[256];
	LWText TargetFile = LWText("User:Sample.bin");
    LWFileStream::MakeFileName(TargetFile, NameBuffer, sizeof(NameBuffer));
	if(LWFileStream::MakeAbsolutePath(LWText(""), Buffer, sizeof(Buffer))==0){
		std::cout << "Error retrieving working directory." << std::endl;
		return 0;
	}
	std::cout << "Working Directory: '" << Buffer << "'" << std::endl;
	if (!TestFolderPath("Game", LWFileStream::Game)) return 0;
	if (!TestFolderPath("Fonts", LWFileStream::Fonts)) return 0;
	if (!TestFolderPath("App", LWFileStream::App)) return 0;
	if (!TestFolderPath("User", LWFileStream::User)) return 0;
	std::cout << "Target File for working: '" << TargetFile.GetCharacters() << "'"<<std::endl;
	if(LWFileStream::MakeAbsolutePath(TargetFile, Buffer, sizeof(Buffer))==0){
		std::cout << "Error retrieving absolute path to target file." << std::endl;
		return 0;
	}
	std::cout << "Absolute path to target: '" << Buffer << "'" << std::endl;
	if(LWFileStream::MakeDirectoryPath(LWText(Buffer), Buffer, sizeof(Buffer))==0){
		std::cout << "Error retrieving directory path to target file." << std::endl;
		return 0;
	}
	std::cout << "Absolute directory path to target: '" << Buffer << "'" << std::endl;

	if(LWFileStream::PathIsAbsolute(TargetFile)){
		std::cout << "Error Path was reported as absolute incorrectly." << std::endl;
		return 0;
	}
	if(!LWFileStream::PathIsAbsolute(Buffer)){
		std::cout << "Error Path was not reported as absolute correctly." << std::endl;
		return 0;
	}
	if(LWFileStream::GetExtension(TargetFile, Buffer, sizeof(Buffer))==0){
		std::cout << "Error extension was not found for target file." << std::endl;
		return 0;
	}
	std::cout << "Target file extension: '" << Buffer << "'" << std::endl;
	if(!LWFileStream::IsExtension(TargetFile, "bin")){
		std::cout << "Error extension reported incorrectly." << std::endl;
		return 0;
	}
	if(LWFileStream::IsExtensions(TargetFile, 3, "abc", "def", "bin")!=2){
		std::cout << "Error extension was reported incorrectly." << std::endl;
		return 0;
	}
	if(!LWDirectory::CreateDir("User:files/", nullptr, false)){
		std::cout << "Error making files directory: " << errno << std::endl;
		return 0;
	}
	std::cout << "Opening file to write to." << std::endl;
	LWFileStream File;
    
	if(!LWFileStream::OpenStream(File, TargetFile, LWFileStream::WriteMode|LWFileStream::BinaryMode, Allocator)){
		std::cout << "Error creating/opening target file for write mode." << std::endl;
		return 0;
	}
	std::cout << "File successfully opened: " << std::endl;
	const char DataBuffer[] = { "a man had a good day. and he was a fun person to be around." };
	uint32_t i = 0;
	File.WriteByte(*DataBuffer); i++;
	File.Write(DataBuffer + i, 20); i += 20;
	File.WriteText(LWText(DataBuffer+i));
	std::cout << "Wrote file, closing now." << std::endl;
	File.Finished();
	if (!LWFileStream::OpenStream(File, TargetFile, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)){
		std::cout << "Error opening target file for read mode." << std::endl;
		return 0;
	}
	std::cout << "File opened. Created at: " << File.GetCreatedTime() << " Modified at: " << File.GetModifiedTime() << " Accessed at: " << File.GetAccessedTime() << " length: " << File.Length() << std::endl;
	if(File.Length()!=sizeof(DataBuffer)-1){
		std::cout << "Error, file was not the size expected." << std::endl;
		return 0;
	}
	File.Seek(0, LWFileStream::SeekEnd);
	if(File.GetPosition()!=File.Length()){
		std::cout << "Error, seek to end failed." << std::endl;
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	if(File.GetPosition()!=0){
		std::cout << "Error, seek to start failed." << std::endl;
		return 0;
	}
	i = 0;
	while(!File.EndOfStream()){
		Buffer[i++] = File.ReadByte();
	}
	Buffer[i-1] = '\0'; //End of file is triggered after we do a failed read, so we need to backpedal.
	if(LWText(DataBuffer)!=LWText(Buffer)){
		std::cout << "Error, reading back buffer failed: '" << Buffer << "'" << std::endl;
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	File.ReadText(Buffer, sizeof(Buffer));
	if(LWText(DataBuffer)!=LWText(Buffer)){
		std::cout << "Error, reading back entire buffer failed." << std::endl;
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	File.Read(Buffer, sizeof(DataBuffer));
	if(LWText(DataBuffer)!=LWText(Buffer)){
		std::cout << "Error, reading full buffer failed." << std::endl;
		return 0;
	}
	std::cout << "Successfully read back file contents: '" << Buffer << "'" << std::endl;
	std::cout << "FilePath: '" << File.GetFilePath().GetCharacters() << "'" << std::endl;
	std::cout << "DirPath: '" << File.GetDirectoryPath().GetCharacters() << "'" << std::endl;
	File.Finished();
	std::cout << "Beginning directory tests for local directory!" << std::endl;
	if(!LWDirectory::DirExists(File.GetDirectoryPath(), nullptr)){
		std::cout << "Directory incorrectly reported as not existing." << std::endl;
		return 0;
	}
	if(!LWDirectory::CreateDir("User:Test/TestB/TestC/", nullptr, true)){
		std::cout << "Error could not create test directory structure." << std::endl;
		return 0;
	}
	LWDirectory Dir;
	if(!LWDirectory::OpenDir(Dir, File.GetDirectoryPath(), Allocator)){
		std::cout << "Error could not open the specified directory." << std::endl;
		return 0;
	}
	std::cout << "Directory opened: '" << Dir.GetDirectoryPath().GetCharacters() << "' Folder Count: " << Dir.GetFolderCount() << " File Count: " << Dir.GetFileCount() << std::endl;
	uint32_t TotalCount = Dir.GetFileCount() + Dir.GetFolderCount();
	for (uint32_t d = 0; d < TotalCount; d++){
		LWFile *F = Dir.GetFile(d);
		std::cout << "File: '" << F->m_Name << "' Size: " << F->m_Size << " Directory: " << ((F->m_Flag&LWDirectory::Directory) ? "Yes" : "No") << " Hidden: " << ((F->m_Flag&LWDirectory::Hidden) ? "Yes" : "No") << " Read-only: " << ((F->m_Flag&LWDirectory::ReadOnly) ? "Yes" : "No") << std::endl;
	}
	if(!Dir.FindFile(LWText(NameBuffer))){
		std::cout << "Error, could not locate target file!" << std::endl;
		return 0;
	}
    if(!LWDirectory::OpenDir(Dir, LWText("Fonts:/"), Allocator)){
        std::cout << "Error could not open the font directory." << std::endl;
        return 0;
    }
    TotalCount = Dir.GetFileCount()+Dir.GetFolderCount();
    std::cout << "Font directory opened: '" << Dir.GetDirectoryPath().GetCharacters() << "' Folder Count: " << Dir.GetFolderCount() << " File count: " << Dir.GetFileCount() << std::endl;
    for (uint32_t d = 0; d < TotalCount; d++){
		LWFile *F = Dir.GetFile(d);
		std::cout << "File: '" << F->m_Name << "' Size: " << F->m_Size << " Directory: " << ((F->m_Flag&LWDirectory::Directory) ? "Yes" : "No") << " Hidden: " << ((F->m_Flag&LWDirectory::Hidden) ? "Yes" : "No") << " Read-only: " << ((F->m_Flag&LWDirectory::ReadOnly) ? "Yes" : "No") << std::endl;
	}
	
	std::cout << "LWDirectory and LWFilestream successfully tested!" << std::endl;
	return 0;
}