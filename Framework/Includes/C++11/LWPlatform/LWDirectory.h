#ifndef LWDIRECTORY_H
#define LWDIRECTORY_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWText.h>
#include <LWPlatform/LWTypes.h>
#include <LWPlatform/LWPlatform.h>

/*! \addtogroup LWPlatform LWPlatform
	@{
*/
/*! \brief the maximum length an individual file can be. */
#define LWMAXFILENAME_SIZE 256
/*! \brief the maximum number of files the LWDirectory object can hold. */
#define LWMAXFILES 256

/*! \brief individual file/directory object, which contains supplementary information about the file.*/

struct LWFile{
	char m_Name[LWMAXFILENAME_SIZE]; /*!< \brief 256 byte buffer to contain the associated filename of the file in question. */
	uint64_t m_Size; /*!< \brief holds the size of the file, for directorys this will be 0, note that being 0 is not a reliable way to check if the file is actually a directory, be sure to check the flag. */
	uint32_t m_Flag; /*!< \brief supplementary flag that represents the file status, see LWDirectory for a list of possible flags this file can contain, and there supplimentary meaning. */
};
	
/*! \brief LWDirectory cache's upto LWMAXFILES files/directorys at a time, if the application expects to encounter more than 256 files, then it is recommended to use a different directory managing class, or recompiling this module with a higher limit.*/
class LWDirectory{
public:
	static const uint32_t Directory = 0x1; /*!< \brief flag which represents the file is actually a directory. */
	static const uint32_t Hidden = 0x2; /*!< \brief flag which represents the file is marked as hidden by the os. */
	static const uint32_t ReadOnly = 0x4; /*!< \brief flag which represents that the file is marked for read only. */

	/*! \brief checks if the actual directory exists or not. */
	static bool DirExists(const LWText &DirectoryPath, const LWFileStream *ExistingStream = nullptr);

	/*! \brief opens a directory path if possible, if failure occurs, false is returned. */
	static bool OpenDir(LWDirectory &DirObject, const LWText &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream = nullptr);

	/*! \brief opens a directory path with a formatted text stream. */
	static bool OpenDirf(LWDirectory &DirObject, const LWText &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...);

	/*! \brief Creates the specified directory.
		\param DirectoryPath the path to the directory to be created.
		\param ExistingStream any existing stream that should be taken into account when creating the path.
		\param MakeParents will create any parent directorys that also do not exist.
	*/
	static bool CreateDir(const LWText &DirectoryPath, const LWFileStream *ExistingStream, bool MakeParents);

	/*! \brief Creates the specified directory with a formatted string. */
	static bool CreateDirf(const LWText &DirectoryPath, const LWFileStream *ExistingStream, bool MakeParents, ...);

	/*! \brief move operator. */
	LWDirectory &operator = (LWDirectory &&Other);

	/*! \brief searches the directory for the specified file or folder. returns null if not found. */
	LWFile *FindFile(const LWText &FileName);

	/*! \brief returns the file/folder at the specified index, Index is valid from FileCount+FolderCount length, no guarantee on order of files/folders, may return null if out of bounds. */
	LWFile *GetFile(uint32_t i);

	/*! \brief returns the total number of non-directoy files contained in the directory. */
	uint32_t GetFileCount(void);

	/*! \brief returns the total number of folders contained in the directory. */
	uint32_t GetFolderCount(void);

	/*! \brief returns the directory path of this directory. */
	const LWText &GetDirectoryPath(void);

	/*! \brief constructs a default move constructor. */
	LWDirectory(LWDirectory &&Other);

	/*! \brief constructs an LWDirectory object. */
	LWDirectory(const LWText &DirectoryPath, LWFile *FileList, uint32_t FileCount, LWAllocator &Allocator);

	LWDirectory();
private:
	LWText m_DirectoryPath;
	LWFile m_FileList[256];
	uint32_t m_FolderCount = 0;
	uint32_t m_FileCount = 0;
	
};
/*! @} */
#endif