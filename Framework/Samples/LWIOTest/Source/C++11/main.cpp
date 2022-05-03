#include <LWCore/LWAllocator.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWFileStream.h>
#include <LWPlatform/LWDirectory.h>
#include <LWCore/LWLogger.h>
#include <iostream>
#include <errno.h>
//This test runs through the LWFileStream methods, and creates and destroys files.

LWLOG_DEFAULT

bool TestFolderPath(const LWUTF8Iterator &FolderName, uint32_t FolderID){
	char8_t Buffer[256];
	if(!LWFileStream::GetFolderPath(FolderID, Buffer, sizeof(Buffer))){
		fmt::print("Error retrieving {} path.\n", FolderName);
		return false;
	}
	fmt::print("{}: '{}'\n", FolderName, LWUTF8Iterator(Buffer));
	return true;
}

int LWMain(int32_t, LWUTF8Iterator *){
	const char8_t DataBuffer[] = u8"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mollis vulputate augue id rutrum. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla vel sem enim. Duis placerat ligula dui, nec lacinia lorem finibus nec. Integer eget convallis libero, eu euismod odio. Integer varius, risus vel finibus cursus, massa metus euismod orci, in posuere ipsum elit vitae augue. Nullam dictum, arcu non gravida euismod, metus metus sollicitudin quam, vitae rhoncus sapien dui sit amet urna. Sed euismod vulputate urna.";
	char8_t Buffer[1024];
	//This sample does not compile for android due to write permissions being denied for anywhere except the sdcard.
	fmt::print("Checking file paths:\n");
	LWAllocator_Default Allocator;
	LWUTF8Iterator DirPath, FileName, FileExt;
	LWUTF8 TargetFile = LWUTF8(u8"User:/Sample.bin", Allocator);
	if (!LWFileStream::GetWorkingDirectory(Buffer, sizeof(Buffer))) {
		fmt::print("Error retrieving working directory.\n");
		return 0;
	}
	fmt::print("Working directory: '{}'\n", LWUTF8Iterator(Buffer));
	LWFileStream::MakeAbsolutePath(*TargetFile, Buffer, sizeof(Buffer));
	LWFileStream::SplitPath(LWUTF8Iterator(Buffer), DirPath, FileName, FileExt);
	fmt::print("Target absolute path: '{}'\n", LWUTF8Iterator(Buffer));
	fmt::print("Target directory: '{}' Target Name: '{}' Target Ext: '{}'\n", DirPath, FileName, FileExt);
	if (!TestFolderPath("Game", LWFileStream::Game)) return 0;
	if (!TestFolderPath("Fonts", LWFileStream::Fonts)) return 0;
	if (!TestFolderPath("App", LWFileStream::App)) return 0;
	if (!TestFolderPath("User", LWFileStream::User)) return 0;
	if(LWFileStream::PathIsAbsolute(*TargetFile)) {
		fmt::print("Error '{}' was reported as absolute incorrectly.\n", TargetFile);
		return 0;
	}
	if(!LWFileStream::PathIsAbsolute(Buffer)){
		fmt::print("Error '{}' was not reported as absolute correctly.\n", LWUTF8Iterator(Buffer));
		return 0;
	}
	if(!LWFileStream::IsExtension(Buffer, "bin")){
		fmt::print("Error extension reported incorrectly.\n");
		return 0;
	}
	if(LWFileStream::IsExtensions(Buffer, "abc", "def", "bin")!=2){
		fmt::print("Error extension was reported incorrectly.\n");
		return 0;
	}
	if(!LWDirectory::CreateDir("User:files/", nullptr)){
		fmt::print("Error making files directory: {}\n", errno);
		return 0;
	}
	fmt::print("Opening file to write to.\n");
	LWFileStream File;
	if(!LWFileStream::OpenStream(File, *TargetFile, LWFileStream::WriteMode|LWFileStream::BinaryMode, Allocator)){
		fmt::print("Error opening file '{}' {}\n", *TargetFile, errno);
		return 0;
	}
	uint32_t i = 0;
	File.WriteByte(*DataBuffer); i++;
	File.Write(DataBuffer + i, 20); i += 20;
	File.WriteText(DataBuffer+i);
	fmt::print("Write file, closing now.\n");
	File.Finished();
	if (!LWFileStream::OpenStream(File, *TargetFile, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)){
		fmt::print("Error opening '{}' for read mode.\n", *TargetFile);
		return 0;
	}
	fmt::print("File opened. Created at: {} Modified at: {} Accessed at: {} Length: {} - {}\n", File.GetCreatedTime(), File.GetModifiedTime(), File.GetAccessedTime(), File.Length(), sizeof(DataBuffer));
	if(File.Length()!=sizeof(DataBuffer)){
		fmt::print("Error, file was not the size expected.\n");
		return 0;
	}
	File.Seek(0, LWFileStream::SeekEnd);
	if(File.GetPosition()!=File.Length()){
		fmt::print("Error, file seek to end failed.\n");
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	if(File.GetPosition()!=0){
		fmt::print("Error, file seek to start failed.\n");
		return 0;
	}
	i = 0;
	while(!File.EndOfStream()) Buffer[i++] = File.ReadByte();
	Buffer[i-1] = '\0'; //End of file is triggered after we do a failed read, so we need to backpedal.
	if(!LWUTF8Iterator(DataBuffer).Compare(Buffer)) {
		fmt::print("Error, reading back buffer failed: '{}'\n", Buffer);
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	File.ReadText(Buffer, sizeof(Buffer));
	if(!LWUTF8Iterator(DataBuffer).Compare(LWUTF8Iterator(Buffer))) {
		fmt::print("Error, reading back entire buffer failed.\n");
		return 0;
	}
	File.Seek(0, LWFileStream::SeekStart);
	File.Read(Buffer, sizeof(DataBuffer));
	if(!LWUTF8Iterator(DataBuffer).Compare(LWUTF8Iterator(Buffer))) {
		fmt::print("Error, reading full buffer failed.\n");
		return 0;
	}
	fmt::print("Successfully read back file contents: '{}'\n", Buffer);
	fmt::print("FilePath: '{}'\n", *File.GetFilePath());
	File.Finished();
	fmt::print("Beginning directory tests for local directory:\n");
	LWFileStream::SplitPath(*File.GetFilePath(), DirPath, FileName);
	if (!LWDirectory::DirExists(DirPath)) {
		fmt::print("Directory '{}' incorrectly reported as not existing.\n", DirPath);
		return 0;
	}
	if (!LWDirectory::CreateDir("User:Test/TestB/TestC/")) {
		fmt::print("Failed to create directory '{}'\n", "User:Test/TestB/TestC/");
		return 0;
	}
	LWDirectory Dir;
	if(!LWDirectory::OpenDir(Dir, DirPath, Allocator)){
		fmt::print("Error could not open the '{}' directory.\n", DirPath);
		return 0;
	}
	fmt::print("Directory opened: '{}' Folder Count: {} File Count: {}\n", Dir.GetDirectoryPath(), Dir.GetFolderCount(), Dir.GetFileCount());
	uint32_t TotalCount = Dir.GetTotalFiles();
	for (uint32_t d = 0; d < TotalCount; d++){
		const LWFile *F = Dir.GetFile(d);
		fmt::print("{}: Name: '{}' Size: {} isDir: {} isHidden: {} isReadable: {} isWriteable: {}\n", d, F->m_Name, F->m_Size, F->isDirectory(), F->isHidden(), F->isReadable(), F->isWriteable());
	}
	if (!Dir.FindFile(FileName)) {
		fmt::print("Error, could not locate file: '{}'\n", FileName);
		return 0;
	}
	if(!LWDirectory::OpenDir(Dir, "Fonts:", Allocator)) {
		fmt::print("Error could not open the font directory.\n");
        return 0;
    }
    TotalCount = Dir.GetTotalFiles();
	fmt::print("Font directory opened: '{}' Folder Count: {} File Count: {}\n", Dir.GetDirectoryPath(), Dir.GetFolderCount(), Dir.GetFileCount());
    for (uint32_t d = 0; d < TotalCount; d++){
		const LWFile *F = Dir.GetFile(d);
		fmt::print("{}: Name: '{}' Size: {} isDir: {} isHidden: {} isReadable: {} isWriteable: {}\n", d, F->m_Name, F->m_Size, F->isDirectory(), F->isHidden(), F->isReadable(), F->isWriteable());
	}
	fmt::print("LWDirectory and LWFilestream successfully tested!\n");
	return 0;
}